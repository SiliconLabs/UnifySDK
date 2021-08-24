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

#ifndef ZIGPC_NODE_STATE_INT_H
#define ZIGPC_NODE_STATE_INT_H

#include "sl_status.h"

#include "zigpc_common_zigbee.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  enum zigbee_node_network_status status;
  zigbee_eui64_t eui64;
} zigpc_node_state_status_change_t;

/**
 * @brief Setup observers to receive events for Node Management.
 *
 * @return sl_status_t SL_STATUS_OK on success, error registering
 * observers if not.
 */
sl_status_t zigpc_node_state_init_observers(void);

/**
 * @brief Process handler for node network status change.
 *
 * @param state_change Node status change information.
 * @return sl_status_t SL_STATUS_OK on success, error if not.
 */

sl_status_t zigpc_node_state_process_event_node_status_change(
  const zigpc_node_state_status_change_t *state_change);

#ifdef __cplusplus
}
#endif

#endif /* ZIGPC_NODE_STATE_INT_H */
