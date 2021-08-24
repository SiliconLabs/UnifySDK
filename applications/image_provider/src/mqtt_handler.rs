use crate::{mqtt_client::*};
use crate::image_watcher;
use std::sync::Arc;
use uic_log::log_error;
use uic_log::log_info;

/// MqttHandler handles subscription messages related to UIC OTA.
/// It subscribes to listen to the correct topics and is to be passed to the mosquitto client
pub struct MqttHandler<'a> {
    /// Reference counted to the mosquitto-client. Notice that its lifetime spans as long as mqttHandler itself.
    /// e.g. the mqtt_client is locked for immutablilty as long as the MqttHandler exists.
    /// This reference is used on a callback of the mosquitto client.
    mqtt_client: Arc<dyn MqttClient<'a>>,
    // matchers that are used to scan incoming mqtt messages
    data_publish_trigger_matcher: TopicMatcher<'a>,
}

impl<'a> MqttMessageReceived for MqttHandler<'a> {
    /// public function to be called by the mosquitto fwk on a received message.
    fn on_message(&mut self, msg: MosqMessage) {
        if let Err(e) = self.process_message(msg) {
            log_error("mqtt_handler", e);
        }
    }
}

impl<'a> MqttHandler<'a> {
    /// crate new handler to be passed into a MqttClient. The native mqtt client is passed as reference counted.
    /// Reference counted objects can be shared and borrowed over multiple instances. 
    pub fn new(
        client: Arc<dyn MqttClient<'a>>,
        matchers: TopicMatcher<'a>,
    ) -> Self {
        MqttHandler {
            mqtt_client: client,
            data_publish_trigger_matcher: matchers
        }
    }

    fn process_message(&mut self, msg: MosqMessage) -> std::result::Result<(), String> {
        let json = json::parse(msg.text())
            .map_err(|e| format!("could not parse body of topic '{}' : {}", msg.topic(), e))?;

        if self.data_publish_trigger_matcher.matches(&msg) {
            self.on_data_publish_trigger(msg.topic(), json)
        } else {
            Err(format!("no handler for subscribed topic {}", msg.topic()))
        }
    }

    /// parse topic message and extract uiid and unid
    fn get_uiid_unid_of_topic(topic: &str) -> std::result::Result<(String, String), String> {
        let list = topic.split_terminator('/').collect::<Vec<&str>>();
        let uiid = list[3];
        let unid = list[4];

        Ok((uiid.to_string(), unid.to_string()))
    }

    /// process a ucl/OTA/data/+/+/get topic
    /// and publishes OTA image on the
    /// ucl/OTA/data/<UIID>/all or ucl/OTA/data/<UIID>/<UNID>
    /// (if image available)
    fn on_data_publish_trigger(
        &mut self,
        topic: &str,
        _json: json::JsonValue,
    ) -> std::result::Result<(), String> {
        let (uiid, unid) = MqttHandler::get_uiid_unid_of_topic(topic)?;

        //Check if image exists for uiid+inid combination 
        let retain: bool = false;
        let topic = format!("ucl/OTA/data/{}/{}", uiid, unid);
        
        let mut payload: Vec<u8> = Vec::new();

        // Read file to buffer
        image_watcher::get_image_file(uiid, unid, &mut payload);

        if payload.len() > 0 {
            log_info(
                "mqtt_handler",
                format!(
                    "Publishing binary payload to \"{}\"",
                    topic,
                ),
            );

            self.mqtt_client
                .publish(&topic, &payload, 0, retain)
                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))?;
        }        
        Ok(())
    }
}

#[cfg(test)]
/// Implementing mocks for MqttClient will result in lifetime issues. MqttClient has a limited lifetime,
/// because of the borrows inside of the native mosquitto client itself. it can be moved while
// the borrow still exists in the MqttHandler. Solution would be to declare the mqttclient-mock static with
// interior mutability (see RefCell), maybe a solution with Pinning can prohibit the instance to be moved and
// therefore garuantuee to be valid for the whole lifetime of the mqtthandler
mod tests {}
