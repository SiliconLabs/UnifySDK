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
// Tool includes
#include "zpc_database_helper.hpp"

// Unify Includes
#include "datastore.h"
#include "datastore_fixt.h"
#include "attribute_store_fixt.h"
#include "config.h"
#include "zpc_config.h"
#include "uic_version.h"
#include "attribute.hpp"
#include "attribute_store.h"
#include "attribute_store_type_registration.h"
#include "sl_log.h"

// ZPC includes
#include "zpc_datastore_fixt.h"
#include "attribute_store_defined_attribute_types.h"

using namespace attribute_store;
constexpr const char *LOG_TAG = "zpc_database_upgrade_tool";

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t init(int argc, char *argv[], int *target_version)
{
  config_add_int("target_version",
                 "Version to convert the current datastore file to.",
                 ZPC_DATASTORE_VERSION);

  config_add_string(CONFIG_KEY_ZPC_DATASTORE_FILE,
                    "ZPC datastore database file",
                    DEFAULT_ZPC_DATASTORE_FILE);

  if (SL_STATUS_OK != config_parse(argc, argv, UIC_VERSION)) {
    sl_log_info(LOG_TAG,
                "Could not parse configuration. "
                "Aborting database upgrade operation.");
    return SL_STATUS_FAIL;
  }

  config_get_as_int("target_version", target_version);
  if (*target_version == 0) {
    sl_log_error(LOG_TAG,
                 "You need to specify a target_version. "
                 "E.g. zpc_database_upgrade_tool --target_version 2 "
                 "Please verify your configuration.");
    return SL_STATUS_FAIL;
  }

  const char *datastore_file;
  if (SL_STATUS_OK
      != config_get_as_string(CONFIG_KEY_ZPC_DATASTORE_FILE, &datastore_file)) {
    sl_log_error(LOG_TAG,
                 "You need to specify a valid datastore file path. "
                 "E.g. zpc_database_tool --" CONFIG_KEY_ZPC_DATASTORE_FILE
                 " " UIC_VAR_DIR "/zpc.db "
                 "Please verify your configuration.");
    return SL_STATUS_FAIL;
  }

  if (SL_STATUS_OK != datastore_fixt_setup(datastore_file)) {
    sl_log_error(LOG_TAG,
                 "Could not initialize the datastore. "
                 "Please verify your configuration.");
    return SL_STATUS_FAIL;
  }

  if (SL_STATUS_OK != attribute_store_init()) {
    sl_log_error(LOG_TAG,
                 "Could not initialize the Attribute Store. "
                 "Please verify your configuration.");
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

static void teardown()
{
  attribute_store_teardown();
  datastore_fixt_teardown();
}

///////////////////////////////////////////////////////////////////////////////
// Conversion functions
///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
// Main
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  int target_version      = 0;
  sl_status_t init_status = init(argc, argv, &target_version);
  if (init_status != SL_STATUS_OK) {
    teardown();
    return 1;
  }

  // Fetch the version of the datastore file.
  int64_t datastore_version = 0;
  if (datastore_contains_int("version") != true) {
    sl_log_error(LOG_TAG,
                 "Cannot read current datastore file version. "
                 "We cannot upgrade such a datastore.");
    teardown();
    return 1;
  }
  datastore_fetch_int("version", &datastore_version);
  sl_log_info(LOG_TAG,
              "Datastore version reported from the datastore file: %d\n",
              datastore_version);

  // Make the conversion, if we know how.
  if (datastore_version == target_version) {
    sl_log_info(LOG_TAG,
                "Datastore is already at version %d. No conversion is needed.",
                datastore_version);
  }
  while (datastore_version < target_version) {
    switch (datastore_version) {
      case DATASTORE_VERSION_V1:
        convert_v1_datastore_to_v2();
        datastore_version++;
        break;
      default:
        sl_log_warning(LOG_TAG,
                       "We do not know how to upgrade version %d. "
                       "Aborting ZPC datastore upgrade.",
                       datastore_version);
        teardown();
        return 1;
        break;
    }
  }

  // Teardown when we are done.
  teardown();
  return 0;
}
