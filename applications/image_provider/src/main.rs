// License
// <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

//! UIC IMAGE PROVIDER
//! Is an UIC helper application that advertises and provides OTA images.

mod image_watcher;

#[macro_use]
extern crate lazy_static;
extern crate mosquitto_client;
use image_watcher::*;
use std::{convert::TryInto};
use uic_config::*;
use uic_log::*;
use std::path::Path;
use std::thread;
use std::sync::mpsc::channel;

use std::time::Duration;

const APP_NAME: &str = "uic-image-provider";
const IMAGE_PATH: &str = "/var/lib/uic-image-provider";
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

    let mqtt_client = mosquitto_client::Mosquitto::new(mqtt_client_id);

    if !ca_file.is_empty() && !cert_file.is_empty() && !key_file.is_empty() {
        let cafilepath = Path::new(ca_file);
        let certfilepath = Path::new(cert_file);
        let keyfilepath = Path::new(key_file);

        if cafilepath.exists() && certfilepath.exists() && keyfilepath.exists() {
            log_info(
                "image_provider",
                format!("setting up tls with ca: {}, device cert: {}, device key :{}", 
                cafilepath.display(), certfilepath.display(), keyfilepath.display()),
            );
            mqtt_client.tls_set(&cafilepath, &certfilepath, &keyfilepath, None)?;
        }
    }
    
    log_info(
        "image_provider",
        format!("Connecting to \"{}\" port:{}", mqtt_broker, mqtt_port),
    );
    mqtt_client.connect(mqtt_broker, mqtt_port.try_into().unwrap(), 200)?;

    // Set-up subscribtion topics
    let data_publish_trigger = mqtt_client.subscribe("ucl/OTA/data/+/+/get", 0)?;
    let data_unretain_trigger = mqtt_client.subscribe("ucl/OTA/info/+/+", 0)?;

    let mut mc = mqtt_client.callbacks(Vec::new());

    // Set-up on_message callbacks
    mc.on_message(|data,msg| {
        if data_publish_trigger.matches(&msg) {
            match json::parse(msg.text()) {
                Ok(_) => {
                    on_data_publish_trigger(msg.topic(), mqtt_client.clone()).unwrap();
                },
                Err(e) => {
                    log_error(
                    "image_provider",
                    format!("could not parse body of topic '{}' : {}", msg.topic(), e));
                }
            }
        };
        if data_unretain_trigger.matches(&msg) {
            // Collect "ucl/OTA/info/+/+" topics name as vector
            data.push(msg.topic().to_string());
        }
    });

    // Clone mqtt_client and Run main mosquitto loop in thread
    let mt = mqtt_client.clone();
    
    mt.threaded();
    let mosq_thread_handle = thread::spawn(move || {
        match mt.loop_forever(-1) {
            Ok(_) => {
                log_info(
                    "image_provider",
                    "Disconnected from broker".to_string()
                );

            },
            Err(e) => {
                log_info(
                    "image_provider",
                    format!("Disconnected from broker: {}", e)
                );

            }
        }
    });

    // Wait some time for receiving retained messages
    // from "ucl/OTA/info/+/+" topics
    thread::sleep(Duration::from_secs(2));
   
    // Unsubscribe from "ucl/OTA/info/+/+"
    mqtt_client.unsubscribe("ucl/OTA/info/+/+")?;

    // Unretain messages 
    for topic in mc.data.clone() {
        log_info(
            "image_provider",
            format!("Removing image advertisement to \"{}\"", topic.to_string())
        );
        let _msg_id = mqtt_client.publish(&topic, &[], 0, true)
        .map_err(|e| format!("error publishing topic '{}' : {}", topic, e));
    }

    // Clean-up topics vector
    mc.data.clear();

    // Run Image Watcher for scanning images.json changes.
    let _image_watcher_thread_handle = ImageWatcher::new(
        mqtt_client.clone(), 
        image_path.to_string(), 
        poll_period
    ).run_image_watcher();

    let (tx, rx) = channel();
    ctrlc::set_handler(move || tx.send(()).expect("Could not send signal on channel."))
        .expect("Error setting Ctrl-C handler");
   
    rx.recv().expect("Could not receive from channel.");

    log_info(
        "image_provider",
        "Application shutdown".to_string());

    // Remove image advertisement
    ImageWatcher::remove_image_advertisement(mqtt_client.clone());

    // Disconnect from broker
    mqtt_client.disconnect().unwrap();
    
    // Join thread
    mosq_thread_handle.join().unwrap();

    Ok(())
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
    topic: &str,
    mqtt_client: mosquitto_client::Mosquitto
) -> std::result::Result<(), String> {
    let (uiid, unid) = get_uiid_unid_of_topic(topic)?;

    //Check if image exists for uiid+inid combination 
    let retain: bool = false;
    let topic = format!("ucl/OTA/data/{}/{}", uiid, unid);
    
    let mut payload: Vec<u8> = Vec::new();

    // Read file to buffer
    image_watcher::get_image_file(uiid, unid, &mut payload);

    if !payload.is_empty() {
        log_info(
            "image_provider",
            format!(
                "Publishing binary payload to \"{}\"",
                topic,
            ),
        );

        mqtt_client
            .publish(&topic, &payload, 0, retain)
            .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))?;
    }        
    Ok(())
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
