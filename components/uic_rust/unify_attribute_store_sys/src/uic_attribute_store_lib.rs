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
unify_tools::include_binding!(concat!(env!("OUT_DIR"), "/binding.rs"), uic_attribute_store);

#[cfg(test)]
mod uic_attribute_store_tests;

use serde::de::DeserializeOwned;
use serde::Serialize;
use std::any::Any;
use std::convert::TryInto;
use std::usize;

pub use uic_attribute_store::attribute_store_node_value_state_t;
pub use uic_attribute_store::attribute_store_node_value_state_t::*;
pub use uic_attribute_store::*;
const SL_STATUS_FAIL: u32 = 0x01;

#[derive(Debug)]
pub enum AttributeStoreError {
    SerializeError(Box<bincode::ErrorKind>),
    LegacyError(uic_attribute_store::sl_status_t),
    TypeMismatch {
        expected_size: usize,
        actual_size: usize,
    },
    NulTerminationError(std::ffi::FromBytesWithNulError),
}

//binding-generator has issues generating this define. hence declare it manual
pub const ATTRIBUTE_STORE_INVALID_NODE: u32 = 0;

/// Convert an [sl_status_t] and a value of any type to [Result]<T, [sl_status_t]> type
fn convert_to_rust_result<T>(
    value: T,
    status: uic_attribute_store::sl_status_t,
) -> Result<T, uic_attribute_store::sl_status_t> {
    match status {
        0 => Ok(value),
        _ => Err(status),
    }
}

pub mod attribute_store {
    use std::time::SystemTime;

    use multimap::MultiMap;
    use once_cell::unsync::OnceCell;
    use unify_log_sys::*;
    use unify_proc_macro::{as_extern_c, generate_extern_c};

    use super::*;
    use crate::uic_attribute_store;

    declare_app_name!("Attribute_store_rs");

    /// Read attribute from the attribute store in a type-safe manner
    ///
    /// ### Example
    /// This requires the attribute store to be initialized, so the test is ignored
    /// ```ignore
    /// use crate::unify_attribute_store_sys_rs::{attribute_store_node_value_state_t::REPORTED_ATTRIBUTE, attribute_store::read_attribute};
    /// let result: Vec<u8> =  attribute_store::read_attribute(42, REPORTED_ATTRIBUTE).unwrap();
    /// println!("Result {:?}", result);
    /// ```
    pub fn read_attribute<T: DeserializeOwned + Any>(
        node: u32,
        value_state: attribute_store_node_value_state_t,
    ) -> Result<T, AttributeStoreError> {
        let bytes = attribute_store::attribute_store_get_node_attribute_value(node, value_state)
            .map_err(AttributeStoreError::LegacyError)?;
        deserialize_data(bytes)
    }

    /// Write attribute value to the attribute store.
    /// param value requires [serde::Serialize] implementation!
    pub fn write_attribute<T: Serialize + Any>(
        node: u32,
        value_state: uic_attribute_store::attribute_store_node_value_state_t,
        value: T,
    ) -> Result<(), AttributeStoreError> {
        let value_bytes = serialize_data(value).map_err(AttributeStoreError::SerializeError)?;
        attribute_store::attribute_store_set_node_attribute_value(node, value_state, &value_bytes)
            .map_err(AttributeStoreError::LegacyError)
    }

    /// helper function that does the actual serialization of data.
    /// serialisation is done by bincode
    /// [Vec<u8>] are passed as is
    /// [String] types are converted to C strings!
    fn serialize_data<T: Serialize + Any>(value: T) -> Result<Vec<u8>, Box<bincode::ErrorKind>> {
        if let Some(val) = (&value as &dyn Any).downcast_ref::<String>() {
            let c_str = std::ffi::CString::new(val.as_str()).unwrap();
            Ok(c_str.as_bytes_with_nul().into())
        } else if let Some(val) = (&value as &dyn Any).downcast_ref::<Vec<u8>>() {
            Ok(val.clone())
        } else {
            let mut result = bincode::serialize(&value)?;
            // bincode can not serialize types smaller as u32.
            // (even thought they are annotated with u8)
            result.resize(std::mem::size_of::<T>(), 0);
            Ok(result)
        }
    }

    /// helper function that does the actual deserialization of data.
    /// deserialization is done by bincode
    /// [Vec<u8>] are passed as is
    /// [String] types are interpreted as C Strings!
    fn deserialize_data<T: DeserializeOwned + Any>(
        mut data: Vec<u8>,
    ) -> Result<T, AttributeStoreError> {
        type ResultType<T> = Result<T, AttributeStoreError>;
        let mut res: ResultType<T> = Err(AttributeStoreError::SerializeError(Box::new(
            bincode::ErrorKind::Custom(String::from("function exited unexpected")),
        )));

        // Strings are stored in the attribute_store as c strings.
        // therefore when String type is asked, Get it as a CString and then
        // convert it to String before returning.
        // Make use of Any trait to safely downcast the value of T to a String.
        if let Some(val) = (&mut res as &mut dyn Any).downcast_mut::<ResultType<String>>() {
            let result: std::ffi::CString = std::ffi::CStr::from_bytes_with_nul(&data)
                .map_err(AttributeStoreError::NulTerminationError)?
                .into();
            *val = Ok(result.into_string().unwrap());
        } else if let Some(val) = (&mut res as &mut dyn Any).downcast_mut::<ResultType<Vec<u8>>>() {
            *val = Ok(data);
        } else {
            if std::mem::size_of::<T>() < data.len() {
                return Err(AttributeStoreError::TypeMismatch {
                    expected_size: std::mem::size_of::<T>(),
                    actual_size: data.len(),
                });
            }

            // bincode has troubles with datatypes smaller as u32
            // to workaround this, add trailing 0's to fix it.
            if data.len() < 4 {
                data.resize(4, 0);
            }
            res = bincode::deserialize(&data).map_err(AttributeStoreError::SerializeError);
        }

        res
    }

    type TimeoutCallbackBox = Box<dyn FnOnce(uic_attribute_store::attribute_store_node_t)>;

    /// This is an Rust anti-pattern and falls into the category
    /// "i know what im doing". Dont replicate this construction.
    /// this static caches Rust closures in order to map them to
    /// timeout callbacks from C.
    /// Since we know that Unify runs synchroniously, we can never
    /// come into a race condition.
    static mut PENDING_TIMEOUT_CALLBACKS: OnceCell<
        MultiMap<
            uic_attribute_store::attribute_store_node_t,
            (std::time::SystemTime, TimeoutCallbackBox),
        >,
    > = OnceCell::new();

    #[generate_extern_c]
    fn dispatch_timeout_callback(node: u32) {
        // This function maps native callbacks back to rust closures.
        // Since this code is called from native code, we cannot return an Result structure.
        // Therefore write out every error/warn to logging.
        unsafe {
            if let Some(callbacks) = PENDING_TIMEOUT_CALLBACKS.get_mut() {
                if let Some(functions) = callbacks.get_vec_mut(&node) {
                    // if we get a callback for a node, assume its for the callback which has the earliest timeout time.
                    // remove it from the list when called.
                    functions.sort_by(|a, b| a.0.cmp(&b.0).reverse());
                    if let Some(item) = functions.pop() {
                        item.1(node);
                    } else {
                        log_warning!("expected an callback to be present in the cache but it isnt! nodeid={}", node);
                    }
                } else {
                    log_error!("Internal timeout callback cache error! nodeid={}", node);
                }
            } else {
                log_error!(
                    "callback for nodeid={}. but callback cache is not initialized",
                    node
                );
            }
        }
    }

    #[derive(Debug)]
    pub enum AttributeStoreTimeOutError {
        Legacy(sl_status_t),
        InternalError,
    }

    pub fn set_callback_on_timeout<F>(
        node: uic_attribute_store::attribute_store_node_t,
        duration: std::time::Duration,
        callback_function: F,
    ) -> Result<(), AttributeStoreTimeOutError>
    where
        F: 'static + FnOnce(uic_attribute_store::attribute_store_node_t),
    {
        unsafe {
            PENDING_TIMEOUT_CALLBACKS.get_or_init(MultiMap::new);
            let mm = PENDING_TIMEOUT_CALLBACKS.get_mut().unwrap();

            mm.insert(
                node,
                (SystemTime::now() + duration, Box::new(callback_function)),
            );

            let duration_seconds: u64 =
                duration.as_secs() * uic_attribute_store::CLOCK_SECOND as u64;

            // redirect callbacks to dispatch_timeout_callback. Callbacks to Rust closures
            // are handled by this function.
            // durations are u32 on x86 and u64 on x64 platforms!
            let res = uic_attribute_store::attribute_timeout_set_callback(
                node,
                duration_seconds.try_into().unwrap(),
                as_extern_c!(dispatch_timeout_callback),
            );

            let result =
                convert_to_rust_result((), res).map_err(AttributeStoreTimeOutError::Legacy);

            if result.is_err() {
                // remove bookkeeping of the result was not oke
                mm.get_vec_mut(&node)
                    .ok_or(AttributeStoreTimeOutError::InternalError)?
                    .pop();
            }
            result
        }
    }

    pub fn init() -> Result<(), sl_status_t> {
        let status = unsafe { uic_attribute_store::attribute_store_init() };
        convert_to_rust_result((), status)
    }

    pub fn teardown() -> Result<(), sl_status_t> {
        convert_to_rust_result((), unsafe {
            uic_attribute_store::attribute_store_teardown()
                .try_into()
                .unwrap()
        })
    }

    pub fn log() {
        unsafe {
            attribute_store_log();
        }
    }

    pub fn attribute_store_set_node_attribute_value(
        node: attribute_store_node_t,
        value_state: attribute_store_node_value_state_t,
        value: &[u8],
    ) -> Result<(), uic_attribute_store::sl_status_t> {
        unsafe {
            let status = uic_attribute_store::attribute_store_set_node_attribute_value(
                node,
                value_state,
                value.as_ptr(),
                value.len().try_into().unwrap(),
            );
            convert_to_rust_result((), status)
        }
    }

    pub fn attribute_store_get_node_attribute_value(
        node: attribute_store_node_t,
        value_state: attribute_store_node_value_state_t,
    ) -> Result<Vec<u8>, uic_attribute_store::sl_status_t> {
        unsafe {
            let mut bytes_written = 0;
            let mut result: Vec<u8> = vec![0; ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH as usize];
            let status = uic_attribute_store::attribute_store_get_node_attribute_value(
                node,
                value_state,
                result.as_mut_ptr(),
                &mut bytes_written,
            );
            if status != 0 {
                return Err(status);
            }

            if bytes_written == 0 {
                return Err(SL_STATUS_FAIL);
            }

            result.resize(bytes_written as usize, 0);
            convert_to_rust_result(result, status)
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
        Ok(node)
    }

    /// Get or add node (and set value) of an attribute store node If `value` is
    /// [Some] it will do an attribute_store_get_node_child_by_value(), if not it
    /// will do attribute_store_get_node_child_by_type If `set_value` is [Some] it
    /// will set the value in the attribute that is get or created.
    /// Returns the node_id of the created (og get) node.
    pub fn get_or_add_node<T: Serialize + Any>(
        typ: attribute_store_type_t,
        parent: attribute_store_node_t,
        value: Option<T>,
        set_value: Option<T>,
    ) -> attribute_store_node_t {
        let mut node;
        unsafe {
            if let Some(v) = value {
                let data = serialize_data(v).unwrap();
                node = attribute_store_get_node_child_by_value(
                    parent,
                    typ,
                    attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
                    data.as_ptr() as *const u8,
                    data.len() as u8,
                    0,
                );
            } else {
                node = attribute_store_get_node_child_by_type(parent, typ, 0);
            }
            if node == ATTRIBUTE_STORE_INVALID_NODE {
                node = attribute_store_add_node(typ, parent);
            }
            if let Some(v) = set_value {
                attribute_store::write_attribute(
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
