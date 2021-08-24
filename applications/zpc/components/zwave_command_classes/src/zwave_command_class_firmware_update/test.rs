#[cfg(test)]
use super::*;

#[test]
fn test_generate_zw_firmware_md_request_get() {
    let frame = ZwaveFirmwareMdRequestGetFrame {
        header: ZwaveCommandClassFrameHeader {
            command_class: 0x84,
            command: 0x03,
        },
        manufacturer_id: 300,
        firmware_id: 1,
        firmware_checksum: 1234,
        firmware_target: 2,
        fragment_size: 1000,
        activation: true,
        hardware_version: 1,
    };
    let mut generated_frame: [u8; 20] = [0; 20];
    let size = frame.to_frame_buffer(&mut generated_frame);
    assert_eq!(size, 13);
    let answer: [u8; 20] = [
        0x84, 0x03, 0x01, 0x2C, 0x00, 0x01, 0x04, 0xD2, 0x02, 0x03, 0xE8, 0x01, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
    ];
    assert_eq!(answer, generated_frame);
}

#[test]
fn test_parsing_zw_firmware_md_request_report() {
    let frame = [
        COMMAND_CLASS_FIRMWARE_UPDATE_MD as u8,
        FIRMWARE_UPDATE_MD_REQUEST_REPORT as u8,
        0xff,
    ];
    let parsed_frame = ZwaveFirmwareMdRequestReport::from(frame.as_ref());
    match parsed_frame {
        ZwaveFirmwareMdRequestReport::ZwaveFirmwareMdRequestReportFrame(frame) => {
            assert_eq!(
                frame.header,
                ZwaveCommandClassFrameHeader {
                    command_class: COMMAND_CLASS_FIRMWARE_UPDATE_MD,
                    command: FIRMWARE_UPDATE_MD_REQUEST_REPORT
                }
            );
            assert_eq!(frame.status, 0xff);
        }
        _ => {
            assert!(false, "parsed_frame shouldn't be None")
        }
    }
}

#[test]
fn test_generate_zw_firmware_update_activation_set() {
    let frame = ZwaveFirmwareUpdateActivationSetFrame {
        header: ZwaveCommandClassFrameHeader {
            command_class: 0x7A,
            command: 0x08,
        },
        manufacture_id: 0x0402,
        firmware_id: 0x01,
        firmware_checksum: 0xABCD,
        firmware_target: 0x02,
        hardware_version: 0x01,
    };
    let mut generated_frame: [u8; 20] = [0; 20];
    let size = frame.to_frame_buffer(&mut generated_frame);
    assert_eq!(size, 10);
    let answer: [u8; 10] = [0x7A, 0x08, 0x04, 0x02, 0x00, 0x01, 0xAB, 0xCD, 0x02, 0x01];
    assert_eq!(answer, generated_frame[0..10]);
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
    let parsed_frame = ZwaveFirmwareUpdateActivationStatusReport::from(frame.as_ref());
    match parsed_frame {
      ZwaveFirmwareUpdateActivationStatusReport::ZwaveFirmwareUpdateActivationStatusReportFrame(frame) => {
            assert_eq!(
                frame.header,
                ZwaveCommandClassFrameHeader {
                    command_class: COMMAND_CLASS_FIRMWARE_UPDATE_MD,
                    command: FIRMWARE_UPDATE_ACTIVATION_STATUS_REPORT
                }
            );
            assert_eq!(frame.manufacture_id, 0x0402);
            assert_eq!(frame.firmware_id, 0x01);
            assert_eq!(frame.firmware_checksum, 0xABCD);
            assert_eq!(frame.firmware_target, 0x01);
            assert_eq!(frame.firmware_update_status, 0xFF);
            assert_eq!(frame.hardware_version, 0x01);
        }
        _ => {
            assert!(false, "parsed_frame shouldn't be None")
        }
    }
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
    let parsed_frame = ZwaveFirmwareUpdateActivationStatusReport::from(frame.as_ref());
    match parsed_frame {
      ZwaveFirmwareUpdateActivationStatusReport::ZwaveFirmwareUpdateActivationStatusReportFrame(frame) => {
            assert_eq!(
                frame.header,
                ZwaveCommandClassFrameHeader {
                    command_class: COMMAND_CLASS_FIRMWARE_UPDATE_MD,
                    command: FIRMWARE_UPDATE_ACTIVATION_STATUS_REPORT
                }
            );
            assert_eq!(frame.manufacture_id, 0x0402);
            assert_eq!(frame.firmware_id, 0x01);
            assert_eq!(frame.firmware_checksum, 0xABCD);
            assert_eq!(frame.firmware_target, 0x01);
            assert_eq!(frame.firmware_update_status, 0xFF);
            assert_eq!(frame.hardware_version, 0x01);
        }
        _ => {
            assert!(false, "parsed_frame shouldn't be None")
        }
    }
}

#[test]
fn test_generate_zw_firmware_update_md_report() {
    let mut frame = ZwaveFirmwareUpdateMdReportFrame {
        header: ZwaveCommandClassFrameHeader {
            command_class: 0x7A,
            command: 0x06,
        },
        last: false,
        report_number: 112,
        data: [0; MAX_FRAME_LEN as usize],
        data_length: 2,
        checksum: 42,
    };
    frame.data[0] = 0x01;
    frame.data[1] = 0x02;
    let mut generated_frame: [u8; 20] = [0; 20];
    let size = frame.to_frame_buffer(&mut generated_frame);
    assert_eq!(size, 8);
    let answer: [u8; 8] = [0x7A, 0x06, 0x00, 0x70, 0x01, 0x02, 0x00, 0x2A];
    assert_eq!(answer, generated_frame[0..8]);
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
    let parsed_frame = ZwaveFirmwareUpdateMdGet::from(frame.as_ref());
    match parsed_frame {
        ZwaveFirmwareUpdateMdGet::ZwaveFirmwareUpdateMdGetFrame(frame) => {
            assert_eq!(
                frame.header,
                ZwaveCommandClassFrameHeader {
                    command_class: COMMAND_CLASS_FIRMWARE_UPDATE_MD,
                    command: FIRMWARE_UPDATE_MD_GET
                }
            );
            assert_eq!(frame.number_of_reports, 3);
            assert_eq!(frame.report_number, 2);
        }
        _ => {
            assert!(false, "parsed_frame shouldn't be None")
        }
    }
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
    let parsed_frame = ZwaveFirmwareMdReport::from(frame.as_ref());
    match parsed_frame {
        ZwaveFirmwareMdReport::ZwaveFirmwareMdReportFrame(frame) => {
            assert_eq!(
                frame.header,
                ZwaveCommandClassFrameHeader {
                    command_class: COMMAND_CLASS_FIRMWARE_UPDATE_MD,
                    command: FIRMWARE_MD_REPORT
                }
            );
            assert_eq!(frame.manufacturer_id, 1 << 8 | 8);
            assert_eq!(frame.firmware_0_id, 2 << 8 | 4);
            assert_eq!(frame.firmware_0_checksum, 5 << 8 | 7);
            assert_eq!(frame.firmware_upgradeable, 1);
            assert_eq!(frame.number_of_firmware_targets, 0);
            assert_eq!(frame.max_fragment_size, 500);
            assert_eq!(frame.firmware_n_id.len(), 0);
            assert_eq!(frame.hardware_version, 0);
            assert_eq!(frame.activation, false);
            assert_eq!(frame.cc, false);
        }
        _ => {
            assert!(false, "parsed_frame shouldn't be None")
        }
    }
}

#[test]
fn test_parsing_adc_t3000_md_report() {
    // Frame from Zniffer for adc-t3000 thermostat
    // FWU CC Version is 3
    let frame: [u8; 16] = [
        0x7A, 0x02, 0x01, 0x90, 0x06, 0x00, 0x09, 0x63, 0xFF, 0x02, 0x00, 0x14, 0x06, 0x01, 0x06,
        0x02,
    ];
    if let ZwaveFirmwareMdReport::ZwaveFirmwareMdReportFrame(f) =
        ZwaveFirmwareMdReport::from(frame.as_ref())
    {
        assert_eq!(0x0190, f.manufacturer_id);
        assert_eq!(0x0600, f.firmware_0_id);
        assert_eq!(0x0963, f.firmware_0_checksum);
        assert_eq!(0xff, f.firmware_upgradeable);
        assert_eq!(0x02, f.number_of_firmware_targets);
        assert_eq!(0x0601, f.firmware_n_id[0]);
        assert_eq!(0x0602, f.firmware_n_id[1]);
    } else {
        assert!(false, "Expected to parse frame successful");
    }
}

#[test]
fn test_conversion_enum() {
    assert_eq!(
        ZwaveFirmwareTransferState::from_bytes(&vec![2]).unwrap(),
        ZwaveFirmwareTransferState::WAITING_FOR_ACTIVATION
    );
    assert!(ZwaveFirmwareTransferState::from_bytes(&vec![99]).is_err());
}
