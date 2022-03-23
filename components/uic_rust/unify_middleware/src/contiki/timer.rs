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
extern crate alloc;
use alloc::rc::{Rc, Weak};
use core::{convert::TryInto, ffi::c_void, task::Waker, time::Duration};
use futures::{future::LocalBoxFuture, Future};
use std::sync::Mutex;
use unify_contiki_sys::*;
use unify_log_sys::*;

use super::TimerTrait;

declare_app_name!("timer_rs");

/// Timer implementation based on contiki's ctimer.
///
/// # Usage
///
/// ```rust, no_run
/// # use core::time::Duration;
/// # use unify_middleware::contiki;
/// # use unify_middleware::contiki::*;
/// async fn timeout_task(timer: contiki::Timer) {
///   timer.on_timeout(Duration::from_secs(5)).await;
///   println!("timeout occurred!");
/// }
///
/// let timer = contiki::Timer::new();
/// contiki_spawn(timeout_task(timer.clone()));
///
/// while !timer.expired() {
///   println!("waiting for timeout..");
/// }
/// ```
#[derive(Clone, Debug)]
pub struct Timer {
    timer_handle: Rc<ctimer>,
    running: Rc<Mutex<bool>>,
}

#[derive(Debug, PartialEq)]
pub struct TimeoutSuccess;
unsafe impl Sync for Timer {}

impl Timer {
    /// Create a new timer object. Use [on_timeout] to start and await the
    /// timer.
    pub fn new() -> Self {
        // INIT_CTIMERS.call_once(|| ctimer_init());
        Timer {
            timer_handle: Rc::new(ctimer::default()),
            running: Rc::new(Mutex::new(false)),
        }
    }
}

impl TimerTrait for Timer {
    /// Await a timeout. can only be called from a async func or spawned using
    /// [contiki_spawn].
    fn on_timeout(&self, timeout: Duration) -> LocalBoxFuture<'static, Option<TimeoutSuccess>> {
        // Set running to true
        *self.running.lock().unwrap() = true;
        Box::pin(CTimerFuture::new(
            timeout,
            self.timer_handle.clone(),
            self.running.clone(),
        ))
    }

    /// Verify if the timer already expired.
    fn expired(&self) -> bool {
        let ptr = Rc::as_ptr(&self.timer_handle) as *mut _;
        unsafe { ctimer_expired(ptr) != 0 }
    }

    /// This function resets the timer with the same interval that was given on
    /// construction. The start point of the interval is the exact time that the
    /// timer last expired. Therefore, this function will cause the timer to be
    /// stable over time, unlike the [restart] function.
    fn reset(&self) {
        let ptr = Rc::as_ptr(&self.timer_handle) as *mut _;
        unsafe { ctimer_reset(ptr) }
    }

    /// This function restarts the timer with the same interval that was given
    /// on construction. The callback timer will start at the current time.
    /// > note A periodic timer will drift if this function is used to reset it.
    /// For periodic timers, use the [reset] function instead.
    fn restart(&self) {
        let ptr = Rc::as_ptr(&self.timer_handle) as *mut _;
        unsafe { ctimer_restart(ptr) }
    }

    /// Stop the current timer. This also abort any current awaited timeout
    /// calls.
    fn stop(&mut self) {
        if !self.expired() {
            let ptr = Rc::as_ptr(&self.timer_handle) as *mut _;
            let waker_ptr = (*self.timer_handle).ptr;
            // Set running to false, this is shared with the timer future.
            *self.running.lock().unwrap() = false;
            unsafe {
                ctimer_stop(ptr);
                wake_waker(waker_ptr);
            }
        }
    }
}

impl Drop for Timer {
    fn drop(&mut self) {
        self.stop();
    }
}

impl PartialEq for Timer {
    fn eq(&self, other: &Self) -> bool {
        Rc::as_ptr(&self.timer_handle) == Rc::as_ptr(&other.timer_handle)
    }
}

struct CTimerFuture {
    timeout: Duration,
    waker: Option<Weak<Waker>>,
    timer_handle: Rc<ctimer>,
    running: Rc<Mutex<bool>>,
}

impl CTimerFuture {
    pub fn new(timeout: Duration, timer_handle: Rc<ctimer>, running: Rc<Mutex<bool>>) -> Self {
        CTimerFuture {
            timeout,
            waker: None,
            timer_handle,
            running,
        }
    }
}

impl Future for CTimerFuture {
    type Output = Option<TimeoutSuccess>;

    fn poll(
        self: std::pin::Pin<&mut Self>,
        cx: &mut std::task::Context<'_>,
    ) -> std::task::Poll<Self::Output> {
        match &self.waker {
            None => {
                let waker = Rc::new(cx.waker().clone());
                let this = self.get_mut();
                this.waker = Some(Rc::downgrade(&waker));
                // We are passing the waker reference to C, the Rc::into_raw
                // increments the reference counter for the waker.
                let weak_ctimer = Rc::downgrade(&this.timer_handle);
                unsafe {
                    ctimer_set(
                        Weak::into_raw(weak_ctimer) as *mut _,
                        (this.timeout.as_secs() * CLOCK_SECOND as u64)
                            .try_into()
                            .unwrap(),
                        Some(wake_waker),
                        Rc::into_raw(waker) as *mut c_void,
                    );
                }
                std::task::Poll::Pending
            }
            Some(weak) => {
                // If the C callback is called, all references to the waker
                // object are gone e.g. the waker object was destructed. this
                // way we know the timeout occurred.
                if weak.upgrade().is_some() {
                    std::task::Poll::Pending
                } else {
                    // If the timer has been stopped, the etimer.p is set to
                    // NULL by the CTimer (C) implementation, in that case
                    // return Ready(None) to signal timer has been cancelled
                    let running = *self.running.lock().unwrap();
                    if !running {
                        std::task::Poll::Ready(None)
                    } else {
                        std::task::Poll::Ready(Some(TimeoutSuccess))
                    }
                }
            }
        }
    }
}

unsafe extern "C" fn wake_waker(data: *mut std::os::raw::c_void) {
    if !data.is_null() {
        let rc = Rc::from_raw(data as *mut Waker);
        // try unwrap will move the object out of Rc. this is only possible if the
        // strong count == 1.
        assert_eq!(1, Rc::strong_count(&rc));
        let waker = Rc::try_unwrap(rc).expect("reference count of waker higher than one!");
        waker.wake();
    }
}
