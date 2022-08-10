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

//! The AttributeStore is an non binary tree structure which can store any type.
//! Nodes are unsorted, Parent nodes can have multiple child nodes, child nodes
//! can only have one parent. The [AttributeTrait] trait defines the
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
//! [AttributeTrait] expose 2 iterators to iterate over nodes in the tree.
//! * [AttributeTrait::iter] iterates breath first over the nodes
//! * [AttributeTrait::iter_dfs] iterates depth first over the nodes
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
//! # let root = AttributeStore::new().unwrap().root();
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
//!             let foo_attribute = AttributeStore::new().unwrap().root()
//!                 .get_child_by_type(12);
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

pub mod attribute;
pub mod attribute_callbacks;
pub mod attribute_event;
mod attribute_iterator;
pub mod attribute_mock;
pub mod attribute_store;
pub mod attribute_store_error;
mod attribute_store_instance;
pub mod attribute_store_trait;
mod attribute_stream;
pub mod attribute_stream_ext;
pub mod attribute_trait;

#[doc(inline)]
pub use attribute::*;
#[doc(inline)]
pub use attribute_callbacks::*;
#[doc(inline)]
pub use attribute_event::*;
#[doc(inline)]
pub use attribute_iterator::*;
#[doc(inline)]
pub use attribute_store::*;
#[doc(inline)]
pub use attribute_store_error::*;
#[doc(inline)]
pub use attribute_store_instance::*;
#[doc(inline)]
pub use attribute_store_trait::*;
#[doc(inline)]
pub use attribute_store_trait::*;
#[doc(inline)]
pub use attribute_stream::*;
#[doc(inline)]
pub use attribute_trait::*;
