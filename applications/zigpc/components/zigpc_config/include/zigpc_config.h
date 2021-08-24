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
 * @file zigpc_config.h
 * @addtogroup zigpc_config ZigPC Configuration system
 * @ingroup zigpc_components
 *
 * @brief This component is the source of the configuration parameters passed
 * through command line arguments and/or the configuration file.
 *
 * The after initialization the configuration parameters are constant.
 *
 * @{
 */

#if !defined(ZIGPC_CONFIG_H)
#define ZIGPC_CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reference to options used by ZigPC.
 *
 */
typedef struct {
  const char *mqtt_host;       ///< Hostname of the MQTT broker
  int mqtt_port;               ///< port of the MQTT broker
  const char *serial_port;     ///< Name of the serial port of the Z-Wave module
  const char *datastore_file;  ///< Name of the serial port of the Z-Wave module
} zigpc_config_t;

/**
 * @brief Get the current configuration. This must only be called after
 * zigpc_config_init.
 */
const zigpc_config_t *zigpc_get_config(void);

/**
 * @brief Initialize the configuration module.
 *
 * This must be called before \ref uic_main.
 *
 * @returns 0 on success.
 */
int zigpc_config_init();

#ifdef __cplusplus
}
#endif

/** @} end zigpc_config */

#endif  // ZIGPC_CONFIG_H
