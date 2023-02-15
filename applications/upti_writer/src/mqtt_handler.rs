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

use crate::log_formatter;
use unify_application_monitoring_sys::unify_application_monitoring_init;
use unify_config_sys::*;
use unify_log_sys::*;
use unify_mqtt_sys::{
    sl_status_t, MosqMessage, MqttClientCallbacksTrait, MqttClientTrait, TopicMatcherType,
};

use std::collections::HashSet;
use std::fs::File;
use std::fs::OpenOptions;
use std::io::{BufWriter, Write};
use std::path::PathBuf;

use mockall::*;

/// MqttHandler handles subscription messages related to PTI adapters
/// It subscribes to listen to the correct topics and is to be passed to the mosquitto client
pub struct MqttHandler<T: MqttClientTrait> {
    _client: T,
    // matchers that are used to scan incoming mqtt messages
    adapters_list_matcher: TopicMatcherType,
    adapters_trace_packages_matcher: TopicMatcherType,
    // File Writer
    writer: BufWriter<File>,
    // Adapters list as HashSet
    adapters_list: HashSet<String>,
}

pub struct UptiWriterConfig {
    adapters: Vec<String>,
    log_path: String,
}

declare_app_name!("upti_writer_mqtt_handler");

impl ConfigLoader for UptiWriterConfig {
    fn from_config() -> Result<Self, unify_config_sys::config_status_t>
    where
        Self: Sized,
    {
        let mut adapters: Vec<String> = vec![];
        let mut log_path: String = String::new();
        match config_get_as_string("upti_writer.log_path") {
            Ok(path) => {
                log_path = path;
            }
            Err(_) => {}
        }
        match config_get_as_string("upti_writer.adapters") {
            Ok(pti_adapters) => {
                if !pti_adapters.is_empty() {
                    adapters = pti_adapters
                        .replace(" ", "")
                        .split(',')
                        .map(String::from)
                        .collect();
                }
            }
            Err(_) => {}
        }

        log_info!("Config: {}:{:?}", log_path, adapters);

        Ok(UptiWriterConfig {
            adapters: adapters,
            log_path: log_path,
        })
    }
}

impl<T: MqttClientTrait> MqttClientCallbacksTrait for MqttHandler<T> {
    /// public function to be called by the mosquitto fwk on a received message.
    fn on_message(&mut self, msg: MosqMessage) {
        if let Err(e) = self.process_received_message(msg) {
            log_error!("{}", e);
        }
    }
    fn before_disconnect(&mut self) {
        self.close_file();
    }
    fn after_connect(&mut self) {
        unify_application_monitoring_init();
    }
}

#[automock]
pub trait MqttHandlerProcessTrait {
    fn process_trace_package(&mut self, json: json::JsonValue, originator: String);
    fn process_adapters_list(&mut self, json: json::JsonValue);
}

impl<T: MqttClientTrait> MqttHandlerProcessTrait for MqttHandler<T> {
    /// Process adapters list and print adapters info messages
    fn process_adapters_list(&mut self, json: json::JsonValue) {
        if json["value"].is_array() {
            log_info!("Adapters list: ");
            let adapters = &json["value"];
            for adapter in adapters.members() {
                log_info!(
                    "Serial number: {}, IP address: {}, is enabled: {}",
                    adapter["SerialNumber"],
                    adapter["PTIAddress"],
                    adapter["Enabled"]
                );
            }
        }
    }

    /// Process TracePackage, Constructs a DebugMessage object from the raw message bytes.
    fn process_trace_package(&mut self, json: json::JsonValue, originator: String) {
        // Take  "RawFrame" value from json, transforms into array of bytes
        if json["RawFrame"].is_array() {
            let raw_frame = &json["RawFrame"];
            let raw_data: Vec<u8> = raw_frame.members().filter_map(|v| v.as_u8()).collect();

            if let Some(record) = log_formatter::log_data_from_raw(raw_data, originator) {
                self.add_record(record);
            }
        }
    }
}

impl<T: MqttClientTrait> MqttHandler<T> {
    pub fn new(client: T, writer_config: UptiWriterConfig) -> Result<Self, sl_status_t> {
        let adapters_list_topic = client.subscribe("ucl/UPTICap/List")?;
        let adapters_trace_packages_topic = client.subscribe("ucl/UPTICap/+/TracePackage")?;

        // Build set of adapters serial numbers from config
        let mut adapters_list: HashSet<String> = HashSet::new();
        for adapter in writer_config.adapters {
            adapters_list.insert(adapter);
        }

        // Build cross platform log file path
        let mut filename = PathBuf::new();
        filename.push(writer_config.log_path);
        filename.push(format!(
            "capture_{}.log",
            chrono::offset::Local::now().format("%H-%M-%S").to_string()
        ));
        log_info!("Using log file {}", filename.display().to_string());
        let log_path = OpenOptions::new()
            .write(true)
            .create(true)
            .open(filename)
            .expect("Unable to open file");

        let mut writer = BufWriter::new(log_path);

        // Add header to log file. It looks Network Analyzer not recognized log files
        // without this header
        let header = "# (c) Ember - InSight Desktop\n".as_bytes();
        writer.write(header).expect("Unable to write data");

        Ok(MqttHandler {
            _client: client,
            adapters_list_matcher: adapters_list_topic,
            adapters_trace_packages_matcher: adapters_trace_packages_topic,
            writer: writer,
            adapters_list: adapters_list,
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

            if self.adapters_trace_packages_matcher.matches(&msg) {
                self.on_trace_package(json, msg.topic())
            } else if self.adapters_list_matcher.matches(&msg) {
                self.on_list_update(json)
            } else {
                Err(format!("no handler for subscribed topic {}", msg.topic()))
            }
        }
    }
    /// ucl/UPTIcap/{SerialNumber}/TracePackage handler
    fn on_trace_package(
        &mut self,
        json: json::JsonValue,
        topic: &str,
    ) -> std::result::Result<(), String> {
        let serial_number = get_serial_number_from_topic(topic)?;
        if self.adapters_list.is_empty() {
            self.process_trace_package(json, serial_number);
        } else if self.adapters_list.contains(&serial_number) {
            self.process_trace_package(json, serial_number);
        }
        Ok(())
    }

    /// ucl/UPTIcap/List handler
    fn on_list_update(&mut self, json: json::JsonValue) -> std::result::Result<(), String> {
        self.process_adapters_list(json);
        Ok(())
    }

    /// Add record to file
    pub fn add_record(&mut self, record: String) {
        self.writer
            .write(record.as_bytes())
            .expect("Unable to write data");
    }

    /// Flush writer's buffer
    pub fn close_file(&mut self) {
        self.writer.flush().unwrap();
        // And it looks that no `close` method for file :(
    }
}

/// Parse topic name and extract SerialNumber
fn get_serial_number_from_topic(topic: &str) -> std::result::Result<String, String> {
    let list = topic.split_terminator('/').collect::<Vec<&str>>();
    let serial_number = list[2];
    Ok(serial_number.to_string())
}

#[cfg(test)]
mod tests {
    use super::*;
    use mockall::predicate::eq;
    use unify_mqtt_sys::MockMqttClientTrait;
    use unify_mqtt_sys::MockTopicMatcherTrait;

    fn create_mocked_mqtt_handler() -> MqttHandler<MockMqttClientTrait> {
        let mut mqtt_client_mock = MockMqttClientTrait::new();
        let writer_config = UptiWriterConfig {
            adapters: vec!["440138076".to_string(), "440138077".to_string()],
            log_path: String::new(),
        };
        let adapters_list_update_matcher_mock: Result<TopicMatcherType, sl_status_t> =
            Ok(Box::new(MockTopicMatcherTrait::new()));
        mqtt_client_mock
            .expect_subscribe()
            .with(eq("ucl/UPTICap/List"))
            .return_once_st(|_| adapters_list_update_matcher_mock);
        let adapters_trace_packages_matcher_mock: Result<TopicMatcherType, sl_status_t> =
            Ok(Box::new(MockTopicMatcherTrait::new()));
        mqtt_client_mock
            .expect_subscribe()
            .with(eq("ucl/UPTICap/+/TracePackage"))
            .return_once_st(|_| adapters_trace_packages_matcher_mock);
        MqttHandler::new(mqtt_client_mock, writer_config).unwrap()
    }

    #[test]
    fn test_get_serial_number_from_topic() {
        let test_topic = "ucl/UPTICap/440138076/TracePackage";
        let serial = get_serial_number_from_topic(test_topic).unwrap();
        assert_eq!(serial, "440138076".to_string());
    }

    #[test]
    fn test_serial_numbers_in_hashset() {
        let mqtt_writer_handler = create_mocked_mqtt_handler();
        assert_eq!(
            mqtt_writer_handler.adapters_list.contains("440138076"),
            true
        );
        assert_eq!(
            mqtt_writer_handler.adapters_list.contains("440138078"),
            false
        );
    }
}
