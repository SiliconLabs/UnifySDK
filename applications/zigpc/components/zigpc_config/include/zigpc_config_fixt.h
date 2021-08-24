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
 * @file zigpc_config_fixt.h
 * @addtogroup zigpc_config
 * @brief ZIGPC Config Fixture
 *
 * Fixture to initialize the zigpc_config from uic_main,
 * to be used in \ref zigpc_config
 *
 * @{
 */

#ifndef ZIGPC_CONFIG_FIXT_H
#define ZIGPC_CONFIG_FIXT_H

#include "sl_status.h"
/**
 * @brief Fixture for setting up the zigpc_config component
 *
 * This will read configurations from \ref config, and store it in zigpc_config.
 *
 * @return SL_STATUS_OK for success, SL_STATUS_FAIL if an error occurred
 */
sl_status_t zigpc_config_fixt_setup(void);

/** @} end of zigpc_config */
#endif  // ZIGPC_CONFIG_FIXT_H
