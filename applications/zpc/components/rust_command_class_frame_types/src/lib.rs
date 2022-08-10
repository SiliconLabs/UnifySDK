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

#![doc(html_no_source)]
pub mod frameparser;

include!(concat!(env!("OUT_DIR"), "/lib.rs"));

#[cfg(test)]
mod tests {
    use crate::command_class_battery::*;
    use crate::command_class_node_naming::*;
    use crate::command_class_switch_color::*;
    use std::convert::TryFrom;
    #[test]
    fn command_with_no_parameters() {
        let f = SwitchColorSupportedGetFrame {};
        assert_eq!(&vec![0x33u8, 0x01u8], FrameParser::from(f).bytes());
    }

    #[test]
    fn command_with_byte_parameter() {
        let f = SwitchColorGetFrame {
            color_component_id: 2,
        };
        assert_eq!(&vec![0x33, 0x03, 0x02], FrameParser::from(f).bytes());
    }

    #[test]
    fn command_with_enum_parameter() {
        let f = SwitchColorReportFrame {
            color_component_id: 2,
            current_value: 3,
            target_value: 4,
            duration: SwitchColorReportDurationEnum::UnknownDuration,
        };
        assert_eq!(
            &vec![0x33, 0x04, 0x02, 0x03, 0x04, 0xFE],
            FrameParser::from(f).bytes()
        );
    }

    #[test]
    fn command_with_bitfield_and_variant() {
        let f = SwitchColorSetFrame {
            properties1: SwitchColorSetProperties1Bits {
                color_component_count: 0,
                reserved: 0,
            },
            vg1: vec![
                SwitchColorSetVg1Vg {
                    color_component_id: 1,
                    value: 0x11,
                },
                SwitchColorSetVg1Vg {
                    color_component_id: 2,
                    value: 0x22,
                },
            ],
            duration: SwitchColorSetDurationEnum::Default,
        };
        let mut frame = FrameParser::from(f);
        assert_eq!(
            &vec![0x33, 0x05, 0x02, 0x01, 0x11, 0x02, 0x22, 0xff],
            frame.bytes()
        );

        let f2 = CommandClassSwitchColorFrame::try_from(&mut frame);
        match f2 {
            Ok(r) => match r {
                CommandClassSwitchColorFrame::SwitchColorSetFrame(scsf) => {
                    let frame2 = FrameParser::from(scsf);
                    assert_eq!(frame2.bytes(), frame.bytes())
                }
                _ => assert_eq!(true, false),
            },
            Err(_e) => assert_eq!(true, false),
        };
    }

    #[test]
    fn command_with_field_enum() {
        let f = BatteryReportFrame {
            battery_level: 0xaa,
            properties1: BatteryReportProperties1Bits {
                replace_recharge: 3,
                low_fluid: false,
                overheating: false,
                backup_battery: true,
                rechargeable: true,
                charging_status: BatteryReportChargingStatusEnum::Charging,
            },
            properties2: BatteryReportProperties2Bits {
                disconnected: true,
                low_temperature_status: false,
                reserved1: 0,
            },
        };

        let mut frame = FrameParser::from(f);
        assert_eq!(&vec![0x80, 0x03, 0xaa, 0x73, 0x1], frame.bytes());

        let f2 = CommandClassBatteryFrame::try_from(&mut frame);
        match f2 {
            Ok(r) => match r {
                CommandClassBatteryFrame::BatteryReportFrame(scsf) => {
                    let frame2 = FrameParser::from(scsf);
                    assert_eq!(frame2.bytes(), frame.bytes())
                }
                _ => assert_eq!(true, false),
            },
            Err(_e) => assert_eq!(true, false),
        };
    }

    #[test]
    fn command_with_array() {
        let f = NodeNamingNodeLocationReportFrame {
            level: NodeNamingNodeLocationReportLevelBits {
                char_presentation: 0x3,
                reserved: 0x0,
            },
            node_location_char: [
                0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
                0x33, 0x33,
            ],
        };
        let mut frame = FrameParser::from(f);
        assert_eq!(
            &vec![
                0x77, 0x06, 0x3, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
                0x33, 0x33, 0x33, 0x33, 0x33
            ],
            frame.bytes()
        );

        let f2 = CommandClassNodeNamingFrame::try_from(&mut frame);
        match f2 {
            Ok(r) => match r {
                CommandClassNodeNamingFrame::NodeNamingNodeLocationReportFrame(scsf) => {
                    let frame2 = FrameParser::from(scsf);
                    assert_eq!(frame2.bytes(), frame.bytes())
                }
                _ => assert_eq!(true, false),
            },
            Err(_e) => assert_eq!(true, false),
        };
    }
}
