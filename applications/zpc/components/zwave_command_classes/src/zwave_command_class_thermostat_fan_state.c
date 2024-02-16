
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

#include "zwave_command_class_thermostat_fan_state.h"
#include "zwave_command_class_thermostat_fan_types.h"
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

#define LOG_TAG "zwave_command_class_thermostat_fan_state"

/////////////////////////////////////////////////////////////////////////////
// Version & Attribute Creation
/////////////////////////////////////////////////////////////////////////////
static void zwave_command_class_thermostat_fan_state_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(
        COMMAND_CLASS_THERMOSTAT_FAN_STATE,
        updated_node)
      == true) {
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
    = {ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_STATE_FAN_OPERATING_STATE};

  attribute_store_add_if_missing(endpoint_node,
                                 attributes,
                                 COUNT_OF(attributes));

}

/////////////////////////////////////////////////////////////////////////////
// Thermostat Fan State Get/Report
/////////////////////////////////////////////////////////////////////////////

static sl_status_t zwave_command_class_thermostat_fan_state_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  (void)node;  // unused.
  ZW_THERMOSTAT_FAN_STATE_GET_FRAME *get_frame
    = (ZW_THERMOSTAT_FAN_STATE_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_FAN_STATE;
  get_frame->cmd      = THERMOSTAT_FAN_STATE_GET;
  *frame_length       = sizeof(ZW_THERMOSTAT_FAN_STATE_GET_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_thermostat_fan_state_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 3) {
    return SL_STATUS_FAIL;
  }

  thermostat_fan_state_t fan_state = frame_data[2] & THERMOSTAT_FAN_STATE_REPORT_LEVEL_FAN_OPERATING_STATE_MASK;

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);
    
  attribute_store_set_child_reported(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_STATE_FAN_OPERATING_STATE,
    &fan_state,
    sizeof(fan_state));

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_thermostat_fan_state_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case THERMOSTAT_FAN_STATE_REPORT:
      return zwave_command_class_thermostat_fan_state_handle_report(
        connection_info,
        frame_data,
        frame_length);
    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}


sl_status_t zwave_command_class_thermostat_fan_state_init()
{
  attribute_store_register_callback_by_type(
    &zwave_command_class_thermostat_fan_state_on_version_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_STATE_VERSION);

  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_STATE_FAN_OPERATING_STATE,
    NULL,
    &zwave_command_class_thermostat_fan_state_get);


  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler
    = zwave_command_class_thermostat_fan_state_control_handler;
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_THERMOSTAT_FAN_STATE;
  handler.version                    = 2;
  handler.command_class_name         = "Thermostat Fan State";
  handler.comments                   = "Experimental";

  return zwave_command_handler_register_handler(handler);
}