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
 * @defgroup aoxpc_config
 * @brief Component initializing all AoXPC parameters.
 *
 * This component register all configuration keys and default values specific
 * to the AoXPC to the uic_config module.
 *
 * @{
 */

#ifndef AOXPC_CONFIG_H
#define AOXPC_CONFIG_H

#include "config.h"
#include "sl_status.h"

// Configuration keys for the AoXPC
#define CONFIG_KEY_AOXPC_DATASTORE_FILE "aoxpc.datastore_file"
#define CONFIG_KEY_AOXPC_CONFIG_FILE    "aoxpc.config"
#define CONFIG_KEY_AOXPC_NCP_SERIAL     "aoxpc.ncp.serial"
#define CONFIG_KEY_AOXPC_NCP_BAUD_RATE  "aoxpc.ncp.baud_rate"
#define CONFIG_KEY_AOXPC_NCP_TPC        "aoxpc.ncp.tcp"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize all AoXPC parameters
 *
 * This will read configurations from \ref config, and store it in aoxpc_config.
 *
 * @return SL_STATUS_OK for success, SL_STATUS_FAIL if an error occurred
 */
sl_status_t aoxpc_config_init();


#ifdef __cplusplus
}
#endif

#endif  //AOXPC_CONFIG_H
/** @} end aoxpc_config */
