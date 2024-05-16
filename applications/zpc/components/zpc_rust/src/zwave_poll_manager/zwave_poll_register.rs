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

use super::zwave_poll_config::read_config_yaml_to_map;
use super::zwave_poll_config::AttributePollMap;
use super::zwave_poll_runnable_trait::PollRunnableTrait;
use crate::zpc_attribute_store::AttributeStreamExt;
use crate::zpc_attribute_store::NetworkAttributesTrait;
use futures::StreamExt;
use unify_attribute_poll::attribute_poll_trait::AttributePollTrait;
use unify_attribute_poll::AttributePoll;
use unify_attribute_poll::PollEngineConfig;
use unify_log_sys::*;
use unify_middleware::{
    contiki::contiki_spawn, Attribute, AttributeEvent, AttributeEventType, AttributeTrait,
    AttributeTypeId, AttributeValueState,
};
use unify_sl_status_sys::*;
declare_app_name!("zwave_poll_manager");

pub const ATTRIBUTE_POLL_ENGINE_MARK: AttributeTypeId = 0x0018;

#[allow(non_camel_case_types)]
type sl_status_t = u32;

const ATTRIBUTE_ZWAVEPLUS_INFO_Z_WAVE_VERSION: AttributeTypeId = (0x5E << 8) | 0x2;
const ATTRIBUTE_HOME_ID: AttributeTypeId = 0x2;
const ATTRIBUTE_ENDPOINT_ID: AttributeTypeId = 0x4;

//< DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS is taken from generated file here : components/unify_dotdot_attribute_store/zap-generated/include/unify_dotdot_defined_attribute_types.h
//< 0xfd02 is the cluster ID of Unify_State.xml 0001 is the attribute ID of NetworkStatus
//< Ideally ZAP should generate the .rs file for attributes ID.
const DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS: AttributeTypeId = 0xfd020001;
// This is taken from Unify_State.xml : NodeStateNetworkStatus type
//< Even if this value is an enum8 in the cluster file, it is represented as u32 in the attribute store.
const ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL: u32 = 0;

struct PollRegister {
    poll_map: Option<AttributePollMap>,
    attribute_poller: Box<dyn AttributePollTrait>,
}

struct ZPCPollConfig {
    attribute_list_file: String,
    backoff: i32,
    default_interval: i32,
}

impl PollRegister {
    fn new(attribute_poller: impl AttributePollTrait + 'static) -> PollRegister {
        PollRegister {
            poll_map: None,
            attribute_poller: Box::new(attribute_poller),
        }
    }

    // Verify for an event if the Attribute should be part of the polling.
    // It will verify that the Attribute is located in our HomeID and that
    // the node is "Online functional"
    fn should_node_be_polled<T: NetworkAttributesTrait>(event: &AttributeEvent<Attribute>) -> bool {
        // Get our HomeID attribute. If it cannot be found, nothing will be polled.
        let home_id_attribute = T::get_home_id();
        if !home_id_attribute.valid() {
            return false;
        }

        // Verify that attributes are under our HomeID attribute.
        let mut is_in_home_id: bool = false;
        if let Some(atr) = event
            .attribute
            .get_first_parent_with_type(ATTRIBUTE_HOME_ID)
        {
            is_in_home_id = atr == home_id_attribute;
        }

        // Additional verifications, node has to be Online functional.
        event.attribute.type_of() == DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS
            && event.event_type == AttributeEventType::ATTRIBUTE_UPDATED
            && event.value_state == AttributeValueState::REPORTED_ATTRIBUTE
            && event.attribute.get_reported::<u32>() == Ok(ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL)
            && is_in_home_id
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
        for ep in node.get_children_by_type(ATTRIBUTE_ENDPOINT_ID) {
            let poll_map = self
                .poll_map
                .as_ref()
                .expect("attribute map expected to be set on the start of the run function");

            let config = match get_zwave_plus_info_version(ep) {
                Some(0) => &poll_map.0,
                Some(1) => &poll_map.1,
                Some(2) => &poll_map.2,
                Some(val) => {
                    log_debug!("Z-Wave Plus Info value {} is unknown to us. Applying Z-Wave Plus v2 poll strategy.", val);
                    &poll_map.2
                }
                None => &poll_map.0,
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

impl PollRunnableTrait for PollRegister {
    /// Start the [PollRegister].
    /// >This call is not blocking!
    ///
    /// # Arguments
    /// * This function takes a `mut self` as argument, meaning that after this
    ///   call the [PollRegister] object is moved into this function
    fn run(self, poll_map: AttributePollMap) {
        let mut context = self;
        context.poll_map = Some(poll_map);

        let task = async move {
            let mut network_status_update_stream = Attribute::attribute_changed_replayed(
                PollRegister::should_node_be_polled::<Attribute>,
            )
            .fuse();

            loop {
                let event = network_status_update_stream.select_next_some().await;
                let parent = event.attribute.parent();
                if parent.valid() {
                    context.register_node(parent);
                }
            }
        };

        contiki_spawn(task);
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

fn get_poll_config() -> Result<ZPCPollConfig, unify_config_sys::config_status_t> {
    Ok(ZPCPollConfig {
        attribute_list_file: unify_config_sys::config_get_as_string(
            "zpc.poll.attribute_list_file",
        )?,
        backoff: unify_config_sys::config_get_as_int("zpc.poll.backoff")?,
        default_interval: unify_config_sys::config_get_as_int("zpc.poll.default_interval")?,
    })
}

fn load_poll_map_from_config(file_name: String) -> Result<AttributePollMap, sl_status_t> {
    let config_item = std::path::PathBuf::from(file_name);

    if !config_item.exists() {
        log_warning!(
            "Attribute poller will not run. Cant find poll_attribute_list config file {:?}",
            config_item
        );
        return Err(SL_STATUS_NOT_FOUND);
    }

    read_config_yaml_to_map(&config_item).map_err(|e| {
        log_error!(
            "parsing errors inside {} - {}",
            config_item.to_string_lossy(),
            e
        );
        SL_STATUS_NOT_SUPPORTED
    })
}

#[no_mangle]
/// Initialize the attribute poll manager
/// return sl_status_t always returns SL_STATUS_OK
pub extern "C" fn zwave_poll_manager_init() -> sl_status_t {
    if let Ok(config) = get_poll_config() {
        if let Ok(poll_map) = load_poll_map_from_config(config.attribute_list_file) {
            AttributePoll::default().initialize(PollEngineConfig {
                backoff: config.backoff as u32,
                default_interval: config.default_interval as u32,
                poll_mark_attribute_type: ATTRIBUTE_POLL_ENGINE_MARK as AttributeTypeId,
            });

            let poll_register = PollRegister::new(AttributePoll::default());
            poll_register.run(poll_map);
        }
    }
    SL_STATUS_OK
}

/// Test
#[cfg(test)]
mod poll_manager_test {
    use crate::{
        zpc_attribute_store, zwave_poll_manager::zwave_poll_runnable_trait::MockPollRunnableTrait,
    };

    use super::PollRegister;
    use super::*;
    use unify_middleware::{Attribute, AttributeStore, AttributeStoreTrait};

    struct MockObject;

    // Mock implementation for the ZPC network nodes
    // Handle 1 is HomeID, 2 is NodeID, 3 is endpoint ID.
    impl zpc_attribute_store::NetworkAttributesTrait for MockObject {
        fn get_home_id() -> Attribute {
            AttributeStore::new().unwrap().from_handle(1)
        }
        // Retrieves the NodeID attribute of the ZPC in its current network
        fn get_zpc_node_id() -> Attribute {
            AttributeStore::new().unwrap().from_handle(2)
        }
        // Retrives the Endpoint ID (0) of the ZPC NodeID in its current network.
        fn get_zpc_endpoint_id() -> Attribute {
            AttributeStore::new().unwrap().from_handle(3)
        }
    }

    #[test]
    fn should_node_be_polled_test() {
        let mut test_event: AttributeEvent<Attribute> = AttributeEvent::default();
        assert_eq!(
            false,
            PollRegister::should_node_be_polled::<MockObject>(&test_event)
        );

        // HomeID should not do anything either
        test_event.attribute.handle = 1;
        assert_eq!(
            false,
            PollRegister::should_node_be_polled::<MockObject>(&test_event)
        );

        // Desired update should not trigger evaluation for polling
        test_event.value_state = AttributeValueState::DESIRED_ATTRIBUTE;
        assert_eq!(
            false,
            PollRegister::should_node_be_polled::<MockObject>(&test_event)
        );

        // TODO: Can't get it to return true unless we create a real attribute store.
        // or mock the `get_first_parent_with_type()` function
    }

    #[test]
    fn continue_when_config_does_not_exist() {
        // This does not currently work due to linker problems
        //assert_eq!(zwave_poll_manager_init(), SL_STATUS_OK)
    }
}
