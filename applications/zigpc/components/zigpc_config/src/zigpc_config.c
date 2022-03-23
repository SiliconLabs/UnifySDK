/*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <string.h>

// UIC Shared components
#include "config.h"
#include "uic_version.h"

#include "zigpc_config.h"
#include "zigpc_config_fixt.h"

// List of default settings
static const char *CONFIG_KEY_ZIGPC_SERIAL_PORT = "zigpc.serial";
static const char *DEFAULT_SERIAL_PORT          = "/dev/ttyUSB0";
static const char *CONFIG_KEY_ZIGPC_USE_TC_WELL_KNOWN_KEY
  = "zigpc.tc_use_well_known_key";
static const bool DEFAULT_USE_TC_WELL_KNOWN_KEY = false;

static const char *CONFIG_KEY_ZIGPC_DATASTORE_FILE = "zigpc.datastore_file";
static const char *DEFAULT_ZIGPC_DATASTORE_FILE    = UIC_VAR_DIR "/zigpc.db";
static const char *CONFIG_KEY_ZIGPC_NCP_FIRMWARE_PATH = "zigpc.ncp_firmware_path";
static const char *CONFIG_FLAG_ZIGPC_NCP_UPDATE = "zigpc.ncp_update";

static zigpc_config_t config;
int zigpc_config_init()
{
  memset(&config, 0, sizeof(config));

  /**
   * These are options which are supported both command line and in config file
   *
   * NOTE: the following options are already configured in
   * components/config/platform/posix/config.cpp:
   * - mqtt.host
   * - mqtt.port
   */
  config_status_t status = CONFIG_STATUS_OK;
  status |= config_add_string(CONFIG_KEY_ZIGPC_DATASTORE_FILE,
                              "Datastore database file",
                              DEFAULT_ZIGPC_DATASTORE_FILE);
  status |= config_add_string(CONFIG_KEY_ZIGPC_SERIAL_PORT,
                              "serial port to use",
                              DEFAULT_SERIAL_PORT);
  status
    |= config_add_bool(CONFIG_KEY_ZIGPC_USE_TC_WELL_KNOWN_KEY,
                       "Allow Trust Center joins using well-known link key",
                       DEFAULT_USE_TC_WELL_KNOWN_KEY);

  status |= config_add_flag(
                CONFIG_FLAG_ZIGPC_NCP_UPDATE, 
                "Flag to initiate an NCP firmware update");

  status |= config_add_string(
                CONFIG_KEY_ZIGPC_NCP_FIRMWARE_PATH,
                "Path to firmware to update NCP",
                ""); //No default path

  return status != CONFIG_STATUS_OK;
}

sl_status_t zigpc_config_fixt_setup()
{
  config_status_t status
    = config_get_as_string(CONFIG_KEY_ZIGPC_SERIAL_PORT, &config.serial_port);
  status |= config_get_as_string(CONFIG_KEY_MQTT_HOST, &config.mqtt_host);
  status |= config_get_as_int(CONFIG_KEY_MQTT_PORT, &config.mqtt_port);
  status
    |= config_get_as_string(CONFIG_KEY_ZIGPC_DATASTORE_FILE, &config.datastore_file);
  status |= config_get_as_bool(CONFIG_KEY_ZIGPC_USE_TC_WELL_KNOWN_KEY,
                               &config.tc_use_well_known_key);

  return status == CONFIG_STATUS_OK ? SL_STATUS_OK : SL_STATUS_FAIL;
}

const zigpc_config_t *zigpc_get_config(void)
{
  return &config;
}
