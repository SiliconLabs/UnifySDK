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

//! This module wraps the c-library unify_application_monitoring_sys.
//! It requires dylib unify_application_monitoring_sys in order to build.
//! The symbols are generated using the bindgen crate in rust.
//! To keep the wrappers light-weight we checked in these files, hence they are
//! not regenerated when the C interface changes.
#![doc(html_no_source)]
use std::ffi::CString;

unify_tools::include_binding!(concat!(env!("OUT_DIR"), "/binding.rs"), c_bindings);
pub use c_bindings::*;

pub fn unify_application_monitoring_init() -> sl_status_t {
    unsafe { c_bindings::unify_application_monitoring_init() }
}

pub fn unify_application_monitoring_set_application_name(application_name: &str) {
    let txt = CString::new(application_name).unwrap();
    unsafe {
        c_bindings::unify_application_monitoring_set_application_name(txt.as_ptr());
    }
}

pub fn unify_application_monitoring_set_application_version(application_version: &str) {
    let txt = CString::new(application_version).unwrap();
    unsafe {
        c_bindings::unify_application_monitoring_set_application_version(txt.as_ptr());
    }
}
