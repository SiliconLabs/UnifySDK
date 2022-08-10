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
#include "zwave_command_class_firmware_update.h"
#include "zwave_command_class_firmware_update_internals.h"
#include "zwave_command_classes_utils.h"

// Interfaces
#include "zwave_command_class_version_types.h"

// Generic includes
#include <stdio.h>
#include <string.h>

// Includes from ZPC components
#include "attribute_store_defined_attribute_types.h"
#include "zwave_tx_scheme_selector.h"
#include "zpc_attribute_store_network_helper.h"

// Includes from Unify components
#include "attribute_store_helper.h"
#include "attribute_timeouts.h"
#include "ota_time.h"
#include "sl_log.h"

// Attribute macro, shortening those long defines:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_FWU_MD_##type

/// Timeout after which we consider a firmware update aborted if we don't hear
/// anything. This should stay higher than SEND_DATA_EMERGENCY_TIMEOUT
#define FIRMWARE_TRANSFER_TIMEOUT 120 * CLOCK_SECOND

// Log tag for logging
#define LOG_TAG "zwave_command_class_firmware_update"

/// Number of bytes used by a Firmware Update Meta Data Report Command (apart from the Data field)
#define FWU_DATA_REPORT_ENCAPSULATION_OVERHEAD 6

/////////////////////////////////////////////////////////////////////////
// Private functions
/////////////////////////////////////////////////////////////////////////
/**
 * @brief Creates a Firmware Transfer node under an endpoint
 *
 * @param firmware_update_node  Attribute Store node for Firmware Update
 *                              attribute (ATTRIBUTE(FWU))
 *
 * @returns attribute_store_node_t for the created firmware_transfer_node.
 */
static attribute_store_node_t create_firmware_transfer_attribute(
  attribute_store_node_t firmware_update_node)
{
  attribute_store_node_t firmware_transfer_node
    = attribute_store_add_node(ATTRIBUTE(FW_TRANSFER), firmware_update_node);
  zwave_firmware_transfer_state_t state = IDLE;
  attribute_store_set_reported(firmware_transfer_node, &state, sizeof(state));
  attribute_store_set_desired(firmware_transfer_node, &state, sizeof(state));

  return firmware_transfer_node;
}

/**
 * @brief Abort a firmware transfer by setting the firmware update state from
 *        ONGOING to IDLE, and the last status to ABORTED
 *
 * Nothing will happen if the reported state is not ONGOING.
 * (i.e. already idle, waiting for activation or reboot)
 *
 * @param firmware_transfer_node  Attribute Store node for Firmware Update
 *                                attribute (ATTRIBUTE(FW_TRANSFER))
 */
static void
  abort_transfer_if_ongoing(attribute_store_node_t firmware_transfer_node)
{
  zwave_firmware_transfer_state_t state = IDLE;
  if (attribute_store_is_value_defined(firmware_transfer_node,
                                       REPORTED_ATTRIBUTE)) {
    attribute_store_get_reported(firmware_transfer_node, &state, sizeof(state));
  } else {
    attribute_store_get_desired(firmware_transfer_node, &state, sizeof(state));
  }

  if (state == ONGOING) {
    sl_log_debug(LOG_TAG,
                 "Setting Firmware Transfer state back to "
                 "idle for Attribute ID %d",
                 firmware_transfer_node);
    attribute_store_undefine_reported(firmware_transfer_node);
    state = IDLE;
    attribute_store_set_desired(firmware_transfer_node, &state, sizeof(state));

    set_firmware_transfer_last_status(firmware_transfer_node, ABORTED);
  }
}

/**
 * @brief Finds the size of a file, in bytes
 *
 * @param file_name        The path of the file to determine the file size
 *
 * @returns The size of the file, in bytes
 */
static size_t get_file_size(const char *file_name)
{
  // Open the file
  FILE *fp = fopen(file_name, "r");

  // File does not exist, we return 0 as if it was empty
  if (fp == NULL) {
    return 0;
  }

  fseek(fp, 0, SEEK_END);
  size_t file_size = ftell(fp);
  fclose(fp);

  return file_size;
}

/**
 * @brief Helper function that starts a firmware update
 *
 * @param firmware_transfer_node  Attribute Store node under which the Firmware
 *                                Target must be set.
 */
static void
  start_firmware_update_operation(attribute_store_node_t firmware_transfer_node)
{
  zwave_firmware_transfer_state_t desired_state = ONGOING;
  attribute_store_set_desired(firmware_transfer_node,
                              &desired_state,
                              sizeof(desired_state));
  attribute_store_undefine_reported(firmware_transfer_node);
}

/////////////////////////////////////////////////////////////////////////
// Protected functions (shared only within the library)
/////////////////////////////////////////////////////////////////////////
sl_status_t
  set_firmware_transfer_file(attribute_store_node_t firmware_transfer_node,
                             const char *file_name)
{
  attribute_store_node_t file_name_node
    = attribute_store_get_first_child_by_type(firmware_transfer_node,
                                              ATTRIBUTE(FW_TRANSFER_FILENAME));
  if (file_name_node == ATTRIBUTE_STORE_INVALID_NODE) {
    file_name_node = attribute_store_add_node(ATTRIBUTE(FW_TRANSFER_FILENAME),
                                              firmware_transfer_node);
  }

  return attribute_store_set_reported_string(file_name_node, file_name);
}

sl_status_t set_firmware_transfer_last_status(
  attribute_store_node_t firmware_transfer_node,
  zwave_firmware_transfer_status_t status)
{
  return attribute_store_set_child_reported(firmware_transfer_node,
                                            ATTRIBUTE(FW_TRANSFER_LAST_STATUS),
                                            &status,
                                            sizeof(status));
}

sl_status_t set_transfer_offset(attribute_store_node_t firmware_transfer_node,
                                uint32_t transfer_offset)
{
  return attribute_store_set_child_reported(firmware_transfer_node,
                                            ATTRIBUTE(FW_TRANSFER_OFFSET),
                                            &transfer_offset,
                                            sizeof(transfer_offset));
}

uint16_t firmware_transfer_get_theoretical_max_fragment_size(
  attribute_store_node_t endpoint_node)
{
  // Gather all the data we need from the attribute store.
  zwave_endpoint_id_t endpoint_id = 0;
  attribute_store_get_reported(endpoint_node,
                               &endpoint_id,
                               sizeof(endpoint_id));
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(endpoint_node,
                                                 ATTRIBUTE_NODE_ID);
  zwave_node_id_t node_id = 0;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));

  // Ask Z-Wave TX scheme the max payload after security encapsulation
  uint16_t maximum_fragment_size
    = zwave_tx_scheme_get_max_application_payload(node_id, endpoint_id);

  // make room for the firmare Update Command overhead
  maximum_fragment_size -= FWU_DATA_REPORT_ENCAPSULATION_OVERHEAD;
  return maximum_fragment_size;
}

sl_status_t set_firmware_transfer_fragment_size(
  attribute_store_node_t firmware_transfer_node)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(firmware_transfer_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  attribute_store_node_t firmware_update_node
    = attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE(FWU));
  attribute_store_node_t fragment_size_node
    = attribute_store_get_first_child_by_type(firmware_update_node,
                                              ATTRIBUTE(FWU_FRAGMENT_SIZE));
  if (attribute_store_is_value_defined(fragment_size_node, REPORTED_ATTRIBUTE)
      == false) {
    return SL_STATUS_FAIL;
  }

  // Make a copy of this attribute under the firmware transfer.
  attribute_store_node_t transfer_fragment_size_node
    = attribute_store_get_first_child_by_type(firmware_transfer_node,
                                              ATTRIBUTE(FWU_FRAGMENT_SIZE));
  if (transfer_fragment_size_node == ATTRIBUTE_STORE_INVALID_NODE) {
    transfer_fragment_size_node
      = attribute_store_add_node(ATTRIBUTE(FWU_FRAGMENT_SIZE),
                                 firmware_transfer_node);
  }
  return attribute_store_copy_value(fragment_size_node,
                                    transfer_fragment_size_node,
                                    REPORTED_ATTRIBUTE);
}

bool node_supports_firmware_activation(attribute_store_node_t endpoint_node)
{
  uint32_t node_supports_firmware_activation = 0;

  attribute_store_node_t firmware_update_node
    = attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE(FWU));

  attribute_store_node_t activation_node
    = attribute_store_get_first_child_by_type(firmware_update_node,
                                              ATTRIBUTE(FWU_ACTIVATION));

  attribute_store_get_reported(activation_node,
                               &node_supports_firmware_activation,
                               sizeof(node_supports_firmware_activation));

  if (node_supports_firmware_activation == 0) {
    return false;
  }

  zwave_cc_version_t version = 0;
  attribute_store_node_t version_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(VERSION));
  attribute_store_get_reported(version_node, &version, sizeof(version));

  if (version < 7) {
    return false;
  }

  return true;
}

bool node_supports_cc(attribute_store_node_t endpoint_node)
{
  uint32_t node_supports_cc = 0;

  attribute_store_node_t firmware_update_node
    = attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE(FWU));

  attribute_store_node_t cc_node
    = attribute_store_get_first_child_by_type(firmware_update_node,
                                              ATTRIBUTE(FWU_CC));

  attribute_store_get_reported(cc_node,
                               &node_supports_cc,
                               sizeof(node_supports_cc));

  return (node_supports_cc == 1);
}

bool is_firmware_target_valid(attribute_store_node_t endpoint_node,
                              uint32_t firmware_target)
{
  attribute_store_node_t firmware_update_node
    = attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE(FWU));

  attribute_store_node_t firmware_entry_node
    = attribute_store_get_node_child_by_value(firmware_update_node,
                                              ATTRIBUTE(FWU_FW),
                                              REPORTED_ATTRIBUTE,
                                              (const uint8_t *)&firmware_target,
                                              sizeof(firmware_target),
                                              0);

  return (firmware_entry_node != ATTRIBUTE_STORE_INVALID_NODE);
}

bool is_firmware_update_ongoing(attribute_store_node_t endpoint_node)
{
  // Check 2 things:
  // 1. The reported value of the firmware transfer
  // 2. The expiry timer is not expired.
  attribute_store_node_t firmware_update_node
    = attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE(FWU));

  attribute_store_node_t firmware_transfer_node
    = attribute_store_get_first_child_by_type(firmware_update_node,
                                              ATTRIBUTE(FW_TRANSFER));

  // During a transfer, we expect Reported = ONGOING.
  zwave_firmware_transfer_state_t state = IDLE;
  attribute_store_get_reported(firmware_transfer_node, &state, sizeof(state));
  if (state != ONGOING) {
    return false;
  }
  // Desired should be IDLE. If it is not, we set it ourselves to keep
  // the state machine consistent
  attribute_store_get_desired(firmware_transfer_node, &state, sizeof(state));
  if (state != IDLE) {
    state = IDLE;
    attribute_store_set_desired(firmware_transfer_node, &state, sizeof(state));
  }

  // Check the expiry timestamp:
  clock_time_t expiry_time = 0;
  attribute_store_node_t expiry_time_node
    = attribute_store_get_first_child_by_type(
      firmware_transfer_node,
      ATTRIBUTE(FW_TRANSFER_EXPIRY_TIME));
  if (attribute_store_is_value_defined(expiry_time_node, REPORTED_ATTRIBUTE)
      == false) {
    // No expiry is defined, the transfer does not expire and is ongoing
    return true;
  }
  attribute_store_get_reported(expiry_time_node,
                               &expiry_time,
                               sizeof(expiry_time));
  if (expiry_time < clock_time()) {
    return false;
  }

  // If we got there, it is ongoing.
  return true;
}

sl_status_t
  postpone_firmware_update_expiry(attribute_store_node_t endpoint_node)
{
  // If nothing is ongoing or it expired, we do not "un"-expire it.
  if (is_firmware_update_ongoing(endpoint_node) == false) {
    return SL_STATUS_FAIL;
  }
  attribute_store_node_t firmware_update_node
    = attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE(FWU));

  attribute_store_node_t firmware_transfer_node
    = attribute_store_get_first_child_by_type(firmware_update_node,
                                              ATTRIBUTE(FW_TRANSFER));

  clock_time_t duration   = FIRMWARE_TRANSFER_TIMEOUT;
  clock_time_t new_expiry = clock_time() + duration;
  attribute_timeout_set_callback(firmware_transfer_node,
                                 duration,
                                 &abort_transfer_if_ongoing);
  return attribute_store_set_child_reported(firmware_transfer_node,
                                            ATTRIBUTE(FW_TRANSFER_EXPIRY_TIME),
                                            &new_expiry,
                                            sizeof(new_expiry));
}

/////////////////////////////////////////////////////////////////////////
// Public functions
/////////////////////////////////////////////////////////////////////////
sl_status_t command_class_firmware_update_initiate_firmware_update(
  zwave_node_id_t node_id,
  zwave_endpoint_id_t endpoint_id,
  uint32_t firmware_target,
  unsigned long apply_after,
  const char *file_name)
{
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_id_node(node_id, endpoint_id);
  attribute_store_node_t firmware_update_node
    = attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE(FWU));

  // Interview not completed
  if (firmware_update_node == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_warning(LOG_TAG,
                   "Attempt to firmware update NodeID/endpoint %d:%d  before "
                   "the end of the interview.",
                   node_id,
                   endpoint_id);
    return SL_STATUS_FAIL;
  }

  // Some update already ongoing.
  if (is_firmware_update_ongoing(endpoint_node) == true) {
    sl_log_warning(LOG_TAG,
                   "Attempt to firmware update NodeID/endpoint %d:%d "
                   "already performing a firmware update. "
                   "Please abort the ongoing update first.",
                   node_id,
                   endpoint_id);
    return SL_STATUS_FAIL;
  }

  attribute_store_node_t firmware_transfer_node
    = attribute_store_get_first_child_by_type(firmware_update_node,
                                              ATTRIBUTE(FW_TRANSFER));
  if (firmware_transfer_node == ATTRIBUTE_STORE_INVALID_NODE) {
    firmware_transfer_node
      = create_firmware_transfer_attribute(firmware_update_node);
  }

  // Set the Firmware target.
  if (is_firmware_target_valid(endpoint_node, firmware_target) == false) {
    sl_log_warning(LOG_TAG,
                   "Attempt to firmware update NodeID/endpoint %d:%d "
                   "with an invalid Firmware Target (%d). Ignoring",
                   node_id,
                   endpoint_id,
                   firmware_target);
    return SL_STATUS_FAIL;
  }
  sl_status_t status
    = attribute_store_set_child_reported(firmware_transfer_node,
                                         ATTRIBUTE(FW_TRANSFER_TARGET),
                                         &firmware_target,
                                         sizeof(firmware_target));

  // Set the apply after timestamp.
  status |= attribute_store_set_child_reported(
    firmware_transfer_node,
    ATTRIBUTE(FW_TRANSFER_APPLY_TIMESTAMP),
    &apply_after,
    sizeof(apply_after));

  // Transfer offset, start at 0
  status |= set_transfer_offset(firmware_transfer_node, 0);

  // Filename to use for firmware transfer
  status |= set_firmware_transfer_file(firmware_transfer_node, file_name);

  // File size
  uint32_t file_size = (uint32_t)get_file_size(file_name);
  if (file_size == 0) {
    sl_log_warning(LOG_TAG,
                   "Firmware file %s is empty or does not exist! "
                   "Aborting firmware update attempt.",
                   file_name);
    return SL_STATUS_FAIL;
  }
  status |= attribute_store_set_child_reported(firmware_transfer_node,
                                               ATTRIBUTE(FW_TRANSFER_SIZE),
                                               &file_size,
                                               sizeof(file_size));

  // Last status is Success for now.
  status |= set_firmware_transfer_last_status(firmware_transfer_node, SUCCESS);
  // Copy the fragment size from the capabilities.
  status |= set_firmware_transfer_fragment_size(firmware_transfer_node);

  // If anything went wrong, bail out.
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Error while configuring Firmware Update data. Aborting");
    return status;
  }

  // Finally, tell the resolver to get started with initiating a Firmware Update
  // Delay the callback trigger if the node does not support activation
  clock_time_t duration = 0;
  long time_difference  = calculate_utc_current_time_difference(apply_after);
  if (node_supports_firmware_activation(endpoint_node) == false
      && time_difference > 0) {
    duration = time_difference * CLOCK_CONF_SECOND;
  }

  // Undefine the expiry time. It is configured first when the transfer starts
  status
    |= attribute_store_set_child_reported(firmware_transfer_node,
                                          ATTRIBUTE(FW_TRANSFER_EXPIRY_TIME),
                                          NULL,
                                          0);

  attribute_timeout_set_callback(firmware_transfer_node,
                                 duration,
                                 &start_firmware_update_operation);
  sl_log_debug(
    LOG_TAG,
    "Initiating Firmware update operation for NodeID/endpoint %d:%d in %d ms",
    node_id,
    endpoint_id,
    duration);
  return status;
}

void command_class_firmware_update_abort_ongoing_firmware_operation(
  zwave_node_id_t node_id, zwave_endpoint_id_t endpoint_id)
{
  sl_log_info(LOG_TAG,
              "Aborting Firmware update operation for NodeID/endpoint %d:%d",
              node_id,
              endpoint_id);

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_id_node(node_id, endpoint_id);
  attribute_store_node_t firmware_update_node
    = attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE(FWU));
  attribute_store_node_t firmware_transfer_node
    = attribute_store_get_first_child_by_type(firmware_update_node,
                                              ATTRIBUTE(FW_TRANSFER));

  abort_transfer_if_ongoing(firmware_transfer_node);
}
