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

// System
#include <stdlib.h>

#include "zwave_command_class_thermostat_operating_state.h"
#include "zwave_command_class_thermostat_operating_state_types.h"
#include "zwave_command_classes_utils.h"
#include "ZW_classcmd.h"

// Includes from other ZPC Components
#include "zwave_command_class_indices.h"
#include "zwave_command_handler.h"
#include "zwave_command_class_version_types.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"

// Unify
#include "attribute_resolver.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "sl_log.h"

#define LOG_TAG "zwave_command_class_thermostat_operating_state"

#define MAX_SUPPORTED_OPERATING_STATE 16

/////////////////////////////////////////////////////////////////////////////
// Version & Attribute Creation
/////////////////////////////////////////////////////////////////////////////
static void
  zwave_command_class_thermostat_operating_state_on_version_attribute_update(
    attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  zwave_cc_version_t version = 0;
  attribute_store_get_reported(updated_node, &version, sizeof(version));

  if (version == 0) {
    return;
  }

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  // The order of the attribute matter since it defines the order of the
  // Z-Wave get command order.
  const attribute_store_type_t attributes[]
    = {ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_CURRENT_STATE};

  attribute_store_add_if_missing(endpoint_node,
                                 attributes,
                                 COUNT_OF(attributes));

  if (version == 2) {
    const attribute_store_type_t attributes_v2[] = {
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_SUPPORTED_BITMASK};

    attribute_store_add_if_missing(endpoint_node,
                                   attributes_v2,
                                   COUNT_OF(attributes_v2));
  }
}

/////////////////////////////////////////////////////////////////////////////
// Thermostat Operating State Get/Report
/////////////////////////////////////////////////////////////////////////////

static sl_status_t zwave_command_class_thermostat_operating_state_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  (void)node;  // unused.
  ZW_THERMOSTAT_OPERATING_STATE_GET_FRAME *get_frame
    = (ZW_THERMOSTAT_OPERATING_STATE_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_OPERATING_STATE;
  get_frame->cmd      = THERMOSTAT_OPERATING_STATE_GET;
  *frame_length       = sizeof(ZW_THERMOSTAT_OPERATING_STATE_GET_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_thermostat_operating_state_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 3) {
    return SL_STATUS_FAIL;
  }

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  thermostat_operating_state_t operating_state = frame_data[2];

  attribute_store_set_child_reported(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_CURRENT_STATE,
    &operating_state,
    sizeof(operating_state));

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Thermostat Operating State Logging Supported Get/Report
/////////////////////////////////////////////////////////////////////////////
static sl_status_t
  zwave_command_class_thermostat_operating_state_logging_supported_get(
    attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  (void)node;  // unused.
  ZW_THERMOSTAT_OPERATING_STATE_LOGGING_SUPPORTED_GET_V2_FRAME *get_frame
    = (ZW_THERMOSTAT_OPERATING_STATE_LOGGING_SUPPORTED_GET_V2_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_OPERATING_STATE;
  get_frame->cmd      = THERMOSTAT_OPERATING_STATE_LOGGING_SUPPORTED_GET_V2;
  *frame_length
    = sizeof(ZW_THERMOSTAT_OPERATING_STATE_LOGGING_SUPPORTED_GET_V2_FRAME);
  return SL_STATUS_OK;
}

sl_status_t
  zwave_command_class_thermostat_operating_state_logging_supported_report(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  if (frame_length < 3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  uint32_t notification_types_bits   = 0x0000;
  uint32_t notification_type_mask    = 0x0000;
  uint8_t number_of_supported_states = 0;
  thermostat_operating_state_t supported_states[MAX_SUPPORTED_OPERATING_STATE];
  uint8_t number_of_bit_masks = frame_length - 2;

  // Since we are using uint32_t we can't have more that 4 bit mask
  if (number_of_bit_masks > 4) {
    sl_log_error(LOG_TAG,
                 "Supported Fan mode type Bit Mask length is not supported\n");
    return SL_STATUS_NOT_SUPPORTED;
  }

  for (int i = number_of_bit_masks; i > 0; i--) {
    notification_types_bits
      = (notification_types_bits << 8) | frame_data[1 + i];
  }

  for (size_t i = 0; i <= MAX_SUPPORTED_OPERATING_STATE; i++) {
    notification_type_mask = 1 << i;
    notification_type_mask &= notification_types_bits;
    if (notification_type_mask) {
      switch (notification_type_mask) {
        case 0b1:
          supported_states[number_of_supported_states]
            = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_HEATING;
          break;
        case 0b10:
          supported_states[number_of_supported_states]
            = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_COOLING;
          break;
        case 0b100:
          supported_states[number_of_supported_states]
            = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_FAN_ONLY;
          break;
        case 0b1000:
          supported_states[number_of_supported_states]
            = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_PENDING_HEAT;
          break;
        case 0b10000:
          supported_states[number_of_supported_states]
            = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_PENDING_COOL;
          break;
        case 0b100000:
          supported_states[number_of_supported_states]
            = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_VENT_ECONOMIZER;
          break;
        case 0b1000000:
          supported_states[number_of_supported_states]
            = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_AUX_HEATING_V2;
          break;
        case 0b10000000:
          supported_states[number_of_supported_states]
            = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_2ND_STAGE_HEATING_V2;
          break;
        case 0b100000000:
          supported_states[number_of_supported_states]
            = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_2ND_STAGE_COOLING_V2;
          break;
        case 0b1000000000:
          supported_states[number_of_supported_states]
            = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_2ND_STAGE_AUX_HEAT_V2;
          break;
        case 0b10000000000:
          supported_states[number_of_supported_states]
            = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_3RD_STAGE_AUX_HEAT_V2;
          break;
        default:
          continue;
      }
      number_of_supported_states++;
    }
  }

  attribute_store_node_t supported_logging_operating_state_bitmask_node
    = attribute_store_get_node_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_SUPPORTED_BITMASK,
      0);

  if (supported_logging_operating_state_bitmask_node
      == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_error(LOG_TAG,
                 "Can't find supported supported logging state bitmask node.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Clear previously reported values
  attribute_store_delete_all_children(
    supported_logging_operating_state_bitmask_node);

  // And Set the new ones
  for (uint8_t i = 0; i < number_of_supported_states; i++) {
    attribute_store_node_t current_supported_fan_mode_node
      = attribute_store_add_node(
        ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_SUPPORTED,
        supported_logging_operating_state_bitmask_node);

    attribute_store_set_node_attribute_value(current_supported_fan_mode_node,
                                             REPORTED_ATTRIBUTE,
                                             &supported_states[i],
                                             sizeof(supported_states[i]));
  }

  sl_status_t result = attribute_store_set_reported(
    supported_logging_operating_state_bitmask_node,
    &notification_types_bits,
    sizeof(notification_types_bits));

  if (result != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Can't set supported logging operating state bitmask.");
  }

  attribute_store_node_t log_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_BITMASK,
    endpoint_node);

  result = attribute_store_set_desired(log_node,
                                       &notification_types_bits,
                                       sizeof(notification_types_bits));

  if (result != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "Can't set log desired states.");
  }

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Thermostat Operating State Logging Get/Report
/////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_thermostat_operating_state_logging_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Extract bitmap from value
  uint32_t full_bitmask = 0;
  sl_status_t result
    = attribute_store_get_desired(node, &full_bitmask, sizeof(full_bitmask));

  if (result != SL_STATUS_OK) {
    sl_log_warning(
      LOG_TAG,
      "Unable to get bitmask for THERMOSTAT_OPERATING_STATE_LOGGING_GET");
    return SL_STATUS_IS_WAITING;
  }

  // Convert 32 bit mask into chunk of 8 bit masks
  const uint8_t MAX_BITMASKS = 2;
  uint8_t bitmasks[MAX_BITMASKS];
  for (int i = 0; i < MAX_BITMASKS; i++) {
    uint32_t mask = ((1 << 8) - 1) << 8 * i;
    bitmasks[i]   = (full_bitmask & mask) >> 8 * i;
  }

  ZW_THERMOSTAT_OPERATING_STATE_LOGGING_GET_2BYTE_V2_FRAME *get_frame
    = (ZW_THERMOSTAT_OPERATING_STATE_LOGGING_GET_2BYTE_V2_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_OPERATING_STATE;
  get_frame->cmd      = THERMOSTAT_OPERATING_STATE_LOGGING_GET_V2;
  get_frame->bitMask1 = bitmasks[0];
  get_frame->bitMask2 = bitmasks[1];
  *frame_length
    = sizeof(ZW_THERMOSTAT_OPERATING_STATE_LOGGING_GET_2BYTE_V2_FRAME);
  return SL_STATUS_OK;
}

attribute_store_node_t helper_add_log_node(attribute_store_node_t log_node,
                                           attribute_store_type_t node_type,
                                           uint8_t value)
{
  attribute_store_node_t new_node
    = attribute_store_add_node(node_type, log_node);

  attribute_store_set_reported(new_node, &value, sizeof(value));

  return new_node;
}

sl_status_t zwave_command_class_thermostat_operating_state_logging_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Just retrieve the data and save it.
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);
  attribute_store_node_t log_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_BITMASK);

  uint8_t report_count = frame_data[2];

  // Clear previously reported values
  attribute_store_delete_all_children(log_node);

  const uint8_t REPORT_FIELD_COUNT = 4;

  for (uint8_t i = 0; i < report_count; i++) {
    thermostat_operating_state_t state
      = frame_data[3 + (REPORT_FIELD_COUNT * i) + i]
        & THERMOSTAT_OPERATING_STATE_REPORT_PROPERTIES1_OPERATING_STATE_MASK_V2;

    attribute_store_node_t state_node = helper_add_log_node(
      log_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_STATE,
      state);

    const attribute_store_type_t attribute_store_types[] = {
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_USAGE_TODAY_HOURS,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_USAGE_TODAY_MIN,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_USAGE_YESTERDAY_HOURS,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_USAGE_YESTERDAY_MIN};

    for (int j = 0; j < REPORT_FIELD_COUNT; j++) {
      const uint8_t base_index = 4 + j;
      const uint8_t offset     = (REPORT_FIELD_COUNT * i) + i;
      thermostat_operating_state_usage_t current_value
        = frame_data[base_index + offset];

      helper_add_log_node(state_node, attribute_store_types[j], current_value);
    }
  }

  attribute_store_set_reported_as_desired(log_node);
  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Control handler (report)
/////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_thermostat_operating_state_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case THERMOSTAT_OPERATING_STATE_REPORT:
      return zwave_command_class_thermostat_operating_state_handle_report(
        connection_info,
        frame_data,
        frame_length);
    case THERMOSTAT_OPERATING_LOGGING_SUPPORTED_REPORT_V2:
      return zwave_command_class_thermostat_operating_state_logging_supported_report(
        connection_info,
        frame_data,
        frame_length);
    case THERMOSTAT_OPERATING_STATE_LOGGING_REPORT_V2:
      return zwave_command_class_thermostat_operating_state_logging_report(
        connection_info,
        frame_data,
        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
// Init
/////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_thermostat_operating_state_init()
{
  attribute_store_register_callback_by_type(
    &zwave_command_class_thermostat_operating_state_on_version_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_VERSION);

  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_CURRENT_STATE,
    NULL,
    &zwave_command_class_thermostat_operating_state_get);

  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_SUPPORTED_BITMASK,
    NULL,
    &zwave_command_class_thermostat_operating_state_logging_supported_get);

  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_BITMASK,
    NULL,
    &zwave_command_class_thermostat_operating_state_logging_get);

  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler
    = zwave_command_class_thermostat_operating_state_control_handler;
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_THERMOSTAT_OPERATING_STATE;
  handler.version                    = THERMOSTAT_OPERATING_STATE_VERSION_V2;
  handler.command_class_name         = "Thermostat Operating State";
  handler.comments                   = "Experimental. Log related functions are not yet exposed to MQTT.";

  return zwave_command_handler_register_handler(handler);
}