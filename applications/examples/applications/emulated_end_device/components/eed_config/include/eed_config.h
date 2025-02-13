/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup eed_config EED Configuration Extension
 * @ingroup eed_components
 *
 * @brief Add the EED-specific fixtures to the Unify \ref config system.
 *
 * This module is the source of all EED-specific configuration parameters.
 * The source of the configuration parameters are command line arguments
 * and the configuration file.
 *
 * The after initialization the configurations parameters are constant.
 *
 * @{
 */

#ifndef EED_CONFIG_H
#define EED_CONFIG_H

// Unify includes
#include "uic_version.h"

// Default setting for the eed.datastore_file.
#define DEFAULT_EED_DATASTORE_FILE UIC_VAR_DIR "/eed.db"

// Default setting for eed supported cluster list.
#define DEFAULT_EED_CLUSTER_LIST ""

// Default setting for eed supported deivce type(s).
#define DEFAULT_EED_DEVICE_TYPE ""

// Config key for the EED datastore file
#define CONFIG_KEY_EED_DATASTORE_FILE "eed.datastore_file"

// Config key for the EED cluster list
#define CONFIG_KEY_EED_CLUSTER_LIST "eed.cluster_list"

// Config key for the EED device type(s)
#define CONFIG_KEY_EED_DEVICE_TYPE "eed.device_type"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  /// Hostname of the MQTT broker
  const char *mqtt_host;
  /// Path to a file containing the PEM encoded trusted CA certificate files.
  const char *mqtt_cafile;
  /// Path to a file containing the PEM encoded certificate file for this client.
  const char *mqtt_certfile;
  /// Path to a file containing the PEM encoded unencrypted private key for this client.
  const char *mqtt_keyfile;
  /// Client ID for MQTT Client for TLS Authentication and encryption
  const char *mqtt_client_id;
  /// Pre shared Key for MQTT Client for TLS Authentication and encryption
  const char *mqtt_client_psk;
  /// Port of the MQTT broker
  int mqtt_port;
  /// File name for datastore/persistent storage
  const char *datastore_file;
  /// String to take supported cluster list
  const char *cluster_list;
  /// String to take supported device types
  const char *device_type;
} eed_config_t;

/**
 * @brief Get the current configuration. This must only be called after
 * eed_config_init.
 */
const eed_config_t *eed_get_config();

/**
 * @brief Register EED configurations in \ref config.
 *
 * This must be called before \ref uic_main.
 *
 * @returns 0 on success.
 */
int eed_config_init();

#ifdef __cplusplus
}
#endif

/** @} end eed_config */

#endif  // EED_CONFIG_H
