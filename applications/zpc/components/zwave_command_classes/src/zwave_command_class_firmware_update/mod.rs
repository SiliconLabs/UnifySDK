use crate::{call_once, ToFrameBuffer, ZwaveHandlerConfig};
use std::{
    ffi::c_void,
    fs::File,
    io::{Read, Seek, SeekFrom},
    time::SystemTime,
    usize,
};

mod definitions;
mod test;
use definitions::*;

use ::zwave_tx::*;
use crc16::*;
use uic_attribute_store::*;
use uic_log::{log_debug, log_error, log_info, log_warning};
use uic_proc_macro::{as_extern_c, generate_extern_c};
use zwave_command_classes::zwave_command_class_get_endpoint_node;
use zwave_controller::{sl_status_t, zwave_node_get_command_class_version};
use zwave_rust_proc_macros::zwave_command_class;
const LOG_TAG: &'static str = "zwave_cc_fwu";

impl From<&[u8]> for ZwaveCommandClassFrameHeader {
    fn from(frame: &[u8]) -> Self {
        ZwaveCommandClassFrameHeader {
            command_class: frame[0] as u16,
            command: frame[1] as u16,
        }
    }
}

/// Convert a byte array into ZwaveFirmwareMdReport
impl From<&[u8]> for ZwaveFirmwareMdReport {
    fn from(frame: &[u8]) -> Self {
        let header = ZwaveCommandClassFrameHeader {
            command_class: frame[0] as u16,
            command: frame[1] as u16,
        };
        // If frame is smaller than Version 1 frame, return None
        if frame.len() < 8 {
            return ZwaveFirmwareMdReport::None;
        }
        // If it isn't the correct CC return None
        if header.command_class != COMMAND_CLASS_FIRMWARE_UPDATE_MD
            || header.command != FIRMWARE_MD_REPORT
        {
            return ZwaveFirmwareMdReport::None;
        }
        let manufacturer_id: u16 = ((frame[2] as u16) << 8) | frame[3] as u16;
        let firmware_0_id: u16 = ((frame[4] as u16) << 8) | frame[5] as u16;
        let firmware_0_checksum: u16 = ((frame[6] as u16) << 8) | frame[7] as u16;
        // End of minimum frame (V1)
        let firmware_upgradable = *frame.get(8).unwrap_or(&DEFAULT_FIRMWARE_UPGRADABLE);
        let number_of_firmware_targets =
            *frame.get(9).unwrap_or(&DEFAULT_NUMBER_OF_FIRMWARE_TARGETS);
        let max_fragment_size = frame
            .get(10..12)
            .map(|v| -> u16 { ((v[0] as u16) << 8) | v[1] as u16 })
            .unwrap_or(MAX_FRAME_LEN as u16);
        let mut firmware_n_id: Vec<u16> = Vec::new();
        for i in 0..number_of_firmware_targets as usize {
            let from = 12 + i * 2;
            let to = 14 + i * 2;
            if let Some(elem) = frame
                .get(from..to)
                .map(|v| ((v[0] as u16) << 8) | v[1] as u16)
            {
                firmware_n_id.push(elem);
            }
        }
        let number_of_firmware_targets_size = (number_of_firmware_targets as usize) * 2;
        let hardware_version = *frame
            .get(12 + number_of_firmware_targets_size)
            .unwrap_or(&DEFAULT_HARDWARE_VERSION);
        let activation = frame
            .get(13 + number_of_firmware_targets_size)
            .map(|v| (v & 0x02) != 0)
            .unwrap_or(DEFAULT_ACTIVATION);
        let cc = frame
            .get(13 + number_of_firmware_targets_size)
            .map(|v| (v & 0x01) != 0)
            .unwrap_or(DEFAULT_CC);

        ZwaveFirmwareMdReport::ZwaveFirmwareMdReportFrame(ZwaveFirmwareMdReportFrame {
            header: header,
            manufacturer_id: manufacturer_id,
            firmware_0_id: firmware_0_id,
            firmware_0_checksum: firmware_0_checksum,
            firmware_upgradeable: firmware_upgradable,
            number_of_firmware_targets: number_of_firmware_targets,
            max_fragment_size: max_fragment_size,
            firmware_n_id: firmware_n_id,
            hardware_version: hardware_version,
            activation: activation,
            cc: cc,
        })
    }
}

impl From<&[u8]> for ZwaveFirmwareMdRequestReport {
    fn from(frame: &[u8]) -> Self {
        let header = ZwaveCommandClassFrameHeader {
            command_class: frame[0] as u16,
            command: frame[1] as u16,
        };
        let status = frame[2];
        ZwaveFirmwareMdRequestReport::ZwaveFirmwareMdRequestReportFrame(
            ZwaveFirmwareMdRequestReportFrame {
                header: header,
                status: status,
            },
        )
    }
}
impl ToFrameBuffer for ZwaveFirmwareMdRequestGetFrame {
    fn to_frame_buffer(self, frame: &mut [u8]) -> usize {
        frame[0] = self.header.command_class as u8;
        frame[1] = self.header.command as u8;
        frame[2] = (self.manufacturer_id >> 8) as u8;
        frame[3] = (self.manufacturer_id & 0xFF) as u8;
        frame[4] = (self.firmware_id >> 8) as u8;
        frame[5] = (self.firmware_id & 0xFF) as u8;
        frame[6] = (self.firmware_checksum >> 8) as u8;
        frame[7] = (self.firmware_checksum & 0xFF) as u8;
        frame[8] = self.firmware_target;
        frame[9] = (self.fragment_size >> 8) as u8;
        frame[10] = (self.fragment_size & 0xFF) as u8;
        frame[11] = if self.activation == true { 1 } else { 0 };
        frame[12] = self.hardware_version;
        13
    }
}

fn zwave_firmware_update_send_abort_frame(
    endpoint_node: u32,
    info: &zwave_controller::zwave_controller_connection_info,
    report_number: u16,
) -> () {
    unsafe {
        log_debug(
        LOG_TAG,
        format!(
            "Sending an Firmware Update Meta Data Report with last=1 (Report number {}) to abort any transfer with {}:{}.",
            report_number,
            info.remote.__bindgen_anon_1.node_id,
            info.remote.endpoint_id
        ),
    );
    }
    // Prepare the report frame (last = 1)
    let abort_frame = ZwaveFirmwareUpdateMdReportFrame {
        header: ZwaveCommandClassFrameHeader {
            command_class: COMMAND_CLASS_FIRMWARE_UPDATE_MD,
            command: FIRMWARE_UPDATE_MD_REPORT,
        },
        last: true,
        report_number: report_number,
        data: [0; MAX_FRAME_LEN as usize],
        data_length: 0,
        checksum: 0,
    };
    let mut generated_frame: [u8; MAX_FRAME_LEN] = [0; MAX_FRAME_LEN];
    let frame_size = abort_frame.to_frame_buffer(&mut generated_frame);

    let tx_info = info.clone();
    let parent_session_id: *mut c_void = std::ptr::null_mut();
    let qos_for_frame: u32 = ZWAVE_TX_QOS_RECOMMENDED_FWU_TRANSFER_PRIORITY;

    // Prepare the Z-Wave TX options.
    let tx_options = zwave_tx::zwave_tx_options {
        number_of_responses: 0,
        qos_priority: qos_for_frame,
        discard_timeout_ms: 0,
        fasttrack: false,
        parent_session_id: std::ptr::null_mut(),
        valid_parent_session_id: false,
        use_parent_frame_options: false,
        is_test_frame: false,
        rf_power: zwave_tx::rf_power_level_t::NORMAL_POWER,
        group_id: 0,
        is_first_follow_up: false,
        send_follow_ups: false,
    };
    let session: *mut zwave_tx::zwave_tx_session_id_t = std::ptr::null_mut();
    let user: *mut ::std::os::raw::c_void = std::ptr::null_mut();

    zwave_tx::send_data(
        tx_info as zwave_tx::zwave_controller_connection_info,
        &generated_frame[0..frame_size],
        tx_options,
        as_extern_c!(on_send_fwu_md_report_callback),
        user,
        session,
    );
}

fn zwave_firmware_update_get_offset_and_fragment_size(
    firmware_transfer_node: u32,
) -> Result<(u32, u32), ()> {
    let offset_node = unsafe {
        attribute_store_get_node_child_by_type(
            firmware_transfer_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_OFFSET,
            0,
        )
    };
    if offset_node == ATTRIBUTE_STORE_INVALID_NODE {
        return Err(());
    }
    let offset: u32;
    match attribute_store_read(
        offset_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(value) => offset = value,
        Err(_) => {
            return Err(());
        }
    }

    let fragment_size_node = unsafe {
        attribute_store_get_node_child_by_type(
            firmware_transfer_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FRAGMENT_SIZE,
            0,
        )
    };
    if fragment_size_node == ATTRIBUTE_STORE_INVALID_NODE {
        return Err(());
    }
    let fragment_size: u32;
    match attribute_store_read(
        fragment_size_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(value) => fragment_size = value,
        Err(_) => {
            return Err(());
        }
    }

    return Ok((offset, fragment_size));
}

fn zwave_firmware_update_is_the_entire_image_transferred(firmware_transfer_node: u32) -> bool {
    let offset_node = unsafe {
        attribute_store_get_node_child_by_type(
            firmware_transfer_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_OFFSET,
            0,
        )
    };
    if offset_node == ATTRIBUTE_STORE_INVALID_NODE {
        return true;
    }
    let offset: u32;
    match attribute_store_read(
        offset_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(value) => offset = value,
        Err(_) => {
            return true;
        }
    }

    let transfer_size_node = unsafe {
        attribute_store_get_node_child_by_type(
            firmware_transfer_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_SIZE,
            0,
        )
    };
    if transfer_size_node == ATTRIBUTE_STORE_INVALID_NODE {
        return true;
    }
    let transfer_size: u32;
    match attribute_store_read(
        transfer_size_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(value) => transfer_size = value,
        Err(_) => {
            return true;
        }
    }

    if transfer_size > offset {
        return false;
    } else {
        return true;
    }
}

#[generate_extern_c]
fn attribute_set_md_fwu_fw_transfer(
    node: u32,
    _data: *mut u8,
    data_length: *mut u16,
) -> sl_status_t {
    // Verify if a firmware update is ongoing (Desired = IDLE, Reported = ONGOING).
    // If so, return "is_waiting" so the resolver knows we are working with the node.
    // We do not set this attribute, only use it to "block" the resolver.

    let reported_fwu_value: ZwaveFirmwareTransferState;
    match attribute_store_read(node, attribute_store_node_value_state_t::REPORTED_ATTRIBUTE) {
        Ok(value) => reported_fwu_value = value,
        Err(_) => return SL_STATUS_FAIL,
    }

    let desired_fwu_value: ZwaveFirmwareTransferState;
    match attribute_store_read(node, attribute_store_node_value_state_t::DESIRED_ATTRIBUTE) {
        Ok(value) => desired_fwu_value = value,
        Err(_) => return SL_STATUS_FAIL,
    }

    // Desired IDLE while it just reported ONGOING: We have to wait for the transfer.
    if reported_fwu_value == ZwaveFirmwareTransferState::ONGOING
        && desired_fwu_value == ZwaveFirmwareTransferState::IDLE
    {
        return SL_STATUS_IS_WAITING;
    }

    // Desired ONGOING while it just reported IDLE
    if reported_fwu_value == ZwaveFirmwareTransferState::IDLE
        && desired_fwu_value == ZwaveFirmwareTransferState::ONGOING
    {
        // The node just declined initiating OTA. Give up on our request.
        unsafe {
            attribute_store_undefine_desired(node);
            *data_length = 0;
        }
        return SL_STATUS_ALREADY_EXISTS;
    }

    // Something is unhandled if we arrive here. Undefine the desired value
    // and tell the resolver that we are good and swipe the dust under
    // the carpet
    unsafe {
        log_warning(LOG_TAG,format!("Unexpected Firmware Transfer desired value for node {}. Setting it back to undefined.",node));
        attribute_store_log_node(node, false);
        attribute_store_undefine_desired(node);
        *data_length = 0;
    }
    return SL_STATUS_ALREADY_EXISTS;
}

#[generate_extern_c]
fn attribute_get_md_fwu_fw_transfer(node: u32, data: *mut u8, length: *mut u16) -> sl_status_t {
    unsafe {
        // Default frame length in case of error:
        *length = 0;
        let desired_transfer_value: ZwaveFirmwareTransferState;
        match attribute_store_read(node, attribute_store_node_value_state_t::DESIRED_ATTRIBUTE) {
            Ok(v) => desired_transfer_value = v,
            Err(status) => {
                set_transfer_state(node, ZwaveFirmwareTransferState::IDLE);
                return SL_STATUS_ALREADY_EXISTS;
            }
        }

        match desired_transfer_value {
            ZwaveFirmwareTransferState::IDLE => {
                let offset: u32;
                let fragment_size: u32;
                match zwave_firmware_update_get_offset_and_fragment_size(node) {
                    Ok((off, frag)) => {
                        offset = off;
                        fragment_size = frag;
                    }
                    Err(_) => {
                        offset = 0;
                        fragment_size = 1;
                    }
                }

                let report_number: u16;
                if fragment_size > 0 {
                    report_number = (offset / fragment_size) as u16;
                } else {
                    report_number = 0;
                }

                let abort_frame = ZwaveFirmwareUpdateMdReportFrame {
                    header: ZwaveCommandClassFrameHeader {
                        command_class: COMMAND_CLASS_FIRMWARE_UPDATE_MD,
                        command: FIRMWARE_UPDATE_MD_REPORT,
                    },
                    last: true,
                    report_number: report_number,
                    data: [0; MAX_FRAME_LEN as usize],
                    data_length: 0,
                    checksum: 0,
                };

                let raw_buffer = std::slice::from_raw_parts_mut(data, MAX_FRAME_LEN);
                raw_buffer[0] = COMMAND_CLASS_FIRMWARE_UPDATE_MD as u8;
                raw_buffer[1] = FIRMWARE_UPDATE_MD_REPORT as u8;
                raw_buffer[2] = ((1 << 7) & ((report_number >> 8) & 0x7F)) as u8;
                raw_buffer[3] = (report_number & 0xFF) as u8;
                // No data, put a 0 checksum directly.
                raw_buffer[4] = 0;
                raw_buffer[5] = 0;
                *length = 6;
                // The supporting node won't confirm abortion.
                // We assume it worked immediately.
                set_transfer_state(node, ZwaveFirmwareTransferState::IDLE);
                return SL_STATUS_OK;
            }
            ZwaveFirmwareTransferState::ONGOING => {
                // Nothing here, we continue and request the node to start updating
            }
            _ => {
                log_warning(
                    LOG_TAG,
                    format!(
                        "Unexpected Firmware Transfer value: {}. Setting back to Idle.",
                        node
                    ),
                );
                set_transfer_state(node, ZwaveFirmwareTransferState::IDLE);
                return SL_STATUS_ALREADY_EXISTS;
            }
        }

        // Getting Manufacture ID from the interviewed data
        let fwu_node =
            attribute_store_get_first_parent_with_type(node, ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU);
        let manufacture_id_node = attribute_store_get_node_child_by_type(
            fwu_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_MANUFACTURER_ID,
            0,
        );
        let manufacture_id: i32;
        match attribute_store_read(
            manufacture_id_node,
            attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        ) {
            Ok(v) => manufacture_id = v,
            Err(status) => {
                set_transfer_state(node, ZwaveFirmwareTransferState::IDLE);
                set_firmware_transfer_last_status(node, ZwaveFirmwareTransferStatus::ABORTED);
                return SL_STATUS_ALREADY_EXISTS;
            }
        }

        // Getting Firmware ID from the interviewed data
        let firmware_target_node = attribute_store_get_node_child_by_type(
            node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_TARGET,
            0,
        );
        let firmware_target: u32;
        match attribute_store_read(
            firmware_target_node,
            attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        ) {
            Ok(v) => firmware_target = v,
            Err(status) => {
                set_transfer_state(node, ZwaveFirmwareTransferState::IDLE);
                set_firmware_transfer_last_status(node, ZwaveFirmwareTransferStatus::ABORTED);
                return SL_STATUS_ALREADY_EXISTS;
            }
        }
        let fwu_fw_node = attribute_store_get_node_child_by_type(
            fwu_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW,
            firmware_target as u32,
        );
        let firmware_id_node = attribute_store_get_node_child_by_type(
            fwu_fw_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW_ID,
            0,
        );
        let firmware_id: i32;
        match attribute_store_read(
            firmware_id_node,
            attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        ) {
            Ok(v) => firmware_id = v,
            Err(status) => {
                set_transfer_state(node, ZwaveFirmwareTransferState::IDLE);
                return SL_STATUS_ALREADY_EXISTS;
            }
        }
        // Calculating checksum
        let transfer_filename_node = attribute_store_get_node_child_by_type(
            node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_FILENAME,
            0,
        );
        let transfer_filename: String;
        match attribute_store_read(
            transfer_filename_node,
            attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        ) {
            Ok(v) => transfer_filename = v,
            Err(status) => {
                set_transfer_state(node, ZwaveFirmwareTransferState::IDLE);
                return SL_STATUS_ALREADY_EXISTS;
            }
        }
        let checksum: u16;
        match std::fs::read(transfer_filename.clone()) {
            Ok(_data) => {
                checksum = State::<AUG_CCITT>::calculate(_data.as_slice());
            }
            Err(error) => {
                set_transfer_state(node, ZwaveFirmwareTransferState::IDLE);
                log_warning(
                    LOG_TAG,
                    format!(
                        "Reading file {} failed due to {}.",
                        &transfer_filename,
                        error.to_string()
                    ),
                );
                return SL_STATUS_ALREADY_EXISTS;
            }
        }
        // Firmware target, we got it when querying firmware ID above

        // Fragment size
        let fragment_size_node = attribute_store_get_node_child_by_type(
            fwu_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FRAGMENT_SIZE,
            0,
        );
        let fragment_size: u32;
        match attribute_store_read(
            fragment_size_node,
            attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        ) {
            Ok(v) => fragment_size = v,
            Err(status) => {
                set_transfer_state(node, ZwaveFirmwareTransferState::IDLE);
                return SL_STATUS_ALREADY_EXISTS;
            }
        }

        // Activation
        let ep_node = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
        let mut activation_bit = false;
        if node_supports_activation(ep_node) {
            activation_bit = true;
        }

        // Hardware version
        let version_report_node = attribute_store_get_node_child_by_type(
            ep_node,
            ATTRIBUTE_CC_VERSION_VERSION_REPORT_DATA,
            0,
        );
        let hardware_version_node = attribute_store_get_node_child_by_type(
            version_report_node,
            ATTRIBUTE_CC_VERSION_HARDWARE_VERSION,
            0,
        );
        let hardware_version: u32 = attribute_store_read(
            hardware_version_node,
            attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        )
        .unwrap_or(1);

        let raw_buffer = std::slice::from_raw_parts_mut(data, MAX_FRAME_LEN);
        raw_buffer[0] = COMMAND_CLASS_FIRMWARE_UPDATE_MD as u8;
        raw_buffer[1] = FIRMWARE_UPDATE_MD_REQUEST_GET as u8;
        raw_buffer[2] = ((manufacture_id >> 8) & 0xFF) as u8;
        raw_buffer[3] = (manufacture_id & 0xFF) as u8;
        raw_buffer[4] = ((firmware_id >> 8) & 0xFF) as u8;
        raw_buffer[5] = (firmware_id & 0xFF) as u8;
        raw_buffer[6] = (checksum >> 8) as u8;
        raw_buffer[7] = (checksum & 0xFF) as u8;
        raw_buffer[8] = firmware_target as u8;
        raw_buffer[9] = (fragment_size >> 8) as u8;
        raw_buffer[10] = (fragment_size & 0xFF) as u8;
        if activation_bit == true {
            raw_buffer[11] = 1
        } else {
            raw_buffer[11] = 0
        };
        raw_buffer[12] = hardware_version as u8;
        *length = 13;
    }
    return SL_STATUS_OK;
}

impl From<&[u8]> for ZwaveFirmwareUpdateMdGet {
    fn from(frame: &[u8]) -> Self {
        let header = ZwaveCommandClassFrameHeader {
            command_class: frame[0] as u16,
            command: frame[1] as u16,
        };
        ZwaveFirmwareUpdateMdGet::ZwaveFirmwareUpdateMdGetFrame(ZwaveFirmwareUpdateMdGetFrame {
            header: header,
            number_of_reports: frame[2],
            report_number: ((frame[3] as u16) << 8) | frame[4] as u16,
        })
    }
}

impl ToFrameBuffer for ZwaveFirmwareUpdateMdReportFrame {
    fn to_frame_buffer(self, frame: &mut [u8]) -> usize {
        frame[0] = self.header.command_class as u8;
        frame[1] = self.header.command as u8;
        // Report number and last bit
        frame[2] = (self.report_number >> 8) as u8;
        frame[3] = (self.report_number & 0xFF) as u8;
        frame[2] = if self.last == true {
            frame[2] | 0x80
        } else {
            frame[2]
        };
        // Image data
        for idx in 0..self.data_length {
            frame[(4 + idx) as usize] = self.data[idx as usize];
        }
        frame[(4 + self.data_length) as usize] = (self.checksum >> 8) as u8;
        frame[(4 + self.data_length + 1) as usize] = (self.checksum & 0xFF) as u8;
        (self.data_length + 6).into()
    }
}

#[generate_extern_c]
fn on_send_fwu_md_report_callback(
    status: u8,
    tx_info: *const zwave_tx::zwapi_tx_report_t,
    user: *mut ::std::os::raw::c_void,
) {
}

#[generate_extern_c]
fn on_send_fwu_activation_set_callback(
    status: u8,
    tx_info: *const zwave_tx::zwapi_tx_report_t,
    user: *mut ::std::os::raw::c_void,
) {
}

fn get_fwu_transfer_file_data(
    _idx: u32,
    _fragment_size: u32,
    _filename: String,
    _data: &mut Vec<u8>,
) -> u16 {
    let mut read_bytes = 0;
    let mut transfer_file: File;
    match File::open(_filename.clone()) {
        Ok(file) => {
            transfer_file = file;
        }
        Err(error) => {
            log_warning(
                LOG_TAG,
                format!(
                    "Opening FW image file {} failed due to {}.",
                    &_filename,
                    error.to_string()
                ),
            );
            return read_bytes;
        }
    }
    // Based on the requested report number, seek to the corresponding segment
    match transfer_file.seek(SeekFrom::Start(((_idx - 1) * _fragment_size).into())) {
        Ok(v) => {}
        Err(error) => {
            log_warning(
                LOG_TAG,
                format!(
                    "Requested FWU file report number exceeds the available chunks in {}.",
                    _filename
                ),
            );
            return read_bytes;
        }
    }
    match transfer_file.read(_data.as_mut_slice()) {
        Ok(n) => {
            read_bytes = n as u16;
        }
        Err(error) => log_warning(
            LOG_TAG,
            format!(
                "Reading FW image file {} failed due to {}.",
                &_filename,
                error.to_string()
            ),
        ),
    }
    std::mem::drop(transfer_file);
    read_bytes
}

fn set_transfer_state(
    transfer_node: uic_attribute_store::attribute_store_node_t,
    state: ZwaveFirmwareTransferState,
) {
    let _ = attribute_store_write(
        transfer_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        state as u32,
    );
}

fn set_transfer_offset(transfer_node: uic_attribute_store::attribute_store_node_t, offset: u32) {
    let size_node = unsafe {
        attribute_store_get_node_child_by_type(
            transfer_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_SIZE,
            0,
        )
    };
    if size_node == ATTRIBUTE_STORE_INVALID_NODE {
        log_warning(
            LOG_TAG,
            format!(
                "Cannot locate Transfer Size in the attribute store for Transfer node {}.",
                transfer_node
            ),
        );
        return;
    }
    let size: u32;
    match attribute_store_read(
        size_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(value) => size = value,
        Err(status) => {
            log_warning(
                LOG_TAG,
                format!(
                    "Reading Transfer Sizefailed for Attribute ID {}.",
                    size_node
                ),
            );
            return;
        }
    }

    let offset_node = unsafe {
        attribute_store_get_node_child_by_type(
            transfer_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_OFFSET,
            0,
        )
    };
    if offset_node == ATTRIBUTE_STORE_INVALID_NODE {
        log_warning(
            LOG_TAG,
            format!(
                "Cannot locate Transfer Offset in the attribute store for Transfer node {}.",
                transfer_node
            ),
        );
        return;
    }

    // The offset should not he higher than the size.
    let adjusted_offset: u32;
    if size < offset {
        adjusted_offset = size;
    } else {
        adjusted_offset = offset;
    }

    let _ = attribute_store_write(
        offset_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        adjusted_offset,
    );
}

fn handle_fwu_md_get(
    info: &zwave_controller::zwave_controller_connection_info,
    frame: &[u8],
) -> sl_status_t {
    // Received get frame

    // Getting endpoint ID
    let ep_node = unsafe { zwave_command_class_get_endpoint_node(info) };
    if ep_node == ATTRIBUTE_STORE_INVALID_NODE {
        log_warning(
            LOG_TAG,
            format!("Endpoint node {} not found in attribute store.", ep_node),
        );
        return SL_STATUS_FAIL;
    }
    let ep_id: u8;
    match attribute_store_read(
        ep_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(v) => ep_id = v,
        Err(status) => {
            log_warning(
                LOG_TAG,
                format!("Reading Endpoint ID failed for Attribute ID {}.", ep_node),
            );
            return SL_STATUS_FAIL;
        }
    }

    // Getting Node ID
    let node_id_node = unsafe { attribute_store_get_node_parent(ep_node) };
    if node_id_node == ATTRIBUTE_STORE_INVALID_NODE {
        log_warning(
            LOG_TAG,
            format!(
                "NodeID node not found in the Attribute Store for Endpoint ID node {}.",
                ep_node
            ),
        );
        return SL_STATUS_FAIL;
    }
    let node_id: u16;
    match attribute_store_read(
        node_id_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(v) => node_id = v,
        Err(status) => {
            log_warning(
                LOG_TAG,
                format!(
                    "Reading NodeID value failed for Attribute {} .",
                    node_id_node
                ),
            );
            return SL_STATUS_FAIL;
        }
    }

    // Getting all related attribute nodes so that we can start querying its status
    let fwu_node: uic_attribute_store::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(ep_node, ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU, 0)
    };
    if fwu_node == ATTRIBUTE_STORE_INVALID_NODE {
        log_warning(
            LOG_TAG,
            format!("FWU node {} not found in attribute store.", fwu_node),
        );
        return SL_STATUS_FAIL;
    }
    let transfer_node: uic_attribute_store::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(
            fwu_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
            0,
        )
    };

    // Parsing MD Get frame, so we know what the node wants
    let get_frame: ZwaveFirmwareUpdateMdGetFrame;
    if let ZwaveFirmwareUpdateMdGet::ZwaveFirmwareUpdateMdGetFrame(f) =
        ZwaveFirmwareUpdateMdGet::from(frame)
    {
        get_frame = f;
    } else {
        set_transfer_state(transfer_node, ZwaveFirmwareTransferState::IDLE);
        unsafe {
            set_firmware_transfer_last_status(transfer_node, ZwaveFirmwareTransferStatus::ABORTED);
        };
        log_warning(LOG_TAG, "Received invalid FWU MD Get frame".to_string());
        zwave_firmware_update_send_abort_frame(ep_node, info, 0);
        return SL_STATUS_FAIL;
    }

    // Before continuing, verify if we should tell the node to stop because
    // we do not consider that the firmware update is ongoing
    unsafe {
        if is_firmware_update_ongoing(ep_node) == false {
            log_debug(
                LOG_TAG,
                format!(
                    "Firmware Update is not ongoing with Endpoint node {}.",
                    ep_node
                ),
            );
            zwave_firmware_update_send_abort_frame(ep_node, info, get_frame.report_number);
            return SL_STATUS_OK;
        }
    }

    // Here we accepted to give the firmware fragment(s).
    // Gather the necessary data to send.
    let fragment_size_node = unsafe {
        attribute_store_get_node_child_by_type(
            fwu_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FRAGMENT_SIZE,
            0,
        )
    };
    if fragment_size_node == ATTRIBUTE_STORE_INVALID_NODE {
        set_transfer_state(transfer_node, ZwaveFirmwareTransferState::IDLE);
        unsafe {
            set_firmware_transfer_last_status(transfer_node, ZwaveFirmwareTransferStatus::ABORTED);
        };
        log_warning(
            LOG_TAG,
            format!(
                "FWU fragment size node {} not found in attribute store.",
                fragment_size_node
            ),
        );
        return SL_STATUS_FAIL;
    }

    let fragment_size: u32;
    match attribute_store_read(
        fragment_size_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(v) => fragment_size = v,
        Err(status) => {
            log_warning(LOG_TAG, format!("Reading FWU fragment size failed."));
            zwave_firmware_update_send_abort_frame(ep_node, info, get_frame.report_number);
            return SL_STATUS_FAIL;
        }
    }

    // Getting filename
    let transfer_filename_node = unsafe {
        attribute_store_get_node_child_by_type(
            transfer_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_FILENAME,
            0,
        )
    };
    if transfer_filename_node == ATTRIBUTE_STORE_INVALID_NODE {
        set_transfer_state(transfer_node, ZwaveFirmwareTransferState::IDLE);
        unsafe {
            set_firmware_transfer_last_status(transfer_node, ZwaveFirmwareTransferStatus::ABORTED);
        };
        log_error(
            LOG_TAG,
            format!(
                "FWU transfer filename node {} not found in attribute store.",
                transfer_filename_node
            ),
        );
        zwave_firmware_update_send_abort_frame(ep_node, info, get_frame.report_number);
    }
    let transfer_filename: String;
    match attribute_store_read(
        transfer_filename_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(v) => transfer_filename = v,
        Err(status) => {
            set_transfer_state(transfer_node, ZwaveFirmwareTransferState::IDLE);
            unsafe {
                set_firmware_transfer_last_status(
                    transfer_node,
                    ZwaveFirmwareTransferStatus::ABORTED,
                );
            };
            log_warning(
                LOG_TAG,
                format!("Reading filename from attribute store failed."),
            );
            zwave_firmware_update_send_abort_frame(ep_node, info, get_frame.report_number);
            return SL_STATUS_FAIL;
        }
    }
    for idx in 0..get_frame.number_of_reports {
        let mut data = vec![0 as u8; fragment_size as usize];
        let read_bytes = get_fwu_transfer_file_data(
            (idx as u32) + (get_frame.report_number as u32),
            fragment_size,
            transfer_filename.clone(),
            &mut data,
        );

        let last: bool = zwave_firmware_update_is_the_entire_image_transferred(transfer_node);

        // Constructing the report frame
        let mut fwu_md_report_frame = ZwaveFirmwareUpdateMdReportFrame {
            header: ZwaveCommandClassFrameHeader {
                command_class: COMMAND_CLASS_FIRMWARE_UPDATE_MD,
                command: FIRMWARE_UPDATE_MD_REPORT,
            },
            last: last,
            report_number: get_frame.report_number + idx as u16,
            data: [0; MAX_FRAME_LEN as usize],
            data_length: read_bytes,
            checksum: 0,
        };
        for idx in 0..read_bytes {
            fwu_md_report_frame.data[idx as usize] = data[idx as usize];
        }
        let mut generated_frame: [u8; MAX_FRAME_LEN] = [0; MAX_FRAME_LEN];
        let mut frame_size = fwu_md_report_frame.to_frame_buffer(&mut generated_frame);
        unsafe {
            if zwave_node_get_command_class_version(
                COMMAND_CLASS_FIRMWARE_UPDATE_MD,
                node_id as u16,
                ep_id,
            ) > 1
            {
                let checksum =
                    State::<AUG_CCITT>::calculate(&generated_frame[0..(read_bytes as usize + 4)]);
                generated_frame[frame_size - 2] = (checksum >> 8) as u8;
                generated_frame[frame_size - 1] = (checksum & 0xFF) as u8;
            } else {
                frame_size -= 2;
            }
        }

        let tx_info = info.clone();
        let parent_session_id: *mut c_void = std::ptr::null_mut();
        // Queue frames so the first get sent first. Decrement by 10 each time.
        let qos_for_frame: u32;
        let qos_offset: u32 = ZWAVE_TX_RECOMMENDED_QOS_GAP * idx as u32;
        if ZWAVE_TX_QOS_RECOMMENDED_FWU_TRANSFER_PRIORITY > qos_offset {
            qos_for_frame = ZWAVE_TX_QOS_RECOMMENDED_FWU_TRANSFER_PRIORITY - qos_offset;
        } else {
            qos_for_frame = ZWAVE_TX_QOS_MIN_PRIORITY;
        }

        // Prepare the Z-Wave TX options.
        let tx_options = zwave_tx::zwave_tx_options {
            number_of_responses: 0,
            qos_priority: qos_for_frame,
            discard_timeout_ms: 0,
            fasttrack: false,
            parent_session_id: parent_session_id,
            valid_parent_session_id: false,
            use_parent_frame_options: false,
            is_test_frame: false,
            rf_power: zwave_tx::rf_power_level_t::NORMAL_POWER,
            group_id: 0,
            is_first_follow_up: false,
            send_follow_ups: false,
        };
        let session: *mut zwave_tx::zwave_tx_session_id_t = std::ptr::null_mut();
        let user: *mut ::std::os::raw::c_void = std::ptr::null_mut();

        zwave_tx::send_data(
            tx_info as zwave_tx::zwave_controller_connection_info,
            &generated_frame[0..frame_size],
            tx_options,
            as_extern_c!(on_send_fwu_md_report_callback),
            user,
            session,
        );

        // Update OFFSET node for UI to display progress bar
        let offset = ((idx as u32) + (get_frame.report_number as u32)) * fragment_size;
        set_transfer_offset(transfer_node, offset);

        unsafe { postpone_firmware_update_expiry(ep_node) };
        // Don't send anymore if this is the last
        if last == true {
            break;
        }
    }

    return SL_STATUS_OK;
}

fn handle_fwu_md_request_report(
    info: &zwave_command_classes::zwave_controller_connection_info,
    frame: &[u8],
) -> sl_status_t {
    let ep_node = unsafe { zwave_command_class_get_endpoint_node(info) };
    let fwu_node: uic_attribute_store::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(ep_node, ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU, 0)
    };
    let transfer_node: uic_attribute_store::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(
            fwu_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
            0,
        )
    };
    let last_status: ZwaveFirmwareTransferStatus;
    if let ZwaveFirmwareMdRequestReport::ZwaveFirmwareMdRequestReportFrame(f) =
        ZwaveFirmwareMdRequestReport::from(frame)
    {
        match f.status {
            0x00 => {
                log_warning(LOG_TAG, format!("Invalid combination of Manufacture ID nad Firmware ID. Firmware update request declined."));
                last_status = ZwaveFirmwareTransferStatus::ABORTED;
            }
            0x01 => {
                log_warning(LOG_TAG, format!("Device expected an authentication event to enable firmware update. Firmware update request declined."));
                last_status = ZwaveFirmwareTransferStatus::NOT_AUTHORIZED;
            }
            0x02 => {
                log_warning(LOG_TAG, format!("The requested fragment size exceeds the max fragment size. Firmware update request declined."));
                last_status = ZwaveFirmwareTransferStatus::ABORTED;
            }
            0x03 => {
                let firmware_target_node = unsafe {
                    attribute_store_get_node_child_by_type(
                        transfer_node,
                        ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_TARGET,
                        0,
                    )
                };
                let mut firmware_target: u32 = 0;
                match attribute_store_read(firmware_target_node, attribute_store_node_value_state_t::REPORTED_ATTRIBUTE) {
                    Ok(v) => firmware_target = v,
                    Err(status) => log_warning(LOG_TAG, format!("Firmware target node is not found while updating firmware update request report.")),
                }
                let fwu_fw_node = unsafe {
                    attribute_store_get_node_child_by_type(
                        fwu_node,
                        ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW,
                        firmware_target as u32,
                    )
                };
                let firmware_upgradable_node = unsafe {
                    attribute_store_get_node_child_by_type(
                        fwu_fw_node,
                        ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW_UPGRADABLE,
                        0,
                    )
                };
                attribute_store_write(
                    firmware_upgradable_node,
                    attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
                    0 as i32,
                )
                .unwrap();
                log_warning(
                    LOG_TAG,
                    format!(
                        "This firmware target is not upgradable. Firmware update request declined."
                    ),
                );
                last_status = ZwaveFirmwareTransferStatus::NOT_SUPPORTED;
            }
            0x04 => {
                log_warning(
                    LOG_TAG,
                    format!("Invalid hardware version. Firmware update request declined."),
                );
                last_status = ZwaveFirmwareTransferStatus::ABORTED;
            }
            0x05 => {
                log_warning(LOG_TAG, format!("Another firmware image is currently being transferred. Firmware update request declined."));
                last_status = ZwaveFirmwareTransferStatus::ABORTED;
            }
            0x06 => {
                log_warning(
                    LOG_TAG,
                    format!("Insufficient battery level. Firmware update request declined."),
                );
                last_status = ZwaveFirmwareTransferStatus::ABORTED;
            }
            0xFF => {
                log_info(
                    LOG_TAG,
                    format!("The node accepted to initiate the Firmware Update operation"),
                );
                attribute_store_write(
                    transfer_node,
                    attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
                    ZwaveFirmwareTransferState::ONGOING as i32,
                )
                .unwrap();
                // Set the desired to IDLE, so we don't set the node to sleep
                // while transferring an image
                attribute_store_write(
                    transfer_node,
                    attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
                    ZwaveFirmwareTransferState::IDLE as i32,
                )
                .unwrap();
                last_status = ZwaveFirmwareTransferStatus::SUCCESS;
            }
            _ => {
                last_status = ZwaveFirmwareTransferStatus::ABORTED;
            }
        }
    } else {
        log_warning(
            LOG_TAG,
            "Received invalid FWU MD Request Report frame".to_string(),
        );
        last_status = ZwaveFirmwareTransferStatus::ABORTED;
    }

    // Reset the transfer state if request report indicates error
    if last_status != ZwaveFirmwareTransferStatus::SUCCESS {
        set_transfer_state(transfer_node, ZwaveFirmwareTransferState::IDLE);
    }
    // Set the last status
    unsafe {
        set_firmware_transfer_last_status(transfer_node, last_status);
    }

    return SL_STATUS_OK;
}

impl From<&[u8]> for ZwaveFirmwareUpdateMdStatusReport {
    fn from(frame: &[u8]) -> Self {
        let header = ZwaveCommandClassFrameHeader {
            command_class: frame[0] as u16,
            command: frame[1] as u16,
        };
        let wait_time: u16;
        if frame.len() > 4 {
            wait_time = ((frame[3] as u16) << 8) | frame[4] as u16;
        } else {
            wait_time = DEFAULT_WAIT_TIME;
        }
        ZwaveFirmwareUpdateMdStatusReport::ZwaveFirmwareUpdateMdStatusReportFrame(
            ZwaveFirmwareUpdateMdStatusReportFrame {
                header: header,
                status: frame[2],
                wait_time: wait_time,
            },
        )
    }
}

impl ToFrameBuffer for ZwaveFirmwareUpdateActivationSetFrame {
    fn to_frame_buffer(self, frame: &mut [u8]) -> usize {
        frame[0] = self.header.command_class as u8;
        frame[1] = self.header.command as u8;
        // Manufacture ID
        frame[2] = (self.manufacture_id >> 8) as u8;
        frame[3] = (self.manufacture_id & 0xFF) as u8;
        // Firmware ID
        frame[4] = (self.firmware_id >> 8) as u8;
        frame[5] = (self.firmware_id & 0xFF) as u8;
        // Checksum
        frame[6] = (self.firmware_checksum >> 8) as u8;
        frame[7] = (self.firmware_checksum & 0xFF) as u8;
        frame[8] = self.firmware_target;
        frame[9] = self.hardware_version;
        10
    }
}

impl From<&[u8]> for ZwaveFirmwareUpdateActivationStatusReport {
    fn from(frame: &[u8]) -> Self {
        let header = ZwaveCommandClassFrameHeader {
            command_class: frame[0] as u16,
            command: frame[1] as u16,
        };
        ZwaveFirmwareUpdateActivationStatusReport::ZwaveFirmwareUpdateActivationStatusReportFrame(
            ZwaveFirmwareUpdateActivationStatusReportFrame {
                header: header,
                manufacture_id: ((frame[2] as u16) << 8) | frame[3] as u16,
                firmware_id: ((frame[4] as u16) << 8) | frame[5] as u16,
                firmware_checksum: ((frame[6] as u16) << 8) | frame[7] as u16,
                firmware_target: frame[8],
                firmware_update_status: frame[9],
                hardware_version: if frame.len() > 10 { frame[10] } else { 1 },
            },
        )
    }
}

fn handle_fwu_activation_status_report(
    info: &zwave_controller_connection_info,
    frame: &[u8],
) -> sl_status_t {
    let ep_node = unsafe {
        zwave_command_class_get_endpoint_node(info as *const zwave_controller_connection_info)
    };
    let fwu_node: uic_attribute_store::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(ep_node, ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU, 0)
    };
    let transfer_node: uic_attribute_store::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(
            fwu_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
            0,
        )
    };
    let last_status: ZwaveFirmwareTransferStatus;
    if let ZwaveFirmwareUpdateActivationStatusReport::ZwaveFirmwareUpdateActivationStatusReportFrame(f) =
    ZwaveFirmwareUpdateActivationStatusReport::from(frame) {
        match f.firmware_update_status {
            0x0 => {
                log_warning(LOG_TAG, format!("Invalid combination of manufacturer ID, firmware ID and hardware version or firmware target. The received image will not be stored."));
                last_status = ZwaveFirmwareTransferStatus::ABORTED;
            }
            0x1 => {
                log_warning(LOG_TAG, format!("Error activating the firmware. Last known firmware image has been restored."));
                last_status = ZwaveFirmwareTransferStatus::INVALID_IMAGE;
            }
            0xff => {
                log_info(LOG_TAG, format!("Firmware update completed successfully."));
                last_status = ZwaveFirmwareTransferStatus::SUCCESS;
            }
            _ => {
              log_debug(LOG_TAG,format!("Unknown status for Firmware Update Activation Report: {}. Assuming that the activation failed.",f.firmware_update_status));
              last_status = ZwaveFirmwareTransferStatus::ABORTED;
            }
        }
    } else {
      last_status = ZwaveFirmwareTransferStatus::ABORTED;
    }

    // Reset the transfer state if the report indicates any error
    if last_status != ZwaveFirmwareTransferStatus::SUCCESS {
        set_transfer_state(transfer_node, ZwaveFirmwareTransferState::IDLE);
        set_transfer_offset(transfer_node, 0);
    }
    // Set the last status
    unsafe {
        set_firmware_transfer_last_status(transfer_node, last_status);
    }

    return SL_STATUS_OK;
}

fn on_activation_timer_callback(tx_info: zwave_tx::zwave_controller_connection_info) {
    let ep_node = unsafe { zwave_command_class_get_endpoint_node(&tx_info) };
    let fwu_node: uic_attribute_store::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(ep_node, ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU, 0)
    };
    let transfer_node: uic_attribute_store::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(
            fwu_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
            0,
        )
    };
    let manufacture_id_node: uic_attribute_store::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(
            fwu_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_MANUFACTURER_ID,
            0,
        )
    };
    let manufacture_id: i32;
    match attribute_store_read(
        manufacture_id_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(v) => manufacture_id = v,
        Err(status) => return,
    }

    // Getting Firmware ID from the interviewed data
    let firmware_target_node: uic_attribute_store::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(
            transfer_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_TARGET,
            0,
        )
    };
    let firmware_target: u32;
    match attribute_store_read(
        firmware_target_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(v) => firmware_target = v,
        Err(status) => return,
    }
    let fwu_fw_node: uic_attribute_store::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(
            fwu_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW,
            firmware_target as u32,
        )
    };
    let firmware_id_node: uic_attribute_store::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(
            fwu_fw_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW_ID,
            0,
        )
    };
    let firmware_id: i32;
    match attribute_store_read(
        firmware_id_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(v) => firmware_id = v,
        Err(status) => return,
    }
    // Calculating checksum
    let transfer_filename_node: uic_attribute_store::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(
            transfer_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_FILENAME,
            0,
        )
    };
    let transfer_filename: String;
    match attribute_store_read(
        transfer_filename_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(v) => transfer_filename = v,
        Err(status) => return,
    }
    let mut checksum = 0;
    match std::fs::read(transfer_filename.clone()) {
        Ok(_data) => {
            checksum = State::<AUG_CCITT>::calculate(_data.as_slice());
        }
        Err(error) => log_warning(
            LOG_TAG,
            format!(
                "Reading file {} failed due to {}.",
                &transfer_filename,
                error.to_string()
            ),
        ),
    }
    let version_report_node = unsafe {
        attribute_store_get_node_child_by_type(ep_node, ATTRIBUTE_CC_VERSION_VERSION_REPORT_DATA, 0)
    };
    let hardware_version_node = unsafe {
        attribute_store_get_node_child_by_type(
            version_report_node,
            ATTRIBUTE_CC_VERSION_HARDWARE_VERSION,
            0,
        )
    };
    let hardware_version: u32 = attribute_store_read(
        hardware_version_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    )
    .unwrap_or(1);

    let parent_session_id: *mut c_void = std::ptr::null_mut();
    // Prepare the Z-Wave TX options.
    let tx_options = zwave_tx::zwave_tx_options {
        number_of_responses: 0,
        qos_priority: ZWAVE_TX_QOS_RECOMMENDED_FWU_TRANSFER_PRIORITY,
        discard_timeout_ms: 0,
        fasttrack: false,
        parent_session_id: parent_session_id,
        valid_parent_session_id: false,
        use_parent_frame_options: false,
        is_test_frame: false,
        rf_power: zwave_tx::rf_power_level_t::NORMAL_POWER,
        group_id: 0,
        is_first_follow_up: false,
        send_follow_ups: false,
    };
    let session: *mut zwave_tx::zwave_tx_session_id_t = std::ptr::null_mut();
    let user: *mut ::std::os::raw::c_void = std::ptr::null_mut();
    let mut generated_frame: [u8; MAX_FRAME_LEN] = [0; MAX_FRAME_LEN];
    let fwu_activation_set_frame = ZwaveFirmwareUpdateActivationSetFrame {
        header: ZwaveCommandClassFrameHeader {
            command_class: COMMAND_CLASS_FIRMWARE_UPDATE_MD,
            command: FIRMWARE_UPDATE_ACTIVATION_SET,
        },
        manufacture_id: manufacture_id as u16,
        firmware_id: firmware_id as u16,
        firmware_checksum: checksum as u16,
        firmware_target: firmware_target as u8,
        hardware_version: hardware_version as u8,
    };
    let frame_size = fwu_activation_set_frame.to_frame_buffer(&mut generated_frame);
    zwave_tx::send_data(
        tx_info,
        &generated_frame[0..frame_size],
        tx_options,
        as_extern_c!(on_send_fwu_activation_set_callback),
        user,
        session,
    );
}

fn on_reboot_complete(transfer_node: uic_attribute_store::attribute_store_node_t) {
    set_transfer_state(transfer_node, ZwaveFirmwareTransferState::IDLE);
}

fn handle_fwu_md_status_report(
    info: &zwave_controller_connection_info,
    frame: &[u8],
) -> sl_status_t {
    let ep_node = unsafe {
        zwave_command_class_get_endpoint_node(info as *const zwave_controller_connection_info)
    };
    let fwu_node: uic_attribute_store::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(ep_node, ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU, 0)
    };
    let transfer_node: uic_attribute_store::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(
            fwu_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
            0,
        )
    };
    let last_status;
    let current_transfer_state: ZwaveFirmwareTransferState;
    if let ZwaveFirmwareUpdateMdStatusReport::ZwaveFirmwareUpdateMdStatusReportFrame(f) =
        ZwaveFirmwareUpdateMdStatusReport::from(frame)
    {
        match f.status {
            0xff => {
                current_transfer_state = ZwaveFirmwareTransferState::WAITING_FOR_REBOOT;
                last_status = ZwaveFirmwareTransferStatus::SUCCESS;
                log_info(LOG_TAG, format!("New firmware was successfully stored in temporary non volatile memory. The device may have restarted itself."));
                call_once(
                    std::time::Duration::from_secs(f.wait_time as u64),
                    &on_reboot_complete,
                    transfer_node.clone(),
                );
            }
            0xfe => {
                current_transfer_state = ZwaveFirmwareTransferState::IDLE;
                last_status = ZwaveFirmwareTransferStatus::SUCCESS;

                log_info(
                    LOG_TAG,
                    format!(
                        "New firmware was successfully stored in temporary non volatile memory."
                    ),
                );
            }
            0xfd => {
                current_transfer_state = ZwaveFirmwareTransferState::WAITING_FOR_ACTIVATION;
                last_status = ZwaveFirmwareTransferStatus::SUCCESS;

                // Find Apply Timestamp
                let apply_timestamp_node: uic_attribute_store::attribute_store_node_t = unsafe {
                    attribute_store_get_node_child_by_type(
                        transfer_node,
                        ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_APPLY_TIMESTAMP,
                        0,
                    )
                };
                let apply_timestamp_as_seconds: u64;
                match attribute_store_read(
                    apply_timestamp_node,
                    attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
                ) {
                    Ok(v) => apply_timestamp_as_seconds = v,
                    Err(status) => return SL_STATUS_FAIL,
                }
                let mut current_time_as_seconds = 0;
                match SystemTime::now().duration_since(SystemTime::UNIX_EPOCH) {
                    Ok(n) => current_time_as_seconds = n.as_secs(),
                    Err(_) => log_warning(LOG_TAG, format!("SystemTime before UNIX EPOCH!")),
                }
                // Safe guard for duration calculation
                let duration = if apply_timestamp_as_seconds > current_time_as_seconds {
                    apply_timestamp_as_seconds - current_time_as_seconds
                } else {
                    0
                };
                log_info(
                    LOG_TAG,
                    format!(
                        "Firmware image downloaded successfully, waiting for activation command sent in {} seconds.", duration
                    ),
                );
                call_once(
                    std::time::Duration::from_secs(duration),
                    &on_activation_timer_callback,
                    info.clone(),
                );
            }
            0x0 => {
                current_transfer_state = ZwaveFirmwareTransferState::IDLE;
                last_status = ZwaveFirmwareTransferStatus::INVALID_IMAGE;
                log_warning(
                    LOG_TAG,
                    format!("The device was unable to receive the requested firmware data without checksum error."),
                );
            }
            0x1 => {
                current_transfer_state = ZwaveFirmwareTransferState::IDLE;
                last_status = ZwaveFirmwareTransferStatus::ABORTED;
                log_warning(
                    LOG_TAG,
                    format!("The device was unable to receive the Firmware image.."),
                );
            }
            0x2 => {
                current_transfer_state = ZwaveFirmwareTransferState::IDLE;
                last_status = ZwaveFirmwareTransferStatus::INVALID_IMAGE;
                log_warning(
                    LOG_TAG,
                    format!("Supporting node reports invalid Manufacturer ID for the transfered image. Firmware Update failed."),
                );
            }
            0x3 => {
                current_transfer_state = ZwaveFirmwareTransferState::IDLE;
                last_status = ZwaveFirmwareTransferStatus::INVALID_IMAGE;
                log_warning(
                  LOG_TAG,
                  format!("Supporting node reports invalid Firmware ID for the transfered image. Firmware Update failed."),
              );
            }
            0x4 => {
                current_transfer_state = ZwaveFirmwareTransferState::IDLE;
                last_status = ZwaveFirmwareTransferStatus::INVALID_IMAGE;
                log_warning(
                LOG_TAG,
                format!("Supporting node reports invalid Firmware Target for the transfered image. Firmware Update failed."),
              );
            }
            0x5 => {
                current_transfer_state = ZwaveFirmwareTransferState::IDLE;
                last_status = ZwaveFirmwareTransferStatus::INVALID_IMAGE;
                log_warning(
              LOG_TAG,
              format!("Supporting node reports invalid header information for the transfered image. Firmware Update failed."),
                );
            }
            0x6 => {
                current_transfer_state = ZwaveFirmwareTransferState::IDLE;
                last_status = ZwaveFirmwareTransferStatus::INVALID_IMAGE;
                log_warning(
            LOG_TAG,
            format!("Supporting node reports invalid header format for the transfered image. Firmware Update failed."),
                );
            }
            0x7 => {
                current_transfer_state = ZwaveFirmwareTransferState::IDLE;
                last_status = ZwaveFirmwareTransferStatus::ABORTED;
                log_warning(
          LOG_TAG,
          format!("Supporting node reports insufficient memory for the transfered image. Firmware Update failed."),
                );
            }
            0x8 => {
                current_transfer_state = ZwaveFirmwareTransferState::IDLE;
                last_status = ZwaveFirmwareTransferStatus::INVALID_IMAGE;
                log_warning(
        LOG_TAG,
        format!("Supporting node reports non-matching hardware version for the transfered image. Firmware Update failed."),
              );
            }
            _ => {
                current_transfer_state = ZwaveFirmwareTransferState::IDLE;
                last_status = ZwaveFirmwareTransferStatus::ABORTED;
                log_warning(
                    LOG_TAG,
                    format!(
                        "The firmware udpate process was failed. Status: {}",
                        f.status
                    ),
                );
            }
        }
    } else {
        log_warning(
            LOG_TAG,
            "Received invalid FWU MD Status Report frame".to_string(),
        );
        current_transfer_state = ZwaveFirmwareTransferState::IDLE;
        last_status = ZwaveFirmwareTransferStatus::ABORTED;
    }
    // Update the transfer state and last status
    set_transfer_state(transfer_node, current_transfer_state);
    unsafe {
        set_firmware_transfer_last_status(transfer_node, last_status);
    }
    return SL_STATUS_OK;
}

const CONFIG: ZwaveHandlerConfig = ZwaveHandlerConfig {
    minimal_scheme:
    zwave_command_handler::zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
    command_class: COMMAND_CLASS_FIRMWARE_UPDATE_MD,
    command_class_name: "Firmware Update\0",
    comments: "\0",
    version: VERSION,
    manual_security_validation: false,
};

fn handle_fw_md_report(
    info: &zwave_command_classes::zwave_controller_connection_info,
    frame: &[u8],
) -> sl_status_t {
    let ep_node = unsafe {
        zwave_command_class_get_endpoint_node(
            info as *const zwave_command_classes::zwave_controller_connection_info,
        )
    };
    let fwu_node: uic_attribute_store::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(ep_node, ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU, 0)
    };
    if let ZwaveFirmwareMdReport::ZwaveFirmwareMdReportFrame(f) = ZwaveFirmwareMdReport::from(frame)
    {
        attribute_store_write(
            fwu_node,
            attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
            0 as i32,
        )
        .unwrap();
        let mut fw = vec![f.firmware_0_id];
        fw.append(f.firmware_n_id.clone().as_mut());

        // firmware_0 +  firmware_n
        let mut fwu_idx: i32 = 0;
        for fwu_id in fw {
            let fw_idx_node = attribute_store::get_or_add_node(
                ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW,
                fwu_node,
                Some(fwu_idx),
                Some(fwu_idx),
            );
            attribute_store::get_or_add_node(
                ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW_ID,
                fw_idx_node,
                None,
                Some(fwu_id as i32),
            );
            attribute_store::get_or_add_node(
                ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW_UPGRADABLE,
                fw_idx_node,
                None,
                Some(f.firmware_upgradeable as i32),
            );

            fwu_idx += 1;
        }
        attribute_store::get_or_add_node(
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_MANUFACTURER_ID,
            fwu_node,
            None,
            Some(f.manufacturer_id as i32),
        );

        attribute_store::get_or_add_node(
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_ACTIVATION,
            fwu_node,
            None,
            Some(f.activation as i32),
        );

        attribute_store::get_or_add_node(
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_CC,
            fwu_node,
            None,
            Some(f.cc as i32),
        );

        let max_fragment_size: u16;
        if f.max_fragment_size == (MAX_FRAME_LEN as u16) {
            unsafe {
                max_fragment_size = get_theoretical_max_fragment_size(ep_node);
            }
        } else {
            max_fragment_size = f.max_fragment_size;
        }
        attribute_store::get_or_add_node(
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FRAGMENT_SIZE,
            fwu_node,
            None,
            Some(max_fragment_size as i32),
        );
    } else {
        log_warning(LOG_TAG, "Received invalid FWU MD Report frame".to_string());
        // Set REPORTED_ATTRIBUTE to 1 to indicate error and stop resolver
        attribute_store_write(
            fwu_node,
            attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
            1 as i32,
        )
        .unwrap();
    }
    return SL_STATUS_OK;
}

#[generate_extern_c]
fn attribute_set_md_fwu(_node: u32, _data: *mut u8, _data_len: *mut u16) -> sl_status_t {
    // FIXME: Remove this function
    return SL_STATUS_FAIL;
}

#[generate_extern_c]
fn attribute_get_md_fwu(_node: u32, data: *mut u8, data_len: *mut u16) -> sl_status_t {
    unsafe {
        let raw_buffer = std::slice::from_raw_parts_mut(data, MAX_FRAME_LEN);
        raw_buffer[0] = COMMAND_CLASS_FIRMWARE_UPDATE_MD as u8;
        raw_buffer[1] = FIRMWARE_MD_GET as u8;

        *data_len = 2;
    }
    return SL_STATUS_OK;
}

#[generate_extern_c]
fn on_attribute_version_change(node: u32, change: attribute_store_change_t) {
    if change == attribute_store_change_t::ATTRIBUTE_UPDATED {
        if unsafe {
            attribute_store_is_value_defined(
                node,
                attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
            )
        } == true
        {
            let ep_node =
                unsafe { attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID) };
            if unsafe {
                attribute_store_get_node_child_by_type(
                    ep_node,
                    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU,
                    0,
                )
            } == ATTRIBUTE_STORE_INVALID_NODE
            {
                if attribute_store::add_node(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU, ep_node).is_err() {
                    return;
                }
            }
        }
    }
}

#[zwave_command_class(CONFIG)]
#[derive(Default)]
pub struct ZwaveCommandClassFirmwareUpdate {}

impl ZwaveCommandClassFirmwareUpdate {
    pub fn on_control(
        &self,
        info: zwave_controller::zwave_controller_connection_info,
        frame: &[u8],
    ) -> sl_status_t {
        let header = ZwaveCommandClassFrameHeader::from(frame);
        if header.command_class != COMMAND_CLASS_FIRMWARE_UPDATE_MD {
            return SL_STATUS_NOT_SUPPORTED;
        }

        match header.command {
            FIRMWARE_MD_REPORT => {
                return handle_fw_md_report(&info, frame);
            }
            FIRMWARE_UPDATE_MD_REQUEST_REPORT => {
                return handle_fwu_md_request_report(&info, frame);
            }
            FIRMWARE_UPDATE_MD_GET => {
                return handle_fwu_md_get(&info, frame);
            }
            FIRMWARE_UPDATE_MD_STATUS_REPORT => {
                return handle_fwu_md_status_report(&info, frame);
            }
            FIRMWARE_UPDATE_ACTIVATION_STATUS_REPORT => {
                return handle_fwu_activation_status_report(&info, frame);
            }
            _ => {
                return SL_STATUS_NOT_SUPPORTED;
            }
        }
    }

    pub fn on_support(
        &self,
        info: zwave_command_classes::zwave_controller_connection_info,
        frame: &[u8],
    ) -> sl_status_t {
        return 0xF; // SL_STATUS_NOT_SUPPORTED
    }
    pub fn on_init(&self, status: sl_status_t) -> sl_status_t {
        attribute_resolver_register_rule(
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
            as_extern_c!(attribute_set_md_fwu_fw_transfer),
            as_extern_c!(attribute_get_md_fwu_fw_transfer),
        );
        attribute_resolver_register_rule(
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU,
            as_extern_c!(attribute_set_md_fwu), // TODO: use NULL (make it work with nullptr)
            as_extern_c!(attribute_get_md_fwu),
        );
        unsafe {
            attribute_store_register_callback_by_type(
                as_extern_c!(on_attribute_version_change),
                ATTRIBUTE_COMMAND_CLASS_FWU_MD_VERSION,
            );
        };
        status
    }
}
