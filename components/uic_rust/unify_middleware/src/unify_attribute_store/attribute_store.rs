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
use super::*;

use cstr_core::c_char;

use once_cell::sync::Lazy;
use std::ffi::CString;
use unify_sl_status_sys::*;

// Constants that should have been mapped
const ATTRIBUTE_TREE_ROOT: u32 = 1;

/// Concrete implemntation of the [AttributeStoreTrait].
pub struct AttributeStore {}

impl AttributeStore {
    pub fn new() -> Result<Self, AttributeStoreError> {
        static ATTR_STORE_INIT: Lazy<unify_attribute_store_sys::sl_status_t> =
            Lazy::new(|| unsafe { unify_attribute_store_sys::attribute_store_init() });
        if *ATTR_STORE_INIT != SL_STATUS_OK {
            return Err(AttributeStoreError::FailedToInitialize);
        }
        Ok(AttributeStore {})
    }
}

impl AttributeStoreTrait<Attribute> for AttributeStore {
    fn root(&self) -> Attribute {
        unsafe {
            let root = unify_attribute_store_sys::attribute_store_get_root();
            self.from_handle_and_type(root, ATTRIBUTE_TREE_ROOT)
        }
    }

    fn from_handle(&self, handle: u32) -> Attribute {
        let type_id = unsafe { unify_attribute_store_sys::attribute_store_get_node_type(handle) };
        self.from_handle_and_type(handle, type_id)
    }

    fn from_handle_and_type(&self, handle: u32, type_id: u32) -> Attribute {
        Attribute { handle, type_id }
    }

    /// Return root Attribute or AttributeStoreError in case of error
    fn attribute_type_from_name(&self, name: &str) -> Result<u32, AttributeStoreError> {
        if let Ok(c_name) = CString::new(name) {
            unsafe {
                let res = unify_attribute_store_sys::attribute_store_get_type_by_name(
                    c_name.as_ptr() as *const c_char,
                );
                if res == 0 {
                    Err(AttributeStoreError::InvalidType)
                } else {
                    Ok(res)
                }
            }
        } else {
            Err(AttributeStoreError::InvalidType)
        }
    }
}
