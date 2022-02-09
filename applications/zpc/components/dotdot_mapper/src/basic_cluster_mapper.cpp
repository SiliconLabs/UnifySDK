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
#include "dotdot_attributes.h"

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
#include <sstream>
#include <iomanip>
// Interfaces
#include "zwave_command_class_version_types.h"
constexpr char LOG_TAG[] = "basic_cluster_mapper";
using namespace attribute_store;

// Manufacturer name data
#include "manufacturer_id_name_list.inc"

/**
 * @brief Maps the Listening/optional protocol bits to Basic cluster power source
 *
 * @param updated_node    Attribute Store node for the Listening or Optional flag.
 * @param change          Attribute store change that occurred.
 */
void on_nif_update_set_power_source(attribute_store_node_t updated_node,
                                    attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_NODE_ID);

  zwave_node_id_t node_id = 0;
  if (attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id))
      != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Cannot determine NodeID from attribute store node %d",
                   updated_node);
    return;
  }

  if (node_id == zwave_network_management_get_node_id()) {
    return;
  }

  int32_t power_source = 0;
  switch (zwave_get_operating_mode(node_id)) {
    case OPERATING_MODE_AL:
      power_source = 4;
      break;
    case OPERATING_MODE_NL:
    case OPERATING_MODE_FL:
      power_source = 3;
      break;
    default:
      power_source = 0;
      break;
  }

  // Create if not existing and set the value of the Basic Power Source for all endpoints.
  for (attribute endpoint_node:
       attribute(node_id_node).children(ATTRIBUTE_ENDPOINT_ID)) {
    attribute_store_set_child_reported(endpoint_node,
                                       DOTDOT_ATTRIBUTE_ID_BASIC_POWER_SOURCE,
                                       &power_source,
                                       sizeof(power_source));
  }
}

/**
 * @brief Maps Version CC Hardware version Basic cluster Hardware version
 *
 * @param updated_node    Attribute Store node for the Hardware version
 * @param change          Attribute store change that occurred.
 */
void on_hardware_version_update(attribute_store_node_t updated_node,
                                attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  zwave_version_hardware_version_t hardware_version = 0;
  if (attribute_store_get_reported(updated_node,
                                   &hardware_version,
                                   sizeof(hardware_version))
      != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Cannot HW version value from attribute store node %d",
                   updated_node);
    return;
  }

  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_NODE_ID);

  // Create if not existing and set the value for the HW version for all endpoints
  int32_t zcl_hardware_version = hardware_version;
  for (attribute endpoint_node:
       attribute(node_id_node).children(ATTRIBUTE_ENDPOINT_ID)) {
    attribute_store_set_child_reported(endpoint_node,
                                       DOTDOT_ATTRIBUTE_ID_BASIC_HW_VERSION,
                                       &zcl_hardware_version,
                                       sizeof(zcl_hardware_version));
  }
}

/**
 * @brief Creates a Manufacturer Name from the Manufacturer ID.
 *
 * @param updated_node
 * @param change
 */
static void device_id_attribute_update(attribute_store_node_t updated_node,
                                       attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  attribute device_id_node(updated_node);
  attribute ep     = device_id_node.first_parent(ATTRIBUTE_ENDPOINT_ID);
  attribute format = device_id_node.parent().child_by_type(
    ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_DATA_FORMAT);

  if (!format.reported_exists() || !device_id_node.reported_exists()) {
    sl_log_warning(LOG_TAG, "Missing Data format attribute");
    return;
  }

  std::stringstream ss;
  switch (format.reported<uint32_t>()) {
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
    = ep.child_by_type(DOTDOT_ATTRIBUTE_ID_BASIC_SERIAL_NUMBER);
  if (!dotdot_serial.is_valid()) {
    dotdot_serial = ep.add_node(DOTDOT_ATTRIBUTE_ID_BASIC_SERIAL_NUMBER);
  }
  dotdot_serial.set_reported(ss.str());
}

/**
 * @brief Creates a Manufacturer Name from the Manufacturer ID.
 *
 * @param updated_node
 * @param change
 */
static void
  manufacturer_id_attribute_update(attribute_store_node_t updated_node,
                                   attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  attribute manufacturer_id_node(updated_node);
  // Get the Customer name from the lookup table
  std::string manufacturer_name = "Unknown Manufacturer";
  try {
    manufacturer_name
      = manufacturer_id_name_map.at(manufacturer_id_node.reported<uint32_t>());
  } catch (const std::exception &e) {
    sl_log_debug(LOG_TAG,
                 "Cannot read Manufacturer ID for node %d. "
                 "Assuming Unknown Manufacturer ID.\n",
                 manufacturer_id_node);
  }

  attribute node_id_node = manufacturer_id_node.first_parent(ATTRIBUTE_NODE_ID);

  for (attribute endpoint_node: node_id_node.children(ATTRIBUTE_ENDPOINT_ID)) {
    attribute dotdot_manf_node
      = endpoint_node.emplace_node(DOTDOT_ATTRIBUTE_ID_BASIC_MANUFACTURER_NAME);

    attribute_store_set_reported_string(dotdot_manf_node,
                                        manufacturer_name.c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
bool basic_cluster_mapper_init()
{
  sl_log_debug(LOG_TAG, "Basic Cluster Mapper initialization");

  // Listen to Manufacturer ID attributes to publish the manufacturer name.
  attribute_store_register_callback_by_type_and_state(
    manufacturer_id_attribute_update,
    ATTRIBUTE_MANUFACTURER_SPECIFIC_MANUFACTURER_ID,
    REPORTED_ATTRIBUTE);

  // Listen to NIF updates: Power Source
  attribute_store_register_callback_by_type_and_state(
    &on_nif_update_set_power_source,
    ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
    REPORTED_ATTRIBUTE);
  attribute_store_register_callback_by_type_and_state(
    &on_nif_update_set_power_source,
    ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
    REPORTED_ATTRIBUTE);

  // Listen to updates: Hardware version
  attribute_store_register_callback_by_type_and_state(
    &on_hardware_version_update,
    ATTRIBUTE_CC_VERSION_HARDWARE_VERSION,
    REPORTED_ATTRIBUTE);

  // Listen to updates: Device id data
  attribute_store_register_callback_by_type_and_state(
    &device_id_attribute_update,
    ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_DATA,
    REPORTED_ATTRIBUTE);

  return true;
}

int basic_cluster_mapper_teardown()
{
  sl_log_debug(LOG_TAG, "Teardown of the Basic Cluster Mapper");
  return 0;
}
