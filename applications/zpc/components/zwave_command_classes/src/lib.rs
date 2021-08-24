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
pub use zwave_command_class_firmware_update::*;

use zwave_command_handler::*;
use zwave_rust_proc_macros::initialize_zwave_command_classes;

pub struct ZwaveHandlerConfig {
    pub minimal_scheme: zwave_controller_encapsulation_scheme_t,
    pub command_class: u16,
    pub command_class_name: &'static str,
    pub comments: &'static str,
    pub version: u8,
    pub manual_security_validation: bool,
}

trait ToFrameBuffer {
    fn to_frame_buffer(self, frame: &mut [u8]) -> usize;
}
trait ToFrameBufferBasedOnVersion {
    fn to_frame_buffer_based_on_version(self, frame: &mut [u8], version: u16) -> usize;
}

initialize_zwave_command_classes!();

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

// TODO: This is a temporary and costly solution. UIC-1100 is created to improve it.
pub fn call_once<T: Send + 'static>(timout: std::time::Duration, func: impl Fn(T) + Send + 'static, t:T) {
    std::thread::spawn(move || {
        std::thread::sleep(timout);
        func(t);
    });
}
