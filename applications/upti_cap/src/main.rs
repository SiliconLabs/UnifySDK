// License
// <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

//! UPTI_CAP Service
//! Application to communicate with Silicon Labs WSTK adapters and publish debug channel messages to a MQTT broker.
#![doc(html_no_source)]
mod mqtt_handler;
mod pti_sniffer;
use mqtt_handler::*;
use unify_config_sys::*;
use unify_log_sys::*;
use unify_middleware::unify_mqtt_client::{sl_status_t, MqttClientTrait, UnifyMqttClient};
use unify_sl_status_sys::*;

use std::ffi::CString;
use std::str;

#[macro_use]
extern crate lazy_static;

declare_app_name!("upti_cap");
const UNID: &'static str = "upti";
const IPS: &'static str = "";
const CONFIG_VERSION: &str = env!("VERSION_STR");

fn main() -> std::result::Result<(), sl_status_t> {
    if parse_application_arguments().is_err() {
        // error message handled by uic_config
        return Ok(());
    }

    ok_or_exit_with_message(run(), "mqtt_client error")
}

fn parse_application_arguments() -> std::result::Result<(), config_status_t> {
    // Create a vector of strings from cmd line arguments
    let args = std::env::args()
        .map(|arg| CString::new(arg).unwrap())
        .collect::<Vec<CString>>();

    config_add_string("upti.unid", "Controller unid", UNID)
        .and(config_add_string(
            "upti.ips",
            "IP adresses list of debug adapters to connect to",
            IPS,
        ))
        .and(config_add_bool(
            "upti.enable_discovery",
            "Enable/disable debug adapters discovery in local network using broadcast addresses",
            true,
        ))
        .and(config_add_string(
            "mqtt.client_id",
            "Client ID for MQTT client",
            APP_NAME,
        ))
        .and(config_parse(args, CONFIG_VERSION))
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

fn run() -> Result<(), sl_status_t> {
    let mqtt_client = UnifyMqttClient::default();
    mqtt_client.initialize()?;
    let handler = MqttHandler::new()?;

    mqtt_client.mainloop(handler)
}
