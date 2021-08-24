/******************************************************************************
* # License
* <b>Copyright 2021  Silicon Laboratories Inc. www.silabs.com</b>
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
#include "zwave_command_class_multi_channel_association.h"
#include "zwave_command_class_association.h"
#include "zwave_command_class_association_internals.hpp"
#include "zwave_command_class_agi.h"
#include "zwave_command_classes_utils.h"
#include "zwave_controller_command_class_indices.h"

// Generic includes
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// Includes from other components
#include "ZW_classcmd.h"
#include "zwave_command_handler.h"
#include "zwave_controller_connection_info.h"
#include "zwave_controller_utils.h"
#include "zwave_network_management.h"
#include "zwave_rx.h"
#include "zpc_attribute_resolver.h"
#include "zwave_unid.h"

// Includes from UIC Components
#include "attribute_store.h"
#include "attribute.hpp"
#include "attribute_store_helper.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_resolver.h"

#include "sl_log.h"
constexpr char LOG_TAG[] = "zwave_command_class_multi_channel_association";

using namespace attribute_store;

///////////////////////////////////////////////////////////////////////////////
// Private definitions
///////////////////////////////////////////////////////////////////////////////
// Note, Multi Channel Association shares attributes with Association
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_##type

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Command Handler functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t
  zwave_command_class_multi_channel_association_handle_supported_groupings_report_command(
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
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(SUPPORTED_GROUPINGS),
                                             0);
  attribute_store_set_reported(group_count_node,
                               &group_count,
                               sizeof(group_count));

  attribute_store_node_t group_node;
  association_group_id_t group_identifer;
  for (association_group_id_t i = 0; i < group_count; i++) {
    group_node = attribute_store_get_node_child_by_type(endpoint_node,
                                                        ATTRIBUTE(GROUP_ID),
                                                        i);

    if (group_node == ATTRIBUTE_STORE_INVALID_NODE) {
      group_node = attribute_store_add_node(ATTRIBUTE(GROUP_ID), endpoint_node);
      group_identifer = i + 1;
      attribute_store_set_node_attribute_value(group_node,
                                               REPORTED_ATTRIBUTE,
                                               (uint8_t *)&group_identifer,
                                               sizeof(association_group_id_t));

      // Create a group content attribute under each group ID :
      attribute_store_add_node(ATTRIBUTE(GROUP_CONTENT), group_node);
      // Create the Max Nodes Supported attribute that represents
      // the maximum number of destinations supported by the advertised association group
      attribute_store_add_node(ATTRIBUTE(MAX_NODES_SUPPORTED), group_node);
    }
  }
  // We are done parsing the frame
  return SL_STATUS_OK;
}

static sl_status_t
  zwave_command_class_multi_channel_association_handle_set_command(
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
    // Empty payload for the set. We successfully add nothing
    return SL_STATUS_OK;
  }
  association_bytes bytes(&frame_data[GROUP_IDENTIFIER_INDEX + 1],
                          &frame_data[frame_length]);

  association_set list;
  get_association_list(bytes, list);

  sl_status_t command_status = SL_STATUS_OK;
  for (auto association: list) {
    command_status
      |= add_reported_association(zwave_network_management_get_node_id(),
                                  0,
                                  group_id,
                                  association);
  }
  if (command_status != SL_STATUS_OK) {
    sl_log_debug(LOG_TAG, "Could not set association properly!");
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

static sl_status_t
  zwave_command_class_multi_channel_association_handle_remove_command(
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

    get_association_list(bytes, list);
  }

  // Too short command or resulting empyt list:
  // It is a remove "all associations" case.
  if ((list.size() == 0) || (frame_length <= 3)) {
    if (group_id != 0) {
      sl_log_debug(LOG_TAG,
                   "Erasing all associations for Group ID %d ",
                   group_id);
      return remove_all_reported_associations(
        zwave_network_management_get_node_id(),
        0,
        group_id);
    }
    // Remove all for all groups.
    sl_status_t removal_status = SL_STATUS_OK;
    sl_log_debug(LOG_TAG, "Erasing all associations for all Group IDs ");
    for (association_group_id_t i = 1; i <= NUMBER_OF_SUPPORTED_GROUPS; i++) {
      removal_status |= remove_all_reported_associations(
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

static sl_status_t
  zwave_command_class_multi_channel_association_handle_get_command(
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
  report.command_class       = COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3;
  report.command             = MULTI_CHANNEL_ASSOCIATION_REPORT_V3;
  report.grouping_identifier = (uint8_t)group_id;
  report.max_nodes_supported = (uint8_t)group_capacity;
  report.reports_to_follow   = RESERVED_BYTE;

  uint8_t group_content_size = 0;
  attribute_store_node_t group_content_node
    = get_group_content_node(zpc_node_id, 0, group_id);
  attribute_store_get_node_attribute_value(group_content_node,
                                           REPORTED_ATTRIBUTE,
                                           report.group_content,
                                           &group_content_size);

  uint8_t report_length
    = offsetof(association_report_frame_t, group_content) + group_content_size;
  return zwave_command_class_send_report(connection,
                                         report_length,
                                         (uint8_t *)&report);
}

static sl_status_t
  zwave_command_class_multi_channel_association_handle_report_command(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  if (frame_length < REPORT_ASSOCIATION_BYTES_INDEX) {
    // Frame too short?? Just ignore it and return SUCCESS
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
    = attribute_store_get_node_child_by_type(group_id_node,
                                             ATTRIBUTE(MAX_NODES_SUPPORTED),
                                             0);
  attribute_store_set_reported(group_capacity_node,
                               &capacity,
                               sizeof(capacity));

  attribute_store_node_t group_content_node
    = attribute_store_get_node_child_by_type(group_id_node,
                                             ATTRIBUTE(GROUP_CONTENT),
                                             0);

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
  get_association_list(bytes, list);
  for (auto association: list) {
    add_reported_association(connection_info->remote.node_id,
                             connection_info->remote.endpoint_id,
                             association_report->grouping_identifier,
                             association);
  }

  return SL_STATUS_OK;
}

static sl_status_t
  zwave_command_class_multi_channel_association_handle_groupings_get_command(
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
  report.command_class       = COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3;
  report.command             = MULTI_CHANNEL_ASSOCIATION_GROUPINGS_REPORT_V3;
  report.supported_groupings = (uint8_t)group_count;

  return zwave_command_class_send_report(connection,
                                         sizeof(report),
                                         (uint8_t *)&report);
}

static sl_status_t
  zwave_command_class_multi_channel_association_support_handler(
    const zwave_controller_connection_info_t *connection,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame_data[COMMAND_CLASS_INDEX]
      != COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case MULTI_CHANNEL_ASSOCIATION_SET_V3:
      return zwave_command_class_multi_channel_association_handle_set_command(
        connection,
        frame_data,
        frame_length);

    case MULTI_CHANNEL_ASSOCIATION_GET_V3:
      return zwave_command_class_multi_channel_association_handle_get_command(
        connection,
        frame_data,
        frame_length);

    case MULTI_CHANNEL_ASSOCIATION_REMOVE_V3:
      return zwave_command_class_multi_channel_association_handle_remove_command(
        connection,
        frame_data,
        frame_length);

    case MULTI_CHANNEL_ASSOCIATION_GROUPINGS_GET_V3:
      return zwave_command_class_multi_channel_association_handle_groupings_get_command(
        connection,
        frame_data,
        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

static sl_status_t
  zwave_command_class_multi_channel_association_control_handler(
    const zwave_controller_connection_info_t *connection,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame_data[COMMAND_CLASS_INDEX]
      != COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case MULTI_CHANNEL_ASSOCIATION_REPORT_V3:
      return zwave_command_class_multi_channel_association_handle_report_command(
        connection,
        frame_data,
        frame_length);

    case MULTI_CHANNEL_ASSOCIATION_GROUPINGS_REPORT_V3:
      return zwave_command_class_multi_channel_association_handle_supported_groupings_report_command(
        connection,
        frame_data,
        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute resolution functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_multi_channel_association_groupings_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Verify that we can use Multi Channel Assocation
  zwave_node_id_t node_id         = 0;
  zwave_endpoint_id_t endpoint_id = 0;
  if (SL_STATUS_OK
      != attribute_store_network_helper_get_zwave_ids_from_node(node,
                                                                &node_id,
                                                                &endpoint_id)) {
    *frame_length = 0;
    return SL_STATUS_FAIL;
  }
  // If no, fall back on Association
  if (zwave_node_get_command_class_version(
        COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
        node_id,
        endpoint_id)
      < 2) {
    return zwave_command_class_association_groupings_get(node,
                                                         frame,
                                                         frame_length);
  }

  zwave_minimum_frame_t *group_get_frame = (zwave_minimum_frame_t *)frame;
  group_get_frame->command_class = COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3;
  group_get_frame->command       = MULTI_CHANNEL_ASSOCIATION_GROUPINGS_GET_V3;
  *frame_length                  = sizeof(zwave_minimum_frame_t);
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_multi_channel_association_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Verify that we can use Multi Channel Assocation
  zwave_node_id_t node_id         = 0;
  zwave_endpoint_id_t endpoint_id = 0;
  if (SL_STATUS_OK
      != attribute_store_network_helper_get_zwave_ids_from_node(node,
                                                                &node_id,
                                                                &endpoint_id)) {
    *frame_length = 0;
    return SL_STATUS_FAIL;
  }
  // If no, fall back on Association
  if (zwave_node_get_command_class_version(
        COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
        node_id,
        endpoint_id)
      < 2) {
    return zwave_command_class_association_get(node, frame, frame_length);
  }
  // Read group identifier
  attribute_store_node_t group_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(GROUP_ID));
  association_group_id_t group_id = 0;
  attribute_store_read_value(group_id_node,
                             REPORTED_ATTRIBUTE,
                             (uint8_t *)&group_id,
                             sizeof(group_id));

  association_get_frame_t *get_frame = (association_get_frame_t *)frame;
  get_frame->command_class       = COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3;
  get_frame->command             = MULTI_CHANNEL_ASSOCIATION_GET_V3;
  get_frame->grouping_identifier = (uint8_t)group_id;
  *frame_length                  = sizeof(association_get_frame_t);
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_multi_channel_association_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
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
    attribute_store_set_reported_as_desired(node);
    return SL_STATUS_ALREADY_EXISTS;
  }

  // Verify that we can use Multi Channel Assocation
  // If no, fall back on Association
  if (zwave_node_get_command_class_version(
        COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
        node_id,
        endpoint_id)
      < 2) {
    return zwave_command_class_association_set(node, frame, frame_length);
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
    get_association_list(desired_bytes, desired_list);
    association_bytes reported_bytes
      = group.get<association_bytes>(REPORTED_ATTRIBUTE);
    get_association_list(reported_bytes, reported_list);
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
    remove_frame->command_class = COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3;
    remove_frame->command       = MULTI_CHANNEL_ASSOCIATION_REMOVE_V3;
    remove_frame->grouping_identifier = (uint8_t)grouping_identifier;

    association_bytes remove_bytes;
    get_association_bytestream(associations_to_remove, remove_bytes);
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
    set_frame->command_class       = COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3;
    set_frame->command             = MULTI_CHANNEL_ASSOCIATION_SET_V3;
    set_frame->grouping_identifier = (uint8_t)grouping_identifier;

    association_bytes add_bytes;
    get_association_bytestream(associations_to_add, add_bytes);
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

///////////////////////////////////////////////////////////////////////////////
// Attribute Resolver callback functions
///////////////////////////////////////////////////////////////////////////////
static void on_group_content_send_data_complete(attribute_store_node_t node,
                                                resolver_rule_type_t rule_type,
                                                zpc_resolver_event_t event)
{
  if (rule_type == RESOLVER_GET_RULE) {
    return;
  }
  switch (event) {
    case FRAME_SENT_EVENT_OK_SUPERVISION_WORKING:
      // Wait for more. Weird to have a working Supervision for Association
      break;

    case FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS:
    case FRAME_SENT_EVENT_OK_NO_SUPERVISION:
      attribute_store_undefine_reported(node);
      break;

    case FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT:
    case FRAME_SENT_EVENT_OK_SUPERVISION_FAIL:
    default:
      attribute_store_undefine_desired(node);
      attribute_store_undefine_reported(node);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute update callbacks
///////////////////////////////////////////////////////////////////////////////
static void
  zwave_command_class_multi_channel_association_on_version_attribute_update(
    attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  // Check that we have the right type of attribute.
  assert(ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_VERSION
         == attribute_store_get_node_type(updated_node));

  uint8_t supporting_node_version = 0;
  attribute_store_get_reported(updated_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Wait for the version to be known.
    return;
  }

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  // Let the rest of the command class perform the job.
  attribute_store_type_t attribute_list[] = {ATTRIBUTE(SUPPORTED_GROUPINGS)};
  attribute_store_add_if_missing(endpoint_node,
                                 attribute_list,
                                 COUNT_OF(attribute_list));
}

static void on_nif_attribute_update(attribute_store_node_t updated_node,
                                    attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  if (true
      == attribute_store_is_value_defined(updated_node, REPORTED_ATTRIBUTE)) {
    return;
  }

  // Fresh NIF created/updated but still undefined, it means we are interviewing.

  // Find the parent endpoint and verify the lifeline setup when the interview
  // is over.
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  attribute_resolver_set_resolution_listener(endpoint_node,
                                             establish_lifeline_association);
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
void establish_lifeline_association(attribute_store_node_t endpoint_node)
{
  // Remove the resolution listener, so we don't establish the lifeline several
  // times
  attribute_resolver_clear_resolution_listener(endpoint_node,
                                               establish_lifeline_association);

  zwave_node_id_t node_id         = 0;
  zwave_endpoint_id_t endpoint_id = 0;
  if (SL_STATUS_OK
      != attribute_store_network_helper_get_zwave_ids_from_node(endpoint_node,
                                                                &node_id,
                                                                &endpoint_id)) {
    sl_log_error(LOG_TAG, "Can't get Z-Wave IDs from node %d", endpoint_node);
    return;
  }

  for (association_group_id_t i = 1;
       i <= get_number_of_groups(node_id, endpoint_id);
       i++) {
    // Check the group profile, associate ourselves if it is reported (or considered)
    // a lifeline profile group.
    // We also accept the N/A profile, some Z-Wave Plus devices do that.
    // Note: We do not check if the destination is Z-Wave Plus, profile data is enough
    agi_profile_t profile
      = zwave_command_class_agi_get_group_profile(node_id, endpoint_id, i);
    if (AGI_LIFELINE_PROFILE != profile && AGI_NA_PROFILE != profile) {
      continue;
    }

    // That will prevent to associate ourselves to endpoints > 0,
    // or any group that does not allow any association
    if (get_group_capacity(node_id, endpoint_id, i) == 0) {
      sl_log_debug(LOG_TAG,
                   "Not establishing lifeline for Node %d:%d Group ID %d as "
                   "max supported associations is 0",
                   node_id,
                   endpoint_id,
                   i);
      continue;
    }

    zwave_node_id_t zpc_node_id = zwave_network_management_get_node_id();
    if (zpc_node_id > 255) {
      // we cannot associate ourselve Assocation does not handle NodeIDs > 255.
      sl_log_warning(LOG_TAG,
                     "ZPC NodeID = %d. Cannot "
                     "establish associations towards the ZPC.",
                     zpc_node_id);
      return;
    }

    association_t new_association = {};
    new_association.node_id       = zpc_node_id;
    new_association.endpoint_id   = 0;
    new_association.type          = NODE_ID;

    // Do we want an Endpoint Association ?
    if (true
          == zwave_node_supports_command_class(COMMAND_CLASS_MULTI_CHANNEL_V4,
                                               node_id,
                                               endpoint_id)
        && zwave_node_get_command_class_version(
             COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
             node_id,
             endpoint_id)
             >= 3) {
      new_association.type = ENDPOINT;
    }
    // Here we set the return route since ZPC is assigning a lifeline destination to itself
    zwave_network_management_assign_return_route(node_id, zpc_node_id);

    if (zwave_network_management_is_zpc_sis() == true) {
      /// CL:008E.01.51.05.1: Bump somebody out only if we are the SIS.
      force_add_desired_association(node_id, endpoint_id, i, new_association);
    } else {
      add_desired_association(node_id, endpoint_id, i, new_association);
    }
  }
}

sl_status_t zwave_command_class_multi_channel_association_init()
{
  attribute_store_register_callback_by_type(
    zwave_command_class_multi_channel_association_on_version_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_VERSION);

  attribute_store_register_callback_by_type(on_nif_attribute_update,
                                            ATTRIBUTE_ZWAVE_NIF);

  // Multi channel is the one registering the rules, and calling the
  // association rules if the supporting node cannot use multi channel
  attribute_resolver_register_rule(
    ATTRIBUTE(SUPPORTED_GROUPINGS),
    NULL,
    zwave_command_class_multi_channel_association_groupings_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(GROUP_CONTENT),
    zwave_command_class_multi_channel_association_set,
    zwave_command_class_multi_channel_association_get);

  // Attribute Resolver event listener:
  register_send_event_handler(ATTRIBUTE(GROUP_CONTENT),
                              &on_group_content_send_data_complete);

  // Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.support_handler
    = &zwave_command_class_multi_channel_association_support_handler;
  handler.control_handler
    = &zwave_command_class_multi_channel_association_control_handler;
  handler.minimal_scheme = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  handler.command_class  = COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3;
  // TODO: I think we should put v4 here, as soon as we are able to discover security keys.
  handler.version                    = MULTI_CHANNEL_ASSOCIATION_VERSION_V3;
  handler.command_class_name         = "Multi Channel Association";
  handler.manual_security_validation = false;

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}
