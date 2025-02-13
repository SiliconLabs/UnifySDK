/*******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "eed_config.h"

// Unify Components
#include "config.h"
#include "sl_log.h"

// Generic includes
#include <string.h>

// Interfaces
#include "uic_version.h"

#define LOG_TAG "eed_config"

static eed_config_t config;

int eed_config_init()
{
  config_status_t status = CONFIG_STATUS_OK;

  status |= config_add_string(CONFIG_KEY_EED_DATASTORE_FILE,
                              "EED datastore database file",
                              DEFAULT_EED_DATASTORE_FILE);
  status |= config_add_string(CONFIG_KEY_EED_CLUSTER_LIST,
                              "EED cluster configuration",
                              DEFAULT_EED_CLUSTER_LIST);
  status |= config_add_string(CONFIG_KEY_EED_DEVICE_TYPE,
                              "EED device type configuration",
                              DEFAULT_EED_DEVICE_TYPE);

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

sl_status_t eed_config_fixt_setup()
{
  memset(&config, 0, sizeof(config));
  config_status_t status = CONFIG_STATUS_OK;
  // Datastore configuration
  status |= config_get_as_string(CONFIG_KEY_EED_DATASTORE_FILE,
                                 &config.datastore_file);
  status |= config_get_as_string(CONFIG_KEY_EED_CLUSTER_LIST,
                                 &config.cluster_list);
  status |= config_get_as_string(CONFIG_KEY_EED_DEVICE_TYPE,
                                 &config.device_type);
  // MQTT configuration
  status |= config_get_as_string(CONFIG_KEY_MQTT_HOST, &config.mqtt_host);
  status |= config_get_as_string(CONFIG_KEY_MQTT_CAFILE, &config.mqtt_cafile);
  status
    |= config_get_as_string(CONFIG_KEY_MQTT_CERTFILE, &config.mqtt_certfile);
  status |= config_get_as_string(CONFIG_KEY_MQTT_KEYFILE, &config.mqtt_keyfile);
  config.mqtt_port = config_get_int_safe(CONFIG_KEY_MQTT_PORT);

  return status == CONFIG_STATUS_OK ? SL_STATUS_OK : SL_STATUS_FAIL;
}

const eed_config_t *eed_get_config()
{
  return &config;
}
