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

// Warn about missing documentation when running cargo doc
// It doesn't seem to work very well, though.
// See also https://doc.rust-lang.org/rustdoc/how-to-write-documentation.html
//! The UPVL Client manages the SmartStart List topic `ucl/SmartStart/List`.
#![doc(html_no_source)]

use std::ffi::CString;
use std::str;
mod upvl_db;
mod upvl_json;
use unify_application_monitoring_sys::*;
use unify_config_sys::*;
use unify_log_sys::*;
use unify_mqtt_sys::{sl_status_t, MqttClientTrait, UnifyMqttClient};
use unify_sl_status_sys::*;
use upvl_db::*;
mod mqtt_handler;
use mqtt_handler::*;

declare_app_name!("unify-upvl-main");

const DB_FILE: &str = "/var/lib/uic/upvl.db";
const FULL_APPLICATION_NAME: &str = "UPVL (Unify Framework Provisioning List)";
const CONFIG_VERSION: &str = env!("VERSION_STR");

fn main() -> Result<(), sl_status_t> {
    // Create a vector of strings from cmd line arguments
    let args = std::env::args()
        .map(|arg| CString::new(arg).unwrap())
        .collect::<Vec<CString>>();

    if parse_application_arguments(args).is_err() {
        // error message handled by unify_config_sys
        return Ok(());
    }
    let upvl_config =
        ok_or_exit_with_message(UpvlConfig::from_config(), "could not load configuration")?;

    run(upvl_config)
}

fn run(upvl_config: UpvlConfig) -> Result<(), sl_status_t> {
    unify_application_monitoring_set_application_name(FULL_APPLICATION_NAME);
    let mqtt_client = UnifyMqttClient::default();
    mqtt_client.initialize()?;
    let handler = MqttHandler::new(mqtt_client, upvl_config)?;
    mqtt_client.mainloop(handler)
}

fn parse_application_arguments(args: Vec<CString>) -> Result<(), config_status_t> {
    log_info!("args: {:?}", args);
    config_add_string(
        CONFIG_KEY_UPVL_DB_FILE,
        "File name of UPVL database",
        DB_FILE,
    )
    .and(config_parse(args, CONFIG_VERSION))
}

fn ok_or_exit_with_message<T, E: std::fmt::Display>(
    res: Result<T, E>,
    message: &str,
) -> Result<T, sl_status_t> {
    res.map_err(|e| {
        log_error!("{} - {}", e, message);
        SL_STATUS_FAIL
    })
}

#[cfg(test)]
mod test {

    use super::*;
    use crate::CString;
    use unify_config_sys::config_status_t::CONFIG_STATUS_ERROR;

    #[test]
    fn test_parse_application_arguments() {
        let test_args = vec![
            CString::new("").expect("CString error"),
            CString::new("--vsion").expect("CString error"),
        ];
        assert_eq!(
            parse_application_arguments(test_args),
            Err(CONFIG_STATUS_ERROR)
        );
    }
}
