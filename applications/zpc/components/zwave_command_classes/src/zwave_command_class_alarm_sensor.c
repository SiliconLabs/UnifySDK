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
#include "zwave_command_class_alarm_sensor.h"

#include "zwave_command_classes_utils.h"

// Generic includes
#include <stdlib.h>

// Interfaces
#include "zwave_command_class_alarm_sensor_types.h"

// Includes from other ZPC Components
#include "ZW_classcmd.h"
#include "attribute_store_defined_attribute_types.h"
#include "zpc_attribute_store_network_helper.h"
#include "zwave_command_class_indices.h"
#include "zwave_command_handler.h"

// Includes from other Unify Components
#include "attribute_resolver.h"
#include "attribute_store_helper.h"
#include "sl_log.h"

// Log tag
#define LOG_TAG "zwave_command_class_alarm_sensor"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_##type

#define REPORT_ID_INDEX                       2
#define REPORT_TYPE_INDEX                     3
#define REPORT_STATE_INDEX                    4
#define REPORT_SECONDS_INDEX                  5
#define SUPPORTED_REPORT_BITMASK_LENGTH_INDEX 2
#define SUPPORTED_REPORT_BITMASK_INDEX        3

///////////////////////////////////////////////////////////////////////////////
// Resolution functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_alarm_sensor_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  alarm_sensor_type_t type = 0;
  attribute_store_node_t type_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(TYPE));
  attribute_store_get_reported(type_node, &type, sizeof(type));

  ZW_SENSOR_ALARM_GET_FRAME *get_frame = (ZW_SENSOR_ALARM_GET_FRAME *)frame;
  get_frame->cmdClass                  = COMMAND_CLASS_SENSOR_ALARM;
  get_frame->cmd                       = SENSOR_ALARM_GET;
  get_frame->sensorType                = type;
  *frame_length                        = sizeof(ZW_SENSOR_ALARM_GET_FRAME);
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_alarm_sensor_supported_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  (void)node;
  zwave_minimum_frame_t *get_frame = (zwave_minimum_frame_t *)frame;
  get_frame->command_class         = COMMAND_CLASS_SENSOR_ALARM;
  get_frame->command               = SENSOR_ALARM_SUPPORTED_GET;
  *frame_length                    = sizeof(zwave_minimum_frame_t);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Frame parsing functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_alarm_sensor_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // We expect to have at least 5 byte of value
  if (frame_length <= REPORT_SECONDS_INDEX) {
    return SL_STATUS_OK;
  }

  // Find the Endpoint
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  // Type
  alarm_sensor_type_t type = 0;
  type                     = frame_data[REPORT_TYPE_INDEX];

  if (type == 0xFF) {
    type = 0;
  }

  attribute_store_node_t type_node
    = attribute_store_get_node_child_by_value(endpoint_node,
                                              ATTRIBUTE(TYPE),
                                              REPORTED_ATTRIBUTE,
                                              &type,
                                              sizeof(type),
                                              0);

  // State
  attribute_store_node_t state_node
    = attribute_store_get_first_child_by_type(type_node, ATTRIBUTE(STATE));

  alarm_sensor_state_t state_value = frame_data[REPORT_STATE_INDEX];

  attribute_store_set_reported(state_node, &state_value, sizeof(state_value));

  // NodeId
  attribute_store_node_t id_node
    = attribute_store_get_first_child_by_type(type_node, ATTRIBUTE(NODE_ID));

  // Is there a id value:
  alarm_sensor_id_t id_value = frame_data[REPORT_ID_INDEX];
  // Same as for current value, sanitize the range.

  attribute_store_set_reported(id_node, &id_value, sizeof(id_value));

  // Seconds
  attribute_store_node_t seconds_node
    = attribute_store_get_first_child_by_type(type_node, ATTRIBUTE(SECONDS));

  alarm_sensor_seconds_t seconds_value = 0;

  if (frame_length > (REPORT_SECONDS_INDEX + 1)) {
    seconds_value
      = (alarm_sensor_seconds_t)(frame_data[REPORT_SECONDS_INDEX] << 8);
    seconds_value
      |= (alarm_sensor_seconds_t)frame_data[REPORT_SECONDS_INDEX + 1];
  }

  attribute_store_set_reported(seconds_node,
                               &seconds_value,
                               sizeof(seconds_value));

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_alarm_sensor_handle_supported_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= REPORT_ID_INDEX) {
    return SL_STATUS_OK;
  }

  // Find the Endpoint
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  attribute_store_node_t bitmask_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(BITMASK));

  uint8_t number_of_bytes = frame_data[SUPPORTED_REPORT_BITMASK_LENGTH_INDEX];

  uint8_t bitmask[number_of_bytes];

  alarm_sensor_type_t type = 0;
  if (frame_length < (SUPPORTED_REPORT_BITMASK_INDEX + number_of_bytes)) {
    return SL_STATUS_OK;
  }

  for (uint8_t byte = 0; byte < number_of_bytes; byte++) {
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (frame_data[byte + SUPPORTED_REPORT_BITMASK_INDEX] & (1 << bit)) {
        type = (byte * 8) + bit;
        attribute_store_node_t type_node
          = attribute_store_emplace(endpoint_node,
                                    ATTRIBUTE(TYPE),
                                    &type,
                                    sizeof(type));
        const attribute_store_type_t type_attribute[]
          = {ATTRIBUTE(STATE), ATTRIBUTE(NODE_ID), ATTRIBUTE(SECONDS)};
        attribute_store_add_if_missing(type_node,
                                       type_attribute,
                                       COUNT_OF(type_attribute));

        bitmask[byte] = frame_data[byte + SUPPORTED_REPORT_BITMASK_INDEX];
      }
    }
  }
  attribute_store_set_node_attribute_value(bitmask_node,
                                           REPORTED_ATTRIBUTE,
                                           bitmask,
                                           number_of_bytes);
  return SL_STATUS_OK;
}
///////////////////////////////////////////////////////////////////////////////
// Attribute Store callback functions
///////////////////////////////////////////////////////////////////////////////
static void zwave_command_class_alarm_sensor_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(
        COMMAND_CLASS_SENSOR_ALARM,
        updated_node)
      == true) {
    return;
  }

  // Confirm that we have a version attribute update
  if (ATTRIBUTE(VERSION) != attribute_store_get_node_type(updated_node)) {
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

  // Now we know we have a Alarm Sensor supporting endpoint.
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  const attribute_store_type_t bitmask_attribute[] = {ATTRIBUTE(BITMASK)};
  attribute_store_add_if_missing(endpoint_node,
                                 bitmask_attribute,
                                 COUNT_OF(bitmask_attribute));
}

///////////////////////////////////////////////////////////////////////////////
// Incoming commands handler
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_alarm_sensor_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Frame too short, it should have not come here.
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame_data[COMMAND_CLASS_INDEX] != COMMAND_CLASS_SENSOR_ALARM) {
    return SL_STATUS_FAIL;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case SENSOR_ALARM_SUPPORTED_REPORT:
      return zwave_command_class_alarm_sensor_handle_supported_report(
        connection_info,
        frame_data,
        frame_length);
    case SENSOR_ALARM_REPORT:
      return zwave_command_class_alarm_sensor_handle_report(connection_info,
                                                            frame_data,
                                                            frame_length);
    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_alarm_sensor_init()
{
  // Attribute resolver rules
  attribute_resolver_register_rule(
    ATTRIBUTE(BITMASK),
    NULL,
    zwave_command_class_alarm_sensor_supported_get);

  attribute_resolver_register_rule(ATTRIBUTE(STATE),
                                   NULL,
                                   zwave_command_class_alarm_sensor_get);

  // Attribute store callbacks
  attribute_store_register_callback_by_type(
    zwave_command_class_alarm_sensor_on_version_attribute_update,
    ATTRIBUTE(VERSION));

  // The support side of things: Register our handler to the Z-Wave CC
  // framework:
  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler = &zwave_command_class_alarm_sensor_control_handler;
  // Not supported, so this does not really matter
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_SENSOR_ALARM;
  handler.version                    = SENSOR_ALARM_VERSION;
  handler.command_class_name         = "Alarm Sensor";
  handler.comments                   = "";

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}
