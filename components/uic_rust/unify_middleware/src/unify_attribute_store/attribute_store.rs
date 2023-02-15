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
use unify_log_sys::*;
use unify_sl_status_sys::*;
declare_app_name!("attribute_store_rs");

// Constants that should have been mapped
pub const ATTRIBUTE_TREE_ROOT: u32 = 1;
pub const ATTRIBUTE_STORE_INVALID_TYPE: u32 = 0;

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
                if res == SL_STATUS_OK {
                    Err(AttributeStoreError::InvalidType)
                } else {
                    Ok(res)
                }
            }
        } else {
            Err(AttributeStoreError::InvalidType)
        }
    }

    fn register_attribute_type(
        &self,
        attribute_type: AttributeTypeId,
        attribute_name: &str,
        parent_type: AttributeTypeId,
        storage_type: AttributeStorageType,
    ) -> Result<(), AttributeStoreError> {
        // Get a C compatible string from the name:
        let c_string_name = match CString::new(attribute_name) {
            Ok(c_string) => c_string,
            Err(e) => {
                log_error!("Cannot convert name to C-String {attribute_name} {e}");
                return Err(AttributeStoreError::WrappedSlStatus {
                    sl_status: SL_STATUS_FAIL,
                });
            }
        };

        // Call the C API and try to push the sl_status_t value back:
        let status;
        unsafe {
            status = unify_attribute_store_sys::attribute_store_register_type(
                attribute_type,
                c_string_name.as_ptr() as *const c_char,
                parent_type,
                storage_type,
            );
        }
        if status == SL_STATUS_OK {
            return Ok(());
        } else {
            return Err(AttributeStoreError::WrappedSlStatus { sl_status: status });
        };
    }

    fn log_type_information(&self, attribute_type: AttributeTypeId) {
        unsafe {
            unify_attribute_store_sys::attribute_store_log_attribute_type_information(
                attribute_type,
            );
        }
    }

    // Enables or disable Attribute Store Type validation.
    fn set_type_validation(&self, enabled: bool) {
        unsafe {
            unify_attribute_store_sys::attribute_store_configuration_set_type_validation(enabled);
        }
    }
}
