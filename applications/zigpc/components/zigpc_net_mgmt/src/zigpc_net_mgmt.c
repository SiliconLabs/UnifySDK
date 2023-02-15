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
#include <stdio.h>

#include <sys/ctimer.h>
// Shared Unify includes
#include <sl_status.h>
#include <sl_log.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>
#include <zigpc_gateway.h>

// Component includes
#include "zigpc_net_mgmt.h"
#include "zigpc_net_mgmt_internal.h"
#include "zigpc_net_mgmt_notify_int.h"

#include "zigpc_net_mgmt_process_send.h"

static const char LOG_TAG[] = "zigpc_net_mgmt";


/**
 * @brief zigpc_netmgmt_permit_timer_expiry
 * Callback for the timer used to timeout a device add request
 * Automatically closes the network to joins
 *
 * @param ptr - unused pointer
 *
 * */
static void zigpc_netmgmt_permit_timer_expiry(void *ptr)
{
    //always close the network
    sl_log_info(LOG_TAG, "Timer expired for opening the network");
    (void)zigpc_gateway_network_permit_joins(false);
    
    enum zigpc_net_mgmt_fsm_state next_supported_states[] = 
    {
        ZIGPC_NET_MGMT_FSM_STATE_IDLE,
        ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD,
        ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE,
        ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL
    };

    zigpc_net_mgmt_notify_state_update(ZIGPC_NET_MGMT_FSM_STATE_IDLE, next_supported_states);
}

static struct ctimer permit_joins_timer;
static const unsigned long permit_joins_timeout_period = 5*60*CLOCK_SECOND; //5 minutes


sl_status_t zigpc_netmgmt_network_permit_joins(bool enable)
{
  sl_status_t status = 
      zigpc_gateway_network_permit_joins(enable);

  int timer_status = ctimer_expired(&permit_joins_timer);
    
  if(timer_status == 0)
  {
      ctimer_stop(&permit_joins_timer);
  }


  if(enable)
  {
    ctimer_set(
        &permit_joins_timer,
        permit_joins_timeout_period,
        zigpc_netmgmt_permit_timer_expiry,
        NULL);
  }

  return status;
}

sl_status_t zigpc_net_mgmt_add_node(const zigbee_eui64_t eui64,
                                    const zigbee_install_code_t install_code,
                                    const uint8_t install_code_length)
{
  sl_status_t status = SL_STATUS_OK;
  if (eui64 == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
  
    status = 
        zigpc_net_mgmt_hdl_idle_add_request( 
            eui64,
            install_code,
            install_code_length);
  }


  return status;
}

sl_status_t zigpc_net_mgmt_remove_node(const zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;
  status = zigpc_net_mgmt_hdl_idle_remove_request(eui64);

  return status;
}

sl_status_t zigpc_net_mgmt_state_change_request(
  const zigpc_net_mgmt_state_change_request_t *action)
{
  
    sl_status_t status = SL_STATUS_OK;
    
    if(action == NULL)
    {
        status = SL_STATUS_NULL_POINTER;
        return status;
    }
    
    zigpc_net_mgmt_fsm_state_t state =
        action->requested_state;
  
    switch(state)
    {
    //DO IDLE
        case ZIGPC_NET_MGMT_FSM_STATE_IDLE:
            status = zigpc_netmgmt_network_permit_joins(false);
            break;
    //DO ADD
        case ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD:
            //TODO Enable this when install codes are properly added to the network management MQTT
            /*
            if(action->param_eui64_filled && (action->param_install_code != NULL))
            {
                status = zigpc_net_mgmt_add_node(
                            action->param_eui64,
                            action->param_install_code,
                            action->param_install_code_length);
            }*/
            
            status = zigpc_netmgmt_network_permit_joins(true);

            break;
        case ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE:
    //DO REMOVE 
            if(action->param_eui64_filled)
            {
                status = zigpc_net_mgmt_remove_node(action->param_eui64);
            }
            else
            {
                sl_log_warning(LOG_TAG, "Unable to remove node as no EUI64 supplied");
            }
            break;
    //HANDLE UNSUPPORTED 
        default:
            break;
    }
  return status;
}
