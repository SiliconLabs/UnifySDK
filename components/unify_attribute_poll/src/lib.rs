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

//! This library features an engine to poll reported values of attributes with a
//! given time interval. The Poll Engine determines if an attribute shall be
//! polled based on a timeout for the given attribute, in which case it clears
//! the `reported` value of the attribute. The Attribute Resolver then takes
//! over and tries to resolve the attribute.
//!
//! # Configuration
//!
//! The Poll Engine is configurable using the
//! [PollEngineConfig](poll_engine::PollEngineConfig) and supports:
//! * Backoff: The minimum time between 2 polls.
//! * Max Polls Per Hour: Maximum number of polls allowed during 1 hour.
//! * Default Poll Interval: In case an attribute is registered for polling with
//!   no interval, this interval is used.
//!
//! # [AttributePoll](attribute_poll::AttributePoll)
//!
//! To control the `PollEngine` the
//! [AttributePoll](attribute_poll::AttributePoll) object is used. More detailed
//! information can be found in the trait documentation of the [AttributePollTrait]
//!
//! # C interface
//!
//! this `unify_attribute_poll` module exposes an C interface. see `attribute_poll.h`
//! for more information.
//!
//! # Example
//!
//! ```rust, no_run
//! # use unify_attribute_poll::*;
//! # use unify_middleware::AttributeStoreTrait;
//! # use unify_middleware::unify_attribute_store::{Attribute, AttributeStore};
//! # use crate::unify_attribute_poll::attribute_poll_trait::AttributePollTrait;
//! # // Initialize the Poll Engine, this shall only be done once.
//! let attribute_poll = AttributePoll::default();
//! # attribute_poll.initialize(PollEngineConfig{backoff: 30, default_interval: 60, poll_mark_attribute_type: 0x18});
//! // Create "dummy" attribute for sake of the example.
//! let my_attribute = AttributeStore::new().unwrap().from_handle(42);
//! // Register my_attribute to be polled every 30 seconds
//! attribute_poll.register(my_attribute.clone(), 30);
//! // Restart the poll
//! attribute_poll.restart(my_attribute.clone());
//! // Stop polling the attribute
//! attribute_poll.deregister(my_attribute.clone());
//! ```
extern crate alloc;
mod attribute_poll;
pub mod attribute_poll_trait;
mod attribute_watcher;
mod attribute_watcher_trait;
mod c_interface;
mod poll_engine;
mod poll_entries;
mod poll_queue_trait;

#[doc(inline)]
pub use attribute_poll::AttributePoll;
use attribute_poll_trait::IntervalType;
use unify_attribute_store_sys::*;
use unify_middleware::AttributeTypeId;
use unify_sl_status_sys::*;

/// Configuration for the PollEngine
#[derive(Debug)]
pub struct PollEngineConfig {
    /// Backoff interval, i.e. the minimum time between 2 polls
    pub backoff: IntervalType,
    /// Default "ideal" poll interval
    pub default_interval: IntervalType,
    /// Attribute Store Type for the "Poll Mark" attribute
    pub poll_mark_attribute_type: AttributeTypeId,
}
