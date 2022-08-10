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

use crate::PollEngineConfig;
use unify_middleware::Attribute;

/// Type for time intervals in seconds
pub type IntervalType = u32;

/// This traits acts as a facade for the [PollEngine] which runs the actual
/// attribute polling logic. There can be multiple instances of the objects
/// implementing this trait but they will always control only one unique
/// [PollEngine]. Objects implementing this trait are therefore cheap to
/// construct.
/// 
#[mockall::automock]
pub trait AttributePollTrait {
    /// starts the `Poll Engine` and puts its state to running. e.g. `resume` is
    /// not required to be explicitly called. Initialize will return immediately
    /// when its called more as once. With the configuration file the poll
    /// behavior can be influenced; The rules for timer scheduling are:
    /// * If deadline of `next` entry < `last_poll` + `backoff`, queue timeout
    ///   for in `backoff` seconds,
    /// * else queue timeout for deadline of `next` entry.
    ///
    /// # Arguments
    ///
    /// * `config`  persists as long as the application is alive. Config cannot
    ///   be re-submitted by calling the `.initialize()` twice
    ///
    fn initialize(&self, config: PollEngineConfig);
    /// Adds a [Attribute] to the `Poll Engine` with a given interval.
    /// Internally it will recalculate the next deadline and reschedule poll
    /// timeouts accordingly.
    ///
    /// # Arguments
    ///
    /// * `attribute`   a valid attribute corresponding a zwave cluster property
    /// * `interval`    interval is in seconds and should be relative from time
    ///   now
    /// 
    fn register(&self, attribute: Attribute, interval: IntervalType);
    /// removes an attribute from the `Poll Engine`. Does nothing if the entry
    /// does not exist.
    /// 
    /// # Arguments
    ///
    /// * `attribute`   a valid attribute corresponding a zwave cluster
    ///   property. Attribute must exists in the poll engine
    fn deregister(&self, attribute: Attribute);
    /// registers the attribute at the front of the queue. The attribute will be
    /// polled on a first available moment.
    /// 
    /// # Arguments
    ///
    /// * `attribute`   a valid attribute corresponding a zwave cluster property
    fn schedule_now(&self, attribute: Attribute);
    /// re-inserts the attribute to the poll queue with the interval it was
    /// given at time of registering. time of poll = current time + interval.
    /// 
    /// # Arguments
    ///
    /// * `attribute`   a valid attribute corresponding a zwave cluster property
    fn restart(&self, attribute: Attribute);
    /// no attributes will be polled. When the attribute poller is paused it can
    /// still receive other commands.
    /// 
    /// Note that, depending on the configured back-off time, it can take a
    /// longer time for the attribute poller to catch up polling attributes if
    /// the attribute poller is paused for a longer period of time. This means
    /// that attributes will be polled later as expected.
    fn pause(&self);
    /// Resume polling attribute
    fn resume(&self);
}
