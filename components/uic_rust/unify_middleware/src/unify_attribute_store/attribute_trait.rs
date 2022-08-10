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
use super::AttributeStoreError;
use crate::AttributeEventType;
use crate::AttributeStoreIterator;
use crate::AttributeStoreIteratorDFS;
use core::any::Any;
use futures::stream::LocalBoxStream;
use serde::de::DeserializeOwned;
use serde::Serialize;
use std::fmt::Debug;

/// This type specifies the actual used storage type inside of the attribute
/// store. Since they are related to the native uic_datastore component the
/// choice was made to explicitly define them, rather then making use of the
/// already present [std::any::TypeId] inside of Rust.
pub type AttributeTypeId = u32;

/// Handle used by the AttributeStore(Trait) that corresponds to a attribute
/// node. These handles can be wrapped by AttributeNode objects for a
/// object-like interface.
pub(crate) type AttributeStoreHandle = u32;

/// see [module documentation](unify_attribute_store) for more information on
/// the usage of this interface.
pub trait AttributeTrait: Debug + Clone
where
    Self: Sized,
{
    // returns an Attribute object which is invalid.
    fn invalid() -> Self;

    // returns the raw handle used to index into the attribute store.
    fn handle(&self) -> AttributeStoreHandle;

    /// Returns true if `self` is a valid attribute in the attribute store, else
    /// false.
    fn valid(&self) -> bool;

    /// function that verifies if the reported value is set returns true if set,
    /// false if not or in case of an invalid attribute
    fn is_reported_set(&self) -> bool;

    /// function that verifies if the desired value is set returns true if set,
    /// false if not or in case of an invalid attribute
    fn is_desired_set(&self) -> bool;

    /// check if the reported value is the same as the desired value. this
    /// function returns false if the attribute is invalid
    fn is_value_matched(&self) -> bool;

    /// copy the reported value to the desired value returns false if the
    /// attribute is in valid. returns true on success.
    fn set_reported_as_desired(&self) -> bool;

    /// copy the desired value to the reported value returns false if the
    /// attribute is in valid. returns true on success.
    fn set_desired_as_reported(&self) -> bool;

    /// Starts a transition from the reported to the desired value
    /// using the duration in ms indicated as a parameter.
    fn start_transition(&self, duration: u32);

    /// Stops an ongoing transition, if any
    fn stop_transition(&self);

    /// Delete an attribute. Note: This function takes `self` by value. it does
    /// not exist anymore after calling this function.
    ///
    /// # Returns
    /// either Ok(()), Err(FailedToDelete), Err(NotInitialized)
    fn delete(self) -> Result<(), AttributeStoreError>;

    /// Return the Type Id of `self`.
    fn type_of(&self) -> AttributeTypeId;

    /// tries to lookup the name of the attribute type. returns the integer
    /// representation otherwise.
    fn type_name(&self) -> String;

    fn child_count(&self) -> usize;
    fn child(&self, index: usize) -> Self;

    /// Get all direct children of a given type, if the attribute has no
    /// children the default error is returned.
    fn get_children_by_type(&self, typ: AttributeTypeId) -> Vec<Self> {
        let mut result = Vec::new();
        for i in 0..self.child_count() {
            let c = self.child(i);
            if c.type_of() == typ {
                result.push(c);
            }
        }
        result
    }

    /// Get first child of a given type Returns the default error the attribute
    /// cannot be found.
    fn get_child_by_type(&self, typ: AttributeTypeId) -> Self {
        let children = self.get_children_by_type(typ);
        if children.is_empty() {
            Self::invalid()
        } else {
            children[0].clone()
        }
    }

    /// Returns the Parent [Attribute] for the `attribute`, or
    /// [AttributeStoreError] in case of errors
    fn parent(&self) -> Self;

    /// Returns the first found parent of  [Attribute] for the `attribute`, or
    /// [AttributeStoreError] in case of errors
    fn get_first_parent_with_type(&self, type_id: AttributeTypeId) -> Option<Self>;

    /// Await a change of the desired value.
    /// ```rust, no_run
    /// # use unify_middleware::unify_attribute_store::*;
    /// # use unify_middleware::contiki::*;
    /// # use futures::stream::StreamExt;
    /// # let attribute = AttributeStore::new().unwrap().root().add::<u32>(123,None, None).unwrap();
    /// async fn await_desired_change(attribute: Attribute) {
    ///     let val = attribute.desired_changed::<u32>().next().await.unwrap();
    ///     println!("desired value changed :{:?}", val);
    /// }
    ///
    /// contiki_spawn(await_desired_change(attribute));
    /// println!("initiating change");
    /// attribute.set_desired(Some(11));
    /// ```
    fn desired_changed<T>(
        &self,
    ) -> LocalBoxStream<'static, Result<(T, AttributeEventType), AttributeStoreError>>
    where
        T: 'static + DeserializeOwned + Any + Unpin;

    /// Await a change of the reported value.
    /// ```rust, no_run
    /// # use unify_middleware::unify_attribute_store::*;
    /// # use unify_middleware::contiki::*;
    /// # use futures::stream::StreamExt;
    /// # let attribute = AttributeStore::new().unwrap().root().add::<u32>(123,None, None).unwrap();
    /// async fn await_reported_change(attribute: Attribute) {
    ///     let val = attribute.reported_changed::<u32>().next().await.unwrap();
    ///     println!("reported value changed :{:?}", val);
    /// }
    ///
    /// contiki_spawn(await_reported_change(attribute));
    /// println!("initiating change");
    /// attribute.set_reported(Some(11));
    /// ```
    fn reported_changed<T>(
        &self,
    ) -> LocalBoxStream<'static, Result<(T, AttributeEventType), AttributeStoreError>>
    where
        T: 'static + DeserializeOwned + Any + Unpin;

    fn get_desired_or_reported<T: 'static + DeserializeOwned + Any>(
        &self,
    ) -> Result<T, AttributeStoreError> {
        self.get_desired().or_else(|_| self.get_reported())
    }

    /// Returns desired value of the `attribute` or [AttributeStoreError] in
    /// case of errors.
    ///
    /// The desired value from the attribute is deserialized into type `T`, e.g.
    /// if `T` is a String type and the desired value stored is a C String a
    /// String will be returned, if the desired value isn't a C String it will
    /// return an [AttributeStoreError].
    fn get_desired<T: 'static + DeserializeOwned + Any>(&self) -> Result<T, AttributeStoreError>;

    /// Returns reported value of the `attribute` or [AttributeStoreError] in
    /// case of errors.
    ///
    /// See [AttributeTrait::get_desired] as the functionality is the same.
    fn get_reported<T: 'static + DeserializeOwned + Any>(&self) -> Result<T, AttributeStoreError>;

    /// Updates the reported value. The value is optional, meaning that the
    /// value can be unset by passing [None].
    fn set_reported<T: Serialize + Any>(&self, value: Option<T>)
        -> Result<(), AttributeStoreError>;

    /// Updates the desired value. The value is optional, meaning that the value
    /// can be unset by passing [None].
    fn set_desired<T: Serialize + Any>(&self, value: Option<T>) -> Result<(), AttributeStoreError>;

    fn get_child_by_type_and_value<T>(
        &self,
        typ: AttributeTypeId,
        value_to_find: &Option<T>,
    ) -> Self
    where
        T: 'static + DeserializeOwned + Eq + Any,
    {
        for c in self.get_children_by_type(typ) {
            if &c.get_reported::<T>().ok() == value_to_find {
                return c;
            }
        }
        Self::invalid()
    }

    /// Iterator that iterates over the attribute store tree breadth first,
    /// starting from the self node.
    fn iter(&self) -> AttributeStoreIterator<Self>;

    /// Iterator that iterates over the attribute store tree, starting from the
    /// self node. [iter_dfs] searches the tree depth first rather then breadth
    /// first.
    fn iter_dfs(&self) -> AttributeStoreIteratorDFS<Self>;

    /// Add a child attribute to the attribute store The `desired` and
    /// `reported` values are optional, meaning if they are set to [Some] the
    /// value will be stored in `desired` and `reported` respectively. If they
    /// are set to [None] the attribute value will remain unset.
    fn add<T: Serialize + Any>(
        &self,
        type_id: AttributeTypeId,
        reported: Option<T>,
        desired: Option<T>,
    ) -> Result<Self, AttributeStoreError>;

    ///
    /// This function will update a reported and desired value of an existing
    /// attribute withe the same type id and reported value exists, otherwise it
    /// will do the same a self.add
    ///
    fn emplace<T>(
        &self,
        type_id: AttributeTypeId,
        reported: Option<T>,
        desired: Option<T>,
    ) -> Result<Self, AttributeStoreError>
    where
        T: 'static + Serialize + DeserializeOwned + Eq + Any,
    {
        let child = self.get_child_by_type_and_value(type_id, &reported);
        if child.valid() {
            child.set_reported(reported)?;
            child.set_desired(desired)?;
            Ok(child)
        } else {
            self.add(type_id, reported, desired)
        }
    }
}
