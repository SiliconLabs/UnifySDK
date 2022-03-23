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

//! UIC Name and location service
//! The Name and Location Service is a helper MQTT component that allows to perform book-keeping on text names and locations that have been assigned.
//! This functionality allows IoT Services to assign and readback a Name and a Location for each node/endpoint.
#![doc(html_no_source)]

mod cache;
mod mqtt_handler;
use crate::cache::{NALConfig, CONFIG_KEY_NAL_DB_FILE};
use mqtt_handler::*;
use unify_config_sys::*;
use unify_log_sys::*;
use unify_middleware::unify_mqtt_client::{sl_status_t, MqttClientTrait, UnifyMqttClient};
use unify_sl_status_sys::*;

declare_app_name!("uic-nal");
const DB_FILE: &str = "/var/lib/uic/nal.db";
const CONFIG_VERSION: &str = env!("VERSION_STR");

fn main() -> std::result::Result<(), sl_status_t> {
    if parse_application_arguments().is_err() {
        // error message handled by unify_config_sys
        return Ok(());
    }

    let nal_config =
        ok_or_exit_with_message(NALConfig::from_config(), "could not load DB configuration")?;
    ok_or_exit_with_message(run(nal_config), "mqtt_client error")
}

fn parse_application_arguments() -> std::result::Result<(), config_status_t> {
    let args = std::env::args()
        .map(|arg| std::ffi::CString::new(arg).unwrap())
        .collect::<Vec<std::ffi::CString>>();

    config_add_string("mqtt.client_id", "override mqtt client name", APP_NAME)
        .and(config_add_string(
            CONFIG_KEY_NAL_DB_FILE,
            "File name of NAL database",
            DB_FILE,
        ))
        .and(config_parse(args, CONFIG_VERSION))
}

fn run(nal_config: NALConfig) -> Result<(), sl_status_t> {
    let mqtt_client = UnifyMqttClient::default();
    mqtt_client.initialize()?;
    let handler = MqttNALHandler::new(mqtt_client, nal_config)?;
    mqtt_client.mainloop(handler)
}

fn ok_or_exit_with_message<T, E: std::fmt::Display>(
    res: std::result::Result<T, E>,
    message: &str,
) -> std::result::Result<T, sl_status_t> {
    res.map_err(|e| {
        log_error!("{} - {}", e, message);
        SL_STATUS_FAIL
    })
}
