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

#include "matter_bridge_config.h"
#include "matter_bridge_config_fixt.h"
#include "config.h"
#include "sl_log.h"
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#define LOG_TAG "matter_bridge_config"

// List of default settings
#define CONFIG_KEY_INTERFACE  "interface"
#define CONFIG_KEY_KVS_PATH   "kvs"
#define CONFIG_KEY_VENDOR_ID  "vendor"
#define CONFIG_KEY_PRODUCT_ID "product"
#define CONFIG_KEY_INTERFACE  "interface"

#ifdef __APPLE__
#define DEFAULT_INTERFACE "en0"
#else
#define DEFAULT_INTERFACE "eth0"
#endif

static matter_bridge_config_t config;

int matter_bridge_config_init()
{
  memset(&config, 0, sizeof(config));
  config_status_t status = CONFIG_STATUS_OK;
  status |= config_add_string(CONFIG_KEY_INTERFACE,
                              "Ethernet interface to use",
                              DEFAULT_INTERFACE);
  status |= config_add_string(CONFIG_KEY_KVS_PATH,
                              "Matter key value store path",
                              "/var/chip_unify_bridge.kvs");
  status |= config_add_int(CONFIG_KEY_VENDOR_ID, "Vendor ID", 0xFFF1);
  status |= config_add_int(CONFIG_KEY_PRODUCT_ID, "Product ID", 0x8001);

  return status != CONFIG_STATUS_OK;
}

static int config_get_int_safe(const char *key)
{
  int val = 0;
  if (SL_STATUS_OK != config_get_as_int(key, &val)) {
    sl_log_error(LOG_TAG, "Failed to get int for key: %s", key);
    assert(false);
  }
  return val;
}

sl_status_t matter_bridge_config_fixt_setup()
{
  config_status_t status = CONFIG_STATUS_OK;
  config_get_as_string(CONFIG_KEY_INTERFACE, &config.interface);
  config_get_as_string(CONFIG_KEY_KVS_PATH, &config.kvs_path);
  config.vendor_id  = config_get_int_safe(CONFIG_KEY_VENDOR_ID);
  config.product_id = config_get_int_safe(CONFIG_KEY_PRODUCT_ID);
  return status == CONFIG_STATUS_OK ? SL_STATUS_OK : SL_STATUS_FAIL;
}

const matter_bridge_config_t *matter_bridge_get_config()
{
  return &config;
}
