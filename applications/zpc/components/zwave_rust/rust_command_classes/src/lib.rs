// License
// <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

#![allow(
    non_snake_case,
    unused_variables,
    non_camel_case_types,
    non_upper_case_globals,
    dead_code
)]

extern crate zwave_rust_proc_macros;

mod zwave_command_class_firmware_update;
use std::ffi::c_void;
pub use zwave_command_class_firmware_update::*;

use ::zwave_tx::*;
use zwave_command_handler::*;
use zwave_controller::sl_status_t;
use zwave_rust_proc_macros::initialize_zwave_command_classes;

// TODO remove this when we have it generated
pub const SL_STATUS_FAIL: u32 = 0x01;
pub const SL_STATUS_OK: u32 = 0x00;
pub const SL_STATUS_NOT_SUPPORTED: u32 = 0xF;
pub const SL_STATUS_ALREADY_EXISTS: u32 = 0x002E;
pub const SL_STATUS_IS_WAITING: u32 = 0x000B;
pub const MAX_FRAME_LEN: usize = 255;

pub struct ZwaveHandlerConfig {
    pub minimal_scheme: zwave_controller_encapsulation_scheme_t,
    pub command_class: u16,
    pub command_class_name: &'static str,
    pub comments: &'static str,
    pub version: u8,
    pub manual_security_validation: bool,
}

initialize_zwave_command_classes!();

/// This is the base type for all command class frames.
/// It is used in context of converting raw byte data into
/// Typed zwave frames. see [zwave_command_class!]
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

pub fn zwave_command_class_send_report<T>(
    info: &zwave_controller::zwave_controller_connection_info,
    data: T,
) -> sl_status_t
where
    T: Into<Vec<u8>>,
{
    let tx_info = info.clone();
    let parent_session_id: *mut c_void = std::ptr::null_mut();
    let qos_for_frame: u32 = ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY;

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

    let bytes: Vec<u8> = data.into();
    zwave_tx::send_data(
        tx_info as zwave_tx::zwave_controller_connection_info,
        &bytes,
        tx_options,
        None,
        user,
        session,
    );
    SL_STATUS_OK
}

#[cfg(test)]
static mut CALLED_REGISTER: bool = false;

#[cfg(test)]
unsafe extern "C" fn zwave_command_handler_register_handler(
    handler: zwave_command_handler_t,
) -> sl_status_t {
    use std::ffi::CStr;

    assert_eq!(handler.minimal_scheme, zwave_controller::zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_NONE);
    assert_eq!(handler.command_class, 4);
    assert_eq!(handler.version, 5);
    assert_eq!(handler.manual_security_validation, true);
    assert_eq!(
        CStr::from_ptr(handler.comments).to_str().unwrap(),
        "TEST COMMENT"
    );
    assert_eq!(
        CStr::from_ptr(handler.command_class_name).to_str().unwrap(),
        "TEST CONFIG"
    );

    CALLED_REGISTER = true;
    0
}

#[cfg(test)]
mod tests {
    use super::*;
    use zwave_rust_proc_macros::zwave_command_class;

    const CONFIG: ZwaveHandlerConfig = ZwaveHandlerConfig {
        minimal_scheme: zwave_controller::zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
        command_class: 4,
        command_class_name: "TEST CONFIG\0",
        comments: "TEST COMMENT\0",
        version: 5,
        manual_security_validation: true,
    };

    #[zwave_command_class(CONFIG)]
    #[derive(Default)]
    struct TestHandler {
        pub control_data: Vec<u8>,
        pub support_data: Vec<u8>,
    }

    impl TestHandler {
        pub fn on_control(
            &mut self,
            info: zwave_controller::zwave_controller_connection_info,
            frame: &[u8],
        ) -> sl_status_t {
            self.control_data = Vec::from(frame);
            0
        }

        pub fn on_support(
            &mut self,
            info: zwave_controller::zwave_controller_connection_info,
            frame: &[u8],
        ) -> sl_status_t {
            self.support_data = Vec::from(frame);
            0
        }
        pub fn on_init(&mut self, status: sl_status_t) -> sl_status_t {
            status
        }
    }

    #[test]
    #[ignore]
    fn test_zwave_command_class_macro_config() {
        unsafe {
            CALLED_REGISTER = false;
            // zwave_command_class_init_rust_handlers function calls
            // test_handler_init() calls the actual zwave_command_handler_register_handler
            test_handler_init();
            assert!(CALLED_REGISTER);

            let info = zwave_controller::zwave_controller_connection_info::default();
            let data = vec![3, 44, 0x33];
            test_handler_control_c(&info, data.as_ptr(), data.len() as u16);
            assert_eq!(test_handler.get().unwrap().control_data, data);

            let data = vec![4, 5, 0x33];
            test_handler_support_c(&info, data.as_ptr(), data.len() as u16);
            assert_eq!(test_handler.get().unwrap().support_data, data);
        }
    }
}
