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

use super::{
    AttributeChangedStream, AttributeEvent, AttributeEventType, AttributeStoreIterator,
    AttributeStoreIteratorDFS, AttributeTrait, AttributeTypeId,
};
use crate::AttributeStoreError;
use crate::AttributeStoreHandle;
use core::fmt::Display;
use core::{any::Any, cmp::Ord, convert::TryInto};
use cstr_core::CStr;
use futures::stream::LocalBoxStream;
use once_cell::sync::Lazy;
use paste::paste;
use serde::{de::DeserializeOwned, Serialize};
use std::fmt::Debug;
use unify_attribute_store_sys::attribute_store_node_value_state_t::*;
use unify_sl_status_sys::*;

const ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE: u32 = 0;
pub const ATTRIBUTE_STORE_INVALID_NODE: u32 = 0;

static ATTR_STORE_INIT: Lazy<unify_attribute_store_sys::sl_status_t> =
    Lazy::new(|| unsafe { unify_attribute_store_sys::attribute_store_init() });

#[no_mangle]
#[cfg(not(test))]
pub unsafe extern "C" fn rust_attribute_store_init() -> u32 {
    use unify_tools::waker_cache::WakerCache;
    if SL_STATUS_OK == *Lazy::force(&ATTR_STORE_INIT) {
        crate::initialize_attribute_changes(WakerCache::<AttributeEvent<Attribute>>::default())
    } else {
        SL_STATUS_FAIL
    }
}

// private helper macro that setup the desired and reported interface functions:
// * get_`reported`
// * set_`reported`
// * is_`reported`_set
// * `reported`_changed
//
// # Arguments
// * `$value`   reported or desired identifier. in the previous list, $value is depicted as `reported`
// * `$value_state` REPORTED_ATTRIBUTE or DESIRED_ATTRIBUTE
macro_rules! setup_desired_or_reported_functions {
    ($value: tt, $value_state: expr) => {
        paste! {
            fn [<get_ $value>]<T: 'static + DeserializeOwned + Any>(&self) -> Result<T, AttributeStoreError> {
                get_attribute_value(
                    *self,
                    $value_state,
                )
            }

            fn [<set_ $value>]<T: Serialize + Any>(
                &self,
                value: Option<T>,
            ) -> Result<(), AttributeStoreError> {
                set_attribute_value(
                    *self,
                    value,
                    $value_state,
                )
            }

            fn [<is_ $value _set>](&self) -> bool {
                unsafe {
                    unify_attribute_store_sys::attribute_store_is_value_defined(
                        self.handle,
                        $value_state,
                    )
                }
            }

            fn [<$value _changed>]<T>(
                &self,
            ) -> LocalBoxStream<'static, Result<(T, AttributeEventType), AttributeStoreError>>
            where
                T: 'static + DeserializeOwned + Any + Unpin,
            {
                fn conversion<A, T>(
                    event: AttributeEvent<A>,
                ) -> Result<(T, AttributeEventType), AttributeStoreError>
                where
                    T: 'static + DeserializeOwned + Any,
                    A: 'static + AttributeTrait + PartialEq + Copy,
                {
                    match event.attribute.[<get_ $value>]() {
                        Ok(data) => Ok((data, event.event_type)),
                        Err(e) => Err(e),
                    }
                }

                let attr_clone = *self;
                let stream = AttributeChangedStream::new(
                    Box::new(move |e: &AttributeEvent<Attribute>| {
                        e.value_state == $value_state && e.attribute == attr_clone
                    }),
                    conversion,
                );

                Box::pin(stream)
            }
        }
    };
}

/// The [Attribute] object is an opaque representation of an Attribute in the
/// attribute store. refer to the
/// (module)[unify_middleware::unify_attribute_store] for information about the
/// usage of the Attribute store interface.
#[derive(Default, Debug, PartialEq, Copy, Clone, Hash, Eq, PartialOrd, Ord)]
pub struct Attribute {
    pub handle: AttributeStoreHandle,
    pub type_id: AttributeTypeId,
}

impl AttributeTrait for Attribute {
    setup_desired_or_reported_functions!(reported, REPORTED_ATTRIBUTE);
    setup_desired_or_reported_functions!(desired, DESIRED_ATTRIBUTE);

    fn invalid() -> Self {
        Attribute {
            handle: ATTRIBUTE_STORE_INVALID_NODE,
            type_id: ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
        }
    }

    fn handle(&self) -> AttributeStoreHandle {
        self.handle
    }

    fn valid(&self) -> bool {
        unsafe { unify_attribute_store_sys::attribute_store_node_exists(self.handle) }
    }

    fn is_value_matched(&self) -> bool {
        unsafe { unify_attribute_store_sys::attribute_store_is_value_matched(self.handle) }
    }

    fn set_reported_as_desired(&self) -> bool {
        unsafe {
            unify_attribute_store_sys::attribute_store_set_reported_as_desired(self.handle)
                == SL_STATUS_OK
        }
    }

    fn set_desired_as_reported(&self) -> bool {
        unsafe {
            unify_attribute_store_sys::attribute_store_set_desired_as_reported(self.handle)
                == SL_STATUS_OK
        }
    }

    fn start_transition(&self, duration: u32) {
        unsafe {
            unify_attribute_utils_sys::attribute_start_transition(self.handle, duration.into());
        }
    }

    fn stop_transition(&self) {
        unsafe {
            unify_attribute_utils_sys::attribute_stop_transition(self.handle);
        }
    }

    fn child_count(&self) -> usize {
        unsafe {
            unify_attribute_store_sys::attribute_store_get_node_child_count(self.handle) as usize
        }
    }

    fn child(&self, index: usize) -> Self {
        unsafe {
            let handle = unify_attribute_store_sys::attribute_store_get_node_child(
                self.handle,
                index.try_into().unwrap(),
            );
            let type_id = unify_attribute_store_sys::attribute_store_get_node_type(handle);
            Attribute { handle, type_id }
        }
    }

    fn delete(self) -> Result<(), AttributeStoreError> {
        let status = unsafe { unify_attribute_store_sys::attribute_store_delete_node(self.handle) };
        match status {
            SL_STATUS_OK => Ok(()),
            SL_STATUS_FAIL => Err(AttributeStoreError::FailedToDelete(self)),
            SL_STATUS_NOT_INITIALIZED => Err(AttributeStoreError::NotInitialized),
            _ => Err(AttributeStoreError::WrappedSlStatus { sl_status: status }),
        }
    }

    fn type_of(&self) -> AttributeTypeId {
        self.type_id
    }

    fn type_name(&self) -> String {
        // Try getting the type as string from C, if it fails print the type as int
        unsafe {
            CStr::from_ptr(unify_attribute_store_sys::attribute_store_get_type_name(
                self.type_of(),
            ))
        }
        .to_string_lossy()
        .into_owned()
    }

    fn parent(&self) -> Self {
        unsafe {
            let handle = unify_attribute_store_sys::attribute_store_get_node_parent(self.handle);
            let type_id = unify_attribute_store_sys::attribute_store_get_node_type(handle);
            Attribute { handle, type_id }
        }
    }

    fn get_first_parent_with_type(&self, type_id: AttributeTypeId) -> Option<Self> {
        let parent = self.parent();
        if parent.type_of() == type_id {
            Some(parent)
        } else {
            match parent.valid() {
                true => parent.get_first_parent_with_type(type_id),
                false => None,
            }
        }
    }

    fn iter(&self) -> AttributeStoreIterator<Self> {
        AttributeStoreIterator::new(*self)
    }

    fn iter_dfs(&self) -> AttributeStoreIteratorDFS<Self> {
        AttributeStoreIteratorDFS::new(*self)
    }

    fn add<T: Serialize + Any>(
        &self,
        type_id: AttributeTypeId,
        reported: Option<T>,
        desired: Option<T>,
    ) -> Result<Self, AttributeStoreError> {
        let handle =
            unsafe { unify_attribute_store_sys::attribute_store_add_node(type_id, self.handle) };
        if handle == 0 {
            return Err(AttributeStoreError::FailedToAdd {
                parent: self.handle,
                type_id,
            });
        }
        let attribute = Attribute { handle, type_id };
        attribute.set_desired(desired)?;
        attribute.set_reported(reported)?;
        Ok(attribute)
    }
}

/// helper function that does the actual serialization of data.
/// serialisation is done by bincode
/// [Vec<u8>] are passed as is
/// [String] types are converted to C strings!
fn serialize_data<T: Serialize + Any>(value: T) -> Result<Vec<u8>, Box<bincode::ErrorKind>> {
    if let Some(val) = (&value as &dyn Any).downcast_ref::<String>() {
        let c_str = std::ffi::CString::new(val.as_str()).unwrap();
        Ok(c_str.as_bytes_with_nul().into())
    } else if let Some(val) = (&value as &dyn Any).downcast_ref::<&str>() {
        let c_str = std::ffi::CString::new(*val).unwrap();
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
    attribute: Attribute,
    mut data: Vec<u8>,
) -> Result<T, AttributeStoreError> {
    type ResultType<T> = Result<T, AttributeStoreError>;
    let mut res: ResultType<T> = Err(AttributeStoreError::FailedToSerialize(attribute));

    // Strings are stored in the attribute_store as c strings.
    // therefore when String type is asked, Get it as a CString and then
    // convert it to String before returning.
    // Make use of Any trait to safely downcast the value of T to a String.
    if let Some(val) = (&mut res as &mut dyn Any).downcast_mut::<ResultType<String>>() {
        let result: std::ffi::CString = std::ffi::CStr::from_bytes_with_nul(&data)
            .map_err(|_| AttributeStoreError::NullTerminationError)?
            .into();
        *val = Ok(result.into_string().unwrap());
    } else if let Some(val) = (&mut res as &mut dyn Any).downcast_mut::<ResultType<Vec<u8>>>() {
        *val = Ok(data);
    } else {
        if std::mem::size_of::<T>() < data.len() {
            return Err(AttributeStoreError::WrongSize {
                expected: std::mem::size_of::<T>(),
                actual: data.len(),
            });
        }

        // bincode has troubles with data-types smaller as u32
        // to workaround this, add trailing 0's to fix it.
        if data.len() < 4 {
            data.resize(4, 0);
        }
        res = bincode::deserialize(&data)
            .map_err(|_| AttributeStoreError::FailedToSerialize(attribute));
    }
    res
}

// Get attribute node value for either Desired or Reported
fn get_attribute_value<T: 'static + DeserializeOwned + Any>(
    attribute: Attribute,
    value_state: unify_attribute_store_sys::attribute_store_node_value_state_t,
) -> Result<T, AttributeStoreError> {
    unsafe {
        let mut bytes_written = 0;
        let mut data: Vec<u8> =
            vec![0; unify_attribute_store_sys::ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH as usize];
        let status = unify_attribute_store_sys::attribute_store_get_node_attribute_value(
            attribute.handle(),
            value_state,
            data.as_mut_ptr(),
            &mut bytes_written,
        );
        match status {
            SL_STATUS_OK => {
                if bytes_written == 0 {
                    return Err(AttributeStoreError::FailedToGet(attribute));
                }
                data.resize(bytes_written.into(), 0);
                deserialize_data(attribute, data)
            }
            SL_STATUS_NOT_INITIALIZED => Err(AttributeStoreError::NotInitialized),
            _ => Err(AttributeStoreError::WrappedSlStatus { sl_status: status }),
        }
    }
}

// Set the attribute value for either Desired or Reported value
fn set_attribute_value(
    attribute: Attribute,
    value: Option<impl Serialize + Any>,
    value_state: unify_attribute_store_sys::attribute_store_node_value_state_t,
) -> Result<(), AttributeStoreError> {
    let result: u32;
    match value {
        None => {
            result = unsafe {
                unify_attribute_store_sys::attribute_store_set_node_attribute_value(
                    attribute.handle(),
                    value_state,
                    core::ptr::null::<u8>(),
                    0,
                )
            }
        }
        Some(val) => {
            let type_id = val.type_id();
            let b_data = serialize_data(val)
                .map_err(|_| AttributeStoreError::FailedToSerialize(attribute))?;
            let b_len: u8 =
                b_data
                    .len()
                    .try_into()
                    .map_err(|_| AttributeStoreError::SizeOverflow {
                        attribute,
                        value_type_id: type_id,
                        actual_size: b_data.len(),
                        max_size: 255,
                    })?;
            result = unsafe {
                unify_attribute_store_sys::attribute_store_set_node_attribute_value(
                    attribute.handle(),
                    value_state,
                    b_data.as_ptr(),
                    b_len,
                )
            };
        }
    }
    match result {
        SL_STATUS_OK => Ok(()),
        SL_STATUS_NOT_INITIALIZED => Err(AttributeStoreError::NotInitialized),
        _ => Err(AttributeStoreError::StaleOrNoneExisting),
    }
}

impl Display for Attribute {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}[{}]", self.type_name(), self.handle())
    }
}
