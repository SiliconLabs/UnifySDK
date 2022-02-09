// License
// <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

use std::{
    collections::{BTreeMap, VecDeque},
    task::Waker,
};

use crate::AttributeEvent;
use unify_log_sys::*;

declare_app_name!("attribute_stream");

// Type that is used to evaluate if the changed attribute is of interest for the
// attribute listener.
type WakePredicate = Box<dyn Fn(&AttributeEvent) -> bool>;

/// Trait interface for the attribute store cache that holds and book keeps
/// attribute store changes. Uses u64 as handles.
#[mockall::automock]
pub trait AttributeStreamCache {
    /// input function of the cache. Will be called by the C callback of the
    /// attribute store
    fn new_event(&mut self, event: AttributeEvent);
    /// register a new listener for attribute changes in the attribute store.
    /// Arguments:
    /// *   waker           to be called if the incoming change is of interest
    ///                     of the listener
    /// *   pred            predicate to determine if the listener is interested
    ///                     in the change.
    fn register(&mut self, waker: Waker, pred: WakePredicate) -> u64;
    /// With this function the change can be retrieved following the waker
    /// signalling the wake call. Note that after calling this function the
    /// change in the cache is being set to None. e.g. Calling the function
    /// twice is not possible without a new change being produce in between
    fn take_change(&mut self, handle: u64) -> Option<AttributeEvent>;
    /// Trigger a poll for all subscribed handles.
    fn force_wake_all(&self);
    /// remove cache entries related to the handle
    fn unsubscribe(&mut self, handle: u64);
}

/// Stores active listeners for attribute changes. It uses handles to do its
/// operations with. Only caches the last change for a given handle.
pub struct AttributeChangeListener {
    wake_list: BTreeMap<u64, (Waker, WakePredicate)>,
    cached_events: BTreeMap<u64, VecDeque<AttributeEvent>>,
    counter: u64,
}

impl AttributeStreamCache for AttributeChangeListener {
    fn new_event(&mut self, event: AttributeEvent) {
        for (handle, (waker, pred)) in &self.wake_list {
            if (pred)(&event) {
                self.cached_events
                    .entry(*handle)
                    .or_default()
                    .push_back(event);
                waker.clone().wake();
            }
        }
    }

    /// returns None when there was an problem registering the internal
    /// attribute store callback
    fn register(&mut self, waker: Waker, pred: WakePredicate) -> u64 {
        let handle = self.counter;
        self.counter += 1;
        self.wake_list.insert(handle, (waker, pred));
        handle
    }

    /// Gets the last available change for a given handle.
    fn take_change(&mut self, handle: u64) -> Option<AttributeEvent> {
        self.cached_events.get_mut(&handle)?.pop_front()
    }

    fn force_wake_all(&self) {
        log_debug!("waking all attribute change streams");
        for (waker, _pred) in self.wake_list.values() {
            waker.clone().wake();
        }
    }

    fn unsubscribe(&mut self, handle: u64) {
        self.wake_list.remove(&handle);
        self.cached_events.remove(&handle);
    }
}

impl Default for AttributeChangeListener {
    fn default() -> Self {
        Self {
            wake_list: BTreeMap::<u64, (Waker, WakePredicate)>::new(),
            cached_events: Default::default(),
            counter: 0,
        }
    }
}
