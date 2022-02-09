/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file
 * @brief Positioning core.
 */

#ifndef POSITIONING_H
#define POSITIONING_H

#include "sl_status.h"

/**
 * @brief Initialize configuration settings for Positioning core.
 *
 * @return SL_STATUS_OK on success
 */
sl_status_t positioning_config_init(void);

/**
 * @brief Fixture for setting up Positioning core.
 *
 * @return SL_STATUS_OK on success
 */
sl_status_t positioning_fixt_setup(void);

#endif // POSITIONING_H
