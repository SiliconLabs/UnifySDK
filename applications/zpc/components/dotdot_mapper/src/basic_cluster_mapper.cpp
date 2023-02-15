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
#include "basic_cluster_mapper.h"

// ZPC includes
#include "zwave_utils.h"
#include "zwave_network_management.h"
#include "attribute_store_defined_attribute_types.h"
#include "zwave_COMMAND_CLASS_MANUFACTURER_SPECIFIC_attribute_id.h"
#include "zpc_attribute_store_network_helper.h"

// Unify component includes
#include "attribute.hpp"
#include "attribute_store_helper.h"
#include "sl_log.h"
#include "ucl_definitions.h"
#include "unify_dotdot_defined_attribute_types.h"
#include <sstream>
#include <iomanip>
// Interfaces
#include "zwave_command_class_version_types.h"
constexpr char LOG_TAG[] = "basic_cluster_mapper";
using namespace attribute_store;

// Manufacturer name data
#include "manufacturer_id_name_list.inc"

/**
 * @brief Set the Basic Cluster serial number attribute
 *        if Device Specific Device Data attribute presents
 *        under endpoint attribute on a Z-Wave domain
 * @param endpoint_id Attribute Store node for endpoint.
 */
static void set_basic_cluster_serial_num(attribute endpoint_node)
{
  attribute device_id_type_node
    = endpoint_node.child_by_type(ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_TYPE);
  attribute device_id_node = device_id_type_node.child_by_type(
    ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_DATA);
  attribute device_id_format_node = device_id_type_node.child_by_type(
    ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_DATA_FORMAT);

  if (device_id_node.reported_exists()
      && device_id_format_node.reported_exists()) {
    std::stringstream ss;
    switch (device_id_format_node.reported<uint32_t>()) {
      case 0x00:  //the Device ID Data MUST be in UTF-8 format.
        ss << device_id_node.reported<std::string>();
        break;
      case 0x01:  //The Device ID Data is in plain binary format and MUST be displayed as hexadecimal values e.g. 0x30, 0x31, 0x32, 0x33 MUST be displayed as h’30313233.
        ss << "h'" << std::setw(2) << std::setfill('0') << std::hex
           << std::uppercase;
        for (auto c: device_id_node.reported<std::vector<uint8_t>>()) {
          ss << (int)c;
        }
        break;
      default:
        sl_log_error(LOG_TAG, "Unknown format id");
        return;
    }
    attribute dotdot_serial
      = endpoint_node.child_by_type(DOTDOT_ATTRIBUTE_ID_BASIC_SERIAL_NUMBER);
    if (!dotdot_serial.is_valid()) {
      dotdot_serial
        = endpoint_node.add_node(DOTDOT_ATTRIBUTE_ID_BASIC_SERIAL_NUMBER);
    }
    attribute_store_set_reported_string(dotdot_serial, ss.str().c_str());
  }
}

/**
 * @brief Set a Manufacturer Name on dotdot domain if Manufacturer ID present in
 *        under endpoint attribute on Z-Wave domain.
 *
 * @param endpoint_node Attribute Store node for a endpoint node.
 */
static void set_manufacturer_name(attribute endpoint_node)
{
  if (endpoint_node
        .child_by_type(ATTRIBUTE_MANUFACTURER_SPECIFIC_MANUFACTURER_ID)
        .reported_exists()) {
    attribute manufacturer_id_node = endpoint_node.child_by_type(
      ATTRIBUTE_MANUFACTURER_SPECIFIC_MANUFACTURER_ID,
      0);
    try {
      std::string manufacturer_name = manufacturer_id_name_map.at(
        manufacturer_id_node.reported<uint32_t>());
      attribute dotdot_man_node = endpoint_node.emplace_node(
        DOTDOT_ATTRIBUTE_ID_BASIC_MANUFACTURER_NAME);
      attribute_store_set_reported_string(dotdot_man_node,
                                          manufacturer_name.c_str());
    } catch (const std::exception &e) {
      sl_log_debug(LOG_TAG,
                   "Cannot read Manufacturer ID for node %d. "
                   "Assuming Unknown Manufacturer ID.\n",
                   manufacturer_id_node);
    }
  }
}

// A callback that will be called when a Network Status Attribute is
// updated.
static void network_status_attribute_update(attribute_store_node_t updated_node,
                                            attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_UNAVAILABLE;
  attribute_store_get_reported(updated_node,
                               &network_status,
                               sizeof(network_status));
  if (network_status != NODE_STATE_TOPIC_STATE_INCLUDED) {
    return;
  }
  attribute network_status_node(updated_node);
  attribute node_id_node  = network_status_node.first_parent(ATTRIBUTE_NODE_ID);
  zwave_node_id_t node_id = 0;
  try {
    node_id = node_id_node.reported<zwave_node_id_t>();
    if (node_id == zwave_network_management_get_node_id()) {
      return;
    }
  } catch (std::exception &e) {
    sl_log_warning(LOG_TAG,
                   "Cannot determine NodeID from attribute store node %d",
                   node_id_node);
  }

  // set the dotdot Basic Cluster attributes
  for (attribute endpoint_node: node_id_node.children(ATTRIBUTE_ENDPOINT_ID)) {
    // set a Manufacturer Name on dotdot domain
    set_manufacturer_name(endpoint_node);
    // Set the Basic Cluster serial number attribute value.
    set_basic_cluster_serial_num(endpoint_node);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
bool basic_cluster_mapper_init()
{
  sl_log_debug(LOG_TAG, "Basic Cluster Mapper initialization");

  // Listen to updates: Network Status attribute
  attribute_store_register_callback_by_type_and_state(
    &network_status_attribute_update,
    ATTRIBUTE_NETWORK_STATUS,
    REPORTED_ATTRIBUTE);

  return true;
}

int basic_cluster_mapper_teardown()
{
  sl_log_debug(LOG_TAG, "Teardown of the Basic Cluster Mapper");
  return 0;
}
