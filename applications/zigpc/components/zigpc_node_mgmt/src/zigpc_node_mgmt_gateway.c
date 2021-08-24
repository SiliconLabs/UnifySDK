/*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "sl_log.h"
#include "sl_status.h"

#include "zigpc_gateway_notify.h"

#include "zigpc_node_mgmt_int.h"
#include "zigpc_node_mgmt.h"

static const char LOG_TAG[] = "zigpc_node_mgmt_gateway";

sl_status_t zigpc_node_mgmt_init_observers(void)
{
  sl_status_t result = SL_STATUS_OK;

  if (result == SL_STATUS_OK) {
    result
      = zigpc_gateway_register_observer(ZIGPC_GATEWAY_NOTIFY_NODE_ADD_COMPLETE,
                                        zigpc_node_mgmt_on_node_add_complete);
  }

  if (result == SL_STATUS_OK) {
    result = zigpc_gateway_register_observer(
      ZIGPC_GATEWAY_NOTIFY_NODE_ENDPOINT_DISCOVERED,
      zigpc_node_mgmt_on_endpoint_discovered);
  }

  return result;
}

void zigpc_node_mgmt_on_endpoint_discovered(void *event_data)
{
  const zigpc_gateway_on_node_endpoint_discovered_t *ep_disc
    = (zigpc_gateway_on_node_endpoint_discovered_t *)event_data;

  sl_status_t result
    = zigpc_nodemgmt_add_node_endpoint(ep_disc->eui64, ep_disc->endpoint);

  if (result != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "Error managing an added endpoint");
  }
}

void zigpc_node_mgmt_on_node_add_complete(void *event_data)
{
  const struct zigpc_gateway_on_node_add *node_add
    = (struct zigpc_gateway_on_node_add *)event_data;

  sl_status_t result = zigpc_nodemgmt_manage_node_eui64(node_add->eui64);

  if (result != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "Error managing an added node");
  }
}
