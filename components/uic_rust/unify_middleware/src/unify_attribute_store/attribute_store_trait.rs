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

// This trait can be seen as an Attribute factory. It is able to create new
// attribute instances. By having a central abstraction that creates attributes
// we can easily swap it with a `Mock` factory. Implementors of this interface
// assure that attribute store is setup successful.
#[mockall::automock]
pub trait AttributeStoreTrait<T>
where
    T: 'static + AttributeTrait,
{
    /// Returns the root of the attribute store. This root attribute always be
    /// valid. As long as the implementor of this trait successfully created an
    /// instance of this trait.
    fn root(&self) -> T;
    /// Create a new Attribute from a `handle`. The `type_id` is read from the
    /// Attribute Store. In case the handle does not exists an invalid attribute
    /// will be returned. callers can check validity of the object by calling
    /// [.valid()](crate::AttributeTrait::valid) on the attribute object itself
    fn from_handle(&self, handle: u32) -> T;
    /// Create a new Attribute from a `handle` and `type_id`. In case the handle
    /// or type_id does not exists an invalid attribute will be returned.
    /// Callers can check validity of the object by calling
    /// [.valid()](crate::AttributeTrait::valid) on the attribute object itself
    fn from_handle_and_type(&self, handle: u32, type_id: u32) -> T;

    fn attribute_type_from_name(&self, name: &str) -> Result<u32, AttributeStoreError>;
}

/// Wraps `attribute.valid()` and returns an
/// `Err(AttributeStoreError::StaleOrNoneExisting)` if the given attribute is
/// not valid. `Ok(attribute)` will be returned in the case the attribute is
/// valid. There are no guarantees given in the attribute store component that
/// the holder of an `Attribute` always has a valid instance. Attributes can be
/// removed or updated in the meantime or render invalid. This functions makes
/// sure that the owner in facts holds a valid reference to an Attribute in the
/// Attributestore.
pub fn verify_attribute<T>(attribute: T) -> Result<T, AttributeStoreError>
where
    T: 'static + AttributeTrait,
{
    match attribute.valid() {
        true => Ok(attribute),
        false => Err(AttributeStoreError::StaleOrNoneExisting),
    }
}

/// Wraps `attribute.valid()` and returns an option. Some(attribute) in the case
/// its valid. None if not. see [verify_attribute] for more information.
pub fn verify_attribute_ok<T>(attribute: T) -> Option<T>
where
    T: 'static + AttributeTrait,
{
    verify_attribute(attribute).ok()
}
