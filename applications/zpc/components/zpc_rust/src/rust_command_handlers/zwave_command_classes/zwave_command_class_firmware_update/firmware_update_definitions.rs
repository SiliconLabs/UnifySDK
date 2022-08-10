///////////////////////////////////////////////////////////////////////////////
// # License
// <b>Copyright 2022  Silicon Laboratories Inc. www.silabs.com</b>
///////////////////////////////////////////////////////////////////////////////
// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.
//
///////////////////////////////////////////////////////////////////////////////

use crate::zwave_controller_sys::{self, *};
use std::convert::{TryFrom, TryInto};

pub const COMMAND_CLASS_FIRMWARE_UPDATE_MD: u16 = 0x7A;
pub const COMMAND_CLASS_VERSION: u16 = 0x86;
pub const FIRMWARE_MD_GET: u16 = 0x01;
pub const FIRMWARE_MD_REPORT: u16 = 0x02;
pub const FIRMWARE_UPDATE_MD_REQUEST_GET: u16 = 0x03;
pub const FIRMWARE_UPDATE_MD_REQUEST_REPORT: u16 = 0x04;
pub const FIRMWARE_UPDATE_MD_GET: u16 = 0x05;
pub const FIRMWARE_UPDATE_MD_REPORT: u16 = 0x06;
pub const FIRMWARE_UPDATE_MD_STATUS_REPORT: u16 = 0x07;
pub const FIRMWARE_UPDATE_ACTIVATION_SET: u16 = 0x08;
pub const FIRMWARE_UPDATE_ACTIVATION_STATUS_REPORT: u16 = 0x09;
pub const FIRMWARE_UPDATE_MD_PREPARE_GET: u16 = 0x0A;
pub const FIRMWARE_UPDATE_MD_PREPARE_REPORT: u16 = 0x0B;
pub const VERSION: u8 = 0x07;

pub const FIRMWARE_0_ID: u16 = 0;

pub const MAX_REPEATERS: u32 = 4;
pub const LAST_USED_ROUTE_CONF_SIZE: u32 = 1;
pub const LAST_USED_ROUTE_SIZE: u32 = MAX_REPEATERS + LAST_USED_ROUTE_CONF_SIZE;
pub const ZWAVE_TX_QOS_RECOMMENDED_FWU_TRANSFER_PRIORITY: u32 = 0x000000FF;
// Default values for ZwaveFirmwareMdReport
pub const DEFAULT_FIRMWARE_UPGRADABLE: u8 = 0xff;
pub const DEFAULT_FIRMWARE_TARGET: u8 = 0;
pub const DEFAULT_NUMBER_OF_FIRMWARE_TARGETS: u8 = 0;
pub const DEFAULT_ZPC_NUMBER_OF_FIRMWARE_TARGETS: u8 = 1;
pub const DEFAULT_HARDWARE_VERSION: u8 = 1;
pub const DEFAULT_ACTIVATION: bool = false;
pub const DEFAULT_WAIT_TIME: u16 = 10;
pub const DEFAULT_CC: bool = false;
pub const DEFAULT_MANUFACTURER_ID: u16 = 0;

// TODO remove this when we have it generated
pub const MAX_FRAME_LEN: usize = 255;

pub struct ZwaveHandlerConfig {
    pub minimal_scheme: zwave_controller_sys::zwave_controller_encapsulation_scheme_t,
    pub command_class: u16,
    pub command_class_name: &'static str,
    pub comments: &'static str,
    pub version: u8,
    pub manual_security_validation: bool,
}

#[derive(Clone, PartialEq, Debug)]
pub enum ZwaveFirmwareUpdateFrame {
    ZwaveFirmwareMdGet,
    ZwaveFirmwareMdReport(ZwaveFirmwareMdReportType),
    ZwaveFirmwareUpdateMdGet(ZwaveFirmwareUpdateMdGetType),
    ZwaveFirmwareUpdateMdReport(ZwaveFirmwareUpdateMdReportType),
    ZwaveFirmwareUpdateMdRequestGet(ZwaveFirmwareUpdateMdRequestGetType),
    ZwaveFirmwareUpdateMdRequestReport(ZwaveFirmwareUpdateMdRequestReportType),
    ZwaveFirmwareUpdateActivationSet(ZwaveFirmwareUpdateActivationSetType),
    ZwaveFirmwareUpdateMdStatusReport(ZwaveFirmwareUpdateMdStatusReportType),
    ZwaveFirmwareUpdateActivationStatusReport(ZwaveFirmwareUpdateActivationStatusReportType),
    ZwaveFirmwareUpdateMdPrepareGet(ZwaveFirmwareUpdateMdPrepareGetType),
    ZwaveFirmwareUpdateMdPrepareReport(ZwaveFirmwareUpdateMdPrepareReportType),
}

#[derive(PartialEq)]
pub enum ZwaveFirmwareUpdateGenericStatus {
    INVALID_COMBINATION = 0x0,
    OUT_OF_BAND_AUTH_REQUIRED = 0x01,
    EXCEEDS_MAX_FRAGMENT_SIZE = 0x02,
    FIRMWARE_NOT_UPGRADABLE = 0x03,
    INVALID_HARDWARE_VERSION = 0x04,
    INITIATE_FIRMWARE_UPDATE = 0xFF,
}

pub enum ZwaveFirmwareUpdateActivationStatus {
    INVALID_COMBINATION = 0x0,
    ERROR_ACTIVATION = 0x01,
    FIRMWARE_UPDATE_SUCCESSFULLY = 0xFF,
}

/// This is the base type for all command class frames.
/// It is used in context of converting raw byte data into
/// Typed zwave frames.
pub type ZwaveFrameResult<T> = Result<T, ZwaveFrameError>;

/// Error type to be used for converting raw byte data into
/// typed rust structures.
#[derive(Debug, PartialEq)]
pub enum ZwaveFrameError {
    ParsingError { cc: u16, command: u16, raw: Vec<u8> },
    ExpectedOtherCommandClass { have_cc: u16, expected_cc: u16 },
    InvalidFrame(u16),
}

impl std::fmt::Display for ZwaveFrameError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            ZwaveFrameError::ParsingError { cc, command, raw } => {
                write!(
                    f,
                    "Error Parsing CC:{:#04x} Command:{:#04x}. {:?}",
                    cc, command, raw
                )
            }
            ZwaveFrameError::ExpectedOtherCommandClass {
                have_cc,
                expected_cc,
            } => {
                write!(
                    f,
                    "Expecting CC: {:#04x} received: {:#04x}",
                    expected_cc, have_cc
                )
            }
            ZwaveFrameError::InvalidFrame(cc) => {
                write!(f, "Invalid frame: {:#04x}", cc)
            }
        }
    }
}

pub fn try_u8slice_to_msb(data: Option<&[u8]>) -> Option<u16> {
    Some(((*data?.get(0)? as u16) << 8) | *data?.get(1)? as u16)
}

impl TryFrom<&[u8]> for ZwaveFirmwareUpdateFrame {
    type Error = ZwaveFrameError;
    fn try_from(frame: &[u8]) -> Result<Self, Self::Error> {
        // If frame is smaller than 2 (CC + command), return Error
        if frame.len() < 2 {
            return Err(ZwaveFrameError::InvalidFrame(
                COMMAND_CLASS_FIRMWARE_UPDATE_MD,
            ));
        }

        // If it isn't the correct CC return Error
        if frame[0] as u16 != COMMAND_CLASS_FIRMWARE_UPDATE_MD {
            return Err(ZwaveFrameError::ExpectedOtherCommandClass {
                have_cc: frame[0] as u16,
                expected_cc: COMMAND_CLASS_FIRMWARE_UPDATE_MD,
            });
        }

        use ZwaveFirmwareUpdateFrame::*;
        // Dont strip header from the frame buffer! the from traits expect
        // the header bytes to still be there.
        let typed_frame = match u16::from(frame[1]) {
            FIRMWARE_MD_GET => ZwaveFirmwareMdGet,
            FIRMWARE_MD_REPORT => ZwaveFirmwareMdReport(ZwaveFirmwareMdReportType::from(frame)),
            FIRMWARE_UPDATE_MD_REQUEST_GET => {
                ZwaveFirmwareUpdateMdRequestGet(ZwaveFirmwareUpdateMdRequestGetType::from(frame))
            }
            FIRMWARE_UPDATE_MD_REQUEST_REPORT => ZwaveFirmwareUpdateMdRequestReport(
                ZwaveFirmwareUpdateMdRequestReportType::from(frame),
            ),
            FIRMWARE_UPDATE_MD_GET => {
                ZwaveFirmwareUpdateMdGet(ZwaveFirmwareUpdateMdGetType::from(frame))
            }
            FIRMWARE_UPDATE_MD_STATUS_REPORT => ZwaveFirmwareUpdateMdStatusReport(
                ZwaveFirmwareUpdateMdStatusReportType::from(frame),
            ),
            FIRMWARE_UPDATE_ACTIVATION_SET => {
                ZwaveFirmwareUpdateActivationSet(ZwaveFirmwareUpdateActivationSetType::from(frame))
            }
            FIRMWARE_UPDATE_ACTIVATION_STATUS_REPORT => ZwaveFirmwareUpdateActivationStatusReport(
                ZwaveFirmwareUpdateActivationStatusReportType::from(frame),
            ),
            FIRMWARE_UPDATE_MD_PREPARE_GET => {
                ZwaveFirmwareUpdateMdPrepareGet(ZwaveFirmwareUpdateMdPrepareGetType::from(frame))
            }
            _ => {
                return Err(ZwaveFrameError::InvalidFrame(
                    COMMAND_CLASS_FIRMWARE_UPDATE_MD,
                ))
            }
        };
        Ok(typed_frame)
    }
}

#[derive(Clone, PartialEq, Debug, Default)]
pub struct ZwaveFirmwareUpdateMdPrepareGetType {
    pub manufacturer_id: u16,
    pub firmware_id: u16,
    pub firmware_target: u8,
    pub fragment_size: u16,
    pub hardware_version: u8,
}

impl From<&[u8]> for ZwaveFirmwareUpdateMdPrepareGetType {
    fn from(frame: &[u8]) -> Self {
        ZwaveFirmwareUpdateMdPrepareGetType {
            manufacturer_id: try_u8slice_to_msb(frame.get(2..4)).unwrap_or_default(),
            firmware_id: try_u8slice_to_msb(frame.get(4..6)).unwrap_or_default(),
            firmware_target: frame.get(6).cloned().unwrap_or(DEFAULT_FIRMWARE_TARGET),
            fragment_size: try_u8slice_to_msb(frame.get(7..9)).unwrap_or(0),
            hardware_version: frame.get(9).cloned().unwrap_or(DEFAULT_HARDWARE_VERSION),
        }
    }
}

#[derive(Clone, PartialEq, Debug, Default)]
pub struct ZwaveFirmwareUpdateMdPrepareReportType {
    pub status: u8,
    pub firmware_checksum: u16,
}

impl From<ZwaveFirmwareUpdateMdPrepareReportType> for Vec<u8> {
    fn from(typ: ZwaveFirmwareUpdateMdPrepareReportType) -> Self {
        vec![
            COMMAND_CLASS_FIRMWARE_UPDATE_MD.try_into().unwrap(),
            FIRMWARE_UPDATE_MD_PREPARE_REPORT.try_into().unwrap(),
            (typ.status),
            (typ.firmware_checksum >> 8) as u8,
            (typ.firmware_checksum & 0xFF) as u8,
        ]
    }
}

#[derive(Clone, PartialEq, Debug, Default)]
pub struct ZwaveFirmwareMdReportType {
    pub manufacturer_id: u16,
    pub firmware_0_id: u16,
    pub firmware_0_checksum: u16,
    pub firmware_upgradeable: u8,
    pub number_of_firmware_targets: u8,
    pub max_fragment_size: u16,
    pub firmware_n_id: Vec<u16>,
    pub hardware_version: u8,
    pub activation: bool,
    pub cc: bool,
}

impl From<&[u8]> for ZwaveFirmwareMdReportType {
    fn from(frame: &[u8]) -> Self {
        let mut report = ZwaveFirmwareMdReportType::default();
        report.manufacturer_id = try_u8slice_to_msb(frame.get(2..4)).unwrap_or_default();
        report.firmware_0_id = try_u8slice_to_msb(frame.get(4..6)).unwrap_or_default();
        report.firmware_0_checksum = try_u8slice_to_msb(frame.get(6..8)).unwrap_or_default();
        report.firmware_upgradeable = *frame.get(8).unwrap_or(&DEFAULT_FIRMWARE_UPGRADABLE);
        report.max_fragment_size = frame
            .get(10..13)
            .map(|v| -> u16 { ((v[0] as u16) << 8) | v[1] as u16 })
            .unwrap_or(MAX_FRAME_LEN as u16);
        // End of minimum frame (V1)
        report.number_of_firmware_targets =
            *frame.get(9).unwrap_or(&DEFAULT_NUMBER_OF_FIRMWARE_TARGETS);

        for i in 0..report.number_of_firmware_targets as usize {
            let from = 12 + i * 2;
            let to = 14 + i * 2;
            if let Some(elem) = frame
                .get(from..to)
                .map(|v| ((v[0] as u16) << 8) | v[1] as u16)
            {
                report.firmware_n_id.push(elem);
            }
        }
        let number_of_firmware_targets_size = (report.number_of_firmware_targets as usize) * 2;
        report.hardware_version = *frame
            .get(12 + number_of_firmware_targets_size)
            .unwrap_or(&DEFAULT_HARDWARE_VERSION);
        report.activation = frame
            .get(13 + number_of_firmware_targets_size)
            .map(|v| (v & 0x02) != 0)
            .unwrap_or(DEFAULT_ACTIVATION);
        report.cc = frame
            .get(13 + number_of_firmware_targets_size)
            .map(|v| (v & 0x01) != 0)
            .unwrap_or(DEFAULT_CC);

        report
    }
}

impl From<ZwaveFirmwareMdReportType> for Vec<u8> {
    fn from(typ: ZwaveFirmwareMdReportType) -> Self {
        let mut frame = vec![
            COMMAND_CLASS_FIRMWARE_UPDATE_MD.try_into().unwrap(),
            FIRMWARE_MD_REPORT.try_into().unwrap(),
            (typ.manufacturer_id >> 8) as u8,
            (typ.manufacturer_id & 0xFF) as u8,
            (typ.firmware_0_id >> 8) as u8,
            (typ.firmware_0_id & 0xFF) as u8,
            (typ.firmware_0_checksum >> 8) as u8,
            (typ.firmware_0_checksum & 0xFF) as u8,
            typ.firmware_upgradeable,
            typ.number_of_firmware_targets,
            (typ.max_fragment_size >> 8) as u8,
            (typ.max_fragment_size & 0xFF) as u8,
        ];

        let mut count = 0;
        for firmware_id in typ.firmware_n_id {
            // We have filled all the firmware IDs
            if count >= typ.number_of_firmware_targets {
                break;
            }
            frame.push((firmware_id >> 8) as u8);
            frame.push((firmware_id & 0xFF) as u8);
            count += 1;
        }
        frame.push(typ.hardware_version);
        let mut flagbits = if typ.activation { 0x02 } else { 0 };
        flagbits |= if typ.cc { 0x01 } else { 0 };
        frame.push(flagbits);

        frame
    }
}

#[derive(Clone, Copy, PartialEq, Debug)]
pub struct ZwaveFirmwareUpdateMdRequestGetType {
    pub manufacturer_id: u16,
    pub firmware_id: u16,
    pub firmware_checksum: u16,
    pub firmware_target: u8,
    pub fragment_size: u16,
    pub activation: bool,
    pub hardware_version: u8,
}

impl From<ZwaveFirmwareUpdateMdRequestGetType> for Vec<u8> {
    fn from(typ: ZwaveFirmwareUpdateMdRequestGetType) -> Self {
        let mut frame = vec![0; 13];
        frame[0] = COMMAND_CLASS_FIRMWARE_UPDATE_MD.try_into().unwrap();
        frame[1] = FIRMWARE_UPDATE_MD_REQUEST_GET.try_into().unwrap();
        frame[2] = (typ.manufacturer_id >> 8) as u8;
        frame[3] = (typ.manufacturer_id & 0xFF) as u8;
        frame[4] = (typ.firmware_id >> 8) as u8;
        frame[5] = (typ.firmware_id & 0xFF) as u8;
        frame[6] = (typ.firmware_checksum >> 8) as u8;
        frame[7] = (typ.firmware_checksum & 0xFF) as u8;
        frame[8] = typ.firmware_target;
        frame[9] = (typ.fragment_size >> 8) as u8;
        frame[10] = (typ.fragment_size & 0xFF) as u8;
        frame[11] = typ.activation as u8;
        frame[12] = typ.hardware_version;
        frame
    }
}

impl From<&[u8]> for ZwaveFirmwareUpdateMdRequestGetType {
    fn from(frame: &[u8]) -> Self {
        ZwaveFirmwareUpdateMdRequestGetType {
            manufacturer_id: try_u8slice_to_msb(frame.get(2..4)).unwrap_or(DEFAULT_MANUFACTURER_ID),
            firmware_id: try_u8slice_to_msb(frame.get(4..6)).unwrap_or(FIRMWARE_0_ID),
            firmware_checksum: try_u8slice_to_msb(frame.get(6..8)).unwrap_or(0),
            firmware_target: frame.get(8).cloned().unwrap_or(DEFAULT_FIRMWARE_TARGET),
            fragment_size: try_u8slice_to_msb(frame.get(9..11)).unwrap_or(0),
            activation: frame.get(11).cloned().unwrap_or(1) != 0,
            hardware_version: frame.get(12).cloned().unwrap_or(DEFAULT_HARDWARE_VERSION),
        }
    }
}

#[derive(Clone, Copy, PartialEq, Debug, Default)]
pub struct ZwaveFirmwareUpdateMdRequestReportType {
    pub status: u8,
}

impl From<&[u8]> for ZwaveFirmwareUpdateMdRequestReportType {
    fn from(frame: &[u8]) -> Self {
        ZwaveFirmwareUpdateMdRequestReportType {
            status: frame.get(2).cloned().unwrap_or_default(),
        }
    }
}

impl From<ZwaveFirmwareUpdateMdRequestReportType> for Vec<u8> {
    fn from(typ: ZwaveFirmwareUpdateMdRequestReportType) -> Self {
        vec![
            COMMAND_CLASS_FIRMWARE_UPDATE_MD.try_into().unwrap(),
            FIRMWARE_UPDATE_MD_REQUEST_REPORT.try_into().unwrap(),
            typ.status,
        ]
    }
}

#[derive(Clone, Copy, PartialEq, Debug, Default)]
pub struct ZwaveFirmwareUpdateMdGetType {
    pub number_of_reports: u8,
    pub report_number: u16,
}

impl From<&[u8]> for ZwaveFirmwareUpdateMdGetType {
    fn from(frame: &[u8]) -> Self {
        ZwaveFirmwareUpdateMdGetType {
            number_of_reports: frame.get(2).cloned().unwrap_or_default(),
            report_number: try_u8slice_to_msb(frame.get(3..5)).unwrap_or_default(),
        }
    }
}

#[derive(Clone, PartialEq, Debug)]
pub struct ZwaveFirmwareUpdateMdReportType {
    last: bool,
    report_number: u16,
    data: Vec<u8>,
    with_checksum: bool,
}

impl From<ZwaveFirmwareUpdateMdReportType> for Vec<u8> {
    fn from(typ: ZwaveFirmwareUpdateMdReportType) -> Self {
        let mut frame = vec![
            COMMAND_CLASS_FIRMWARE_UPDATE_MD.try_into().unwrap(),
            FIRMWARE_UPDATE_MD_REPORT.try_into().unwrap(),
            (typ.report_number >> 8) as u8,
            (typ.report_number & 0xFF) as u8,
        ];
        if typ.last {
            frame[2] |= 0x80;
        }
        // Image data
        for idx in 0..typ.data.len() {
            frame.push(typ.data[idx as usize]);
        }

        if typ.with_checksum {
            let (msb, lsb) = crc_16(&frame);
            frame.push(msb);
            frame.push(lsb);
        }
        frame
    }
}

impl ZwaveFirmwareUpdateMdReportType {
    pub fn new(last: bool, report_number: u16, data: Vec<u8>) -> ZwaveFirmwareUpdateMdReportType {
        ZwaveFirmwareUpdateMdReportType {
            last,
            report_number,
            data,
            with_checksum: false,
        }
    }

    pub fn new_with_checksum(
        last: bool,
        report_number: u16,
        data: Vec<u8>,
    ) -> ZwaveFirmwareUpdateMdReportType {
        ZwaveFirmwareUpdateMdReportType {
            last,
            report_number,
            data,
            with_checksum: true,
        }
    }

    pub fn abort(report_number: u16) -> ZwaveFirmwareUpdateMdReportType {
        ZwaveFirmwareUpdateMdReportType {
            last: true,
            report_number,
            data: Vec::new(),
            with_checksum: false,
        }
    }
}

fn crc_16(data: &[u8]) -> (u8, u8) {
    let checksum = crc16::State::<crc16::AUG_CCITT>::calculate(data);
    ((checksum >> 8) as u8, (checksum & 0xFF) as u8)
}

#[derive(Clone, Copy, PartialEq, Debug, Default)]
pub struct ZwaveFirmwareUpdateMdStatusReportType {
    pub status: u8,
    pub wait_time: u16,
}

impl From<&[u8]> for ZwaveFirmwareUpdateMdStatusReportType {
    fn from(frame: &[u8]) -> Self {
        ZwaveFirmwareUpdateMdStatusReportType {
            status: frame.get(2).cloned().unwrap_or_default(),
            wait_time: try_u8slice_to_msb(frame.get(3..5)).unwrap_or_default(),
        }
    }
}

#[derive(Clone, Copy, PartialEq, Debug)]
pub struct ZwaveFirmwareUpdateActivationSetType {
    pub manufacturer_id: u16,
    pub firmware_id: u16,
    pub firmware_checksum: u16,
    pub firmware_target: u8,
    pub hardware_version: u8,
}

impl From<&[u8]> for ZwaveFirmwareUpdateActivationSetType {
    fn from(frame: &[u8]) -> Self {
        ZwaveFirmwareUpdateActivationSetType {
            manufacturer_id: try_u8slice_to_msb(frame.get(2..4)).unwrap_or(0),
            firmware_id: try_u8slice_to_msb(frame.get(4..6)).unwrap_or(0),
            firmware_checksum: try_u8slice_to_msb(frame.get(6..8)).unwrap_or(0),
            firmware_target: frame.get(8).cloned().unwrap_or(0),
            hardware_version: frame.get(9).cloned().unwrap_or(0),
        }
    }
}

impl From<ZwaveFirmwareUpdateActivationSetType> for Vec<u8> {
    fn from(typ: ZwaveFirmwareUpdateActivationSetType) -> Self {
        vec![
            COMMAND_CLASS_FIRMWARE_UPDATE_MD.try_into().unwrap(),
            FIRMWARE_UPDATE_ACTIVATION_SET.try_into().unwrap(),
            (typ.manufacturer_id >> 8) as u8,
            (typ.manufacturer_id & 0xFF) as u8,
            (typ.firmware_id >> 8) as u8,
            (typ.firmware_id & 0xFF) as u8,
            (typ.firmware_checksum >> 8) as u8,
            (typ.firmware_checksum & 0xFF) as u8,
            typ.firmware_target,
            typ.hardware_version,
        ]
    }
}

#[derive(Clone, Copy, PartialEq, Debug, Default)]
pub struct ZwaveFirmwareUpdateActivationStatusReportType {
    pub manufacturer_id: u16,
    pub firmware_id: u16,
    pub firmware_checksum: u16,
    pub firmware_target: u8,
    pub firmware_update_status: u8,
    pub hardware_version: u8,
}

impl From<&[u8]> for ZwaveFirmwareUpdateActivationStatusReportType {
    fn from(frame: &[u8]) -> Self {
        ZwaveFirmwareUpdateActivationStatusReportType {
            manufacturer_id: try_u8slice_to_msb(frame.get(2..4)).unwrap_or_default(),
            firmware_id: try_u8slice_to_msb(frame.get(4..6)).unwrap_or_default(),
            firmware_checksum: try_u8slice_to_msb(frame.get(6..8)).unwrap_or_default(),
            firmware_target: frame.get(8).cloned().unwrap_or_default(),
            firmware_update_status: frame.get(9).cloned().unwrap_or_default(),
            hardware_version: frame.get(10).get_or_insert(&1).to_owned(),
        }
    }
}

impl From<ZwaveFirmwareUpdateActivationStatusReportType> for Vec<u8> {
    fn from(typ: ZwaveFirmwareUpdateActivationStatusReportType) -> Self {
        vec![
            COMMAND_CLASS_FIRMWARE_UPDATE_MD.try_into().unwrap(),
            FIRMWARE_UPDATE_ACTIVATION_STATUS_REPORT.try_into().unwrap(),
            (typ.manufacturer_id >> 8) as u8,
            (typ.manufacturer_id & 0xFF) as u8,
            (typ.firmware_id >> 8) as u8,
            (typ.firmware_id & 0xFF) as u8,
            (typ.firmware_checksum >> 8) as u8,
            (typ.firmware_checksum & 0xFF) as u8,
            typ.firmware_target,
            typ.firmware_update_status,
            typ.hardware_version,
        ]
    }
}

/// Firmware transfer state value
#[repr(u32)]
#[derive(serde::Serialize, serde::Deserialize, PartialEq, Clone, Copy, Debug)]
pub enum ZwaveFirmwareTransferState {
    // No Firmware transfer is ongoing
    IDLE = 0,
    // A Firmware transfer is ongoing
    ONGOING = 1,
    // A Firmware transfer is completed, wait for a subsequent activation
    WAITING_FOR_ACTIVATION = 2,
    // A Firmware transfer is completed, waiting for the node to reboot
    WAITING_FOR_REBOOT = 3,
}

#[derive(PartialEq, Clone, Copy)]
#[repr(C)]
pub enum ZwaveFirmwareTransferStatus {
    SUCCESS = 0,
    ABORTED = 1,
    NOT_AUTHORIZED = 2,
    INVALID_IMAGE = 3,
    REQUIRE_MORE_IMAGE = 4,
    NOT_SUPPORTED = 5,
}

macro_rules! DEFINE_ATTRIBUTE {
    ($name:ident, $value:expr) => {
        pub const $name: u32 = $value as u32;
    };
}

// TODO: Generate attribute rust bindings, and remove duplicated code
DEFINE_ATTRIBUTE!(ATTRIBUTE_NODE_ID, 0x0003);
DEFINE_ATTRIBUTE!(ATTRIBUTE_ENDPOINT_ID, 0x0004);
DEFINE_ATTRIBUTE!(ATTRIBUTE_ZWAVE_NIF, 0x0101);

DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_VERSION,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x01)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x02)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x03)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW_ID,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x04)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW_UPGRADABLE,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x05)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_MANUFACTURER_ID,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x06)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FRAGMENT_SIZE,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x07)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_ACTIVATION,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x08)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_CC,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x09)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x0A)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_TARGET,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x0B)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_APPLY_TIMESTAMP,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x0C)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_FILENAME,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x0D)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_OFFSET,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x0E)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_SIZE,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x0F)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_LAST_STATUS,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x10)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_EXPIRY_TIME,
    ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x11)
);
DEFINE_ATTRIBUTE!(
    ATTRIBUTE_CC_VERSION_HARDWARE_VERSION,
    ((COMMAND_CLASS_VERSION << 8) | 0x0E)
);

DEFINE_ATTRIBUTE!(
    ATTRIBUTE_CC_VERSION_VERSION_REPORT_DATA,
    ((COMMAND_CLASS_VERSION << 8) | 0x02)
);

extern "C" {
    pub fn postpone_firmware_update_expiry(
        endpoint_node: unify_attribute_store_sys::attribute_store_node_t,
    ) -> sl_status_t;

    pub fn node_supports_firmware_activation(
        ep_node: unify_attribute_store_sys::attribute_store_node_t,
    ) -> bool;

    pub fn node_supports_cc(ep_node: unify_attribute_store_sys::attribute_store_node_t) -> bool;

    pub fn is_firmware_update_ongoing(
        ep_node: unify_attribute_store_sys::attribute_store_node_t,
    ) -> bool;

    pub fn firmware_transfer_get_theoretical_max_fragment_size(
        ep_node: unify_attribute_store_sys::attribute_store_node_t,
    ) -> u16;

    pub fn set_firmware_transfer_last_status(
        firmware_transfer_node: unify_attribute_store_sys::attribute_store_node_t,
        status: ZwaveFirmwareTransferStatus,
    ) -> sl_status_t;
}
