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
#include "zwave_command_class_thermostat_mode.h"
#include "zwave_command_class_thermostat_mode_attributes.h"
#include "zwave_command_classes_utils.h"

// Includes from other components
#include "sl_log.h"
#include "sl_status.h"
#include "zwave_controller_command_class_indices.h"
#include "ZW_classcmd.h"
#include "zwave_command_handler.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_store_helper.h"
#include "attribute_store.h"
#include "attribute_resolver.h"

// Generic includes
#include <assert.h>

// Log define
#define LOG_TAG "zwave_command_class_thermostat_mode"

///////////////////////////////////////////////////////////////////////////////
// Attribute Callback functions
///////////////////////////////////////////////////////////////////////////////
static void zwave_command_class_thermostat_mode_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(
        COMMAND_CLASS_THERMOSTAT_MODE_V3,
        updated_node)
      == true) {
    return;
  }

  // Check that we have the right type of attribute.
  assert(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE_VERSION
         == attribute_store_get_node_type(updated_node));

  // Now we know we have a supporting node.
  attribute_store_node_t endpoint_node
    = attribute_store_get_node_parent(updated_node);

  // Let the rest of the command class perform the job.
  attribute_store_type_t additional_attribute_types[]
    = {ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES,
       ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE};
  attribute_store_add_if_missing(endpoint_node,
                                 additional_attribute_types,
                                 COUNT_OF(additional_attribute_types));
}

///////////////////////////////////////////////////////////////////////////////
// Attribute resolution functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_thermostat_mode_supported_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_len)
{
  // Check that we have the right type of attribute.
  assert(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES
         == attribute_store_get_node_type(node));

  // Default frame length in case of error
  *frame_len = 0;

  // We will just always use 4 bytes, precision 2.
  ZW_THERMOSTAT_MODE_SUPPORTED_GET_FRAME *supported_get_frame
    = (ZW_THERMOSTAT_MODE_SUPPORTED_GET_FRAME *)frame;

  supported_get_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_MODE_V3;
  supported_get_frame->cmd      = THERMOSTAT_MODE_SUPPORTED_GET_V3;

  *frame_len = sizeof(ZW_THERMOSTAT_MODE_SUPPORTED_GET_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_thermostat_mode_get(attribute_store_node_t node,
                                                    uint8_t *frame,
                                                    uint16_t *frame_len)
{
  // Check that we have the right type of attribute.
  assert(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE
         == attribute_store_get_node_type(node));

  // Default frame length in case of error
  *frame_len = 0;

  // We will just always use 4 bytes, precision 2.
  ZW_THERMOSTAT_MODE_GET_FRAME *get_frame
    = (ZW_THERMOSTAT_MODE_GET_FRAME *)frame;

  get_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_MODE_V3;
  get_frame->cmd      = THERMOSTAT_MODE_GET_V3;

  *frame_len = sizeof(ZW_THERMOSTAT_MODE_GET_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_thermostat_mode_set(attribute_store_node_t node,
                                                    uint8_t *frame,
                                                    uint16_t *frame_len)
{
  // Check that we have the right type of attribute.
  assert(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE
         == attribute_store_get_node_type(node));

  // Default frame length in case of error
  *frame_len = 0;

  // We will just always use 4 bytes, precision 2.
  ZW_THERMOSTAT_MODE_SET_FRAME *set_frame
    = (ZW_THERMOSTAT_MODE_SET_FRAME *)frame;

  set_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_MODE_V3;
  set_frame->cmd      = THERMOSTAT_MODE_SET_V3;
  // set_frame->level ? This is 3 bits reserved / 5 bits Mode.
  int32_t mode = 0;
  attribute_store_read_value(node, DESIRED_ATTRIBUTE, &mode, sizeof(mode));
  set_frame->level = mode & 0x1F;
  *frame_len       = sizeof(ZW_THERMOSTAT_MODE_SET_FRAME);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Command Handler functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_thermostat_mode_handle_supported_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Frame too short, ignore
  if (frame_length <= THERMOSTAT_MODE_SUPPORTED_REPORT_BITMASK_INDEX) {
    return SL_STATUS_FAIL;
  }
  // Just retrieve the data and save it.
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  attribute_store_node_t supported_modes_node
    = attribute_store_get_node_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES,
      0);
  uint8_t bitmask_length
    = frame_length - THERMOSTAT_MODE_SUPPORTED_REPORT_BITMASK_INDEX;

  attribute_store_set_node_attribute_value(
    supported_modes_node,
    REPORTED_ATTRIBUTE,
    &frame_data[THERMOSTAT_MODE_SUPPORTED_REPORT_BITMASK_INDEX],
    bitmask_length);

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_thermostat_mode_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Frame too short, ignore
  if (frame_length <= THERMOSTAT_MODE_REPORT_MODE_INDEX) {
    return SL_STATUS_FAIL;
  }
  // Just retrieve the data and save it.
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);
  unid_t unid;
  zwave_endpoint_id_t endpoint_id;
  if (SL_STATUS_OK
      != attribute_store_network_helper_get_unid_endpoint_from_node(
        endpoint_node,
        unid,
        &endpoint_id)) {
    sl_log_warning(LOG_TAG,
                   "Failed to find unid and endpoint from node %d",
                   endpoint_node);

    return SL_STATUS_FAIL;
  }

  command_class_thermostat_mode_set_mode(
    unid,
    endpoint_id,
    REPORTED_ATTRIBUTE,
    frame_data[THERMOSTAT_MODE_REPORT_MODE_INDEX]);

  // We have a set function for this attribute, so we also align the desired here.
  command_class_thermostat_mode_set_mode(
    unid,
    endpoint_id,
    DESIRED_ATTRIBUTE,
    frame_data[THERMOSTAT_MODE_REPORT_MODE_INDEX]);

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_thermostat_mode_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Frame too short, it should have not come here.
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  assert(frame_data[COMMAND_CLASS_INDEX] == COMMAND_CLASS_THERMOSTAT_MODE_V3);

  switch (frame_data[COMMAND_INDEX]) {
    case THERMOSTAT_MODE_REPORT_V3:
      return zwave_command_class_thermostat_mode_handle_report(connection_info,
                                                               frame_data,
                                                               frame_length);

    case THERMOSTAT_MODE_SUPPORTED_REPORT_V3:
      return zwave_command_class_thermostat_mode_handle_supported_report(
        connection_info,
        frame_data,
        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Init functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_thermostat_mode_init()
{
  // Resolver functions.
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES,
    NULL,
    zwave_command_class_thermostat_mode_supported_get);

  attribute_resolver_register_rule(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE,
                                   zwave_command_class_thermostat_mode_set,
                                   zwave_command_class_thermostat_mode_get);

  // Listening for supporting nodes
  attribute_store_register_callback_by_type(
    zwave_command_class_thermostat_mode_on_version_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE_VERSION);

  // Register Thermostat Mode CC handler to the Z-Wave CC framework
  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler
    = &zwave_command_class_thermostat_mode_control_handler;
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.command_class      = COMMAND_CLASS_THERMOSTAT_MODE_V3;
  handler.version            = THERMOSTAT_MODE_VERSION_V3;
  handler.command_class_name = "Thermostat Mode";
  handler.comments           = "Partial Control: Not all modes can be set";
  handler.manual_security_validation = false;

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}