// System
#include <stdlib.h>

#include "zwave_command_class_thermostat_fan_mode.h"
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

#define LOG_TAG "zwave_command_class_thermostat_fan_mode"

#define MAX_SUPPORTED_FAN_MODE_TYPES 24

/////////////////////////////////////////////////////////////////////////////
// Utils
/////////////////////////////////////////////////////////////////////////////
/**
 * @brief Get current thermostat fan version of the CC
 *
 * @param node  Endpoint node
 *
 * @returns zwave_cc_version_t 0 if anything goes wrong, version number otherwise
 */
static zwave_cc_version_t
  get_thermostat_fan_version(attribute_store_node_t endpoint_node)
{
  attribute_store_node_t version_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_VERSION);
  // We need to check the version of the supporting node:
  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_get_reported(version_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  return supporting_node_version;
}

// Return true if supported_fan_mode_type is compatible with current_version
// False otherwise
static bool is_fan_mode_compatible_with_version(
  thermostat_fan_mode_t supported_fan_mode_type,
  zwave_cc_version_t current_version)
{
  bool compatibility = false;

  switch (current_version) {
    case 1:
      compatibility = (supported_fan_mode_type <= 0x03);
      break;
    case 2:
      compatibility = (supported_fan_mode_type <= 0x05);
      break;
    case 3:
      compatibility = (supported_fan_mode_type <= 0x07);
      break;
    case 4:
      compatibility = (supported_fan_mode_type <= 0x0A);
      break;
    case 5:
      compatibility = (supported_fan_mode_type <= 0x0B);
      break;
    default:
      compatibility = false;
  }

  if (!compatibility) {
    sl_log_warning(
      LOG_TAG,
      "Fan mode %#04x is not compatible with Thermostat Fan Mode Version %d",
      supported_fan_mode_type,
      current_version);
  }

  return compatibility;
}

static bool
  is_fan_control_mode_supported(thermostat_fan_mode_t fan_control_mode_type,
                                attribute_store_node_t endpoint_id_node)
{
  // First check version
  zwave_cc_version_t current_version
    = get_thermostat_fan_version(endpoint_id_node);

  // Check version compatibility
  if (!is_fan_mode_compatible_with_version(fan_control_mode_type,
                                                   current_version)) {
    return false;
  }

  attribute_store_node_t supported_type_node
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_SUPPORTED_MODES);

  if (supported_type_node == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_warning(
      LOG_TAG,
      "Can't get ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_SUPPORTED_MODES "
      "from attribute store.");
    return false;
  }

  thermostat_fan_supported_modes_t supported_types;
  sl_status_t status = attribute_store_get_reported(supported_type_node,
                                                    &supported_types,
                                                    sizeof(supported_types));

  if (status != ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_warning(
      LOG_TAG,
      "Can't get reported value of "
      "ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_SUPPORTED_MODES.");
    return false;
  }

  switch (fan_control_mode_type) {
    case THERMOSTAT_FAN_MODE_REPORT_FAN_MODE_AUTO_LOW:
      return supported_types & 0x01;
    case THERMOSTAT_FAN_MODE_REPORT_FAN_MODE_LOW:
      return supported_types & 0x02;
    case THERMOSTAT_FAN_MODE_REPORT_FAN_MODE_AUTO_HIGH:
      return supported_types & 0x04;
    case THERMOSTAT_FAN_MODE_REPORT_FAN_MODE_HIGH:
      return supported_types & 0x08;
    case THERMOSTAT_FAN_MODE_REPORT_FAN_MODE_AUTO_MEDIUM_V2:
      return supported_types & 0x10;
    case THERMOSTAT_FAN_MODE_REPORT_FAN_MODE_MEDIUM_V2:
      return supported_types & 0x20;
    case THERMOSTAT_FAN_MODE_REPORT_FAN_MODE_CIRCULATION_V3:
      return supported_types & 0x40;
    case THERMOSTAT_FAN_MODE_REPORT_FAN_MODE_HUMIDITY_V3:
      return supported_types & 0x80;
    case THERMOSTAT_FAN_MODE_REPORT_FAN_MODE_LEFT_RIGHT_V4:
      return supported_types & 0x100;
    case THERMOSTAT_FAN_MODE_REPORT_FAN_MODE_UP_DOWN_V4:
      return supported_types & 0x200;
    case THERMOSTAT_FAN_MODE_REPORT_FAN_MODE_QUIET_V4:
      return supported_types & 0x400;
    case THERMOSTAT_FAN_MODE_REPORT_FAN_MODE_EXTERNAL_CIRCULATION_V5:
      return supported_types & 0x800;
    default:
      return false;
  }
}

/////////////////////////////////////////////////////////////////////////////
// Version & Attribute Creation
/////////////////////////////////////////////////////////////////////////////
static void zwave_command_class_thermostat_fan_mode_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(
        COMMAND_CLASS_THERMOSTAT_FAN_MODE,
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
    = {ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_SUPPORTED_MODES,
       ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_CURRENT_MODE};

  attribute_store_add_if_missing(endpoint_node,
                                 attributes,
                                 COUNT_OF(attributes));

  // Add Off flag needed by version 2+
  if (version >= 2) {
    const attribute_store_type_t v2_plus_attributes[]
      = {ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_OFF_FLAG};
    attribute_store_add_if_missing(endpoint_node,
                                   v2_plus_attributes,
                                   COUNT_OF(v2_plus_attributes));
  }
}

/////////////////////////////////////////////////////////////////////////////
// Thermostat Get/Set/Report
/////////////////////////////////////////////////////////////////////////////

static sl_status_t zwave_command_class_thermostat_fan_mode_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  (void)node;  // unused.
  ZW_THERMOSTAT_FAN_MODE_GET_FRAME *get_frame
    = (ZW_THERMOSTAT_FAN_MODE_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_FAN_MODE;
  get_frame->cmd      = THERMOSTAT_FAN_MODE_GET;
  *frame_length       = sizeof(ZW_THERMOSTAT_FAN_MODE_GET_FRAME);
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_thermostat_fan_mode_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  thermostat_fan_mode_t fan_mode = 0;
  sl_status_t result
    = attribute_store_get_desired_else_reported(node,
                                                &fan_mode,
                                                sizeof(fan_mode));

  if (result != SL_STATUS_OK) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  zwave_cc_version_t current_version
    = get_thermostat_fan_version(endpoint_node);

  // Check version compatibility
  if (!is_fan_control_mode_supported(fan_mode, endpoint_node)) {
    sl_log_warning(LOG_TAG, "Unsupported fan mode %#04x. Clearing desired value.", fan_mode);
    attribute_store_undefine_desired(node);
    return SL_STATUS_NOT_SUPPORTED;
  }

  ZW_THERMOSTAT_FAN_MODE_SET_FRAME *set_frame
    = (ZW_THERMOSTAT_FAN_MODE_SET_FRAME *)frame;
  set_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_FAN_MODE;
  set_frame->cmd      = THERMOSTAT_FAN_MODE_SET;
  *frame_length       = sizeof(ZW_THERMOSTAT_FAN_MODE_SET_FRAME);

  // No Off bit in version 1
  if (current_version == 1) {
    set_frame->level = fan_mode;
  } else {
    attribute_store_node_t off_node = attribute_store_get_node_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_OFF_FLAG,
      0);

    if (off_node == ATTRIBUTE_STORE_INVALID_NODE) {
      return SL_STATUS_FAIL;
    }

    thermostat_fan_mode_off_flag_t off_flag = 0;
    result = attribute_store_get_desired_else_reported(off_node,
                                                       &off_flag,
                                                       sizeof(off_flag));

    if (result != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }

    if (off_flag > 1) {
      off_flag = 1;
    }
    // Off Bit (1bit) Reserved (3 bits) Fan Mode (4 bits)
    set_frame->level = fan_mode | (off_flag << 7);
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_thermostat_fan_mode_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  thermostat_fan_mode_t fan_mode
    = frame_data[2] & THERMOSTAT_FAN_MODE_REPORT_PROPERTIES1_FAN_MODE_MASK_V3;

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  if (!is_fan_control_mode_supported(fan_mode, endpoint_node)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  zwave_cc_version_t current_version
    = get_thermostat_fan_version(endpoint_node);

  if (current_version >= 3) {
    // Extract fan mode from frame (first bit)
    thermostat_fan_mode_off_flag_t off_flag
      = (frame_data[2] & THERMOSTAT_FAN_MODE_REPORT_PROPERTIES1_OFF_BIT_MASK_V3)
        >> 7;
    attribute_store_set_child_reported(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_OFF_FLAG,
      &off_flag,
      sizeof(off_flag));
  }

  attribute_store_set_child_reported(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_CURRENT_MODE,
    &fan_mode,
    sizeof(fan_mode));

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Thermostat Supported Get/Report
/////////////////////////////////////////////////////////////////////////////

static sl_status_t zwave_command_class_thermostat_fan_mode_supported_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  (void)node;  // unused.
  ZW_THERMOSTAT_FAN_MODE_SUPPORTED_GET_FRAME *get_frame
    = (ZW_THERMOSTAT_FAN_MODE_SUPPORTED_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_FAN_MODE;
  get_frame->cmd      = THERMOSTAT_FAN_MODE_SUPPORTED_GET;
  *frame_length       = sizeof(ZW_THERMOSTAT_FAN_MODE_SUPPORTED_GET_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_thermostat_fan_mode_supported_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  thermostat_fan_supported_modes_t supported_modes_bitmask = 0x0000;
  uint8_t bitmask_length = frame_length - 2;

  // Since we are using uint32_t we can't have more that 4 bit mask
  if (bitmask_length > 4) {
    sl_log_error(LOG_TAG,
                 "Supported Fan mode type Bit Mask length is not supported\n");
    return SL_STATUS_NOT_SUPPORTED;
  }

  for (int i = bitmask_length; i > 0; i--) {
    supported_modes_bitmask
      = (supported_modes_bitmask << 8) | frame_data[1 + i];
  }

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  sl_status_t set_value_status = attribute_store_set_child_reported(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_SUPPORTED_MODES,
      &supported_modes_bitmask,
      sizeof(supported_modes_bitmask));

  if (set_value_status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Can't set ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_SUPPORTED_MODES");
    return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_thermostat_fan_mode_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case THERMOSTAT_FAN_MODE_REPORT:
      return zwave_command_class_thermostat_fan_mode_handle_report(
        connection_info,
        frame_data,
        frame_length);
    case THERMOSTAT_FAN_MODE_SUPPORTED_REPORT:
      return zwave_command_class_thermostat_fan_mode_supported_handle_report(
        connection_info,
        frame_data,
        frame_length);
    default:
      return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwave_command_class_thermostat_fan_mode_init()
{
  attribute_store_register_callback_by_type(
    &zwave_command_class_thermostat_fan_mode_on_version_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_VERSION);

  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_CURRENT_MODE,
    &zwave_command_class_thermostat_fan_mode_set,
    &zwave_command_class_thermostat_fan_mode_get);

  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_OFF_FLAG,
    &zwave_command_class_thermostat_fan_mode_set,
    &zwave_command_class_thermostat_fan_mode_get);

  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_SUPPORTED_MODES,
    NULL,
    &zwave_command_class_thermostat_fan_mode_supported_get);

  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler
    = zwave_command_class_thermostat_fan_mode_control_handler;
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_THERMOSTAT_FAN_MODE;
  handler.version                    = 5;
  handler.command_class_name         = "Thermostat Fan Mode";
  handler.comments                   = "Experimental";

  return zwave_command_handler_register_handler(handler);
}