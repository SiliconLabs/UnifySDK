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

#ifndef ZIGPC_PROTCTRL_INT_H
#define ZIGPC_PROTCTRL_INT_H

#include "process.h"
#include "sl_status.h"

#include "zigpc_common_ucl.h"
#include "zigpc_common_unid.h"

#include "zigpc_net_mgmt.h"
#include "zigpc_net_mgmt_notify.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  zigbee_eui64_t pc_eui64;
  char pc_eui64_str[ZIGBEE_EUI64_HEX_STR_LENGTH];
} zigpc_protctrl_state_t;

/**
 * @brief Protocol Controller State
 *
 */
zigpc_protctrl_state_t protctrl_state;

/**
 * @brief Process handler for Network Management initialization.
 *
 * This will allow the component to get information needed to operate
 * as a Protocol Controller monitor (such as the PC UNID)
 *
 * @param data Network Initialization data
 * @return sl_status_t SL_STATUS_OK on success, error if not.
 */

sl_status_t zigpc_protctrl_process_event_net_init(
  const struct zigpc_net_mgmt_on_network_init *data);

/**
 * @brief Process handler for Network Management FSM state updates.
 *
 * This handler will provide any information from the Network Management
 * FSM to the MQTT topic under "ProtocolController/NetworkManagement"
 *
 * NOTE: The requested parameters list of character arrays is assumed to be
 * dynamically allocated. Therefore this function deallocates each of these
 * character arrays (provided that the requested parameter list count is
 * consistent).
 *
 * @param data Network Management FSM state change information
 * @return sl_status_t SL_STATUS_OK on success, error if not.
 */
sl_status_t zigpc_protctrl_process_event_net_state_update(
  const struct zigpc_net_mgmt_on_network_state_update *data);

/**
 * @brief Setup observers to receive event updated from Network
 * Managment
 *
 * @return sl_status_t SL_STATUS_OK on success, error registering
 * observers if not
 */
sl_status_t zigpc_protctrl_init_net_mgmt_observers(void);

#ifdef __cplusplus
}
#endif

#endif /* ZIGPC_PROTCTRL_INT_H */
