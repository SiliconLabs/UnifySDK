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

use once_cell::unsync::Lazy;
use std::os::raw::c_char;
use std::{
    ffi::{CStr, CString},
    ops::DerefMut,
};

use crate::unify_mqtt_client::{
    MqttClientCallbacksTrait, MqttClientTrait, TopicMatcherTrait, TopicMatcherType,
};
pub use unify_contiki_sys::{ctimer_init, process_init};
pub use unify_log_sys::*;
pub use unify_main_sys::uic_main;
pub use unify_mqtt_sys::{
    size_t, sl_status_t, uic_mqtt_publish, uic_mqtt_set_after_connect_callback,
    uic_mqtt_set_before_disconnect_callback, uic_mqtt_setup, uic_mqtt_subscribe,
    uic_mqtt_unsubscribe,
};
pub use unify_sl_status_sys::*;

// MQTT Handler is the MQTT client struct owning callback functions and alike.
static mut MQTT_HANDLER: Lazy<Option<Box<dyn MqttClientCallbacksTrait>>> = Lazy::new(|| None);

// Initialization functions setting up contiki and running the uic_main_loop.
extern "C" {
    pub fn uic_signal_handler_setup();
    pub fn uic_main_loop();
    pub fn procinit_init();
}

// Linking to the native mosquitto library client.
#[link(name = "mosquitto")]
extern "C" {
    pub fn mosquitto_topic_matches_sub(sub: *const c_char, topic: *const c_char, result: *mut u8);
}

// This function will do an initial setup of contiki before running any
// contiki loop.
pub unsafe fn init_contiki() {
    ctimer_init();
    process_init();
    procinit_init();
}

// Mosquitto message struct which is received on callbacks listening on MQTT
// subscriptions.
#[derive(Default, Clone)]
pub struct MosqMessage {
    topic: String,
    text: String,
}

impl MosqMessage {
    pub fn topic(&self) -> &str {
        &self.topic
    }

    pub fn text(&self) -> &str {
        &self.text
    }
}

// The implementation of the MQTT client wrapper around the C/C++ shared MQTT
// client component.
#[derive(Default, Clone, Copy)]
pub struct UnifyMqttClient {}

declare_app_name!("mqtt_client");

unsafe extern "C" fn message_callback(
    topic: *const ::std::os::raw::c_char,
    message: *const ::std::os::raw::c_char,
    _message_length: size_t,
) {
    let topic = CStr::from_ptr(topic).to_string_lossy();
    let message = CStr::from_ptr(message).to_string_lossy();
    let msg = MosqMessage {
        topic: topic.into_owned(),
        text: message.into_owned(),
    };

    if let Some(ref mut handler) = MQTT_HANDLER.deref_mut() {
        (*handler).on_message(msg);
    }
}

unsafe extern "C" fn before_disconnect_callback() {
    if let Some(ref mut handler) = MQTT_HANDLER.deref_mut() {
        (*handler).before_disconnect();
    }
}

unsafe extern "C" fn after_connect_callback() {
    if let Some(ref mut handler) = MQTT_HANDLER.deref_mut() {
        (*handler).after_connect();
    }
}

impl MqttClientTrait for UnifyMqttClient {
    fn initialize(&self) -> Result<(), sl_status_t> {
        unsafe {
            init_contiki();
            uic_mqtt_setup();
            uic_signal_handler_setup();
        }
        Ok(())
    }

    fn subscribe(&self, topic: &str) -> Result<TopicMatcherType, sl_status_t> {
        let topic_c_string = CString::new(topic).expect("Failed converting payload message");
        unsafe {
            uic_mqtt_subscribe(topic_c_string.as_ptr(), Some(message_callback));
        }
        Ok(Box::new(MosquittoTopicMatcher::new(topic)))
    }

    fn mainloop<T>(&self, handler: T) -> Result<(), sl_status_t>
    where
        T: 'static + MqttClientCallbacksTrait,
    {
        unsafe {
            *MQTT_HANDLER = Some(Box::new(handler));
            uic_mqtt_set_before_disconnect_callback(Some(before_disconnect_callback));
            uic_mqtt_set_after_connect_callback(Some(after_connect_callback));
            uic_main_loop();
        }
        Ok(())
    }

    fn unsubscribe(&self, topic: &str) -> Result<(), sl_status_t> {
        let topic = CString::new(topic).expect("Failed converting rust string to CString");
        unsafe {
            uic_mqtt_unsubscribe(topic.as_ptr(), Some(message_callback));
        }
        Ok(())
    }

    fn publish(&self, topic: &str, payload: &[u8], retain: bool) -> Result<(), sl_status_t> {
        let message_length: size_t = payload.len() as size_t;
        let topic = CString::new(topic).expect("Failed converting topic string");

        unsafe {
            uic_mqtt_publish(
                topic.as_ptr(),
                payload.as_ptr() as *const std::os::raw::c_char,
                message_length,
                retain,
            );
        }

        Ok(())
    }
}

// The mosquitto topic matcher is for matching on specific topics.
// It enables the use of MQTT wildcards such as "#" and "+".
pub struct MosquittoTopicMatcher {
    topic: String,
}

impl MosquittoTopicMatcher {
    pub fn new(topic: &str) -> Self {
        Self {
            topic: topic.to_string(),
        }
    }
}

// Implementation of the matching function of the MosquittoTopicMatcher.
impl TopicMatcherTrait for MosquittoTopicMatcher {
    fn matches(&self, msg: &MosqMessage) -> bool {
        let mut matched: u8 = 0;
        let sub = CString::new(self.topic.to_string()).unwrap();
        let topic_received = CString::new(msg.topic()).unwrap();
        unsafe {
            mosquitto_topic_matches_sub(sub.as_ptr(), topic_received.as_ptr(), &mut matched);
        }
        matched > 0
    }
}

#[cfg(test)]
mod tests {
    // Note this useful idiom: importing names from outer (for mod tests) scope.
    use super::*;

    #[test]
    fn test_topic_matcher() {
        // 1
        // Multi level wildcard test
        let subscribed_topic = MosquittoTopicMatcher {
            topic: "ucl/by-unid/#".to_string(),
        };

        let mes = MosqMessage {
            topic: "ucl/by-unid/hello/are/you/here".to_string(),
            text: "This is the payload".to_string(),
        };

        let mes_2 = MosqMessage {
            topic: "ucl/hello/are/you/here".to_string(),
            text: "This is the payload".to_string(),
        };

        assert_eq!(subscribed_topic.matches(&mes), true);
        assert_eq!(subscribed_topic.matches(&mes_2), false);

        // 2
        // Single level wildcard test
        let subscribed_topic = MosquittoTopicMatcher {
            topic: "ucl/by-unid/+/test".to_string(),
        };

        let mes = MosqMessage {
            topic: "ucl/by-unid/hello/test".to_string(),
            text: "This is the payload".to_string(),
        };

        let mes_2 = MosqMessage {
            topic: "ucl/by-unid/hello/test/test_2".to_string(),
            text: "This is the payload".to_string(),
        };

        assert_eq!(subscribed_topic.matches(&mes), true);
        assert_eq!(subscribed_topic.matches(&mes_2), false);
    }

    // TODO: When a viable mocking process is in place for binding generated
    // shared components some small test cases should be setup for testing the
    // MQTT client rust wrapper
    #[test]
    fn test_mqtt_rust_wrapper() {}
}
