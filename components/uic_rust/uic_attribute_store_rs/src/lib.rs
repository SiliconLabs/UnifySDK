// License
// <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

//! This module wraps the c-library uic_config.
//! It requires dylib uic_config in order to build.
//! The symbols are generated using the bindgen crate in rust.
//! To keep the wrappers light-weight we checked in these files, hence they are
//! not regenerated when the C interface changes.

#![allow(missing_docs)]
#![allow(dead_code)]
#![allow(non_camel_case_types)]

mod uic_attribute_store_bindings;

use std::convert::TryInto;
use std::string::FromUtf8Error;
use std::usize;

pub use uic_attribute_store_bindings::attribute_store_node_value_state_t;
pub use uic_attribute_store_bindings::attribute_store_node_value_state_t::*;
pub use uic_attribute_store_bindings::*;

/// attribute_resolver_callback type
pub type attribute_resolver_function_t = Option<
    unsafe extern "C" fn(
        node: uic_attribute_store_bindings::attribute_store_node_t,
        frame: *mut u8,
        frame_len: *mut u16,
    ) -> uic_attribute_store_bindings::sl_status_t,
>;

extern "C" {
    #[link_name = "attribute_resolver_register_rule"]
    fn attribute_resolver_register_rule_extern_c(
        node_type: uic_attribute_store_bindings::attribute_store_type_t,
        set_func: attribute_resolver_function_t,
        get_func: attribute_resolver_function_t,
    ) -> uic_attribute_store_bindings::sl_status_t;
}

pub fn attribute_resolver_register_rule(
    node_type: uic_attribute_store_bindings::attribute_store_type_t,
    set_func: attribute_resolver_function_t,
    get_func: attribute_resolver_function_t,
) -> uic_attribute_store_bindings::sl_status_t {
    unsafe { attribute_resolver_register_rule_extern_c(node_type, set_func, get_func) }
}

//binding-generator has issues generating this define. hence declare it manual
pub const ATTRIBUTE_STORE_INVALID_NODE: u32 = 0;

/// Convert an [sl_status_t] and a value of any type to [Result]<T, [sl_status_t]> type
fn convert_to_rust_result<T>(
    value: T,
    status: uic_attribute_store_bindings::sl_status_t,
) -> Result<T, uic_attribute_store_bindings::sl_status_t> {
    match status {
        0 => Ok(value),
        _ => Err(status),
    }
}

pub trait ByteArray {
    type Error;
    fn to_bytes(self) -> Vec<u8>;
    fn from_bytes(bytes: &[u8]) -> Result<Self, Self::Error>
    where
        Self: Sized;
}

impl ByteArray for i32 {
    type Error = ();

    fn to_bytes(self) -> Vec<u8> {
        self.to_ne_bytes().to_vec()
    }
    fn from_bytes(bytes: &[u8]) -> Result<Self, Self::Error> {
        Ok(i32::from_ne_bytes(bytes.try_into().unwrap()))
    }
}

impl ByteArray for u32 {
    type Error = ();

    fn to_bytes(self) -> Vec<u8> {
        self.to_ne_bytes().to_vec()
    }
    fn from_bytes(bytes: &[u8]) -> Result<Self, Self::Error> {
        Ok(u32::from_ne_bytes(bytes.try_into().unwrap()))
    }
}

impl ByteArray for i16 {
    type Error = ();

    fn to_bytes(self) -> Vec<u8> {
        self.to_ne_bytes().to_vec()
    }
    fn from_bytes(bytes: &[u8]) -> Result<Self, Self::Error> {
        Ok(i16::from_ne_bytes(bytes.try_into().unwrap()))
    }
}

impl ByteArray for u16 {
    type Error = ();

    fn to_bytes(self) -> Vec<u8> {
        self.to_ne_bytes().to_vec()
    }

    fn from_bytes(bytes: &[u8]) -> Result<Self, Self::Error> {
        Ok(u16::from_ne_bytes(bytes.try_into().unwrap()))
    }
}

impl ByteArray for u64 {
    type Error = ();

    fn to_bytes(self) -> Vec<u8> {
        self.to_ne_bytes().to_vec()
    }

    fn from_bytes(bytes: &[u8]) -> Result<Self, Self::Error> {
        Ok(u64::from_ne_bytes(bytes.try_into().unwrap()))
    }
}

impl ByteArray for i64 {
    type Error = ();

    fn to_bytes(self) -> Vec<u8> {
        self.to_ne_bytes().to_vec()
    }
    fn from_bytes(bytes: &[u8]) -> Result<Self, Self::Error> {
        Ok(i64::from_ne_bytes(bytes.try_into().unwrap()))
    }
}

impl ByteArray for usize {
    type Error = ();

    fn to_bytes(self) -> Vec<u8> {
        self.to_ne_bytes().to_vec()
    }

    fn from_bytes(bytes: &[u8]) -> Result<Self, Self::Error> {
        Ok(usize::from_ne_bytes(bytes.try_into().unwrap()))
    }
}

impl ByteArray for u8 {
    type Error = ();

    fn to_bytes(self) -> Vec<u8> {
        self.to_ne_bytes().to_vec()
    }

    fn from_bytes(bytes: &[u8]) -> Result<Self, Self::Error> {
        Ok(u8::from_ne_bytes(bytes.try_into().unwrap()))
    }
}

impl ByteArray for Vec<u8> {
    type Error = ();

    fn to_bytes(self) -> Vec<u8> {
        self
    }

    fn from_bytes(bytes: &[u8]) -> Result<Self, Self::Error> {
        Ok(Vec::from(bytes))
    }
}

impl ByteArray for String {
    type Error = FromUtf8Error;

    fn to_bytes(self) -> Vec<u8> {
        let c_string = std::ffi::CString::new(self).unwrap();
        let bytes = c_string.as_bytes();
        let mut result = bytes.to_vec();
        result.push(0);
        result
    }

    fn from_bytes(bytes: &[u8]) -> Result<Self, Self::Error> {
        let mut vec = Vec::from(bytes);
        if vec.ends_with(&[0]) {
            vec.pop();
        }
        String::from_utf8(vec)
    }
}

/// Read attribute from the attribute store in a type-safe manner
///
/// ### Example
/// This requires the attribute store to be initialized, so the test is ignored
/// ```ignore
/// use crate::uic_attribute_store_rs::{attribute_store_node_value_state_t::REPORTED_ATTRIBUTE, attribute_store_read};
/// let result: Vec<u8> =  attribute_store_read(42, REPORTED_ATTRIBUTE).unwrap();
/// println!("Result {:?}", result);
/// ```
pub fn attribute_store_read<T: ByteArray>(
    node: u32,
    value_state: attribute_store_node_value_state_t,
) -> Result<T, uic_attribute_store_bindings::sl_status_t> {
    unsafe {
        let mut size: u8 = 0;
        let mut result: Vec<u8> = vec![0; ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH as usize];
        let status = uic_attribute_store_bindings::attribute_store_get_node_attribute_value(
            node,
            value_state,
            result.as_mut_ptr(),
            &mut size,
        );
        if status != 0 {
            return Err(status);
        }

        result.resize(size as usize, 0);
        T::from_bytes(&result).map_err(|_| 1)
    }
}

/// Write attribute value to the attribute store
pub fn attribute_store_write<T: ByteArray>(
    node: u32,
    value_state: uic_attribute_store_bindings::attribute_store_node_value_state_t,
    value: T,
) -> Result<(), uic_attribute_store_bindings::sl_status_t> {
    unsafe {
        let value_bytes = value.to_bytes();
        let size = value_bytes.len() as u8;
        let status = uic_attribute_store_bindings::attribute_store_set_node_attribute_value(
            node,
            value_state,
            value_bytes.as_slice().as_ptr(),
            size,
        );
        convert_to_rust_result((), status)
    }
}

pub mod attribute_store {
    use super::*;

    pub fn init() -> Result<(), sl_status_t> {
        let status = unsafe { attribute_store_init() };
        convert_to_rust_result((), status)
    }

    pub fn teardown() -> Result<(), sl_status_t> {
        convert_to_rust_result((), unsafe {
            attribute_store_teardown().try_into().unwrap()
        })
    }

    pub fn log() {
        unsafe {
            attribute_store_log();
        }
    }

    /// Add node to the attribute store, returns [Result] with node_id or Err if
    /// node_id is ATTRIBUTE_STORE_INVALID_NODE
    pub fn add_node(
        type_: attribute_store_type_t,
        parent: attribute_store_node_t,
    ) -> Result<attribute_store_node_t, ()> {
        let node = unsafe { attribute_store_add_node(type_, parent) };

        if node == ATTRIBUTE_STORE_INVALID_NODE {
            return Err(());
        }
        return Ok(node);
    }

    /// Get or add node (and set value) of an attribute store node If `value` isß
    /// [Some] it will do an attribute_store_get_node_child_by_value(), if not it
    /// will do attribute_store_get_node_child_by_type If `set_value` is [Some] it
    /// will set the value in the attribute that is get or created.
    /// Returns the node_id of the created (og get) node.
    pub fn get_or_add_node<T: ByteArray>(
        type_: attribute_store_type_t,
        parent: attribute_store_node_t,
        value: Option<T>,
        set_value: Option<T>,
    ) -> attribute_store_node_t {
        let mut node;
        unsafe {
            if let Some(v) = value {
                let data = v.to_bytes();
                node = attribute_store_get_node_child_by_value(
                    parent,
                    type_,
                    attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
                    data.as_ptr() as *const u8,
                    data.len() as u8,
                    0,
                );
            } else {
                node = attribute_store_get_node_child_by_type(parent, type_, 0);
            }
            if node == ATTRIBUTE_STORE_INVALID_NODE {
                node = attribute_store_add_node(type_, parent);
            }
            if let Some(v) = set_value {
                attribute_store_write(
                    node,
                    attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
                    v,
                )
                .unwrap();
            }
            node
        }
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use serial_test::serial;
    use std::ffi::CString;
    use uic_proc_macro::{as_extern_c, generate_extern_c};

    extern "C" {
        pub fn datastore_init(database: *const std::os::raw::c_char) -> sl_status_t;
        pub fn datastore_teardown() -> sl_status_t;
    }

    fn setup_datastore() {
        unsafe {
            let database_name = CString::new(":memory:").unwrap();
            assert_eq!(0, datastore_init(database_name.into_raw()));
            attribute_store::init().unwrap();
        }
    }
    fn teardown() {
        unsafe {
            assert_eq!(0, datastore_teardown());
        }
    }

    #[test]
    #[serial]
    fn add_simple_node_to_datastore() {
        setup_datastore();
        unsafe {
            let node_id = attribute_store_add_node(42, attribute_store_get_root());
            assert_ne!(0, node_id);
        }
        teardown();
    }

    #[test]
    #[serial]
    fn add_write_and_read() {
        setup_datastore();
        let node_id = attribute_store::add_node(42, unsafe { attribute_store_get_root() }).unwrap();
        let desired: u8 = 13;
        attribute_store_write(
            node_id,
            attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
            desired,
        )
        .unwrap();
        let readback: u8 = attribute_store_read(
            node_id,
            attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
        )
        .unwrap();
        assert_eq!(readback, desired);
        let desired_i32: i32 = 65212444;
        attribute_store_write(
            node_id,
            attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
            desired_i32,
        )
        .unwrap();
        let readback_i32: i32 = attribute_store_read(
            node_id,
            attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
        )
        .unwrap();
        assert_eq!(desired_i32, readback_i32);
        let desired_bytes: Vec<u8> = vec![65, 66, 0];
        attribute_store_write(
            node_id,
            attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
            desired_bytes,
        )
        .unwrap();
        let readback_string: String = attribute_store_read(
            node_id,
            attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
        )
        .unwrap();
        assert_eq!(readback_string, "AB");
        attribute_store_write(
            node_id,
            attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
            String::from("CD"),
        )
        .unwrap();
        let readback_string2: String = attribute_store_read(
            node_id,
            attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
        )
        .unwrap();
        assert_eq!(readback_string2, "CD");
        let readback_bytes2: Vec<u8> = attribute_store_read(
            node_id,
            attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
        )
        .unwrap();
        assert_eq!(readback_bytes2, vec![67, 68, 0]);
        teardown();
    }

    #[generate_extern_c]
    fn set_func(_node: u32, _data: *mut u8, _length: *mut u16) -> sl_status_t {
        return 0;
    }

    #[generate_extern_c]
    fn get_func(_node: u32, _data: *mut u8, _length: *mut u16) -> sl_status_t {
        return 0;
    }

    #[test]
    #[serial]
    /// Test that attribute_resolver_register_rule is callable and doesn't return an error
    fn attribute_resolver() {
        setup_datastore();
        assert_eq!(
            0,
            attribute_resolver_register_rule(42, as_extern_c!(set_func), as_extern_c!(get_func))
        );
        teardown();
    }

    static mut NODE_ID: u32 = 666;
    static mut NODE_TYP: attribute_store_change_t = attribute_store_change_t::ATTRIBUTE_DELETED;
    #[generate_extern_c]
    fn callback_stub(node: u32, typ: attribute_store_change_t) {
        unsafe {
            NODE_ID = node;
            NODE_TYP = typ;
        }
    }

    static mut TYPE_CB_CALLED: bool = false;
    #[generate_extern_c]
    fn callback_type(_node: u32, _typ: attribute_store_change_t) {
        unsafe {
            TYPE_CB_CALLED = true;
        }
    }

    #[test]
    #[serial]
    fn test_callback_node() {
        setup_datastore();

        unsafe {
            attribute_store_register_callback(as_extern_c!(callback_stub));
            attribute_store_register_callback_by_type(as_extern_c!(callback_type), 42);
            let node_id = attribute_store_add_node(42, attribute_store_get_root());
            assert_eq!(NODE_ID, node_id);
            assert_eq!(NODE_TYP, attribute_store_change_t::ATTRIBUTE_CREATED);
            assert!(TYPE_CB_CALLED);
        }

        teardown();
    }
}
