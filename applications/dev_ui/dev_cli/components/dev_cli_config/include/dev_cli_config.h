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
 * @file dev_cli_config.h
 * @addtogroup dev_cli_config Configuration system
 * @ingroup components
 *
 * @brief Configuration system for DEV_CLI
 *
 * This module is the source of all configuration parameters of DEV_CLI.
 * The source of the configuration parameters are command line arguments
 * and the configuration file.
 *
 * After initialization, the configurations parameters are constant.
 *
 * @{
 */

#if !defined(DEV_CLI_CONFIG_H)
#define DEV_CLI_CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  const char *mqtt_host;  ///< Hostname of the MQTT broker;
  int mqtt_port;          ///< port of the MQTT broker
  const char *command;
  const char *dsk;      ///< DSK key to use for SmartStart list manipulation
  const char *include;  ///< Include to use for SmartStart list manipulation
  const char *protocolunid;  ///< protcol controller unid to use for SmartStart
                             ///< list manipulation
  const char *unid;          ///< unid to use for SmartStart list manipulation
} dev_cli_config_t;

/**
 * @brief Get the current configuration. This function must
 * only be called after dev_cli_config_init.
 */
const dev_cli_config_t *dev_cli_get_config(void);

/**
 * @brief Initialize the configuration module.
 *
 * This function must be called before \ref uic_main.
 *
 * @returns 0 on success.
 */
int dev_cli_config_init();

#ifdef __cplusplus
}
#endif

/** @} end dev_cli_config */

#endif  // dev_cli_CONFIG_H
