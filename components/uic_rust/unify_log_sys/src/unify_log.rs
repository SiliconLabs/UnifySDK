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

//! This module wraps the c-library unify_log_sys.
//! It requires dylib libunify_log_sys in order to build.
//! The symbols are generated using the bindgen crate in rust.
//! To keep the wrappers light-weight we checked in these files, hence they are
//! not regenerated when the C interface changes.
#![doc(html_no_source)]
use std::ffi::CString;

unify_tools::include_binding!(concat!(env!("OUT_DIR"), "/binding.rs"), uic_log);
use uic_log::*;

impl std::fmt::Display for sl_log_level_t {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            sl_log_level_t::SL_LOG_DEBUG => write!(f, "SL_LOG_DEBUG"),
            sl_log_level_t::SL_LOG_INFO => write!(f, "SL_LOG_INFO"),
            sl_log_level_t::SL_LOG_NOTICE => write!(f, "SL_LOG_NOTICE"),
            sl_log_level_t::SL_LOG_WARNING => write!(f, "SL_LOG_WARNING"),
            sl_log_level_t::SL_LOG_ERROR => write!(f, "SL_LOG_ERROR"),
            sl_log_level_t::SL_LOG_CRITICAL => write!(f, "SL_LOG_CRITICAL"),
        }
    }
}

pub fn sl_log_set_level(level: sl_log_level_t) {
    unsafe {
        uic_log::sl_log_set_level(level);
    }
}

pub fn sl_log_set_tag_level(tag: &str, level: sl_log_level_t) {
    let txt = CString::new(tag).unwrap();
    unsafe {
        uic_log::sl_log_set_tag_level(txt.as_ptr(), level);
    }
}

pub fn sl_log_unset_tag_level(tag: &str) {
    let txt = CString::new(tag).unwrap();

    unsafe {
        uic_log::sl_log_unset_tag_level(txt.as_ptr());
    }
}

pub fn sl_log_level_from_string(level: &str) -> Result<sl_log_level_t, sl_status_t> {
    // todo: fix generation of defines fo sl_status
    let txt = CString::new(level).map_err(|_| 1_u32)?;
    let mut lvl = sl_log_level_t::SL_LOG_DEBUG;
    let status = unsafe { uic_log::sl_log_level_from_string(txt.as_ptr(), &mut lvl) };
    match status {
        0 => Ok(lvl),
        _ => Err(status),
    }
}

#[macro_export]
macro_rules! declare_app_name {
    ($args:expr) => {
        const APP_NAME: &str = $args;
    };
}

#[macro_export]
macro_rules! log_debug {
    ($($args:expr),*) => {
        //print_log(APP_NAME, sl_log_level::SL_LOG_DEBUG, format!($($args),*));
        log_debug_fn(APP_NAME, format!($($args),*));
    };
}

#[macro_export]
macro_rules! log_error {
    ($($args:expr),*) => {
        log_error_fn(APP_NAME, format!($($args),*));
    };
}

#[macro_export]
macro_rules! log_info {
    ($($args:expr),*) => {
        log_info_fn(APP_NAME, format!($($args),*));
    };
}

#[macro_export]
macro_rules! log_warning {
    ($($args:expr),*) => {
        log_warning_fn(APP_NAME, format!($($args),*));
    };
}

#[macro_export]
macro_rules! log_critical {
    ($($args:expr),*) => {
        log_critical_fn(APP_NAME, format!($($args),*));
    };
}

pub fn log_notice_fn(log_tag: &str, message: String) {
    print_log(log_tag, sl_log_level::SL_LOG_NOTICE, &message);
}

pub fn log_debug_fn(log_tag: &str, message: String) {
    print_log(log_tag, sl_log_level::SL_LOG_DEBUG, &message);
}

pub fn log_error_fn(log_tag: &str, message: String) {
    print_log(log_tag, sl_log_level::SL_LOG_ERROR, &message);
}

pub fn log_info_fn(log_tag: &str, message: String) {
    print_log(log_tag, sl_log_level::SL_LOG_INFO, &message);
}

pub fn log_warning_fn(log_tag: &str, message: String) {
    print_log(log_tag, sl_log_level::SL_LOG_WARNING, &message);
}

pub fn log_critical_fn(log_tag: &str, message: String) {
    print_log(log_tag, sl_log_level::SL_LOG_CRITICAL, &message);
}

pub fn sl_log_read_config() {
    unsafe { uic_log::sl_log_read_config() }
}

fn print_log(log_tag: &str, log_level: sl_log_level, message: &str) {
    let c_to_print = CString::new(message).expect("CString::new failed");
    let tag = CString::new(log_tag).expect("CString::new failed");
    unsafe { sl_log(tag.as_ptr(), log_level, c_to_print.as_ptr()) };
}
