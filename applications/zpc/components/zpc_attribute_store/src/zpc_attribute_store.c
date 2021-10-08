/* <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
// Includes from this component
#include "zpc_attribute_store.h"
#include "attribute_store_defined_attribute_types.h"
#include "zpc_attribute_store_debug.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "zwave_network_management.h"

#include "sl_log.h"
#define LOG_TAG "zpc_attribute_store"

////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////
bool is_zpc_unid(const unid_t unid)
{
  zwave_node_id_t my_node_id = zwave_network_management_get_node_id();
  unid_t my_unid;
  zwave_unid_from_node_id(my_node_id, my_unid);

  if (strcmp(unid, my_unid) == 0) {
    return true;
  } else {
    return false;
  }
}

attribute_store_node_t get_zpc_network_node()
{
  zwave_home_id_t my_home_id = zwave_network_management_get_home_id();

  return attribute_store_get_node_child_by_value(attribute_store_get_root(),
                                                 ATTRIBUTE_HOME_ID,
                                                 REPORTED_ATTRIBUTE,
                                                 (uint8_t *)&my_home_id,
                                                 sizeof(zwave_home_id_t),
                                                 0);
}

attribute_store_node_t get_zpc_node_id_node()
{
  zwave_node_id_t my_node_id = zwave_network_management_get_node_id();

  return attribute_store_get_node_child_by_value(get_zpc_network_node(),
                                                 ATTRIBUTE_NODE_ID,
                                                 REPORTED_ATTRIBUTE,
                                                 (uint8_t *)&my_node_id,
                                                 sizeof(zwave_node_id_t),
                                                 0);
}

attribute_store_node_t get_zpc_endpoint_id_node(zwave_endpoint_id_t endpoint_id)
{
  return attribute_store_get_node_child_by_value(get_zpc_node_id_node(),
                                                 ATTRIBUTE_ENDPOINT_ID,
                                                 REPORTED_ATTRIBUTE,
                                                 (uint8_t *)&endpoint_id,
                                                 sizeof(zwave_endpoint_id_t),
                                                 0);
}

///////////////////////////////////////////////////////////////////////////////
// Initialization specifically for ZPC attribute_store
///////////////////////////////////////////////////////////////////////////////
static sl_status_t invoke_update_callbacks_in_network()
{
  attribute_store_node_t home_id = get_zpc_network_node();
  sl_status_t refresh
    = attribute_store_refresh_node_and_children_callbacks(home_id);

  if (refresh != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Failed to trigger a refresh callback for attribute Store "
                 "node %d. This node should represent our HomeID");
    return refresh;
  }

  sl_log_info(
    LOG_TAG,
    "Refreshed callbacks on Attribute Node %d, representing our current HomeID",
    home_id);
  return refresh;
}

sl_status_t zpc_attribute_store_init()
{
  zpc_attribute_store_debug_init();
  return invoke_update_callbacks_in_network();
}
