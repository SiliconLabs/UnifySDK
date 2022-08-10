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

use std::{any::TypeId, fmt::Display};

use crate::{Attribute, AttributeStoreHandle, AttributeTypeId};

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
