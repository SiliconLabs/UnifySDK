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

//! UIC GMS
//! Is an UIC helper application that enables propagation of the mqtt ucl/by-group functionality.
//! Groups MAY span several Protocol Controllers, so the capabilities of a group will not be advertised by Protocol Controllers themselves.
//! This application alleviates the aggregation logic for other MQTT Clients such as IoT Service that want to make use of the group functionalities.
#![doc(html_no_source)]
mod cache;
mod mqtt_handler;
use mqtt_handler::*;
use unify_config_sys::*;
use unify_log_sys::*;
use unify_middleware::unify_mqtt_client::{sl_status_t, MqttClientTrait, UnifyMqttClient};

declare_app_name!("uic-gms");
const CONFIG_VERSION: &str = env!("VERSION_STR");

fn main() -> std::result::Result<(), sl_status_t> {
    if parse_application_arguments().is_err() {
        // error message handled by unify_config_sys
        return Ok(());
    }

    run()
}

fn run() -> Result<(), sl_status_t> {
    let mqtt_client = UnifyMqttClient::default();
    mqtt_client.initialize()?;
    let handler = MqttGroupHandler::new(mqtt_client)?;
    mqtt_client.mainloop(handler)
}

fn parse_application_arguments() -> std::result::Result<(), config_status_t> {
    let args = std::env::args()
        .map(|arg| std::ffi::CString::new(arg).unwrap())
        .collect::<Vec<std::ffi::CString>>();

    config_add_string("mqtt.client_id", "override mqtt client name", APP_NAME)
        .and(config_parse(args, CONFIG_VERSION))
}
