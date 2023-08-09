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
 * @defgroup zpc_node_state ZPC Node State
 * @brief Support for Z-Wave specific command for the State topis like discover neighbors and remove offline
 *
 *
 * @{
 */

#ifndef ZPC_NODE_STATE_H
#define ZPC_NODE_STATE_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the component
 *
 * @return sl_status_t
 */
sl_status_t zpc_node_state_init();

#ifdef __cplusplus
}
#endif

#endif  //ZPC_NODE_STATE_H
/** @} end zpc_node_state */
