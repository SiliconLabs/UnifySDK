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

use crate::{poll_engine::PollEngineCommand, poll_entries::PollQueueTrait};
use futures::channel::mpsc::{unbounded, UnboundedReceiver, UnboundedSender};
use futures::{select, StreamExt};
use once_cell::sync::Lazy;
use std::{cell::RefCell, rc::Rc};
use unify_attribute_store_sys::attribute_store_node_t;
use unify_log_sys::*;
use unify_sl_status_sys::SL_STATUS_OK;

use unify_middleware::{
    contiki::contiki_spawn, Attribute, AttributeEvent, AttributeEventType, AttributeStoreTrait,
};
declare_app_name!("poll_engine");

static mut TOUCH_CHANNEL: Lazy<(UnboundedSender<Attribute>, UnboundedReceiver<Attribute>)> =
    Lazy::new(|| unbounded());

/// Run a Tasks which awaits changes of Attributes that are in the `PollQueue`.
/// On an change it will send a command over the mpsc channel so that the
/// [PollEngine](crate::poll_engine::PollEngine) can act accordingly. This
/// watcher will watch for the following events:
/// * [unify_middleware::AttributeEventType::ATTRIBUTE_DELETED]
/// * [unify_middleware::AttributeEventType::ATTRIBUTE_UPDATED]
/// * Touch events
pub(crate) unsafe fn run_attribute_watcher(
    mut command_sender: UnboundedSender<PollEngineCommand>,
    poll_queue: Rc<RefCell<dyn PollQueueTrait>>,
) {
    // UIC-1550 Technical debt! merge these touch callbacks with the other callbacks so that
    // the attribute streams support touch callbacks
    if unify_attribute_store_sys::attribute_store_register_touch_notification_callback(Some(
        uic_attribute_store_get_attribute_touch_callback,
    )) != SL_STATUS_OK
    {
        log_error!("Failed to register attribute touch notification callback");
        return;
    }

    let watcher = async move {
        let queue_clone = poll_queue.clone();
        let attribute_predicate = move |e: &AttributeEvent| {
            e.event_type == AttributeEventType::ATTRIBUTE_DELETED
                && queue_clone.borrow().contains(&e.attribute)
        };

        let mut attribute_changes = Attribute::attribute_changed(attribute_predicate).fuse();
        loop {
            let cmd = select! {
                event = attribute_changes.select_next_some() => {
                    let cmd = match event.event_type {
                        AttributeEventType::ATTRIBUTE_DELETED => PollEngineCommand::Deleted {
                            attribute: event.attribute,
                        },
                        AttributeEventType::ATTRIBUTE_UPDATED => PollEngineCommand::Restart {
                            attribute: event.attribute,
                        },
                        e => {
                            panic!("did not subscribe for {:?}", e);
                        }
                    };
                    Some(cmd)
                },
                touch_attr = TOUCH_CHANNEL.1.select_next_some() => {
                    let borrow = poll_queue.borrow();
                    if borrow.contains(&touch_attr) && !borrow.is_flagged(touch_attr) {
                        Some(PollEngineCommand::Restart {
                            attribute: touch_attr,
                        })
                    } else {
                        None
                    }
                }
            };

            if let Some(command) = cmd {
                if let Err(e) = command_sender.start_send(command) {
                    log_error!("poll engine should outlive this attribute watcher {}", e);
                }
            }
        }
    };

    contiki_spawn(watcher);
}

// Callback function for when an attribute is touched
unsafe extern "C" fn uic_attribute_store_get_attribute_touch_callback(
    node: attribute_store_node_t,
) {
    match Attribute::new_from_handle(node) {
        Ok(attribute) => {
            if let Err(e) = TOUCH_CHANNEL.0.start_send(attribute) {
                log_error!("attribute watcher disappeared!{}", e);
            }
        }
        Err(err) => log_error!("touch callback called with invalid attribute {:?}", err),
    };
}
