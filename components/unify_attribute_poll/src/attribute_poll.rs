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
use crate::attribute_poll_trait::AttributePollTrait;
use crate::attribute_poll_trait::IntervalType;
use crate::attribute_watcher::AttributeWatcher;
use crate::poll_engine::PollEngine;
use crate::poll_engine::PollEngineCommand;
use crate::poll_entries::PollEntries;
use crate::PollEngineConfig;
use futures::channel::mpsc::unbounded;
use futures::channel::mpsc::UnboundedSender;
use once_cell::sync::OnceCell;
use unify_log_sys::*;
use unify_middleware::contiki::contiki_spawn;
use unify_middleware::contiki::ContikiPlatform;
use unify_middleware::Attribute;
declare_app_name!("poll_engine");

static SENDER: OnceCell<UnboundedSender<PollEngineCommand>> = OnceCell::new();

/// Its purpose is to interface with the single `Attribute PollEngine` instance.
/// And allows for easy sharing of the `PollEngine`,
#[derive(Debug, Default, Clone, Copy)]
pub struct AttributePoll {}

impl AttributePollTrait for AttributePoll {
    fn initialize(&self, config: PollEngineConfig) {
        if SENDER.get().is_some() {
            return;
        }

        let ( sender, receiver) = unbounded();
        SENDER
            .set(sender.clone())
            .expect("should only be called once");

        // task running the actual poll engine
        contiki_spawn(async {
            let context = PollEngine::new(ContikiPlatform::default(), PollEntries::new(), config);
            PollEngine::run(context, AttributeWatcher::new(), receiver).await;
            log_info!("exiting the poll engine");
        });
    }

    fn register(&self, attribute: Attribute, interval: IntervalType) {
        if let Some(sender) = SENDER.get() {
            sender
                .clone()
                .start_send(PollEngineCommand::Register {
                    attribute,
                    interval,
                })
                .unwrap();
        }
    }

    fn deregister(&self, attribute: Attribute) {
        if let Some(sender) = SENDER.get() {
            sender
                .clone()
                .start_send(PollEngineCommand::Deregister { attribute })
                .unwrap();
        }
    }

    fn schedule_now(&self, attribute: Attribute) {
        if let Some(sender) = SENDER.get() {
            sender
                .clone()
                .start_send(PollEngineCommand::Schedule { attribute })
                .unwrap();
        }
    }

    fn restart(&self, attribute: Attribute) {
        if let Some(sender) = SENDER.get() {
            sender
                .clone()
                .start_send(PollEngineCommand::Restart { attribute })
                .unwrap();
        }
    }

    fn pause(&self) {
        if let Some(sender) = SENDER.get() {
            sender
                .clone()
                .start_send(PollEngineCommand::Disable)
                .unwrap();
        }
    }

    fn resume(&self) {
        if let Some(sender) = SENDER.get() {
            sender
                .clone()
                .start_send(PollEngineCommand::Enable)
                .unwrap();
        }
    }
}
