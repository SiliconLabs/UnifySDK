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

use std::{
    collections::{BTreeMap, VecDeque},
    task::Waker,
};

use crate::waker_cache_trait::{WakePredicate, WakerCacheTrait};

/// Stores active listeners for attribute changes. It uses handles to do its
/// operations with. Only caches the last change for a given handle.
pub struct WakerCache<T> {
    wake_list: BTreeMap<u64, (Waker, WakePredicate<T>)>,
    cached_events: BTreeMap<u64, VecDeque<T>>,
    counter: u64,
}

impl<T> WakerCacheTrait<T> for WakerCache<T>
where
    T: Clone,
{
    fn new_event(&mut self, event: T) {
        for (handle, (waker, pred)) in &self.wake_list {
            if (pred)(&event) {
                self.cached_events
                    .entry(*handle)
                    .or_default()
                    .push_back(event.clone());
                waker.clone().wake();
            }
        }
    }

    /// returns None when there was an problem registering the internal
    /// attribute store callback
    fn register(&mut self, waker: Waker, pred: WakePredicate<T>) -> u64 {
        let handle = self.counter;
        self.counter += 1;
        self.wake_list.insert(handle, (waker, pred));
        handle
    }

    /// Gets the last available change for a given handle.
    fn take_change(&mut self, handle: u64) -> Option<T> {
        self.cached_events.get_mut(&handle)?.pop_front()
    }

    fn force_wake_all(&self) {
        for (waker, _pred) in self.wake_list.values() {
            waker.clone().wake();
        }
    }

    fn unsubscribe(&mut self, handle: u64) {
        self.wake_list.remove(&handle);
        self.cached_events.remove(&handle);
    }
}

impl<T> Default for WakerCache<T> {
    fn default() -> Self {
        Self {
            wake_list: BTreeMap::default(),
            cached_events: BTreeMap::default(),
            counter: 0,
        }
    }
}
