use crate::{ZwaveHandlerConfig, MAX_FRAME_LEN};
use std::{
    convert::TryInto,
    ffi::c_void,
    fs::File,
    io::{Read, Seek, SeekFrom},
    time::SystemTime,
    usize,
};

mod firmware_update_definitions;
#[cfg(test)]
mod firmware_update_tests;
use firmware_update_definitions::*;

use crate::zwave_command_class_send_report;
use ::zwave_tx_sys::*;
use crc16::*;
use unify_attribute_store_sys::*;
use unify_log_sys::*;
use unify_proc_macro::{as_extern_c, generate_extern_c};
use unify_sl_status_sys::*;
use zwave_command_classes_sys::zwave_command_class_get_endpoint_node;
use zwave_controller_sys::{sl_status_t, zwave_node_get_command_class_version};
use zwave_rust_proc_macros::zwave_command_class;
declare_app_name!("zwave_cc_fwu");

fn zwave_firmware_update_send_abort_frame(
    endpoint_node: u32,
    info: &zwave_controller_sys::zwave_controller_connection_info,
    report_number: u16,
) {
    unsafe {
        log_debug!("Sending an Firmware Update Meta Data Report with last=1 (Report number {}) to abort any transfer with {}:{}.",
              report_number,
              info.remote.__bindgen_anon_1.node_id,
              info.remote.endpoint_id
        );
    }
    zwave_command_class_send_report(info, ZwaveFirmwareUpdateMdReportType::abort(report_number));
}

fn zwave_firmware_update_initiate_interview(transfer_node: attribute_store_node_t) -> sl_status_t {
    unsafe {
        let node_id_node =
            attribute_store_get_first_parent_with_type(transfer_node, ATTRIBUTE_NODE_ID);
        let mut current_endpoint =
            attribute_store_get_node_child_by_type(node_id_node, ATTRIBUTE_ENDPOINT_ID, 0);
        while current_endpoint != ATTRIBUTE_STORE_INVALID_NODE {
            // No harm in calling delete on an invalid node.
            if SL_STATUS_OK != attribute_store_delete_node(current_endpoint) {
                return SL_STATUS_FAIL;
            }
            // Keep the child index to 0 here, since we delete them
            current_endpoint =
                attribute_store_get_node_child_by_type(node_id_node, ATTRIBUTE_ENDPOINT_ID, 0);
        }

        // Now that we wiped the endpoints, create endpoint 0 again with an empty NIF.
        let new_ep_id_node = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, node_id_node);
        attribute_store::write_attribute(
            new_ep_id_node,
            REPORTED_ATTRIBUTE,
            0 as zwave_endpoint_id_t,
        )
        .unwrap();
        attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, new_ep_id_node);
    }
    SL_STATUS_OK
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
    match attribute_store::read_attribute(
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
    match attribute_store::read_attribute(
        fragment_size_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(value) => fragment_size = value,
        Err(_) => {
            return Err(());
        }
    }

    Ok((offset, fragment_size))
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
    match attribute_store::read_attribute(
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
    match attribute_store::read_attribute(
        transfer_size_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(value) => transfer_size = value,
        Err(_) => {
            return true;
        }
    }

    transfer_size <= offset
}

#[generate_extern_c]
unsafe fn attribute_set_md_fwu_fw_transfer(
    node: u32,
    _data: *mut u8,
    data_length: *mut u16,
) -> sl_status_t {
    // Verify if a firmware update is ongoing (Desired = IDLE, Reported = ONGOING).
    // If so, return "is_waiting" so the resolver knows we are working with the node.
    //
    // If firmware update is WAITINT_FOR_ACTIVATION, we tell the resovler that "is_waiting"
    // as the attribute listener will take care of it and undefine the reported value, which
    // in turn triggers a Get resolution to send Activation Set and in this way activation can
    // survive a ZPC reboot, even with a sleeping nodes.

    let reported_fwu_value: ZwaveFirmwareTransferState;
    match attribute_store::read_attribute(
        node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(value) => reported_fwu_value = value,
        Err(_) => return SL_STATUS_FAIL,
    }

    let desired_fwu_value: ZwaveFirmwareTransferState;
    match attribute_store::read_attribute(
        node,
        attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
    ) {
        Ok(value) => desired_fwu_value = value,
        Err(_) => return SL_STATUS_FAIL,
    }

    if desired_fwu_value == ZwaveFirmwareTransferState::IDLE {
        match reported_fwu_value {
            ZwaveFirmwareTransferState::IDLE => return SL_STATUS_ALREADY_EXISTS,
            ZwaveFirmwareTransferState::ONGOING => return SL_STATUS_IS_WAITING,
            ZwaveFirmwareTransferState::WAITING_FOR_ACTIVATION => {
                return SL_STATUS_IS_WAITING;
            }
            ZwaveFirmwareTransferState::WAITING_FOR_REBOOT => {
                // attribute listener is handling the callback so reboot timer can be called even for sleeping nodes
                return SL_STATUS_IS_WAITING;
            }
        }
    }

    // Desired ONGOING while it just reported IDLE
    if reported_fwu_value == ZwaveFirmwareTransferState::IDLE
        && desired_fwu_value == ZwaveFirmwareTransferState::ONGOING
    {
        // The node just declined initiating OTA. Give up on our request.
        attribute_store_undefine_desired(node);
        *data_length = 0;
        return SL_STATUS_ALREADY_EXISTS;
    }

    // Something is unhandled if we arrive here. Undefine the desired value
    // and tell the resolver that we are good and swipe the dust under
    // the carpet
    log_warning!(
        "Unexpected Firmware Transfer desired value for node {}. Setting it back to undefined.",
        node
    );
    attribute_store_log_node(node, false);
    attribute_store_undefine_desired(node);
    *data_length = 0;
    SL_STATUS_ALREADY_EXISTS
}

#[generate_extern_c]
unsafe fn attribute_get_md_fwu_fw_transfer(
    node: u32,
    data: *mut u8,
    length: *mut u16,
) -> sl_status_t {
    // Default frame length in case of error:
    *length = 0;
    let desired_transfer_value: ZwaveFirmwareTransferState;
    match attribute_store::read_attribute(
        node,
        attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
    ) {
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

            let abort: Vec<u8> = ZwaveFirmwareUpdateMdReportType::abort(report_number).into();
            let raw_buffer = std::slice::from_raw_parts_mut(data, abort.len());
            raw_buffer.copy_from_slice(&abort);
            *length = abort.len() as u16;

            // The supporting node won't confirm abortion.
            // We assume it worked immediately.
            set_transfer_state(node, ZwaveFirmwareTransferState::IDLE);
            return SL_STATUS_OK;
        }
        ZwaveFirmwareTransferState::WAITING_FOR_ACTIVATION => {
            let fwu_node = attribute_store_get_first_parent_with_type(
                node,
                ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU,
            );
            let manufacturer_id_node: unify_attribute_store_sys::attribute_store_node_t =
                attribute_store_get_node_child_by_type(
                    fwu_node,
                    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_MANUFACTURER_ID,
                    0,
                );
            let manufacturer_id: i32;
            match attribute_store::read_attribute(
                manufacturer_id_node,
                attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
            ) {
                Ok(v) => manufacturer_id = v,
                Err(status) => return SL_STATUS_ALREADY_EXISTS,
            }

            // Getting Firmware ID from the interviewed data
            let firmware_target_node: unify_attribute_store_sys::attribute_store_node_t =
                attribute_store_get_node_child_by_type(
                    node,
                    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_TARGET,
                    0,
                );
            let firmware_target: u32;
            match attribute_store::read_attribute(
                firmware_target_node,
                attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
            ) {
                Ok(v) => firmware_target = v,
                Err(status) => return SL_STATUS_ALREADY_EXISTS,
            }
            let fwu_fw_node: unify_attribute_store_sys::attribute_store_node_t =
                attribute_store_get_node_child_by_type(
                    fwu_node,
                    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW,
                    firmware_target as u32,
                );
            let firmware_id_node: unify_attribute_store_sys::attribute_store_node_t =
                attribute_store_get_node_child_by_type(
                    fwu_fw_node,
                    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW_ID,
                    0,
                );
            let firmware_id: i32;
            match attribute_store::read_attribute(
                firmware_id_node,
                attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
            ) {
                Ok(v) => firmware_id = v,
                Err(status) => return SL_STATUS_ALREADY_EXISTS,
            }
            // Calculating checksum
            let transfer_filename_node: unify_attribute_store_sys::attribute_store_node_t =
                attribute_store_get_node_child_by_type(
                    node,
                    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_FILENAME,
                    0,
                );
            let transfer_filename: String;
            match attribute_store::read_attribute(
                transfer_filename_node,
                attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
            ) {
                Ok(v) => transfer_filename = v,
                Err(status) => return SL_STATUS_ALREADY_EXISTS,
            }
            let mut checksum = 0;
            match std::fs::read(transfer_filename.clone()) {
                Ok(_data) => {
                    checksum = State::<AUG_CCITT>::calculate(_data.as_slice());
                }
                Err(error) => {
                    log_warning!(
                        "Reading file {} failed due to {}.",
                        &transfer_filename,
                        error
                    );
                }
            }
            let ep_node = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
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
            let hardware_version: u32 = attribute_store::read_attribute(
                hardware_version_node,
                attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
            )
            .unwrap_or(DEFAULT_HARDWARE_VERSION.into());

            let new_buffer: Vec<u8> = ZwaveFirmwareUpdateActivationSetType {
                manufacturer_id: manufacturer_id.try_into().unwrap(),
                firmware_id: firmware_id.try_into().unwrap(),
                firmware_checksum: checksum.try_into().unwrap(),
                firmware_target: firmware_target.try_into().unwrap(),
                hardware_version: hardware_version.try_into().unwrap(),
            }
            .into();

            let raw_buffer = std::slice::from_raw_parts_mut(data, new_buffer.len());
            raw_buffer.copy_from_slice(&new_buffer);
            *length = new_buffer.len() as u16;
            return SL_STATUS_OK;
        }
        ZwaveFirmwareTransferState::ONGOING => {
            // Nothing here, we continue and request the node to start updating
        }
        _ => {
            log_warning!(
                "Unexpected Firmware Transfer value: {}. Setting back to Idle.",
                node
            );
            set_transfer_state(node, ZwaveFirmwareTransferState::IDLE);
            return SL_STATUS_ALREADY_EXISTS;
        }
    }

    // Getting Manufacture ID from the interviewed data
    let fwu_node =
        attribute_store_get_first_parent_with_type(node, ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU);
    let manufacturer_id_node = attribute_store_get_node_child_by_type(
        fwu_node,
        ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_MANUFACTURER_ID,
        0,
    );
    let manufacturer_id: u32;
    match attribute_store::read_attribute(
        manufacturer_id_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(v) => manufacturer_id = v,
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
    match attribute_store::read_attribute(
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
    let firmware_id: u32;
    match attribute_store::read_attribute(
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
    match attribute_store::read_attribute(
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
            log_warning!(
                "Reading file {} failed due to {}.",
                &transfer_filename,
                error.to_string()
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
    match attribute_store::read_attribute(
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
    if node_supports_firmware_activation(ep_node) {
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
    let hardware_version: u32 = attribute_store::read_attribute(
        hardware_version_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    )
    .unwrap_or(1);

    let new_buffer: Vec<u8> = ZwaveFirmwareUpdateMdRequestGetType {
        manufacturer_id: manufacturer_id.try_into().unwrap(),
        firmware_id: firmware_id.try_into().unwrap(),
        firmware_checksum: checksum,
        firmware_target: firmware_target.try_into().unwrap(),
        fragment_size: fragment_size.try_into().unwrap(),
        activation: activation_bit,
        hardware_version: hardware_version.try_into().unwrap(),
    }
    .into();

    let raw_buffer = std::slice::from_raw_parts_mut(data, new_buffer.len());
    raw_buffer.copy_from_slice(&new_buffer);
    *length = new_buffer.len() as u16;
    SL_STATUS_OK
}

fn get_fwu_transfer_file_data(idx: u32, fragment_size: u32, filename: String) -> Vec<u8> {
    let mut read_bytes = 0;
    let mut data = vec![0; fragment_size as usize];
    let mut transfer_file: File;
    match File::open(filename.clone()) {
        Ok(file) => {
            transfer_file = file;
        }
        Err(error) => {
            log_warning!(
                "Opening FW image file {} failed due to {}.",
                &filename,
                error
            );
            return data[0..read_bytes].to_vec();
        }
    }
    // Based on the requested report number, seek to the corresponding segment
    match transfer_file.seek(SeekFrom::Start(((idx - 1) * fragment_size).into())) {
        Ok(v) => {}
        Err(error) => {
            log_warning!(
                "Requested FWU file report number exceeds the available chunks in {}.",
                filename
            );
            return data[0..read_bytes].to_vec();
        }
    }
    match transfer_file.read(data.as_mut_slice()) {
        Ok(n) => {
            read_bytes = n;
        }
        Err(error) => log_warning!(
            "Reading FW image file {} failed due to {}.",
            &filename,
            error
        ),
    }
    std::mem::drop(transfer_file);
    data[0..read_bytes].to_vec()
}

fn set_transfer_state(
    transfer_node: unify_attribute_store_sys::attribute_store_node_t,
    state: ZwaveFirmwareTransferState,
) {
    let _ = attribute_store::write_attribute(
        transfer_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        state as u32,
    );
}

fn set_transfer_offset(
    transfer_node: unify_attribute_store_sys::attribute_store_node_t,
    offset: u32,
) {
    let size_node = unsafe {
        attribute_store_get_node_child_by_type(
            transfer_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_SIZE,
            0,
        )
    };
    if size_node == ATTRIBUTE_STORE_INVALID_NODE {
        log_warning!(
            "Cannot locate Transfer Size in the attribute store for Transfer node {}.",
            transfer_node
        );
        return;
    }
    let size: u32;
    match attribute_store::read_attribute(
        size_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(value) => size = value,
        Err(status) => {
            log_warning!(
                "Reading Transfer Sizefailed for Attribute ID {}.",
                size_node
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
        log_warning!(
            "Cannot locate Transfer Offset in the attribute store for Transfer node {}.",
            transfer_node
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

    let _ = attribute_store::write_attribute(
        offset_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        adjusted_offset,
    );
}

fn handle_fwu_md_get(
    fwu_node_info: FirmwareNodeInfo,
    get_frame: ZwaveFirmwareUpdateMdGetType,
) -> sl_status_t {
    let ep_id: u8;
    match attribute_store::read_attribute(
        fwu_node_info.ep_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(v) => ep_id = v,
        Err(status) => {
            log_warning!(
                "Reading Endpoint ID failed for Attribute ID {}.",
                fwu_node_info.ep_node
            );
            return SL_STATUS_FAIL;
        }
    }

    // Getting Node ID
    let node_id_node = unsafe { attribute_store_get_node_parent(fwu_node_info.ep_node) };
    if node_id_node == ATTRIBUTE_STORE_INVALID_NODE {
        log_warning!(
            "NodeID node not found in the Attribute Store for Endpoint ID node {}.",
            fwu_node_info.ep_node
        );
        return SL_STATUS_FAIL;
    }
    let node_id: u16;
    match attribute_store::read_attribute(
        node_id_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(v) => node_id = v,
        Err(status) => {
            log_warning!(
                "Reading NodeID value failed for Attribute {} .",
                node_id_node
            );
            return SL_STATUS_FAIL;
        }
    }

    // Getting all related attribute nodes so that we can start querying its status
    let fwu_node: unify_attribute_store_sys::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(
            fwu_node_info.ep_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU,
            0,
        )
    };
    if fwu_node == ATTRIBUTE_STORE_INVALID_NODE {
        log_warning!("FWU node {} not found in attribute store.", fwu_node);
        return SL_STATUS_FAIL;
    }
    let transfer_node: unify_attribute_store_sys::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(
            fwu_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
            0,
        )
    };

    // Before continuing, verify if we should tell the node to stop because
    // we do not consider that the firmware update is ongoing
    unsafe {
        if !is_firmware_update_ongoing(fwu_node_info.ep_node) {
            log_debug!(
                "Firmware Update is not ongoing with Endpoint node {}.",
                fwu_node_info.ep_node
            );
            zwave_firmware_update_send_abort_frame(
                fwu_node_info.ep_node,
                &fwu_node_info.info,
                get_frame.report_number,
            );
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
        log_warning!(
            "FWU fragment size node {} not found in attribute store.",
            fragment_size_node
        );
        return SL_STATUS_FAIL;
    }

    let fragment_size: u32;
    match attribute_store::read_attribute(
        fragment_size_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(v) => fragment_size = v,
        Err(status) => {
            log_warning!("Reading FWU fragment size failed.");
            zwave_firmware_update_send_abort_frame(
                fwu_node_info.ep_node,
                &fwu_node_info.info,
                get_frame.report_number,
            );
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
        log_error!(
            "FWU transfer filename node {} not found in attribute store.",
            transfer_filename_node
        );
        zwave_firmware_update_send_abort_frame(
            fwu_node_info.ep_node,
            &fwu_node_info.info,
            get_frame.report_number,
        );
    }
    let transfer_filename: String;
    match attribute_store::read_attribute(
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
            log_warning!("Reading filename from attribute store failed.");
            zwave_firmware_update_send_abort_frame(
                fwu_node_info.ep_node,
                &fwu_node_info.info,
                get_frame.report_number,
            );
            return SL_STATUS_FAIL;
        }
    }
    for idx in 0..get_frame.number_of_reports {
        let data = get_fwu_transfer_file_data(
            (idx as u32) + (get_frame.report_number as u32),
            fragment_size,
            transfer_filename.clone(),
        );

        // Queue frames so the first get sent first. Decrement by 10 each time.
        let qos_for_frame: u32;
        let qos_offset: u32 = ZWAVE_TX_RECOMMENDED_QOS_GAP * idx as u32;
        if ZWAVE_TX_QOS_RECOMMENDED_FWU_TRANSFER_PRIORITY > qos_offset {
            qos_for_frame = ZWAVE_TX_QOS_RECOMMENDED_FWU_TRANSFER_PRIORITY - qos_offset;
        } else {
            qos_for_frame = ZWAVE_TX_QOS_MIN_PRIORITY;
        }

        // Prepare the Z-Wave TX options.
        let parent_session_id: *mut c_void = std::ptr::null_mut();
        let tx_options = zwave_tx_sys::zwave_tx_options {
            number_of_responses: 0,
            qos_priority: qos_for_frame,
            discard_timeout_ms: 0,
            fasttrack: false,
            parent_session_id,
            valid_parent_session_id: false,
            is_test_frame: false,
            rf_power: zwave_tx_sys::rf_power_level_t::NORMAL_POWER,
            group_id: 0,
            is_first_follow_up: false,
            send_follow_ups: false,
        };

        // Constructing the report frame
        let fwu_md_report_frame: Vec<u8>;
        let last: bool = zwave_firmware_update_is_the_entire_image_transferred(transfer_node);

        unsafe {
            if zwave_node_get_command_class_version(
                COMMAND_CLASS_FIRMWARE_UPDATE_MD,
                node_id as u16,
                ep_id,
            ) > 1
            {
                fwu_md_report_frame = ZwaveFirmwareUpdateMdReportType::new_with_checksum(
                    last,
                    get_frame.report_number + idx as u16,
                    data,
                )
                .into();
            } else {
                fwu_md_report_frame = ZwaveFirmwareUpdateMdReportType::new(
                    last,
                    get_frame.report_number + idx as u16,
                    data,
                )
                .into();
            }
        }

        let session: *mut zwave_tx_sys::zwave_tx_session_id_t = std::ptr::null_mut();
        let user: *mut ::std::os::raw::c_void = std::ptr::null_mut();
        let tx_info = fwu_node_info.info;
        zwave_tx_sys::send_data(
            tx_info as zwave_tx_sys::zwave_controller_connection_info,
            &fwu_md_report_frame,
            tx_options,
            None,
            user,
            session,
        );

        // Update OFFSET node for UI to display progress bar
        let offset = ((idx as u32) + (get_frame.report_number as u32)) * fragment_size;
        set_transfer_offset(transfer_node, offset);

        // Don't send anymore if this is the last
        if last {
            break;
        }
    }

    // We just received a command, postpone the firmware update expiry.
    unsafe { postpone_firmware_update_expiry(fwu_node_info.ep_node) };

    SL_STATUS_OK
}

fn handle_fwu_md_prepare_get(
    fwu_node_info: FirmwareNodeInfo,
    prepare_get: ZwaveFirmwareUpdateMdPrepareGetType,
) -> sl_status_t {
    let mut status = ZwaveFirmwareUpdateGenericStatus::FIRMWARE_NOT_UPGRADABLE;
    if unify_config_sys::config_get_as_int("zpc.manufacturer_id")
        .unwrap_or(DEFAULT_MANUFACTURER_ID.into()) as u16
        != prepare_get.manufacturer_id
    {
        status = ZwaveFirmwareUpdateGenericStatus::INVALID_COMBINATION;
    }
    if prepare_get.firmware_id > DEFAULT_ZPC_NUMBER_OF_FIRMWARE_TARGETS as u16 {
        status = ZwaveFirmwareUpdateGenericStatus::INVALID_COMBINATION;
    }
    if unify_config_sys::config_get_as_int("zpc.hardware_version")
        .unwrap_or(DEFAULT_HARDWARE_VERSION.into()) as u16
        != prepare_get.hardware_version as u16
    {
        status = ZwaveFirmwareUpdateGenericStatus::INVALID_HARDWARE_VERSION;
    }
    if prepare_get.fragment_size
        > unsafe { firmware_transfer_get_theoretical_max_fragment_size(fwu_node_info.ep_node) }
        || (prepare_get.fragment_size == 0)
    {
        status = ZwaveFirmwareUpdateGenericStatus::EXCEEDS_MAX_FRAGMENT_SIZE;
    }

    let prepare_report_frame = ZwaveFirmwareUpdateMdPrepareReportType {
        status: status as u8,
        firmware_checksum: 0,
    };
    zwave_command_class_send_report(&fwu_node_info.info, prepare_report_frame)
}

fn handle_fwu_md_request_get(
    fwu_node_info: FirmwareNodeInfo,
    request_get: ZwaveFirmwareUpdateMdRequestGetType,
) -> sl_status_t {
    // Getting ep ID
    let ep_id: u8;
    match attribute_store::read_attribute(
        fwu_node_info.ep_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(v) => ep_id = v,
        Err(status) => {
            log_warning!(
                "Reading Endpoint ID failed for Attribute ID {}.",
                fwu_node_info.ep_node
            );
            return SL_STATUS_FAIL;
        }
    }

    // Getting Node ID
    let node_id_node = unsafe { attribute_store_get_node_parent(fwu_node_info.ep_node) };
    if node_id_node == ATTRIBUTE_STORE_INVALID_NODE {
        log_warning!(
            "NodeID node not found in the Attribute Store for Endpoint ID node {}.",
            fwu_node_info.ep_node
        );
        return SL_STATUS_FAIL;
    }
    let node_id: u16;
    match attribute_store::read_attribute(
        node_id_node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        Ok(v) => node_id = v,
        Err(status) => {
            log_warning!(
                "Reading NodeID value failed for Attribute {} .",
                node_id_node
            );
            return SL_STATUS_FAIL;
        }
    }

    let mut status = ZwaveFirmwareUpdateGenericStatus::FIRMWARE_NOT_UPGRADABLE;
    if unify_config_sys::config_get_as_int("zpc.manufacturer_id")
        .unwrap_or(DEFAULT_MANUFACTURER_ID.into()) as u16
        != request_get.manufacturer_id
    {
        status = ZwaveFirmwareUpdateGenericStatus::INVALID_COMBINATION;
    } else if (request_get.hardware_version != 0)
        && (unify_config_sys::config_get_as_int("zpc.hardware_version")
            .unwrap_or(DEFAULT_HARDWARE_VERSION.into()) as u16
            != request_get.hardware_version as u16)
    {
        status = ZwaveFirmwareUpdateGenericStatus::INVALID_HARDWARE_VERSION;
    } else if (unsafe {
        zwave_node_get_command_class_version(
            COMMAND_CLASS_FIRMWARE_UPDATE_MD,
            node_id as u16,
            ep_id,
        )
    } > 1)
        && (request_get.fragment_size
            > unsafe { firmware_transfer_get_theoretical_max_fragment_size(fwu_node_info.ep_node) }
            || (request_get.fragment_size == 0))
    {
        status = ZwaveFirmwareUpdateGenericStatus::EXCEEDS_MAX_FRAGMENT_SIZE;
    }

    let request_report_frame = ZwaveFirmwareUpdateMdRequestReportType {
        status: status as u8,
    };
    zwave_command_class_send_report(&fwu_node_info.info, request_report_frame)
}

fn handle_fwu_md_request_report(
    fwu_node_info: FirmwareNodeInfo,
    request_report: ZwaveFirmwareUpdateMdRequestReportType,
) -> sl_status_t {
    let transfer_node: unify_attribute_store_sys::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(
            fwu_node_info.fwu_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
            0,
        )
    };
    let last_status = match request_report.status {
        0x00 => {
            log_info!("Invalid combination of Manufacturer ID and Firmware ID. Firmware update request declined.");
            ZwaveFirmwareTransferStatus::ABORTED
        }
        0x01 => {
            log_info!("Device expected an authentication event to enable firmware update. Firmware update request declined.");
            ZwaveFirmwareTransferStatus::NOT_AUTHORIZED
        }
        0x02 => {
            log_error!("The requested fragment size exceeds the max fragment size. Firmware update request declined.");
            ZwaveFirmwareTransferStatus::ABORTED
        }
        0x03 => {
            log_info!("{} reports that its firmware target is not upgradable. Firmware update request declined.",
                        fwu_node_info
                );

            ZwaveFirmwareTransferStatus::NOT_SUPPORTED
        }
        0x04 => {
            log_warning!("Invalid hardware version. Firmware update request declined.");
            ZwaveFirmwareTransferStatus::ABORTED
        }
        0x05 => {
            log_info!("Another firmware image is currently being transferred. Firmware update request declined.");
            ZwaveFirmwareTransferStatus::ABORTED
        }
        0x06 => {
            log_info!("Insufficient battery level. Firmware update request declined.");
            ZwaveFirmwareTransferStatus::ABORTED
        }
        0xFF => {
            log_debug!("The node accepted to initiate the Firmware Update operation");
            if unsafe { attribute_store_node_exists(transfer_node) } {
                attribute_store::write_attribute(
                    transfer_node,
                    attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
                    ZwaveFirmwareTransferState::ONGOING as i32,
                )
                .unwrap();
                // Set the desired to IDLE, so we don't set the node to sleep
                // while transferring an image
                attribute_store::write_attribute(
                    transfer_node,
                    attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
                    ZwaveFirmwareTransferState::IDLE as i32,
                )
                .unwrap();
            }
            ZwaveFirmwareTransferStatus::SUCCESS
        }
        _ => ZwaveFirmwareTransferStatus::ABORTED,
    };

    // Reset the transfer state if request report indicates error
    if last_status != ZwaveFirmwareTransferStatus::SUCCESS {
        set_transfer_state(transfer_node, ZwaveFirmwareTransferState::IDLE);
    }
    // Set the last status
    unsafe {
        set_firmware_transfer_last_status(transfer_node, last_status);
    }

    SL_STATUS_OK
}

fn handle_fwu_activation_set(
    fwu_node_info: FirmwareNodeInfo,
    activation_set: ZwaveFirmwareUpdateActivationSetType,
) -> sl_status_t {
    let mut activation_status = ZwaveFirmwareUpdateActivationStatus::ERROR_ACTIVATION;
    if unify_config_sys::config_get_as_int("zpc.hardware_version")
        .unwrap_or(DEFAULT_HARDWARE_VERSION.into()) as u16
        != activation_set.hardware_version as u16
    {
        activation_status = ZwaveFirmwareUpdateActivationStatus::INVALID_COMBINATION;
    }

    let activation_status_report_frame = ZwaveFirmwareUpdateActivationStatusReportType {
        manufacturer_id: activation_set.manufacturer_id,
        firmware_id: activation_set.firmware_id,
        firmware_checksum: activation_set.firmware_checksum,
        firmware_target: activation_set.firmware_target,
        firmware_update_status: activation_status as u8,
        hardware_version: activation_set.hardware_version,
    };
    zwave_command_class_send_report(&fwu_node_info.info, activation_status_report_frame)
}

fn handle_fwu_activation_status_report(
    fwu_node_info: FirmwareNodeInfo,
    status_report: ZwaveFirmwareUpdateActivationStatusReportType,
) -> sl_status_t {
    let transfer_node: unify_attribute_store_sys::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(
            fwu_node_info.fwu_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
            0,
        )
    };
    let last_status = match status_report.firmware_update_status {
        0x0 => {
            log_warning!("Invalid combination of manufacturer ID, firmware ID and hardware version or firmware target. The received image will not be stored.");
            ZwaveFirmwareTransferStatus::ABORTED
        }
        0x1 => {
            log_info!(
                "Error activating the firmware. Last known firmware image has been restored."
            );
            ZwaveFirmwareTransferStatus::INVALID_IMAGE
        }
        0xff => {
            log_debug!("Firmware update completed successfully.");
            ZwaveFirmwareTransferStatus::SUCCESS
        }
        _ => {
            log_info!("Unknown status for Firmware Update Activation Report: {}. Assuming that the activation failed.",status_report.firmware_update_status);
            ZwaveFirmwareTransferStatus::ABORTED
        }
    };

    // Reset the transfer offset if the report indicates any error
    if last_status != ZwaveFirmwareTransferStatus::SUCCESS {
        set_transfer_offset(transfer_node, 0);
    }

    // Set both states to IDLE
    set_transfer_state(transfer_node, ZwaveFirmwareTransferState::IDLE);
    if unsafe { attribute_store_node_exists(transfer_node) } {
        attribute_store::write_attribute(
            transfer_node,
            DESIRED_ATTRIBUTE,
            ZwaveFirmwareTransferState::IDLE,
        )
        .unwrap();
    }
    // Set the last status
    unsafe {
        set_firmware_transfer_last_status(transfer_node, last_status);
    }
    zwave_firmware_update_initiate_interview(transfer_node);

    SL_STATUS_OK
}

fn on_fwu_waiting_timer_callback(transfer_node: attribute_store_node_t) {
    let mut desired_fwu_value: ZwaveFirmwareTransferState = ZwaveFirmwareTransferState::IDLE;
    if unsafe {
        attribute_store_is_value_defined(
            transfer_node,
            attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
        )
    } {
        match attribute_store::read_attribute(
            transfer_node,
            attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
        ) {
            Ok(value) => desired_fwu_value = value,
            Err(_) => {
                log_warning!("Error on reading FWU_TRANSFER desired value");
                return;
            }
        }
    }
    let mut reported_fwu_value: ZwaveFirmwareTransferState = ZwaveFirmwareTransferState::IDLE;
    if unsafe {
        attribute_store_is_value_defined(
            transfer_node,
            attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        )
    } {
        match attribute_store::read_attribute(
            transfer_node,
            attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        ) {
            Ok(value) => reported_fwu_value = value,
            Err(_) => {
                log_warning!("Error on reading FWU_TRANSFER desired value");
                return;
            }
        }
    }
    if desired_fwu_value == ZwaveFirmwareTransferState::IDLE
        && reported_fwu_value == ZwaveFirmwareTransferState::WAITING_FOR_ACTIVATION
    {
        attribute_store::write_attribute(
            transfer_node,
            attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
            ZwaveFirmwareTransferState::WAITING_FOR_ACTIVATION,
        )
        .unwrap();
        unsafe {
            attribute_store_undefine_reported(transfer_node);
        }
    }
}

fn on_reboot_complete(transfer_node: unify_attribute_store_sys::attribute_store_node_t) {
    set_transfer_state(transfer_node, ZwaveFirmwareTransferState::IDLE);
    zwave_firmware_update_initiate_interview(transfer_node);
}

fn handle_fwu_md_status_report(
    fwu_node_info: FirmwareNodeInfo,
    status_report: ZwaveFirmwareUpdateMdStatusReportType,
) -> sl_status_t {
    let transfer_node: unify_attribute_store_sys::attribute_store_node_t = unsafe {
        attribute_store_get_node_child_by_type(
            fwu_node_info.fwu_node,
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
            0,
        )
    };

    let last_status;
    let current_transfer_state: ZwaveFirmwareTransferState;
    match status_report.status {
        0xff => {
            // Skipping reboot state as waittime is 0.
            if status_report.wait_time == 0 {
                current_transfer_state = ZwaveFirmwareTransferState::IDLE;
                last_status = ZwaveFirmwareTransferStatus::SUCCESS;
                log_debug!(
                    "New firmware was successfully stored in temporary non volatile memory."
                );
            } else {
                current_transfer_state = ZwaveFirmwareTransferState::WAITING_FOR_REBOOT;
                last_status = ZwaveFirmwareTransferStatus::SUCCESS;

                // Save the waittime to EXPIRY_TIME
                let expiry_node: unify_attribute_store_sys::attribute_store_node_t = unsafe {
                    attribute_store_get_node_child_by_type(
                        transfer_node,
                        ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_EXPIRY_TIME,
                        0,
                    )
                };
                if unsafe { attribute_store_node_exists(expiry_node) } {
                    attribute_store::write_attribute(
                        expiry_node,
                        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
                        status_report.wait_time as u64,
                    )
                    .unwrap();
                }
                log_debug!("New firmware was successfully stored in temporary non volatile memory. The device may have restarted itself. Wait for {} seconds for the deivce to be ready.", status_report.wait_time);
            }
        }
        0xfe => {
            current_transfer_state = ZwaveFirmwareTransferState::IDLE;
            last_status = ZwaveFirmwareTransferStatus::SUCCESS;

            log_debug!("New firmware was successfully stored in temporary non volatile memory.");
        }
        0xfd => {
            current_transfer_state = ZwaveFirmwareTransferState::WAITING_FOR_ACTIVATION;
            last_status = ZwaveFirmwareTransferStatus::SUCCESS;

            log_debug!("Firmware image downloaded successfully. The node now awaits for the activation command.");
        }
        0x0 => {
            current_transfer_state = ZwaveFirmwareTransferState::IDLE;
            last_status = ZwaveFirmwareTransferStatus::INVALID_IMAGE;
            log_warning!("The device was unable to receive the requested firmware data without checksum error.");
        }
        0x1 => {
            current_transfer_state = ZwaveFirmwareTransferState::IDLE;
            last_status = ZwaveFirmwareTransferStatus::ABORTED;
            log_info!("The device was unable to receive the Firmware image..");
        }
        0x2 => {
            current_transfer_state = ZwaveFirmwareTransferState::IDLE;
            last_status = ZwaveFirmwareTransferStatus::INVALID_IMAGE;
            log_warning!("Supporting node reports invalid Manufacturer ID for the transfered image. Firmware Update failed.");
        }
        0x3 => {
            current_transfer_state = ZwaveFirmwareTransferState::IDLE;
            last_status = ZwaveFirmwareTransferStatus::INVALID_IMAGE;
            log_warning!("Supporting node reports invalid Firmware ID for the transfered image. Firmware Update failed.");
        }
        0x4 => {
            current_transfer_state = ZwaveFirmwareTransferState::IDLE;
            last_status = ZwaveFirmwareTransferStatus::INVALID_IMAGE;
            log_warning!("Supporting node reports invalid Firmware Target for the transfered image. Firmware Update failed.");
        }
        0x5 => {
            current_transfer_state = ZwaveFirmwareTransferState::IDLE;
            last_status = ZwaveFirmwareTransferStatus::INVALID_IMAGE;
            log_warning!("Supporting node reports invalid header information for the transfered image. Firmware Update failed.");
        }
        0x6 => {
            current_transfer_state = ZwaveFirmwareTransferState::IDLE;
            last_status = ZwaveFirmwareTransferStatus::INVALID_IMAGE;
            log_warning!("Supporting node reports invalid header format for the transfered image. Firmware Update failed.");
        }
        0x7 => {
            current_transfer_state = ZwaveFirmwareTransferState::IDLE;
            last_status = ZwaveFirmwareTransferStatus::ABORTED;
            log_warning!("Supporting node reports insufficient memory for the transfered image. Firmware Update failed.");
        }
        0x8 => {
            current_transfer_state = ZwaveFirmwareTransferState::IDLE;
            last_status = ZwaveFirmwareTransferStatus::INVALID_IMAGE;
            log_warning!("Supporting node reports non-matching hardware version for the transfered image. Firmware Update failed.");
        }
        _ => {
            current_transfer_state = ZwaveFirmwareTransferState::IDLE;
            last_status = ZwaveFirmwareTransferStatus::ABORTED;
            log_warning!(
                "The firmware udpate process was failed. Status: {}",
                status_report.status
            );
        }
    }

    // Update the transfer state and last status
    set_transfer_state(transfer_node, current_transfer_state);
    unsafe {
        set_firmware_transfer_last_status(transfer_node, last_status);
    }
    SL_STATUS_OK
}

const CONFIG: ZwaveHandlerConfig = ZwaveHandlerConfig {
    minimal_scheme:
    zwave_command_handler_sys::zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
    command_class: COMMAND_CLASS_FIRMWARE_UPDATE_MD,
    command_class_name: "Firmware Update\0",
    comments: "\0",
    version: VERSION,
    manual_security_validation: false,
};

fn handle_fw_md_get(fwu_node_info: FirmwareNodeInfo) -> sl_status_t {
    let report_frame = ZwaveFirmwareMdReportType {
        manufacturer_id: unify_config_sys::config_get_as_int("zpc.manufacturer_id")
            .unwrap_or(DEFAULT_MANUFACTURER_ID.into()) as u16,
        firmware_0_id: FIRMWARE_0_ID,
        firmware_0_checksum: 0,
        firmware_upgradeable: 0,
        number_of_firmware_targets: DEFAULT_ZPC_NUMBER_OF_FIRMWARE_TARGETS,
        max_fragment_size: unsafe {
            firmware_transfer_get_theoretical_max_fragment_size(fwu_node_info.ep_node)
        },
        firmware_n_id: vec![unify_config_sys::config_get_as_int("zpc.hardware_version")
            .unwrap_or(DEFAULT_HARDWARE_VERSION.into()) as u16],
        hardware_version: unify_config_sys::config_get_as_int("zpc.hardware_version")
            .unwrap_or(DEFAULT_HARDWARE_VERSION.into()) as u8,
        activation: unsafe { node_supports_firmware_activation(fwu_node_info.ep_node) },
        cc: unsafe { node_supports_cc(fwu_node_info.ep_node) },
    };
    zwave_command_class_send_report(&fwu_node_info.info, report_frame)
}

fn handle_fw_md_report(
    fwu_node_info: FirmwareNodeInfo,
    report: ZwaveFirmwareMdReportType,
) -> sl_status_t {
    if unsafe { attribute_store_node_exists(fwu_node_info.fwu_node) } {
        attribute_store::write_attribute(
            fwu_node_info.fwu_node,
            attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
            0_i32,
        )
        .unwrap();
    }
    let mut fw = vec![report.firmware_0_id];
    fw.append(report.firmware_n_id.clone().as_mut());

    // firmware_0 +  firmware_n
    let mut fwu_idx: i32 = 0;
    for fwu_id in fw {
        let fw_idx_node = attribute_store::get_or_add_node(
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW,
            fwu_node_info.fwu_node,
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
            Some(report.firmware_upgradeable as i32),
        );

        fwu_idx += 1;
    }
    attribute_store::get_or_add_node(
        ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_MANUFACTURER_ID,
        fwu_node_info.fwu_node,
        None,
        Some(report.manufacturer_id as i32),
    );

    attribute_store::get_or_add_node(
        ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_ACTIVATION,
        fwu_node_info.fwu_node,
        None,
        Some(report.activation as i32),
    );

    attribute_store::get_or_add_node(
        ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_CC,
        fwu_node_info.fwu_node,
        None,
        Some(report.cc as i32),
    );

    let max_fragment_size: u16;
    if report.max_fragment_size == (MAX_FRAME_LEN as u16) {
        unsafe {
            max_fragment_size =
                firmware_transfer_get_theoretical_max_fragment_size(fwu_node_info.ep_node);
        }
    } else {
        max_fragment_size = report.max_fragment_size;
    }
    attribute_store::get_or_add_node(
        ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FRAGMENT_SIZE,
        fwu_node_info.fwu_node,
        None,
        Some(max_fragment_size as i32),
    );
    SL_STATUS_OK
}

#[generate_extern_c]
unsafe fn attribute_get_md_fwu(_node: u32, data: *mut u8, data_len: *mut u16) -> sl_status_t {
    let raw_buffer = std::slice::from_raw_parts_mut(data, 2);
    raw_buffer[0] = COMMAND_CLASS_FIRMWARE_UPDATE_MD as u8;
    raw_buffer[1] = FIRMWARE_MD_GET as u8;

    *data_len = 2;
    SL_STATUS_OK
}

#[generate_extern_c]
unsafe fn on_attribute_version_change(node: u32, change: attribute_store_change_t) {
    if change == attribute_store_change_t::ATTRIBUTE_UPDATED
        && attribute_store_is_value_defined(
            node,
            attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        )
    {
        let ep_node = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
        if attribute_store_get_node_child_by_type(ep_node, ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU, 0)
            == ATTRIBUTE_STORE_INVALID_NODE
            && attribute_store::add_node(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU, ep_node).is_err()
        {
        }
    }
}

#[generate_extern_c]
unsafe fn on_fwu_waiting_timer(node: u32, change: attribute_store_change_t) {
    let mut desired_fwu_value: ZwaveFirmwareTransferState = ZwaveFirmwareTransferState::IDLE;
    if attribute_store_is_value_defined(node, attribute_store_node_value_state_t::DESIRED_ATTRIBUTE)
    {
        match attribute_store::read_attribute(
            node,
            attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
        ) {
            Ok(value) => desired_fwu_value = value,
            Err(_) => {
                log_warning!("Error on reading FWU_TRANSFER desired value");
                return;
            }
        }
    }
    let mut reported_fwu_value: ZwaveFirmwareTransferState = ZwaveFirmwareTransferState::IDLE;
    if attribute_store_is_value_defined(
        node,
        attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
    ) {
        match attribute_store::read_attribute(
            node,
            attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        ) {
            Ok(value) => reported_fwu_value = value,
            Err(_) => {
                log_warning!("Error on reading FWU_TRANSFER desired value");
                return;
            }
        }
    }
    if change == attribute_store_change_t::ATTRIBUTE_UPDATED
        && reported_fwu_value == ZwaveFirmwareTransferState::WAITING_FOR_ACTIVATION
        && desired_fwu_value == ZwaveFirmwareTransferState::IDLE
    {
        // Find Apply Timestamp
        let apply_timestamp_node: unify_attribute_store_sys::attribute_store_node_t =
            attribute_store_get_node_child_by_type(
                node,
                ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_APPLY_TIMESTAMP,
                0,
            );

        let apply_timestamp_as_seconds: u64;
        match attribute_store::read_attribute(
            apply_timestamp_node,
            attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        ) {
            Ok(v) => apply_timestamp_as_seconds = v,
            Err(status) => return,
        }
        let mut current_time_as_seconds = 0;
        match SystemTime::now().duration_since(SystemTime::UNIX_EPOCH) {
            Ok(n) => current_time_as_seconds = n.as_secs(),
            Err(_) => log_warning!("SystemTime before UNIX EPOCH!"),
        }
        // Safe guard for duration calculation
        let duration = if apply_timestamp_as_seconds > current_time_as_seconds {
            apply_timestamp_as_seconds - current_time_as_seconds
        } else {
            0
        };

        if let Err(e) = attribute_store::set_callback_on_timeout(
            node,
            std::time::Duration::from_secs(duration),
            move |_| on_fwu_waiting_timer_callback(node),
        ) {
            log_error!(
                "Scheduling of activation timer callback failed for attribute node: {:#04x}",
                node
            );
        }
    } else if change == attribute_store_change_t::ATTRIBUTE_UPDATED
        && reported_fwu_value == ZwaveFirmwareTransferState::WAITING_FOR_REBOOT
        && desired_fwu_value == ZwaveFirmwareTransferState::IDLE
    {
        let expiry_node: unify_attribute_store_sys::attribute_store_node_t =
            attribute_store_get_node_child_by_type(
                node,
                ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_EXPIRY_TIME,
                0,
            );
        let expiry_time: u64;
        match attribute_store::read_attribute(
            expiry_node,
            attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        ) {
            Ok(v) => expiry_time = v,
            Err(status) => {
                log_debug!("Error on reading expiry time");
                return;
            }
        }
        log_info!("Device is rebooting after firmware update. Scheduling of reboot timer callback for attribute node: {:#04}", node);
        if let Err(e) = attribute_store::set_callback_on_timeout(
            node,
            std::time::Duration::from_secs(expiry_time as u64),
            on_reboot_complete,
        ) {
            log_error!(
                "Scheduling of activation timer callback failed for node: {:#04x}",
                node
            );
        }
    }
}

#[derive(Clone, Copy)]
struct FirmwareNodeInfo {
    info: zwave_controller_sys::zwave_controller_connection_info,
    fwu_node: attribute_store_node_t,
    ep_node: attribute_store_node_t,
}

impl FirmwareNodeInfo {
    fn new(info: zwave_controller_sys::zwave_controller_connection_info) -> Self {
        unsafe {
            let ep_node = zwave_command_class_get_endpoint_node(
                &info as *const zwave_command_classes_sys::zwave_controller_connection_info,
            );

            let fwu_node = attribute_store_get_node_child_by_type(
                ep_node,
                ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU,
                0,
            );
            FirmwareNodeInfo {
                info,
                fwu_node,
                ep_node,
            }
        }
    }
}

impl std::fmt::Display for FirmwareNodeInfo {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        unsafe {
            write!(
                f,
                "NodeID {}:{}",
                self.info.remote.__bindgen_anon_1.node_id, self.info.remote.endpoint_id
            )
        }
    }
}

#[zwave_command_class(CONFIG)]
#[derive(Default)]
pub struct ZwaveCommandClassFirmwareUpdate {}

impl ZwaveCommandClassFirmwareUpdate {
    pub fn on_control(
        &self,
        info: zwave_controller_sys::zwave_controller_connection_info,
        frame: ZwaveFirmwareUpdateFrame,
    ) -> sl_status_t {
        use ZwaveFirmwareUpdateFrame::*;

        let node_info = FirmwareNodeInfo::new(info);
        match frame {
            ZwaveFirmwareMdReport(report) => handle_fw_md_report(node_info, report),
            ZwaveFirmwareUpdateMdRequestReport(request_report) => {
                handle_fwu_md_request_report(node_info, request_report)
            }
            ZwaveFirmwareUpdateMdGet(data) => handle_fwu_md_get(node_info, data),
            ZwaveFirmwareUpdateMdStatusReport(data) => handle_fwu_md_status_report(node_info, data),
            ZwaveFirmwareUpdateActivationStatusReport(data) => {
                handle_fwu_activation_status_report(node_info, data)
            }
            _ => SL_STATUS_NOT_SUPPORTED,
        }
    }

    pub fn on_support(
        &self,
        info: zwave_command_classes_sys::zwave_controller_connection_info,
        frame: ZwaveFirmwareUpdateFrame,
    ) -> sl_status_t {
        use ZwaveFirmwareUpdateFrame::*;

        let node_info = FirmwareNodeInfo::new(info);
        match frame {
            ZwaveFirmwareMdGet => handle_fw_md_get(node_info),
            ZwaveFirmwareUpdateMdRequestGet(request_get) => {
                handle_fwu_md_request_get(node_info, request_get)
            }
            ZwaveFirmwareUpdateActivationSet(activation_set) => {
                handle_fwu_activation_set(node_info, activation_set)
            }
            ZwaveFirmwareUpdateMdPrepareGet(prepare_get) => {
                handle_fwu_md_prepare_get(node_info, prepare_get)
            }
            _ => SL_STATUS_NOT_SUPPORTED,
        }
    }

    pub fn on_init(&self, status: sl_status_t) -> sl_status_t {
        attribute_resolver_register_rule(
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
            as_extern_c!(attribute_set_md_fwu_fw_transfer),
            as_extern_c!(attribute_get_md_fwu_fw_transfer),
        );
        attribute_resolver_register_rule(
            ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU,
            None,
            as_extern_c!(attribute_get_md_fwu),
        );
        unsafe {
            attribute_store_register_callback_by_type_and_state(
                as_extern_c!(on_fwu_waiting_timer),
                ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
                REPORTED_ATTRIBUTE,
            );
            attribute_store_register_callback_by_type(
                as_extern_c!(on_attribute_version_change),
                ATTRIBUTE_COMMAND_CLASS_FWU_MD_VERSION,
            );
        };
        status
    }
}
