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
use crate::image_watcher::{get_image_file, ImageWatcher, OtaConfig};
use unify_application_monitoring_sys::unify_application_monitoring_init;
use unify_log_sys::*;
use unify_mqtt_sys::{
    sl_status_t, MosqMessage, MqttClientCallbacksTrait, MqttClientTrait, TopicMatcherType,
};

/// hanldes subscription message related to OTA
pub struct MqttOtaHandler<T: MqttClientTrait> {
    client: T,
    // matchers that are used to scan incoming mqtt messages
    data_publish_trigger_matcher: TopicMatcherType,
}

declare_app_name!("ota_handler");

impl<T: MqttClientTrait + std::clone::Clone> MqttClientCallbacksTrait for MqttOtaHandler<T> {
    // a callback function which will be called when mosquitto client receives
    // message on MQTT interface and disconnect.
    fn on_message(&mut self, msg: MosqMessage) {
        if let Err(e) = self.process_received_message(msg) {
            log_error!("{}", e);
        }
    }

    fn before_disconnect(&mut self) {
        ImageWatcher::remove_image_advertisement().unwrap()
    }

    fn after_connect(&mut self) {
        unify_application_monitoring_init();
    }
}

impl<T: MqttClientTrait + std::clone::Clone> MqttOtaHandler<T> {
    pub fn new(client: T, ota_config: OtaConfig) -> Result<Self, sl_status_t> {
        let data_publish_trigger = client.subscribe("ucl/OTA/data/+/+/get")?;
        let mut image_watcher = ImageWatcher::new::<T>(ota_config);
        image_watcher.run_image_watcher();

        Ok(MqttOtaHandler {
            client,
            data_publish_trigger_matcher: data_publish_trigger,
        })
    }

    fn process_received_message(&mut self, msg: MosqMessage) -> std::result::Result<(), String> {
        if msg.text().is_empty() {
            log_debug!("Unretain of {}", msg.topic());
            Ok(())
        } else {
            if self.data_publish_trigger_matcher.matches(&msg) {
                self.on_data_publish_trigger(msg.topic()).unwrap();
            } else {
                log_info!(
                    "no handler for subscribed topic {} is registred",
                    msg.topic()
                );
            };
            Ok(())
        }
    }

    /// parse topic message and extract uiid and unid
    fn get_uiid_unid_of_topic(&self, topic: &str) -> std::result::Result<(String, String), String> {
        let list = topic.split_terminator('/').collect::<Vec<&str>>();
        let uiid = list[3];
        let unid = list[4];

        Ok((uiid.to_string(), unid.to_string()))
    }

    /// process a ucl/OTA/data/+/+/get topic
    /// and publishes OTA image on the
    /// ucl/OTA/data/<UIID>/all or ucl/OTA/data/<UIID>/<UNID>
    /// (if image available)
    fn on_data_publish_trigger(&self, topic: &str) -> std::result::Result<(), String> {
        let (uiid, unid) = self.get_uiid_unid_of_topic(topic)?;

        //Check if image exists for uiid+inid combination
        let retain: bool = false;
        let topic = format!("ucl/OTA/data/{}/{}", uiid, unid);

        let mut payload: Vec<u8> = Vec::new();

        // Read file to buffer
        get_image_file(uiid, unid, &mut payload);

        if !payload.is_empty() {
            log_info!("Publishing binary payload to \"{}\"", topic);

            self.client
                .publish(&topic, &payload, retain)
                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))?;
        }
        Ok(())
    }
}
