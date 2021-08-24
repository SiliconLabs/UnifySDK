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
 * @file dev_cli_config_fixt.h
 * @addtogroup dev_cli_config
 * @brief dev_cli Config Fixture
 *
 * Fixture to initialize the dev_cli_config from uic_main,
 * which should be used in \ref dev_cli_config.
 *
 * @{
 */

#ifndef DEV_CLI_CONFIG_FIXT_H
#define DEV_CLI_CONFIG_FIXT_H

#include "sl_status.h"
/**
 * @brief Set up the dev_cli_config component.
 *
 * This fixture reads configurations from \ref configuration and store it in
 * dev_cli_config.
 *
 * @return SL_STATUS_OK for success, SL_STATUS_FAIL if an error occurs
 */
sl_status_t dev_cli_config_fixt_setup(void);

/** @} end of dev_cli_config */
#endif  // DEV_CLI_CONFIG_FIXT_H
