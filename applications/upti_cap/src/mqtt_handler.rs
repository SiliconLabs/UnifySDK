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

use crate::pti_sniffer;
use unify_application_monitoring_sys::unify_application_monitoring_init;
use unify_log_sys::*;
use unify_mqtt_sys::{
    sl_status_t, MosqMessage, MqttClientCallbacksTrait, MqttClientTrait, TopicMatcherType,
    UnifyMqttClient,
};

declare_app_name!("upti_cap_mqtt_handler");

/// MqttHandler handles subscription messages related to Unify groups.
/// It subscribes to listen to the correct topics and is to be passed to the mosquitto client
pub struct MqttHandler {
    // matchers that are used to scan incoming mqtt messages
    adapter_toggle: TopicMatcherType,
    adapters_discovery: TopicMatcherType,
}

impl MqttClientCallbacksTrait for MqttHandler {
    /// public function to be called by the mosquitto fwk on a received message.
    fn on_message(&mut self, msg: MosqMessage) {
        if let Err(e) = self.process_received_message(msg) {
            log_error!("{}", e);
        }
    }

    fn before_disconnect(&mut self) {
        pti_sniffer::remove_list().unwrap()
    }

    fn after_connect(&mut self) {
        unify_application_monitoring_init();
        pti_sniffer::discovery_adapters().unwrap()
    }
}

impl MqttHandler {
    pub fn new() -> Result<Self, sl_status_t> {
        let client = UnifyMqttClient::default();
        Ok(MqttHandler {
            adapter_toggle: client.subscribe("ucl/UPTICap/+/Toggle")?,
            adapters_discovery: client.subscribe("ucl/UPTICap/Discovery")?,
        })
    }

    fn process_received_message(&mut self, msg: MosqMessage) -> std::result::Result<(), String> {
        if msg.text().is_empty() {
            // Ignore unretain message
            log_debug!("Unretain of {}", msg.topic());
            Ok(())
        } else {
            let json = json::parse(msg.text())
                .map_err(|e| format!("could not parse body of topic '{}' : {}", msg.topic(), e))?;

            if self.adapter_toggle.matches(&msg) {
                self.on_adapter_toggle(json, msg.topic())
            } else if self.adapters_discovery.matches(&msg) {
                self.on_discovery()
            } else {
                Err(format!("no handler for subscribed topic {}", msg.topic()))
            }
        }
    }

    /// Parse topic name and extract SerialNumber
    fn get_serial_number_from_topic(&self, topic: &str) -> std::result::Result<String, String> {
        let list = topic.split_terminator('/').collect::<Vec<&str>>();
        let serial_number = list[2];
        Ok(serial_number.to_string())
    }

    /// ucl/UPTIcap/{SerialNumber}/Toggle
    fn on_adapter_toggle(
        &mut self,
        json: json::JsonValue,
        topic: &str,
    ) -> std::result::Result<(), String> {
        let serial_number = self.get_serial_number_from_topic(topic)?;
        match pti_sniffer::toggle_adapter(json, serial_number) {
            Ok(_) => {}
            Err(_) => {}
        }
        Ok(())
    }

    /// ucl/UPTIcap/Discover
    fn on_discovery(&mut self) -> std::result::Result<(), String> {
        let out = pti_sniffer::discovery_adapters();
        match out {
            Ok(_v) => return Ok(()),
            Err(e) => return Err(format!("Failed discovering adapters. Error code: {}", e)),
        }
    }
}
