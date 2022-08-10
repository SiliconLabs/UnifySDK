//! The Unify MQTT client exposed in rust.
//!
//! This rust client wraps the MQTT client from the C/C++ shared
//! component. The rust client currently only exposes partial
//! functionality of the original MQTT client. This functionality includes
//! publishing, subscribing and unsubscribing.
//!
//! When starting the main loop of the MQTT client a contiki process will start
//! up running the MQTT client. This loop will run the uic_main_loop also from
//! the C/C++ shared library component.
//!
//! The traits of the MQTT client are a [MqttClientCallbacksTrait] which
//! defines callbacks for receiving a MQTT message, before a disconnect
//! and after a connect callbacks.
//!
//! The [TopicMatcherTrait] allows to check for matches on a topic string. It
//! allows for MQTT wildcards such as "#" and "+".
//!
//! Finally the [MqttClientTrait] exposes the basic functionalities of the MQTT
//! client. This includes initializing the MQTT client, the aforementioned
//! [MqttClientTrait::subscribe], [MqttClientTrait::publish] and
//! [MqttClientTrait::unsubscribe] and additionally the
//! [MqttClientTrait::mainloop]. The main loop takes the a generic handler
//! arugment. The handler argument must have implemented the
//! [MqttClientCallbacksTrait].
//!
//! An example starting the main loop looks like the following:
//! ```rust, no_run
//! # use unify_mqtt_sys::{UnifyMqttClient};
//! # use crate::unify_mqtt_sys::MqttClientTrait;
//! # use unify_mqtt_sys::MockMqttClientCallbacksTrait;
//! let mqtt_client = UnifyMqttClient::default();
//! mqtt_client.initialize().unwrap();
//!
//! mqtt_client.mainloop(MockMqttClientCallbacksTrait::new());
//!```

use crate::uic_mqtt::sl_status_t;
use mockall::*;

#[automock]
pub trait MqttClientCallbacksTrait {
    fn on_message(&mut self, msg: MosqMessage);
    fn before_disconnect(&mut self);
    fn after_connect(&mut self);
}

#[automock]
pub trait TopicMatcherTrait {
    fn matches(&self, msg: &MosqMessage) -> bool;
}

pub type TopicMatcherType = Box<dyn TopicMatcherTrait>;

#[automock]
pub trait MqttClientTrait {
    fn initialize(&self) -> Result<(), sl_status_t>;

    fn subscribe(&self, topic: &str) -> Result<TopicMatcherType, sl_status_t>;

    fn unsubscribe(&self, topic: &str) -> Result<(), sl_status_t>;

    fn mainloop<T>(&self, handler: T) -> Result<(), sl_status_t>
    where
        T: 'static + MqttClientCallbacksTrait;

    fn publish(&self, topic: &str, payload: &[u8], retain: bool) -> Result<(), sl_status_t>;
}

// Mosquitto message struct which is received on callbacks listening on MQTT
// subscriptions.
#[derive(Default, Clone)]
pub struct MosqMessage {
    topic: String,
    text: String,
}

impl MosqMessage {
    pub fn new(topic: String, text: String) -> Self {
        MosqMessage { topic, text }
    }

    pub fn topic(&self) -> &str {
        &self.topic
    }

    pub fn text(&self) -> &str {
        &self.text
    }
}
