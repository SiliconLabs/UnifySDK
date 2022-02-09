// License
// <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

//! This module reads yaml configuration file that contains attribute types and
//! its polling interval values in [seconds] as follow: Keys for polling
//! intervals: ... polling_interval_zwave -> represents the interval for Z-Wave
//! node ... polling_interval_zwave_v1 -> represents the interval for Z-Wave
//! Plus v1 node ... polling_interval_zwave_v2 -> represents the interval for
//! Z-Wave Plus v2 node
//!
//! Example registering polling interval for classic Z-Wave and Zwave Plus v1
//! device classes:
//! ```yaml
//!   - attribute_type: ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_STATE
//!     polling_interval_zwave: 100
//!     polling_interval_zwave_v1: 100
//!```
//! Example registering polling interval for all device classes:
//!``` yaml
//!   - attribute_type: ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_STATE
//!     polling_interval_zwave: 100
//!     polling_interval_zwave_v1: 100
//!     polling_interval_zwave_v2: 30
//!```
//!
//! [PollRegister] will watch for included nodes in the attribute store. When a
//! new node is included or gets updated. It will schedule a poll interval for
//! that node. Given the user configured a poll interval for that node
//! matching the zwave plus info version number.
#![doc(html_no_source)]

mod zwave_poll_config;

extern crate alloc;

use futures::stream::LocalBoxStream;
use futures::StreamExt;
use unify_attribute_poll::AttributePoll;
use unify_attribute_poll::AttributePollTrait;
use unify_log_sys::*;
use unify_middleware::contiki::contiki_spawn;
use unify_middleware::AttributeStoreExtTrait;
use unify_middleware::AttributeValueState;
use unify_middleware::{Attribute, AttributeStoreTrait, AttributeTypeId};
use unify_middleware::{AttributeEvent, AttributeEventType};
use unify_sl_status_sys::*;
use zpc_attribute_store::AttributeStreamExt;
use zwave_poll_config::read_config_yaml_to_map;
use zwave_poll_config::AttributePollMap;

declare_app_name!("zwave_poll_manager");

#[allow(non_camel_case_types)]
type sl_status_t = u32;

const ATTRIBUTE_ZWAVEPLUS_INFO_Z_WAVE_VERSION: AttributeTypeId = (0x5E << 8) | 0x2;
const ATTRIBUTE_ENDPOINT_ID: AttributeTypeId = 0x4;

//< This represents the Network Status of a node. node_state_topic_state_t
const ATTRIBUTE_NETWORK_STATUS: AttributeTypeId = 0x000D;
const NODE_STATE_TOPIC_STATE_INCLUDED: u8 = 1;

struct PollRegister {
    poll_map: AttributePollMap,
    attribute_poller: Box<dyn AttributePollTrait>,
}

impl PollRegister {
    fn new(
        poll_map: AttributePollMap,
        attribute_poller: impl AttributePollTrait + 'static,
    ) -> PollRegister {
        PollRegister {
            poll_map,
            attribute_poller: Box::new(attribute_poller),
        }
    }

    /// configure a attribute stream that listens for newly included zwave nodes
    /// in the attribute store
    fn create_network_status_stream(&self) -> LocalBoxStream<'static, AttributeEvent> {
        let network_status_update = |event: &AttributeEvent| {
            event.attribute.type_of() == ATTRIBUTE_NETWORK_STATUS
                && event.event_type == AttributeEventType::ATTRIBUTE_UPDATED
                && event.value_state == AttributeValueState::REPORTED_ATTRIBUTE
                && event.attribute.get_reported::<u8>() == Ok(NODE_STATE_TOPIC_STATE_INCLUDED)
        };
        Attribute::attribute_changed_replayed(network_status_update)
    }

    /// Start the [PollRegister].
    /// >This call is not blocking!
    ///
    /// # Arguments
    /// * This function takes a `mut self` as argument, meaning that after this
    ///   call the [PollRegister] object is moved into this function
    fn run(mut self) {
        let task = async move {
            let mut network_status_update_stream = self.create_network_status_stream().fuse();

            loop {
                let event = network_status_update_stream.select_next_some().await;
                if let Ok(node) = event.attribute.get_parent() {
                    self.register_node(node);
                }
            }
        };

        contiki_spawn(task);
    }

    /// This functions schedules poll timers of attributes, part of the given
    /// node. Attributes that don't have a interval configured will not be
    /// polled. see more about poll scheduling in
    /// [unify_attribute_poll::AttributePollTrait]
    ///
    /// Attributes that previously where registered will be updated on a
    /// successive call, if changed. Note that for reschedules there is
    /// a separate interface call available in the
    /// [attribute poll interface](unify_attribute_poll::AttributePollTrait)
    ///
    /// # Arguments
    ///
    /// * `node`   zwave node_id to be registered for polling
    fn register_node(&mut self, node: Attribute) {
        for ep in node.get_children_by_type(ATTRIBUTE_ENDPOINT_ID).unwrap() {
            let config = match get_zwave_plus_info_version(ep) {
                Some(0) => &self.poll_map.0,
                Some(1) => &self.poll_map.1,
                Some(2) => &self.poll_map.2,
                Some(val) => {
                    log_debug!("Z-Wave Plus Info value {} is unknown to us. Applying Z-Wave Plus v2 poll strategy.", val);
                    &self.poll_map.2
                }
                None => &self.poll_map.0,
            };

            for attribute in ep.iter() {
                if let Some(interval) = config.get(&attribute.type_of()) {
                    self.attribute_poller
                        .register(attribute, (*interval).into());
                }
            }
        }
    }
}

/// Find the ATTRIBUTE_ZWAVEPLUS_INFO_Z_WAVE_VERSION for the given endpoint in
/// the attribute store.
///
/// # Arguments
///
/// * `endpoint`    given endpoint
///
/// # Returns
///
/// * `Some(u32)`   when the ATTRIBUTE_ZWAVEPLUS_INFO_Z_WAVE_VERSION is
///   found in the attribute store and no errors occured retrieving it.
/// * `None`        no ATTRIBUTE_ZWAVEPLUS_INFO_Z_WAVE_VERSION found in the
///   attribute store or an error occurred retrieving it
fn get_zwave_plus_info_version(endpoint: Attribute) -> Option<u32> {
    endpoint
        .iter()
        .find(|x| x.type_of() == ATTRIBUTE_ZWAVEPLUS_INFO_Z_WAVE_VERSION)
        .and_then(|a| a.get_reported::<u32>().ok())
}

#[no_mangle]
/// Initialize the attribute poll manager
/// return sl_status_t SL_STATUS_OK on success
pub extern "C" fn zwave_poll_manager_init() -> sl_status_t {
    let config_item = &unify_config_sys::config_get_as_string("zpc.poll.attribute_list_file");
    if let Ok(zwave_poll_config) = config_item.clone() {
        match read_config_yaml_to_map(&zwave_poll_config) {
            Ok(map) => {
                PollRegister::new(map, AttributePoll::default()).run();
                SL_STATUS_OK
            }
            Err(e) => {
                log_error!("parsing errors inside {} - {}", zwave_poll_config, e);
                SL_STATUS_FAIL
            }
        }
    } else {
        log_error!(
            "Cant find poll_attribute_list config file {:?}",
            config_item
        );
        SL_STATUS_FAIL
    }
}

/// Test
#[cfg(test)]
mod poll_manager_test {}
