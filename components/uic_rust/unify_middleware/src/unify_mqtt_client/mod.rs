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
//! # use unify_middleware::unify_mqtt_client::{UnifyMqttClient};
//! # use crate::unify_middleware::unify_mqtt_client::MqttClientTrait;
//! # use unify_middleware::unify_mqtt_client::MockMqttClientCallbacksTrait;
//! let mqtt_client = UnifyMqttClient::default();
//! mqtt_client.initialize().unwrap();
//!
//! mqtt_client.mainloop(MockMqttClientCallbacksTrait::new());
//!```

mod mqtt_client;
pub use self::mqtt_client::*;
use mockall::*;
pub use unify_mqtt_sys::sl_status_t;

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
