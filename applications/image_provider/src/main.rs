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

//! UIC IMAGE PROVIDER
//! Is an UIC helper application that advertises and provides OTA images.
#![doc(html_no_source)]

mod image_watcher;
mod mqtt_ota;

#[macro_use]
extern crate lazy_static;
use crate::mqtt_ota::MqttOtaHandler;
use image_watcher::OtaConfig;
use unify_config_sys::*;
use unify_log_sys::*;
use unify_middleware::unify_mqtt_client::{sl_status_t, MqttClientTrait, UnifyMqttClient};
use unify_sl_status_sys::*;

declare_app_name!("uic-image-provider");
const IMAGE_PATH: &str = "/var/lib/uic-image-provider";
const POLL_PERIOD: i32 = 15;
const CONFIG_VERSION: &str = env!("VERSION_STR");

fn main() -> std::result::Result<(), sl_status_t> {
    if parse_application_arguments().is_err() {
        // error message handled by unify_config_sys
        return Ok(());
    }

    let ota_config =
        ok_or_exit_with_message(OtaConfig::from_config(), "could not load configuration")?;

    ok_or_exit_with_message(run(ota_config), "mqtt_client error")
}

fn run(ota_config: OtaConfig) -> Result<(), sl_status_t> {
    let mqtt_client = UnifyMqttClient::default();
    mqtt_client.initialize()?;
    let handler = MqttOtaHandler::new(mqtt_client.clone(), ota_config)?;
    mqtt_client.mainloop(handler)
}

fn parse_application_arguments() -> std::result::Result<(), config_status_t> {
    let args = std::env::args()
        .map(|arg| std::ffi::CString::new(arg).unwrap())
        .collect::<Vec<std::ffi::CString>>();

    config_add_string("mqtt.client_id", "override mqtt client name", APP_NAME)
        .and(config_add_string(
            "image_provider.image_path",
            "image path",
            IMAGE_PATH,
        ))
        .and(config_add_int(
            "image_provider.poll_period",
            "poll period",
            POLL_PERIOD,
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
