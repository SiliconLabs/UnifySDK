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

// Includes from this component
#include "zwave_command_class_association.h"
#include "zwave_command_class_association_internals.h"
#include "zwave_command_class_multi_channel_association.h"
#include "zwave_command_class_agi.h"
#include "zwave_command_classes_utils.h"
#include "zwave_command_class_indices.h"

// Generic includes
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include <algorithm>
#include <vector>

// Unify Includes
#include "sl_log.h"
#include "attribute.hpp"
#include "attribute_store_helper.h"
#include "attribute_resolver.h"

// Contiki includes
#include "ctimer.h"

// ZPC Components
#include "zwave_association_toolbox.hpp"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "zwave_unid.h"
#include "zwave_tx.h"
#include "zwave_controller_keyset.h"
#include "zwave_tx_scheme_selector.h"
#include "zwave_command_handler.h"

// Interfaces
#include "ZW_classcmd.h"

// Log tag
#define LOG_TAG "zwave_command_association"

// Attribute macro, shortening those long defines:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_##type

using namespace attribute_store;

// Declare our timer object
static struct ctimer association_clean_up_timer;

// How long we will let the resolver try to clean up associations (ms)
constexpr int ASSOCIATION_CLEAN_UP_TIMEOUT = 1500;

// Minimum 2 attributes to create under a Group ID when created:
constexpr attribute_store_type_t group_attributes[]
  = {ATTRIBUTE(GROUP_CONTENT), ATTRIBUTE(MAX_NODES_SUPPORTED)};

///////////////////////////////////////////////////////////////////////////////
// Private functions, used to handle individual incoming commands.
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Handles incoming Association Supported Groupings Report Commands
 */
static sl_status_t
  zwave_command_class_association_handle_groupings_report_command(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  if (frame_length < GROUPING_REPORT_MINIMUM_LENGTH) {
    // Frame too short?? Just ignore it and return SUCCESS to supervision
    // Since it is a report.
    return SL_STATUS_OK;
  }

  // Find out the unid of the sending node:
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  association_group_count_t group_count
    = frame_data[GROUPING_REPORT_NUMBER_OF_GROUPS_INDEX];
  attribute_store_node_t group_count_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(SUPPORTED_GROUPINGS));
  attribute_store_set_reported(group_count_node,
                               &group_count,
                               sizeof(group_count));

  attribute_store_node_t group_node;
  for (association_group_id_t i = 1; i <= group_count; i++) {
    group_node = attribute_store_emplace(endpoint_node,
                                         ATTRIBUTE(GROUP_ID),
                                         &i,
                                         sizeof(i));

    attribute_store_add_if_missing(group_node,
                                   group_attributes,
                                   COUNT_OF(group_attributes));
  }
  // We are done parsing the frame
  return SL_STATUS_OK;
}

// Handles incoming Association Report Commands considering
// there are one report frame.
// FIXME: to Handle when Report to Follow field is not zero
static sl_status_t zwave_command_class_association_handle_report_command(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < REPORT_ASSOCIATION_BYTES_INDEX) {
    return SL_STATUS_OK;
  }

  // Find out the endpoint node of the sending node:
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  const association_report_frame_t *association_report
    = (association_report_frame_t *)frame_data;
  association_group_id_t group_id = association_report->grouping_identifier;
  attribute_store_node_t group_id_node
    = attribute_store_get_node_child_by_value(endpoint_node,
                                              ATTRIBUTE(GROUP_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&group_id,
                                              sizeof(association_group_id_t),
                                              0);

  association_group_capacity_t capacity
    = association_report->max_nodes_supported;
  attribute_store_node_t group_capacity_node
    = attribute_store_get_first_child_by_type(group_id_node,
                                              ATTRIBUTE(MAX_NODES_SUPPORTED));
  attribute_store_set_reported(group_capacity_node,
                               &capacity,
                               sizeof(capacity));

  attribute_store_node_t group_content_node
    = attribute_store_get_first_child_by_type(group_id_node,
                                              ATTRIBUTE(GROUP_CONTENT));

  if (frame_length <= REPORT_ASSOCIATION_BYTES_INDEX) {
    // Put an association marker to prevent re-resolution
    const uint8_t marker = ASSOCIATION_MARKER;
    attribute_store_set_reported(group_content_node, &marker, sizeof(marker));
    return SL_STATUS_OK;
  }
  // Else parse the list of associations.
  association_bytes bytes(&frame_data[REPORT_ASSOCIATION_BYTES_INDEX],
                          &frame_data[frame_length]);

  association_set list;
  get_node_id_association_list(bytes, list);

  // Convert back into bytes to save in the attribute store
  bytes.clear();
  get_association_bytestream(list, bytes);
  attribute group(group_content_node);
  group.set<association_bytes>(REPORTED_ATTRIBUTE, bytes);

  for (auto association: list) {
    // Make sure the node has return routes for all its destinations
    zwave_network_management_assign_return_route(
      connection_info->remote.node_id,
      association.node_id);
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_association_handle_set_command(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= GROUP_IDENTIFIER_INDEX) {
    return SL_STATUS_FAIL;
  }

  association_group_id_t group_id = frame_data[GROUP_IDENTIFIER_INDEX];
  if (group_id > NUMBER_OF_SUPPORTED_GROUPS) {
    return SL_STATUS_FAIL;
  }

  // Make a bytestream out of the frame payload
  if ((GROUP_IDENTIFIER_INDEX + 1) >= (frame_length)) {
    // Empty list, do not try to parse.
    return SL_STATUS_OK;
  }
  association_bytes bytes(&frame_data[GROUP_IDENTIFIER_INDEX + 1],
                          &frame_data[frame_length]);

  association_set list;
  get_node_id_association_list(bytes, list);

  sl_status_t command_status = SL_STATUS_OK;
  for (auto association: list) {
    command_status
      |= add_reported_association(zwave_network_management_get_node_id(),
                                  0,
                                  group_id,
                                  association);
  }
  if (command_status != SL_STATUS_OK) {
    sl_log_debug(LOG_TAG, "Could not set a NodeID association properly!");
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_association_handle_groupings_get_command(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (connection->local.is_multicast == true) {
    return SL_STATUS_OK;
  }

  zwave_node_id_t zpc_node_id = zwave_network_management_get_node_id();
  association_group_count_t group_count = get_number_of_groups(zpc_node_id, 0);

  association_grouping_report_frame_t report = {};
  report.command_class                       = COMMAND_CLASS_ASSOCIATION_V2;
  report.command                             = ASSOCIATION_GROUPINGS_REPORT_V2;
  report.supported_groupings                 = (uint8_t)group_count;

  return zwave_command_class_send_report(connection,
                                         sizeof(report),
                                         (uint8_t *)&report);
}

static sl_status_t zwave_command_class_association_handle_remove_command(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  association_group_id_t group_id = 0;
  if (frame_length > GROUP_IDENTIFIER_INDEX) {
    group_id = frame_data[GROUP_IDENTIFIER_INDEX];
  }
  if (group_id > NUMBER_OF_SUPPORTED_GROUPS) {
    return SL_STATUS_FAIL;
  }

  // Make a list out of the frame payload
  association_set list;
  if (frame_length > GROUP_IDENTIFIER_INDEX + 1) {
    uint16_t bytestream_length = frame_length - GROUP_IDENTIFIER_INDEX;
    association_bytes bytes(
      &frame_data[GROUP_IDENTIFIER_INDEX + 1],
      &frame_data[GROUP_IDENTIFIER_INDEX + bytestream_length]);

    get_node_id_association_list(bytes, list);
  }

  // Too short command or resulting empyt list:
  // It is a remove "all associations" case.
  if ((list.size() == 0) || (frame_length <= 3)) {
    if (group_id != 0) {
      sl_log_debug(LOG_TAG,
                   "Erasing all NodeID associations for Group ID %d ",
                   group_id);
      return remove_all_reported_node_id_associations(
        zwave_network_management_get_node_id(),
        0,
        group_id);
    }
    // Remove all for all groups.
    sl_status_t removal_status = SL_STATUS_OK;
    sl_log_debug(LOG_TAG, "Erasing all NodeID associations for all Group IDs ");
    for (association_group_id_t i = 1; i <= NUMBER_OF_SUPPORTED_GROUPS; i++) {
      removal_status |= remove_all_reported_node_id_associations(
        zwave_network_management_get_node_id(),
        0,
        i);
    }
    return removal_status;
  }

  // Else we just remove the list we were given.
  sl_status_t removal_status = SL_STATUS_OK;
  if (group_id == 0) {
    // Remove the association from all groups
    for (association_group_id_t i = 1; i <= NUMBER_OF_SUPPORTED_GROUPS; i++) {
      for (auto association: list) {
        removal_status
          |= remove_reported_association(zwave_network_management_get_node_id(),
                                         0,
                                         i,
                                         association);
      }
    }
  } else {
    // Remove only in the specified GroupID.
    for (auto association: list) {
      removal_status
        |= remove_reported_association(zwave_network_management_get_node_id(),
                                       0,
                                       group_id,
                                       association);
    }
  }
  return removal_status;
}

static sl_status_t zwave_command_class_association_handle_get_command(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (connection->local.is_multicast == true) {
    return SL_STATUS_OK;
  }
  association_group_id_t group_id = LIFELINE_GROUP_ID;
  if (frame_length > GROUP_IDENTIFIER_INDEX) {
    group_id = frame_data[GROUP_IDENTIFIER_INDEX];
  }

  if ((group_id == 0) || (group_id > NUMBER_OF_SUPPORTED_GROUPS)) {
    group_id = LIFELINE_GROUP_ID;
  }

  zwave_node_id_t zpc_node_id = zwave_network_management_get_node_id();
  association_group_capacity_t group_capacity
    = get_group_capacity(zpc_node_id, 0, group_id);

  association_report_frame_t report = {};
  report.command_class              = COMMAND_CLASS_ASSOCIATION;
  report.command                    = ASSOCIATION_REPORT;
  report.grouping_identifier        = (uint8_t)group_id;
  report.max_nodes_supported        = (uint8_t)group_capacity;
  report.reports_to_follow          = RESERVED_BYTE;

  association_set list;
  try {
    attribute group(get_group_content_node(zpc_node_id, 0, group_id));
    association_bytes bytes = group.get<association_bytes>(REPORTED_ATTRIBUTE);
    // Get only the NodeID part of the associations.
    get_node_id_association_list(bytes, list);
    // Convert back to bytes
  } catch (const std::exception &e) {
    sl_log_warning(LOG_TAG,
                   "Could not retrieve our Group ID %d content",
                   group_id);
  }
  association_bytes reported_bytes;
  get_node_id_association_bytestream(list, reported_bytes);
  // Copy to the report.
  memcpy(report.group_content, &reported_bytes[0], reported_bytes.size());
  uint8_t report_length = offsetof(association_report_frame_t, group_content)
                          + reported_bytes.size();
  return zwave_command_class_send_report(connection,
                                         report_length,
                                         (uint8_t *)&report);
}

static sl_status_t
  zwave_command_class_association_handle_specific_group_get_command(
    const zwave_controller_connection_info_t *connection,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  if (connection->local.is_multicast == true) {
    return SL_STATUS_OK;
  }

  association_specific_report_frame_t report = {};
  report.command_class                       = COMMAND_CLASS_ASSOCIATION_V2;
  report.command = ASSOCIATION_SPECIFIC_GROUP_REPORT_V2;
  report.group   = RESERVED_BYTE;

  return zwave_command_class_send_report(connection,
                                         sizeof(report),
                                         (uint8_t *)&report);
}

static sl_status_t zwave_command_class_association_support_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame_data[COMMAND_CLASS_INDEX] != COMMAND_CLASS_ASSOCIATION) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case ASSOCIATION_GET_V2:
      return zwave_command_class_association_handle_get_command(connection_info,
                                                                frame_data,
                                                                frame_length);

    case ASSOCIATION_GROUPINGS_GET_V2:
      return zwave_command_class_association_handle_groupings_get_command(
        connection_info,
        frame_data,
        frame_length);

    case ASSOCIATION_REMOVE_V2:
      return zwave_command_class_association_handle_remove_command(
        connection_info,
        frame_data,
        frame_length);

    case ASSOCIATION_SET_V2:
      return zwave_command_class_association_handle_set_command(connection_info,
                                                                frame_data,
                                                                frame_length);

    case ASSOCIATION_SPECIFIC_GROUP_GET_V2:
      return zwave_command_class_association_handle_specific_group_get_command(
        connection_info,
        frame_data,
        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

static sl_status_t zwave_command_class_association_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame_data[COMMAND_CLASS_INDEX] != COMMAND_CLASS_ASSOCIATION) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case ASSOCIATION_GROUPINGS_REPORT_V2:
      return zwave_command_class_association_handle_groupings_report_command(
        connection_info,
        frame_data,
        frame_length);

    case ASSOCIATION_REPORT_V2:
      return zwave_command_class_association_handle_report_command(
        connection_info,
        frame_data,
        frame_length);

    case ASSOCIATION_SPECIFIC_GROUP_REPORT_V2:
      /// FIXME: Implement
      return SL_STATUS_OK;

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

static void zwave_command_class_association_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }
  // Confirm that we have a ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_VERSION update
  assert(ATTRIBUTE(VERSION) == attribute_store_get_node_type(updated_node));

  uint8_t supporting_node_version = 0;
  attribute_store_get_reported(updated_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Wait for the version to be known.
    return;
  }

  // Now we know we have a Association CC supporting node.
  // All we need here is just a Supported Association Groups attribute.
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  const attribute_store_type_t attributes[] = {ATTRIBUTE(SUPPORTED_GROUPINGS)};
  attribute_store_add_if_missing(endpoint_node,
                                 attributes,
                                 COUNT_OF(attributes));
}

///////////////////////////////////////////////////////////////////////////////
// Attribute resolutions functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_association_groupings_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  assert(ATTRIBUTE(SUPPORTED_GROUPINGS) == attribute_store_get_node_type(node));
  zwave_minimum_frame_t *get_frame = (zwave_minimum_frame_t *)frame;
  get_frame->command_class         = COMMAND_CLASS_ASSOCIATION;
  get_frame->command               = ASSOCIATION_GROUPINGS_GET_V2;
  *frame_length                    = sizeof(zwave_minimum_frame_t);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_association_set(attribute_store_node_t node,
                                                uint8_t *frame,
                                                uint16_t *frame_length)
{
  zwave_node_id_t node_id         = 0;
  zwave_endpoint_id_t endpoint_id = 0;
  if (SL_STATUS_OK
      != attribute_store_network_helper_get_zwave_ids_from_node(node,
                                                                &node_id,
                                                                &endpoint_id)) {
    *frame_length = 0;
    return SL_STATUS_FAIL;
  }
  // Are we by accident trying to resolve ourselves?
  zwave_node_id_t zpc_node_id = zwave_network_management_get_node_id();
  if (zpc_node_id == node_id) {
    sl_log_warning(LOG_TAG,
                   "ZPC just updated its Association Group Content desired "
                   "value. This should not happen !");
    attribute_store_undefine_desired(node);
    return SL_STATUS_ALREADY_EXISTS;
  }
  // Else let's resolve it ourselves.
  attribute_store_node_t group_identifier_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(GROUP_ID));
  association_group_id_t grouping_identifier = 0;
  attribute_store_get_reported(group_identifier_node,
                               &grouping_identifier,
                               sizeof(grouping_identifier));
  // Compute the desired/reported lists
  association_set desired_list;
  association_set reported_list;
  try {
    attribute group(node);
    association_bytes desired_bytes
      = group.get<association_bytes>(DESIRED_ATTRIBUTE);
    get_node_id_association_list(desired_bytes, desired_list);
    association_bytes reported_bytes
      = group.get<association_bytes>(REPORTED_ATTRIBUTE);
    get_node_id_association_list(reported_bytes, reported_list);
  } catch (const std::exception &e) {
    sl_log_warning(LOG_TAG, "Could not compute Group content differences");
  }

  // Make a diff and see if we have to remove something
  association_set associations_to_remove;
  substract_association_lists(desired_list,
                              reported_list,
                              associations_to_remove);
  association_set associations_to_add;
  substract_association_lists(reported_list, desired_list, associations_to_add);

  if (associations_to_remove.size() > 0) {
    association_set_frame_t *remove_frame = (association_set_frame_t *)frame;
    remove_frame->command_class           = COMMAND_CLASS_ASSOCIATION;
    remove_frame->command                 = ASSOCIATION_REMOVE;
    remove_frame->grouping_identifier     = (uint8_t)grouping_identifier;

    association_bytes remove_bytes;
    get_node_id_association_bytestream(associations_to_remove, remove_bytes);
    std::copy(remove_bytes.begin(),
              remove_bytes.end(),
              remove_frame->group_content);
    *frame_length
      = offsetof(association_set_frame_t, group_content) + remove_bytes.size();

    if (associations_to_add.size() > 0) {
      return SL_STATUS_IN_PROGRESS;
    } else {
      return SL_STATUS_OK;
    }
  }

  if (associations_to_add.size() > 0) {
    association_set_frame_t *set_frame = (association_set_frame_t *)frame;
    set_frame->command_class           = COMMAND_CLASS_ASSOCIATION;
    set_frame->command                 = ASSOCIATION_SET;
    set_frame->grouping_identifier     = (uint8_t)grouping_identifier;

    association_bytes add_bytes;
    get_node_id_association_bytestream(associations_to_add, add_bytes);
    std::copy(add_bytes.begin(), add_bytes.end(), set_frame->group_content);
    *frame_length
      = offsetof(association_set_frame_t, group_content) + add_bytes.size();
    return SL_STATUS_OK;
  }

  sl_log_warning(
    LOG_TAG,
    "Nothing to add or remove from the association group, but they were still "
    "mismatched. Please use the helper functions to add/remove assocations.");
  attribute_store_log_node(node, false);
  attribute_store_set_desired_as_reported(node);
  return SL_STATUS_ALREADY_EXISTS;
}

// return a frame for an Association Get Command.
sl_status_t zwave_command_class_association_get(attribute_store_node_t node,
                                                uint8_t *frame,
                                                uint16_t *frame_length)
{
  // Read group identifier
  attribute_store_node_t group_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(GROUP_ID));
  association_group_id_t group_id;
  attribute_store_get_reported(group_id_node, &group_id, sizeof(group_id));

  association_get_frame_t *get_frame = (association_get_frame_t *)frame;
  get_frame->command_class           = COMMAND_CLASS_ASSOCIATION;
  get_frame->command                 = ASSOCIATION_GET_V2;
  get_frame->grouping_identifier     = (uint8_t)group_id;
  *frame_length                      = sizeof(association_get_frame_t);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Z-Wave Controller reset steps
///////////////////////////////////////////////////////////////////////////////
void zwave_command_class_association_clean_up_on_zpc_reset_complete(void *)
{
  sl_log_debug(LOG_TAG, "Reset step: Associations clean-up completed.");
  zwave_controller_on_reset_step_complete(
    ZWAVE_CONTROLLER_CLEAN_UP_ASSOCIATIONS_STEP_PRIORITY);
}

sl_status_t zwave_command_class_association_clean_up_on_zpc_reset()
{
  sl_log_info(LOG_TAG, "Reset step: Associations clean-up");
  // We are leaving the network, try to undefine the Lifeline associations
  remove_desired_node_id_from_all_associations_in_network(
    zwave_network_management_get_node_id());

  ctimer_set(&association_clean_up_timer,
             ASSOCIATION_CLEAN_UP_TIMEOUT
               + (zwave_network_management_get_network_size() * 300),
             &zwave_command_class_association_clean_up_on_zpc_reset_complete,
             nullptr);

  // We want to give a few seconds to the resolver to try setting stuff, before we
  // move on
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////

sl_status_t zwave_command_class_association_init()
{
  // Attribute Store callbacks
  attribute_store_register_callback_by_type(
    zwave_command_class_association_on_version_attribute_update,
    ATTRIBUTE(VERSION));

  // Tell the Z-Wave Controller that we have to do something on reset
  zwave_controller_register_reset_step(
    &zwave_command_class_association_clean_up_on_zpc_reset,
    ZWAVE_CONTROLLER_CLEAN_UP_ASSOCIATIONS_STEP_PRIORITY);

  // Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.support_handler    = &zwave_command_class_association_support_handler;
  handler.control_handler    = &zwave_command_class_association_control_handler;
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  handler.command_class      = COMMAND_CLASS_ASSOCIATION;
  handler.command_class_name = "Association";
  handler.version            = ASSOCIATION_VERSION_V2;
  handler.manual_security_validation = false;

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}