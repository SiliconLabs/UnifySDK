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
use crate::upvl_db;
use crate::upvl_json;
use unify_application_monitoring_sys::unify_application_monitoring_init;
use unify_log_sys::*;
use unify_mqtt_sys::{
    sl_status_t, MosqMessage, MqttClientCallbacksTrait, MqttClientTrait, TopicMatcherType,
};
use unify_sl_status_sys::SL_STATUS_OK;
use upvl_db::*;

/// MqttHandler handles subscription messages related to Unify groups.
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
    fn after_connect(&mut self) {
        unify_application_monitoring_init();
    }
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
        let status: sl_status_t = upvl_db::db_upsert_entry(&self.db_conn, provision);

        // Publish only if the database could take the update.
        if status == SL_STATUS_OK {
            self.publish_list();
        }
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
        if upvl_db::db_remove_entry(&self.db_conn, &provision) > 0 {
            // Return true to say: Publish an updated List
            self.publish_list();
        } else {
            log_debug!("Skipping SmartStart list publication, as no entry was removed.");
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
    use super::*;
    use mockall::predicate;
    use mockall::predicate::eq;
    use unify_mqtt_sys::{MockMqttClientTrait, MockTopicMatcherTrait};

    fn create_mocked_mqtt_handler() -> MqttHandler<MockMqttClientTrait> {
        let mut mqtt_client_mock = MockMqttClientTrait::new();
        let update_topic_matcher_mock: Result<TopicMatcherType, sl_status_t> =
            Ok(Box::new(MockTopicMatcherTrait::new()));
        mqtt_client_mock
            .expect_subscribe()
            .with(eq("ucl/SmartStart/List/Update/#"))
            .return_once_st(|_| update_topic_matcher_mock);

        let remove_topic_matcher_mock: Result<TopicMatcherType, sl_status_t> =
            Ok(Box::new(MockTopicMatcherTrait::new()));
        mqtt_client_mock
            .expect_subscribe()
            .with(eq("ucl/SmartStart/List/Remove/#"))
            .return_once_st(|_| remove_topic_matcher_mock);

        // MqttHandler publishes SmartStart list in it's constructor - so we have to handle this MQTT publication
        {
            let expected = predicate::function(|x| {
                x == serde_json::json!({ "value": serde_json::Value::Array(vec![]) })
                    .to_string()
                    .as_bytes()
            });
            mqtt_client_mock
                .expect_publish()
                .with(eq("ucl/SmartStart/List"), expected, eq(true))
                .return_once_st(|_, _, _| Ok(()));
        }

        MqttHandler::new(mqtt_client_mock, UpvlConfig::new_for_unit_tests()).unwrap()
    }

    macro_rules! EXPECT_PUBLISH_VALUE {
        ($mqtt_handler:ident, $topic:expr, $value:expr) => {{
            let expected = predicate::function(|x| {
                x == serde_json::json!({ "value": serde_json::Value::Array($value) })
                    .to_string()
                    .as_bytes()
            });
            log_error!(
                "expected: '{}'",
                serde_json::json!({ "value": serde_json::Value::from($value) }).to_string()
            );
            $mqtt_handler
                .client
                .expect_publish()
                .with(eq($topic), expected, eq(true))
                .return_once_st(|_, _, _| Ok(()));
        }};
    }

    #[test]
    fn test_update_remove() {
        let mut mqtt_upvl_handler = create_mocked_mqtt_handler();

        let list_topic = "ucl/SmartStart/List";

        // add the first node
        EXPECT_PUBLISH_VALUE!(
            mqtt_upvl_handler,
            list_topic,
            vec![
                serde_json::json! ({"DSK":"1111", "Include":true, "ProtocolControllerUnid":"zw-D7C4FD24-00001", "Unid":"zw-D7C4FD24-0001"})
            ]
        );
        {
            let json: serde_json::Value = serde_json::from_str("{\"DSK\":\"1111\",\"Unid\":\"zw-D7C4FD24-0001\",\"ProtocolControllerUnid\":\"zw-D7C4FD24-00001\",\"Include\":true}").unwrap();
            assert_eq!(mqtt_upvl_handler.on_update_subscriber(json), Ok(()));
        }

        // add the second node
        EXPECT_PUBLISH_VALUE!(
            mqtt_upvl_handler,
            list_topic,
            vec![
                serde_json::json! ({"DSK":"1111", "Include":true,  "ProtocolControllerUnid":"zw-D7C4FD24-00001", "Unid":"zw-D7C4FD24-0001"}),
                serde_json::json! ({"DSK":"2222", "Include":false, "ProtocolControllerUnid":"zw-D7C4FD24-00002", "Unid":"zw-D7C4FD24-0002"})
            ]
        );
        {
            let json: serde_json::Value = serde_json::from_str("{\"DSK\":\"2222\",\"Unid\":\"zw-D7C4FD24-0002\",\"ProtocolControllerUnid\":\"zw-D7C4FD24-00002\",\"Include\":false}").unwrap();
            assert_eq!(mqtt_upvl_handler.on_update_subscriber(json), Ok(()));
        }

        // add the third node
        EXPECT_PUBLISH_VALUE!(
            mqtt_upvl_handler,
            list_topic,
            vec![
                serde_json::json! ({"DSK":"1111", "Include":true,  "ProtocolControllerUnid":"zw-D7C4FD24-00001", "Unid":"zw-D7C4FD24-0001"}),
                serde_json::json! ({"DSK":"2222", "Include":false, "ProtocolControllerUnid":"zw-D7C4FD24-00002", "Unid":"zw-D7C4FD24-0002"}),
                serde_json::json! ({"DSK":"3333", "Include":true , "ProtocolControllerUnid":"zw-D7C4FD24-00003", "Unid":"zw-D7C4FD24-0003"})
            ]
        );
        {
            let json: serde_json::Value = serde_json::from_str("{\"DSK\":\"3333\",\"Unid\":\"zw-D7C4FD24-0003\",\"ProtocolControllerUnid\":\"zw-D7C4FD24-00003\",\"Include\":true}").unwrap();
            assert_eq!(mqtt_upvl_handler.on_update_subscriber(json), Ok(()));
        }

        // delete the second node
        EXPECT_PUBLISH_VALUE!(
            mqtt_upvl_handler,
            list_topic,
            vec![
                serde_json::json! ({"DSK":"1111", "Include":true,  "ProtocolControllerUnid":"zw-D7C4FD24-00001", "Unid":"zw-D7C4FD24-0001"}),
                serde_json::json! ({"DSK":"3333", "Include":true , "ProtocolControllerUnid":"zw-D7C4FD24-00003", "Unid":"zw-D7C4FD24-0003"})
            ]
        );
        {
            let json: serde_json::Value = serde_json::from_str("{\"DSK\":\"2222\"}").unwrap();
            assert_eq!(mqtt_upvl_handler.on_remove_subscriber(json), Ok(()));
        }
    }

    #[test]
    fn test_update_existing_dsk() {
        let mut mqtt_upvl_handler = create_mocked_mqtt_handler();

        let list_topic = "ucl/SmartStart/List";

        // add the first node
        EXPECT_PUBLISH_VALUE!(
            mqtt_upvl_handler,
            list_topic,
            vec![
                serde_json::json! ({"DSK":"1111", "Include":true, "ProtocolControllerUnid":"zw-D7C4FD24-00001", "Unid":"zw-D7C4FD24-0001"})
            ]
        );
        {
            let json: serde_json::Value = serde_json::from_str("{\"DSK\":\"1111\",\"Unid\":\"zw-D7C4FD24-0001\",\"ProtocolControllerUnid\":\"zw-D7C4FD24-00001\",\"Include\":true}").unwrap();
            assert_eq!(mqtt_upvl_handler.on_update_subscriber(json), Ok(()));
        }

        // add the second node
        EXPECT_PUBLISH_VALUE!(
            mqtt_upvl_handler,
            list_topic,
            vec![
                serde_json::json! ({"DSK":"1111", "Include":true,  "ProtocolControllerUnid":"zw-D7C4FD24-00001", "Unid":"zw-D7C4FD24-0001"}),
                serde_json::json! ({"DSK":"2222", "Include":false, "ProtocolControllerUnid":"zw-D7C4FD24-00002", "Unid":"zw-D7C4FD24-0002"})
            ]
        );
        {
            let json: serde_json::Value = serde_json::from_str("{\"DSK\":\"2222\",\"Unid\":\"zw-D7C4FD24-0002\",\"ProtocolControllerUnid\":\"zw-D7C4FD24-00002\",\"Include\":false}").unwrap();
            assert_eq!(mqtt_upvl_handler.on_update_subscriber(json), Ok(()));
        }

        // update several fields of the first node
        EXPECT_PUBLISH_VALUE!(
            mqtt_upvl_handler,
            list_topic,
            vec![
                serde_json::json! ({"DSK":"1111", "Include":false,  "ProtocolControllerUnid":"zw-D7C4FD24-00003", "Unid":"zw-D7C4FD24-0001"}),
                serde_json::json! ({"DSK":"2222", "Include":false, "ProtocolControllerUnid":"zw-D7C4FD24-00002", "Unid":"zw-D7C4FD24-0002"})
            ]
        );
        {
            let json: serde_json::Value = serde_json::from_str("{\"DSK\":\"1111\",\"ProtocolControllerUnid\":\"zw-D7C4FD24-00003\",\"Include\":false}").unwrap();
            assert_eq!(mqtt_upvl_handler.on_update_subscriber(json), Ok(()));
        }
    }
}
