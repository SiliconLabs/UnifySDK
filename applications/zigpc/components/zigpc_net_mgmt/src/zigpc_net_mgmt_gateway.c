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

#include <stdlib.h>
#include <string.h>

#include <process.h>
#include <sl_log.h>
#include <sl_status.h>

#include <zigpc_discovery.h>
#include "zigpc_net_mgmt_internal.h"
#include "zigpc_net_mgmt_gateway.h"
#include "zigpc_net_mgmt_process.h"
#include "zigpc_net_mgmt_process_send.h"

#include "zigpc_common_observable.h"

#include "zigpc_gateway_notify.h"

static const char LOG_TAG[] = "zigpc_net_mgmt_gateway";

void zigpc_net_mgmt_callback_on_network_initialized(void *event_data)
{
  const zigpc_gateway_on_network_init_t *net_init
    = (zigpc_gateway_on_network_init_t *)event_data;

    sl_status_t status  =
        zigpc_net_mgmt_init_complete(
            net_init->zigpc_eui64,
            net_init->zigpc_endpoint_id,
            net_init->zigpc_panid,
            net_init->zigpc_ext_panid,
            net_init->zigpc_radio_channel);

    if (status != SL_STATUS_OK) 
    {
        sl_log_warning(
                LOG_TAG,
                 "Network Initialization failed with status: 0x%X",
                 status);
    }

}

void zigpc_net_mgmt_callback_on_node_add_start(void *event_data)
{
  
    sl_log_info(LOG_TAG, "Device announce");
    
    const struct zigpc_gateway_on_node_add *node_add
        = (struct zigpc_gateway_on_node_add *)event_data;

    sl_status_t status = zigpc_net_mgmt_node_add_complete(node_add->eui64);
    
    if (status != SL_STATUS_OK) 
    {
        sl_log_warning(
                LOG_TAG,
                 "Node add failed with status: 0x%X",
                 status);
    }

}

void zigpc_net_mgmt_callback_on_node_add_complete(void *event_data)
{
    (void) event_data;
    sl_log_info(LOG_TAG, "Trust center complete");
}

void zigpc_net_mgmt_on_discovery_status(
  zigbee_eui64_uint_t eui64_uint, zigpc_discovery_status_t discovery_status)
{
    zigbee_eui64_t eui64;
    zigbee_uint_to_eui64(eui64_uint, eui64);

    sl_status_t status =
        zigpc_net_mgmt_publish_interview_status(
            eui64,
            discovery_status);
    
    if (status != SL_STATUS_OK) 
    {
        sl_log_warning(
                LOG_TAG,
                 "Node interview failed with status: 0x%X",
                 status);
    }
}

void zigpc_net_mgmt_callback_on_node_removed(void *event_data)
{

  const zigpc_gateway_on_node_removed_t *node_removed
    = (zigpc_gateway_on_node_removed_t *)event_data;

    sl_status_t status = 
        zigpc_net_mgmt_node_remove_complete(
            node_removed->eui64);
    
    if (status != SL_STATUS_OK) 
    {
        sl_log_warning(
                LOG_TAG,
                 "Node remove failed with status: 0x%X",
                 status);
    }
}

sl_status_t zigpc_net_mgmt_gateway_init_observer(void)
{
  sl_status_t result;

  result = zigpc_gateway_register_observer(
    ZIGPC_GATEWAY_NOTIFY_NETWORK_INIT,
    zigpc_net_mgmt_callback_on_network_initialized);

  if (result == SL_STATUS_OK) {
    result = zigpc_gateway_register_observer(
      ZIGPC_GATEWAY_NOTIFY_NODE_ADD_START,
      zigpc_net_mgmt_callback_on_node_add_start);
  }

  if (result == SL_STATUS_OK) {
    result = zigpc_gateway_register_observer(
      ZIGPC_GATEWAY_NOTIFY_NODE_ADD_COMPLETE,
      zigpc_net_mgmt_callback_on_node_add_complete);
  }

  if (result == SL_STATUS_OK) {
    result = zigpc_discovery_add_listener(zigpc_net_mgmt_on_discovery_status);
  }

  if (result == SL_STATUS_OK) {
    result = zigpc_gateway_register_observer(
      ZIGPC_GATEWAY_NOTIFY_NODE_REMOVED,
      zigpc_net_mgmt_callback_on_node_removed);
  }

  if (result != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Gateway observer registrations failed: 0x%X",
                 result);
  }

  return result;
}
