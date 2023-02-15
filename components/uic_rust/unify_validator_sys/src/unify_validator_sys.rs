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

unify_tools::include_binding!(concat!(env!("OUT_DIR"), "/binding.rs"), unify_validator);
pub use unify_validator::*;

use std::ffi::CString;

pub fn validate_unid(unid: &str) -> bool {
    let c_str = CString::new(unid).expect("CString::new failed");
    unsafe {
        return unify_validator::validator_validate_unid(c_str.as_ptr());
    }
}

pub fn validate_string_contains_letter_or_digit(value: &str) -> bool {
    let c_str = CString::new(value).expect("CString::new failed");
    unsafe {
        return unify_validator::validator_validate_string_contains_letter_or_digit(c_str.as_ptr());
    }
}
