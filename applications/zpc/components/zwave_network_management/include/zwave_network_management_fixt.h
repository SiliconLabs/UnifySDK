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

#ifndef ZWAVE_NETWORK_MANAGEMENT_FIXT_H
#define ZWAVE_NETWORK_MANAGEMENT_FIXT_H

#include <stdbool.h>
#include "sl_status.h"

/**
 * @defgroup zwave_network_management_fixt Z-Wave Network Management Fixture
 * @ingroup zwave_network_management
 * @brief Z-Wave Network Management Fixture
 *
 * This sub-component handles initializes all required resources for
 * Z-Wave network management.
 *
 * The @ref zwave_network_management_fixt requires the following to be initialized before it can properly be initialized itself:
 * - @ref ZWAPI
 * - @ref zwave_rx
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Z-Wave Network Management Module.
 *
 * This function must be called before any network management operation is
 * started.
 * @returns SL_STATUS_OK on success SL_STATUS_FAIL otherwise
 */
sl_status_t zwave_network_management_fixt_setup(void);

/**
 * @brief Take down fixture for Z-Wave network Management.
 *
 * @returns This always return 0.
 */
int zwave_network_management_fixt_teardown(void);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_NETWORK_MANAGEMENT_FIXT_H
/** @} end zwave_network_management_fixt */
