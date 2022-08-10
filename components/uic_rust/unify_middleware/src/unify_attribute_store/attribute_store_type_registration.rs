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

//! This file contains Rust bindings for Attribute types registration API.
use std::ffi::CString;
use unify_attribute_store_sys::*;
use unify_sl_status_sys::*;

/// # Description
///
/// Try to register a new attribute type with specified type ID.
/// If such ID has been already registered - returns an error.
///
/// # Arguments
///
/// * _type_id_ - The type ID that should be registered.
/// * _type_name_ - The text representation of the type.
pub fn attribute_store_register_type(
    type_id: attribute_store_type_t,
    type_name: &str,
    parent_type: attribute_store_type_t,
    storage_type: attribute_store_storage_type_t,
) -> sl_status_t {
    let c_str = CString::new(type_name).expect("CString::new failed");
    unsafe {
        return unify_attribute_store_sys::attribute_store_register_type(
            type_id,
            c_str.as_ptr(),
            parent_type,
            storage_type,
        );
    }
}

/// # Description
///
/// Checks if type is already registered.
///
/// # Arguments
///
/// * _type_id_ - The type ID that presence should be checked.
pub fn attribute_store_is_type_registered(type_id: attribute_store_type_t) -> bool {
    unsafe {
        return unify_attribute_store_sys::attribute_store_is_type_registered(type_id);
    }
}
