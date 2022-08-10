/*******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
// Includes from this component
#include "epc_config.h"

// Unify Components
#include "config.h"
#include "sl_log.h"

// Generic includes
#include <string.h>

// Interfaces
#include "uic_version.h"

#define LOG_TAG "epc_config"

static epc_config_t config;

int epc_config_init()
{
  config_status_t status = CONFIG_STATUS_OK;

  status |= config_add_string(CONFIG_KEY_EPC_DATASTORE_FILE,
                              "EPC datastore database file",
                              DEFAULT_EPC_DATASTORE_FILE);

  return status != CONFIG_STATUS_OK;
}

static int config_get_int_safe(const char *key)
{
  int val = 0;
  if (SL_STATUS_OK != config_get_as_int(key, &val)) {
    sl_log_error(LOG_TAG, "Failed to get int for key: %s", key);
  }
  return val;
}

sl_status_t epc_config_fixt_setup()
{
  memset(&config, 0, sizeof(config));
  config_status_t status = CONFIG_STATUS_OK;
  // Datastore configuration
  status |= config_get_as_string(CONFIG_KEY_EPC_DATASTORE_FILE,
                                 &config.datastore_file);
  // MQTT configuration
  status |= config_get_as_string(CONFIG_KEY_MQTT_HOST, &config.mqtt_host);
  status |= config_get_as_string(CONFIG_KEY_MQTT_CAFILE, &config.mqtt_cafile);
  status
    |= config_get_as_string(CONFIG_KEY_MQTT_CERTFILE, &config.mqtt_certfile);
  status |= config_get_as_string(CONFIG_KEY_MQTT_KEYFILE, &config.mqtt_keyfile);
  config.mqtt_port = config_get_int_safe(CONFIG_KEY_MQTT_PORT);

  return status == CONFIG_STATUS_OK ? SL_STATUS_OK : SL_STATUS_FAIL;
}

const epc_config_t *epc_get_config()
{
  return &config;
}
