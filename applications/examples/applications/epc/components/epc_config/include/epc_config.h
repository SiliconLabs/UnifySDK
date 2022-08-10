/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup epc_config EPC Configuration Extension
 * @ingroup epc_components
 *
 * @brief Add the EPC-specific fixtures to the Unify \ref config system.
 *
 * This module is the source of all EPC-specific configuration parameters.
 * The source of the configuration parameters are command line arguments
 * and the configuration file.
 *
 * The after initialization the configurations parameters are constant.
 *
 * @{
 */

#ifndef EPC_CONFIG_H
#define EPC_CONFIG_H

// Unify includes
#include "uic_version.h"

// Default setting for the epc.datastore_file.
#define DEFAULT_EPC_DATASTORE_FILE UIC_VAR_DIR "/epc.db"

// Config key for the EPC datastore file
#define CONFIG_KEY_EPC_DATASTORE_FILE "epc.datastore_file"

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
} epc_config_t;

/**
 * @brief Get the current configuration. This must only be called after
 * epc_config_init.
 */
const epc_config_t *epc_get_config();

/**
 * @brief Register EPC configurations in \ref config.
 *
 * This must be called before \ref uic_main.
 *
 * @returns 0 on success.
 */
int epc_config_init();

#ifdef __cplusplus
}
#endif

/** @} end epc_config */

#endif  // EPC_CONFIG_H
