//! This module contains a single-threaded cooperative task executer driven by
//! the native uic_contiki library. It makes use of rust's async idiom to do its
//! control flow. Even though the name suggest a-synchronicity, tasks are
//! executed sequentially rather then concurrently.
//!
//! The contiki executor works in tandem with the native rust_process library.
//! It requests polls if progression can be made on one of the queue'ed tasks.
//! On a poll it will progress one of the tasks. If more progress can be made,
//! several more successive polls will be requested. The goal is to spread out
//! tasks as much as possible over different polls in order to avoid choking or
//! stalling the native contiki main-loop.
//!
//! Spawning tasks onto the contiki main-loop is done with the function
//! [contiki_spawn], [contiki_spawn_on_timeout].
//!
//! In this document, the definition of a 'task' is synonym for rust's future
//!
//! # Usage Examples
//!
//!```rust, no_run
//! # use unify_middleware::contiki::*;
//! # use std::time::Duration;
//!// arbitrary rust function
//!async fn foo() {
//!    println!("foo ");
//!    bar().await;
//!}
//!
//!async fn bar() {
//!    println!("bar!");
//!}
//!
//!fn main() {
//!    // example 1: call rust function
//!    contiki_spawn(foo());
//!
//!    // example 2: queue a block
//!    let func = async {
//!        println!("hello!");
//!    };
//!    contiki_spawn(func);
//!
//!    // example 3: queue a block that gets called after 2 seconds.
//!    // captured variables need to be moved into the function!
//!    let capture_var = 2_u8;
//!    contiki_spawn_on_timeout(Duration::from_secs(2), async move {
//!        println!("after {} sec", capture_var);
//!    });
//!}
//!```
//!
//! # Static lifetime of Task's
//!
//! The futures that are scheduled onto the contiki executor are required to
//! have 'static lifetimes. This means that also variables are captured by
//! value! The following example does not compile:
//!
//!```rust, compile_fail
//! // does not compile
//! let foo = String::from("foo");
//! # use unify_middleware::contiki::*;
//! let func = async {
//!     println!("{}", foo);
//! };
//! contiki_spawn(func);
//!```
//!
//! move the variable into the function!
//!
//!```rust, no_run
//! # use unify_middleware::contiki::*;
//! let foo = String::from("foo");
//! let func = async move {
//!     println!("{}", foo);
//! };
//! contiki_spawn(func);
//! ```
//!
//! # Endless Futures
//!
//! It is possible to queue futures that cannot finish because they, for
//! example, contain an endless loop. Be aware that this is only possible when
//! your [Future] gives control back once in a while to the contiki executor. By
//! returning [Poll::Pending] or awaiting another task that returns
//! [Poll::Pending]. Failing to do so will stall the executor and therefore the
//! whole application. The contiki executor operates on a single thread and can
//! not run futures concurrently!
//!
//! The following example is ok, we give back control every time we are waiting
//! for the timer to timeout:
//!
//! ```rust, no_run
//! # use unify_middleware::*;
//! # use std::time::Duration;
//! # use crate::unify_middleware::contiki::TimerTrait;
//! async fn timer_task() {
//!     let timer = contiki::Timer::new();
//!     loop {
//!         // every time we await for the timeout we give control
//!         // back to the executor.
//!         timer.on_timeout(Duration::from_secs(2)).await;
//!     }
//! }
//! contiki::contiki_spawn(timer_task());
//! ```
//!
//! The following example is a case where we are not exiting the task. This task
//! will stall the whole application!
//!
//! ```rust, no_run
//! # use unify_middleware::*;
//! # use std::time::Duration;
//! async fn print_task() {
//!     loop {
//!         println!("hello world!");
//!     }
//! }
//! contiki::contiki_spawn(print_task());
//! ```
//!
//! Note that a [Future] that is able to complete its task in one poll, Will
//! immediate return the completed status, [Poll::Ready], and therefore will not
//! give control back. The following example also stalls the application:
//!
//! ```rust, no_run
//! # use unify_middleware::*;
//! async fn bar() {
//!     println!("bar");
//! }
//!
//! async fn foo() {
//!     loop {
//!         println!("foo");
//!         bar().await;
//!     }
//! }
//! contiki::contiki_spawn(foo());
//! ```

pub mod timer;
use core::time::Duration;
use futures::{future::LocalBoxFuture, stream::FuturesUnordered, StreamExt};
use once_cell::unsync::Lazy;
use std::{
    future::Future,
    ops::{Deref, DerefMut},
    task::{Context, Poll, RawWaker, RawWakerVTable, Waker},
};
#[doc(inline)]
pub use timer::*;
use unify_log_sys::*;
use unify_proc_macro::{as_extern_c, generate_extern_c};
declare_app_name!("rust_contiki");

// Implemented a custom waker based on a call that will trigger a process_poll
// call inside the native rust_process component. This way we are only
// triggering the contiki future executor when there is an task to be progressed
// on.
static VTABLE: RawWakerVTable = RawWakerVTable::new(vt_clone, execute_wake, execute_wake, vt_drop);

unsafe fn vt_clone(data: *const ()) -> RawWaker {
    RawWaker::new(data, &VTABLE)
}
unsafe fn vt_drop(_data: *const ()) {}
unsafe fn execute_wake(_data: *const ()) {
    unify_contiki_sys::request_poller_poll_request();
}

/// This is the queue containing all futures that need to be driven to
/// completion. On a native contiki poll, run_once will be called to progress
/// the first task that is ready to be progressed on
/// ([FuturesUnordered::poll_next_unpin]). Note that the items contained by
/// * the [FuturesUnordered] are of type [LocalBoxFuture]. This means that
///   futures dont need to have [Sync] requirements.
/// * Futures need to have a static lifetime.
static mut POOL: Lazy<FuturesUnordered<LocalBoxFuture<'static, ()>>> = Lazy::new(|| {
    unsafe { unify_contiki_sys::register_poll_handler(as_extern_c!(run_once)) };
    FuturesUnordered::new()
});

/// This function adds a task to the contiki task executor. see [contiki] for
/// more information on usage.
pub fn contiki_spawn(future: impl Future<Output = ()> + 'static) {
    unsafe {
        POOL.deref().push(Box::pin(future));
        unify_contiki_sys::request_poller_poll_request();
    }
}

/// ignoring sync requirements for static because we run single threaded
static mut SHUTDOWN_CALLBACKS: Vec<Box<dyn Fn()>> = Vec::new();

pub fn contiki_register_shutdown_callback<F>(callback: F)
where
    F: 'static + Fn(),
{
    unsafe {
        SHUTDOWN_CALLBACKS.push(Box::new(callback));
    }
}

/// Execute an task after a specified timeout. Be aware that this is not a
/// precise timeout and can have a error margin depending on the load of the
/// main loop. see [contiki] for more information on usage.
pub fn contiki_spawn_on_timeout(
    timeout: std::time::Duration,
    future: impl Future<Output = ()> + 'static,
) {
    contiki_spawn(async move {
        let timer = timer::Timer::new();
        timer.on_timeout(timeout).await;
        future.await;
    })
}

/// This function drives the task executor. It will try to make the minimal
/// progression as possible. It will request more poll's if more progression is
/// made. Otherwise do nothing. It will get called again in the case a waker of
/// one of the tasks triggered. (that will spawn a new request_poll).
#[generate_extern_c]
pub unsafe fn run_once() {
    let waker = Waker::from_raw(RawWaker::new(&(), &VTABLE));
    let mut cx = Context::from_waker(&waker);
    match POOL.deref_mut().poll_next_unpin(&mut cx) {
        // tasks in the queue, but not ready to progress on.
        Poll::Pending => (),
        // there is more to be processed. queue another poll request.
        Poll::Ready(Some(_)) => unify_contiki_sys::request_poller_poll_request(),
        Poll::Ready(None) => log_debug!("future stream ended."),
    }
}

#[no_mangle]
pub unsafe extern "C" fn rust_contiki_teardown() -> std::os::raw::c_int {
    for cb in SHUTDOWN_CALLBACKS.drain(..) {
        cb();
    }
    0
}

/// this might not be the correct place for this. Move it to a platform component
/// see UIC-1569 for more info
/// Ideally we want to have a place that contain all our platform interfaces and have
/// a place for all the concrete implementations, in this case contiki.
#[mockall::automock]
pub trait TimerTrait {
    fn on_timeout(&self, timeout: Duration) -> LocalBoxFuture<'static, Option<TimeoutSuccess>>;
    fn expired(&self) -> bool;
    fn reset(&self);
    fn restart(&self);
    fn stop(&mut self);
}
#[mockall::automock]
pub trait PlatformTrait: Sync + Send {
    fn clock_seconds(&self) -> u64;
    fn get_timer_object(&self) -> Box<dyn TimerTrait>;
}

#[derive(Debug, Default, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct ContikiPlatform {}

impl PlatformTrait for ContikiPlatform {
    fn clock_seconds(&self) -> u64 {
        unsafe { unify_contiki_sys::clock_seconds() as u64 }
    }

    fn get_timer_object(&self) -> Box<dyn TimerTrait> {
        Box::new(Timer::new())
    }
}
