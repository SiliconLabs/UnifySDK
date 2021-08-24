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
 * @file zigpc_node_state.h
 * @defgroup zigpc_node_state ZigPC Node State Monitor
 * @ingroup zigpc_components
 * @brief The ZigPC Node state monitor component is responsible for publishing
 * the network state of individual nodes and their cluster states serviced by
 * the protocol controller.
 *
 * The following topic is managed:
 * - ucl/by-unid/\<UNID\>/State
 *
 * Managing this topic will include publishing the following parameters:
 * - Node Network Status (see enum zigbee_node_network_status)
 *
 * @{
 */

#ifndef ZIGPC_NODE_STATE_H
#define ZIGPC_NODE_STATE_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Setup fixture for ZigPC Node State Monitor
 *
 * @return SL_SATUS_OK if initialization is successful, error otherwise
 */
sl_status_t zigpc_node_state_setup(void);

/** @brief Shutdown fixture for ZigPC Node State Monitor
 *
 * @return Return Always return 0.
 */
int zigpc_node_state_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* ZIGPC_NODE_STATE_H */

/** @} end zigpc_node_state_process */
