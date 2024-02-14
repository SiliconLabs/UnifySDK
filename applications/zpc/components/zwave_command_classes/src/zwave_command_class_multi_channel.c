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
#include "zwave_command_class_multi_channel.h"

// Generic includes
#include <assert.h>
#include <stdbool.h>
#include <string.h>

// Includes from other components
#include "ZW_classcmd.h"
#include "zwave_command_handler.h"
#include "zwave_controller_connection_info.h"
#include "zwave_controller_utils.h"
#include "zwave_command_class_indices.h"
#include "zwave_rx.h"

#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_resolver.h"

#include "zwave_unid.h"

#include "sl_log.h"
#define LOG_TAG "zwave_command_class_multi_channel"

// Frame parsing defines
// Multi Channel End Point Report
#define REPORT_IDENTICAL_ENDPOINTS_MASK   0x40
#define REPORT_IDENTICAL_ENDPOINTS_INDEX  2
#define REPORT_INDIVIDUAL_ENDPOINTS_INDEX 3
#define REPORT_AGGREGATED_ENDPOINTS_INDEX 4

// Multi Channel Capability Report
#define CAPABILITY_REPORT_ENDPOINT_INDEX              2
#define CAPABILITY_REPORT_GENERIC_DEVICE_CLASS_INDEX  3
#define CAPABILITY_REPORT_SPECIFIC_DEVICE_CLASS_INDEX 4
#define CAPABILITY_REPORT_COMMAND_CLASS_LIST_INDEX    5

// Multi Channel End Point Find Report
#define CAPABILITY_REPORT_ENDPOINT_LIST_INDEX 5

// Multi Channel Aggregated Member Report
#define AGGREGATED_MEMBER_REPORT_ENDPOINT_INDEX            2
#define AGGREGATED_MEMBER_REPORT_NUMBER_OF_BIT_MASKS_INDEX 3
#define AGGREGATED_MEMBER_REPORT_FIRST_BITMASK_INDEX       4

// Attributes for Multi Channel nodes, under endpoint 0
static const attribute_store_type_t v3_attributes[]
  = {ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_IDENTICAL_ENDPOINTS,
     ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_INDIVIDUAL_ENDPOINTS,
     ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_ALL_INDIVIDUAL_ENDPOINTS_FOUND};

static const attribute_store_type_t v4_attributes[]
  = {ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_ENDPOINTS};

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Creates a new endpoint in the attribute store
 *
 * Creation will be skipped if the endpoint already exists.
 *
 * @param node_id_node        The attribute store node that represents the NodeID
 * @param endpoint_id         Endpoint ID to create for the node.
 * @param is_aggregated       is the endpoint aggregated?
 *
 * @returns attribute_store_node_t of the endpoint created (or that already exists).
 */
static attribute_store_node_t
  create_new_endpoint(attribute_store_node_t node_id_node,
                      zwave_endpoint_id_t endpoint_id,
                      bool is_aggregated)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_node_child_by_value(node_id_node,
                                              ATTRIBUTE_ENDPOINT_ID,
                                              REPORTED_ATTRIBUTE,
                                              &endpoint_id,
                                              sizeof(endpoint_id),
                                              0);
  if (endpoint_node != ATTRIBUTE_STORE_INVALID_NODE) {
    return endpoint_node;
  }

  endpoint_node = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, node_id_node);

  attribute_store_set_node_attribute_value(endpoint_node,
                                           REPORTED_ATTRIBUTE,
                                           &endpoint_id,
                                           sizeof(endpoint_id));

  const attribute_store_type_t new_attributes[]
    = {ATTRIBUTE_ZWAVE_NIF,
       ATTRIBUTE_ZWAVE_GENERIC_DEVICE_CLASS,
       ATTRIBUTE_ZWAVE_SPECIFIC_DEVICE_CLASS};
  attribute_store_add_if_missing(endpoint_node,
                                 new_attributes,
                                 COUNT_OF(new_attributes));

  if (is_aggregated == true) {
    const attribute_store_type_t aggregated_members_attribute[]
      = {ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_MEMBERS};
    attribute_store_add_if_missing(endpoint_node,
                                   aggregated_members_attribute,
                                   COUNT_OF(aggregated_members_attribute));
  }

  return endpoint_node;
}

/**
 * @brief Creates aggregated endpoints for a Z-Wave Node
 *
 * Call this function only when all the individual endpoints have been
 * discovered. It will verify how many aggregated endpoints are reported
 * under the endpoint 0 "number of aggregated endpoints" attribute, then
 * create them in a consecutive range after the last individual endpoint.
 *
 * @param node_id_node    The attribute store node that represents the NodeID
 */
static void create_aggregated_endpoints(attribute_store_node_t node_id_node)
{
  zwave_endpoint_id_t root_device = 0;
  attribute_store_node_t root_device_node
    = attribute_store_get_node_child_by_value(node_id_node,
                                              ATTRIBUTE_ENDPOINT_ID,
                                              REPORTED_ATTRIBUTE,
                                              &root_device,
                                              sizeof(zwave_endpoint_id_t),
                                              0);
  attribute_store_node_t aggregated_eps_node
    = attribute_store_get_first_child_by_type(
      root_device_node,
      ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_ENDPOINTS);

  uint8_t aggregated_endpoints = 0;
  attribute_store_read_value(aggregated_eps_node,
                             REPORTED_ATTRIBUTE,
                             &aggregated_endpoints,
                             sizeof(aggregated_endpoints));

  zwave_endpoint_id_t last_individual_endpoint = 0;
  attribute_store_node_t current_endpoint_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ENDPOINT_ID,
                                             last_individual_endpoint);
  while (ATTRIBUTE_STORE_INVALID_NODE != current_endpoint_node) {
    if (true == is_endpoint_aggregated(current_endpoint_node)) {
      // something went wrong. Abort
      sl_log_warning(LOG_TAG,
                     "Aggregated endpoints already present. Aborting new "
                     "aggregated endpoint creation for NodeID Attribute ID %d",
                     node_id_node);
      return;
    }
    last_individual_endpoint += 1;
    current_endpoint_node
      = attribute_store_get_node_child_by_type(node_id_node,
                                               ATTRIBUTE_ENDPOINT_ID,
                                               last_individual_endpoint);
  }

  for (uint8_t i = 0; i < aggregated_endpoints; i++) {
    // Create the endpoint, assign its value and an aggregated member list
    create_new_endpoint(node_id_node, last_individual_endpoint + i, true);
  }
}

/**
 * @brief Counts in the attribute store if the number of individual endpoints
 * attribute matches the current list of individual endpoints.
 *
 * It will write the value 1 to the
 * ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_ALL_INDIVIDUAL_ENDPOINTS_FOUND
 * attribute under endpoint 0 when it is the case.
 *
 * @param node_id_node    The attribute store node that represents the NodeID
 */
static void check_if_all_individual_endpoints_are_found(
  attribute_store_node_t node_id_node)
{
  zwave_endpoint_id_t root_device_endpoint = 0;
  attribute_store_node_t root_device_node
    = attribute_store_get_node_child_by_value(node_id_node,
                                              ATTRIBUTE_ENDPOINT_ID,
                                              REPORTED_ATTRIBUTE,
                                              &root_device_endpoint,
                                              sizeof(root_device_endpoint),
                                              0);
  attribute_store_node_t individual_endpoint_count
    = attribute_store_get_first_child_by_type(
      root_device_node,
      ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_INDIVIDUAL_ENDPOINTS);

  uint8_t number_of_individual_ep = 0;
  if (SL_STATUS_OK
      != attribute_store_read_value(individual_endpoint_count,
                                    REPORTED_ATTRIBUTE,
                                    &number_of_individual_ep,
                                    sizeof(number_of_individual_ep))) {
    return;
  }

  // Now count manually in the attribute store, and compare
  uint32_t child_index          = 0;
  uint8_t attribute_store_count = 0;
  attribute_store_node_t current_endpoint_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ENDPOINT_ID,
                                             child_index);
  while (current_endpoint_node != ATTRIBUTE_STORE_INVALID_NODE) {
    if (current_endpoint_node != root_device_node
        && is_endpoint_aggregated(current_endpoint_node) == false) {
      attribute_store_count += 1;
    }
    child_index += 1;
    current_endpoint_node
      = attribute_store_get_node_child_by_type(node_id_node,
                                               ATTRIBUTE_ENDPOINT_ID,
                                               child_index);
  }

  if (number_of_individual_ep <= attribute_store_count) {
    attribute_store_node_t endpoint_found_node
      = attribute_store_get_first_child_by_type(
        root_device_node,
        ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_ALL_INDIVIDUAL_ENDPOINTS_FOUND);
    uint8_t found = 1;
    attribute_store_set_reported(endpoint_found_node, &found, sizeof(found));
    create_aggregated_endpoints(node_id_node);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Command Handler functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t
  zwave_command_class_multi_channel_handle_endpoint_report_command(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  // Find the NodeID in the attribute store.
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(
      connection_info->remote.node_id);

  // Save this data under the root device, aka endpoint 0
  zwave_endpoint_id_t root_device_endpoint = 0;
  attribute_store_node_t endpoint_node
    = attribute_store_get_node_child_by_value(node_id_node,
                                              ATTRIBUTE_ENDPOINT_ID,
                                              REPORTED_ATTRIBUTE,
                                              &root_device_endpoint,
                                              sizeof(root_device_endpoint),
                                              0);

  // Save the data from this command, first Identical endpoints
  uint8_t field_value = 0;
  if (frame_length > REPORT_IDENTICAL_ENDPOINTS_INDEX) {
    field_value = frame_data[REPORT_IDENTICAL_ENDPOINTS_INDEX]
                  & REPORT_IDENTICAL_ENDPOINTS_MASK;
  }
  attribute_store_node_t identical_node
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_IDENTICAL_ENDPOINTS);
  attribute_store_set_reported(identical_node,
                               &field_value,
                               sizeof(field_value));

  // Number of individual endpoints
  field_value = 0;
  if (frame_length > REPORT_INDIVIDUAL_ENDPOINTS_INDEX) {
    field_value = frame_data[REPORT_INDIVIDUAL_ENDPOINTS_INDEX] & ENDPOINT_MASK;
  }
  attribute_store_node_t individual_endpoints_node
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_INDIVIDUAL_ENDPOINTS);
  attribute_store_set_reported(individual_endpoints_node,
                               &field_value,
                               sizeof(field_value));

  // Number of aggregated endpoints
  field_value = 0;
  if (frame_length > REPORT_AGGREGATED_ENDPOINTS_INDEX) {
    field_value = frame_data[REPORT_AGGREGATED_ENDPOINTS_INDEX] & ENDPOINT_MASK;
  }
  attribute_store_node_t aggregated_endpoints_node
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_ENDPOINTS);
  attribute_store_set_reported(aggregated_endpoints_node,
                               &field_value,
                               sizeof(field_value));

  return SL_STATUS_OK;
}

static sl_status_t
  zwave_command_class_multi_channel_handle_capability_report_command(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  if (frame_length <= CAPABILITY_REPORT_COMMAND_CLASS_LIST_INDEX) {
    // Frame too short?? Just ignore it and return SUCCESS to supervision
    // Since it is a report.
    return SL_STATUS_OK;
  }

  // Find the NodeID in the attribute store.
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(
      connection_info->remote.node_id);

  // Find the Endpoint ID
  zwave_endpoint_id_t endpoint
    = (frame_data[CAPABILITY_REPORT_ENDPOINT_INDEX] & ENDPOINT_MASK);
  attribute_store_node_t endpoint_node
    = attribute_store_get_node_child_by_value(node_id_node,
                                              ATTRIBUTE_ENDPOINT_ID,
                                              REPORTED_ATTRIBUTE,
                                              &endpoint,
                                              sizeof(endpoint),
                                              0);

  attribute_store_node_t endpoint_generic_device_class
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE_ZWAVE_GENERIC_DEVICE_CLASS);
  attribute_store_set_node_attribute_value(
    endpoint_generic_device_class,
    REPORTED_ATTRIBUTE,
    &frame_data[CAPABILITY_REPORT_GENERIC_DEVICE_CLASS_INDEX],
    sizeof(uint8_t));

  attribute_store_node_t endpoint_specific_device_class
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE_ZWAVE_SPECIFIC_DEVICE_CLASS);
  attribute_store_set_node_attribute_value(
    endpoint_specific_device_class,
    REPORTED_ATTRIBUTE,
    &frame_data[CAPABILITY_REPORT_SPECIFIC_DEVICE_CLASS_INDEX],
    sizeof(uint8_t));

  attribute_store_node_t endpoint_nif
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE_ZWAVE_NIF);
  attribute_store_set_node_attribute_value(
    endpoint_nif,
    REPORTED_ATTRIBUTE,
    &frame_data[CAPABILITY_REPORT_COMMAND_CLASS_LIST_INDEX],
    (uint8_t)(frame_length - CAPABILITY_REPORT_COMMAND_CLASS_LIST_INDEX));

  return SL_STATUS_OK;
}

static sl_status_t
  zwave_command_class_multi_channel_handle_endpoint_find_report_command(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  if (frame_length < CAPABILITY_REPORT_ENDPOINT_LIST_INDEX) {
    // Frame too short?? Just ignore it and return SUCCESS to supervision
    // Since it is a report.
    return SL_STATUS_OK;
  }

  // Find the NodeID in the attribute store.
  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(
      connection_info->remote.node_id);

  // Report to follow is ignored. If the node sends us more reports, they will
  // be accepted and processed here again.

  // Create those endpoints!
  for (uint8_t index = CAPABILITY_REPORT_ENDPOINT_LIST_INDEX;
       index < frame_length;
       index++) {
    zwave_endpoint_id_t current_endpoint = frame_data[index] & ENDPOINT_MASK;
    create_new_endpoint(node_id_node, current_endpoint, false);
  }
  check_if_all_individual_endpoints_are_found(node_id_node);

  return SL_STATUS_OK;
}

static sl_status_t
  zwave_command_class_multi_channel_handle_aggregated_members_report_command(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  if (frame_length <= AGGREGATED_MEMBER_REPORT_ENDPOINT_INDEX) {
    // Frame too short?? Just ignore it and return SUCCESS to supervision
    // Since it is a report.
    return SL_STATUS_OK;
  }

  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(
      connection_info->remote.node_id);

  zwave_endpoint_id_t aggregated_endpoint
    = frame_data[AGGREGATED_MEMBER_REPORT_ENDPOINT_INDEX] & ENDPOINT_MASK;
  uint8_t aggregated_members_length = 0;
  if (frame_length > AGGREGATED_MEMBER_REPORT_NUMBER_OF_BIT_MASKS_INDEX) {
    aggregated_members_length
      = frame_data[AGGREGATED_MEMBER_REPORT_NUMBER_OF_BIT_MASKS_INDEX];
  }

  attribute_store_node_t endpoint_node
    = attribute_store_get_node_child_by_value(node_id_node,
                                              ATTRIBUTE_ENDPOINT_ID,
                                              REPORTED_ATTRIBUTE,
                                              &aggregated_endpoint,
                                              sizeof(aggregated_endpoint),
                                              0);

  attribute_store_node_t aggregated_members_node
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_MEMBERS);

  if (aggregated_members_length == 0) {
    // oh oh, we just asked aggregated members to an individual (or non-existing)
    // endpoint. Ensure that it is not marked as aggregated in the attribute
    // store
    attribute_store_delete_node(aggregated_members_node);
    return SL_STATUS_OK;
  }

  if (aggregated_members_length
      > frame_length - AGGREGATED_MEMBER_REPORT_FIRST_BITMASK_INDEX) {
    aggregated_members_length
      = (uint8_t)(frame_length - AGGREGATED_MEMBER_REPORT_FIRST_BITMASK_INDEX);
  }

  attribute_store_set_reported(
    aggregated_members_node,
    &(frame_data[AGGREGATED_MEMBER_REPORT_FIRST_BITMASK_INDEX]),
    aggregated_members_length);

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_multi_channel_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame_data[COMMAND_CLASS_INDEX] != COMMAND_CLASS_MULTI_CHANNEL_V4) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[1]) {
    case MULTI_CHANNEL_END_POINT_REPORT_V4:
      return zwave_command_class_multi_channel_handle_endpoint_report_command(
        connection_info,
        frame_data,
        frame_length);

    case MULTI_CHANNEL_CAPABILITY_REPORT_V4:
      return zwave_command_class_multi_channel_handle_capability_report_command(
        connection_info,
        frame_data,
        frame_length);

    case MULTI_CHANNEL_END_POINT_FIND_REPORT_V4:
      return zwave_command_class_multi_channel_handle_endpoint_find_report_command(
        connection_info,
        frame_data,
        frame_length);

    case MULTI_CHANNEL_AGGREGATED_MEMBERS_REPORT_V4:
      return zwave_command_class_multi_channel_handle_aggregated_members_report_command(
        connection_info,
        frame_data,
        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute resolution functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_multi_channel_endpoint_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_len)
{
  // Create a frame for the attribute resolver
  ZW_MULTI_CHANNEL_END_POINT_GET_V4_FRAME *get_frame
    = (ZW_MULTI_CHANNEL_END_POINT_GET_V4_FRAME *)frame;

  get_frame->cmdClass = COMMAND_CLASS_MULTI_CHANNEL_V4;
  get_frame->cmd      = MULTI_CHANNEL_END_POINT_GET_V4;

  *frame_len = sizeof(ZW_MULTI_CHANNEL_END_POINT_GET_V4_FRAME);
  return SL_STATUS_OK;
}

// This function is public, the node Info resolver is in charge of calling it.
sl_status_t zwave_command_class_multi_channel_capability_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_len)
{
  // This function resolves a ATTRIBUTE_ZWAVE_NIF attribute under an End Point
  // that has an ID > 0.
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  zwave_endpoint_id_t endpoint_id = 0;
  attribute_store_read_value(endpoint_node,
                             REPORTED_ATTRIBUTE,
                             &endpoint_id,
                             sizeof(endpoint_id));

  if (endpoint_id == 0) {
    *frame_len = 0;
    return SL_STATUS_NOT_SUPPORTED;
  }

  ZW_MULTI_CHANNEL_CAPABILITY_GET_V4_FRAME *get_frame
    = (ZW_MULTI_CHANNEL_CAPABILITY_GET_V4_FRAME *)frame;
  get_frame->cmdClass    = COMMAND_CLASS_MULTI_CHANNEL_V4;
  get_frame->cmd         = MULTI_CHANNEL_CAPABILITY_GET_V4;
  get_frame->properties1 = endpoint_id & ENDPOINT_MASK;

  *frame_len = sizeof(ZW_MULTI_CHANNEL_CAPABILITY_GET_V4_FRAME);
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_multi_channel_endpoint_find(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_len)
{
  uint8_t generic_device_class = 0;
  uint8_t specific_device_class = 0;

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  sl_status_t result = attribute_store_get_child_reported(
    endpoint_node,
    ATTRIBUTE_ZWAVE_GENERIC_DEVICE_CLASS,
    &generic_device_class,
    sizeof(generic_device_class));

  if (result != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "Can't find generic device class. Setting to default 0xff");
    generic_device_class = 0xFF; 
  }

 result = attribute_store_get_child_reported(
    endpoint_node,
    ATTRIBUTE_ZWAVE_SPECIFIC_DEVICE_CLASS,
    &specific_device_class,
    sizeof(specific_device_class));

  if (result != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "Can't find specific device class. Setting to default 0xff");
    specific_device_class = 0xFF; 
  }

  // Create a frame for the attribute resolver
  ZW_MULTI_CHANNEL_END_POINT_FIND_V4_FRAME *get_frame
    = (ZW_MULTI_CHANNEL_END_POINT_FIND_V4_FRAME *)frame;
  get_frame->cmdClass            = COMMAND_CLASS_MULTI_CHANNEL_V4;
  get_frame->cmd                 = MULTI_CHANNEL_END_POINT_FIND_V4;
  get_frame->genericDeviceClass  = generic_device_class;
  get_frame->specificDeviceClass = specific_device_class;

  *frame_len = sizeof(ZW_MULTI_CHANNEL_END_POINT_FIND_V4_FRAME);
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_multi_channel_aggregated_members_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_len)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  zwave_endpoint_id_t endpoint_id = 0;
  attribute_store_read_value(endpoint_node,
                             REPORTED_ATTRIBUTE,
                             &endpoint_id,
                             sizeof(endpoint_id));

  if (endpoint_id == 0) {
    *frame_len = 0;
    return SL_STATUS_NOT_SUPPORTED;
  }

  ZW_MULTI_CHANNEL_AGGREGATED_MEMBERS_GET_V4_FRAME *get_frame
    = (ZW_MULTI_CHANNEL_AGGREGATED_MEMBERS_GET_V4_FRAME *)frame;
  get_frame->cmdClass    = COMMAND_CLASS_MULTI_CHANNEL_V4;
  get_frame->cmd         = MULTI_CHANNEL_AGGREGATED_MEMBERS_GET_V4;
  get_frame->properties1 = endpoint_id & ENDPOINT_MASK;

  *frame_len = sizeof(ZW_MULTI_CHANNEL_AGGREGATED_MEMBERS_GET_V4_FRAME);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute update callbacks
///////////////////////////////////////////////////////////////////////////////
static void zwave_command_class_multi_channel_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  // Check that we have the right type of attribute.
  assert(ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_VERSION
         == attribute_store_get_node_type(updated_node));

  uint8_t supporting_node_version = 0;
  attribute_store_read_value(updated_node,
                             REPORTED_ATTRIBUTE,
                             &supporting_node_version,
                             sizeof(supporting_node_version));

  if (supporting_node_version < 3) {
    // can't do anything if version is unknown or <=2
    return;
  }

  // Now we know we have a supporting node.
  // Check that the minimum attributes are created
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  // We do not create any attribute if an endpoint > 0 claims to support
  // Multi Channel. This will prevent interviewing loops.
  zwave_endpoint_id_t endpoint_id = 0;
  attribute_store_read_value(endpoint_node,
                             REPORTED_ATTRIBUTE,
                             &endpoint_id,
                             sizeof(zwave_endpoint_id_t));
  if (endpoint_id != 0) {
    return;
  }

  attribute_store_add_if_missing(endpoint_node,
                                 v3_attributes,
                                 COUNT_OF(v3_attributes));

  if (supporting_node_version >= 4) {
    attribute_store_add_if_missing(endpoint_node,
                                   v4_attributes,
                                   COUNT_OF(v4_attributes));
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
bool is_endpoint_aggregated(attribute_store_node_t endpoint_node)
{
  const attribute_store_node_t aggregated_members_node
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_MEMBERS);

  return (aggregated_members_node != ATTRIBUTE_STORE_INVALID_NODE);
}

sl_status_t zwave_command_class_multi_channel_init()
{
  // Attribute resolver rule registration
  sl_status_t init_status = SL_STATUS_OK;

  init_status |= attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_IDENTICAL_ENDPOINTS,
    NULL,
    &zwave_command_class_multi_channel_endpoint_get);

  init_status |= attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_ALL_INDIVIDUAL_ENDPOINTS_FOUND,
    NULL,
    &zwave_command_class_multi_channel_endpoint_find);

  init_status |= attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_MEMBERS,
    NULL,
    &zwave_command_class_multi_channel_aggregated_members_get);

  // Attribute update listeners
  init_status |= attribute_store_register_callback_by_type(
    &zwave_command_class_multi_channel_on_version_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_VERSION);

  // Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  handler.command_class      = COMMAND_CLASS_MULTI_CHANNEL_V4;
  handler.version            = MULTI_CHANNEL_VERSION;
  handler.command_class_name = "Multi Channel";
  handler.manual_security_validation = false;
  handler.support_handler            = NULL;
  handler.control_handler = &zwave_command_class_multi_channel_control_handler;

  init_status |= zwave_command_handler_register_handler(handler);

  return init_status;
}
