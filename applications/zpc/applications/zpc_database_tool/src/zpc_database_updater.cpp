/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "zpc_database_updater.hpp"

// ZPC components
#include "zpc_datastore_fixt.h"
#include "attribute_store_defined_attribute_types.h"

// Interface 
#include "zpc_database_helper.hpp"

// Unify components
#include "attribute.hpp"
#include "attribute_store.h"
#include "attribute_store_type_registration.h"
#include "sl_log.h"

// Datastore   
#include "datastore.h"

using namespace attribute_store;
constexpr const char *LOG_TAG = "zpc_database_updater";

namespace zpc_database_updater {
///////////////////////////////////////////////////////////////////////////////
// Helper function
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Update bitmask from an array to a uint32_t
 * 
 * @param attribute_to_update Attribute to update
 * 
 * @returns true if the update was successful
 * @returns false if the update was unsuccessful
 */
bool update_bitmask_attribute(attribute attribute_to_update)
{
  std::vector<uint8_t> current_value
    = attribute_to_update.reported<std::vector<uint8_t>>();

  auto bitmask_length                   = current_value.size();
  uint32_t new_bitmask = 0;

  // Since we are using uint32_t we can't have more that 4 bit mask
  if (bitmask_length > 4  || bitmask_length == 0) {
    return false;
  }

  for (int i = bitmask_length - 1; i >= 0; i--) {
    new_bitmask
      = (new_bitmask << 8) | current_value[i];
  }

  attribute_to_update.set_reported<uint32_t>(new_bitmask);

  return true;
}

/**
 * @brief Update an attribute type from a bitmask (array) to a uint32_t
 * 
 * @param attribute_type The type of attribute to update
 * 
 */
void helper_convert_bitmask_to_new_format(attribute_store_type_t attribute_type)
{
  auto attribute_list = get_attribute_list(attribute_type);
  const std::string attribute_name = attribute_store_get_type_name(attribute_type);

  if (attribute_list.size() == 0) {
    sl_log_info(LOG_TAG,
                "No attribute of type %s found. Nothing to do.",
                attribute_name.c_str());
  } else {
    sl_log_info(LOG_TAG,
                "Updating attribute(s) %s to uint32_t...",
                attribute_name.c_str());

    for (auto current_attribute: attribute_list) {
      if (!update_bitmask_attribute(current_attribute)) {
        sl_log_warning(
          LOG_TAG,
          "Could not update %s (id : %d) to uint32_t. Removing node instead.",
          attribute_name.c_str(),
          current_attribute);

        current_attribute.delete_node();
      }
    }

    sl_log_info(LOG_TAG, "Done.");
  }
}




///////////////////////////////////////////////////////////////////////////////
// Conversion functions
///////////////////////////////////////////////////////////////////////////////
// See zpc_datastore_version for details on the versioning
sl_status_t convert_v1_datastore_to_v2()
{
  // Association group ID change: uint32_t to uint8:
  sl_log_info(LOG_TAG,
              "Updating Association Group IDs from uint32_t to uint8_t.");

  for (attribute endpoint_id: endpoint_id_list()) {
    for (attribute group_id:
         endpoint_id.children(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID)) {
      sl_log_debug(LOG_TAG,
                   "Updating Attribute ID %d from uint32_t to uint8_t",
                   group_id);
      uint32_t current_value = group_id.reported<uint32_t>();
      if (current_value > 255) {
        sl_log_warning(LOG_TAG,
                       "Association Group ID found with value >255. "
                       "This should not happen. Deleting data.");
        group_id.delete_node();
      } else {
        uint8_t new_value = current_value;
        group_id.set_reported<uint8_t>(new_value);
      }
    }
  }

  // Wake Up Command Class Setting representation change.
  // clang-format off
  sl_log_info(LOG_TAG, "Updating Wake Up Settings to new tree.");
  for (attribute endpoint_id: endpoint_id_list()) {
    attribute interval_node = endpoint_id.child_by_type(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL);
    attribute node_id_node = endpoint_id.child_by_type(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_NODE_ID);

    if (!interval_node.is_valid() || !node_id_node.is_valid()) {
      continue;
    }

    sl_log_debug(LOG_TAG, "Converting Wake Up data for Endpoint node %d", endpoint_id);
    // Do not set a setting value, we may ask the node again when it wakes up.
    attribute setting = endpoint_id.emplace_node(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_SETTING);

    attribute new_node_id_node = setting.emplace_node(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_NODE_ID);
    attribute new_interval_node = setting.emplace_node(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL);

    // Copy the values (Desired is never persisted)
    new_node_id_node.set_reported<uint16_t>(node_id_node.reported<uint16_t>());
    new_interval_node.set_reported<uint32_t>(interval_node.reported<uint32_t>());

    // Remove the old nodes
    interval_node.delete_node();
    node_id_node.delete_node();

    // Create capabilities for any supporting node from this version.
    attribute capabilities = endpoint_id.emplace_node(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_CAPABILITIES);
    // Capabilities sub-attributes
    attribute new_min_interval_node = capabilities.emplace_node(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_MINIMUM_INTERVAL);
    attribute new_max_interval_node = capabilities.emplace_node(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_MAXIMUM_INTERVAL);
    attribute new_default_interval_node = capabilities.emplace_node(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_DEFAULT_INTERVAL);
    attribute new_interval_step_node = capabilities.emplace_node(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL_STEP);
    attribute new_bitmask_node = capabilities.emplace_node(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_CAPABILITIES_BITMASK);

    // Check if previous capabilities are here:
    attribute min_interval_node = endpoint_id.child_by_type(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_MINIMUM_INTERVAL);
    attribute max_interval_node = endpoint_id.child_by_type(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_MAXIMUM_INTERVAL);
    attribute default_interval_node = endpoint_id.child_by_type(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_DEFAULT_INTERVAL);
    attribute interval_step_node = endpoint_id.child_by_type(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL_STEP);
    attribute bitmask_node = endpoint_id.child_by_type(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_CAPABILITIES_BITMASK);

    // If valid, copy and erase:
    if (min_interval_node.is_valid()) {
      new_min_interval_node.set_reported<uint32_t>(min_interval_node.reported<uint32_t>());
      min_interval_node.delete_node();
    }
    if (max_interval_node.is_valid()) {
      new_max_interval_node.set_reported<uint32_t>(max_interval_node.reported<uint32_t>());
      max_interval_node.delete_node();
    }
    if (default_interval_node.is_valid()) {
      new_default_interval_node.set_reported<uint32_t>(default_interval_node.reported<uint32_t>());
      default_interval_node.delete_node();
    }
    if (interval_step_node.is_valid()) {
      new_interval_step_node.set_reported<uint32_t>(interval_step_node.reported<uint32_t>());
      interval_step_node.delete_node();
    }

    int8_t bitmask_value = 0;
    if (bitmask_node.is_valid()) {
      bitmask_value = static_cast<uint8_t>(bitmask_node.reported<uint32_t>());
      bitmask_node.delete_node();
    }
    new_bitmask_node.set_reported<uint8_t>(bitmask_value);
  }
  // clang-format on

  // Write down the new version of the database
  sl_log_info(LOG_TAG, "Successfully converted from version 1 to version 2.\n");
  return datastore_store_int("version", DATASTORE_VERSION_V2);
}

// See zpc_datastore_version for details on the versioning
sl_status_t convert_v2_datastore_to_v3() {
  // Update ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES
  helper_convert_bitmask_to_new_format(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES);

  // Update ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_SUPPORTED_SETPOINT_TYPES
  helper_convert_bitmask_to_new_format(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_SUPPORTED_SETPOINT_TYPES);

  // Setpoint types
  auto setpoint_types_list
    = get_attribute_list(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPE);
  const std::string attribute_name = attribute_store_get_type_name(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPE);

  if (setpoint_types_list.size() == 0) {
    sl_log_info(LOG_TAG,
                "No attribute of type %s found. Nothing to do.",
                attribute_name.c_str());
  } else {
    sl_log_info(LOG_TAG,
                "Updating attribute(s) ID under %s...",
                attribute_name.c_str());


    // WARNING : Do not use defined type value here. We need those specific ID's from 
    // the v2 database and form the v3 database.
    // Specify ID's for the setpoint types since they have changed since v2
    constexpr attribute_store_type_t OLD_MIN_VALUE_TYPE       = 0x4306;
    constexpr attribute_store_type_t OLD_MIN_VALUE_SCALE_TYPE = 0x4307;
    constexpr attribute_store_type_t OLD_MAX_VALUE_TYPE       = 0x4308;
    constexpr attribute_store_type_t OLD_MAX_VALUE_SCALE_TYPE = 0x4309;
    // New attribute ids
    constexpr attribute_store_type_t NEW_VALUE_PRECISION_TYPE     = 0x4306;
    constexpr attribute_store_type_t NEW_MIN_VALUE_TYPE           = 0x4307;
    constexpr attribute_store_type_t NEW_MIN_VALUE_SCALE_TYPE     = 0x4308;
    constexpr attribute_store_type_t NEW_MIN_VALUE_PRECISION_TYPE = 0x4309;
    constexpr attribute_store_type_t NEW_MAX_VALUE_TYPE           = 0x430A;
    constexpr attribute_store_type_t NEW_MAX_VALUE_SCALE_TYPE     = 0x430B;
    constexpr attribute_store_type_t NEW_MAX_VALUE_PRECISION_TYPE = 0x430C;
    // Default precision (keep same behavior as before)
    constexpr uint8_t DEFAULT_PRECISION = 3;

    for(auto current_setpoint: setpoint_types_list) {
      // Update the scale for value (same ID)
      uint32_t old_value_scale
        = current_setpoint
            .child_by_type(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE_SCALE)
            .reported<uint32_t>();
      // Those cases should NOT happen
      if (old_value_scale > 255) {
        sl_log_warning(LOG_TAG, "Value scale is too high. Setting it to 255.");
        old_value_scale = 255;
      }
      current_setpoint.child_by_type(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE_SCALE)
        .set_reported<uint8_t>(old_value_scale);

      // First get old values
      int32_t old_min_value = current_setpoint.child_by_type(OLD_MIN_VALUE_TYPE)
                                .reported<int32_t>();
      uint32_t old_min_value_scale
        = current_setpoint.child_by_type(OLD_MIN_VALUE_SCALE_TYPE)
            .reported<uint32_t>();
      int32_t old_max_value = current_setpoint.child_by_type(OLD_MAX_VALUE_TYPE)
                                .reported<int32_t>();
      uint32_t old_max_value_scale
        = current_setpoint.child_by_type(OLD_MAX_VALUE_SCALE_TYPE)
            .reported<uint32_t>();

      // Those cases should NOT happen
      if (old_min_value_scale > 255) {
        sl_log_warning(LOG_TAG,
                       "Min value scale is too high. Setting it to 255.");
        old_min_value_scale = 255;               
      }
      if (old_max_value_scale > 255) {
        sl_log_warning(LOG_TAG,
                       "Max value scale is too high. Setting it to 255.");
        old_max_value_scale = 255;               
      }

      // Then update current attribute tree
      // Those already exists, we just need to update them with the right values
      current_setpoint.child_by_type(NEW_VALUE_PRECISION_TYPE)
        .set_reported<uint8_t>(DEFAULT_PRECISION);
      current_setpoint.child_by_type(NEW_MIN_VALUE_TYPE)
        .set_reported<int32_t>(old_min_value);
      current_setpoint.child_by_type(NEW_MIN_VALUE_SCALE_TYPE)
        .set_reported<uint8_t>(old_min_value_scale);
      current_setpoint.child_by_type(NEW_MIN_VALUE_PRECISION_TYPE)
        .set_reported<uint8_t>(DEFAULT_PRECISION);
      // Create the missing ones
      current_setpoint.emplace_node<int32_t>(NEW_MAX_VALUE_TYPE, old_max_value);
      current_setpoint.emplace_node<uint8_t>(NEW_MAX_VALUE_SCALE_TYPE,
                                              old_max_value_scale);
      current_setpoint.emplace_node<uint8_t>(NEW_MAX_VALUE_PRECISION_TYPE, 3);
    }

    sl_log_info(LOG_TAG, "Done.");
  }
  
  sl_log_info(LOG_TAG, "Successfully converted from version 2 to version 3.\n");
  return datastore_store_int("version", DATASTORE_VERSION_V3);
}


///////////////////////////////////////////////////////////////////////////////
// Exposed functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t update_datastore(int64_t datastore_version,
                             int64_t target_version)
{
  while (datastore_version < target_version) {
    switch (datastore_version) {
      case DATASTORE_VERSION_V2:
        convert_v2_datastore_to_v3();
        datastore_version++;
        break;
      case DATASTORE_VERSION_V1:
        convert_v1_datastore_to_v2();
        datastore_version++;
        break;
      default:
        sl_log_warning(LOG_TAG,
                       "We do not know how to upgrade version %d. "
                       "Aborting ZPC datastore upgrade.",
                       datastore_version);
        return SL_STATUS_FAIL;
        break;
    }
  }
  return SL_STATUS_OK;
}

}  // namespace zpc_database_updater