use std::vec;

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
use crate::attribute_poll_trait::IntervalType;
use crate::attribute_watcher_trait::AttributeWatcherTrait;
use crate::poll_queue_trait::PollQueueTrait;
use crate::PollEngineConfig;
use futures::channel::mpsc::UnboundedReceiver;
use futures::{select_biased, FutureExt, StreamExt};

use unify_attribute_store_sys::{attribute_resolver_register_rule, sl_status_t};
use unify_log_sys::*;
use unify_middleware::contiki::PlatformTrait;
use unify_middleware::{
    Attribute, AttributeEvent, AttributeEventType, AttributeStoreError, AttributeTrait,
};
use unify_proc_macro::{as_extern_c, generate_resolver_callback};
use unify_sl_status_sys::SL_STATUS_ALREADY_EXISTS;

declare_app_name!("poll_engine");

pub const ATTRIBUTE_POLL_ENGINE_MARK: u32 = 0x0018;

#[derive(Debug)]
/// Enum with commands send send to the PollEngine
pub enum PollEngineCommand {
    Register {
        attribute: Attribute,
        interval: IntervalType,
    },
    Deregister {
        attribute: Attribute,
    },
    Schedule {
        attribute: Attribute,
    },
    Restart {
        attribute: Attribute,
    },
    Enable,
    Disable,
}

enum PollEngineDispatch {
    PollEngineCommand(PollEngineCommand),
    PollTimeout,
    AttributeUpdated(AttributeEvent<Attribute>),
}

/// The [PollEngine] is a state-machine that contains the core logic of the
/// `unify_attribute_poll` library, it handles scheduling timers for next
/// upcoming Poll, handles backoff and other limitations specified in
/// [PollEngineConfig]. It also contains the list,
/// [PollEntries](crate::poll_entries::PollEntries), of all attributes queue-ed
/// in the polling system.
///
/// The [PollEngine] is not to be instantiated on its own. Rather its part of
/// the public [AttributePoll](crate::attribute_poll::AttributePoll) object.
/// Refer to its doc for more info on usage.
///
/// The [PollEngine] is controlled via a mpsc channel. In Which
/// [PollEngineCommand] objects can be passed.
///
/// The rules for timer scheduling are:
/// * If deadline of `next` entry < `last_poll` + `backoff`, queue timeout for
///   in `backoff` seconds,
/// * else queue timeout for deadline of `next` entry in [PollEntries].
///
/// The PollEngine is aware of state changes inside the attribute store. There
/// is no need for explicitly deleting the entries on the event of a delete
/// node.
pub(crate) struct PollEngine {
    config: PollEngineConfig,
    poll_queue: Box<dyn PollQueueTrait>,
    platform: Box<dyn PlatformTrait>,
    last_poll: u64,
    running: bool,
}

#[generate_resolver_callback]
fn poll_engine_mark(a: Attribute) -> Result<(sl_status_t, Vec<u8>), AttributeStoreError> {
    // Clear the reported attribute of the the parent to trigger a resolution
    // and delete this meta attribute
    a.parent().set_reported::<u32>(None)?;
    a.delete()?;
    // This meta rule does not produce a frame
    Ok((SL_STATUS_ALREADY_EXISTS, vec![]))
}

impl PollEngine {
    pub fn new(
        platform: impl PlatformTrait + 'static,
        poll_queue: impl PollQueueTrait + 'static,
        config: PollEngineConfig,
    ) -> Self {
        attribute_resolver_register_rule(
            ATTRIBUTE_POLL_ENGINE_MARK,
            None,
            as_extern_c!(poll_engine_mark),
        );

        PollEngine {
            config,
            poll_queue: Box::new(poll_queue),
            platform: Box::new(platform),

            last_poll: 0,
            running: true,
        }
    }

    fn next_poll_timeout(&self) -> Option<u64> {
        let upcoming_deadline = self.poll_queue.upcoming()?.1;
        let now = self.platform.clock_seconds();
        let next_timeout = calculate_poll_time(
            self.config.backoff.into(),
            upcoming_deadline,
            now,
            self.last_poll,
        );
        Some(next_timeout)
    }

    /// (run)[PollEngine::run] is the main entry point of the [PollEngine] and
    /// is required to be called before executing any command.
    ///
    /// This async function will never return and its lifetime will span the
    /// whole application. Therefore make sure that calling this function will
    /// not starve the program. Run this in a separate task
    /// with [unify_middleware::contiki::contiki_spawn]
    ///
    /// Note that the PollEngine is moved by value. this means that after this
    /// function the instance of [PollEngine] is consumed by the run function
    /// and does not exist any more.
    pub async fn run(
        mut context: PollEngine,
        mut attribute_watcher: impl AttributeWatcherTrait,
        mut command_receiver: UnboundedReceiver<PollEngineCommand>,
    ) {
        loop {
            // select_biased! awaits 3 functions:
            // * a wait function until there is an incoming [PollEngineCommand]
            //   send to the pollEngine. e.g to register an attribute for
            //   polling
            // * a wait for a change in the attribute store, if the attribute is
            //   currently in the poll engine we want to update our bookkeeping
            //   if for instance its gets deleted
            // * wait until a scheduled poll timer goes off. in this case we
            //   want to clear the value for the upcoming attribute in the queue
            //
            // select_biased functions the same as a select! block. The only
            // difference is that when multiple items are ready, the item
            // highest in the order of declaration will be picked. for select!
            // on the other hand its undefined which item will be picked.
            //
            // when one of the awaited functions produces an value, we exit the
            // selected_biased block so we can process it. This means that the
            // other functions inside the select block will not be polled
            // anymore and go out of scope as well. for instance, the timer
            // inside the schedule poll will be stopped and all the stack
            // allocations made inside of the `schedule_poll()` will be
            // released.
            //
            // we cannot process the selected future inside of the
            // selected_biased block since we require to have an mutable borrow
            // of the context object. And according rust we can have only one
            // mutable borrower or several immutable borrowers at once. Doing
            // something immutable inside of the select_biased block in our case
            // would be completely fine.
            //
            // functions that are being awaited inside `select!` blocks need to
            // be `fused`. A requirement enforced by select!. Fusing an future
            // means that after the future completed and is returned, its fuse
            // is to be broken and will therefore not be polled anymore by the
            // select! block. In our case its not relevant since we create
            // and fuse our futures inside the select block. i.e. we always have
            // a fresh `FusedFuture`. Would one of the futures for instance be
            // initialized outside of the loop, and its returned by the
            // select block, then it will never be polled anymore in successive
            // iterations of the while loop and therefore never be triggered
            // again.
            let dispatch_cmd = select_biased! {
                res = command_receiver.select_next_some() => PollEngineDispatch::PollEngineCommand(res),
                res = PollEngine::find_updated_attribute(&context, &mut attribute_watcher).fuse() => res,
                res = PollEngine::schedule_poll(&context).fuse() => res,
            };

            // At this point one of the awaited functions went off and produced
            // an [PollEngineDispatch] command which needs to be executed. since
            // we don't have any immutable borrowers any more we borrow mutable
            // here and call mutable functions on the context object!
            match dispatch_cmd {
                PollEngineDispatch::PollEngineCommand(cmd) => context.on_handle_command(cmd),
                PollEngineDispatch::PollTimeout => context.do_poll(),
                PollEngineDispatch::AttributeUpdated(event) => {
                    context.on_handle_attribute_update(event)
                }
            }
            // we are finished dispatching the internal data. we loop back again
            // to await again for a produced result on one of the 3 functions
        }
    }

    /// This function will schedule a poll timeout and will return as soon as
    /// the timeout occurs. The exact timeout is retrieved by looking up the
    /// upcoming deadline inside the [PollEntries] poll queue.
    ///
    /// *This function will never return in the case when the engine is paused
    /// or if number of attribute in the poll queue equals 0!*
    ///
    /// # Returns
    ///
    /// `Some(PollTimeout)`    when an timeout on the poll timer occurred
    /// `BLOCKING`             nothing to be polled or the engine is paused     
    async fn schedule_poll(&self) -> PollEngineDispatch {
        if let Some(timeout) = self.is_running_and_upcoming_timeout() {
            // the timer stops automatically when the timer object goes out of
            // scope. therefore, we don't have to manually cleanup the timer
            // object.
            let timer = self.platform.get_timer_object();
            timer
                .on_timeout(core::time::Duration::from_secs(timeout))
                .await;
            return PollEngineDispatch::PollTimeout;
        }
        // if there is nothing to poll, schedule a future that always returns
        // std::task::Poll::Pending state. e.g it will never complete
        log_debug!("no poll scheduled");
        futures::future::pending().await
    }

    /// Listens to the attribute store for status updates of attributes It will
    /// only return an update if the attribute is present in the poll queue. In
    /// this case we might want to update our bookkeeping accordingly.
    ///
    /// # Arguments
    ///
    /// * `attribute_watcher`   a watcher that watches for attribute DELETED and
    ///   UPDATED events as well as touch UPDATED events
    ///
    /// # Returns
    ///
    /// * `PollEngineDispatch::AttributeUpdated(event)` when an update is found
    ///   AttributeUpdated is returned with the event specifics as its argument.
    async fn find_updated_attribute(
        &self,
        attribute_watcher: &mut impl AttributeWatcherTrait,
    ) -> PollEngineDispatch {
        loop {
            let event = attribute_watcher.next_change().await;
            if self.poll_queue.contains(&event.attribute) {
                return PollEngineDispatch::AttributeUpdated(event);
            }
        }
    }

    /// * Returns `Some(timeout)` (in sec) when the engine is running and there
    /// is an upcoming poll interval calculated.
    /// * Returns None if the engine is paused or there are no items in the poll
    ///   queue.
    fn is_running_and_upcoming_timeout(&self) -> Option<u64> {
        self.running.then(|| self.next_poll_timeout()).flatten()
    }

    /// This function does 2 things:
    /// * Clears the reported value of the attribute that needs polling.
    /// * Stores te last time this poll action was executed.
    fn do_poll(&mut self) {
        let (attribute, interval) = match self.poll_queue.pop_next() {
            Some(x) => x,
            None => {
                log_error!(
                    "scheduled timer for upcoming attribute. but queue appears to be empty!"
                );
                return;
            }
        };

        // In order to support cases where the resolver tree is paused, attribute are marked for polling
        // by placing the meta attribute ATTRIBUTE_POLL_ENGINE_MARK under the attribute to be polled.
        // An attribute resolver rule will then clear the reported value of the parent, ensuring that
        // the device is polled when its ready. In this way we even persist poll request over reboots.
        if let Err(e) = attribute.emplace::<u32>(ATTRIBUTE_POLL_ENGINE_MARK, None, None) {
            log_warning!("Failed to clear reported for {} {}", attribute, e);
        } else {
            let now = self.platform.clock_seconds();
            self.last_poll = now;
            self.poll_queue.queue(attribute, interval, now);
            log_debug!("Marking {} for poll", attribute);
        }
    }

    /// handles the incoming commands of the engine.
    fn on_handle_command(&mut self, cmd: PollEngineCommand) {
        match cmd {
            PollEngineCommand::Register {
                attribute,
                interval,
            } => self.on_register(attribute, interval),
            PollEngineCommand::Deregister { attribute } => self.on_deregister(attribute),
            PollEngineCommand::Schedule { attribute } => self.on_schedule(attribute),
            PollEngineCommand::Restart { attribute } => self.on_restart(attribute),
            PollEngineCommand::Enable => self.on_enable(),
            PollEngineCommand::Disable => self.on_disable(),
        }

        // Comment this in to get poll_engine to print the entire configuration
        //#[cfg(not(test))]
        //log_debug!("{}", self.poll_queue);
    }

    fn on_handle_attribute_update(&mut self, event: AttributeEvent<Attribute>) {
        match event.event_type {
            AttributeEventType::ATTRIBUTE_DELETED => {
                log_debug!("Removing {} because it is deleted", event.attribute);
                if !self.poll_queue.remove(&event.attribute) {
                    log_warning!(
                        "could not delete {}. not in the poll_queue anymore",
                        &event.attribute
                    );
                }
            }
            AttributeEventType::ATTRIBUTE_UPDATED => {
                if self
                    .poll_queue
                    .requeue(event.attribute, self.platform.clock_seconds())
                {
                    log_debug!("updated {} in poll-queue", &event.attribute);
                }
            }
            e => {
                log_error!("did not subscribe for {:?}", e);
            }
        }
    }

    fn on_register(&mut self, attribute: Attribute, interval: u32) {
        let used_interval = default_if_zero(&self.config, interval);
        log_debug!("Register {} with interval {}s", attribute, used_interval);

        let now = self.platform.clock_seconds();
        self.poll_queue.queue(attribute, used_interval, now);
    }

    fn on_deregister(&mut self, attribute: Attribute) {
        log_debug!("Remove {}", attribute);
        if !self.poll_queue.remove(&attribute) {
            log_warning!("didn't remove {}", attribute);
        }
    }

    fn on_schedule(&mut self, attribute: Attribute) {
        log_debug!("Register {} with interval {}s", attribute, 0);
        let now = self.platform.clock_seconds();
        self.poll_queue.queue(attribute, 0, now);
    }

    fn on_restart(&mut self, attribute: Attribute) {
        log_debug!("Restart {}", attribute);
        if !self
            .poll_queue
            .requeue(attribute, self.platform.clock_seconds())
        {
            log_warning!("cannot restart, attribute {} not present", attribute);
        }
    }

    fn on_enable(&mut self) {
        log_debug!("resuming poll engine");
        self.running = true;
    }

    fn on_disable(&mut self) {
        log_debug!("suspending poll engine");
        self.running = false;
    }
}

fn default_if_zero(config: &PollEngineConfig, interval: u32) -> u64 {
    if interval == 0 {
        config.default_interval as u64
    } else {
        interval as u64
    }
}

/// Calculate when the next attribute should be polled.
/// return interval 1 if poll timeout is in the past
fn calculate_poll_time(backoff: u64, upcoming_entry: u64, now: u64, last_poll: u64) -> u64 {
    assert!(last_poll <= now);
    let minimum_poll_time = last_poll + backoff as u64;
    let deadline = std::cmp::max(upcoming_entry, minimum_poll_time);
    match deadline.overflowing_sub(now) {
        (_, true) => 1u64,
        (val, false) => val,
    }
}

#[cfg(test)]
mod tests {
    use crate::attribute_watcher::test::AttributeWatcherStub;
    use crate::poll_queue_trait::MockPollQueueTrait;

    use super::*;
    use crate::poll_entries::PollEntries;
    use async_trait::async_trait;
    use futures::channel::mpsc::unbounded;
    use futures::pin_mut;
    use futures::stream;
    use futures_test::assert_stream_pending;
    use futures_test::future::FutureTestExt;
    use mockall::predicate::eq;
    use mockall::Sequence;
    use serial_test::serial;
    use std::time::Duration;
    use unify_middleware::contiki::MockPlatformTrait;
    use unify_middleware::contiki::MockTimerTrait;
    use unify_middleware::contiki::TimeoutSuccess;
    use unify_middleware::AttributeStore;
    use unify_middleware::AttributeStoreTrait;
    use unify_middleware::AttributeValueState;
    struct NoValueAttributeWatcher;

    #[async_trait]
    impl AttributeWatcherTrait for NoValueAttributeWatcher {
        async fn next_change(&mut self) -> AttributeEvent<Attribute> {
            futures::future::pending().await
        }
    }

    #[test]
    fn minimum_deadline() {
        assert_eq!(calculate_poll_time(0, 1, 0, 0), 1);
        // backoff is higher as scheduled
        assert_eq!(calculate_poll_time(3, 1, 0, 0), 3);
        // if poll is in the past, schedule after 1 sec
        assert_eq!(calculate_poll_time(0, 1, 6, 3), 1);
        assert_eq!(calculate_poll_time(0, 9, 6, 3), 3);
    }

    #[test]
    #[should_panic]
    fn last_poll_in_the_future_panics() {
        assert_eq!(calculate_poll_time(2, 10, 5, 12), 14);
    }

    #[test]
    fn test_default_to_zero() {
        let cfg = PollEngineConfig {
            backoff: 3,
            default_interval: 1,
        };
        assert_eq!(default_if_zero(&cfg, 0), 1);
        assert_eq!(default_if_zero(&cfg, 44), 44);
    }

    #[test]
    #[serial(poll_engine)]

    fn engine_does_nothing_on_creation() {
        let contiki_clock_mock = MockPlatformTrait::new();
        let mut poll_queue_mock = MockPollQueueTrait::new();
        let attribute_watcher_mock = NoValueAttributeWatcher;
        let cfg = PollEngineConfig {
            backoff: 0,
            default_interval: 0,
        };

        let mut sequence = Sequence::new();

        poll_queue_mock
            .expect_upcoming()
            .with()
            .times(1)
            .in_sequence(&mut sequence)
            .return_const(None);

        let (_, receiver) = unbounded();
        let engine = PollEngine::new(contiki_clock_mock, poll_queue_mock, cfg);

        let stream =
            stream::once(PollEngine::run(engine, attribute_watcher_mock, receiver).pending_once());
        pin_mut!(stream);

        // does not matter how much we poll the engine, nothing is happening
        assert_stream_pending!(stream);
        assert_stream_pending!(stream);
        assert_stream_pending!(stream);
        assert_stream_pending!(stream);
        assert_stream_pending!(stream);
    }

    #[test]
    #[ignore = "reasons"]
    #[serial(poll_engine)]
    fn do_poll_is_connected() {
        let attribute_store = AttributeStore::new().unwrap();
        let mut contiki_clock_mock = MockPlatformTrait::new();
        contiki_clock_mock
            .expect_clock_seconds()
            .with()
            .return_const(0u64);

        let (mut timeout_sender, mut timeout_receiver) = unbounded::<Option<TimeoutSuccess>>();
        let timeout_result_fut = Box::pin(async move { timeout_receiver.select_next_some().await });
        let mut timer_mock = Box::new(MockTimerTrait::new());
        timer_mock
            .expect_on_timeout()
            .withf(|d| d == &Duration::from_secs(22))
            .return_once(|_| timeout_result_fut);
        contiki_clock_mock
            .expect_get_timer_object()
            .with()
            .return_once(move || timer_mock);

        let cfg = PollEngineConfig {
            backoff: 0,
            default_interval: 0,
        };

        let mut sequence = Sequence::new();
        let mut poll_queue_mock = MockPollQueueTrait::new();
        poll_queue_mock
            .expect_upcoming()
            .times(1)
            .in_sequence(&mut sequence)
            .with()
            .return_const(Some((attribute_store.from_handle_and_type(1, 1), 22u64)));
        poll_queue_mock
            .expect_pop_next()
            .times(1)
            .in_sequence(&mut sequence)
            .with()
            .return_const((attribute_store.from_handle_and_type(1, 1), 22));
        poll_queue_mock
            .expect_upcoming()
            .times(1)
            .in_sequence(&mut sequence)
            .with()
            .return_const(None);

        let (_, receiver) = unbounded();
        let engine = PollEngine::new(contiki_clock_mock, poll_queue_mock, cfg);
        let stream =
            stream::once(PollEngine::run(engine, NoValueAttributeWatcher, receiver).pending_once());
        pin_mut!(stream);

        assert_stream_pending!(stream);
        assert_stream_pending!(stream);

        timeout_sender.start_send(Some(TimeoutSuccess)).unwrap();
        assert_stream_pending!(stream);
    }

    #[test]
    #[serial(poll_engine)]
    fn disable_does_not_schedule_poll() {
        let mut contiki_clock_mock = MockPlatformTrait::new();
        contiki_clock_mock
            .expect_clock_seconds()
            .with()
            .return_const(0u64);

        let (_, mut timeout_receiver) = unbounded::<Option<TimeoutSuccess>>();
        let timeout_result_fut = Box::pin(async move { timeout_receiver.select_next_some().await });
        let mut timer_mock = Box::new(MockTimerTrait::new());
        timer_mock
            .expect_on_timeout()
            .withf(|d| d == &Duration::from_secs(22))
            .return_once(|_| timeout_result_fut);
        contiki_clock_mock
            .expect_get_timer_object()
            .with()
            .return_once(move || timer_mock);

        let cfg = PollEngineConfig {
            backoff: 0,
            default_interval: 0,
        };

        let mut poll_queue_mock = MockPollQueueTrait::new();
        poll_queue_mock
            .expect_upcoming()
            .times(1)
            .with()
            .return_const(None);

        let (mut sender, receiver) = unbounded();
        let engine = PollEngine::new(contiki_clock_mock, poll_queue_mock, cfg);

        let stream =
            stream::once(PollEngine::run(engine, NoValueAttributeWatcher, receiver).pending_once());
        pin_mut!(stream);

        assert_stream_pending!(stream);
        assert_stream_pending!(stream);

        sender.start_send(PollEngineCommand::Disable).unwrap();

        assert_stream_pending!(stream);
        sender.start_send(PollEngineCommand::Enable).unwrap();
    }

    #[test]
    #[serial(poll_engine)]
    fn register_is_connected() {
        let attribute_store = AttributeStore::new().unwrap();
        let mut contiki_clock_mock = MockPlatformTrait::new();
        contiki_clock_mock
            .expect_clock_seconds()
            .with()
            .return_const(0u64);

        let (_, mut timeout_receiver) = unbounded::<Option<TimeoutSuccess>>();
        let timeout_result_fut = Box::pin(async move { timeout_receiver.select_next_some().await });
        let mut timer_mock = Box::new(MockTimerTrait::new());
        timer_mock
            .expect_on_timeout()
            .withf(|d| d == &Duration::from_secs(22))
            .return_once(|_| timeout_result_fut);
        contiki_clock_mock
            .expect_get_timer_object()
            .with()
            .return_once(move || timer_mock);

        let cfg = PollEngineConfig {
            backoff: 0,
            default_interval: 0,
        };

        let mut sequence = Sequence::new();
        let mut poll_queue_mock = MockPollQueueTrait::new();
        poll_queue_mock
            .expect_queue()
            .times(1)
            .in_sequence(&mut sequence)
            .with(
                eq(attribute_store.from_handle_and_type(1, 1)),
                eq(22),
                eq(0),
            )
            .returning(|_, _, _| true);

        poll_queue_mock
            .expect_remove()
            .times(1)
            .in_sequence(&mut sequence)
            .with(eq(attribute_store.from_handle_and_type(1, 1)))
            .returning(|_| true);

        let (mut sender, receiver) = unbounded();
        let engine = PollEngine::new(contiki_clock_mock, poll_queue_mock, cfg);

        let stream =
            stream::once(PollEngine::run(engine, NoValueAttributeWatcher, receiver).pending_once());
        pin_mut!(stream);

        sender.start_send(PollEngineCommand::Disable).unwrap();

        assert_stream_pending!(stream);
        assert_stream_pending!(stream);

        sender
            .start_send(PollEngineCommand::Register {
                attribute: attribute_store.from_handle_and_type(1, 1),
                interval: 22,
            })
            .unwrap();
        sender
            .start_send(PollEngineCommand::Deregister {
                attribute: attribute_store.from_handle_and_type(1, 1),
            })
            .unwrap();

        assert_stream_pending!(stream);
        assert_stream_pending!(stream);
    }

    #[test]
    #[serial(poll_engine)]
    fn poll_engine_mark_test() {
        let store = AttributeStore::new().unwrap();
        let mut a = store.root().add(11111, Some(42), Some(24)).unwrap();

        let cfg = PollEngineConfig {
            backoff: 0,
            default_interval: 0,
        };

        let mut poll_queue = PollEntries::new();
        let mut sequence = Sequence::new();
        let mut contiki_clock_mock = MockPlatformTrait::new();

        contiki_clock_mock
            .expect_clock_seconds()
            .with()
            .return_const(0u64);

        let mut engine = PollEngine::new(contiki_clock_mock, poll_queue, cfg);
        engine.on_register(a, 1111);
        engine.do_poll();
        engine.do_poll();

        assert_eq!(a.get_reported::<u32>(), Ok(42));
        assert_eq!(a.get_desired::<u32>(), Ok(24));

        let mark = a.get_child_by_type(ATTRIBUTE_POLL_ENGINE_MARK);
        assert_ne!(mark, Attribute::invalid());
        assert_eq!(
            poll_engine_mark_rust(mark),
            Ok((SL_STATUS_ALREADY_EXISTS, vec! {}))
        );

        assert_eq!(a.is_reported_set(), false);
        assert_eq!(a.get_desired::<u32>(), Ok(24));
    }

    #[test]
    #[serial(poll_engine)]
    fn poll_engine_delete_test() {
        let attribute_store = AttributeStore::new().unwrap();
        let a = attribute_store.root().add(1, Some(42), Some(52)).unwrap();
        let (mut attribute_watcher_sender, attribute_watcher_receiver) = unbounded();

        let attribute_watcher_stub = AttributeWatcherStub::new(attribute_watcher_receiver);
        let cfg = PollEngineConfig {
            backoff: 0,
            default_interval: 0,
        };

        let mut poll_queue_mock = MockPollQueueTrait::new();
        let mut sequence = Sequence::new();
        poll_queue_mock
            .expect_queue()
            .times(1)
            .in_sequence(&mut sequence)
            .with(eq(a), eq(22), eq(0))
            .returning(|_, _, _| true);
        poll_queue_mock
            .expect_contains()
            .times(1)
            .in_sequence(&mut sequence)
            .with(eq(a))
            .returning(|_| true);
        poll_queue_mock
            .expect_remove()
            .times(1)
            .in_sequence(&mut sequence)
            .with(eq(a))
            .returning(|_| true);
        poll_queue_mock
            .expect_upcoming()
            .with()
            .times(1)
            .in_sequence(&mut sequence)
            .return_const(None);

        let mut contiki_clock_mock = MockPlatformTrait::new();

        contiki_clock_mock
            .expect_clock_seconds()
            .with()
            .return_const(0u64);
        let (mut sender, receiver) = unbounded();
        let engine = PollEngine::new(contiki_clock_mock, poll_queue_mock, cfg);

        let stream =
            stream::once(PollEngine::run(engine, attribute_watcher_stub, receiver).pending_once());
        pin_mut!(stream);
        sender
            .start_send(PollEngineCommand::Register {
                attribute: a,
                interval: 22,
            })
            .unwrap();
        assert_stream_pending!(stream);
        let attribute_event = AttributeEvent {
            attribute: a,
            event_type: AttributeEventType::ATTRIBUTE_DELETED,
            value_state: AttributeValueState::DESIRED_OR_REPORTED_ATTRIBUTE,
        };
        attribute_watcher_sender
            .start_send(attribute_event)
            .unwrap();
        assert_stream_pending!(stream);
    }
}
