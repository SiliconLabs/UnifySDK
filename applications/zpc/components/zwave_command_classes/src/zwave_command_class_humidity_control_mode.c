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

#include "zwave_command_class_humidity_control_mode.h"
#include "zwave_command_class_humidity_control_types.h"
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

#define LOG_TAG "zwave_command_class_humidity_control_mode"

/////////////////////////////////////////////////////////////////////////////
// Utils
/////////////////////////////////////////////////////////////////////////////
/**
 * @brief Get current humidity control mode version of the CC
 *
 * @param node  Endpoint node
 *
 * @returns zwave_cc_version_t 0 if anything goes wrong, version number otherwise
 */
static zwave_cc_version_t
  get_humidity_control_mode_version(attribute_store_node_t endpoint_node)
{
  attribute_store_node_t version_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_VERSION);
  // We need to check the version of the supporting node:
  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_get_reported(version_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  return supporting_node_version;
}

// Return true if supported_humidity_control_mode_type is compatible with current_version
// False otherwise
static bool is_humidity_control_mode_compatible_with_version(
  humidity_control_mode_t supported_humidity_control_mode_type,
  zwave_cc_version_t current_version)
{
  bool compatibility = false;

  switch (current_version) {
    case 1:
      compatibility = (supported_humidity_control_mode_type <= 0x02);
      break;
    case 2:
      compatibility = (supported_humidity_control_mode_type <= 0x03);
      break;
    default:
      compatibility = false;
  }

  if (!compatibility) {
    sl_log_warning(LOG_TAG,
                   "Humidity Control Mode %#04x is not compatible with "
                   "Humidity Control Mode Version %d",
                   supported_humidity_control_mode_type,
                   current_version);
  }

  return compatibility;
}

static bool is_humidity_control_mode_supported(
  humidity_control_mode_t humidity_control_mode_type,
  attribute_store_node_t endpoint_id_node)
{
  // First check version
  zwave_cc_version_t current_version
    = get_humidity_control_mode_version(endpoint_id_node);

  // Check version compatibility
  if (!is_humidity_control_mode_compatible_with_version(
        humidity_control_mode_type,
        current_version)) {
    return false;
  }

  attribute_store_node_t supported_type_node
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_SUPPORTED_MODES);

  if (supported_type_node == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_warning(
      LOG_TAG,
      "Can't get ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_SUPPORTED_MODES "
      "from attribute store.");
    return false;
  }

  humidity_control_supported_modes_t supported_types;
  sl_status_t status = attribute_store_get_reported(supported_type_node,
                                                    &supported_types,
                                                    sizeof(supported_types));

  if (status != ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_warning(
      LOG_TAG,
      "Can't get reported value of "
      "ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_SUPPORTED_MODES.");
    return false;
  }

  switch (humidity_control_mode_type) {
    case HUMIDITY_CONTROL_MODE_SET_MODE_HUMIDIFY:
      return supported_types & 0x01;
    case HUMIDITY_CONTROL_MODE_SET_MODE_DEHUMIDIFY:
      return supported_types & 0x02;
    case HUMIDITY_CONTROL_MODE_SET_MODE_AUTO_V2:
      return supported_types & 0x04;
    default:
      return false;
  }
}

/////////////////////////////////////////////////////////////////////////////
// Version & Attribute Creation
/////////////////////////////////////////////////////////////////////////////
static void
  zwave_command_class_humidity_control_mode_on_version_attribute_update(
    attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(
        COMMAND_CLASS_HUMIDITY_CONTROL_MODE,
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
    = {ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_SUPPORTED_MODES,
       ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_CURRENT_MODE};

  attribute_store_add_if_missing(endpoint_node,
                                 attributes,
                                 COUNT_OF(attributes));
}

/////////////////////////////////////////////////////////////////////////////
// Humidity Conntrol Mode Get/Set/Report
/////////////////////////////////////////////////////////////////////////////

static sl_status_t zwave_command_class_humidity_control_mode_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  (void)node;  // unused.
  ZW_HUMIDITY_CONTROL_MODE_GET_FRAME *get_frame
    = (ZW_HUMIDITY_CONTROL_MODE_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_HUMIDITY_CONTROL_MODE;
  get_frame->cmd      = HUMIDITY_CONTROL_MODE_GET;
  *frame_length       = sizeof(ZW_HUMIDITY_CONTROL_MODE_GET_FRAME);
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_humidity_control_mode_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  humidity_control_mode_t humidity_control_mode = 0;
  sl_status_t result
    = attribute_store_get_desired_else_reported(node,
                                                &humidity_control_mode,
                                                sizeof(humidity_control_mode));

  if (result != SL_STATUS_OK) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (!is_humidity_control_mode_supported(humidity_control_mode,
                                          endpoint_node)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  ZW_HUMIDITY_CONTROL_MODE_SET_FRAME *set_frame
    = (ZW_HUMIDITY_CONTROL_MODE_SET_FRAME *)frame;
  set_frame->cmdClass    = COMMAND_CLASS_HUMIDITY_CONTROL_MODE;
  set_frame->cmd         = HUMIDITY_CONTROL_MODE_SET;
  set_frame->properties1 = humidity_control_mode;
  *frame_length          = sizeof(ZW_HUMIDITY_CONTROL_MODE_SET_FRAME);

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_humidity_control_mode_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  humidity_control_mode_t humidity_control_mode
    = frame_data[2] & HUMIDITY_CONTROL_MODE_REPORT_PROPERTIES1_MODE_MASK;

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);


  if (!is_humidity_control_mode_supported(humidity_control_mode, endpoint_node)) {
    sl_log_warning(LOG_TAG, "Reported humidity control mode mismatch.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  attribute_store_set_child_reported(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_CURRENT_MODE,
    &humidity_control_mode,
    sizeof(humidity_control_mode));

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Humidity Control Mode Supported Get/Report
/////////////////////////////////////////////////////////////////////////////

static sl_status_t zwave_command_class_humidity_control_mode_supported_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  (void)node;  // unused.
  ZW_HUMIDITY_CONTROL_MODE_SUPPORTED_GET_FRAME *get_frame
    = (ZW_HUMIDITY_CONTROL_MODE_SUPPORTED_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_HUMIDITY_CONTROL_MODE;
  get_frame->cmd      = HUMIDITY_CONTROL_MODE_SUPPORTED_GET;
  *frame_length       = sizeof(ZW_HUMIDITY_CONTROL_MODE_SUPPORTED_GET_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_humidity_control_mode_supported_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 3) {
    return SL_STATUS_FAIL;
  }

  uint8_t supported_modes = frame_data[2];

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  attribute_store_set_child_reported(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_SUPPORTED_MODES,
    &supported_modes,
    sizeof(supported_modes));

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_humidity_control_mode_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case HUMIDITY_CONTROL_MODE_REPORT:
      return zwave_command_class_humidity_control_mode_handle_report(
        connection_info,
        frame_data,
        frame_length);
    case HUMIDITY_CONTROL_MODE_SUPPORTED_REPORT:
      return zwave_command_class_humidity_control_mode_supported_handle_report(
        connection_info,
        frame_data,
        frame_length);
    default:
      return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwave_command_class_humidity_control_mode_init()
{
  attribute_store_register_callback_by_type(
    &zwave_command_class_humidity_control_mode_on_version_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_VERSION);

  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_CURRENT_MODE,
    &zwave_command_class_humidity_control_mode_set,
    &zwave_command_class_humidity_control_mode_get);

  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_SUPPORTED_MODES,
    NULL,
    &zwave_command_class_humidity_control_mode_supported_get);

  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler
    = zwave_command_class_humidity_control_mode_control_handler;
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_HUMIDITY_CONTROL_MODE;
  handler.version                    = 2;
  handler.command_class_name         = "Humidity Control Mode";
  handler.comments                   = "Experimental";

  return zwave_command_handler_register_handler(handler);
}