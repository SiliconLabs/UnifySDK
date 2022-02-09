// License
// <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

//! This module wraps the c-library unify_config_sys.
//! It requires dylib unify_config_sys in order to build.
//! The symbols are generated using the bindgen crate in rust.
//! To keep the wrappers light-weight we checked in these files, hence they are
//! not regenerated when the C interface changes.
#![doc(html_no_source)]
unify_tools::include_binding!(uic_config);
pub use uic_config::config_status_t;
use unify_log_sys::*;

use std::ffi::CStr;
use std::ffi::CString;
use std::os::raw::c_char;
use std::os::raw::c_int;

impl std::error::Error for config_status_t {}
impl std::fmt::Display for config_status_t {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            config_status_t::CONFIG_STATUS_OK => write!(f, "Ok(0)"),
            config_status_t::CONFIG_STATUS_NOK => write!(f, "NOk(1)"),
            config_status_t::CONFIG_STATUS_ERROR => write!(f, "Error(2)"),
            config_status_t::CONFIG_STATUS_INVALID_TYPE => write!(f, "Invalid type(3)"),
            config_status_t::CONFIG_STATUS_DOES_NOT_EXIST => write!(f, "Does not exist(4)"),
        }
    }
}
pub trait ConfigLoader {
    fn from_config() -> Result<Self, config_status_t>
    where
        Self: Sized;
}

impl From<config_status_t> for i32 {
    fn from(status: config_status_t) -> Self {
        status as i32
    }
}

pub fn config_add_string(name: &str, help: &str, value: &str) -> Result<(), config_status_t> {
    let c_name = as_cstring(name)?;
    let c_help = as_cstring(help)?;
    let c_value = as_cstring(value)?;

    let status = unsafe {
        uic_config::config_add_string(c_name.as_ptr(), c_help.as_ptr(), c_value.as_ptr())
    };
    convert_to_rust_result((), status)
}

pub fn config_add_int(name: &str, help: &str, value: i32) -> Result<(), config_status_t> {
    let c_name = as_cstring(name)?;
    let c_help = as_cstring(help)?;
    let c_value = value;

    let status = unsafe { uic_config::config_add_int(c_name.as_ptr(), c_help.as_ptr(), c_value) };
    convert_to_rust_result((), status)
}

pub fn config_add_double(name: &str, help: &str, value: f64) -> Result<(), config_status_t> {
    let c_name = as_cstring(name)?;
    let c_help = as_cstring(help)?;
    let c_value = value;

    let status =
        unsafe { uic_config::config_add_double(c_name.as_ptr(), c_help.as_ptr(), c_value) };
    convert_to_rust_result((), status)
}

pub fn config_add_bool(name: &str, help: &str, value: bool) -> Result<(), config_status_t> {
    let c_name = as_cstring(name)?;
    let c_help = as_cstring(help)?;
    let c_value = value;

    let status = unsafe { uic_config::config_add_bool(c_name.as_ptr(), c_help.as_ptr(), c_value) };
    convert_to_rust_result((), status)
}

pub fn config_parse(args: Vec<CString>, version: &str) -> Result<(), config_status_t> {
    let ver = as_cstring(version)?;
    // convert the strings to raw pointers
    let c_args = args
        .iter()
        .map(|arg| arg.as_ptr())
        .collect::<Vec<*const c_char>>();
    let status = unsafe {
        uic_config::config_parse(
            c_args.len() as c_int,
            c_args.as_ptr() as *mut *mut c_char,
            ver.as_ptr(),
        )
    };
    if status == config_status_t::CONFIG_STATUS_OK {
        sl_log_read_config();
    }
    convert_to_rust_result((), status)
}

pub fn config_get_as_string(name: &str) -> Result<String, config_status_t> {
    let c_name = as_cstring(name)?;
    let mut str_value: *const c_char = std::ptr::null();

    unsafe {
        let status = uic_config::config_get_as_string(c_name.as_ptr(), &mut str_value);
        match status {
            config_status_t::CONFIG_STATUS_OK => {
                let cstring = CStr::from_ptr(str_value)
                    .to_str()
                    .map_err(|_| config_status_t::CONFIG_STATUS_ERROR)?
                    .to_string();
                Ok(cstring)
            }
            _ => Err(status),
        }
    }
}

pub fn config_get_as_int(name: &str) -> Result<i32, config_status_t> {
    let c_name = as_cstring(name)?;
    let mut value: i32 = 0;

    let status = unsafe { uic_config::config_get_as_int(c_name.as_ptr(), &mut value) };
    convert_to_rust_result(value, status)
}

pub fn config_get_as_double(name: &str) -> Result<f64, config_status_t> {
    let c_name = as_cstring(name)?;
    let mut value: f64 = 0.0;

    let status = unsafe { uic_config::config_get_as_double(c_name.as_ptr(), &mut value) };
    convert_to_rust_result(value, status)
}

pub fn config_get_as_bool(name: &str) -> Result<bool, config_status_t> {
    let c_name = as_cstring(name)?;
    let mut value = false;

    let status = unsafe { uic_config::config_get_as_bool(c_name.as_ptr(), &mut value) };
    convert_to_rust_result(value, status)
}

pub fn config_reset() {
    unsafe {
        uic_config::config_reset();
    }
}

fn as_cstring(txt: &str) -> Result<CString, config_status_t> {
    CString::new(txt).map_err(|_| config_status_t::CONFIG_STATUS_ERROR)
}

fn convert_to_rust_result<T>(value: T, status: config_status_t) -> Result<T, config_status_t> {
    match status {
        config_status_t::CONFIG_STATUS_OK => Ok(value),
        _ => Err(status),
    }
}

// Run Tests Consecutively:
// cargo test --target armv7-unknown-linux-gnueabihf -- --test-threads=1
#[cfg(test)]
mod test {
    // Note this useful idiom: importing names from outer (for mod tests) scope.
    use super::*;
    use serial_test::serial;

    #[test]
    #[serial]
    fn test_conf_parse() {
        let mut args: Vec<CString> = Vec::new();

        // Emulate cmd line arguments
        args.push(CString::new("upvl").expect("CString::new failed"));
        args.push(CString::new("--help").expect("CString::new failed"));

        // Parse cmd line arguments
        assert_eq!(
            config_parse(args, "1.1.0"),
            Err(config_status_t::CONFIG_STATUS_NOK)
        );
        config_reset();
    }

    #[test]
    #[serial]
    fn test_conf_parse_wrong_arg() {
        let mut args: Vec<CString> = Vec::new();

        // Emulate cmd line arguments
        args.push(CString::new("upvl").expect("CString::new failed"));
        args.push(CString::new("---help--").expect("CString::new failed"));

        // Parse cmd line arguments
        assert_eq!(
            config_parse(args, "1.1.0"),
            Err(config_status_t::CONFIG_STATUS_ERROR)
        );
        config_reset();
    }

    #[test]
    #[serial]
    fn test_conf_add_get() {
        // Add some keys and values to config
        assert_eq!(
            config_add_string("test.str_value", "Some string parameter", "Some String"),
            Ok(())
        );
        assert_eq!(
            config_add_int("test.int_value", "Some int parameter", 1234),
            Ok(())
        );
        assert_eq!(
            config_add_double("test.float_value", "Some float parameter", 1.234e3),
            Ok(())
        );
        assert_eq!(
            config_add_bool("test.bool_value", "Some bool parameter", true),
            Ok(())
        );

        // Add key with empty string as value. It should be replaced with value from command line
        assert_eq!(
            config_add_string(
                "test.updated_str_value",
                "Some updated string parameter",
                "",
            ),
            Ok(())
        );
        // Emulate cmd line arguments
        let mut args: Vec<CString> = Vec::new();
        args.push(CString::new("upvl").expect("CString::new failed"));
        args.push(CString::new("--test.updated_str_value").expect("CString::new failed"));
        args.push(CString::new("Value").expect("CString::new failed"));

        // Parse cmd line arguments
        assert_eq!(config_parse(args, "1.1.0"), Ok(()));

        // Check, if keys exists in config
        assert_eq!(
            config_get_as_string("test.str_value"),
            Ok("Some String".to_string())
        );
        assert_eq!(config_get_as_int("test.int_value"), Ok(1234));
        assert_eq!(config_get_as_double("test.float_value"), Ok(1.234e3));
        assert_eq!(config_get_as_bool("test.bool_value"), Ok(true));
        assert_eq!(
            config_get_as_string("test.updated_str_value"),
            Ok("Value".to_string())
        );

        // Try to get non existing key
        assert_eq!(
            config_get_as_string("test.not_exist"),
            Err(config_status_t::CONFIG_STATUS_DOES_NOT_EXIST)
        );

        // Try to get int value from string key
        assert_eq!(
            config_get_as_int("test.str_value"),
            Err(config_status_t::CONFIG_STATUS_INVALID_TYPE)
        );
        config_reset();
    }
}
