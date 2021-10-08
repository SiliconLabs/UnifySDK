/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "zwave_command_class_multilevel_sensor.h"
#include "zwave_command_classes_utils.h"

// Includes from other components
#include "sl_log.h"
#include "sl_status.h"
#include "zwave_controller_command_class_indices.h"
#include "zwave_controller_utils.h"
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
#define LOG_TAG         "zwave_command_class_multilevel_sensor"
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_##type

///////////////////////////////////////////////////////////////////////////////
// Attribute Callback functions
///////////////////////////////////////////////////////////////////////////////
static void zwave_command_class_multilevel_sensor_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  // Check that we have the right type of attribute.
  assert(ATTRIBUTE(VERSION) == attribute_store_get_node_type(updated_node));

  uint8_t multilevel_sensor_version = 0;
  attribute_store_get_reported(updated_node,
                               &multilevel_sensor_version,
                               sizeof(multilevel_sensor_version));

  if (multilevel_sensor_version == 0) {
    // Wait for the version to be known.
    return;
  }

  // Now we know we have a supporting node. Verify that the capability
  // bitmask is present.
  attribute_store_node_t endpoint_node
    = attribute_store_get_node_parent(updated_node);

  // Let the rest of the command class perform the job.
  attribute_store_type_t supported_sensor_types[]
    = {ATTRIBUTE(SUPPORTED_SENSOR_TYPES)};
  attribute_store_add_if_missing(endpoint_node,
                                 supported_sensor_types,
                                 COUNT_OF(supported_sensor_types));
}

///////////////////////////////////////////////////////////////////////////////
// Attribute creation functions
///////////////////////////////////////////////////////////////////////////////
static void zwave_command_class_multilevel_sensor_create_type(
  attribute_store_node_t endpoint_node,
  uint8_t type,
  uint8_t multilevel_sensor_version)
{
  // Do we already have the node ?
  attribute_store_node_t type_node
    = attribute_store_get_node_child_by_value(endpoint_node,
                                              ATTRIBUTE(SENSOR_TYPE),
                                              REPORTED_ATTRIBUTE,
                                              &type,
                                              sizeof(type),
                                              0);

  if (type_node != ATTRIBUTE_STORE_INVALID_NODE) {
    // Already exists, nothing to do.
    return;
  }

  type_node = attribute_store_add_node(ATTRIBUTE(SENSOR_TYPE), endpoint_node);

  if (SL_STATUS_OK
      != attribute_store_set_node_attribute_value(type_node,
                                                  REPORTED_ATTRIBUTE,
                                                  &type,
                                                  sizeof(type))) {
    return;
  }

  // Add other nodes under the type. Only supported scales for version >4
  if (multilevel_sensor_version > 4) {
    const attribute_store_type_t additional_nodes[]
      = {ATTRIBUTE(SUPPORTED_SCALES)};
    attribute_store_add_if_missing(type_node,
                                   additional_nodes,
                                   COUNT_OF(additional_nodes));
  } else {
    const attribute_store_type_t additional_nodes[]
      = {ATTRIBUTE(SUPPORTED_SCALES),
         ATTRIBUTE(SCALE),
         ATTRIBUTE(SENSOR_VALUE)};
    attribute_store_add_if_missing(type_node,
                                   additional_nodes,
                                   COUNT_OF(additional_nodes));
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute resolution functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_multilevel_sensor_supported_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_len)
{
  // First we need to find out which version of the supporting node we have
  // Check that we have the right type of attribute.
  assert(ATTRIBUTE(SUPPORTED_SENSOR_TYPES)
         == attribute_store_get_node_type(node));
  // Default frame length in case of error
  *frame_len = 0;

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t version_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(VERSION),
                                             0);

  uint8_t supporting_node_version = 0;
  attribute_store_read_value(version_node,
                             REPORTED_ATTRIBUTE,
                             &supporting_node_version,
                             sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Something weird happened. Bail out.
    sl_log_warning(LOG_TAG, "Cannot read supporting node version.");
    return SL_STATUS_FAIL;
  }

  // If version 1-4, we just do a Multilvel Sensor Get. (they support only 1)
  if (supporting_node_version <= 4) {
    ZW_SENSOR_MULTILEVEL_GET_V11_FRAME *get_frame
      = (ZW_SENSOR_MULTILEVEL_GET_V11_FRAME *)frame;

    get_frame->cmdClass    = COMMAND_CLASS_SENSOR_MULTILEVEL_V11;
    get_frame->cmd         = SENSOR_MULTILEVEL_GET_V11;
    get_frame->sensorType  = 0;  // This will be ignored by the supporting node
    get_frame->properties1 = 0;  // This will be ignored by the supporting node

    *frame_len = sizeof(ZW_SENSOR_MULTILEVEL_GET_V11_FRAME);
    return SL_STATUS_OK;

  } else {
    // From version 5, we can ask nodes what they support first.
    ZW_SENSOR_MULTILEVEL_SUPPORTED_GET_SENSOR_V11_FRAME *supported_get_frame
      = (ZW_SENSOR_MULTILEVEL_SUPPORTED_GET_SENSOR_V11_FRAME *)frame;

    supported_get_frame->cmdClass = COMMAND_CLASS_SENSOR_MULTILEVEL_V11;
    supported_get_frame->cmd      = SENSOR_MULTILEVEL_SUPPORTED_GET_SENSOR_V11;

    *frame_len = sizeof(ZW_SENSOR_MULTILEVEL_SUPPORTED_GET_SENSOR_V11_FRAME);
    return SL_STATUS_OK;
  }
}

sl_status_t zwave_command_class_multilevel_sensor_supported_scale_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_len)
{
  // Check that we have the right type of attribute.
  assert(ATTRIBUTE(SUPPORTED_SCALES) == attribute_store_get_node_type(node));

  // Default frame length in case of error
  *frame_len = 0;

  attribute_store_node_t type_node = attribute_store_get_node_parent(node);
  attribute_store_node_t endpoint_node
    = attribute_store_get_node_parent(type_node);
  attribute_store_node_t version_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(VERSION),
                                             0);

  uint8_t supporting_node_version = 0;
  attribute_store_get_reported(version_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Something weird happened. Bail out.
    sl_log_warning(LOG_TAG, "Cannot read supporting node version.");
    return SL_STATUS_FAIL;
  }

  // If version 1-4, we just do a Multilvel Sensor Get. (they support only 1 scale)
  if (supporting_node_version <= 4) {
    ZW_SENSOR_MULTILEVEL_GET_V11_FRAME *get_frame
      = (ZW_SENSOR_MULTILEVEL_GET_V11_FRAME *)frame;

    get_frame->cmdClass    = COMMAND_CLASS_SENSOR_MULTILEVEL_V11;
    get_frame->cmd         = SENSOR_MULTILEVEL_GET_V11;
    get_frame->sensorType  = 0;  // This will be ignored by the supporting node
    get_frame->properties1 = 0;  // This will be ignored by the supporting node

    *frame_len = sizeof(ZW_SENSOR_MULTILEVEL_GET_V11_FRAME);
    return SL_STATUS_OK;

  } else {
    // From version 5, we can ask nodes what they support first.
    ZW_SENSOR_MULTILEVEL_SUPPORTED_GET_SCALE_V11_FRAME
    *supported_scales_get_frame
      = (ZW_SENSOR_MULTILEVEL_SUPPORTED_GET_SCALE_V11_FRAME *)frame;

    supported_scales_get_frame->cmdClass = COMMAND_CLASS_SENSOR_MULTILEVEL_V11;
    supported_scales_get_frame->cmd = SENSOR_MULTILEVEL_SUPPORTED_GET_SCALE_V11;
    supported_scales_get_frame->sensorType = 0;
    attribute_store_get_reported(
      type_node,
      &supported_scales_get_frame->sensorType,
      sizeof(supported_scales_get_frame->sensorType));

    *frame_len = sizeof(ZW_SENSOR_MULTILEVEL_SUPPORTED_GET_SCALE_V11_FRAME);
    return SL_STATUS_OK;
  }
}

sl_status_t zwave_command_class_multilevel_sensor_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_len)
{
  // Check that we have the right type of attribute.
  assert(ATTRIBUTE(SENSOR_VALUE) == attribute_store_get_node_type(node));

  // Default frame length in case of error
  *frame_len = 0;

  attribute_store_node_t type_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(SENSOR_TYPE));
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t version_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(VERSION),
                                             0);

  uint8_t supporting_node_version = 0;
  attribute_store_get_reported(version_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Something weird happened. Bail out.
    sl_log_warning(LOG_TAG, "Cannot read supporting node version.");
    return SL_STATUS_FAIL;
  }

  ZW_SENSOR_MULTILEVEL_GET_V11_FRAME *get_frame
    = (ZW_SENSOR_MULTILEVEL_GET_V11_FRAME *)frame;

  get_frame->cmdClass = COMMAND_CLASS_SENSOR_MULTILEVEL_V11;
  get_frame->cmd      = SENSOR_MULTILEVEL_GET_V11;

  if (supporting_node_version <= 4) {
    get_frame->sensorType  = 0;  // This will be ignored by the supporting node
    get_frame->properties1 = 0;  // This will be ignored by the supporting node
  } else {
    if (SL_STATUS_OK
        != attribute_store_get_reported(type_node,
                                        &get_frame->sensorType,
                                        sizeof(get_frame->sensorType))) {
      sl_log_warning(LOG_TAG,
                     "Cannot read Sensor Type for Attribute %d.",
                     type_node);
      return SL_STATUS_FAIL;
    }
    attribute_store_node_t supported_scales_node
      = attribute_store_get_node_child_by_type(type_node,
                                               ATTRIBUTE(SUPPORTED_SCALES),
                                               0);

    uint32_t supported_scales = 0;
    if (SL_STATUS_OK
        != attribute_store_get_reported(supported_scales_node,
                                        &supported_scales,
                                        sizeof(supported_scales))) {
      sl_log_warning(LOG_TAG,
                     "Unknown supported scales for Attribute %d. "
                     "Aborting resolution.",
                     supported_scales_node);

      return SL_STATUS_FAIL;
    }

    uint8_t first_supported_scale = 0;
    for (uint8_t i = 0; i < 4; i++) {
      if (supported_scales & (1 << i)) {
        first_supported_scale = i;
        break;
      }
    }
    get_frame->properties1 = first_supported_scale << 3;
  }

  *frame_len = sizeof(ZW_SENSOR_MULTILEVEL_GET_V11_FRAME);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Command Handler functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_multilevel_sensor_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // We expect to have at least 1 byte of value
  if (frame_length <= REPORT_SENSOR_VALUE_INDEX) {
    return SL_STATUS_FAIL;
  }

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  // We need the version of the supporting node here
  uint8_t multilevel_sensor_version = 0;
  attribute_store_node_t version_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(VERSION),
                                             0);
  attribute_store_get_reported(version_node,
                               &multilevel_sensor_version,
                               sizeof(multilevel_sensor_version));

  if (multilevel_sensor_version == 0) {
    return SL_STATUS_FAIL;
  }

  attribute_store_node_t type_node = attribute_store_get_node_child_by_value(
    endpoint_node,
    ATTRIBUTE(SENSOR_TYPE),
    REPORTED_ATTRIBUTE,
    &frame_data[REPORT_SENSOR_TYPE_INDEX],
    sizeof(uint8_t),
    0);

  uint8_t size = frame_data[REPORT_PROPERTIES_INDEX] & REPORT_SIZE_BITMASK;
  int32_t scale
    = (frame_data[REPORT_PROPERTIES_INDEX] & REPORT_SCALE_BITMASK) >> 3;
  int32_t precision
    = (frame_data[REPORT_PROPERTIES_INDEX] & REPORT_PRECISION_BITMASK) >> 5;

  if (type_node == ATTRIBUTE_STORE_INVALID_NODE) {
    if (multilevel_sensor_version > 4) {
      sl_log_debug(LOG_TAG,
                   "Quirk Detected: sensor reports on type (%d),"
                   " that was not reported as supported",
                   frame_data[REPORT_SENSOR_TYPE_INDEX]);
    }
    // This is how we discover that the type is supported for nodes with version
    // less than 4.
    // For nodes with later versions than 4, we have seen nodes, that doesn't
    // report the sensor type correctly, thus we create it here as a fallback
    // solution
    if (multilevel_sensor_version <= 4) {
      // Create a dummy supported bitmask based on the type the node reports.
      uint8_t bitmask[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
      uint8_t bitmask_size                                  = (scale / 8) + 1;
      bitmask[(scale / 8)]                                  = 1 << (scale % 8);
      attribute_store_set_child_reported(endpoint_node,
                                         ATTRIBUTE(SUPPORTED_SENSOR_TYPES),
                                         bitmask,
                                         bitmask_size);
    }
    zwave_command_class_multilevel_sensor_create_type(
      endpoint_node,
      frame_data[REPORT_SENSOR_TYPE_INDEX],
      multilevel_sensor_version);
  }

  // Save the scale under the type
  attribute_store_set_child_reported(type_node,
                                     ATTRIBUTE(SCALE),
                                     &scale,
                                     sizeof(scale));

  if (multilevel_sensor_version) {
    // Also save the scale as the supported scales udner the type
    uint8_t supported_scales = 1 << scale;
    attribute_store_set_child_reported(type_node,
                                       ATTRIBUTE(SUPPORTED_SCALES),
                                       &supported_scales,
                                       sizeof(supported_scales));
  }

  // Now extract the value from the frame:
  int32_t sensor_value
    = command_class_get_int32_value(size,
                                    precision,
                                    &frame_data[REPORT_SENSOR_VALUE_INDEX]);

  sl_log_debug(LOG_TAG,
               "Current sensor reading for NodeID %d:%d - Type: 0x%02X, "
               "reading (3 decimal digits): %d",
               connection_info->remote.node_id,
               connection_info->remote.endpoint_id,
               frame_data[REPORT_SENSOR_TYPE_INDEX],
               sensor_value);

  // Save the value under the type
  attribute_store_set_child_reported(type_node,
                                     ATTRIBUTE(SENSOR_VALUE),
                                     &sensor_value,
                                     sizeof(sensor_value));

  return SL_STATUS_OK;
}

static sl_status_t
  zwave_command_class_multilevel_sensor_handle_supported_report(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  // We expect to have at least 1 bitmask
  if (frame_length <= SUPPORTED_REPORT_BITMASK_INDEX) {
    return SL_STATUS_FAIL;
  }

  // First up, save our supported types in the Attribute Store.
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  attribute_store_node_t supported_bitmask_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(SUPPORTED_SENSOR_TYPES),
                                             0);

  attribute_store_set_reported(supported_bitmask_node,
                               &frame_data[SUPPORTED_REPORT_BITMASK_INDEX],
                               frame_length - SUPPORTED_REPORT_BITMASK_INDEX);

  uint8_t version
    = zwave_node_get_command_class_version(COMMAND_CLASS_SENSOR_MULTILEVEL_V11,
                                           connection_info->remote.node_id,
                                           connection_info->remote.endpoint_id);

  // Now create a type attribute for each supported type
  for (uint8_t byte = 0; byte < frame_length - SUPPORTED_REPORT_BITMASK_INDEX;
       byte++) {
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (frame_data[SUPPORTED_REPORT_BITMASK_INDEX + byte] & (1 << bit)) {
        // This type is supported.
        uint8_t type = byte * 8 + bit + 1;
        zwave_command_class_multilevel_sensor_create_type(endpoint_node,
                                                          type,
                                                          version);
      }
    }
  }

  return SL_STATUS_OK;
}

static sl_status_t
  zwave_command_class_multilevel_sensor_handle_supported_scale_report(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  // Frame too short, ignore
  if (frame_length <= SUPPORTED_SCALE_REPORT_BITMASK_INDEX) {
    return SL_STATUS_FAIL;
  }

  // Just retrieve the data and save it.
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  attribute_store_node_t type_node = attribute_store_get_node_child_by_value(
    endpoint_node,
    ATTRIBUTE(SENSOR_TYPE),
    REPORTED_ATTRIBUTE,
    &frame_data[SUPPORTED_SCALE_REPORT_SENSOR_TYPE_INDEX],
    sizeof(uint8_t),
    0);

  if (type_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_FAIL;
  }

  attribute_store_node_t supported_scale_node
    = attribute_store_get_node_child_by_type(type_node,
                                             ATTRIBUTE(SUPPORTED_SCALES),
                                             0);

  if (supported_scale_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_FAIL;
  }
  int32_t scale = frame_data[SUPPORTED_SCALE_REPORT_BITMASK_INDEX];
  attribute_store_set_reported(supported_scale_node, &scale, sizeof(scale));

  uint8_t version
    = zwave_node_get_command_class_version(COMMAND_CLASS_SENSOR_MULTILEVEL_V11,
                                           connection_info->remote.node_id,
                                           connection_info->remote.endpoint_id);
  if (version > 4) {
    // Now that we probed the supported scales, as for the Sensor Value
    const attribute_store_type_t additional_nodes[]
      = {ATTRIBUTE(SCALE), ATTRIBUTE(SENSOR_VALUE)};
    attribute_store_add_if_missing(type_node,
                                   additional_nodes,
                                   COUNT_OF(additional_nodes));
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_multilevel_sensor_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Frame too short, it should have not come here.
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  assert(frame_data[COMMAND_CLASS_INDEX]
         == COMMAND_CLASS_SENSOR_MULTILEVEL_V11);

  switch (frame_data[COMMAND_INDEX]) {
    case SENSOR_MULTILEVEL_REPORT_V11:
      return zwave_command_class_multilevel_sensor_handle_report(
        connection_info,
        frame_data,
        frame_length);

    case SENSOR_MULTILEVEL_SUPPORTED_SENSOR_REPORT_V11:
      return zwave_command_class_multilevel_sensor_handle_supported_report(
        connection_info,
        frame_data,
        frame_length);

    case SENSOR_MULTILEVEL_SUPPORTED_SCALE_REPORT_V11:
      return zwave_command_class_multilevel_sensor_handle_supported_scale_report(
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

sl_status_t zwave_command_class_multilevel_sensor_init()
{
  // Resolver functions.
  attribute_resolver_register_rule(
    ATTRIBUTE(SUPPORTED_SENSOR_TYPES),
    NULL,
    zwave_command_class_multilevel_sensor_supported_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(SUPPORTED_SCALES),
    NULL,
    zwave_command_class_multilevel_sensor_supported_scale_get);

  attribute_resolver_register_rule(ATTRIBUTE(SENSOR_VALUE),
                                   NULL,
                                   zwave_command_class_multilevel_sensor_get);

  // Listening for supporting nodes
  attribute_store_register_callback_by_type(
    zwave_command_class_multilevel_sensor_on_version_attribute_update,
    ATTRIBUTE(VERSION));

  // Register Multilevel CC handler to the Z-Wave CC framework
  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler
    = zwave_command_class_multilevel_sensor_control_handler;
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.command_class      = COMMAND_CLASS_SENSOR_MULTILEVEL_V11;
  handler.command_class_name = "Multilevel Sensor";
  handler.comments           = "Partial control: <br>"
                     "1. Not all scales are supported <br>"
                     "2. No regular probing is done ";
  handler.version                    = SENSOR_MULTILEVEL_VERSION_V11;
  handler.manual_security_validation = false;

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}