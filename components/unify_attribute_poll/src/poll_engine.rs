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
use super::IntervalType;
use crate::attribute_watcher::run_attribute_watcher;
use crate::poll_entries::PollQueueTrait;
use crate::PollEngineConfig;
use alloc::rc::Rc;
use core::cell::RefCell;
use futures::channel::mpsc::{UnboundedReceiver, UnboundedSender};
use futures::{select, FutureExt, StreamExt};
use unify_log_sys::*;
use unify_middleware::contiki::PlatformTrait;
use unify_middleware::{Attribute, AttributeStoreTrait};
declare_app_name!("poll_engine");

#[derive(Debug)]
/// Enum with commands send send to the PollEngine
pub(crate) enum PollEngineCommand {
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
    Deleted {
        attribute: Attribute,
    },
    Initialize {
        config: PollEngineConfig,
    },
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
/// The [PollEngine::run()] does not exit and consumes self
///
/// The rules for timer scheduling are:
/// * If deadline of `next` entry < `last_poll` + `backoff`, queue timeout for
///   in `backoff` seconds,
/// * else queue timeout for deadline of `next` entry in [PollEntries].
///
/// When an attribute is removed, the [PollEngine] deletes the entry from
/// [PollEntries](crate::poll_entries::PollEntries).
pub(crate) struct PollEngine {
    config: Option<PollEngineConfig>,
    // poll_queue declared as Rc<RefCell<>> as it needs
    // to be shared to the attribute_watcher
    poll_queue: Rc<RefCell<dyn PollQueueTrait>>,
    platform: Box<dyn PlatformTrait>,
    command_receiver: UnboundedReceiver<PollEngineCommand>,
    command_sender: UnboundedSender<PollEngineCommand>,
    last_poll: u64,
    running: bool,
}

impl PollEngine {
    pub fn new(
        platform: impl PlatformTrait + 'static,
        poll_queue: impl PollQueueTrait + 'static,
        command_receiver: UnboundedReceiver<PollEngineCommand>,
        command_sender: UnboundedSender<PollEngineCommand>,
    ) -> Self {
        PollEngine {
            config: None,
            poll_queue: Rc::new(RefCell::new(poll_queue)),
            platform: Box::new(platform),
            last_poll: 0,
            command_receiver,
            command_sender,
            running: false,
        }
    }

    fn next_poll_timeout(&self) -> Option<u64> {
        let upcoming_deadline = self.poll_queue.borrow().upcoming()?.1;
        let config = self.config.as_ref()?;
        let now = self.platform.clock_seconds();
        let next_timeout = calculate_poll_time(
            config.backoff.into(),
            upcoming_deadline,
            now,
            self.last_poll,
        );
        Some(next_timeout)
    }

    // Run the PollEngine, this call is blocking. It waits for either incoming
    // [PollEngineCommand] through the [UnboundedReceiver<PollEngineCommand>] or for
    // timeout for the next poll using [Timer].
    pub async fn run(mut self) {
        unsafe { run_attribute_watcher(self.command_sender.clone(), self.poll_queue.clone()) };
        loop {
            // There are 2 states in the PollEngine:
            // * There is an attribute in the poll-queue, and a poll timer needs to be scheduled
            // * Wait for a new command. There is nothing in the poll-queue or the PollEngine is paused.
            match self.running.then(|| self.next_poll_timeout()).flatten() {
                // if the poll engine is running and there is an available timeout to
                // be polled, handle it
                Some(timeout) => self.handle_available_timeout(timeout).await,
                // if there is nothing in the poll queue. start listening for
                // incoming commands
                None => {
                    let cmd = self.command_receiver.select_next_some().await;
                    self.on_handle_command(cmd);
                }
            }
        }
    }

    /// Handle the sub state of the PollEngine.
    async fn handle_available_timeout(&mut self, timeout: u64) {
        match self.process_poll(timeout).await {
            Ok(false) => log_warning!("timer stopped externally"),
            Ok(true) => self.do_poll(),
            Err(cmd) => self.on_handle_command(cmd),
        }
    }

    /// Wait for the timeout to happen. Or for a command to be called while we
    /// where waiting. In the latter, the command is returned with Err(cmd) but
    /// not yet process. Any command can mutate the poll queue which means that
    /// we require to reschedule a new poll timer.
    ///
    /// the timer stops automatically when the timer object goes out of scope.
    /// therefore, we dont have to manually cleanup the timer object once we
    /// for instance, receive a new command.
    ///
    /// # Arguments
    ///
    /// * timeout   interval in seconds from now when the next attribute needs
    ///             polling.
    ///
    /// # Returns
    ///
    /// * Ok(true)  Timeout happened
    /// * Ok(false) Timer got cancelled prematurely
    /// * Err(cmd)  a command was received. this needs to be processed before we can
    ///             handling timeouts again.
    async fn process_poll(&mut self, timeout: u64) -> Result<bool, PollEngineCommand> {
        log_debug!("awaking in {} seconds", timeout);

        let timer = self.platform.get_timer_object();
        let mut timeout = timer
            .on_timeout(core::time::Duration::from_secs(timeout))
            .fuse();

        select! {
            cmd = self.command_receiver.select_next_some() => Err(cmd),
            result = timeout => {
                 match result{
                     Some(_) => Ok(true),
                     None => Ok(false),
                }
            },
        }
    }

    /// This function does 2 things:
    /// * Clears the reported value of the attribute that needs polling.
    /// * Stores te last time such a poll action was executed.
    fn do_poll(&mut self) {
        let mut poll_queue = self.poll_queue.borrow_mut();

        let attribute = match poll_queue.upcoming() {
            Some(x) => x.0,
            None => {
                log_error!(
                    "scheduled timer for upcoming attribute. but queue appears to be empty!"
                );
                return;
            }
        };

        log_debug!("Clearing reported for {}", attribute);
        if let Err(e) = attribute.set_reported(None as Option<u32>) {
            log_warning!("Failed to clear reported for {} {}", attribute, e);
        }

        poll_queue.requeue(attribute);

        // work-around to prevent the attribute watcher from triggering a touch event
        // on an attribute update that we just triggered ourselves.
        poll_queue.set_flag(attribute);
        self.last_poll = self.platform.clock_seconds();
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
            PollEngineCommand::Deleted { attribute } => self.on_deleted(attribute),
            PollEngineCommand::Initialize { config } => self.on_initialize(config),
        }

        log_debug!("{}", self.poll_queue.borrow());
    }

    fn on_register(&mut self, attribute: Attribute, interval: u32) {
        if let Some(cfg) = &self.config {
            let used_interval = default_if_zero(cfg, interval);
            log_debug!("Register {} with interval {}s", attribute, used_interval);
            self.poll_queue.borrow_mut().queue(attribute, used_interval);
        } else {
            log_error!("not initialized yet");
        }
    }

    fn on_deregister(&mut self, attribute: Attribute) {
        log_debug!("Remove {}", attribute);
        if !self.poll_queue.borrow_mut().remove(&attribute) {
            log_warning!("didn't remove {}", attribute);
        }
    }

    fn on_schedule(&mut self, attribute: Attribute) {
        log_debug!("Register {} with interval {}s", attribute, 0);
        self.poll_queue.borrow_mut().queue(attribute, 0u64);
    }

    fn on_restart(&mut self, attribute: Attribute) {
        log_debug!("Restart {}", attribute);
        if !self.poll_queue.borrow_mut().requeue(attribute) {
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

    fn on_deleted(&mut self, attribute: Attribute) {
        if !self.poll_queue.borrow_mut().remove(&attribute) {
            log_warning!(
                "could not delete {}. not in the poll_queue anymore",
                attribute
            );
        }
    }

    fn on_initialize(&mut self, config: PollEngineConfig) {
        if self.config.is_some() {
            log_debug!("already initialized");
            return;
        }

        self.config = Some(config);
        self.running = true;
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
    use super::*;
    //use crate::poll_entries::MockPollQueueTrait;
    // use futures::channel::mpsc::unbounded;
    // use futures::pin_mut;
    // use futures::stream;
    // use futures_test::assert_stream_pending;
    // use futures_test::future::FutureTestExt;
    // use mockall::predicate::eq;
    // use mockall::Sequence;
    // use serial_test::serial;
    // use std::time::Duration;
    // use unify_middleware::contiki::MockPlatformTrait;
    // use unify_middleware::contiki::MockTimerTrait;
    // use unify_middleware::contiki::TimeoutSuccess;

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

    // We need a proper mocking library which can handle our needs
    // #[test]
    // #[serial(poll_engine)]

    // fn engine_does_nothing_on_creation() {
    //     let contiki_clock_mock = MockPlatformTrait::new();
    //     let poll_queue_mock = MockPollQueueTrait::new();

    //     let (sender, receiver) = unbounded();
    //     let engine = PollEngine::new(contiki_clock_mock, poll_queue_mock, receiver, sender);

    //     let stream = stream::once(engine.run().pending_once());
    //     pin_mut!(stream);

    //     // does not matter how much we poll the engine, nothing is happening
    //     assert_stream_pending!(stream);
    //     assert_stream_pending!(stream);
    //     assert_stream_pending!(stream);
    //     assert_stream_pending!(stream);
    //     assert_stream_pending!(stream);
    // }

    // #[test]
    // #[serial(poll_engine)]

    // fn initialize_engine() {
    //     let contiki_clock_mock = MockPlatformTrait::new();
    //     let mut poll_queue_mock = MockPollQueueTrait::new();
    //     poll_queue_mock.expect_upcoming().with().return_const(None);

    //     let (mut sender, receiver) = unbounded();
    //     let engine = PollEngine::new(
    //         contiki_clock_mock,
    //         poll_queue_mock,
    //         receiver,
    //         sender.clone(),
    //     );

    //     let stream = stream::once(engine.run().pending_once());
    //     pin_mut!(stream);

    //     assert_stream_pending!(stream);
    //     assert_stream_pending!(stream);

    //     sender
    //         .start_send(PollEngineCommand::Initialize {
    //             config: PollEngineConfig {
    //                 backoff: 3,
    //                 default_interval: 1,
    //             },
    //         })
    //         .unwrap();

    //     assert_stream_pending!(stream);
    // }

    // #[test]
    // #[serial(poll_engine)]

    // fn register_and_timeout_attribute() {
    //     let mut contiki_clock_mock = MockPlatformTrait::new();
    //     contiki_clock_mock
    //         .expect_clock_seconds()
    //         .with()
    //         .return_const(0u64);

    //     let (mut timeout_sender, mut timeout_receiver) = unbounded::<Option<TimeoutSuccess>>();
    //     let timeout_result_fut = Box::pin(async move { timeout_receiver.select_next_some().await });
    //     let mut timer_mock = Box::new(MockTimerTrait::new());
    //     timer_mock
    //         .expect_on_timeout()
    //         .withf(|d| d == &Duration::from_secs(22))
    //         .return_once(|_| timeout_result_fut);

    //     contiki_clock_mock
    //         .expect_get_timer_object()
    //         .with()
    //         .return_once(move || timer_mock);

    //     let mut poll_queue_mock = MockPollQueueTrait::new();
    //     let mut sequence = Sequence::new();
    //     poll_queue_mock
    //         .expect_upcoming()
    //         .times(1)
    //         .in_sequence(&mut sequence)
    //         .with()
    //         .return_const(None);
    //     poll_queue_mock
    //         .expect_queue()
    //         .times(1)
    //         .in_sequence(&mut sequence)
    //         .with(eq(Attribute::new(1, 1).unwrap()), eq(22))
    //         .returning(|_, _| true);
    //     poll_queue_mock
    //         .expect_upcoming()
    //         .times(1)
    //         .in_sequence(&mut sequence)
    //         .with()
    //         .return_const(Some((Attribute::new(1, 1).unwrap(), 22u64)));
    //     poll_queue_mock
    //         .expect_pop_next()
    //         .times(1)
    //         .in_sequence(&mut sequence)
    //         .with()
    //         .return_const(Attribute::new(1, 1).unwrap());
    //     poll_queue_mock
    //         .expect_upcoming()
    //         .times(1)
    //         .in_sequence(&mut sequence)
    //         .with()
    //         .return_const(None);

    //     let (mut sender, receiver) = unbounded();
    //     let engine = PollEngine::new(
    //         contiki_clock_mock,
    //         poll_queue_mock,
    //         receiver,
    //         sender.clone(),
    //     );

    //     let stream = stream::once(engine.run().pending_once());
    //     pin_mut!(stream);

    //     assert_stream_pending!(stream);
    //     assert_stream_pending!(stream);

    //     sender
    //         .start_send(PollEngineCommand::Initialize {
    //             config: PollEngineConfig {
    //                 backoff: 3,
    //                 default_interval: 1,
    //             },
    //         })
    //         .unwrap();

    //     assert_stream_pending!(stream);

    //     sender
    //         .start_send(PollEngineCommand::Register {
    //             attribute: Attribute::new(1, 1).unwrap(),
    //             interval: 22,
    //         })
    //         .unwrap();

    //     assert_stream_pending!(stream);

    //     timeout_sender.start_send(Some(TimeoutSuccess)).unwrap();
    //     assert_stream_pending!(stream);
    // }

    // #[test]
    // #[should_panic]
    // #[serial(poll_engine)]

    // fn register_and_timeout_fails() {
    //     let mut contiki_clock_mock = MockPlatformTrait::new();
    //     contiki_clock_mock
    //         .expect_clock_seconds()
    //         .with()
    //         .return_const(0u64);

    //     let (mut timeout_sender, mut timeout_receiver) = unbounded::<Option<TimeoutSuccess>>();
    //     let timeout_result_fut = Box::pin(async move { timeout_receiver.select_next_some().await });
    //     let mut timer_mock = Box::new(MockTimerTrait::new());
    //     timer_mock
    //         .expect_on_timeout()
    //         .withf(|d| d == &Duration::from_secs(22))
    //         .return_once(|_| timeout_result_fut);

    //     contiki_clock_mock
    //         .expect_get_timer_object()
    //         .with()
    //         .return_once(move || timer_mock);

    //     let mut poll_queue_mock = MockPollQueueTrait::new();
    //     let mut sequence = Sequence::new();
    //     poll_queue_mock
    //         .expect_upcoming()
    //         .times(1)
    //         .in_sequence(&mut sequence)
    //         .with()
    //         .return_const(None);
    //     poll_queue_mock
    //         .expect_queue()
    //         .times(1)
    //         .in_sequence(&mut sequence)
    //         .with(eq(Attribute::new(1, 1).unwrap()), eq(22))
    //         .returning(|_, _| true);
    //     poll_queue_mock
    //         .expect_upcoming()
    //         .times(1)
    //         .in_sequence(&mut sequence)
    //         .with()
    //         .return_const(Some((Attribute::new(1, 1).unwrap(), 22u64)));

    //     let (mut sender, receiver) = unbounded();
    //     let engine = PollEngine::new(
    //         contiki_clock_mock,
    //         poll_queue_mock,
    //         receiver,
    //         sender.clone(),
    //     );

    //     let stream = stream::once(engine.run().pending_once());
    //     pin_mut!(stream);

    //     assert_stream_pending!(stream);
    //     assert_stream_pending!(stream);

    //     sender
    //         .start_send(PollEngineCommand::Initialize {
    //             config: PollEngineConfig {
    //                 backoff: 3,
    //                 default_interval: 1,
    //             },
    //         })
    //         .unwrap();

    //     assert_stream_pending!(stream);

    //     sender
    //         .start_send(PollEngineCommand::Register {
    //             attribute: Attribute::new(1, 1).unwrap(),
    //             interval: 22,
    //         })
    //         .unwrap();

    //     assert_stream_pending!(stream);

    //     timeout_sender.start_send(None).unwrap();
    //     assert_stream_pending!(stream);
    // }

    // #[test]
    // #[serial(poll_engine)]
    // fn command_between_timeout() {
    //     let mut contiki_clock_mock = MockPlatformTrait::new();
    //     contiki_clock_mock
    //         .expect_clock_seconds()
    //         .with()
    //         .return_const(0u64);

    //     let mut poll_queue_mock = MockPollQueueTrait::new();
    //     let mut sequence = Sequence::new();
    //     poll_queue_mock
    //         .expect_upcoming()
    //         .times(1)
    //         .in_sequence(&mut sequence)
    //         .with()
    //         .return_const(None);
    //     poll_queue_mock
    //         .expect_queue()
    //         .times(1)
    //         .in_sequence(&mut sequence)
    //         .with(eq(Attribute::new(1, 1).unwrap()), eq(22))
    //         .returning(|_, _| true);
    //     poll_queue_mock
    //         .expect_upcoming()
    //         .times(1)
    //         .in_sequence(&mut sequence)
    //         .with()
    //         .return_const(None);

    //     let (mut sender, receiver) = unbounded();
    //     let engine = PollEngine::new(
    //         contiki_clock_mock,
    //         poll_queue_mock,
    //         receiver,
    //         sender.clone(),
    //     );

    //     let stream = stream::once(engine.run().pending_once());
    //     pin_mut!(stream);

    //     assert_stream_pending!(stream);
    //     assert_stream_pending!(stream);

    //     sender
    //         .start_send(PollEngineCommand::Initialize {
    //             config: PollEngineConfig {
    //                 backoff: 3,
    //                 default_interval: 1,
    //             },
    //         })
    //         .unwrap();

    //     assert_stream_pending!(stream);

    //     sender
    //         .start_send(PollEngineCommand::Register {
    //             attribute: Attribute::new(1, 1).unwrap(),
    //             interval: 22,
    //         })
    //         .unwrap();

    //     assert_stream_pending!(stream);

    //     sender.start_send(PollEngineCommand::Disable).unwrap();
    //     assert_stream_pending!(stream);
    //     assert_stream_pending!(stream);
    // }

    // #[test]
    // #[serial(poll_engine)]
    // fn delete_called() {
    //     let contiki_clock_mock = MockPlatformTrait::new();
    //     let mut poll_queue_mock = MockPollQueueTrait::new();
    //     poll_queue_mock.expect_upcoming().with().return_const(None);
    //     poll_queue_mock
    //         .expect_remove()
    //         .with(eq(Attribute::new(1, 1).unwrap()))
    //         .returning(|_| true);

    //     let (mut sender, receiver) = unbounded();
    //     let engine = PollEngine::new(
    //         contiki_clock_mock,
    //         poll_queue_mock,
    //         receiver,
    //         sender.clone(),
    //     );

    //     let stream = stream::once(engine.run().pending_once());
    //     pin_mut!(stream);

    //     assert_stream_pending!(stream);
    //     assert_stream_pending!(stream);

    //     sender
    //         .start_send(PollEngineCommand::Initialize {
    //             config: PollEngineConfig {
    //                 backoff: 3,
    //                 default_interval: 1,
    //             },
    //         })
    //         .unwrap();

    //     assert_stream_pending!(stream);

    //     sender
    //         .start_send(PollEngineCommand::Deleted {
    //             attribute: Attribute::new(1, 1).unwrap(),
    //         })
    //         .unwrap();

    //     assert_stream_pending!(stream);
    // }

    // #[test]
    // #[serial(poll_engine)]
    // fn schedule_called() {
    //     let contiki_clock_mock = MockPlatformTrait::new();
    //     let mut poll_queue_mock = MockPollQueueTrait::new();
    //     poll_queue_mock.expect_upcoming().with().return_const(None);
    //     poll_queue_mock
    //         .expect_queue()
    //         .with(eq(Attribute::new(1, 1).unwrap()), eq(0))
    //         .returning(|_, _| true);

    //     let (mut sender, receiver) = unbounded();
    //     let engine = PollEngine::new(
    //         contiki_clock_mock,
    //         poll_queue_mock,
    //         receiver,
    //         sender.clone(),
    //     );

    //     let stream = stream::once(engine.run().pending_once());
    //     pin_mut!(stream);

    //     assert_stream_pending!(stream);
    //     assert_stream_pending!(stream);

    //     sender
    //         .start_send(PollEngineCommand::Initialize {
    //             config: PollEngineConfig {
    //                 backoff: 3,
    //                 default_interval: 1,
    //             },
    //         })
    //         .unwrap();

    //     assert_stream_pending!(stream);

    //     sender
    //         .start_send(PollEngineCommand::Schedule {
    //             attribute: Attribute::new(1, 1).unwrap(),
    //         })
    //         .unwrap();

    //     assert_stream_pending!(stream);
    // }
}
