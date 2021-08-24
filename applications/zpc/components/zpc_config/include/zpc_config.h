/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @file zpc_config.h
 * @addtogroup zpc_config Configuration Extension
 * @ingroup components
 *
 * @brief Add the ZPC-specific fixtures to the Unify \ref config system.
 *
 * This module is the source of all ZPC-specific configuration parameters.
 * The source of the configuration parameters are command line arguments
 * and the configuration file.
 *
 * The after initialization the configurations parameters are constant.
 *
 * @{
 */

#if !defined(ZPC_CONFIG_H)
#define ZPC_CONFIG_H

#include <stdint.h>

// Inclusion protocol preference values
/// Z-Wave
#define ZWAVE_CONFIG_REPRESENTATION "1"
/// Z-Wave Long Range
#define ZWAVE_LONG_RANGE_CONFIG_REPRESENTATION "2"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  const char *mqtt_host;  ///< Hostname of the MQTT broker;
  const char *
    mqtt_cafile;  ///< path to a file containing the PEM encoded trusted CA certificate files.
  const char *
    mqtt_certfile;  ///< path to a file containing the PEM encoded certificate file for this client.
  const char *
    mqtt_keyfile;  /// < path to a file containing the PEM encoded unencrypted private key for this client.
  const char *
    mqtt_client_id;  ///< Client ID for MQTT Client for TLS Authentication and encryption
  const char *
    mqtt_client_psk;  ///< Pre shared Key for MQTT Client for TLS Authentication and encryption
  int mqtt_port;            ///< port of the MQTT broker
  const char *serial_port;  ///< Name of the serial port of the Z-Wave module
  const char *serial_log_file;  ///< If set the Serial log will be written here
  /// Z-Wave RF Region setting. It can be one of the following:
  // "EU", "US","ANZ", HK", "IN", "IL", "RU", "CN", "JP", "KR"
  const char *zwave_rf_region;
  /// Transmit power for the Z-Wave module. Refer to \ref zwave_rx_init
  int zwave_normal_tx_power_dbm;
  /// Measured 0dBm output power for the Z-Wave module.
  /// Refer to \ref zwave_rx_init
  int zwave_measured_0dbm_power;
  ///< Default wakeup interval to be used when including wakeup nodes
  int default_wakeup_interval;
  uint16_t manufacturer_id;
  uint16_t product_type;
  uint16_t product_id;
  const char *device_id;
  ///< hardware version of the device where ZPC is running on.
  int hardware_version;
  ///< This value represents a maximum number of missing wakeup periods.
  uint8_t missing_wakeup_notification;
  ///< This value represents a maximum number of accepted frame transmission failure.
  uint8_t accepted_transmit_failure;
  ///< Prioritized list of protocols to use for SmartStart inclusions
  const char *inclusion_protocol_preference;
  ///< OTA cache path, writable location where we can cache OTA images
  const char *ota_cache_path;
  ///< OTA cache size in kb
  int ota_cache_size;
} zpc_config_t;

/**
 * @brief Get the current configuration. This must only be called after
 * zpc_config_init.
 */
const zpc_config_t *zpc_get_config();

/**
 * @brief Register ZPC configurations in \ref config.
 *
 * This must be called before \ref uic_main.
 *
 * @returns 0 on success.
 */
int zpc_config_init();

#ifdef __cplusplus
}
#endif

/** @} end zpc_config */

#endif  // ZPC_CONFIG_H
