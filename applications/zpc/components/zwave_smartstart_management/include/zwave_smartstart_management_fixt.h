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
 * @defgroup zwave_smartstart_management_fixt Z-Wave SmartStart Management Fixture
 * @ingroup zwave_smartstart_management
 * @brief Fixtures for Z-Wave SmarStart Management
 *
 * @{
 */

#ifndef ZWAVE_SMARTSTART_MANAGEMENT_FIXT_H
#define ZWAVE_SMARTSTART_MANAGEMENT_FIXT_H
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Z-Wave SmartStart Management Module.
 *
 * Calls zwave_controller_register_callbacks() to set following callbacks
 *  .on_node_deleted
 *  .on_node_added
 *  .on_smart_start_inclusion_request
 * and then starts the smartstart_management_process Contiki process and
 * notifies to the process that there are some SmartStart entries awaiting
 * inclusion.
 * @returns SL_STATUS_OK on success, other codes in case of error
 */

sl_status_t zwave_smartstart_management_setup_fixt(void);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_SMARTSTART_MANAGEMENT_FIXT_H
/** @} end zwave_smartstart_management_fixt */

