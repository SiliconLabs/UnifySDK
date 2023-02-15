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
#include "zwave_command_class_basic.h"
#include "zwave_command_classes_utils.h"

// Generic includes
#include <stdlib.h>
#include <assert.h>

// Includes from other ZPC Components
#include "zwave_command_class_indices.h"
#include "zwave_controller_utils.h"
#include "zwave_command_handler.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_tx.h"
#include "zwave_tx_scheme_selector.h"

// Includes from other Unify Components
#include "attribute_store_helper.h"
#include "attribute_resolver.h"
#include "attribute_timeouts.h"
#include "dotdot_mqtt_generated_commands.h"
#include "sl_log.h"

// Log tag
#define LOG_TAG "zwave_command_class_basic"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_BASIC_##type

// Version 1 attributes
static const attribute_store_type_t v1_attributes[]
  = {ATTRIBUTE(VALUE), ATTRIBUTE(VERSION)};

// Frame parsing indices:
#define REPORT_VALUE_INDEX        2
#define REPORT_TARGET_VALUE_INDEX 3
#define REPORT_DURATION_INDEX     4
#define SET_VALUE_INDEX           2

// Constants
#define ON  0xFF
#define OFF 0x00

// States of the Basic Probe
typedef enum {
  // Basic Get has never been sent to the node.
  NOT_REQUESTED = 0,
  // Basic Get has been sent to the node, no response (yet?).
  REQUESTED = 1,
  // Basic Get has been sent to the node, and it responded.
  ANSWERED = 2,
} basic_probe_status_t;

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief get the Basic probe status for an endpoint node
 * @param endpoint_id_node    Attribute Store node for the Endpoint ID
 * @returns basic_probe_status_t value of the probe status.
 */
static basic_probe_status_t
  get_basic_probe_status(attribute_store_node_t endpoint_id_node)
{
  // Can't find the attribute will mean that we did not ask for it.
  basic_probe_status_t probe_status = NOT_REQUESTED;

  attribute_store_node_t probe_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(PROBE_STATUS));
  attribute_store_get_reported(probe_node, &probe_status, sizeof(probe_status));
  return probe_status;
}

/**
 * @brief Sets the Basic probe status for an endpoint node
 * @param endpoint_id_node    Attribute Store node for the Endpoint ID
 * @param probe_status        Value of the probe status to set
 * @returns sl_status_t indicating if the value was set.
 */
static sl_status_t
  set_basic_probe_status(attribute_store_node_t endpoint_id_node,
                         basic_probe_status_t probe_status)
{
  if (attribute_store_get_node_type(endpoint_id_node)
      != ATTRIBUTE_ENDPOINT_ID) {
    sl_log_warning(
      LOG_TAG,
      "Trying to set the Probe Status under the wrong type of node.");
    attribute_store_log_node(endpoint_id_node, false);
    return SL_STATUS_FAIL;
  }
  attribute_store_node_t probe_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(PROBE_STATUS));

  if (probe_node == ATTRIBUTE_STORE_INVALID_NODE) {
    probe_node
      = attribute_store_add_node(ATTRIBUTE(PROBE_STATUS), endpoint_id_node);
  }

  return attribute_store_set_reported(probe_node,
                                      &probe_status,
                                      sizeof(probe_status));
}

/**
 * @brief  Checks if we can use Basic to control a node
 * @param nif_node    Attribute Store node for the NIF. It can be Secure or Non-secure NIF.
 * @returns true if Basic MUST NOT be used for this node. False if it MAY.
 */
static bool do_not_try_basic(attribute_store_node_t nif_node)
{
  uint8_t nif[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  uint8_t nif_length                                = 0;

  attribute_store_get_node_attribute_value(nif_node,
                                           REPORTED_ATTRIBUTE,
                                           nif,
                                           &nif_length);

  // Retrieve the CCs from the NIF to an array.
  zwave_command_class_t supported_command_classes[ZWAVE_MAX_NIF_SIZE] = {0};
  size_t supported_command_classes_length                             = 0;
  zwave_parse_nif(nif,
                  nif_length,
                  supported_command_classes,
                  &supported_command_classes_length,
                  NULL,
                  NULL);

  for (size_t i = 0; i < supported_command_classes_length; i++) {
    if (is_actuator_command_class(supported_command_classes[i])
        && zwave_command_handler_controls(supported_command_classes[i])) {
      // Actuator CC that we control. Do not try Basic.
      return true;
    }
  }
  return false;
}

/**
 * @brief  Sends a Basic Probe to an endpoint
 * @param endpoint_node    Attribute Store node for the Endpoint
 */
static void send_basic_probe(attribute_store_node_t endpoint_node)
{
  const uint8_t basic_get[] = {COMMAND_CLASS_BASIC_V2, BASIC_GET_V2};
  zwave_tx_options_t tx_options
    = {.number_of_responses = 1,
       .qos_priority        = ZWAVE_TX_QOS_RECOMMENDED_NODE_INTERVIEW_PRIORITY
                       + 10 * ZWAVE_TX_RECOMMENDED_QOS_GAP};
  zwave_controller_connection_info_t connection = {};
  zwave_node_id_t node_id                       = 0;
  zwave_endpoint_id_t endpoint_id               = 0;

  if (SL_STATUS_OK
      != attribute_store_network_helper_get_zwave_ids_from_node(endpoint_node,
                                                                &node_id,
                                                                &endpoint_id)) {
    sl_log_warning(
      LOG_TAG,
      "Failed to look up NodeID /Endpoint ID for attribute store node %d",
      endpoint_node);
    return;
  }

  zwave_tx_scheme_get_node_connection_info(node_id, endpoint_id, &connection);
  zwave_tx_send_data(&connection,
                     sizeof(basic_get),
                     basic_get,
                     &tx_options,
                     NULL,
                     NULL,
                     NULL);
  // Continue also if Tx failed. It will allow to accept incoming Basic Reports.
  sl_log_debug(LOG_TAG,
               "Sending a Basic Get Probe to NodeID:Endpoint %d:%d ",
               node_id,
               endpoint_id);
  // We are expecting a report, mark it in the attribute store.
  set_basic_probe_status(endpoint_node, REQUESTED);
}

///////////////////////////////////////////////////////////////////////////////
// Resolution functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_basic_get(attribute_store_node_t node,
                                                 uint8_t *frame,
                                                 uint16_t *frame_length)
{
  zwave_minimum_frame_t *get_frame = (zwave_minimum_frame_t *)frame;
  get_frame->command_class         = COMMAND_CLASS_BASIC_V2;
  get_frame->command               = BASIC_GET_V2;
  *frame_length                    = sizeof(zwave_minimum_frame_t);
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_basic_set(attribute_store_node_t node,
                                                 uint8_t *frame,
                                                 uint16_t *frame_length)
{
  // Check on the duration, if non-zero, don't do anything
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  attribute_store_node_t duration_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(DURATION));
  uint32_t duration = 0;
  attribute_store_get_reported(duration_node, &duration, sizeof(duration));
  if (duration != 0) {
    *frame_length = 0;
    return SL_STATUS_IS_WAITING;
  }

  // Else just use the desired value to set.
  uint32_t value = 0;
  attribute_store_get_desired(node, &value, sizeof(value));
  // For simplicity, we avoid using Start level Change / Stop level change.
  ZW_BASIC_SET_V2_FRAME *set_frame = (ZW_BASIC_SET_V2_FRAME *)frame;
  set_frame->cmdClass              = COMMAND_CLASS_BASIC_V2;
  set_frame->cmd                   = BASIC_SET_V2;
  set_frame->value                 = (uint8_t)value;
  *frame_length                    = sizeof(ZW_BASIC_SET_V2_FRAME);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Frame parsing functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Handles incoming Basic Set Commands.
 *
 * It will convert it into a Generated OnOff command (either On or Off)
 *
 * @param connection_info   Connection information with the sender
 * @param frame_data        Pointer to a frame data
 * @param frame_length      Length of the data that can be read in the frame_data
 *                          pointer.
 *
 * @return Always SL_STATUS_NOT_SUPPORTED as this command is not supported.
 */
static sl_status_t zwave_command_class_basic_handle_set(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // We expect to have at least 1 byte of value.
  if (frame_length <= SET_VALUE_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  unid_t node_unid;
  zwave_unid_from_node_id(connection_info->remote.node_id, node_unid);
  dotdot_endpoint_id_t endpoint_id = connection_info->remote.endpoint_id;

  // Indicate if the node is trying to set on or off.
  bool received_value = frame_data[SET_VALUE_INDEX];
  if (received_value) {
    uic_mqtt_dotdot_on_off_publish_generated_on_command(node_unid, endpoint_id);
  } else {
    uic_mqtt_dotdot_on_off_publish_generated_off_command(node_unid,
                                                         endpoint_id);
  }

  return SL_STATUS_NOT_SUPPORTED;
}

static sl_status_t zwave_command_class_basic_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // We expect to have at least 1 byte of value
  if (frame_length <= REPORT_VALUE_INDEX) {
    return SL_STATUS_FAIL;
  }

  // Find the Endpoint
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  basic_probe_status_t probe_status = get_basic_probe_status(endpoint_node);
  if (probe_status == NOT_REQUESTED) {
    // Somebody is sending Basic Report while we do not want to control it.
    return SL_STATUS_OK;
  }

  // Check the probe status
  if (probe_status == REQUESTED) {
    // Is that the first time we discover a Basic node ?
    attribute_store_add_if_missing(endpoint_node,
                                   v1_attributes,
                                   COUNT_OF(v1_attributes));
    probe_status = ANSWERED;
    set_basic_probe_status(endpoint_node, probe_status);
  }

  // Now save all the data reported by the node
  attribute_store_node_t value_node
    = attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE(VALUE));
  uint32_t current_value = frame_data[REPORT_VALUE_INDEX];
  if (current_value >= 1 && current_value <= 100) {
    current_value = ON;
  }
  if ((current_value == OFF) || (current_value == ON)) {
    attribute_store_set_reported(value_node,
                                 &current_value,
                                 sizeof(current_value));
  }

  // Make sure to cancel previous value probes
  attribute_timeout_cancel_callback(value_node,
                                    &attribute_store_undefine_reported);

  // Duration:
  uint32_t duration = 0;  // Assumed value if no duration is provided.
  if (frame_length > REPORT_DURATION_INDEX) {
    attribute_store_node_t duration_node
      = attribute_store_get_first_child_by_type(endpoint_node,
                                                ATTRIBUTE(DURATION));
    if (duration_node == ATTRIBUTE_STORE_INVALID_NODE
        && probe_status == ANSWERED) {
      duration_node
        = attribute_store_add_node(ATTRIBUTE(DURATION), endpoint_node);
    }
    duration = frame_data[REPORT_DURATION_INDEX];
    attribute_store_undefine_desired(duration_node);
    attribute_store_set_reported(duration_node, &duration, sizeof(duration));
  }

  // Is there a target value:
  if (frame_length > REPORT_TARGET_VALUE_INDEX) {
    uint32_t target_value = frame_data[REPORT_TARGET_VALUE_INDEX];
    // Same as for current value, sanitize the range.
    if (target_value >= 1 && target_value <= 100) {
      target_value = ON;
    }
    if ((target_value == OFF) || (target_value == ON)) {
      attribute_store_set_desired(value_node,
                                  &target_value,
                                  sizeof(target_value));
    }
  } else {
    attribute_store_undefine_desired(value_node);
  }

  // Current/target value adjustments based on the reported duration.
  if (duration == 0) {
    // Ensure no mismatch between reported/desired if duration is 0.
    if (attribute_store_is_desired_defined(value_node)) {
      attribute_store_set_reported_as_desired(value_node);
      attribute_store_undefine_desired(value_node);
    }
  }
  if (duration > 0) {
    // here we want to probe the value again after that duration.
    attribute_timeout_set_callback(value_node,
                                   zwave_duration_to_time((uint8_t)duration)
                                     + PROBE_BACK_OFF,
                                   &attribute_store_undefine_reported);
  }

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Resolver callback functions
///////////////////////////////////////////////////////////////////////////////
static void
  on_endpoint_interview_completed(attribute_store_node_t endpoint_node)
{
  // Do not ever try again for this endpoint.
  attribute_resolver_clear_resolution_listener(
    endpoint_node,
    &on_endpoint_interview_completed);

  // First off, verify if we should abstain to create a Command Class for the root device
  if (is_zwave_command_class_filtered_for_root_device(COMMAND_CLASS_BASIC_V2,
                                                      endpoint_node)
      == true) {
    return;
  }

  // Next stop, look at the NIFs:
  // First Non-secure
  attribute_store_node_t nif_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE_ZWAVE_NIF);
  if (do_not_try_basic(nif_node) == true) {
    return;
  }
  // Then secure
  nif_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE_ZWAVE_SECURE_NIF);
  if (do_not_try_basic(nif_node) == true) {
    return;
  }

  // Okay at that point we can try to probe basic.
  // Double check that transmissions to that node are allowed
  if (is_node_or_parent_paused(endpoint_node)) {
    // Try again when it wakes up
    sl_log_debug(LOG_TAG,
                 "NodeID resolution paused, postponing Basic Get probe");
    attribute_resolver_set_resolution_resumption_listener(
      endpoint_node,
      &on_endpoint_interview_completed);
    return;
  }

  // Finally, just double check that we did not already do that by any chance
  // On restart we get a few "ATTRIBUTE_CREATED" callbbaks that are not true original creations.
  if (get_basic_probe_status(endpoint_node) != NOT_REQUESTED) {
    sl_log_debug(LOG_TAG,
                 "Prevented an attempt to send several Basic Get Probes "
                 "for Attribute node %d.",
                 endpoint_node);
    return;
  }

  send_basic_probe(endpoint_node);
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Store callback functions
///////////////////////////////////////////////////////////////////////////////
static void on_nif_created(attribute_store_node_t node,
                           attribute_store_change_t change)
{
  if (change != ATTRIBUTE_CREATED) {
    return;
  }
  // We just created a new NIF somewhere, we are interviewing.
  // Set a listener so we can Basic Probe at the end of the interview
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
  attribute_resolver_set_resolution_listener(endpoint_node,
                                             &on_endpoint_interview_completed);
}

///////////////////////////////////////////////////////////////////////////////
// Incoming commands handler
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_basic_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Frame too short, it should have not come here.
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  assert(frame_data[COMMAND_CLASS_INDEX] == COMMAND_CLASS_BASIC_V2);

  switch (frame_data[COMMAND_INDEX]) {
    case BASIC_SET_V2:
      // Note: This command is not supported, but we want to indicate
      // to the application that we received it.
      return zwave_command_class_basic_handle_set(connection_info,
                                                  frame_data,
                                                  frame_length);
    case BASIC_REPORT_V2:
      return zwave_command_class_basic_handle_report(connection_info,
                                                     frame_data,
                                                     frame_length);
    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_basic_init()
{
  // Attribute resolver rules
  attribute_resolver_register_rule(ATTRIBUTE(VALUE),
                                   zwave_command_class_basic_set,
                                   zwave_command_class_basic_get);

  // Attribute store callbacks
  attribute_store_register_callback_by_type_and_state(on_nif_created,
                                                      ATTRIBUTE_ZWAVE_NIF,
                                                      REPORTED_ATTRIBUTE);

  // Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler         = &zwave_command_class_basic_control_handler;
  // Not supported, so this does not really matter
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_BASIC_V2;
  handler.version                    = BASIC_VERSION_V2;
  handler.command_class_name         = "Basic";
  handler.comments                   = "";

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}
