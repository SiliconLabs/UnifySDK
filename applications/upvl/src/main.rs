// License
// <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

// Warn about missing documentation when running cargo doc
// It doesn't seem to work very well, though.
// See also https://doc.rust-lang.org/rustdoc/how-to-write-documentation.html
#![warn(missing_docs)]

//! The UPVL Client manages the SmartStart List topic `ucl/SmartStart/List`.

// imports, external crates

// JSON-related imports
// Serialize/deserialize crate
extern crate serde;
// Crate to ser/de-ser into json syntax
extern crate serde_json;

// Use some "standard libraries"
use std::ffi::CString;
use std::str; //TODO: try to avoid using ffi in main

// Import the locally defined modules and types
mod upvl_db;

mod upvl_json;

mod upvl_mqtt;

use upvl_log::{self, log_error};

use upvl_conf::*;

const LOG_TAG: &'static str = "upvl_main";
const DB_FILE: &'static str = "upvl_test.db";
const CLIENT_NAME: &'static str = "upvl_client";

fn main() -> Result<(), i32> {
    if parse_application_arguments().is_err() {
        // error message handled by uic_config
        return Ok(());
    }

    let res = get_config();
    let (mqtt_broker, mqtt_port, db_file, ca_file, cert_file, key_file, log_level) =
        ok_or_exit_with_message(res, "could not load configuration")?;

    println!("MQTT broker from config: {}", &mqtt_broker);
    println!("MQTT port number from config: {}", &mqtt_port);
    println!("UPVL DB filename from config: {}", &db_file);
    println!("CA Certificate file from config: {}", &ca_file);
    println!(
        "Certificate file for this client from config: {}",
        &cert_file
    );
    println!(
        "Private key file for this client from config: {}",
        &key_file
    );
    println!("Logging level from config: {}", &log_level);

    // Connect to the data store.
    // This currently lives outside the mqtt module, since we may also
    // want to store other data than provisions in the data store,
    // e.g., configuration settings.
    let db_conn = upvl_db::db_setup(db_file);

    // Debug printout
    // of the provisions found in the data store.
    // Useful for the desk tests
    upvl_db::db_debug_print_provisions(&db_conn);

    // Create a handler to the mqtt client interface (see upvl_mqtt.rs)
    // Connect to the broker.
    let mq_ses = upvl_mqtt::UpvlMqttSession::new(
        &mqtt_broker,
        mqtt_port as u32,
        &ca_file,
        &cert_file,
        &key_file,
    );

    // Spawn the demo-thread that publishes the updates to test the
    // subscriber (see upvl_mqtt.rs)
    //
    // To run the demo, include this call and comment out the
    // #[cfg(test)] in upvl_mqtt.
    //mq_ses.spawn_test_thread();

    // The mqtt session data contains whatever data we want to mqtt
    // event handler callbacks to know and share.
    // The only important data at the moment is a reference to data
    // store connection.
    let ses_data = upvl_mqtt::UpvlMqttSessionData::new(&db_conn);

    // Before starting the infinity loop publish the smart start list to the
    // ucl/SmartStart/List topic for anyone listening before update
    mq_ses.publish_list(upvl_db::db_list_provisions(&db_conn));

    // Start infinity loop
    mq_ses.run(ses_data);

    // Debug printout of the final contents of the data store.
    upvl_db::db_debug_print_provisions(&db_conn);

    // Close the mqtt connection
    mq_ses.disconnect().expect("Disconnect failed.");

    // Close the database
    upvl_db::db_teardown(db_conn);

    upvl_log::log_info(LOG_TAG, format!("Goodbye, world!"));
    Ok(())
}

fn get_config() -> Result<(String, i32, String, String, String, String, String), config_status_t> {
    upvl_log::sl_log_read_config();

    let mqtt_broker = config_get_as_string("mqtt.host")?;
    let mqtt_port = config_get_as_int("mqtt.port")?;
    let db_file = config_get_as_string("upvl.db_file")?;
    let ca_file = config_get_as_string("mqtt.cafile")?;
    let cert_file = config_get_as_string("mqtt.certfile")?;
    let key_file = config_get_as_string("mqtt.keyfile")?;
    let log_level = config_get_as_string("log.level")?;

    Ok((
        mqtt_broker,
        mqtt_port,
        db_file,
        ca_file,
        cert_file,
        key_file,
        log_level,
    ))
}

fn parse_application_arguments() -> Result<(), config_status_t> {
    let version = env!("VERSION_STR");

    // Create a vector of strings from cmd line arguments
    let args = std::env::args()
        .map(|arg| CString::new(arg).unwrap())
        .collect::<Vec<CString>>();

    config_add_string("upvl.db_file", "File name of UPVL database", DB_FILE)
        .or(config_add_string(
            "mqtt.client_id",
            "Client ID for MQTT client",
            CLIENT_NAME,
        ))
        .or(upvl_conf::config_parse(args, version))
}

fn ok_or_exit_with_message<T, E: std::fmt::Display>(
    res: std::result::Result<T, E>,
    message: &str,
) -> std::result::Result<T, i32> {
    res.map_err(|e| {
        log_error(LOG_TAG, format!("{} - {}", e, message));
        -1
    })
}
