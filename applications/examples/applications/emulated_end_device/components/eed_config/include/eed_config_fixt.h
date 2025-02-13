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
 * @defgroup eed_config_fixture EED Config Fixture
 * @ingroup eed_config
 * @brief EED Configuration fixture.
 *
 * Fixture to initialize the eed_config from uic_main,
 * to be used in @ref eed_config
 *
 * @{
 */

#ifndef EED_CONFIG_FIXT_H
#define EED_CONFIG_FIXT_H

#include <stdbool.h>
#include "sl_status.h"
/**
 * @brief Fixture for setting up the eed_config component.
 *
 * This will read configurations from \ref config, and store it in eed_config.
 *
 * @return SL_STATUS_OK for success, SL_STATUS_FAIL if an error occurred
 */
sl_status_t eed_config_fixt_setup(void);

/** @} end of eed_config */
#endif  // EED_CONFIG_FIXT_H
