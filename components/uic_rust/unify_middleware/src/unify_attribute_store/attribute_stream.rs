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

use crate::{contiki::contiki_register_shutdown_callback, AttributeEvent};
use core::task::Poll::{Pending, Ready};
use futures::Stream;
use mockall_double::double;
use unify_attribute_store_sys::*;
use unify_log_sys::*;
use unify_sl_status_sys::*;
declare_app_name!("attribute_stream");

use unify_tools::waker_cache_trait::WakerCacheTrait;
#[double]
use wrapped_c::inner;

// lazy static that will get executed on the first time its called. it will
// return the return value, sl_status_t, every successive time.
static mut REGISTERED: Option<sl_status_t> = None;

/// Type that stores information about which value of the attribute changed.
/// Reported, Desired or both.
pub type AttributeValueState = attribute_store_node_value_state_t;

/// Type that stores if the changed attribute was added,removed or updated.
pub type AttributeEventType = attribute_store_change_t;

/// Static struct that book-keeps attribute change subscriptions.
static mut CHANGE_LISTENER: Option<Box<dyn WakerCacheTrait<AttributeEvent>>> = None;

#[allow(unused_variables)]
pub fn initialize_attribute_changes<Cache>(attr_cache: Cache) -> u32
where
    Cache: WakerCacheTrait<AttributeEvent> + 'static,
{
    unsafe {
        // workaround: other test-cases will call this init function
        // concurrently. Since we set the CHANGE_LISTENER explicitly
        // in the tests, ignore it in test builds.
        if cfg!(not(test)) {
            CHANGE_LISTENER = Some(Box::new(attr_cache));
        }

        let status =
            inner::register_callback(Some(uic_attribute_store_get_attribute_change_callback));

        if status == SL_STATUS_OK {
            contiki_register_shutdown_callback(|| {
                // Close all running streams by setting REGISTERED == None. next
                // time streams are polled they will return Ready(None) which
                // indicates it will close. by calling force_wake_all we are
                // triggering every open stream to be polled.
                REGISTERED = None;
                if let Some(cache) = &CHANGE_LISTENER {
                    cache.force_wake_all()
                }
            });
        }
        REGISTERED = Some(status);
        status
    }
}

/// Stream that awaits a series of inexhaustible attribute store updates for a
/// given predicate. Make sure that before using this stream the initialization
/// function is called ([initialize_attribute_changes]).
///
/// Detailed behavior of this stream is experimental, e.g. might be subject to
/// changes. The stream behaves as followed:
/// * On first poll, the waker from the context is set. no values produced.
///   [core::task::Poll::Pending] is returned.
/// * The stream is robust against intermittent polls that are called without an
///   actually wake call being transmitted. The stream will return
///   [core::task::Poll::Pending] in these cases.
/// * In the event the waker is being woken. We assume a value is ready to read
///   from the attribute store. If there is no value ready, we assume this is
///   one of the intermittent polls and return [core::task::Poll::Pending]
/// * On all calls were we have a new change available we return it with state
///   Ready(Some(..)).
/// * The waker is not reset after a poll is called. This is necessary in
///   environments where the context can change, due to tasks being pushed to
///   different threads. We don't have multi context environment, so we should
///   be good ?!
/// * Errors in cache bookkeeping as well as forgotten initialization will
///   result in the channel being closed.
/// * When the attribute store closes down or updates its state != Running
///   it will end and close down the stream.
pub struct AttributeChangedStream<T, Pred>
where
    Pred: 'static + Fn(&AttributeEvent) -> bool,
{
    predicate: Option<Box<Pred>>,
    change_handle: Option<u64>,
    conversion: fn(AttributeEvent) -> T,
}

impl<T, Pred> AttributeChangedStream<T, Pred>
where
    Pred: 'static + Fn(&AttributeEvent) -> bool,
{
    pub fn new(predicate: Pred, conversion: fn(AttributeEvent) -> T) -> Self {
        AttributeChangedStream {
            predicate: Some(Box::new(predicate)),
            change_handle: None,
            conversion,
        }
    }
}

impl<T, Pred> Stream for AttributeChangedStream<T, Pred>
where
    T: Unpin,
    Pred: 'static + Fn(&AttributeEvent) -> bool,
{
    type Item = T;

    fn poll_next(
        self: std::pin::Pin<&mut Self>,
        cx: &mut std::task::Context<'_>,
    ) -> std::task::Poll<Option<Self::Item>> {
        if let Some(exit) = has_initialization_errors::<T>() {
            return exit;
        }

        let cache;
        unsafe {
            cache = CHANGE_LISTENER
                .as_mut()
                .expect("CHANGE_LISTENER is valid. has_initialization_errors has validated this");
        }

        let this = self.get_mut();

        // The first time poll is called, we set the waker and subscribe for any
        // changes coming from the C-api. We know its the first time because the
        // change object hasn't been set yet.
        if this.change_handle.is_none() {
            this.change_handle = Some(
                cache.register(
                    cx.waker().clone(),
                    this.predicate
                        .take()
                        .expect("double registration of attribute listener"),
                ),
            );

            // From this point the stream is initialized and awaiting for the
            // waker to be called.
            return Pending;
        }

        let handle = this
            .change_handle
            .expect("this should not happen. Handle is always set on the previous lines");

        // On any successive calls we assume that the waker was called for this
        // stream object, which means there are values ready to be returned to
        // the awaiting caller. When this poll function is subsequently called
        // before the waker was actually called it will still return `Pending`.
        // The stream needs to be robust against these intermittent poll calls.
        if let Some(change) = cache.take_change(handle) {
            let data = (this.conversion)(change);
            Ready(Some(data))
        } else {
            Pending
        }
    }
}

fn has_initialization_errors<T>() -> Option<std::task::Poll<Option<T>>> {
    unsafe {
        if REGISTERED.is_none() || REGISTERED.unwrap() != SL_STATUS_OK {
            log_error!("attribute store callback not registered");
            return Some(Ready(None));
        }

        if CHANGE_LISTENER.is_none() {
            log_error!("attribute change initialize not called");
            return Some(Ready(None));
        }
    }
    None
}

impl<T, Pred> Drop for AttributeChangedStream<T, Pred>
where
    Pred: 'static + Fn(&AttributeEvent) -> bool,
{
    fn drop(&mut self) {
        unsafe {
            if CHANGE_LISTENER.as_mut().and(self.change_handle).is_some() {
                CHANGE_LISTENER
                    .as_mut()
                    .unwrap()
                    .unsubscribe(self.change_handle.unwrap());
            }
        }
    }
}

/// Actual C callback which is received from the C attribute store. It will loop
/// through the active listeners to see if one of the listeners is interested in
/// the change. If this is the case, the change will be send over that
/// corresponding channel via the sender and the entry is removed from the
/// listeners list.
#[no_mangle]
unsafe extern "C" fn uic_attribute_store_get_attribute_change_callback(
    event_data: *mut attribute_changed_event_t,
) {
    assert!(!event_data.is_null());
    let node_id = (*event_data).updated_node;
    let node_type = (*event_data).type_;

    let attribute =
        super::Attribute::new(node_id, node_type).expect("internal error constructing attribute");

    let change = AttributeEvent {
        event_type: (*event_data).change,
        value_state: (*event_data).value_state,
        attribute,
    };

    CHANGE_LISTENER
        .as_mut()
        .expect("initialize happened in order to get callbacks. we should never end here")
        .new_event(change);
}

mod wrapped_c {
    #![allow(dead_code)]
    #[mockall::automock]
    pub mod inner {
        use unify_attribute_store_sys::*;

        pub fn register_callback(
            callback_function: attribute_store_node_update_callback_t,
        ) -> sl_status_t {
            unsafe { attribute_store_register_callback(callback_function) }
        }
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::{contiki::rust_contiki_teardown, Attribute};
    use futures::task::noop_waker;
    use mockall::{
        predicate::{self, eq},
        Sequence,
    };
    use serial_test::serial;
    use std::{pin::Pin, task::Context};
    use unify_tools::waker_cache::WakerCache;
    use unify_tools::waker_cache_trait::MockWakerCacheTrait;

    fn teardown_change_listeners() {
        unsafe {
            CHANGE_LISTENER = None;
            REGISTERED = None;
        }
    }

    #[test]
    #[serial(change_listener)]
    fn register_callback_ok() {
        teardown_change_listeners();

        let ctx = inner::register_callback_context();
        ctx.expect().withf(|f| f.is_some()).return_const(0u32);

        unsafe {
            CHANGE_LISTENER = Some(Box::new(MockWakerCacheTrait::<AttributeEvent>::default()))
        };

        let mut listener = WakerCache::<AttributeEvent>::default();
        assert_eq!(0, listener.register(noop_waker(), Box::new(|_| true)));

        // also consecutive times
        assert_eq!(
            1,
            listener.register(
                noop_waker(),
                Box::new(|ev| ev.value_state
                    == attribute_store_node_value_state_t::DESIRED_OR_REPORTED_ATTRIBUTE)
            )
        );

        // get change
        let attribute = Attribute::new(123, 345).unwrap();
        listener.new_event(AttributeEvent::default());

        assert_eq!(Some(AttributeEvent::default()), listener.take_change(0));
        assert_eq!(Some(AttributeEvent::default()), listener.take_change(1));

        // test unregister
        listener.unsubscribe(0);
        assert_eq!(None, listener.take_change(0));

        // test change did not happen on false predicate
        let mut event = AttributeEvent::default();
        event.value_state = attribute_store_node_value_state_t::DESIRED_ATTRIBUTE;
        event.attribute = attribute;
        listener.new_event(event);
        assert_eq!(None, listener.take_change(1));
    }

    #[test]
    #[serial(change_listener)]
    fn stream_happy_flow() {
        teardown_change_listeners();

        let ctx = inner::register_callback_context();
        ctx.expect().withf(|f| f.is_some()).return_const(0u32);

        let waker = noop_waker();
        let mut context = Context::from_waker(&waker);
        let mut stream = AttributeChangedStream::new(|_| true, |a| a);

        // not registered test
        assert_eq!(Pin::new(&mut stream).poll_next(&mut context), Ready(None));

        let mut seq = Sequence::new();
        let mut mock = MockWakerCacheTrait::<AttributeEvent>::new();

        mock.expect_register()
            .with(predicate::always(), predicate::always())
            .return_const(2u64)
            .times(1)
            .in_sequence(&mut seq);

        let mut test_event1 = AttributeEvent::default();
        test_event1.attribute = Attribute::new(1, 1).unwrap();

        let test1_clone = test_event1.clone();
        mock.expect_take_change()
            .with(eq(2))
            .returning(move |_| {
                return Some(test1_clone);
            })
            .times(1)
            .in_sequence(&mut seq);

        let mut test_event2 = AttributeEvent::default();
        test_event2.attribute = Attribute::new(3, 3).unwrap();
        let test2_clone = test_event2.clone();

        mock.expect_take_change()
            .with(eq(2))
            .returning(move |_| {
                return Some(test2_clone);
            })
            .times(1)
            .in_sequence(&mut seq);

        mock.expect_unsubscribe()
            .with(eq(2))
            .return_const(())
            .times(1)
            .in_sequence(&mut seq);

        unsafe { CHANGE_LISTENER = Some(Box::new(mock)) };
        unsafe { REGISTERED = Some(0) };

        // prepare stream, returns Pending
        assert_eq!(Pin::new(&mut stream).poll_next(&mut context), Pending);

        // Wake called, produces result
        assert_eq!(
            Pin::new(&mut stream).poll_next(&mut context),
            Ready(Some(test_event1))
        );

        // Wake called, produces result
        assert_eq!(
            Pin::new(&mut stream).poll_next(&mut context),
            Ready(Some(test_event2))
        );
    }

    #[test]
    #[serial(change_listener)]
    fn attribute_state_tests() {
        teardown_change_listeners();

        let waker = noop_waker();
        let mut context = Context::from_waker(&waker);
        let mut stream = AttributeChangedStream::new(|_| true, |a| a);

        // no cache initialized
        assert_eq!(Pin::new(&mut stream).poll_next(&mut context), Ready(None));

        let mut mock = MockWakerCacheTrait::<AttributeEvent>::new();
        mock.expect_register()
            .with(predicate::always(), predicate::always())
            .return_const(2u64)
            .times(1);
        mock.expect_unsubscribe()
            .with(eq(2))
            .return_const(())
            .times(1);

        unsafe { CHANGE_LISTENER = Some(Box::new(mock)) };

        // not registered
        assert_eq!(Pin::new(&mut stream).poll_next(&mut context), Ready(None));

        unsafe { REGISTERED = Some(0) };

        // Everything ok
        assert_eq!(Pin::new(&mut stream).poll_next(&mut context), Pending);
    }

    #[test]
    #[serial(change_listener)]
    fn teardown_forces_wake() {
        let mut mock = MockWakerCacheTrait::<AttributeEvent>::new();
        mock.expect_force_wake_all().returning(|| ());
        unsafe { CHANGE_LISTENER = Some(Box::new(mock)) };

        unsafe {
            rust_contiki_teardown();
        }
    }
}
