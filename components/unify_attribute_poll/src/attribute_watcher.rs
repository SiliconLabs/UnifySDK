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

use async_trait::async_trait;
use futures::channel::mpsc::{unbounded, UnboundedReceiver, UnboundedSender};
use futures::{select, StreamExt};
use once_cell::sync::Lazy;
use unify_attribute_store_sys::attribute_store_node_t;
use unify_log_sys::*;
use unify_middleware::{attribute_changed, AttributeValueState};
use unify_middleware::{attribute_store_or_return_with, Attribute, AttributeTrait};
use unify_middleware::{contiki::contiki_spawn, AttributeEvent, AttributeEventType};
use unify_sl_status_sys::SL_STATUS_OK;

use crate::attribute_watcher_trait::AttributeWatcherTrait;
declare_app_name!("poll_engine");

static mut TOUCH_CHANNEL: Lazy<(
    UnboundedSender<AttributeEvent<Attribute>>,
    UnboundedReceiver<AttributeEvent<Attribute>>,
)> = Lazy::new(|| unbounded());

/// Run a Tasks which awaits changes of Attributes that are in the `PollQueue`.
/// On an change it will send a command over the mpsc channel so that the
/// [PollEngine](crate::poll_engine::PollEngine) can act accordingly. This
/// watcher will watch for the following events:
/// * [unify_middleware::AttributeEventType::ATTRIBUTE_DELETED]
/// * [unify_middleware::AttributeEventType::ATTRIBUTE_UPDATED]
/// * Touch events
#[derive(Debug)]
pub struct AttributeWatcher {
    receiver: UnboundedReceiver<AttributeEvent<Attribute>>,
}

impl AttributeWatcher {
    pub fn new() -> Self {
        unsafe {
            let (sender, receiver) = unbounded();

            // UIC-1550 Technical debt! merge these touch callbacks with the other callbacks so that
            // the attribute streams support touch callbacks
            if unify_attribute_store_sys::attribute_store_register_touch_notification_callback(
                Some(uic_attribute_store_get_attribute_touch_callback),
            ) != SL_STATUS_OK
            {
                log_error!("Failed to register attribute touch notification callback");
            } else {
                contiki_spawn(AttributeWatcher::listen(sender));
            }

            Self { receiver }
        }
    }

    async unsafe fn listen(mut sender: UnboundedSender<AttributeEvent<Attribute>>) {
        let attribute_predicate = move |e: &AttributeEvent<Attribute>| {
            e.event_type == AttributeEventType::ATTRIBUTE_DELETED
        };

        let mut attribute_changes = attribute_changed(attribute_predicate).fuse();
        loop {
            let cmd = select! {
                event = attribute_changes.select_next_some() => event,
                event = TOUCH_CHANNEL.1.select_next_some() => event,
            };

            if let Err(e) = sender.start_send(cmd) {
                log_error!("poll engine should outlive this attribute watcher {}", e);
                break;
            }
        }
    }
}

#[async_trait]
impl AttributeWatcherTrait for AttributeWatcher {
    async fn next_change(&mut self) -> AttributeEvent<Attribute> {
        self.receiver.select_next_some().await
    }
}

// Callback function for when an attribute is touched
unsafe extern "C" fn uic_attribute_store_get_attribute_touch_callback(
    node: attribute_store_node_t,
) {
    let attribute_store = attribute_store_or_return_with!();
    let attribute = attribute_store.from_handle(node);

    if attribute.valid() {
        let event = AttributeEvent {
            attribute,
            event_type: AttributeEventType::ATTRIBUTE_UPDATED,
            value_state: AttributeValueState::REPORTED_ATTRIBUTE,
        };

        if let Err(e) = TOUCH_CHANNEL.0.start_send(event) {
            log_error!("attribute watcher disappeared!{}", e);
        }
    } else {
        log_error!("touch callback called with invalid attribute {}", attribute);
    }
}
