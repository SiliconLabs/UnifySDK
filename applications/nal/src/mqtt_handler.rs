// NAL is "Name and Location"

use crate::cache::*;
use unify_log_sys::*;
use unify_mqtt_sys::{
    sl_status_t, MosqMessage, MqttClientCallbacksTrait, MqttClientTrait, TopicMatcherType,
    UnifyMqttClient,
};
use unify_validator_sys::*;

/// MqttNALHandler handles subscription messages related to Name and Location service.
/// It subscribes to listen to the correct topics and is to be passed to the mosquitto client.
pub struct MqttNALHandler<T: MqttClientTrait> {
    client: T,
    /// Lookup cache for nodes
    cache: Cache,
    /// Matches "ucl/by-unid/+/State" topic
    state_matcher: TopicMatcherType,
    /// Matches "ucl/by-unid/+/State/Attributes/EndpointIdList/Reported" topic
    endpoint_id_list_matcher: TopicMatcherType,
    /// Matches "ucl/by-unid/+/+/NameAndLocation/#" topic
    attributes_matcher: TopicMatcherType,
    /// Matches "ucl/by-unid/+/+/Basic/Attributes/LocationDescription/#" topic
    location_description_matcher: TopicMatcherType,
}

declare_app_name!("mqtt_handler");

impl<T: MqttClientTrait> MqttClientCallbacksTrait for MqttNALHandler<T> {
    /// public function to be called by the mosquitto fwk on a received message.
    fn on_message(&mut self, msg: MosqMessage) {
        if let Err(e) = self.process_message(msg) {
            if !e.is_empty() {
                log_debug!("Error: {}", e);
            }
        }
    }
    fn before_disconnect(&mut self) {
        self.service_stop_callback();
    }
    fn after_connect(&mut self) {
        // restore Name and Location information, stored in database
        let nodes_to_init: Vec<NodeEpKey> = self.cache.get_all_nodes().unwrap();

        for it in &nodes_to_init {
            let mut attributes = Attributes::default();
            let _ = self.cache.node_get(&it, &mut attributes);
            let _ = self.pub_attr_name_msgs(&it, &attributes, true, true);
            let _ = self.pub_attr_location_msgs(&it, &attributes, true, true);

            {
                let topic = format!(
                    "ucl/by-unid/{}/ep{}/NameAndLocation/SupportedCommands",
                    it.unid, it.ep_id
                );
                let payload = String::from("{\"value\":[\"WriteAttributes\"]}");
                let _ = self
                    .client
                    .publish(&topic.as_str(), &payload.as_str().as_bytes(), true);
            }
        }
    }
}

impl<T: MqttClientTrait> MqttNALHandler<T> {
    pub fn new(client: T, nal_config: NALConfig) -> Result<Self, sl_status_t> {
        let state = client.subscribe("ucl/by-unid/+/State")?;
        let endpoint_id_list =
            client.subscribe("ucl/by-unid/+/State/Attributes/EndpointIdList/Reported")?;
        let attributes = client.subscribe("ucl/by-unid/+/+/NameAndLocation/#")?;
        let location_description =
            client.subscribe("ucl/by-unid/+/+/Basic/Attributes/LocationDescription/#")?;

        Ok(MqttNALHandler {
            client: client,
            state_matcher: state,
            endpoint_id_list_matcher: endpoint_id_list,
            attributes_matcher: attributes,
            location_description_matcher: location_description,
            cache: Cache::new(nal_config),
        })
    }

    fn process_message(&mut self, msg: MosqMessage) -> std::result::Result<(), String> {
        let payload: &str = msg.text();

        // Now dispatch the payload/topic to the handler
        if self.state_matcher.matches(&msg) {
            self.on_state_reported(msg.topic(), payload)
        } else if self.endpoint_id_list_matcher.matches(&msg) {
            self.on_endpoint_id_list_reported(msg.topic(), payload)
        } else if self.attributes_matcher.matches(&msg) {
            self.on_attributes_reported(msg.topic(), payload)
        } else if self.location_description_matcher.matches(&msg) {
            self.on_location_description_reported(msg.topic(), payload)
        } else {
            Err(format!("no handler for subscribed topic {}", msg.topic()))
        }
    }

    /// # Description
    ///
    /// Implements a `"ucl/by-unid/+/State"` topic handler.
    /// Extracts UNID from topic.
    /// If payload is empty:
    /// searches for all nodes with such UNID in database and deletes them and unretain theirs publications to such topics:
    ///
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Reported`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Reported`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/SupportedCommands`
    ///
    /// If payload is **not** empty - checks is such UNID already registered
    /// if yes - does nothing;
    /// if **not** - registers this UNID and processes delayed messages for this UNID (see `process_delayed_messages_state()`)
    fn on_state_reported(&mut self, topic: &str, payload: &str) -> std::result::Result<(), String> {
        let unid = get_unid_from_topic(topic);
        if false == validate_unid(unid.as_str()) {
            return Err(String::from("Invalid UNID provided"));
        }
        // If payload of "ucl/by-unid/+/State" topic is empty - all nodes with matched unid should be deleted
        let is_delete: bool = payload.is_empty();

        if is_delete {
            self.remove_nodes_via_name(unid.as_str())?;
        } else if false == self.cache.is_unid_registered(unid.as_str()) {
            // If such UNID is already registered - do nothing.
            // If it doesn't - register it.
            self.cache.register_unid(unid.as_str())?;

            // check delayed messages for this UNID and processes, if any
            // please note that delayed messages will be processed only if EndpointIdList message already presents in delayed messages
            // if not - they will be processed in EndpointIdList topic handler
            self.process_delayed_messages_state(unid.as_str())?;
        }

        Ok(())
    }

    /// # Description
    ///
    /// Implements a `"ucl/by-unid/+/State/Attributes/EndpointIdList/Reported"` topic handler.
    /// Extracts UNID from topic.
    /// If such UNID isn't already registered (the State topic for such UNID hasn't been received yet):
    /// adds this message to `delayed_messages` table
    ///
    /// If it is already registered: (the State topic for such UNID has been already received):
    /// for each endpoint in message's payload adds a new node to database and  publish `{"value":""}` message to such topics:
    ///
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Reported`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Reported`
    ///
    /// and publish `"{"value":["WriteAttributes"]}"` message to this topic:
    ///
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/SupportedCommands`
    ///
    /// Then, checks delayed messages for this UNID and processes them, if any.
    fn on_endpoint_id_list_reported(
        &mut self,
        topic: &str,
        payload: &str,
    ) -> std::result::Result<(), String> {
        let unid = get_unid_from_topic(topic);
        if false == validate_unid(unid.as_str()) {
            return Err(String::from("Invalid UNID provided"));
        }

        // if such UNID isn't registered - the node can't be processed right now
        // but we have to save this message to delayed messages to process it later, if the State topic message for such UNID will be received
        // the priority is '0' because the EndpointIdList message should be processed before any Attribute message
        if false == self.cache.is_unid_registered(unid.as_str()) {
            self.cache
                .add_delayed_message(unid.as_str(), topic, payload, 0)?;
            return Err(String::from(""));
        }

        self.cache.register_received_epidlist(unid.as_str())?;

        // create node for all endpoints, reported by EndpointIdList
        let ids: serde_json::Value = serde_json::from_str(payload)
            .map_err(|e| format!("Could not parse payload for topic '{}' : {}", topic, e))?;

        let mut i = 0;

        while ids["value"][i].as_u64() != None {
            let key = NodeEpKey {
                unid: unid.clone(),
                ep_id: ids["value"][i].as_u64().unwrap() as u16,
            };

            // if node with such key already presents - do nothing to not owerwrite already existing Name and Location
            // otherwise create a node with empty Name and Location
            if false == self.cache.node_presents(&key) {
                let attributes = Attributes::default();
                self.node_create(&key, &attributes)?;
            }

            i += 1;
        }

        // check delayed messages for this UNID and process, if any
        self.process_delayed_messages_epidlist(unid.as_str())?;

        Ok(())
    }

    /// # Description
    ///
    /// Implements a `"ucl/by-unid/+/+/NameAndLocation/#"` topic handler.
    ///
    /// Extracts UNID from topic.
    /// Checks is such UNID already registered and is EndpointIdList for such UNID already received.
    /// If either condition is failed - adds received message to `delayed_messages` table and returns.
    ///
    /// It both conditions are met - searches for the UNID/endpoint_ID node in database.
    /// If **not**found - the provided endpoint_id is invalid - returns error.
    ///
    /// If topic ends with `"WriteAttributes"` - parses payload
    /// If payload is Json with key either "Name" or "Location" - update node in cache
    /// If payload has another format - do nothing
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
        payload: &str,
    ) -> std::result::Result<(), String> {
        let last_topic_chunk = get_last_topic_chunk(topic);
        if last_topic_chunk != "WriteAttributes" {
            // NAL has not idea how to process this message - so just skip it
            return Ok(());
        }

        let unid = get_unid_from_topic(topic);
        let ep_id = get_ep_id_from_topic(topic)?;

        if false == validate_unid(unid.as_str()) {
            return Err(String::from("Invalid UNID provided"));
        }

        // If such UNID hasn't been already registered - this message can't be processed right now
        // If EndpointIdList hasn't been already received - this message can't be processed right now
        // but we have to save it to delayed messages to process it later
        // The priority is '1' to be processed after EndpointIdList message
        if false == self.cache.is_unid_registered(unid.as_str())
            || false == self.cache.is_epidlist_received(unid.as_str())
        {
            self.cache
                .add_delayed_message(unid.as_str(), topic, payload, 1)?;
            return Err(String::from(""));
        }

        let key = NodeEpKey {
            unid: unid.clone(),
            ep_id: ep_id,
        };

        // if a node with such UNID/endpoint ID doesn't present in database - this message can't be processed
        if false == self.cache.node_presents(&key) {
            return Err(format!(
                "Can't process message: unknown endpoint '{}' for UNID '{}' detected",
                ep_id, unid
            ));
        }

        // convert payload to Json
        let json: serde_json::Value = serde_json::from_str(payload)
            .map_err(|e| format!("Could not parse payload for topic '{}' : {}", topic, e))?;

        if json["Name"] != serde_json::Value::Null {
            let new_name: &str = json["Name"].as_str().unwrap();

            if new_name.len() != 0 && false == validate_string_contains_letter_or_digit(new_name) {
                return Err(String::from("Invalid Name attribute value provided"));
            }

            let mut attributes = Attributes::default();
            self.cache.node_get(&key, &mut attributes)?;
            attributes.name_desired = new_name.to_string();
            attributes.name_reported = new_name.to_string();

            self.cache.node_set(&key, &attributes)?;

            self.pub_attr_name_msgs(&key, &attributes, true, true)?;
        }

        if json["Location"] != serde_json::Value::Null {
            let new_location: &str = json["Location"].as_str().unwrap();

            if new_location.len() != 0
                && false == validate_string_contains_letter_or_digit(new_location)
            {
                return Err(String::from("Invalid Location attribute value provided"));
            }

            let mut attributes = Attributes::default();
            self.cache.node_get(&key, &mut attributes)?;
            if attributes.is_proxied {
                // "proxying" feature implementation
                let topic = format!(
                    "ucl/by-unid/{}/ep{}/Basic/Commands/WriteAttributes",
                    key.unid, key.ep_id
                );
                let payload = serde_json::json!({
                    "LocationDescription": serde_json::Value::from(new_location)
                })
                .to_string();
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

        Ok(())
    }

    /// # Description
    ///
    /// Implements a `"ucl/by-unid/+/+/Basic/Attributes/LocationDescription/#"` topic handler.
    ///
    /// Extracts UNID from topic.
    /// Checks is such UNID already registered and is EndpointIdList for such UNID already received.
    /// If either condition is failed - adds received message to `delayed_messages` table and returns.
    ///
    /// It both conditions are met - searches for the UNID/endpoint_ID node in database.
    /// If **not**found - the provided endpoint_id is invalid - returns error.
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
    fn on_location_description_reported(
        &mut self,
        topic: &str,
        payload: &str,
    ) -> std::result::Result<(), String> {
        let penultimate_topic_chunk = get_penultimate_topic_chunk(topic);
        if penultimate_topic_chunk != "LocationDescription" {
            // NAL has not idea how to process this message - so just skip it
            return Ok(());
        }

        let last_topic_chunk = get_last_topic_chunk(topic);
        let unid = get_unid_from_topic(topic);
        let ep_id = get_ep_id_from_topic(topic)?;

        if false == validate_unid(unid.as_str()) {
            return Err(String::from("Invalid UNID provided"));
        }

        // If such UNID hasn't been already registered - this message can't be processed right now
        // If EndpointIdList hasn't been already received - this message can't be processed right now
        // but we have to save it to delayed messages to process it later
        // The priority is '1' to be processed after EndpointIdList message
        if false == self.cache.is_unid_registered(unid.as_str())
            || false == self.cache.is_epidlist_received(unid.as_str())
        {
            self.cache
                .add_delayed_message(unid.as_str(), topic, payload, 1)?;
            return Err(String::from(""));
        }

        let key = NodeEpKey {
            unid: unid.clone(),
            ep_id: ep_id,
        };

        // if a node with such UNID/endpoint ID doesn't present in database - this message can't be processed
        if false == self.cache.node_presents(&key) {
            return Err(format!(
                "Can't process message: unknown endpoint '{}' for UNID '{}' detected",
                ep_id, unid
            ));
        }

        // convert payload to Json
        let json: serde_json::Value = serde_json::from_str(payload)
            .map_err(|e| format!("Could not parse payload for topic '{}' : {}", topic, e))?;

        if json["value"] != serde_json::Value::Null {
            // "proxying" feature implementation
            let new_location: &str = json["value"].as_str().unwrap();

            if new_location.len() != 0
                && false == validate_string_contains_letter_or_digit(new_location)
            {
                return Err(String::from("Invalid Location attribute value provided"));
            }

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

    /// # Description
    ///
    /// Removes all nodes with provided UNID from database.
    /// Removes this UNID from 'unids' table.
    fn remove_nodes_via_name(&mut self, unid: &str) -> std::result::Result<(), String> {
        let nodes_to_remove: Vec<NodeEpKey> = self.cache.get_all_nodes_match_name(unid).unwrap();

        for it in &nodes_to_remove {
            self.node_delete(&it)?;
        }

        self.cache.remove_unid(unid)?;
        Ok(())
    }

    /// # Description
    ///
    /// Adds a new key/value pair to the database with a given node_key/attributes values.
    /// Publishes Name and Location values to such topics (as retained messages):
    ///
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Reported`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Reported`
    ///
    /// Publishes the `"{"value":["WriteAttributes"]}"` message to this topic:
    ///
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

    /// # Description
    ///
    /// Deletes node with a given UNID/endpoint_id key from the database.
    /// Unretain all publications, related to this node (see `unretain_all_node_messages()`)
    fn node_delete(&mut self, key: &NodeEpKey) -> std::result::Result<(), String> {
        self.cache.node_remove(key)?;

        // unretain all publications
        self.unretain_all_node_messages(key)?;

        Ok(())
    }

    /// # Description
    ///
    /// Deletes node with a given UNID/endpoint_id key from the database.
    /// Unretain all publications to such topics (by publishing an empty message):
    ///
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Name/Reported`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Desired`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Attributes/Location/Reported`
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/SupportedCommands`
    ///
    /// /// Cancels subscription tu such topic:
    /// * `ucl/by-unid/<UNID>/ep<endpoint_id>/NameAndLocation/Commands/WriteAttributes`
    fn unretain_all_node_messages(&mut self, key: &NodeEpKey) -> std::result::Result<(), String> {
        let mqtt_client = UnifyMqttClient::default();

        {
            let topic = format!(
                "ucl/by-unid/{}/ep{}/NameAndLocation/Commands/WriteAttributes",
                key.unid, key.ep_id
            );
            let _ = mqtt_client.unsubscribe(&topic)
                .map_err(|e| format!("error unsubscribing from topic '{}' : {}", topic, e));
        }
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
        {
            let topic = format!(
                "ucl/by-unid/{}/ep{}/NameAndLocation/Commands/WriteAttributes",
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
    /// The payload is `name_desired` and `name_reported` attribute values, respectively.
    ///
    /// # Arguments
    ///
    /// * _key_        - UNID and endpoint_ID to construct a topic are given from here
    /// * _attributes_ - messages' payload will be given from here
    /// * _pub_des_    - if true - publishes the "Desired"  topic
    /// * _pub_rep_    - if true - publishes the "Reported" topic
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
    /// The payload is `location_desired` and `location_reported` attribute values, respectively.
    ///
    /// # Arguments
    ///
    /// * _key_        - UNID and endpoint_ID to construct a topic are given from here
    /// * _attributes_ - messages' payload will be given from here
    /// * _pub_des_    - if true - publishes the "Desired"  topic
    /// * _pub_rep_    - if true - publishes the "Reported" topic
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

    /// # Description
    ///
    /// This function should be called from State topic handler only!
    ///
    /// Gets all delayed messages for provided UNID from `delayed_messages` table.
    /// Rows in a table are sorted by their `priority` value in ascendent order.
    /// The first message should be EndpointIdList message vith priority `0`.
    /// If it is - processes all messages.
    /// Calls either `on_endpoint_id_list_reported()` or `on_attributes_reported()` function with topic and payload got from the delayed message.
    ///
    /// If it isn't - does nothing.
    ///
    /// # Arguments
    ///
    /// * _unid_ - messages for this UNID will be processed
    fn process_delayed_messages_state(&mut self, unid: &str) -> std::result::Result<(), String> {
        let msgs_to_exec: Vec<(String, String)> =
            self.cache.get_all_delayed_messages(unid).unwrap();

        if msgs_to_exec.len() == 0 {
            return Ok(());
        }

        // now we have to determine is the first delayed message the 'EndpointIdList' message
        // if yes - process all messages
        // if not - do nothing, all delayed messages will be processed in EndpointIdList topic handler, after receiving of EndpointIdList message
        if get_penultimate_topic_chunk(msgs_to_exec[0].0.as_str()) != "EndpointIdList" {
            return Ok(());
        }

        // delete all messages to avoid infinite recursion calls
        self.cache.delete_all_delayed_messages(unid)?;

        for it in &msgs_to_exec {
            let topic = it.0.clone();
            let payload = it.1.clone();

            if get_penultimate_topic_chunk(it.0.as_str()) == "EndpointIdList" {
                self.on_endpoint_id_list_reported(topic.as_str(), payload.as_str())?;
            } else {
                self.on_attributes_reported(topic.as_str(), payload.as_str())?;
            }
        }

        Ok(())
    }

    /// # Description
    ///
    /// This function should be called from EndpointIdList topic handler only!
    ///
    /// Gets all delayed messages for provided UNID from `delayed_messages` table and processes them, if any.
    /// Calls `on_attributes_reported()` function with topic and payload got from the delayed message.
    ///
    /// # Arguments
    ///
    /// * _unid_ - messages for this UNID will be processed
    fn process_delayed_messages_epidlist(&mut self, unid: &str) -> std::result::Result<(), String> {
        let msgs_to_exec: Vec<(String, String)> =
            self.cache.get_all_delayed_messages(unid).unwrap();

        if msgs_to_exec.len() == 0 {
            return Ok(());
        }

        for it in &msgs_to_exec {
            let topic = it.0.clone();
            let payload = it.1.clone();

            self.on_attributes_reported(topic.as_str(), payload.as_str())?;
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

/// # Description
///
/// Parses topic and extracts it's penultimate chunk.
/// I.e. for topic `ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Reported`
/// the "Location" string will be returned.
///
/// If topic is an empty string - returns an empty string.
fn get_penultimate_topic_chunk(topic: &str) -> String {
    if topic.len() == 0 {
        return String::from("");
    }

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

/// # Description
///
/// Parses topic and extracts it's last chunk.
/// I.e. for topic `ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Reported`
/// the "Reported" string will be returned.
fn get_last_topic_chunk(topic: &str) -> String {
    let list = topic.split_terminator('/').collect::<Vec<&str>>();
    let last_chunk = list.last().unwrap();
    last_chunk.to_string()
}

/// # Description
///
/// Parses topic and extracts unid.
/// I.e. for topic `ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Reported`
/// the "node_1" string will be returned.
fn get_unid_from_topic(topic: &str) -> String {
    let list = topic.split_terminator('/').collect::<Vec<&str>>();
    let unid = list[2];
    unid.to_string()
}

/// # Description
///
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
    use unify_mqtt_sys::{MockMqttClientTrait, MockTopicMatcherTrait};

    fn create_mocked_mqtt_handler() -> MqttNALHandler<MockMqttClientTrait> {
        let mut mqtt_client_mock = MockMqttClientTrait::new();
        let state_topic_matcher_mock: Result<TopicMatcherType, sl_status_t> =
            Ok(Box::new(MockTopicMatcherTrait::new()));
        mqtt_client_mock
            .expect_subscribe()
            .with(eq("ucl/by-unid/+/State"))
            .return_once_st(|_| state_topic_matcher_mock);

        let endpoint_id_list_topic_matcher_mock: Result<TopicMatcherType, sl_status_t> =
            Ok(Box::new(MockTopicMatcherTrait::new()));
        mqtt_client_mock
            .expect_subscribe()
            .with(eq("ucl/by-unid/+/State/Attributes/EndpointIdList/Reported"))
            .return_once_st(|_| endpoint_id_list_topic_matcher_mock);

        let attributes_topic_matcher_mock: Result<TopicMatcherType, sl_status_t> =
            Ok(Box::new(MockTopicMatcherTrait::new()));
        mqtt_client_mock
            .expect_subscribe()
            .with(eq("ucl/by-unid/+/+/NameAndLocation/#"))
            .return_once_st(|_| attributes_topic_matcher_mock);

        let location_description_topic_matcher_mock: Result<TopicMatcherType, sl_status_t> =
            Ok(Box::new(MockTopicMatcherTrait::new()));
        mqtt_client_mock
            .expect_subscribe()
            .with(eq("ucl/by-unid/+/+/Basic/Attributes/LocationDescription/#"))
            .return_once_st(|_| location_description_topic_matcher_mock);

        MqttNALHandler::new(mqtt_client_mock, NALConfig::new_for_unit_tests()).unwrap()
    }

    #[test]
    fn test_get_penultimate_topic_chunk() {
        {
            let test_topic = "ucl/by-unid/some_unid/State";
            assert_eq!(
                get_penultimate_topic_chunk(test_topic),
                String::from("some_unid")
            );
        }
        {
            let test_topic = "";
            assert_eq!(get_penultimate_topic_chunk(test_topic), String::from(""));
        }
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
        {
            let test_topic = "ucl/by-unid/unid/ep5/NameAndLocation/Commands/WriteAttributes";
            let res: u16 = get_ep_id_from_topic(test_topic).unwrap();
            assert_eq!(res, 5);
        }
        {
            let test_topic =
                "ucl/by-unid/unid/invalid_ep_id/NameAndLocation/Commands/WriteAttributes";
            assert_eq!(get_ep_id_from_topic(test_topic), Err(String::from("")));
        }
        {
            let test_topic =
                "ucl/by-unid/unid/epinvaliddecimanvalue/NameAndLocation/Commands/WriteAttributes";
            assert_eq!(
                get_ep_id_from_topic(test_topic),
                Err(String::from(
                    "could not parse invaliddecimanvalue to integer. invalid digit found in string"
                ))
            );
        }
    }

    #[test]
    fn test_validate_unid() {
        assert_eq!(validate_unid("zw-EE7F73CF-0001"), true);
        assert_eq!(validate_unid("z"), true);
        assert_eq!(validate_unid("Z"), true);
        assert_eq!(validate_unid("2"), true);
        assert_eq!(validate_unid("zw-EE7F 73CF-0001"), false);
        assert_eq!(validate_unid("zw-EE7F/73CF-0001"), false);
        assert_eq!(validate_unid(r"zw-EE7F\73CF-0001"), false);
        assert_eq!(validate_unid(""), false);
        assert_eq!(validate_unid("\""), false);
        assert_eq!(validate_unid("\"\"\""), false);
        assert_eq!(validate_unid("\'"), false);
        assert_eq!(validate_unid("\'\'\'"), false);
    }

    #[test]
    fn test_validate_name_or_location() {
        assert_eq!(validate_string_contains_letter_or_digit(""), false);
        assert_eq!(validate_string_contains_letter_or_digit("new_loc"), true);
        assert_eq!(validate_string_contains_letter_or_digit("new-loc"), true);
        assert_eq!(validate_string_contains_letter_or_digit("new loc"), true);
        assert_eq!(validate_string_contains_letter_or_digit("1"), true);
        assert_eq!(validate_string_contains_letter_or_digit("_"), false);
        assert_eq!(validate_string_contains_letter_or_digit("---"), false);
        assert_eq!(validate_string_contains_letter_or_digit("---"), false);
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
                x == serde_json::json! ({"value" : serde_json::Value::from("")})
                    .to_string()
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
                x == serde_json::json!({ "value": serde_json::Value::from($value) })
                    .to_string()
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

        let state_topic = "ucl/by-unid/test_node_1/State";
        let state_payload = "some_dummy_data";
        let ep_id_list_topic = "ucl/by-unid/test_node_1/State/Attributes/EndpointIdList/Reported";
        let ep_id_list_payload = r#"{"value": [1, 2]}"#;

        mqtt_nal_handler
            .on_state_reported(state_topic, state_payload)
            .unwrap();

        // test adding node
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
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep2/NameAndLocation/Attributes/Name/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep2/NameAndLocation/Attributes/Name/Reported"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep2/NameAndLocation/Attributes/Location/Desired"
        );
        EXPECT_PUBLISH_VALUE_EMPTY!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep2/NameAndLocation/Attributes/Location/Reported"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep2/NameAndLocation/SupportedCommands",
            vec!["WriteAttributes"]
        );

        mqtt_nal_handler
            .on_endpoint_id_list_reported(ep_id_list_topic, ep_id_list_payload)
            .unwrap();

        let key_presents_1 = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 1,
        };
        let key_presents_2 = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 2,
        };
        let key_not_pres_1 = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 0,
        };
        let key_not_pres_2 = NodeEpKey {
            unid: String::from("wrong_node"),
            ep_id: 1,
        };

        assert_eq!(
            mqtt_nal_handler.cache.is_unid_registered("test_node_1"),
            true
        );
        assert_eq!(mqtt_nal_handler.cache.node_presents(&key_presents_1), true);
        assert_eq!(mqtt_nal_handler.cache.node_presents(&key_presents_2), true);
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
        assert_eq!(mqtt_nal_handler.cache.register_unid("test_node_1"), Ok(()));

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
        assert_eq!(
            mqtt_nal_handler.cache.is_unid_registered("test_node_1"),
            false
        );
    }

    #[test]
    #[serial]
    fn test_write_attributes() {
        let mut mqtt_nal_handler = create_mocked_mqtt_handler();
        mqtt_nal_handler.cache.clean_up_db_for_unit_tests();

        let attributes_topic =
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Commands/WriteAttributes";
        let attributes_payload_name = r#"{"Name": "New name"}"#;
        let attributes_payload_location = r#"{"Location": "New location"}"#;

        let key = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 0,
        };

        assert_eq!(mqtt_nal_handler.cache.register_unid("test_node_1"), Ok(()));
        assert_eq!(
            mqtt_nal_handler
                .cache
                .register_received_epidlist("test_node_1"),
            Ok(())
        );
        {
            let attributes = Attributes::default();
            assert_eq!(mqtt_nal_handler.cache.node_set(&key, &attributes), Ok(()));
        }

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
            mqtt_nal_handler.on_attributes_reported(attributes_topic, attributes_payload_name),
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
            mqtt_nal_handler.on_attributes_reported(attributes_topic, attributes_payload_location),
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

        let attributes_topic =
            "ucl/by-unid/test_node_1/ep1/NameAndLocation/Commands/WriteAttributes";
        let attributes_payload = r#"{"value":"some_dummy_data_to_detect_endpoint"}"#;

        // add node "test_node_1", endpoint 1
        // node with UNID "test_node_1" doesn't exist (there wasn't pub to "ucl/by-unid/test_node_1/State" topic)
        // so the endpoint 1 node should not be added
        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(attributes_topic, attributes_payload),
            Err(String::from(""))
        );
    }

    #[test]
    #[serial]
    fn test_topics_not_associated_with_nal() {
        let mut mqtt_nal_handler = create_mocked_mqtt_handler();
        mqtt_nal_handler.cache.clean_up_db_for_unit_tests();

        let topic = "ucl/by-unid/test_node_1/ep1/Basic/Attributes/PowerSource/Reported";
        let payload_1 = r#"{"value":"some_dummy_data_to_detect_endpoint"}"#;
        let payload_2 = r#"{"value":""}"#;
        let payload_3 = "";

        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(topic, payload_1),
            Ok(())
        );
        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(topic, payload_2),
            Ok(())
        );
        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(topic, payload_3),
            Ok(())
        );
        assert_eq!(
            mqtt_nal_handler.on_location_description_reported(topic, payload_1),
            Ok(())
        );
        assert_eq!(
            mqtt_nal_handler.on_location_description_reported(topic, payload_2),
            Ok(())
        );
        assert_eq!(
            mqtt_nal_handler.on_location_description_reported(topic, payload_3),
            Ok(())
        );
    }

    #[test]
    #[serial]
    fn test_proxying() {
        let mut mqtt_nal_handler = create_mocked_mqtt_handler();
        mqtt_nal_handler.cache.clean_up_db_for_unit_tests();

        let state_topic = "ucl/by-unid/test_node_1/State";
        let ep_id_list_topic = "ucl/by-unid/test_node_1/State/Attributes/EndpointIdList/Reported";
        let ep_id_list_payload = r#"{"value": [0]}"#;
        let attributes_topic =
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Commands/WriteAttributes";
        let attributes_location_description_desired_topic =
            "ucl/by-unid/test_node_1/ep0/Basic/Attributes/LocationDescription/Desired";
        let attributes_location_description_reported_topic =
            "ucl/by-unid/test_node_1/ep0/Basic/Attributes/LocationDescription/Reported";

        let key = NodeEpKey {
            unid: String::from("test_node_1"),
            ep_id: 0,
        };

        mqtt_nal_handler
            .on_state_reported(state_topic, "some_dummy_data")
            .unwrap();

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

        assert_eq!(
            mqtt_nal_handler.on_endpoint_id_list_reported(ep_id_list_topic, ep_id_list_payload),
            Ok(())
        );

        // Protocol controller takes control of node
        // test Desired location
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Desired",
            "New location"
        );

        assert_eq!(
            mqtt_nal_handler.on_location_description_reported(
                attributes_location_description_desired_topic,
                r#"{"value":"New location"}"#
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
            mqtt_nal_handler.on_location_description_reported(
                attributes_location_description_reported_topic,
                r#"{"value":"New location"}"#
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
                x == serde_json::json! ({"LocationDescription" : serde_json::Value::from("Updated location")}).to_string().as_bytes()
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
            mqtt_nal_handler
                .on_attributes_reported(attributes_topic, r#"{"Location":"Updated location"}"#),
            Ok(())
        );

        // simulate that the Protocol controller received message and made a responce:
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node_1/ep0/NameAndLocation/Attributes/Location/Desired",
            "Updated location"
        );

        assert_eq!(
            mqtt_nal_handler.on_location_description_reported(
                attributes_location_description_desired_topic,
                r#"{"value":"Updated location"}"#
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
            mqtt_nal_handler.on_location_description_reported(
                attributes_location_description_reported_topic,
                r#"{"value":"Updated location"}"#
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
            mqtt_nal_handler.on_attributes_reported(attributes_topic, r#"{"Name":"New name"}"#),
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

    #[test]
    #[serial]
    fn test_topics_sequence_state_epidlist_attributes() {
        let mut mqtt_nal_handler = create_mocked_mqtt_handler();
        mqtt_nal_handler.cache.clean_up_db_for_unit_tests();

        let state_topic = "ucl/by-unid/test_node/State";
        let ep_id_list_topic = "ucl/by-unid/test_node/State/Attributes/EndpointIdList/Reported";
        let ep_id_list_payload = r#"{"value": [1]}"#;
        let attributes_topic = "ucl/by-unid/test_node/ep1/NameAndLocation/Commands/WriteAttributes";
        let attributes_payload = r#"{"Name": "test_name"}"#;

        let key = NodeEpKey {
            unid: String::from("test_node"),
            ep_id: 1,
        };

        // State topic
        assert_eq!(
            mqtt_nal_handler.on_state_reported(state_topic, "dummy_payload"),
            Ok(())
        );

        assert_eq!(mqtt_nal_handler.cache.is_unid_registered("test_node"), true);
        assert_eq!(
            mqtt_nal_handler.cache.is_epidlist_received("test_node"),
            false
        );

        // EndpointIdList topic
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
            ""
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep1/NameAndLocation/Attributes/Name/Reported",
            ""
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

        assert_eq!(
            mqtt_nal_handler.on_endpoint_id_list_reported(ep_id_list_topic, ep_id_list_payload),
            Ok(())
        );

        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node", 1, true);
        assert_eq!(
            mqtt_nal_handler.cache.is_epidlist_received("test_node"),
            true
        );

        // Attributes topic (Name)
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep1/NameAndLocation/Attributes/Name/Desired",
            "test_name"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep1/NameAndLocation/Attributes/Name/Reported",
            "test_name"
        );

        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(attributes_topic, attributes_payload),
            Ok(())
        );

        {
            let mut attributes = Attributes::default();
            mqtt_nal_handler
                .cache
                .node_get(&key, &mut attributes)
                .unwrap();
            assert_eq!(attributes.name_desired, String::from("test_name"));
            assert_eq!(attributes.name_reported, String::from("test_name"));
        }
    }

    #[test]
    #[serial]
    fn test_topics_sequence_state_attributes_epidlist() {
        let mut mqtt_nal_handler = create_mocked_mqtt_handler();
        mqtt_nal_handler.cache.clean_up_db_for_unit_tests();

        let state_topic = "ucl/by-unid/test_node/State";
        let ep_id_list_topic = "ucl/by-unid/test_node/State/Attributes/EndpointIdList/Reported";
        let ep_id_list_payload = r#"{"value": [1]}"#;
        let attributes_topic = "ucl/by-unid/test_node/ep1/NameAndLocation/Commands/WriteAttributes";
        let attributes_payload = r#"{"Name": "test_name"}"#;

        let key = NodeEpKey {
            unid: String::from("test_node"),
            ep_id: 1,
        };

        // State topic
        assert_eq!(
            mqtt_nal_handler.on_state_reported(state_topic, "dummy_payload"),
            Ok(())
        );

        assert_eq!(mqtt_nal_handler.cache.is_unid_registered("test_node"), true);

        // Attributes topic (Name) (should go to delayed messages)
        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(attributes_topic, attributes_payload),
            Err(String::from(""))
        );

        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node", 1, false);

        // EndpointIdList topic
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
            "test_name"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep1/NameAndLocation/Attributes/Name/Reported",
            "test_name"
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

        assert_eq!(
            mqtt_nal_handler.on_endpoint_id_list_reported(ep_id_list_topic, ep_id_list_payload),
            Ok(())
        );

        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node", 1, true);
        assert_eq!(
            mqtt_nal_handler.cache.is_epidlist_received("test_node"),
            true
        );

        {
            let mut attributes = Attributes::default();
            mqtt_nal_handler
                .cache
                .node_get(&key, &mut attributes)
                .unwrap();
            assert_eq!(attributes.name_desired, String::from("test_name"));
            assert_eq!(attributes.name_reported, String::from("test_name"));
        }
    }

    #[test]
    #[serial]
    fn test_topics_sequence_epidlist_state_attributes() {
        let mut mqtt_nal_handler = create_mocked_mqtt_handler();
        mqtt_nal_handler.cache.clean_up_db_for_unit_tests();

        let state_topic = "ucl/by-unid/test_node/State";
        let ep_id_list_topic = "ucl/by-unid/test_node/State/Attributes/EndpointIdList/Reported";
        let ep_id_list_payload = r#"{"value": [1]}"#;
        let attributes_topic = "ucl/by-unid/test_node/ep1/NameAndLocation/Commands/WriteAttributes";
        let attributes_payload = r#"{"Name": "test_name"}"#;

        let key = NodeEpKey {
            unid: String::from("test_node"),
            ep_id: 1,
        };

        // EndpointIdList topic (should go to delayed messages)
        assert_eq!(
            mqtt_nal_handler.on_endpoint_id_list_reported(ep_id_list_topic, ep_id_list_payload),
            Err(String::from(""))
        );

        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node", 1, false);
        assert_eq!(
            mqtt_nal_handler.cache.is_unid_registered("test_node"),
            false
        );

        // State topic
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

        assert_eq!(
            mqtt_nal_handler.on_state_reported(state_topic, "dummy_payload"),
            Ok(())
        );

        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node", 1, true);
        assert_eq!(mqtt_nal_handler.cache.is_unid_registered("test_node"), true);
        assert_eq!(
            mqtt_nal_handler.cache.is_epidlist_received("test_node"),
            true
        );

        // Attributes topic (Name)
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep1/NameAndLocation/Attributes/Name/Desired",
            "test_name"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep1/NameAndLocation/Attributes/Name/Reported",
            "test_name"
        );

        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(attributes_topic, attributes_payload),
            Ok(())
        );

        {
            let mut attributes = Attributes::default();
            mqtt_nal_handler
                .cache
                .node_get(&key, &mut attributes)
                .unwrap();
            assert_eq!(attributes.name_desired, String::from("test_name"));
            assert_eq!(attributes.name_reported, String::from("test_name"));
        }
    }

    #[test]
    #[serial]
    fn test_topics_sequence_epidlist_attributes_state() {
        let mut mqtt_nal_handler = create_mocked_mqtt_handler();
        mqtt_nal_handler.cache.clean_up_db_for_unit_tests();

        let state_topic = "ucl/by-unid/test_node/State";
        let ep_id_list_topic = "ucl/by-unid/test_node/State/Attributes/EndpointIdList/Reported";
        let ep_id_list_payload = r#"{"value": [1]}"#;
        let attributes_topic = "ucl/by-unid/test_node/ep1/NameAndLocation/Commands/WriteAttributes";
        let attributes_payload = r#"{"Name": "test_name"}"#;

        let key = NodeEpKey {
            unid: String::from("test_node"),
            ep_id: 1,
        };

        // EndpointIdList topic (should go to delayed messages)
        assert_eq!(
            mqtt_nal_handler.on_endpoint_id_list_reported(ep_id_list_topic, ep_id_list_payload),
            Err(String::from(""))
        );

        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node", 1, false);
        assert_eq!(
            mqtt_nal_handler.cache.is_unid_registered("test_node"),
            false
        );

        // Attributes topic (Name)
        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(attributes_topic, attributes_payload),
            Err(String::from(""))
        );

        // State topic
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
            "test_name"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep1/NameAndLocation/Attributes/Name/Reported",
            "test_name"
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

        assert_eq!(
            mqtt_nal_handler.on_state_reported(state_topic, "dummy_payload"),
            Ok(())
        );

        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node", 1, true);
        assert_eq!(mqtt_nal_handler.cache.is_unid_registered("test_node"), true);
        assert_eq!(
            mqtt_nal_handler.cache.is_epidlist_received("test_node"),
            true
        );

        {
            let mut attributes = Attributes::default();
            mqtt_nal_handler
                .cache
                .node_get(&key, &mut attributes)
                .unwrap();
            assert_eq!(attributes.name_desired, String::from("test_name"));
            assert_eq!(attributes.name_reported, String::from("test_name"));
        }
    }

    #[test]
    #[serial]
    fn test_topics_sequence_attributes_state_epidlist() {
        let mut mqtt_nal_handler = create_mocked_mqtt_handler();
        mqtt_nal_handler.cache.clean_up_db_for_unit_tests();

        let state_topic = "ucl/by-unid/test_node/State";
        let ep_id_list_topic = "ucl/by-unid/test_node/State/Attributes/EndpointIdList/Reported";
        let ep_id_list_payload = r#"{"value": [1]}"#;
        let attributes_topic = "ucl/by-unid/test_node/ep1/NameAndLocation/Commands/WriteAttributes";
        let attributes_payload = r#"{"Name": "test_name"}"#;

        let key = NodeEpKey {
            unid: String::from("test_node"),
            ep_id: 1,
        };

        // Attributes topic (Name)
        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(attributes_topic, attributes_payload),
            Err(String::from(""))
        );

        // State topic
        assert_eq!(
            mqtt_nal_handler.on_state_reported(state_topic, "dummy_payload"),
            Ok(())
        );

        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node", 1, false);
        assert_eq!(mqtt_nal_handler.cache.is_unid_registered("test_node"), true);
        assert_eq!(
            mqtt_nal_handler.cache.is_epidlist_received("test_node"),
            false
        );

        // EndpointIdList topic
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
            "test_name"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep1/NameAndLocation/Attributes/Name/Reported",
            "test_name"
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

        assert_eq!(
            mqtt_nal_handler.on_endpoint_id_list_reported(ep_id_list_topic, ep_id_list_payload),
            Ok(())
        );

        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node", 1, true);
        assert_eq!(mqtt_nal_handler.cache.is_unid_registered("test_node"), true);
        assert_eq!(
            mqtt_nal_handler.cache.is_epidlist_received("test_node"),
            true
        );

        {
            let mut attributes = Attributes::default();
            mqtt_nal_handler
                .cache
                .node_get(&key, &mut attributes)
                .unwrap();
            assert_eq!(attributes.name_desired, String::from("test_name"));
            assert_eq!(attributes.name_reported, String::from("test_name"));
        }
    }

    #[test]
    #[serial]
    fn test_topics_sequence_attributes_epidlist_state() {
        let mut mqtt_nal_handler = create_mocked_mqtt_handler();
        mqtt_nal_handler.cache.clean_up_db_for_unit_tests();

        let state_topic = "ucl/by-unid/test_node/State";
        let ep_id_list_topic = "ucl/by-unid/test_node/State/Attributes/EndpointIdList/Reported";
        let ep_id_list_payload = r#"{"value": [1]}"#;
        let attributes_topic = "ucl/by-unid/test_node/ep1/NameAndLocation/Commands/WriteAttributes";
        let attributes_payload = r#"{"Name": "test_name"}"#;

        let key = NodeEpKey {
            unid: String::from("test_node"),
            ep_id: 1,
        };

        // Attributes topic (Name)
        assert_eq!(
            mqtt_nal_handler.on_attributes_reported(attributes_topic, attributes_payload),
            Err(String::from(""))
        );

        // EndpointIdList topic
        assert_eq!(
            mqtt_nal_handler.on_endpoint_id_list_reported(ep_id_list_topic, ep_id_list_payload),
            Err(String::from(""))
        );

        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node", 1, false);
        assert_eq!(
            mqtt_nal_handler.cache.is_unid_registered("test_node"),
            false
        );
        assert_eq!(
            mqtt_nal_handler.cache.is_epidlist_received("test_node"),
            false
        );

        // State topic
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
            "test_name"
        );
        EXPECT_PUBLISH_VALUE!(
            mqtt_nal_handler,
            "ucl/by-unid/test_node/ep1/NameAndLocation/Attributes/Name/Reported",
            "test_name"
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

        assert_eq!(
            mqtt_nal_handler.on_state_reported(state_topic, "dummy_payload"),
            Ok(())
        );

        CHECK_NODE_PRESENCE_IN_CACHE!(mqtt_nal_handler, "test_node", 1, true);
        assert_eq!(mqtt_nal_handler.cache.is_unid_registered("test_node"), true);
        assert_eq!(
            mqtt_nal_handler.cache.is_epidlist_received("test_node"),
            true
        );

        {
            let mut attributes = Attributes::default();
            mqtt_nal_handler
                .cache
                .node_get(&key, &mut attributes)
                .unwrap();
            assert_eq!(attributes.name_desired, String::from("test_name"));
            assert_eq!(attributes.name_reported, String::from("test_name"));
        }
    }
}
