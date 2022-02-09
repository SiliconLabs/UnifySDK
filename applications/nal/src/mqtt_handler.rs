// NAL is "Name and Location"

use crate::cache::*;
use unify_log_sys::*;
use unify_middleware::unify_mqtt_client::{
    sl_status_t, MosqMessage, MqttClientCallbacksTrait, MqttClientTrait, TopicMatcherType,
    UnifyMqttClient,
};

/// MqttNALHandler handles subscription messages related to Name and Location service.
/// It subscribes to listen to the correct topics and is to be passed to the mosquitto client.
pub struct MqttNALHandler<T: MqttClientTrait> {
    client: T,
    /// Lookup cache for nodes
    cache: Cache,
    /// Matches "ucl/by-unid/+/State" topic
    state_matcher: TopicMatcherType,
    /// Matches "ucl/by-unid/+/+/#" topic
    attributes_matcher: TopicMatcherType,
}

declare_app_name!("mqtt_handler");

impl<T: MqttClientTrait> MqttClientCallbacksTrait for MqttNALHandler<T> {
    /// public function to be called by the mosquitto fwk on a received message.
    fn on_message(&mut self, msg: MosqMessage) {
        if let Err(e) = self.process_message(msg) {
            if !e.is_empty() {
                log_error!("{}", e);
            }
        }
    }
    fn before_disconnect(&mut self) {
        self.service_stop_callback();
    }
    fn after_connect(&mut self) {}
}

impl<T: MqttClientTrait> MqttNALHandler<T> {
    pub fn new(client: T, nal_config: NALConfig) -> Result<Self, sl_status_t> {
        let state = client.subscribe("ucl/by-unid/+/State")?;
        let attributes = client.subscribe("ucl/by-unid/+/+/#")?;

        Ok(MqttNALHandler {
            client: client,
            state_matcher: state,
            attributes_matcher: attributes,
            cache: Cache::new(nal_config),
        })
    }

    fn process_message(&mut self, msg: MosqMessage) -> std::result::Result<(), String> {
        log_debug!(
            "Incoming MQTT Publication - {} - Payload : {}",
            msg.topic(),
            msg.text()
        );
        let payload: &str = msg.text();

        // Now dispatch the payload/topic to the handler
        if self.state_matcher.matches(&msg) {
            self.on_state_reported(msg.topic(), payload)
        } else if self.attributes_matcher.matches(&msg) {
            // Get our payload into a JSON object.
            let json: json::JsonValue = json::parse(payload).map_err(|e| {
                format!(
                    "Could not parse payload for topic '{}' : {}",
                    msg.topic(),
                    e
                )
            })?;
            self.on_attributes_reported(msg.topic(), json)
        } else {
            Err(format!("no handler for subscribed topic {}", msg.topic()))
        }
    }

    /// Implements a `"ucl/by-unid/+/State"` topic handler.
    /// # Description
    ///
    /// If payload is empty:
    /// searches for the node in cache and deletes this node and unretain it's publications to such topics:
    ///
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Reported`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Reported`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/SupportedCommands`
    ///
    /// If payload is *not* empty - searches for the node in cache
    /// if found - does nothing;
    /// if *not* found - adds a new node to the cache and publish `{"value":""}` message to such topics:
    ///
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Reported`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Reported`
    ///
    /// and publish `"{"value":["WriteAttributes"]}"` message to `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/SupportedCommands` topic
    fn on_state_reported(&mut self, topic: &str, payload: &str) -> std::result::Result<(), String> {
        let unid = get_unid_from_topic(topic);
        // If payload of "ucl/by-unid/+/State" topic is empty - all nodes with matched unid should be deleted
        let is_delete: bool = payload.is_empty();
        let key = NodeEpKey {
            unid: unid.clone(),
            ep_id: 0, // the default endpoint ID value is '0'
        };

        if is_delete {
            self.remove_nodes_via_name(unid.as_str())?;
        } else if false == self.cache.node_presents(&key) {
            // If node with such unid and endpoint ID "0" already presents - do nothing.
            // If it doesn't - create a new node.
            let attributes = Attributes::default();
            self.node_create(&key, &attributes)?;

            // check for delayed messages for this UNID
            self.exec_delayed_messages(unid.as_str())?;
        }

        Ok(())
    }

    /// Implements a `"ucl/by-unid/+/+/#"` topic handler.
    /// # Description
    ///
    /// Searches for the unid/endpoint ID "0" key in cache
    /// If node with ep_id "0" doesn't present - this node can't be processed
    ///
    /// Searches for the unid/endpoint_ID key in cache
    /// If *not* found:
    /// create a new cache node with a given unid/endpoint_ID
    ///
    /// If topic ends with `"WriteAttributes"` - parse payload
    /// If payload is Json with key either "Name" or "Location" - update node in cache
    /// If payload has another format - do nothing
    ///
    /// If penultimate topic chunk is `"LocationDescription"` and topic ends with either `"Desired"` or `"Reported"` -
    /// update the desired/reported location of a given node.
    ///
    /// For every node update operation - update the payload of retained messages for such topics:
    ///
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Reported`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Reported`
    ///
    /// The node may be "proxied" by the Protocol Controller - this means it shouldn't update it's "Location" attribute directly after
    /// "WriteAttributes" message received, but it has to send a message with `"{"LocationDescription":"<new_location>"}"` payload
    /// to `ucl/by-unid/<UNID>/ep<endpoint_id>/Basic/Commands/WriteAttributes` topic (that is handled by the Protocol Controller)
    /// and update the "Location" attribute value only after Protocol Controller accepts it, by publishing a message to
    /// `ucl/by-unid/<UNID>/ep<endpoint_id>/Basic/Attributes/LocationDescription/<Desired/Reported>`.
    fn on_attributes_reported(
        &mut self,
        topic: &str,
        json: json::JsonValue,
    ) -> std::result::Result<(), String> {
        let penultimate_topic_chunk = get_penultimate_topic_chunk(topic);
        let last_topic_chunk = get_last_topic_chunk(topic);
        let unid = get_unid_from_topic(topic);
        let ep_id = get_ep_id_from_topic(topic)?;

        let key = NodeEpKey {
            unid: unid.clone(),
            ep_id: ep_id,
        };

        {
            // if a node with such unid and endpoint ID 0 doesn't present - the node can't be processed right now
            // but we have to save this message to process it later if the State topic message for such UNID will be received
            let key_ep0 = NodeEpKey {
                unid: unid.clone(),
                ep_id: 0,
            };
            if false == self.cache.node_presents(&key_ep0) {
                self.cache
                    .add_delayed_message(unid.as_str(), topic, json.dump().as_str())?;
                return Err(String::from(""));
            }
        }

        // if a new endpoint was detected - add a node in the cache
        if false == self.cache.node_presents(&key) {
            let attributes = Attributes::default();

            self.node_create(&key, &attributes)?;
        }

        if last_topic_chunk == "WriteAttributes" {
            if json["Name"] != json::Null {
                let new_name: &str = json["Name"].as_str().unwrap();

                let mut attributes = Attributes::default();
                self.cache.node_get(&key, &mut attributes)?;
                attributes.name_desired = new_name.to_string();
                attributes.name_reported = new_name.to_string();

                self.cache.node_set(&key, &attributes)?;

                self.pub_attr_name_msgs(&key, &attributes, true, true)?;
            }

            if json["Location"] != json::Null {
                let new_location: &str = json["Location"].as_str().unwrap();

                let mut attributes = Attributes::default();
                self.cache.node_get(&key, &mut attributes)?;
                if attributes.is_proxied {
                    let topic = format!(
                        "ucl/by-unid/{}/ep{}/Basic/Commands/WriteAttributes",
                        key.unid, key.ep_id
                    );
                    let payload = json::object! {"LocationDescription" => json::JsonValue::from(new_location)}.dump();
                    self.client
                        .publish(&topic.as_str(), &payload.as_bytes(), false)
                        .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))?;
                } else {
                    attributes.location_desired = new_location.to_string();
                    attributes.location_reported = new_location.to_string();

                    self.cache.node_set(&key, &attributes)?;

                    self.pub_attr_location_msgs(&key, &attributes, true, true)?;
                }
            }
        } else if penultimate_topic_chunk == "LocationDescription" && json["value"] != json::Null {
            let new_location: &str = json["value"].as_str().unwrap();

            if last_topic_chunk == "Desired" {
                let mut attributes = Attributes::default();
                self.cache.node_get(&key, &mut attributes)?;
                attributes.is_proxied = true;
                attributes.location_desired = new_location.to_string();
                self.cache.node_set(&key, &attributes)?;

                self.pub_attr_location_msgs(&key, &attributes, true, false)?;
            } else if last_topic_chunk == "Reported" {
                let mut attributes = Attributes::default();
                self.cache.node_get(&key, &mut attributes)?;
                attributes.is_proxied = true;
                attributes.location_reported = new_location.to_string();
                self.cache.node_set(&key, &attributes)?;

                self.pub_attr_location_msgs(&key, &attributes, false, true)?;
            }
        }

        Ok(())
    }

    /// Removes all nodes with a given "unid" from cache
    fn remove_nodes_via_name(&mut self, unid: &str) -> std::result::Result<(), String> {
        let nodes_to_remove: Vec<NodeEpKey> = self.cache.get_all_nodes_match_name(unid).unwrap();

        for it in &nodes_to_remove {
            self.node_delete(&it)?;
        }

        Ok(())
    }

    /// Adds a new key/value pair to the cache with a given node key/attributes values.
    /// Publishes Name and Location values to such topics (retained messages):
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Reported`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Reported`
    /// Publishes the `"{"value":["WriteAttributes"]}"` message to topic:
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/SupportedCommands`
    fn node_create(
        &mut self,
        key: &NodeEpKey,
        attributes: &Attributes,
    ) -> std::result::Result<(), String> {
        self.cache.node_set(key, attributes)?;

        self.pub_attr_name_msgs(key, attributes, true, true)?;
        self.pub_attr_location_msgs(key, attributes, true, true)?;

        {
            let topic = format!(
                "ucl/by-unid/{}/ep{}/NameAndLocation/SupportedCommands",
                key.unid, key.ep_id
            );
            let payload = String::from("{\"value\":[\"WriteAttributes\"]}");
            self.client
                .publish(&topic.as_str(), &payload.as_str().as_bytes(), true)
                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))?;
        }

        Ok(())
    }

    /// Deletes node with a given unid/endpoint_id key from the cache.
    /// Unretain all publications, related to this node (see `unretain_all_node_messages()`)
    fn node_delete(&mut self, key: &NodeEpKey) -> std::result::Result<(), String> {
        self.cache.node_remove(key)?;

        // unretain all publications
        self.unretain_all_node_messages(key)?;

        Ok(())
    }

    /// Unretain all publications to such topics (by publishing an empty message):
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Reported`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Reported`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/SupportedCommands`
    fn unretain_all_node_messages(&mut self, key: &NodeEpKey) -> std::result::Result<(), String> {
        let mqtt_client = UnifyMqttClient::default();

        {
            let topic = format!(
                "ucl/by-unid/{}/ep{}/NameAndLocation/Attributes/Name/Desired",
                key.unid, key.ep_id
            );
            let _msg_id = mqtt_client
                .publish(&topic, &[], true)
                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e));
        }
        {
            let topic = format!(
                "ucl/by-unid/{}/ep{}/NameAndLocation/Attributes/Name/Reported",
                key.unid, key.ep_id
            );
            let _msg_id = mqtt_client
                .publish(&topic, &[], true)
                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e));
        }
        {
            let topic = format!(
                "ucl/by-unid/{}/ep{}/NameAndLocation/Attributes/Location/Desired",
                key.unid, key.ep_id
            );
            let _msg_id = mqtt_client
                .publish(&topic, &[], true)
                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e));
        }
        {
            let topic = format!(
                "ucl/by-unid/{}/ep{}/NameAndLocation/Attributes/Location/Reported",
                key.unid, key.ep_id
            );
            let _msg_id = mqtt_client
                .publish(&topic, &[], true)
                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e));
        }
        {
            let topic = format!(
                "ucl/by-unid/{}/ep{}/NameAndLocation/SupportedCommands",
                key.unid, key.ep_id
            );
            let _msg_id = mqtt_client
                .publish(&topic, &[], true)
                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e));
        }

        Ok(())
    }

    /// # Description
    ///
    /// Publishes messages to such topics:
    ///
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Reported`
    ///
    /// The payload is "Name" attribute value.
    ///
    /// # Arguments
    ///
    /// * pub_des - if true - publishes the "Desired"  topic
    /// * pub_rep - if true - publishes the "Reported" topic
    fn pub_attr_name_msgs(
        &mut self,
        key: &NodeEpKey,
        attributes: &Attributes,
        pub_des: bool,
        pub_rep: bool,
    ) -> std::result::Result<(), String> {
        if pub_des {
            let topic = format!(
                "ucl/by-unid/{}/ep{}/NameAndLocation/Attributes/Name/Desired",
                key.unid, key.ep_id
            );
            let payload = format!("{{\"value\":\"{}\"}}", attributes.name_desired);
            self.client
                .publish(&topic.as_str(), &payload.as_str().as_bytes(), true)
                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))?;
        }

        if pub_rep {
            let topic = format!(
                "ucl/by-unid/{}/ep{}/NameAndLocation/Attributes/Name/Reported",
                key.unid, key.ep_id
            );
            let payload = format!("{{\"value\":\"{}\"}}", attributes.name_reported);
            self.client
                .publish(&topic.as_str(), &payload.as_str().as_bytes(), true)
                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))?;
        }

        Ok(())
    }

    /// # Description
    ///
    /// Publishes messages to such topics:
    ///
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Reported`
    ///
    /// The payload is "Location" attribute value.
    ///
    /// # Arguments
    ///
    /// * pub_des - if true - publishes the "Desired"  topic
    /// * pub_rep - if true - publishes the "Reported" topic
    fn pub_attr_location_msgs(
        &mut self,
        key: &NodeEpKey,
        attributes: &Attributes,
        pub_des: bool,
        pub_rep: bool,
    ) -> std::result::Result<(), String> {
        if pub_des {
            let topic = format!(
                "ucl/by-unid/{}/ep{}/NameAndLocation/Attributes/Location/Desired",
                key.unid, key.ep_id
            );
            let payload = format!("{{\"value\":\"{}\"}}", attributes.location_desired);
            self.client
                .publish(&topic.as_str(), &payload.as_str().as_bytes(), true)
                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))?;
        }

        if pub_rep {
            let topic = format!(
                "ucl/by-unid/{}/ep{}/NameAndLocation/Attributes/Location/Reported",
                key.unid, key.ep_id
            );
            let payload = format!("{{\"value\":\"{}\"}}", attributes.location_reported);
            self.client
                .publish(&topic.as_str(), &payload.as_str().as_bytes(), true)
                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))?;
        }

        Ok(())
    }

    /// Gets all delayed messages for provided UNID and processes them -
    /// calls on_attributes_reported() function with topic and payload got from the delayed message.
    fn exec_delayed_messages(&mut self, unid: &str) -> std::result::Result<(), String> {
        let msgs_to_exec: Vec<(String, String)> =
            self.cache.get_all_delayed_messages(unid).unwrap();

        for it in &msgs_to_exec {
            let topic = it.0.clone();
            let payload = it.1.clone();
            // Get our payload into a JSON object.
            let json: json::JsonValue = json::parse(payload.as_str())
                .map_err(|e| format!("Could not parse payload for topic '{}' : {}", topic, e))?;
            self.on_attributes_reported(topic.as_str(), json)?
        }

        Ok(())
    }

    /// Unretain all messages for all nodes.
    fn service_stop_callback(&mut self) {
        let nodes_to_unretain: Vec<NodeEpKey> = self.cache.get_all_nodes().unwrap();

        for it in &nodes_to_unretain {
            let _ = self.unretain_all_node_messages(&it);
        }
    }
}

/// Parses topic and extracts it's penultimate chunk.
/// I.e. for topic `ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Reported`
/// the "Location" string will be returned.
fn get_penultimate_topic_chunk(topic: &str) -> String {
    let list = topic.split_terminator('/').collect::<Vec<&str>>();
    let index: usize;
    if list.len() >= 2 {
        index = list.len() - 2;
    } else {
        return String::from("");
    }
    let penult_chunk = list[index];
    penult_chunk.to_string()
}

/// Parses topic and extracts it's last chunk.
/// I.e. for topic `ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Reported`
/// the "Reported" string will be returned.
fn get_last_topic_chunk(topic: &str) -> String {
    let list = topic.split_terminator('/').collect::<Vec<&str>>();
    let last_chunk = list.last().unwrap();
    last_chunk.to_string()
}

/// Parses topic and extracts unid.
/// I.e. for topic `ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Reported`
/// the "node_1" string will be returned.
fn get_unid_from_topic(topic: &str) -> String {
    let list = topic.split_terminator('/').collect::<Vec<&str>>();
    let unid = list[2];
    unid.to_string()
}

/// Parses topic and extracts endpoint_ID.
/// I.e. for topic `ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Reported`
/// the 0:u16 value will be returned.
fn get_ep_id_from_topic(topic: &str) -> std::result::Result<u16, String> {
    let list = topic.split_terminator('/').collect::<Vec<&str>>();
    let stripped = list[3].strip_prefix("ep").ok_or("")?;
    let ep_id = stripped
        .parse::<u16>()
        .map_err(|e| format!("could not parse {} to integer. {}", stripped, e))?;

    Ok(ep_id)
}

#[cfg(test)]
mod tests {
    use super::*;
    use mockall::predicate;
    use mockall::predicate::eq;
    use serial_test::serial;
    use unify_middleware::unify_mqtt_client::{MockMqttClientTrait, MockTopicMatcherTrait};

    fn create_mocked_mqtt_handler() -> MqttNALHandler<MockMqttClientTrait> {
        let mut mqtt_client_mock = MockMqttClientTrait::new();
        let state_topic_matcher_mock: Result<TopicMatcherType, sl_status_t> =
            Ok(Box::new(MockTopicMatcherTrait::new()));
        mqtt_client_mock
            .expect_subscribe()
            .with(eq("ucl/by-unid/+/State"))
            .return_once_st(|_| state_topic_matcher_mock);
        let attributes_topic_matcher_mock: Result<TopicMatcherType, sl_status_t> =
            Ok(Box::new(MockTopicMatcherTrait::new()));
        mqtt_client_mock
            .expect_subscribe()
            .with(eq("ucl/by-unid/+/+/#"))
            .return_once_st(|_| attributes_topic_matcher_mock);

        MqttNALHandler::new(mqtt_client_mock, NALConfig::new_for_unit_tests()).unwrap()
    }

    #[test]
    fn test_get_penultimate_topic_chunk() {
        let test_topic = "ucl/by-unid/some_unid/State";
        assert_eq!(
            get_penultimate_topic_chunk(test_topic),
            String::from("some_unid")
        );
    }

    #[test]
    fn test_get_last_topic_chunk() {
        let test_topic = "ucl/by-unid/unid/State";
        assert_eq!(get_last_topic_chunk(test_topic), String::from("State"));
    }

    #[test]
    fn test_get_unid_from_topic() {
        let test_topic = "ucl/by-unid/some_unid/State";
        assert_eq!(get_unid_from_topic(test_topic), String::from("some_unid"));
    }

    #[test]
    fn test_get_ep_id_from_topic() {
        let test_topic = "ucl/by-unid/unid/ep5/NameAndLocation/Commands/WriteAttributes";
        let res: u16 = get_ep_id_from_topic(test_topic).unwrap();
        assert_eq!(res, 5);
    }

    macro_rules! EXPECT_PUBLISH_EMPTY {
        ($mqtt_handler:ident, $topic:expr) => {{
            let expected = predicate::function(|x| x == "".as_bytes());
            $mqtt_handler
                .client
                .expect_publish()
                .with(eq($topic), expected, eq(true))
                .return_once_st(|_, _, _| Ok(()));
        }};
    }

    macro_rules! EXPECT_PUBLISH_VALUE_EMPTY {
        ($mqtt_handler:ident, $topic:expr) => {{
            let expected = predicate::function(|x| {
                x == json::object! {"value" => json::JsonValue::from("")}
                    .dump()
                    .as_bytes()
            });
            $mqtt_handler
                .client
                .expect_publish()
                .with(eq($topic), expected, eq(true))
                .return_once_st(|_, _, _| Ok(()));
        }};
    }

    macro_rules! EXPECT_PUBLISH_VALUE {
        ($mqtt_handler:ident, $topic:expr, $value:expr) => {{
            let expected = predicate::function(|x| {
                x == json::object! {"value" => json::JsonValue::from($value)}
                    .dump()
                    .as_bytes()
            });
            $mqtt_handler
                .client
                .expect_publish()
                .with(eq($topic), expected, eq(true))
                .return_once_st(|_, _, _| Ok(()));
        }};
    }

    macro_rules! CHECK_NODE_PRESENCE_IN_CACHE {
        ($mqtt_handler:ident, $unid:expr, $ep_id:expr, $expected:expr) => {{
            let key = NodeEpKey {
                unid: String::from($unid),
                ep_id: $ep_id,
            };
            assert_eq!($mqtt_handler.cache.node_presents(&key), $expected);
        }};
    }

    #[test]
    #[serial]
    fn test_add_node() {
        let mut mqtt_nal_handler = create_mocked_mqtt_handler();
        mqtt_nal_handler.cache.clean_up_db_for_unit_tests();

        let mock_topic = "ucl/by-unid/test_node_1/State";
        let payload = "some_dummy_data";

        // test adding node
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Name/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Name/Reported"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Reported"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/SupportedCommands",
            vec!["WriteAttributes"]
        );

        mqtt_nal_handler
            .on_state_reported(mock_topic, payload)
            .unwrap();

        let key_presents = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 0,
        };
        let key_not_pres_1 = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 1,
        };
        let key_not_pres_2 = NodeEpKey {
            unid: String::from("wrong_node"),
            ep_id: 0,
        };

        assert_eq!(mqtt_nal_handler.cache.node_presents(&key_presents), true);
        assert_eq!(mqtt_nal_handler.cache.node_presents(&key_not_pres_1), false);
        assert_eq!(mqtt_nal_handler.cache.node_presents(&key_not_pres_2), false);
    }

    #[test]
    #[serial]
    fn test_delete_node() {
        let mut mqtt_nal_handler = create_mocked_mqtt_handler();
        mqtt_nal_handler.cache.clean_up_db_for_unit_tests();

        let mock_topic = "ucl/by-unid/test_node_1/State";

        let key = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 0,
        };
        let attributes = Attributes::default();

        assert_eq!(mqtt_nal_handler.cache.node_set(&key, &attributes), Ok(()));

        EXPECT_PUBLISH_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Name/Desired"
        );
        EXPECT_PUBLISH_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Name/Reported"
        );
        EXPECT_PUBLISH_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Desired"
        );
        EXPECT_PUBLISH_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Reported"
        );
        EXPECT_PUBLISH_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/SupportedCommands"
        );

        mqtt_nal_handler.on_state_reported(mock_topic, "").unwrap();

        assert_eq!(mqtt_nal_handler.cache.node_presents(&key), false);
    }

    #[test]
    #[serial]
    fn test_add_delete_multiple_nodes() {
        let mut mqtt_nal_handler = create_mocked_mqtt_handler();
        mqtt_nal_handler.cache.clean_up_db_for_unit_tests();

        // add node "test_node_1", endpoint 0
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Name/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Name/Reported"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Reported"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/SupportedCommands",
            vec!["WriteAttributes"]
        );

        mqtt_nal_handler
            .on_state_reported("ucl/by-unid/test_node_1/State", "some_dummy_data")
            .unwrap();

        // add node "test_node_2", endpoint 0
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_2/ep0/NameAndLocation/Attributes/Name/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_2/ep0/NameAndLocation/Attributes/Name/Reported"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_2/ep0/NameAndLocation/Attributes/Location/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_2/ep0/NameAndLocation/Attributes/Location/Reported"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_2/ep0/NameAndLocation/SupportedCommands",
            vec!["WriteAttributes"]
        );

        mqtt_nal_handler
            .on_state_reported("ucl/by-unid/test_node_2/State", "some_dummy_data")
            .unwrap();

        // add node "test_node_1", endpoint 1
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep1/NameAndLocation/Attributes/Name/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep1/NameAndLocation/Attributes/Name/Reported"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep1/NameAndLocation/Attributes/Location/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep1/NameAndLocation/Attributes/Location/Reported"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep1/NameAndLocation/SupportedCommands",
            vec!["WriteAttributes"]
        );

        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(
                "ucl/by-unid/test_node_1/ep1/Basic/Attributes/PowerSource/Reported",
                json::parse("{\"value\":\"some_dummy_data_to_detect_endpoint\"}").unwrap()
            ),
            Ok(())
        );

        // add node "test_node_2", endpoint 1
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_2/ep1/NameAndLocation/Attributes/Name/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_2/ep1/NameAndLocation/Attributes/Name/Reported"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_2/ep1/NameAndLocation/Attributes/Location/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_2/ep1/NameAndLocation/Attributes/Location/Reported"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_2/ep1/NameAndLocation/SupportedCommands",
            vec!["WriteAttributes"]
        );

        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(
                "ucl/by-unid/test_node_2/ep1/Basic/Attributes/PowerSource/Reported",
                json::parse("{\"value\":\"some_dummy_data_to_detect_endpoint\"}").unwrap()
            ),
            Ok(())
        );

        // verify all 4 nodes presents in cache
        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node_1", 0, true);
        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node_1", 1, true);
        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node_2", 0, true);
        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node_2", 1, true);

        // delete nodes with unid 'test_node_1'
        EXPECT_PUBLISH_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Name/Desired"
        );
        EXPECT_PUBLISH_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Name/Reported"
        );
        EXPECT_PUBLISH_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Desired"
        );
        EXPECT_PUBLISH_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Reported"
        );
        EXPECT_PUBLISH_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/SupportedCommands"
        );

        EXPECT_PUBLISH_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep1/NameAndLocation/Attributes/Name/Desired"
        );
        EXPECT_PUBLISH_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep1/NameAndLocation/Attributes/Name/Reported"
        );
        EXPECT_PUBLISH_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep1/NameAndLocation/Attributes/Location/Desired"
        );
        EXPECT_PUBLISH_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep1/NameAndLocation/Attributes/Location/Reported"
        );
        EXPECT_PUBLISH_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep1/NameAndLocation/SupportedCommands"
        );

        mqtt_nal_handler
            .on_state_reported("ucl/by-unid/test_node_1/State", "")
            .unwrap();

        // verify only nodes with unid 'test_node_2' present in cache
        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node_1", 0, false);
        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node_1", 1, false);
        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node_2", 0, true);
        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node_2", 1, true);
    }

    #[test]
    #[serial]
    fn test_write_attributes() {
        let mut mqtt_nal_handler = create_mocked_mqtt_handler();
        mqtt_nal_handler.cache.clean_up_db_for_unit_tests();

        let key = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 0,
        };
        let attributes = Attributes::default();

        assert_eq!(mqtt_nal_handler.cache.node_set(&key, &attributes), Ok(()));

        // check Name
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Name/Desired",
            "New name"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Name/Reported",
            "New name"
        );

        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(
                "ucl/by-unid/test_node_1/ep0/NameAndLocation/Commands/WriteAttributes",
                json::parse("{\"Name\":\"New name\"}").unwrap()
            ),
            Ok(())
        );
        {
            let mut attributes = Attributes::default();
            mqtt_nal_handler
                .cache
                .node_get(&key, &mut attributes)
                .unwrap();
            assert_eq!(attributes.name_desired, String::from("New name"));
            assert_eq!(attributes.name_reported, String::from("New name"));
        }

        // check Location
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Desired",
            "New location"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Reported",
            "New location"
        );

        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(
                "ucl/by-unid/test_node_1/ep0/NameAndLocation/Commands/WriteAttributes",
                json::parse("{\"Location\":\"New location\"}").unwrap()
            ),
            Ok(())
        );
        {
            let mut attributes = Attributes::default();
            mqtt_nal_handler
                .cache
                .node_get(&key, &mut attributes)
                .unwrap();
            assert_eq!(attributes.location_desired, String::from("New location"));
            assert_eq!(attributes.location_reported, String::from("New location"));
        }
    }

    #[test]
    #[serial]
    fn test_non_existing_unid() {
        let mut mqtt_nal_handler = create_mocked_mqtt_handler();
        mqtt_nal_handler.cache.clean_up_db_for_unit_tests();

        // add node "test_node_1", endpoint 1
        // node with unid "test_node_1" doesn't exist (there wasn't pub to "ucl/by-unid/test_node_1/State" topic)
        // so the endpoint 1 node should not be added
        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(
                "ucl/by-unid/test_node_1/ep1/Basic/Attributes/PowerSource/Reported",
                json::parse("{\"value\":\"some_dummy_data_to_detect_endpoint\"}").unwrap()
            ),
            Err(String::from(""))
        );
    }

    #[test]
    #[serial]
    fn test_attributes_topic_received_before_state_topic() {
        let mut mqtt_nal_handler = create_mocked_mqtt_handler();
        mqtt_nal_handler.cache.clean_up_db_for_unit_tests();

        let key_ep_1 = NodeEpKey {
            unid: String::from("test_node"),
            ep_id: 1,
        };

        // the error is returned, but a row with delayed message should be added to nal_delayed_messages table
        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(
                "ucl/by-unid/test_node/ep1/NameAndLocation/Commands/WriteAttributes",
                json::parse("{\"Name\":\"name\"}").unwrap()
            ),
            Err(String::from(""))
        );

        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node", 1, false);

        // pub to State topic to create an "endpoint 0" node and execute delayed message
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep0/NameAndLocation/Attributes/Name/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep0/NameAndLocation/Attributes/Name/Reported"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep0/NameAndLocation/Attributes/Location/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep0/NameAndLocation/Attributes/Location/Reported"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep0/NameAndLocation/SupportedCommands",
            vec!["WriteAttributes"]
        );

        // expect publishing all "endpoint 1" topics with already updated "name" attributes
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep1/NameAndLocation/Attributes/Name/Desired",
            ""
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep1/NameAndLocation/Attributes/Name/Reported",
            ""
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep1/NameAndLocation/Attributes/Name/Desired",
            "name"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep1/NameAndLocation/Attributes/Name/Reported",
            "name"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep1/NameAndLocation/Attributes/Location/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep1/NameAndLocation/Attributes/Location/Reported"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep1/NameAndLocation/SupportedCommands",
            vec!["WriteAttributes"]
        );

        mqtt_nal_handler
            .on_state_reported("ucl/by-unid/test_node/State", "some_data")
            .unwrap();

        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node", 0, true);
        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node", 1, true);

        let mut attributes = Attributes::default();
        mqtt_nal_handler
            .cache
            .node_get(&key_ep_1, &mut attributes)
            .unwrap();

        assert_eq!(attributes.name_desired, String::from("name"));
        assert_eq!(attributes.name_reported, String::from("name"));
    }

    #[test]
    #[serial]
    fn test_proxying() {
        let mut mqtt_nal_handler = create_mocked_mqtt_handler();
        mqtt_nal_handler.cache.clean_up_db_for_unit_tests();

        let mock_topic = "ucl/by-unid/test_node_1/State";
        let payload = "some_dummy_data";

        let key = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 0,
        };

        // add node
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Name/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Name/Reported"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Reported"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/SupportedCommands",
            vec!["WriteAttributes"]
        );

        mqtt_nal_handler
            .on_state_reported(mock_topic, payload)
            .unwrap();

        // Protocol controller takes control of node
        // test Desired location
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Desired",
            "New location"
        );

        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(
                "ucl/by-unid/test_node_1/ep0/Basic/Attributes/LocationDescription/Desired",
                json::parse("{\"value\":\"New location\"}").unwrap()
            ),
            Ok(())
        );
        {
            let mut attributes = Attributes::default();
            mqtt_nal_handler
                .cache
                .node_get(&key, &mut attributes)
                .unwrap();
            assert_eq!(attributes.location_desired, String::from("New location"));
            assert_eq!(attributes.is_proxied, true);
        }

        // test Reported location
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Reported",
            "New location"
        );

        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(
                "ucl/by-unid/test_node_1/ep0/Basic/Attributes/LocationDescription/Reported",
                json::parse("{\"value\":\"New location\"}").unwrap()
            ),
            Ok(())
        );
        {
            let mut attributes = Attributes::default();
            mqtt_nal_handler
                .cache
                .node_get(&key, &mut attributes)
                .unwrap();
            assert_eq!(attributes.location_reported, String::from("New location"));
            assert_eq!(attributes.is_proxied, true);
        }

        // test proxying
        {
            let expected = predicate::function(|x| {
                x == json::object! {"LocationDescription" => json::JsonValue::from("Updated location")}.dump().as_bytes()
            });
            mqtt_nal_handler
                .client
                .expect_publish()
                .with(
                    eq("ucl/by-unid/test_node_1/ep0/Basic/Commands/WriteAttributes"),
                    expected,
                    eq(false),
                )
                .return_once_st(|_, _, _| Ok(()));
        }
        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(
                "ucl/by-unid/test_node_1/ep0/NameAndLocation/Commands/WriteAttributes",
                json::parse("{\"Location\":\"Updated location\"}").unwrap()
            ),
            Ok(())
        );

        // simulate that the Protocol controller received message and made a resonce:
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Desired",
            "Updated location"
        );

        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(
                "ucl/by-unid/test_node_1/ep0/Basic/Attributes/LocationDescription/Desired",
                json::parse("{\"value\":\"Updated location\"}").unwrap()
            ),
            Ok(())
        );
        {
            let mut attributes = Attributes::default();
            mqtt_nal_handler
                .cache
                .node_get(&key, &mut attributes)
                .unwrap();
            assert_eq!(
                attributes.location_desired,
                String::from("Updated location")
            );
        }

        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Reported",
            "Updated location"
        );

        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(
                "ucl/by-unid/test_node_1/ep0/Basic/Attributes/LocationDescription/Reported",
                json::parse("{\"value\":\"Updated location\"}").unwrap()
            ),
            Ok(())
        );
        {
            let mut attributes = Attributes::default();
            mqtt_nal_handler
                .cache
                .node_get(&key, &mut attributes)
                .unwrap();
            assert_eq!(
                attributes.location_reported,
                String::from("Updated location")
            );
        }

        // verify that name changing wasn't affected
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Name/Desired",
            "New name"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Name/Reported",
            "New name"
        );

        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(
                "ucl/by-unid/test_node_1/ep0/NameAndLocation/Commands/WriteAttributes",
                json::parse("{\"Name\":\"New name\"}").unwrap()
            ),
            Ok(())
        );
        {
            let mut attributes = Attributes::default();
            mqtt_nal_handler
                .cache
                .node_get(&key, &mut attributes)
                .unwrap();
            assert_eq!(attributes.name_desired, String::from("New name"));
            assert_eq!(attributes.name_reported, String::from("New name"));
            assert_eq!(attributes.is_proxied, true);
        }
    }
}
