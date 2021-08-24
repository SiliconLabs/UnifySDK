use num_derive::FromPrimitive;
use num_traits::FromPrimitive;
use uic_attribute_store::ByteArray;
use zwave_controller::sl_status_t;

pub const MAX_FRAME_LEN: usize = 255;
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
pub const VERSION: u8 = 0x07;
pub const SL_STATUS_FAIL: u32 = 0x01;
pub const SL_STATUS_OK: u32 = 0x00;
pub const SL_STATUS_NOT_SUPPORTED: u32 = 0xF;
pub const SL_STATUS_ALREADY_EXISTS: u32 = 0x002E;
pub const SL_STATUS_IS_WAITING: u32 = 0x000B;

pub const MAX_REPEATERS: u32 = 4;
pub const LAST_USED_ROUTE_CONF_SIZE: u32 = 1;
pub const LAST_USED_ROUTE_SIZE: u32 = MAX_REPEATERS + LAST_USED_ROUTE_CONF_SIZE;
pub const ZWAVE_TX_QOS_RECOMMENDED_FWU_TRANSFER_PRIORITY: u32 = 0x000000FF;
// Default values for ZwaveFirmwareMdReport
pub const DEFAULT_FIRMWARE_UPGRADABLE: u8 = 0xff;
pub const DEFAULT_NUMBER_OF_FIRMWARE_TARGETS: u8 = 0;
pub const DEFAULT_HARDWARE_VERSION: u8 = 0;
pub const DEFAULT_ACTIVATION: bool = false;
pub const DEFAULT_WAIT_TIME: u16 = 10;
pub const DEFAULT_CC: bool = false;

#[derive(PartialEq, Debug)]
pub struct ZwaveCommandClassFrameHeader {
    pub command_class: u16,
    pub command: u16,
}

pub struct ZwaveFirmwareMdReportFrame {
    pub header: ZwaveCommandClassFrameHeader,
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

pub enum ZwaveFirmwareMdReport {
    ZwaveFirmwareMdReportFrame(ZwaveFirmwareMdReportFrame),
    None,
}

pub struct ZwaveFirmwareMdRequestGetFrame {
    pub header: ZwaveCommandClassFrameHeader,
    pub manufacturer_id: u16,
    pub firmware_id: u16,
    pub firmware_checksum: u16,
    pub firmware_target: u8,
    pub fragment_size: u16,
    pub activation: bool,
    pub hardware_version: u8,
}

pub struct ZwaveFirmwareMdRequestReportFrame {
    pub header: ZwaveCommandClassFrameHeader,
    pub status: u8,
}
pub enum ZwaveFirmwareMdRequestReport {
    ZwaveFirmwareMdRequestReportFrame(ZwaveFirmwareMdRequestReportFrame),
    None,
}

pub struct ZwaveFirmwareUpdateMdGetFrame {
    pub header: ZwaveCommandClassFrameHeader,
    pub number_of_reports: u8,
    pub report_number: u16,
}
pub enum ZwaveFirmwareUpdateMdGet {
    ZwaveFirmwareUpdateMdGetFrame(ZwaveFirmwareUpdateMdGetFrame),
    None,
}

pub struct ZwaveFirmwareUpdateMdReportFrame {
    pub header: ZwaveCommandClassFrameHeader,
    pub last: bool,
    pub report_number: u16,
    pub data: [u8; MAX_FRAME_LEN as usize],
    pub data_length: u16,
    pub checksum: u16,
}

pub struct ZwaveFirmwareUpdateMdStatusReportFrame {
    pub header: ZwaveCommandClassFrameHeader,
    pub status: u8,
    pub wait_time: u16,
}

pub enum ZwaveFirmwareUpdateMdStatusReport {
    ZwaveFirmwareUpdateMdStatusReportFrame(ZwaveFirmwareUpdateMdStatusReportFrame),
    None,
}

pub struct ZwaveFirmwareUpdateActivationSetFrame {
    pub header: ZwaveCommandClassFrameHeader,
    pub manufacture_id: u16,
    pub firmware_id: u16,
    pub firmware_checksum: u16,
    pub firmware_target: u8,
    pub hardware_version: u8,
}
pub enum ZwaveFirmwareUpdateActivationSet {
    ZwaveFirmwareUpdateActivationSetFrame(ZwaveFirmwareUpdateActivationSetFrame),
    None,
}

pub struct ZwaveFirmwareUpdateActivationStatusReportFrame {
    pub header: ZwaveCommandClassFrameHeader,
    pub manufacture_id: u16,
    pub firmware_id: u16,
    pub firmware_checksum: u16,
    pub firmware_target: u8,
    pub firmware_update_status: u8,
    pub hardware_version: u8,
}

pub enum ZwaveFirmwareUpdateActivationStatusReport {
    ZwaveFirmwareUpdateActivationStatusReportFrame(ZwaveFirmwareUpdateActivationStatusReportFrame),
    None,
}

/// Firmware transfer state value
#[derive(FromPrimitive, PartialEq, Clone, Copy, Debug)]
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

impl ByteArray for ZwaveFirmwareTransferState {
    type Error = ();

    fn to_bytes(self) -> Vec<u8> {
        vec![self as u8]
    }

    fn from_bytes(bytes: &[u8]) -> Result<Self, Self::Error> {
        if bytes.len() == 0 {
            Err(())
        } else {
            FromPrimitive::from_u8(bytes[0]).ok_or(())
        }
    }
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
        endpoint_node: uic_attribute_store::attribute_store_node_t,
    ) -> sl_status_t;
}

extern "C" {
    pub fn node_supports_activation(ep_node: uic_attribute_store::attribute_store_node_t) -> bool;
}

extern "C" {
    pub fn is_firmware_update_ongoing(ep_node: uic_attribute_store::attribute_store_node_t)
        -> bool;
}

extern "C" {
    pub fn get_theoretical_max_fragment_size(
        ep_node: uic_attribute_store::attribute_store_node_t,
    ) -> u16;
}

extern "C" {
    pub fn set_firmware_transfer_last_status(
        firmware_transfer_node: uic_attribute_store::attribute_store_node_t,
        status: ZwaveFirmwareTransferStatus,
    ) -> sl_status_t;
}
