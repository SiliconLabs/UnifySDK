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
///
use crate::{AttributeEventType, AttributeTrait, AttributeValueState};

/// Attribute are structs used by attribute change streams,
/// [crate::attribute_changed]. they contain information about what changed and
/// which attribute changed
#[derive(Clone, Debug, PartialEq, Hash, Eq)]
pub struct AttributeEvent<T>
where
    T: AttributeTrait,
{
    pub attribute: T,
    pub event_type: AttributeEventType,
    pub value_state: AttributeValueState,
}

impl<T> Default for AttributeEvent<T>
where
    T: AttributeTrait + Default,
{
    fn default() -> Self {
        Self {
            attribute: Default::default(),
            event_type: AttributeEventType::ATTRIBUTE_DELETED,
            value_state: AttributeValueState::DESIRED_OR_REPORTED_ATTRIBUTE,
        }
    }
}
