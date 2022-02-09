// License
// <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

use core::{
    any::{Any, TypeId},
    cmp::Ord,
    convert::TryInto,
    ptr,
};
use cstr_core::{c_char, CStr};
use once_cell::sync::Lazy;
use serde::{de::DeserializeOwned, Serialize};
use std::fmt::Debug;
use std::{ffi::CString, fmt::Display};
use unify_sl_status_sys::*;

use crate::{
    initialize_attribute_changes, AttributeChangeListener, AttributeEvent, AttributeEventType,
    AttributeStoreExtTrait, AttributeStoreTrait, AttributeTypeId, AttributeValueState,
    UicAttributeStoreIterator, UicAttributeStoreIteratorDFS,
};
use futures::stream::LocalBoxStream;

use super::attribute_stream::{self};

// Constants that should have been mapped
const ATTRIBUTE_STORE_INVALID_NODE: u32 = 0;
const ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE: u32 = 0;
static ATTR_STORE_INIT: Lazy<unify_attribute_store_sys::sl_status_t> =
    Lazy::new(|| unsafe { unify_attribute_store_sys::attribute_store_init() });

#[no_mangle]
pub unsafe extern "C" fn rust_attribute_store_init() -> u32 {
    if SL_STATUS_OK == *Lazy::force(&ATTR_STORE_INIT) {
        initialize_attribute_changes(AttributeChangeListener::default())
    } else {
        SL_STATUS_FAIL
    }
}

/// Errors that can be returned by the attribute store.
#[derive(Debug, PartialEq, Clone)]
pub enum AttributeStoreError {
    NotInitialized,
    NullTerminationError,
    WrongSize {
        expected: usize,
        actual: usize,
    },
    StaleOrNoneExisting,
    InvalidHandle {
        handle: AttributeStoreHandle,
    },
    InvalidType,
    FailedToInitialize,
    FailedToAdd {
        parent: AttributeStoreHandle,
        type_id: AttributeTypeId,
    },
    FailedToSerialize(Attribute),
    FailedToDelete(Attribute),
    FailedToGet(Attribute),
    SizeOverflow {
        attribute: Attribute,
        actual_size: usize,
        max_size: usize,
        value_type_id: TypeId,
    },
    WrappedSlStatus {
        sl_status: u32,
    },
}

impl Default for AttributeStoreError {
    fn default() -> Self {
        AttributeStoreError::NotInitialized
    }
}

impl Display for AttributeStoreError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            AttributeStoreError::NotInitialized => write!(f, "not initialized"),
            AttributeStoreError::NullTerminationError => write!(f, "null termination error"),
            AttributeStoreError::WrongSize { expected, actual } => {
                write!(f, "wrong size. expected={}, actual={}", expected, actual)
            }
            AttributeStoreError::StaleOrNoneExisting => write!(f, "stale or none existing"),
            AttributeStoreError::InvalidHandle { handle } => write!(f, "invalid handle {}", handle),
            AttributeStoreError::InvalidType => write!(f, "invalid type"),
            AttributeStoreError::FailedToInitialize => write!(f, "failed to initialize"),
            AttributeStoreError::FailedToAdd { parent, type_id } => {
                write!(f, "failed to add. parent={} type_id={}", parent, type_id)
            }
            AttributeStoreError::FailedToSerialize(_) => write!(f, "failed to serialize"),
            AttributeStoreError::FailedToDelete(_) => write!(f, "failed to delete"),
            AttributeStoreError::FailedToGet(_) => write!(f, "failed to get"),
            AttributeStoreError::SizeOverflow {
                attribute,
                actual_size,
                max_size,
                value_type_id,
            } => write!(
                f,
                "size overflow. attribute={:?} actual_size={} max_size={} value_type_id={:?}",
                attribute, actual_size, max_size, value_type_id
            ),
            AttributeStoreError::WrappedSlStatus { sl_status } => {
                write!(f, "sl_status error {}", sl_status)
            }
        }
    }
}

/// Handle used by the AttributeStore(Trait) that corresponds
/// to a attribute node.
/// These handles can be wrapped by AttributeNode objects for
/// a object-like interface.
pub(crate) type AttributeStoreHandle = u32;

/// The [Attribute] object is an opaque representation of an Attribute in the
/// attribute store. refer to the
/// (module)[unify_middleware::unify_attribute_store] for information about the
/// usage of the Attribute store interface.
#[derive(Default, Debug, PartialEq, Copy, Clone, Hash, Eq, PartialOrd, Ord)]
pub struct Attribute {
    pub handle: AttributeStoreHandle,
    pub type_id: AttributeTypeId,
}

impl Display for Attribute {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        // Try getting the type as string from C, if it fails print the type as int
        let c_str = unsafe {
            CStr::from_ptr(unify_attribute_store_sys::attribute_store_name_by_type(
                self.type_id,
            ))
        }
        .to_str();
        let type_str = if c_str.is_ok() {
            c_str.unwrap().to_owned()
        } else {
            format!("{}", self.type_id)
        };

        write!(f, "{}[{}]", type_str, self.handle)
    }
}

impl Attribute {
    /// Return root Attribute or AttributeStoreError in case of error
    pub fn root() -> Result<Attribute, AttributeStoreError> {
        if *ATTR_STORE_INIT != SL_STATUS_OK {
            return Err(AttributeStoreError::FailedToInitialize);
        }
        unsafe {
            let root = unify_attribute_store_sys::attribute_store_get_root();
            if root == ATTRIBUTE_STORE_INVALID_NODE {
                return Err(AttributeStoreError::NotInitialized);
            }
            Attribute::new_from_handle(root)
        }
    }

    pub fn attribute_type_from_name(name: &str) -> Result<u32, AttributeStoreError> {
        if let Ok(c_name) = CString::new(name) {
            unsafe {
                let res = unify_attribute_store_sys::attribute_store_type_by_name(
                    c_name.as_ptr() as *const c_char
                );
                if res == 0xffffffff {
                    Err(AttributeStoreError::InvalidType)
                } else {
                    Ok(res)
                }
            }
        } else {
            Err(AttributeStoreError::InvalidType)
        }
    }
    /// Create a new Attribute from a `handle` and `type_id`
    pub fn new(handle: u32, type_id: u32) -> Result<Self, AttributeStoreError> {
        Ok(Attribute { handle, type_id })
    }

    /// Create a new Attribute from a `handle` the `type_id` is read from the
    /// Attribute Store, AttributeStoreError is returned in case the `handle`
    /// doesn't exist in the Attribute Store
    pub fn new_from_handle(handle: u32) -> Result<Self, AttributeStoreError> {
        let type_id = unsafe { unify_attribute_store_sys::attribute_store_get_node_type(handle) };
        if type_id == ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE {
            return Err(AttributeStoreError::InvalidHandle { handle });
        }
        Attribute::new(handle, type_id)
    }

    // Set the attribute value for either Desired or Reported value
    fn set_attribute_value(
        &self,
        value: Option<impl Serialize + Any>,
        value_state: unify_attribute_store_sys::attribute_store_node_value_state_t,
    ) -> Result<(), AttributeStoreError> {
        let result: u32;
        match value {
            None => {
                result = unsafe {
                    unify_attribute_store_sys::attribute_store_set_node_attribute_value(
                        self.handle,
                        value_state,
                        ptr::null::<u8>(),
                        0,
                    )
                }
            }
            Some(val) => {
                let type_id = val.type_id();
                let b_data = serialize_data(val)
                    .map_err(|_| AttributeStoreError::FailedToSerialize(*self))?;
                let b_len: u8 =
                    b_data
                        .len()
                        .try_into()
                        .map_err(|_| AttributeStoreError::SizeOverflow {
                            attribute: (*self),
                            value_type_id: type_id,
                            actual_size: b_data.len(),
                            max_size: 255,
                        })?;
                result = unsafe {
                    unify_attribute_store_sys::attribute_store_set_node_attribute_value(
                        self.handle,
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

    // Get attribute node value for either Desired or Reported
    fn get_attribute_value<T: 'static + DeserializeOwned + Any>(
        &self,
        value_state: unify_attribute_store_sys::attribute_store_node_value_state_t,
    ) -> Result<T, AttributeStoreError> {
        unsafe {
            let mut bytes_written = 0;
            let mut data: Vec<u8> =
                vec![0; unify_attribute_store_sys::ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH as usize];
            let status = unify_attribute_store_sys::attribute_store_get_node_attribute_value(
                self.handle,
                value_state,
                data.as_mut_ptr(),
                &mut bytes_written,
            );
            match status {
                SL_STATUS_OK => {
                    if bytes_written == 0 {
                        return Err(AttributeStoreError::FailedToGet(*self));
                    }
                    data.resize(bytes_written.into(), 0);
                    deserialize_data(&self, data)
                }
                SL_STATUS_NOT_INITIALIZED => Err(AttributeStoreError::NotInitialized),
                _ => Err(AttributeStoreError::WrappedSlStatus { sl_status: status }),
            }
        }
    }
}

impl AttributeStoreTrait for Attribute {
    type Attribute = Attribute;
    type Error = AttributeStoreError;

    fn exists(&self) -> bool {
        unsafe { unify_attribute_store_sys::attribute_store_node_exists(self.handle) }
    }

    fn get_parent(&self) -> Result<Attribute, Self::Error> {
        let handle =
            unsafe { unify_attribute_store_sys::attribute_store_get_node_parent(self.handle) };
        if handle == ATTRIBUTE_STORE_INVALID_NODE {
            Err(AttributeStoreError::StaleOrNoneExisting)
        } else {
            Attribute::new_from_handle(handle)
        }
    }

    fn get_first_parent_with_type(
        &self,
        type_id: AttributeTypeId,
    ) -> Result<Self::Attribute, Self::Error> {
        if let Ok(p) = self.get_parent() {
            if p.type_of() == type_id {
                return Ok(p);
            } else {
                return p.get_first_parent_with_type(type_id);
            }
        }

        Err(AttributeStoreError::StaleOrNoneExisting)
    }

    fn get_desired<T: 'static + DeserializeOwned + Any>(&self) -> Result<T, Self::Error> {
        self.get_attribute_value(
            unify_attribute_store_sys::attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
        )
    }

    fn get_reported<T: 'static + DeserializeOwned + Any>(&self) -> Result<T, Self::Error> {
        self.get_attribute_value(
            unify_attribute_store_sys::attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        )
    }

    fn set_reported(&self, value: Option<impl Serialize + Any>) -> Result<(), Self::Error> {
        self.set_attribute_value(
            value,
            unify_attribute_store_sys::attribute_store_node_value_state_t::REPORTED_ATTRIBUTE,
        )
    }

    fn set_desired(&self, value: Option<impl Serialize + Any>) -> Result<(), Self::Error> {
        self.set_attribute_value(
            value,
            unify_attribute_store_sys::attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
        )
    }

    fn delete(self) -> Result<(), Self::Error> {
        let status = unsafe { unify_attribute_store_sys::attribute_store_delete_node(self.handle) };
        match status {
            SL_STATUS_OK => Ok(()),
            SL_STATUS_FAIL => Err(AttributeStoreError::FailedToDelete(self)),
            SL_STATUS_NOT_INITIALIZED => Err(AttributeStoreError::NotInitialized),
            _ => Err(AttributeStoreError::WrappedSlStatus { sl_status: status }),
        }
    }

    fn add<T: Serialize + Any>(
        &self,
        type_id: AttributeTypeId,
        reported: Option<T>,
        desired: Option<T>,
    ) -> Result<Attribute, Self::Error> {
        let handle =
            unsafe { unify_attribute_store_sys::attribute_store_add_node(type_id, self.handle) };
        if handle == 0 {
            return Err(AttributeStoreError::FailedToAdd {
                parent: self.handle,
                type_id,
            });
        }
        let attribute = Attribute::new(handle, type_id)?;
        attribute.set_desired(desired)?;
        attribute.set_reported(reported)?;
        Ok(attribute)
    }

    fn type_of(&self) -> AttributeTypeId {
        self.type_id
    }

    fn iter(&self) -> Box<dyn Iterator<Item = Self::Attribute>> {
        Box::new(UicAttributeStoreIterator::new(*self))
    }

    fn iter_dfs(&self) -> Box<dyn Iterator<Item = Self::Attribute>> {
        Box::new(UicAttributeStoreIteratorDFS::new(*self))
    }

    fn desired_changed<T>(
        &self,
    ) -> LocalBoxStream<'static, Result<(T, AttributeEventType), Self::Error>>
    where
        T: 'static + DeserializeOwned + Any + Unpin,
    {
        fn conversion<T>(
            event: AttributeEvent,
        ) -> Result<(T, AttributeEventType), AttributeStoreError>
        where
            T: 'static + DeserializeOwned + Any,
        {
            match event.attribute.get_desired() {
                Ok(data) => Ok((data, event.event_type)),
                Err(e) => Err(e),
            }
        }

        let stream = attribute_stream::AttributeChangedStream::new(
            Box::new(|e: &AttributeEvent| e.value_state == AttributeValueState::DESIRED_ATTRIBUTE),
            conversion::<T>,
        );

        Box::pin(stream)
    }

    fn reported_changed<T>(
        &self,
    ) -> LocalBoxStream<'static, Result<(T, AttributeEventType), Self::Error>>
    where
        T: 'static + DeserializeOwned + Any + Unpin,
    {
        fn conversion<T>(
            event: AttributeEvent,
        ) -> Result<(T, AttributeEventType), AttributeStoreError>
        where
            T: 'static + DeserializeOwned + Any,
        {
            match event.attribute.get_reported() {
                Ok(data) => Ok((data, event.event_type)),
                Err(e) => Err(e),
            }
        }

        let stream = attribute_stream::AttributeChangedStream::new(
            Box::new(|e: &AttributeEvent| e.value_state == AttributeValueState::REPORTED_ATTRIBUTE),
            conversion,
        );

        Box::pin(stream)
    }

    fn attribute_changed(
        predicate: impl Fn(&AttributeEvent) -> bool + 'static,
    ) -> LocalBoxStream<'static, AttributeEvent> {
        fn conversion(event: AttributeEvent) -> AttributeEvent {
            event
        }

        Box::pin(attribute_stream::AttributeChangedStream::new(
            Box::new(predicate),
            conversion,
        ))
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
    attribute: &Attribute,
    mut data: Vec<u8>,
) -> Result<T, AttributeStoreError> {
    type ResultType<T> = Result<T, AttributeStoreError>;
    let mut res: ResultType<T> = Err(AttributeStoreError::FailedToSerialize(*attribute));

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
            .map_err(|_| AttributeStoreError::FailedToSerialize(*attribute));
    }
    res
}

impl AttributeStoreExtTrait for Attribute {
    fn child_count(&self) -> usize {
        unsafe {
            unify_attribute_store_sys::attribute_store_get_node_child_count(self.handle) as usize
        }
    }

    fn child(&self, index: usize) -> Result<Self, Self::Error> {
        unsafe {
            let handle = unify_attribute_store_sys::attribute_store_get_node_child(
                self.handle,
                index.try_into().unwrap(),
            );
            Attribute::new_from_handle(handle)
        }
    }
}
