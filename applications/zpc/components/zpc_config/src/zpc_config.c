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

#include "zpc_config.h"
#include "device_id.h"
#include "config.h"
#include <string.h>
#include "sl_log.h"
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "uic_version.h"

#define LOG_TAG "zpc_config"

// List of default settings
#define DEFAULT_ZWAVE_RF_REGION                             "EU"
#define DEFAULT_ZWAVE_NORMAL_TX_POWER_DBM                   0
#define DEFAULT_ZWAVE_MEASURED_0DBM_POWER                   0
#define DEFAULT_SERIAL_PORT                                 "/dev/ttyUSB0"
#define DEFAULT_WAKE_UP_INTERVAL                            4200
#define DEFAULT_MANUFACTURER_ID                             0x0000
#define DEFAULT_PRODUCT_TYPE                                0x0005
#define DEFAULT_PRODUCT_ID                                  0x0001
#define DEFAULT_HARDWARE_VERSION                            1
#define DEFAULT_NUMBER_OF_MISSING_WAKE_UP_NOTIFICATION      2
#define DEFAULT_NUMBER_OF_ACCEPTED_FRAME_TRANSMISSION_ERROR 2
#define DEFAULT_INCLUSION_PROTOCOL_PREFERENCE               "1,2"
#define DEFAULT_OTA_CACHE_PATH                              UIC_VAR_DIR "/zpc/ota_cache"
#define DEFAULT_OTA_CACHE_SIZE                              2 * 1024

static zpc_config_t config;

int zpc_config_init()
{
  memset(&config, 0, sizeof(config));

  // These are options which are supported both command line and in config file
  config_status_t status = CONFIG_STATUS_OK;
  status |= config_add_string("zpc.serial",
                              "Serial port where Z-Wave module is connected",
                              DEFAULT_SERIAL_PORT);
  status |= config_add_string("zpc.rf_region",
                              "Z-Wave RF region setting"
                              "\n  EU: Europe"
                              "\n  US: US"
                              "\n  US_LR: US Long Range"
                              "\n  ANZ:Australia / New Zealand"
                              "\n  HK: Hong Kong"
                              "\n  IN: India"
                              "\n  IL: Israel"
                              "\n  RU: Russia"
                              "\n  CN: China"
                              "\n  JP: Japan"
                              "\n  KR: Korea",
                              DEFAULT_ZWAVE_RF_REGION);
  status |= config_add_int("zpc.manufacturer_id",
                           "Manufacturer ID (16 bit Decimal)",
                           DEFAULT_MANUFACTURER_ID);
  status |= config_add_int("zpc.product_type",
                           "Product Type (16 bit Decimal)",
                           DEFAULT_PRODUCT_TYPE);
  status |= config_add_int("zpc.product_id",
                           "Product ID (16 bit Decimal)",
                           DEFAULT_PRODUCT_ID);
  status |= config_add_string("zpc.device_id",
                              "Device Specific ID",
                              get_device_id());
  status |= config_add_int("zpc.normal_tx_power_dbm",
                           "Z-Wave normal tx power (deci dBm), The power level"
                           " used when transmitting frames at normal power. The"
                           " power level is in deci dBm. E.g. 1dBm output power"
                           "will be 10 in normal_tx_power_dbm and -2dBm  will "
                           "be -20 in normal_tx_power_dbm. Not all Z-Wave "
                           "modules support this setting and it will be "
                           "applied only with compatible Z-Wave APIs.",
                           DEFAULT_ZWAVE_NORMAL_TX_POWER_DBM);
  status |= config_add_int("zpc.measured_0dbm_power",
                           "Z-Wave measured 0dBm output power (deci dBm). The "
                           " output power measured from the antenna when "
                           "normal_tx_power_dbm is set to 0dBm. The power level"
                           " is in deci dBm. E.g. 1dBm output power will be 10 "
                           "in measured_0dbm_power and -2dBm will be -20 in "
                           "measured_0dbm_power. Calibration value which should"
                           " be adjusted to the physical z-wave module. "
                           "This value can be obtained by measuring the output"
                           " power of the antenna when normal_tx_power_dbm"
                           " Not all Z-Wave modules support this setting and "
                           "it will be applied only with compatible Z-Wave "
                           "APIs.",
                           DEFAULT_ZWAVE_MEASURED_0DBM_POWER);
  status |= config_add_int("zpc.default_wake_up_interval",
                           "Default wake up interval in seconds, which will be "
                           "configured on sleeping (NL) Z-Wave nodes after "
                           "inclusion. Used as default only if there are no"
                           " certification requirements. Also note that if the"
                           "sleeping (NL) Z-Wave device does NOT advertise a "
                           "default Wake Up internal in the Wake Up Command "
                           "Class, this value is used.",
                           DEFAULT_WAKE_UP_INTERVAL);

  status |= config_add_string("zpc.serial_log_file",
                              "If this is set, the ZPC will write a log of the "
                              "communcation over the serial interface with the "
                              "Z-Wave module to the path provided. "
                              "If the file exists, the log will be appended to "
                              "this file, otherwise the file will be created. "
                              "The ZPC will NOT handle log rotation etc.",
                              "");

  status |= config_add_int(
    "zpc.hardware_version",
    "A unique hardware version value that identifies "
    "the hardware version of the device where zpc is running on.",
    DEFAULT_HARDWARE_VERSION);

  status |= config_add_int(
    "zpc.missing_wake_up_notification",
    "This value represents a maximum number of missing wake up periods."
    "If the sleeping nodes missed issuing wake up notification for more than"
    "a value defined here, the node shall be considered as failing node.",
    DEFAULT_NUMBER_OF_MISSING_WAKE_UP_NOTIFICATION);

  status |= config_add_int(
    "zpc.accepted_transmit_failure",
    "This value represents a maximum number of accepted frame transmission "
    "failure."
    "If a frame transmission to a given node is failed more than"
    "a value defined here, the node shall be considered as failing node.",
    DEFAULT_NUMBER_OF_ACCEPTED_FRAME_TRANSMISSION_ERROR);

  status |= config_add_string(
    "zpc.inclusion_protocol_preference",
    "This value represents a prioritized list of protocols to prefer when "
    "including Z-Wave nodes with SmartStart, when the SmartStart list does "
    "not indicate any preference. The protocols are represented as follow:"
    "\n  1: Z-Wave"
    "\n  2: Z-Wave Long Range"
    "\nFor example, '1,2' means Z-Wave, then Z-Wave Long Range. "
    "'2' For Z-Wave Long Range only.",
    DEFAULT_INCLUSION_PROTOCOL_PREFERENCE);

  status |= config_add_string(
    "zpc.ota_cache_path",
    "Path on the filesystem where the ZPC can cache ota image. If this "
    "path does not exists the ZPC will try to create it",
    DEFAULT_OTA_CACHE_PATH);

  status |= config_add_int("zpc.ota_cache_size",
                           "Maximum OTA cache size in kb.",
                           DEFAULT_OTA_CACHE_SIZE);

  return status != CONFIG_STATUS_OK;
}

sl_status_t is_hex_string(const char *str)
{
  int i = 0;
  if (strlen(str) % 2) {
    sl_log_error(LOG_TAG,
                 "Invalid length of zpc.device_id in conf file. "
                 "Expected hexstring (even number of hex digits)");
    return CONFIG_STATUS_ERROR;
  }
  for (i = 0; i < strlen(str); i++) {
    if (!isxdigit(str[i])) {
      sl_log_error(LOG_TAG,
                   "Invalid character %c(%dth) in zpc-device_id in "
                   "conf file. Expected hexstring",
                   str[i],
                   i);
      return CONFIG_STATUS_ERROR;
    }
  }
  return CONFIG_STATUS_OK;
}

static int config_get_int_safe(const char *key)
{
  int val = 0;
  assert(SL_STATUS_OK == config_get_as_int(key, &val));
  return val;
}

sl_status_t zpc_config_fixt_setup()
{
  config_status_t status = CONFIG_STATUS_OK;
  status |= config_get_as_string("zpc.serial", &config.serial_port);
  status |= config_get_as_string("zpc.rf_region", &config.zwave_rf_region);
  config.manufacturer_id = config_get_int_safe("zpc.manufacturer_id");
  config.product_type    = config_get_int_safe("zpc.product_type");
  config.product_id      = config_get_int_safe("zpc.product_id");
  status |= config_get_as_string("zpc.device_id", &config.device_id);
  status |= is_hex_string(config.device_id);
  status |= config_get_as_string(CONFIG_KEY_MQTT_HOST, &config.mqtt_host);
  status |= config_get_as_string(CONFIG_KEY_MQTT_CAFILE, &config.mqtt_cafile);
  status
    |= config_get_as_string(CONFIG_KEY_MQTT_CERTFILE, &config.mqtt_certfile);
  status |= config_get_as_string(CONFIG_KEY_MQTT_KEYFILE, &config.mqtt_keyfile);
  config.mqtt_port = config_get_int_safe(CONFIG_KEY_MQTT_PORT);
  config.zwave_measured_0dbm_power
    = config_get_int_safe("zpc.measured_0dbm_power");
  config.default_wake_up_interval
    = config_get_int_safe("zpc.default_wake_up_interval");
  config.hardware_version = config_get_int_safe("zpc.hardware_version");
  config.accepted_transmit_failure
    = config_get_int_safe("zpc.accepted_transmit_failure");
  config.missing_wake_up_notification
    = config_get_int_safe("zpc.missing_wake_up_notification");

  status |= config_get_as_string("zpc.inclusion_protocol_preference",
                                 &config.inclusion_protocol_preference);
  status
    |= config_get_as_string("zpc.serial_log_file", &config.serial_log_file);

  status |= config_get_as_string("zpc.ota_cache_path", &config.ota_cache_path);
  config.ota_cache_size = config_get_int_safe("zpc.ota_cache_size");

  return status == CONFIG_STATUS_OK ? SL_STATUS_OK : SL_STATUS_FAIL;
}

const zpc_config_t *zpc_get_config()
{
  return &config;
}
