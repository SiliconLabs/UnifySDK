use crate::upvl_db;
use crate::upvl_json;
use unify_log_sys::*;
use unify_middleware::unify_mqtt_client::{
    sl_status_t, MosqMessage, MqttClientCallbacksTrait, MqttClientTrait, TopicMatcherType,
};
use upvl_db::*;

/// MqttHandler handles subscription messages related to UIC groups.
/// It subscribes to listen to the correct topics and is to be passed to the mosquitto client
pub struct MqttHandler<T: MqttClientTrait> {
    client: T,
    db_conn: rusqlite::Connection,
    // matchers that are used to scan incoming mqtt messages
    update_subscriber: TopicMatcherType,
    remove_subscriber: TopicMatcherType,
}

declare_app_name!("upvl_handler"); // a callback function which will be called when mosquitto client receives
impl<T: MqttClientTrait> MqttClientCallbacksTrait for MqttHandler<T> {
    // message on MQTT interface and disconnect.
    fn on_message(&mut self, msg: MosqMessage) {
        if let Err(e) = self.process_received_message(msg) {
            log_error!("{}", e);
        }
    }
    fn before_disconnect(&mut self) {}
    fn after_connect(&mut self) {}
}

impl<T: MqttClientTrait> MqttHandler<T> {
    pub fn new(client: T, upvl_config: UpvlConfig) -> Result<Self, sl_status_t> {
        let update_sub = client.subscribe("ucl/SmartStart/List/Update/#").unwrap();
        let remove_sub = client.subscribe("ucl/SmartStart/List/Remove/#").unwrap();
        // Connect to the data store.
        // This currently lives outside the mqtt module, since we may also
        // want to store other data than provisions in the data store,
        // e.g., configuration settings.
        let db_conn = upvl_db::db_setup(upvl_config);

        // Debug printout
        // of the provisions found in the data store.
        // Useful for the desk tests
        upvl_db::db_debug_print_provisions(&db_conn);
        let mut handler = MqttHandler {
            client: client,
            db_conn: db_conn,
            update_subscriber: update_sub,
            remove_subscriber: remove_sub,
        };
        handler.publish_list();
        Ok(handler)
    }

    fn process_received_message(&mut self, msg: MosqMessage) -> std::result::Result<(), String> {
        if msg.text().is_empty() {
            // Ignore unretain message
            log_debug!("Unretain of {}", msg.topic());
            Ok(())
        } else {
            let json: serde_json::Value = serde_json::from_str(msg.text())
                .map_err(|e| format!("could not parse body of topic '{}' : {}", msg.topic(), e))?;

            if self.update_subscriber.matches(&msg) {
                self.on_update_subscriber(json)
            } else if self.remove_subscriber.matches(&msg) {
                self.on_remove_subscriber(json)
            } else {
                Err(format!("no handler for subscribed topic {}", msg.topic()))
            }
        }
    }

    /// process a ucl/SmartStart/List/Update/# topic
    /// update SmartSatartList
    fn on_update_subscriber(&mut self, json: serde_json::Value) -> std::result::Result<(), String> {
        // Return type of from_str is Result<UpvlUpdate,_> here.
        // We pass on the error to the outer context with ?
        let provision: upvl_json::SmartStartEntry = upvl_json::SmartStartEntry::from(json);

        // Update the data store from the received message/topic.
        // TODO: put this in a business logic module?
        //
        // TODO: If we only want to publish a List
        // if there are changes in the database,
        // upsert() should return whether the
        // provision was a duplicate.
        upvl_db::db_upsert_entry(&self.db_conn, provision);
        self.publish_list();
        Ok(())
    }

    /// process a ucl/SmartStart/List/Remove/# topic
    /// remove SmartSatartList entry
    fn on_remove_subscriber(&mut self, json: serde_json::Value) -> std::result::Result<(), String> {
        // if the payload is legal json, but not a UpvlDSK, it
        // is also an Err.
        let provision: upvl_json::SmartStartEntry = upvl_json::SmartStartEntry::from(json);

        // Update the data store from the received message.
        // TODO: put this in a business logic module?
        //
        // Currently, the interface uses the upvl_json types.
        if upvl_db::db_remove_entry(&self.db_conn, &provision) == 1 {
            // Return true to say: Publish an updated List
            self.publish_list();
        } else {
            log_error!("Cannot update db");
        }
        Ok(())
    }

    pub fn publish_list(&mut self) {
        // Create an updated list and hand over the
        // list to publish_list
        let list = upvl_db::db_list_provisions(&self.db_conn);
        let mut json_object = serde_json::Map::new();
        json_object.insert("value".to_string(), serde_json::Value::Array(list));
        let payload = serde_json::to_string(&json_object)
            .expect("Cannot build JSON from this structure.")
            .into_bytes();
        let topic = "ucl/SmartStart/List";
        self.client
            .publish(&topic, &payload, true)
            .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))
            .unwrap();
    }
}

#[cfg(test)]
mod tests {
    #[test]
    fn test_subscriptions() {
        // let mut mqtt_client = MockMqttClient::new();
        // mqtt_client.expect_subscribe().returning(|_, _| Err(_));
        // let handler = MqttHandler::new(mqtt_client);
    }
}
