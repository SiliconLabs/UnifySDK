use crate::mqtt_handler::MqttHandler;
use mosquitto_client::Mosquitto;
pub use mosquitto_client::{MosqMessage, Result, TopicMatcher};
use uic_log::log_info;
use std::path::Path;
pub trait MqttMessageReceived {
    fn on_message(&mut self, msg: MosqMessage);
}

/// interface wrapper of the mosquitto_client, so that we can reuse it for testig purposes.
pub trait MqttClient<'a, T: MqttMessageReceived = MqttHandler<'a>> {
    fn connect(&self, mqtt_broker: &str, mqtt_port: u32) -> mosquitto_client::Result<()>;
    fn tls_set(&self, cafilepath: &Path, certfilepath: &Path, keyfilepath: &Path) -> mosquitto_client::Result<()>;
    fn subscribe(&'a self, topic: &str, qos: u32) -> mosquitto_client::Result<TopicMatcher<'a>>;
    fn mainloop(&self, handler: T) -> mosquitto_client::Result<()>;
    fn publish(
        &self,
        topic: &str,
        payload: &[u8],
        qos: u32,
        retain: bool,
    ) -> mosquitto_client::Result<i32>;
}

pub struct MosquittoClient {
    client: Mosquitto,
}

impl MosquittoClient {
    pub fn new(mqtt_client_id: &str) -> Self {
        MosquittoClient {
            client: Mosquitto::new(&mqtt_client_id),
        }
    }
}

impl<'a> MqttClient<'a> for MosquittoClient {
    fn connect(&self, mqtt_broker: &str, mqtt_port: u32) -> mosquitto_client::Result<()> {
        log_info(
            "mqtt_client",
            format!("connecting to \"{}\" port:{}", mqtt_broker, mqtt_port),
        );
        self.client.connect(mqtt_broker, mqtt_port, 200)
    }

    fn tls_set(&self, cafilepath: &Path, certfilepath: &Path, keyfilepath: &Path) -> mosquitto_client::Result<()> {
        log_info(
            "mqtt_client",
            format!("setting up tls with ca: {}, device cert: {}, device key :{}", 
            cafilepath.display(), certfilepath.display(), keyfilepath.display()),
        );
        self.client.tls_set(cafilepath, certfilepath, keyfilepath, None)
    }

    fn subscribe(&'a self, topic: &str, qos: u32) -> mosquitto_client::Result<TopicMatcher<'a>> {
        log_info(
            "mqtt_client",
            format!("subscribing to \"{}\" qos:{}", topic, qos),
        );
        self.client.subscribe(topic, qos)
    }

    fn mainloop(&self, handler: MqttHandler<'a>) -> mosquitto_client::Result<()> {
        let mut mc = self.client.callbacks(handler);
        mc.on_message(|mqtt_handler, msg| mqtt_handler.on_message(msg));
        self.client.loop_until_disconnect(-1)
    }

    fn publish(
        &self,
        topic: &str,
        payload: &[u8],
        qos: u32,
        retain: bool,
    ) -> mosquitto_client::Result<i32> {
        log_info(
            "mqtt_client",
            format!(
                "publishing \"{}\" payload:{} ret:{}",
                topic,
                std::str::from_utf8(payload).unwrap(),
                retain
            ),
        );
        self.client.publish(topic, payload, qos, retain)
    }
}
