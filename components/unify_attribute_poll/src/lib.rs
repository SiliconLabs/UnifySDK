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
//! # use unify_middleware::unify_attribute_store::Attribute;
//! # // Initialize the Poll Engine, this shall only be done once.
//! let attribute_poll = AttributePoll::default();
//! # attribute_poll.initialize(PollEngineConfig{backoff: 30, default_interval: 60});
//! // Create "dummy" attribute for sake of the example.
//! let my_attribute = Attribute::new_from_handle(42).unwrap();
//! // Register my_attribute to be polled every 30 seconds
//! attribute_poll.register(my_attribute.clone(), 30);
//! // Restart the poll
//! attribute_poll.restart(my_attribute.clone());
//! // Stop polling the attribute
//! attribute_poll.deregister(my_attribute.clone());
//! ```
//!
extern crate alloc;
mod attribute_poll;
mod attribute_watcher;
mod poll_engine;
mod poll_entries;

#[doc(inline)]
pub use attribute_poll::AttributePoll;
use unify_attribute_store_sys::*;
use unify_middleware::Attribute;
use unify_sl_status_sys::*;

/// Type for time intervals in seconds
type IntervalType = u32;

#[mockall::automock]
/// Interface for the Attribute Poll engine.
pub trait AttributePollTrait {
    /// initializes the `Poll Engine`. Secondarily it will set the state of to
    /// running. e.g. `resume` is not required to be explicitly called. With the
    /// configuration file the poll behavior can be influenced; The rules for
    /// timer scheduling are:
    /// * If deadline of `next` entry < `last_poll` + `backoff`, queue timeout
    ///   for in `backoff` seconds,
    /// * else queue timeout for deadline of `next` entry.
    fn initialize(&self, config: PollEngineConfig);
    /// Adds a [Attribute] to the `Poll Engine` with a given interval.
    /// Internally it will recalculate the next deadline and reschedule poll
    /// timeouts accordingly
    fn register(&self, attribute: Attribute, interval: IntervalType);
    /// removes an attribute from the `Poll Engine`. Does nothing if the entry
    /// does not exist.
    fn deregister(&self, attribute: Attribute);
    /// similar to register, only that no interval is given, meaning the attribute will be polled on the first available moment
    fn queue(&self, attribute: Attribute);
    /// re-inserts the attribute at the current time to the poll queue with the interval it was given at time of registering.
    fn restart(&self, attribute: Attribute);
    /// no attributes will be polled. When the `Poll Engine` is paused it can still receive other commands
    fn pause(&self);
    /// attributes will be polled again.
    fn resume(&self);
}

/// Configuration for the PollEngine
#[derive(Debug)]
pub struct PollEngineConfig {
    /// Backoff interval, i.e. the minimum time between 2 polls
    pub backoff: IntervalType,
    /// Default "ideal" poll interval
    pub default_interval: IntervalType,
}

/// C interface for the Attribute Poll Engine wrapping (inlcude/attribute_poll.h)
mod c_interface {
    use super::*;

    #[no_mangle]
    pub unsafe extern "C" fn attribute_poll_register(
        handle: attribute_store_node_t,
        interval: IntervalType,
    ) -> sl_status_t {
        if let Ok(attribute) = Attribute::new_from_handle(handle) {
            AttributePoll::default().register(attribute, interval);
            SL_STATUS_OK
        } else {
            SL_STATUS_FAIL
        }
    }

    #[no_mangle]
    pub unsafe extern "C" fn attribute_poll_deregister(
        handle: attribute_store_node_t,
    ) -> sl_status_t {
        if let Ok(attribute) = Attribute::new_from_handle(handle) {
            AttributePoll::default().deregister(attribute);
            SL_STATUS_OK
        } else {
            SL_STATUS_FAIL
        }
    }

    #[no_mangle]
    pub unsafe extern "C" fn attribute_poll_schedule(
        handle: attribute_store_node_t,
    ) -> sl_status_t {
        if let Ok(attribute) = Attribute::new_from_handle(handle) {
            AttributePoll::default().queue(attribute);
            SL_STATUS_OK
        } else {
            SL_STATUS_FAIL
        }
    }

    #[no_mangle]
    pub extern "C" fn attribute_poll_init() -> sl_status_t {
        // Fixme:Both backoff and default interval config should be parameters of
        // the init function instead of reading from config. (will be fixed in UIC-1526).
        // FYI: Currently, the Poll Engine is used only via ZPC.
        if let Ok(backoff) = unify_config_sys::config_get_as_int("zpc.poll.backoff") {
            if let Ok(default_interval) =
                unify_config_sys::config_get_as_int("zpc.poll.default_interval")
            {
                AttributePoll::default().initialize(PollEngineConfig {
                    backoff: backoff as u32,
                    default_interval: default_interval as u32,
                });
                return SL_STATUS_OK;
            }
        }
        SL_STATUS_FAIL
    }

    #[no_mangle]
    pub unsafe extern "C" fn attribute_poll_disable() -> sl_status_t {
        AttributePoll::default().pause();
        SL_STATUS_OK
    }

    #[no_mangle]
    pub unsafe extern "C" fn attribute_poll_enable() -> sl_status_t {
        AttributePoll::default().resume();
        SL_STATUS_OK
    }
}
