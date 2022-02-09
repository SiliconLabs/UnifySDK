// License
// <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

//! The AttributeStore is an non binary tree structure which can store any type.
//! Nodes are unsorted, Parent nodes can have multiple child nodes, child nodes
//! can only have one parent. The [AttributeStoreTrait] trait defines the
//! operations on the attribute store.
//!
//! ## Storing data
//!
//! The Attribute Store serializes and deserializes data using [serde]. It can
//! store any type as long as this type implements [serde::Serialize] and
//! [serde::Deserialize].
//!
//! ## Iterating over the Attribute Store
//!
//! [AttributeStoreTrait] expose 2 iterators to iterate over nodes in the tree.
//! * [AttributeStoreTrait::iter] iterates breath first over the nodes
//! * [AttributeStoreTrait::iter_dfs] iterates depth first over the nodes
//!
//! ## Stale and deleted nodes
//!
//! One of the downside of having the [AttributeNode] abstraction is that it
//! allows for stale nodes, where this is not immediately visible. e.g
//!
//! ```rust, no_run
//! # use unify_middleware::AttributeStoreError::*;
//! # use unify_middleware::*;
//! # const HOME_ID: AttributeTypeId = 123;
//! #
//! # let root = Attribute::root().unwrap();
//! let node = root.add(HOME_ID,Some(42u32), None).unwrap();
//! let clone = node.clone();
//! node.delete().unwrap();
//!
//! // node is deleted from the attribute store from this point onwards.
//! // both nodes, node and clone, are not valid anymore!
//! assert_eq!(clone.get_desired::<u32>().unwrap_err(), StaleOrNoneExisting);
//! ```
//!
//! # Examples
//! The following example shows some simple usage of the AttributeStore. In this
//! example we construct an attributestore, add 2 nodes, then we try to find the
//! second node by matching a property of an attribute type, finally we delete
//! the nodes.
//!
//! ```rust, no_run
//! // TODO: Write usage of the attribute store. Adding some children,
//! // finding some children and updating their desired values.
//! ```
//! ## Zwave Command Class Implementation Example
//!
//!```rust, no_run
//! # use unify_middleware::contiki::*;
//! # use unify_middleware::*;
//! # use unify_middleware::uic_attribute_store::*;
//! # use std::future::Future;
//! # use futures::stream::StreamExt;
//! # struct FooFrame{}
//! # let FooType: u32 = 13;
//! struct FooCommandClass {}
//!
//! impl FooCommandClass {
//!     fn on_control(&self, frame: FooFrame) {}
//!
//!     fn on_support(&self, frame: FooFrame) {}
//!
//!     fn do_things(&mut self) {}
//!
//!     /// Note that on_init passes `self` by value. This means that the actual
//!     /// command class "lives" inside of the contiki task. As its moved by value
//!     /// inside the async block. This also means that the command class will be
//!     /// dropped as soon as the contiki task goes out of scope. Even though this
//!     /// contiki task does not exit, because of the endless loop, it will *not*
//!     /// block the contiki message-queue. The tasks gives up control of the task
//!     /// by waiting for the attribute_store call to give use some data. see more
//!     /// [contiki].
//!     ///
//!     /// Since the actual object is self contained inside the async block, it
//!     /// should get easier to build application logic for this CC.
//!     ///
//!     /// in this example i placed the contiki task inside of the struct, but
//!     /// it might turn out to be more practical to do it outside in a loose task.
//!     fn on_init(mut self) {
//!         contiki_spawn(async move {
//!             let foo_attribute = Attribute::root()
//!                 .unwrap()
//!                 .get_child_by_type(12)
//!                 .unwrap();
//!             let mut changes_stream = foo_attribute.desired_changed::<u32>();
//!
//!             loop {
//!                 // only do something when the desired value changed. ignore the
//!                 // actual value it changed to.
//!                 if let Some(Ok((_, AttributeEventType::ATTRIBUTE_UPDATED))) =
//!                     changes_stream.next().await
//!                 {
//!                     self.do_things();
//!                 }
//!             }
//!         });
//!     }
//! }
//! ```
//!

mod attribute_iterator;
mod attribute_stream;
mod attribute_stream_cache;
pub mod attribute_stream_ext;
pub mod uic_attribute_store;

#[doc(inline)]
pub use attribute_iterator::*;
#[doc(inline)]
pub use attribute_stream::*;
#[doc(inline)]
pub use attribute_stream_cache::*;
use futures::stream::LocalBoxStream;
use serde::{de::DeserializeOwned, Serialize};
use std::any::Any;
use std::fmt::Debug;
pub use uic_attribute_store::rust_attribute_store_init;
#[doc(inline)]
pub use uic_attribute_store::*;
/// This type specifies the actual used storage type inside of the attribute store.
/// Since they are related to the native uic_datastore component the choice was made to explicitly
/// define them, rather then making use of the already present [std::any::TypeId] inside of Rust.
pub type AttributeTypeId = u32;

#[derive(Clone, Copy, Debug, PartialEq, Hash, Eq)]
pub struct AttributeEvent {
    pub attribute: Attribute,
    pub event_type: AttributeEventType,
    pub value_state: AttributeValueState,
}

impl Default for AttributeEvent {
    fn default() -> Self {
        Self {
            attribute: Default::default(),
            event_type: AttributeEventType::ATTRIBUTE_DELETED,
            value_state: AttributeValueState::DESIRED_OR_REPORTED_ATTRIBUTE,
        }
    }
}

/// see [module documentation](unify_attribute_store) for more information on the usage of this interface.
pub trait AttributeStoreTrait: Debug + Copy
where
    Self: Sized,
{
    type Attribute;
    type Error: Default;

    /// Returns true if `self` is a valid attribute in the attribute store, else false.
    fn exists(&self) -> bool;

    /// Returns the Parent [Attribute] for the `attribute`, or
    /// [AttributeStoreError] in case of errors
    fn get_parent(&self) -> Result<Self, Self::Error>;

    /// Returns the first found parent of  [Attribute] for the `attribute`, or
    /// [AttributeStoreError] in case of errors
    fn get_first_parent_with_type(
        &self,
        type_id: AttributeTypeId,
    ) -> Result<Self::Attribute, Self::Error>;

    /// Returns desired value of the `attribute` or [AttributeStoreError] in
    /// case of errors.
    ///
    /// The desired value from the attribute is deserialized into type `T`, e.g.
    /// if `T` is a String type and the desired value stored is a C String a
    /// String will be returned, if the desired value isn't a C String it will
    /// return an [AttributeStoreError].
    fn get_desired<T: 'static + DeserializeOwned + Any>(&self) -> Result<T, Self::Error>;

    /// Returns reported value of the `attribute` or [AttributeStoreError] in case of errors.
    ///
    /// See [AttributeStoreTrait::get_desired] as the functionality is the same.
    fn get_reported<T: 'static + DeserializeOwned + Any>(&self) -> Result<T, Self::Error>;

    /// Updates the reported value.
    /// The value is optional, meaning that the value can be unset by passing [None].
    fn set_reported(&self, value: Option<impl Serialize + Any>) -> Result<(), Self::Error>;

    /// Updates the desired value.
    /// The value is optional, meaning that the value can be unset by passing [None].
    fn set_desired(&self, value: Option<impl Serialize + Any>) -> Result<(), Self::Error>;

    /// Delete an attribute.
    /// Note: This function takes `self` by value. it does not exist anymore
    /// after calling this function.
    fn delete(self) -> Result<(), Self::Error>;

    /// Add a child attribute to the attribute store The `desired` and `reported`
    /// values are optional, meaning if they are set to [Some] the value will be
    /// stored in `desired` and `reported` respectively. If they are set to
    /// [None] the attribute value will remain unset.
    fn add<T: Serialize + Any>(
        &self,
        type_id: AttributeTypeId,
        reported: Option<T>,
        desired: Option<T>,
    ) -> Result<Self, Self::Error>;

    /// Return the Type Id of `self`.
    fn type_of(&self) -> AttributeTypeId;

    /// Iterator that iterates over the attribute store tree breadth first,
    /// starting from the self node.
    fn iter(&self) -> Box<dyn Iterator<Item = Self::Attribute>>;

    /// Iterator that iterates over the attribute store tree, starting from the
    /// self node. [iter_dfs] searches the tree depth first rather then breadth
    /// first.
    fn iter_dfs(&self) -> Box<dyn Iterator<Item = Self::Attribute>>;

    /// Await a change of the desired value.
    /// ```rust, no_run
    /// # use unify_middleware::unify_attribute_store::*;
    /// # use unify_middleware::contiki::*;
    /// # use futures::stream::StreamExt;
    /// # let attribute = Attribute::root().unwrap().add::<u32>(123,None, None).unwrap();
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
    ) -> LocalBoxStream<'static, Result<(T, AttributeEventType), Self::Error>>
    where
        T: 'static + DeserializeOwned + Any + Unpin;

    /// Await a change of the reported value.
    /// ```rust, no_run
    /// # use unify_middleware::unify_attribute_store::*;
    /// # use unify_middleware::contiki::*;
    /// # use futures::stream::StreamExt;
    /// # let attribute = Attribute::root().unwrap().add::<u32>(123,None, None).unwrap();
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
    ) -> LocalBoxStream<'static, Result<(T, AttributeEventType), Self::Error>>
    where
        T: 'static + DeserializeOwned + Any + Unpin;

    /// Await a change from the attribute store for a given predicate.
    ///
    /// # Arguments
    ///
    /// * `predicate`   -   Define the criteria for when to trigger an
    ///   attribute_change.
    ///
    /// # Returns
    ///
    /// A future that returns tuple with the change attribute + eventtype of
    /// that change The future returns an error in case there was an internal
    /// error or the attribute change callbacks could not be registered.
    ///
    /// # Examples
    ///
    /// ```rust, no_run
    ///# async fn wrapper_func() {
    ///#     use unify_middleware::contiki::*;
    ///#     use unify_middleware::unify_attribute_store::*;
    ///#     use futures::stream::StreamExt;
    /// let added_predicate = |change: &AttributeEvent| {
    ///     change.event_type == AttributeEventType::ATTRIBUTE_CREATED
    /// };
    /// let mut added_stream = Attribute::attribute_changed(added_predicate);
    /// if let Some(attr) = added_stream.next().await {
    ///     println!("attribute {:?} added!", attr);
    /// }
    ///
    /// let root_changed =
    ///     |change: &AttributeEvent| change.attribute == Attribute::root().unwrap();
    /// let mut root_change_stream = Attribute::attribute_changed(root_changed);
    /// if let Some(_) = root_change_stream.next().await {
    ///     println!("change of root!");
    /// }
    ///# }
    /// ```
    fn attribute_changed(
        predicate: impl Fn(&AttributeEvent) -> bool + 'static,
    ) -> LocalBoxStream<'static, AttributeEvent>;
    ////////////////////////////// Helper functions ////////////////////////////////////

    fn get_desired_or_reported<T: 'static + DeserializeOwned + Any>(
        &self,
    ) -> Result<T, Self::Error> {
        match self.get_desired() {
            Ok(v) => Ok(v),
            _ => self.get_reported(),
        }
    }
}

/// Trait that abstracts the loading of children of the attribute store. Using
/// this trait we enable our attribute store iterators to be generic.
pub trait AttributeStoreExtTrait: AttributeStoreTrait
where
    Self: Sized,
{
    fn child_count(&self) -> usize;
    fn child(&self, index: usize) -> Result<Self, Self::Error>;

    /// Get all direct children of a given type, if the attribute
    /// has no children the default error is returned.
    ///
    fn get_children_by_type(&self, typ: AttributeTypeId) -> Result<Vec<Self>, Self::Error> {
        let mut result = Vec::new();
        for i in 0..self.child_count() {
            let c = self.child(i)?;
            if c.type_of() == typ {
                result.push(c);
            }
        }
        if result.len() == 0 {
            Err(Default::default())
        } else {
            Ok(result)
        }
    }

    /// Get first child of a given type
    /// Returns the default error the attribute cannot be found.
    fn get_child_by_type(&self, typ: AttributeTypeId) -> Result<Self, Self::Error> {
        let children = self.get_children_by_type(typ)?;
        Ok(children[0])
    }

    /// Get first child of a given type and reported value value.
    ///
    fn get_child_by_type_and_value<T: 'static + DeserializeOwned + Eq + Any>(
        &self,
        typ: AttributeTypeId,
        value_to_find: &Option<T>,
    ) -> Result<Self, Self::Error> {
        for c in self.get_children_by_type(typ)? {
            if &c.get_reported::<T>().ok() == value_to_find {
                return Ok(c);
            }
        }
        Err(Default::default())
    }

    ///
    /// This function will update a reported and desired value of an existing
    /// attribute withe the same type id and reported value exists,
    /// otherwise it will do the same a self.add
    ///
    fn emplace<T: 'static + Serialize + DeserializeOwned + Eq + Any>(
        &self,
        type_id: AttributeTypeId,
        reported: Option<T>,
        desired: Option<T>,
    ) -> Result<Self, Self::Error> {
        let child = self.get_child_by_type_and_value(type_id, &reported);
        match child {
            Ok(child) => {
                child.set_reported(desired)?;
                child.set_desired(reported)?;
                Ok(child)
            }
            Err(_) => self.add(type_id, reported, desired),
        }
    }
}
