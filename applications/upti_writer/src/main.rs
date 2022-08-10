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

//! UPTI_WRITER
//! Application to save trace packages, published by UPTI_CAP, to file in Network Analyzer compatible format.
#![doc(html_no_source)]

mod log_formatter;
mod mqtt_handler;
use mqtt_handler::*;
use unify_config_sys::*;
use unify_log_sys::*;
use unify_mqtt_sys::{sl_status_t, MqttClientTrait, UnifyMqttClient};
use unify_sl_status_sys::*;

use std::ffi::CString;
use std::str;

declare_app_name!("unify-upti-writer");
const UNID: &'static str = "upti_w";
const IPS: &'static str = "";
const CONFIG_VERSION: &str = env!("VERSION_STR");
const LOG_PATH: &str = "/var/lib/uic-upti-writer";

fn main() -> std::result::Result<(), sl_status_t> {
    if parse_application_arguments().is_err() {
        // error message handled by uic_config
        return Ok(());
    }

    let writer_config = ok_or_exit_with_message(
        UptiWriterConfig::from_config(),
        "could not load configuration",
    )?;

    ok_or_exit_with_message(run(writer_config), "init error")
}

fn parse_application_arguments() -> std::result::Result<(), config_status_t> {
    // Create a vector of strings from cmd line arguments
    let args = std::env::args()
        .map(|arg| CString::new(arg).unwrap())
        .collect::<Vec<CString>>();

        config_add_string(
            "upti_writer.unid",
            "Controller unid",
            UNID,
        )
        .and(config_add_string(
            "upti_writer.log_path",
            "Log path",
            LOG_PATH,
        ))
        .and(config_add_string(
            "upti_writer.adapters",
            "List of serial numbers of debug adapters to log",
            IPS,
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

fn run(writer_config: UptiWriterConfig) -> Result<(), sl_status_t> {
    let mqtt_client = UnifyMqttClient::default();
    mqtt_client.initialize()?;
    let handler = MqttHandler::new(mqtt_client, writer_config)?;

    mqtt_client.mainloop(handler)
}
