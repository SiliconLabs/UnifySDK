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

#include "zigpc_config.h"
#include "zigpc_config_fixt.h"

// List of default settings
static const char *CONFIG_KEY_ZIGPC_SERIAL_PORT = "zigpc.serial";
static const char *DEFAULT_SERIAL_PORT          = "/dev/ttyUSB0";

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
   * - datastore.file
   */
  config_status_t status = CONFIG_STATUS_OK;
  status |= config_add_string(CONFIG_KEY_ZIGPC_SERIAL_PORT,
                              "serial port to use",
                              DEFAULT_SERIAL_PORT);

  return status != CONFIG_STATUS_OK;
}

sl_status_t zigpc_config_fixt_setup()
{
  config_status_t status
    = config_get_as_string(CONFIG_KEY_ZIGPC_SERIAL_PORT, &config.serial_port);
  status |= config_get_as_string(CONFIG_KEY_MQTT_HOST, &config.mqtt_host);
  status |= config_get_as_int(CONFIG_KEY_MQTT_PORT, &config.mqtt_port);
  status
    |= config_get_as_string(CONFIG_KEY_DATASTORE_FILE, &config.datastore_file);
  return status == CONFIG_STATUS_OK ? SL_STATUS_OK : SL_STATUS_FAIL;
}

const zigpc_config_t *zigpc_get_config(void)
{
  return &config;
}
