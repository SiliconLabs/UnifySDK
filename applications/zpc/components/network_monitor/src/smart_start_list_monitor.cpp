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
#include "smart_start_list_monitor.h"

// Includes from ZPC components
#include "zwave_controller_callbacks.h"
#include "zpc_attribute_store.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "zpc_converters.h"

// Includes from UIC components
#include "attribute.hpp"
#include "smartstart_management.hpp"
#include "sl_log.h"

constexpr char LOG_TAG[] = "smart_start_list_monitor";

using namespace attribute_store;
using namespace smartstart;

sl_status_t smart_start_list_monitor_reset_step()
{  // Go around in our network, find all UNIDs
  sl_log_info(LOG_TAG,
              "Removing UNIDs and setting ManualInterventionRequired for our "
              "network nodes in the SmartStart list.");

  attribute home_id_node = get_zpc_network_node();
  for (auto node_id_node: home_id_node.children(ATTRIBUTE_NODE_ID)) {
    if (node_id_node == get_zpc_node_id_node()) {
      continue;
    }

    // See if their UNID is in the SmartStart list
    unid_t unid;
    if (SL_STATUS_OK
        != attribute_store_network_helper_get_unid_from_node(node_id_node,
                                                             unid)) {
      sl_log_warning(LOG_TAG,
                     "Cannot compute UNID from the Attribute Store node %d. "
                     "This should not happen.",
                     node_id_node);
      continue;
    }

    // See if SmartStart list has such a UNID:
    sl_status_t removal_status
      = Management::get_instance()->notify_node_removed(std::string(unid));

    if (removal_status != SL_STATUS_OK) {
      continue;
    }

    // If the node was included and just removed, we want to also indicate that
    // it needs to leave its network before it can join our future network.
    attribute dsk_node = node_id_node.child_by_type(ATTRIBUTE_S2_DSK, 0);
    if (!dsk_node.is_valid()) {
      // No DSK information, no luck.
      continue;
    }

    std::vector<uint8_t> dsk_data;
    try {
      dsk_data = dsk_node.reported<std::vector<uint8_t>>();
    } catch (std::exception &e) {
      sl_log_warning(
        LOG_TAG,
        "Failed to read the DSK reported value for attribute %d: %s",
        e.what(),
        dsk_node);
      continue;
    }

    char dsk_string[DSK_STR_LEN] = {};
    if (zpc_converters_dsk_to_str(&dsk_data[0], dsk_string, sizeof(dsk_string))
        != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Failed to convert DSK to string for Attribute node %d",
                   dsk_node);
      attribute_store_log_node(dsk_node, false);
      continue;
    }

    Management::get_instance()->set_manual_intervention_required(
      std::string(dsk_string),
      true);
  }

  // We are done (i.e. no subsequent callbacks),
  // so we return something different than SL_STATUS_OK
  return SL_STATUS_IDLE;
}

////////////////////////////////////////////////////////////////////////////////
// Shared functions
////////////////////////////////////////////////////////////////////////////////
sl_status_t smart_start_list_monitor_init()
{
  sl_log_debug(LOG_TAG, "Initalizing SmartStart list monitor");
  sl_status_t init_status = zwave_controller_register_reset_step(
    &smart_start_list_monitor_reset_step,
    ZWAVE_CONTROLLER_SMART_START_LIST_UPDATE_STEP_PRIORITY);

  return init_status;
}