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

#include <string.h>
#include <malloc.h>

/* Contiki includes */
#include "sl_status.h"
#include "sl_log.h"

#include "zigpc_protctrl_int.h"
#include "zigpc_protctrl_process_send.h"

static const char LOG_TAG[] = "zigpc_protctrl_observers";

/**
 * @brief Event handler for Net Mgmt init
 *
 * @param event_data event data containing network information
 */
void zigpc_protctrl_observer_net_mgmt_on_net_init(void *event_data)
{
  static struct zigpc_net_mgmt_on_network_init process_data = {0};

  const struct zigpc_net_mgmt_on_network_init *net_init
    = (struct zigpc_net_mgmt_on_network_init *)event_data;

  memcpy(process_data.pc_eui64, net_init->pc_eui64, sizeof(zigbee_eui64_t));

  zigpc_protctrl_process_send_event(ZIGPC_PROTCTRL_EVENT_NET_INIT,
                                    (void *)&process_data);
}

/**
 * @brief Event handler for Net Mgmt state updates
 *
 * @param event_data event data containing new state information
 */
void zigpc_protctrl_observer_net_mgmt_on_state_update(void *event_data)
{
  static struct zigpc_net_mgmt_on_network_state_update process_data = {0};

  const struct zigpc_net_mgmt_on_network_state_update *state_update
    = (struct zigpc_net_mgmt_on_network_state_update *)event_data;

  process_data.new_state = state_update->new_state;

  process_data.next_supported_states_count
    = state_update->next_supported_states_count;

  memcpy(process_data.next_supported_states_list,
         state_update->next_supported_states_list,
         sizeof(enum zigpc_net_mgmt_fsm_state)
           * process_data.next_supported_states_count);

  process_data.requested_state_parameter_count
    = state_update->requested_state_parameter_count;

  for (uint8_t i = 0; i < process_data.requested_state_parameter_count; i++) {
    process_data.requested_state_parameter_list[i]
      = state_update->requested_state_parameter_list[i];
  }

  zigpc_protctrl_process_send_event(ZIGPC_PROTCTRL_EVENT_NET_STATE_UPDATE,
                                    (void *)&process_data);
}

sl_status_t zigpc_protctrl_init_net_mgmt_observers(void)
{
  sl_status_t status;

  status = zigpc_net_mgmt_register_observer(
    ZIGPC_NET_MGMT_NOTIFY_NETWORK_INIT,
    zigpc_protctrl_observer_net_mgmt_on_net_init);

  if (status == SL_STATUS_OK) {
    status = zigpc_net_mgmt_register_observer(
      ZIGPC_NET_MGMT_NOTIFY_STATE_UPDATE,
      zigpc_protctrl_observer_net_mgmt_on_state_update);
  }

  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Network management observer registrations failed: 0x%X",
                 status);
  }

  return status;
}
