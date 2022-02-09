use super::*;
use crate::ZwaveFrameError::*;
use std::convert::TryFrom;

#[test]
fn test_invalid_frames() {
    let raw: Vec<u8> = vec![0x0, 0x0, 0x0, 0x0, 0x0];
    assert_eq!(
        Err(ExpectedOtherCommandClass {
            have_cc: 0,
            expected_cc: 122
        }),
        ZwaveFirmwareUpdateFrame::try_from(raw.as_slice())
    );

    let raw: Vec<u8> = vec![0x3, 0x3];
    assert_eq!(
        Err(ExpectedOtherCommandClass {
            have_cc: 3,
            expected_cc: 122
        }),
        ZwaveFirmwareUpdateFrame::try_from(raw.as_slice())
    );
}

#[test]
fn test_generate_zw_firmware_md_request_get() {
    use ZwaveFirmwareUpdateFrame::*;
    let frame: Vec<u8> = ZwaveFirmwareUpdateMdRequestGetType {
        manufacturer_id: 300,
        firmware_id: 1,
        firmware_checksum: 1234,
        firmware_target: 2,
        fragment_size: 1000,
        activation: true,
        hardware_version: 1,
    }
    .into();

    assert_eq!(frame.len(), 13);
    let answer: Vec<u8> = vec![
        0x7A, 0x03, 0x01, 0x2C, 0x00, 0x01, 0x04, 0xD2, 0x02, 0x03, 0xE8, 0x01, 0x01,
    ];
    assert_eq!(answer, frame);

    let parsed_frame = ZwaveFirmwareUpdateFrame::try_from(answer.as_ref());
    assert_eq!(
        parsed_frame,
        Ok(ZwaveFirmwareUpdateMdRequestGet(
            ZwaveFirmwareUpdateMdRequestGetType {
                manufacturer_id: 300,
                firmware_id: 1,
                firmware_checksum: 1234,
                firmware_target: 2,
                fragment_size: 1000,
                activation: true,
                hardware_version: 1
            }
        ))
    );
}

#[test]
fn test_parsing_zw_firmware_md_request_report() {
    use ZwaveFirmwareUpdateFrame::*;

    let frame = [
        COMMAND_CLASS_FIRMWARE_UPDATE_MD as u8,
        FIRMWARE_UPDATE_MD_REQUEST_REPORT as u8,
        0xff,
    ];
    let parsed_frame = ZwaveFirmwareUpdateFrame::try_from(frame.as_ref());
    assert_eq!(
        parsed_frame,
        Ok(ZwaveFirmwareUpdateMdRequestReport(
            ZwaveFirmwareUpdateMdRequestReportType { status: 255 }
        ))
    );
}

#[test]
fn test_generate_zw_firmware_update_activation_set() {
    use ZwaveFirmwareUpdateFrame::*;
    let frame: Vec<u8> = ZwaveFirmwareUpdateActivationSetType {
        manufacturer_id: 0x0402,
        firmware_id: 0x01,
        firmware_checksum: 0xABCD,
        firmware_target: 0x02,
        hardware_version: 0x01,
    }
    .into();

    assert_eq!(frame.len(), 10);
    let answer: [u8; 10] = [0x7A, 0x08, 0x04, 0x02, 0x00, 0x01, 0xAB, 0xCD, 0x02, 0x01];
    assert_eq!(answer, frame.as_slice());

    let parsed_frame = ZwaveFirmwareUpdateFrame::try_from(answer.as_ref());
    assert_eq!(
        parsed_frame,
        Ok(ZwaveFirmwareUpdateActivationSet(
            ZwaveFirmwareUpdateActivationSetType {
                manufacturer_id: 0x0402,
                firmware_id: 0x01,
                firmware_checksum: 0xABCD,
                firmware_target: 0x02,
                hardware_version: 0x01,
            }
        ))
    );
}

#[test]
fn test_parsing_zw_firmware_update_activation_status_report() {
    let frame = [
        COMMAND_CLASS_FIRMWARE_UPDATE_MD as u8,
        FIRMWARE_UPDATE_ACTIVATION_STATUS_REPORT as u8,
        0x04,
        0x02,
        0x00,
        0x01,
        0xAB,
        0xCD,
        0x01,
        0xFF,
        0x01,
    ];
    let parsed_frame = ZwaveFirmwareUpdateFrame::try_from(frame.as_ref());

    assert_eq!(
        parsed_frame,
        Ok(
            ZwaveFirmwareUpdateFrame::ZwaveFirmwareUpdateActivationStatusReport(
                ZwaveFirmwareUpdateActivationStatusReportType {
                    manufacturer_id: 0x0402,
                    firmware_id: 0x01,
                    firmware_checksum: 0xABCD,
                    firmware_target: 0x01,
                    firmware_update_status: 0xFF,
                    hardware_version: 0x01
                }
            )
        )
    );
}

#[test]
fn test_generate_zw_firmware_update_md_prepare_get() {
    use ZwaveFirmwareUpdateFrame::*;
    let frame: [u8; 10] = [0x7A, 0x0A, 0x04, 0x02, 0x00, 0x01, 0x02, 0x00, 0x1C, 0x01];

    let parsed_frame = ZwaveFirmwareUpdateFrame::try_from(frame.as_ref());
    assert_eq!(
        parsed_frame,
        Ok(ZwaveFirmwareUpdateMdPrepareGet(
            ZwaveFirmwareUpdateMdPrepareGetType {
                manufacturer_id: 0x0402,
                firmware_id: 0x01,
                firmware_target: 0x02,
                fragment_size: 0x1C,
                hardware_version: 0x01,
            }
        ))
    );
}

#[test]
fn test_generate_zw_firmware_update_md_prepare_report() {
    let frame: Vec<u8> = ZwaveFirmwareUpdateMdPrepareReportType {
        status: 0x03,
        firmware_checksum: 0xAA,
    }
    .into();

    assert_eq!(frame.len(), 5);
    let answer: [u8; 5] = [0x7A, 0x0B, 0x03, 0x00, 0xAA];
    assert_eq!(answer, frame.as_slice());
}

#[test]
fn test_parsing_zw_firmware_update_activation_status_report_v4() {
    let frame = [
        COMMAND_CLASS_FIRMWARE_UPDATE_MD as u8,
        FIRMWARE_UPDATE_ACTIVATION_STATUS_REPORT as u8,
        0x04,
        0x02,
        0x00,
        0x01,
        0xAB,
        0xCD,
        0x01,
        0xFF,
    ];
    let parsed_frame = ZwaveFirmwareUpdateFrame::try_from(frame.as_ref());

    assert_eq!(
        parsed_frame,
        Ok(
            ZwaveFirmwareUpdateFrame::ZwaveFirmwareUpdateActivationStatusReport(
                ZwaveFirmwareUpdateActivationStatusReportType {
                    manufacturer_id: 0x0402,
                    firmware_id: 0x01,
                    firmware_checksum: 0xABCD,
                    firmware_target: 0x01,
                    firmware_update_status: 0xFF,
                    hardware_version: 0x01
                }
            )
        )
    );
}

#[test]
fn test_generate_zw_firmware_update_md_report() {
    let frame: Vec<u8> =
        ZwaveFirmwareUpdateMdReportType::new_with_checksum(false, 112, vec![0x01, 0x02]).into();

    assert_eq!(frame.len(), 8);
    let answer: [u8; 8] = [0x7A, 0x06, 0x00, 0x70, 0x01, 0x02, 0xF4, 0x5E];
    assert_eq!(answer, frame.as_slice());
}

#[test]
fn test_parsing_zw_firmware_update_md_get() {
    // TODO: Add additional parsing tests
    let frame = [
        COMMAND_CLASS_FIRMWARE_UPDATE_MD as u8,
        FIRMWARE_UPDATE_MD_GET as u8,
        3,
        0,
        2,
    ];
    let parsed_frame = ZwaveFirmwareUpdateFrame::try_from(frame.as_ref());
    assert_eq!(
        parsed_frame,
        Ok(ZwaveFirmwareUpdateFrame::ZwaveFirmwareUpdateMdGet(
            ZwaveFirmwareUpdateMdGetType {
                number_of_reports: 3,
                report_number: 2,
            }
        ))
    )
}

#[test]
fn test_parsing_zw_firmware_md_report() {
    // TODO: Add additional parsing tests
    let frame = [
        COMMAND_CLASS_FIRMWARE_UPDATE_MD as u8,
        FIRMWARE_MD_REPORT as u8,
        1,
        8,
        2,
        4,
        5,
        7,
        1,
    ];
    let parsed_frame = ZwaveFirmwareUpdateFrame::try_from(frame.as_ref());
    assert_eq!(
        parsed_frame,
        Ok(ZwaveFirmwareUpdateFrame::ZwaveFirmwareMdReport(
            ZwaveFirmwareMdReportType {
                manufacturer_id: 1 << 8 | 8,
                firmware_0_id: 2 << 8 | 4,
                firmware_0_checksum: 5 << 8 | 7,
                firmware_upgradeable: 1,
                number_of_firmware_targets: 0,
                max_fragment_size: 255,
                firmware_n_id: Vec::new(),
                hardware_version: DEFAULT_HARDWARE_VERSION,
                activation: false,
                cc: false,
            }
        ))
    );
}

#[test]
fn test_parsing_adc_t3000_md_report() {
    // Frame from Zniffer for adc-t3000 thermostat
    // FWU CC Version is 3
    let frame: [u8; 16] = [
        0x7A, 0x02, 0x01, 0x90, 0x06, 0x00, 0x09, 0x63, 0xFF, 0x02, 0x00, 0x14, 0x06, 0x01, 0x06,
        0x02,
    ];
    let parsed_frame = ZwaveFirmwareUpdateFrame::try_from(frame.as_ref());
    assert_eq!(
        parsed_frame,
        Ok(ZwaveFirmwareUpdateFrame::ZwaveFirmwareMdReport(
            ZwaveFirmwareMdReportType {
                manufacturer_id: 0x0190,
                firmware_0_id: 0x0600,
                firmware_0_checksum: 0x0963,
                firmware_upgradeable: 0xff,
                number_of_firmware_targets: 0x02,
                max_fragment_size: 20,
                firmware_n_id: vec![0x0601, 0x0602],
                hardware_version: DEFAULT_HARDWARE_VERSION,
                ..Default::default()
            }
        ))
    );
}
