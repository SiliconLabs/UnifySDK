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

#ifndef ZWAVE_TRANSPORTS_FIXT_H
#define ZWAVE_TRANSPORTS_FIXT_H

// Generic includes
#include <stdbool.h>
#include "sl_status.h"

/**
 * @defgroup zwave_transports Z-Wave Transports
 * @ingroup components
 * @brief Encapsulation/Decapsulation Z-Wave Command Class services
 *
 */

/**
 * @defgroup zwave_transports_fixt Transport fixture
 * @ingroup zwave_transports
 * @brief Initializations for Z-Wave Transports
 *
 * The Z-Wave Transport fixture provides an initialization
 * functions allowing to initialize all Z-Wave Transports.
 *
 * Transports are registered to the Z-Wave Controller using
 * the \ref zwave_controller_transport_register function.
 *
 * @{
 */

/**
 * @brief Setup fixture for @ref zwave_transports.
 *
 * This setup function will call the setup/initialization functions for
 * every Z-Wave Transport in this module.
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_transports_init(void);

/** @} end zwave_transports_fixt */

#endif  // ZWAVE_TRANSPORTS_FIXT_H
