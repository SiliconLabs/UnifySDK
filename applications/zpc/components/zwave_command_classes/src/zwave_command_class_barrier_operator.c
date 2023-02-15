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
#include "zwave_command_class_barrier_operator.h"

#include "zwave_command_classes_utils.h"

// Interfaces
#include "zwave_command_class_barrier_operator_types.h"

// Generic includes
#include <stdlib.h>

// Includes from other ZPC Components
#include "ZW_classcmd.h"
#include "attribute_store_defined_attribute_types.h"
#include "zpc_attribute_store_network_helper.h"
#include "zwave_command_class_indices.h"
#include "zwave_command_handler.h"
#include "zpc_attribute_resolver.h"

// Includes from other Unify Components
#include "dotdot_mqtt.h"
#include "dotdot_mqtt_generated_commands.h"
#include "attribute_resolver.h"
#include "attribute_store_helper.h"
#include "sl_log.h"

// Unify Components
#include "attribute_timeouts.h"
#include "attribute_store_helper.h"

// Log tag
#define LOG_TAG "zwave_command_class_barrier_operator"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_BARRIER_OPERATOR_##type

#define TARGET_VALUE_INDEX             2
#define STATE_INDEX                    2
#define SUPPORTED_REPORT_BITMASK_INDEX 2
#define SUBSYSTEM_TYPE_INDEX           2
#define SUBSYSTEM_STATE_INDEX          3

#define CLOSING_STATE 0xFC
#define OPENING_STATE 0xFE

///////////////////////////////////////////////////////////////////////////////
// Resolution functions
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Generates a Barrier Operator Get Command.
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_barrier_operator_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  (void)node;
  ZW_BARRIER_OPERATOR_GET_FRAME *get_frame
    = (ZW_BARRIER_OPERATOR_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_BARRIER_OPERATOR;
  get_frame->cmd      = BARRIER_OPERATOR_GET;
  *frame_length       = sizeof(ZW_BARRIER_OPERATOR_GET_FRAME);
  return SL_STATUS_OK;
}

/**
 * @brief Generates a Barrier Operator Set Command.
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_barrier_operator_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  barrier_operator_state_t target_value = 0;
  attribute_store_get_desired_else_reported(node,
                                            &target_value,
                                            sizeof(target_value));

  ZW_BARRIER_OPERATOR_SET_FRAME *set_frame
    = (ZW_BARRIER_OPERATOR_SET_FRAME *)frame;
  set_frame->cmdClass    = COMMAND_CLASS_BARRIER_OPERATOR;
  set_frame->cmd         = BARRIER_OPERATOR_SET;
  set_frame->targetValue = target_value;
  *frame_length          = sizeof(ZW_BARRIER_OPERATOR_SET_FRAME);
  return SL_STATUS_OK;
}

/**
 * @brief Generates a Barrier Operator Supported Get Command.
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_barrier_operator_supported_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  /*This command is used to query a device for available subsystems which
  may be controlled via Z-Wave.*/
  (void)node;
  ZW_BARRIER_OPERATOR_SIGNAL_SUPPORTED_GET_FRAME *get_frame
    = (ZW_BARRIER_OPERATOR_SIGNAL_SUPPORTED_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_BARRIER_OPERATOR;
  get_frame->cmd      = BARRIER_OPERATOR_SIGNAL_SUPPORTED_GET;
  *frame_length       = sizeof(ZW_BARRIER_OPERATOR_SIGNAL_SUPPORTED_GET_FRAME);
  return SL_STATUS_OK;
}

/**
 * @brief Generates a Barrier Operator Signal Set Command.
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_barrier_operator_signal_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  barrier_operator_subsystem_type_t subsystem_type   = 0;
  barrier_operator_subsystem_state_t subsystem_state = 0;

  attribute_store_node_t subsystem_type_node
    = attribute_store_get_first_parent_with_type(node,
                                                 ATTRIBUTE(SUBSYSTEM_TYPE));
  attribute_store_get_reported(subsystem_type_node,
                               &subsystem_type,
                               sizeof(subsystem_type));
  attribute_store_get_desired_else_reported(node,
                                            &subsystem_state,
                                            sizeof(subsystem_state));

  if (subsystem_type == 0) {
    sl_log_error(LOG_TAG,
                 "Invalid type found while searching for Subsystem type value. "
                 "Deleting attribute ID %d",
                 subsystem_type_node);
    attribute_store_delete_node(subsystem_type_node);
    return SL_STATUS_ALREADY_EXISTS;
  }

  ZW_BARRIER_OPERATOR_SIGNAL_SET_FRAME *set_frame
    = (ZW_BARRIER_OPERATOR_SIGNAL_SET_FRAME *)frame;
  set_frame->cmdClass       = COMMAND_CLASS_BARRIER_OPERATOR;
  set_frame->cmd            = BARRIER_OPERATOR_SIGNAL_SET;
  set_frame->subsystemType  = subsystem_type;
  set_frame->subsystemState = subsystem_state;
  *frame_length             = sizeof(ZW_BARRIER_OPERATOR_SIGNAL_SET_FRAME);
  return SL_STATUS_OK;
}

/**
 * @brief Generates a Barrier Operator Signal Get Command.
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_barrier_operator_signal_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  attribute_store_node_t subsystem_type_node
    = attribute_store_get_first_parent_with_type(node,
                                                 ATTRIBUTE(SUBSYSTEM_TYPE));
  barrier_operator_subsystem_type_t type = 0;
  attribute_store_get_reported(subsystem_type_node, &type, sizeof(type));

  if (type == 0) {
    sl_log_error(LOG_TAG,
                 "Invalid type found while searching for Subsystem type value. "
                 "Deleting attribute ID %d",
                 subsystem_type_node);
    attribute_store_delete_node(subsystem_type_node);
    return SL_STATUS_ALREADY_EXISTS;
  }

  ZW_BARRIER_OPERATOR_SIGNAL_GET_FRAME *get_frame
    = (ZW_BARRIER_OPERATOR_SIGNAL_GET_FRAME *)frame;
  get_frame->cmdClass      = COMMAND_CLASS_BARRIER_OPERATOR;
  get_frame->cmd           = BARRIER_OPERATOR_SIGNAL_GET;
  get_frame->subsystemType = type;
  *frame_length            = sizeof(ZW_BARRIER_OPERATOR_SIGNAL_GET_FRAME);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Frame parsing functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Handles incoming Barrier Operator Set Commands.
 *
 * It will convert it into a Generated GoToPercent command
 *
 * @param connection_info   Connection information with the sender
 * @param frame             Pointer to frame data
 * @param frame_length      Number of bytes contained in the frame
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_barrier_operator_handle_set(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= TARGET_VALUE_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  unid_t node_unid;
  zwave_unid_from_node_id(connection_info->remote.node_id, node_unid);
  dotdot_endpoint_id_t endpoint_id = connection_info->remote.endpoint_id;

  barrier_operator_state_t target_value = frame_data[TARGET_VALUE_INDEX];

  const uic_mqtt_dotdot_barrier_control_command_go_to_percent_fields_t target
    = {.percent_open = target_value};
  if ((target_value < 100) || (target_value == 255)) {
    uic_mqtt_dotdot_barrier_control_publish_generated_go_to_percent_command(
      node_unid,
      endpoint_id,
      &target);
    return SL_STATUS_OK;
  }
  return SL_STATUS_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// Frame parsing functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Handle incoming Barrier Operator Report Commands
 *
 * @param connection_info   zwave_controller_connection_info_t object.
 * @param frame             Pointer to frame data
 * @param frame_length      Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t zwave_command_class_barrier_operator_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // We expect to have at least 5 byte of value
  if (frame_length <= STATE_INDEX) {
    return SL_STATUS_OK;
  }

  // Find the Endpoint
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  // Type
  barrier_operator_state_t state = 0;
  state                          = frame_data[STATE_INDEX];

  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(STATE),
                                     &state,
                                     sizeof(state));
  attribute_store_node_t state_node
    = attribute_store_get_node_child(endpoint_node, ATTRIBUTE(STATE));
  if ((state >= CLOSING_STATE) && (state <= OPENING_STATE)) {
    attribute_timeout_set_callback(state_node,
                                   15 * CLOCK_CONF_SECOND,
                                   &attribute_store_undefine_reported);
  } else {
    attribute_timeout_cancel_callback(state_node,
                                      &attribute_store_undefine_reported);
    attribute_store_set_desired_as_reported(state_node);
  }
  return SL_STATUS_OK;
}

/**
 * @brief Handle incoming Barrier Operator Singal Report Commands
 *
 * @param connection_info   zwave_controller_connection_info_t object.
 * @param frame             Pointer to frame data
 * @param frame_length      Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t zwave_command_class_barrier_operator_handle_signal_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= SUBSYSTEM_STATE_INDEX) {
    return SL_STATUS_OK;
  }

  // Find the Endpoint
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  // Subsystem Type
  barrier_operator_subsystem_type_t type = 0;
  type                                   = frame_data[SUBSYSTEM_TYPE_INDEX];

  attribute_store_node_t type_node
    = attribute_store_get_node_child_by_value(endpoint_node,
                                              ATTRIBUTE(SUBSYSTEM_TYPE),
                                              REPORTED_ATTRIBUTE,
                                              &type,
                                              sizeof(type),
                                              0);
  // Subsystem State
  barrier_operator_subsystem_state_t subsystem_state_value
    = frame_data[SUBSYSTEM_STATE_INDEX];

  attribute_store_set_child_reported(type_node,
                                     ATTRIBUTE(SUBSYSTEM_STATE),
                                     &subsystem_state_value,
                                     sizeof(subsystem_state_value));
  return SL_STATUS_OK;
}

/**
 * @brief Handle incoming Barrier Operator Supported Report Commands
 *
 * @param connection_info   zwave_controller_connection_info_t object.
 * @param frame             Pointer to frame data
 * @param frame_length      Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t zwave_command_class_barrier_operator_handle_supported_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Find the Endpoint
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  attribute_store_node_t bitmask_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(BITMASK));

  // No bitmask given, we set a 0 in the attribute store.
  if (frame_length <= SUPPORTED_REPORT_BITMASK_INDEX) {
    const uint8_t empty_bitmask = 0;
    attribute_store_set_reported(bitmask_node,
                                 &empty_bitmask,
                                 sizeof(empty_bitmask));
    return SL_STATUS_OK;
  }

  uint8_t number_of_bytes
    = ((uint8_t)frame_length) - SUPPORTED_REPORT_BITMASK_INDEX;

  // Create subsytem types attribute for all supported types.
  barrier_operator_subsystem_type_t type = 0;
  for (uint8_t byte = 0; byte < number_of_bytes; byte++) {
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (frame_data[byte + SUPPORTED_REPORT_BITMASK_INDEX] & (1 << bit)) {
        type = (byte * 8) + bit + 1;
        attribute_store_node_t type_node
          = attribute_store_emplace(endpoint_node,
                                    ATTRIBUTE(SUBSYSTEM_TYPE),
                                    &type,
                                    sizeof(type));
        const attribute_store_type_t type_attribute[]
          = {ATTRIBUTE(SUBSYSTEM_STATE)};
        attribute_store_add_if_missing(type_node,
                                       type_attribute,
                                       COUNT_OF(type_attribute));
      }
    }
  }

  // Copy the bitmask value in the attribute store.
  attribute_store_set_node_attribute_value(
    bitmask_node,
    REPORTED_ATTRIBUTE,
    &frame_data[SUPPORTED_REPORT_BITMASK_INDEX],
    number_of_bytes);

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Store callback functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Attribute callback function listening for Barrier Operator
 *        version attribute updates.
 *
 * @param updated_node    Attribute Store node that was modified.
 * @param change          Type of change applied to the node.
 */
static void zwave_command_class_barrier_operator_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(
        COMMAND_CLASS_BARRIER_OPERATOR,
        updated_node)
      == true) {
    return;
  }

  // Do not create the attributes until we are sure of the version
  uint8_t supporting_node_version = 0;
  attribute_store_get_reported(updated_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Wait that the version becomes non-zero.
    return;
  }

  // Now we know we have a Barrier Operator supporting endpoint.
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  const attribute_store_type_t v1_attributes[]
    = {ATTRIBUTE(BITMASK), ATTRIBUTE(STATE)};
  attribute_store_add_if_missing(endpoint_node,
                                 v1_attributes,
                                 COUNT_OF(v1_attributes));
}

///////////////////////////////////////////////////////////////////////////////
// Incoming commands handler
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_barrier_operator_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Frame too short, it should have not come here.
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame_data[COMMAND_CLASS_INDEX] != COMMAND_CLASS_BARRIER_OPERATOR) {
    return SL_STATUS_FAIL;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case BARRIER_OPERATOR_SET:
      // Note: This command is not supported, but we want to indicate
      // to the application that we received it.
      return zwave_command_class_barrier_operator_handle_set(connection_info,
                                                             frame_data,
                                                             frame_length);

    case BARRIER_OPERATOR_SIGNAL_SUPPORTED_REPORT:
      return zwave_command_class_barrier_operator_handle_supported_report(
        connection_info,
        frame_data,
        frame_length);
    case BARRIER_OPERATOR_REPORT:
      return zwave_command_class_barrier_operator_handle_report(connection_info,
                                                                frame_data,
                                                                frame_length);
    case BARRIER_OPERATOR_SIGNAL_REPORT:
      return zwave_command_class_barrier_operator_handle_signal_report(
        connection_info,
        frame_data,
        frame_length);
    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_barrier_operator_init()
{
  // Attribute resolver rules
  attribute_resolver_register_rule(
    ATTRIBUTE(BITMASK),
    NULL,
    &zwave_command_class_barrier_operator_supported_get);

  attribute_resolver_register_rule(ATTRIBUTE(STATE),
                                   &zwave_command_class_barrier_operator_set,
                                   &zwave_command_class_barrier_operator_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(SUBSYSTEM_STATE),
    &zwave_command_class_barrier_operator_signal_set,
    &zwave_command_class_barrier_operator_signal_get);

  // Attribute store callbacks
  attribute_store_register_callback_by_type(
    &zwave_command_class_barrier_operator_on_version_attribute_update,
    ATTRIBUTE(VERSION));

  // The support side of things: Register our handler to the Z-Wave CC
  // framework:
  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler
    = &zwave_command_class_barrier_operator_control_handler;
  // Not supported, so this does not really matter
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_BARRIER_OPERATOR;
  handler.version                    = BARRIER_OPERATOR_VERSION;
  handler.command_class_name         = "Barrier Operator";
  handler.comments                   = "";

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}
