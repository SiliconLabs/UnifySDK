// License
// <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

//! UIC IMAGE PROVIDER
//! Is an UIC helper application that announce and provide OTA images.

// mod cache;
mod mqtt_client;
mod mqtt_handler;
mod image_watcher;

#[macro_use]
extern crate lazy_static;

use mqtt_client::*;
use mqtt_handler::*;
use image_watcher::*;
use std::{convert::TryInto};
use std::sync::Arc;
use uic_config::*;
use uic_log::*;
use std::path::Path;

const APP_NAME: &'static str = "uic-image-provider";
const IMAGE_PATH: &'static str = "/var/lib/uic-image-provider";
const POLL_PERIOD: i32 = 15;
const CONFIG_VERSION: &str = env!("VERSION_STR");

fn main() -> std::result::Result<(), i32> {
    if parse_application_arguments().is_err() {
        // error message handled by uic_config
        return Ok(());
    }
 
    let res = get_config();
    let (mqtt_broker, mqtt_port, mqtt_client_id, ca_file, cert_file, key_file, image_path, poll_period) =
        ok_or_exit_with_message(res, "could not load configuration")?;

    let res = run(&mqtt_client_id, 
        &mqtt_broker, 
        mqtt_port, 
        &ca_file, 
        &cert_file, 
        &key_file, 
        &image_path, 
        poll_period);
        ok_or_exit_with_message(res, "mqtt_client error")
}

fn run(mqtt_client_id: &str, 
    mqtt_broker: &str, 
    mqtt_port: i32, 
    ca_file: &str, 
    cert_file: &str, 
    key_file: &str,
    image_path: &str, 
    poll_period: i32) -> mosquitto_client::Result<()> {
    let mqtt_client = Arc::new(MosquittoClient::new(mqtt_client_id));

    if !ca_file.is_empty() && !cert_file.is_empty() && !key_file.is_empty() {
        let cafilepath = Path::new(ca_file);
        let certfilepath = Path::new(cert_file);
        let keyfilepath = Path::new(key_file);

        if cafilepath.exists() && certfilepath.exists() && keyfilepath.exists() {
            mqtt_client.tls_set(&cafilepath, &certfilepath, &keyfilepath)?;
        }
    }
    
    mqtt_client.connect(&mqtt_broker, mqtt_port.try_into().unwrap())?;

    // Run Image Watcher for scanning images.json changes. Share mqtt_client 
    ImageWatcher::new(
        mqtt_client.clone(), 
        image_path.to_string(), 
        poll_period
    ).run_image_watcher();

    let data_publish_trigger = mqtt_client.subscribe("ucl/OTA/data/+/+/get", 0)?;

    let handler = MqttHandler::new(
        mqtt_client.clone(),
        data_publish_trigger,
    );

    // Endless loop here
    mqtt_client.mainloop(handler)
}

fn get_config() -> std::result::Result<(String, i32, String, String, String, String, String, i32), 
    config_status_t> {
    sl_log_read_config();

    let mqtt_broker = config_get_as_string("mqtt.host")?;
    let mqtt_port = config_get_as_int("mqtt.port")?;
    let mqtt_client_id = config_get_as_string("mqtt.client_id")?;

    let ca_file = config_get_as_string("mqtt.cafile")?;
    let cert_file = config_get_as_string("mqtt.certfile")?;
    let key_file = config_get_as_string("mqtt.keyfile")?;

    let image_path = config_get_as_string("image_provider.image_path")?;
    let poll_period = config_get_as_int("image_provider.poll_period")?;
    
    Ok((mqtt_broker, mqtt_port, mqtt_client_id, ca_file, cert_file, key_file, image_path, poll_period))
}

fn parse_application_arguments() -> std::result::Result<(), config_status_t> {
    let args = std::env::args()
        .map(|arg| std::ffi::CString::new(arg).unwrap())
        .collect::<Vec<std::ffi::CString>>();

    config_add_string("mqtt.client_id", "override mqtt client name", APP_NAME)
    .or(config_add_string("image_provider.image_path", "image path", IMAGE_PATH))
    .or(config_add_int("image_provider.poll_period", "poll period", POLL_PERIOD))
    .and(config_parse(args, CONFIG_VERSION))
}

fn ok_or_exit_with_message<T, E: std::fmt::Display>(
    res: std::result::Result<T, E>,
    message: &str,
) -> std::result::Result<T, i32> {
    res.map_err(|e| {
        log_error(APP_NAME, format!("{} - {}", e, message));
        -1
    })
}
