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
use std::task::Waker;

// Type that is used to evaluate if the changed is of interest for the
// attribute listener.
pub type WakePredicate<T> = Box<dyn Fn(&T) -> bool>;

/// Trait interface for a cache that holds and book-keeps changes of any Type T.
/// * It caches new changes in a FIFO.
/// * any type of K can be used
///
/// This trait interface is used in scenarios where `Future` or `Stream` objects
/// need to be made on top of native code.
#[mockall::automock]
pub trait WakerCacheTrait<T>
where
    T: Clone,
{
    /// input function of the cache. Will be called by the C callback of the
    /// attribute store
    fn new_event(&mut self, event: T);
    /// register a new listener for changes.
    /// Arguments:
    /// *   waker           to be called if the incoming change is of interest
    ///                     of the listener
    /// *   pred            predicate to determine if the listener is interested
    ///                     in the change.
    fn register(&mut self, waker: Waker, pred: WakePredicate<T>) -> u64;
    /// With this function the change can be retrieved following the waker
    /// signalling the wake call. Note that after calling this function the
    /// change in the cache is being set to None. e.g. Calling the function
    /// twice is not possible without a new change being produce in between
    fn take_change(&mut self, handle: u64) -> Option<T>;
    /// Trigger a poll for all subscribed handles.
    fn force_wake_all(&self);
    /// remove cache entries related to the handle
    fn unsubscribe(&mut self, handle: u64);
}
