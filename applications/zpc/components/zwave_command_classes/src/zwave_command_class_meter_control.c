/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "zwave_command_class_meter_control.h"
#include "zwave_command_classes_utils.h"

// Interfaces
#include "zwave_command_class_meter_types.h"
#include "attribute_store_defined_attribute_types.h"

// Includes from other ZPC Components
#include "zwave_command_class_indices.h"
#include "zwave_controller_connection_info.h"
#include "zwave_controller_utils.h"
#include "zwave_command_handler.h"
#include "zpc_attribute_store_network_helper.h"
#include "zpc_attribute_resolver.h"

// Includes from Unify Components
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_resolver.h"

// Generic includes
#include <string.h>

#include "sl_log.h"
#define LOG_TAG "zwave_command_class_meter"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_METER_##type

// ZW_classcmd does not know about v6.
#define COMMAND_CLASS_METER_V6 COMMAND_CLASS_METER_V5
#define METER_VERSION_V6       6

// How many bytes will we parse from the supported scales bitmask:
#define SCALE_BITMASK_MAX_LENTGH 4

// Frame parsing indice
#define METER_REPORT_VALUE_INDEX 4

// Configuration default Attributes. For all versions.
// They all resolve with the Suppported Get Command.
static const attribute_store_type_t default_attributes[]
  = {ATTRIBUTE(RESET_SUPPORTED),
     ATTRIBUTE(SUPPORTED_SCALES),
     ATTRIBUTE(SUPPORTED_RATE_TYPES)};

static const attribute_store_type_t value_attributes[]
  = {ATTRIBUTE(VALUE), ATTRIBUTE(PRECISION)};

static const attribute_store_type_t rate_type_attributes[]
  = {ATTRIBUTE(RATE_TYPE)};

// Frame struct definitions
typedef struct meter_reset_v6_frame {
  uint8_t command_class;
  uint8_t command;
  uint8_t scale_rate_type;
  uint8_t precision_scale_size;
  uint8_t meter_value_1;
  uint8_t meter_value_2;
  uint8_t meter_value_3;
  uint8_t meter_value_4;
  uint8_t scale_2;
} meter_reset_v6_frame_t;

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Checks if an attribute is a Meter value, and undefines its Reported/
 * Desired values if it is.
 *
 * @param node        Attribute store node to check.
 */
static void undefine_meter_values(attribute_store_node_t node)
{
  if (attribute_store_get_node_type(node) == ATTRIBUTE(VALUE)) {
    attribute_store_undefine_desired(node);
    attribute_store_undefine_reported(node);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute resolution functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Prepares a Meter Reset Command.
 *
 * @param node          Attribute Store node to resolve
 * @param frame         Pointer at which frame data can be written.
 * @param frame_length  Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_meter_reset(
  attribute_store_node_t value_node, uint8_t *frame, uint16_t *frame_length)
{
  // First, support check:
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(value_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t reset_supported_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(RESET_SUPPORTED));
  meter_reset_supported_t reset_support = false;
  attribute_store_get_reported(reset_supported_node,
                               &reset_support,
                               sizeof(reset_support));
  if (reset_support == false) {
    sl_log_debug(LOG_TAG,
                 "Meter Reset requested for a non-supporting node. "
                 "Ignoring.");
    attribute_store_undefine_desired(value_node);
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  // Collect all we need from the attribute store.
  meter_value_t value = 0;
  attribute_store_get_desired_else_reported(value_node, &value, sizeof(value));

  attribute_store_node_t rate_type_node
    = attribute_store_get_first_parent_with_type(value_node,
                                                 ATTRIBUTE(RATE_TYPE));
  meter_rate_type_t rate_type = 0;
  attribute_store_get_reported(rate_type_node, &rate_type, sizeof(rate_type));
  meter_scale_t scale = 0;
  attribute_store_node_t scale_node
    = attribute_store_get_first_parent_with_type(rate_type_node,
                                                 ATTRIBUTE(SCALE));
  attribute_store_get_reported(scale_node, &scale, sizeof(scale));
  meter_type_t type = 0;
  attribute_store_node_t type_node
    = attribute_store_get_first_parent_with_type(scale_node, ATTRIBUTE(TYPE));
  attribute_store_get_reported(type_node, &type, sizeof(type));
  attribute_store_node_t precision_node
    = attribute_store_get_first_child_by_type(rate_type_node,
                                              ATTRIBUTE(PRECISION));
  meter_precision_t precision = 0;
  attribute_store_get_desired_else_reported(precision_node,
                                            &precision,
                                            sizeof(precision));

  // Construct the frame:
  meter_reset_v6_frame_t *reset_frame = (meter_reset_v6_frame_t *)frame;
  reset_frame->command_class          = COMMAND_CLASS_METER_V6;
  reset_frame->command                = METER_RESET_V5;
  reset_frame->scale_rate_type
    = (uint8_t)(((rate_type & 0x3) << 5) | (type & 0x1F));
  if (scale >= 4) {
    reset_frame->scale_rate_type |= 0x80;
  }
  // Hardcode size = 4 here.
  reset_frame->precision_scale_size = (uint8_t)(((precision & 0x7) << 5) | 4);
  if (scale >= 7) {
    reset_frame->precision_scale_size |= 0x18;
  } else {
    reset_frame->precision_scale_size |= ((scale & 0x3) << 3);
  }

  reset_frame->meter_value_1 = (value >> 24) & 0xFF;
  reset_frame->meter_value_2 = (value >> 16) & 0xFF;
  reset_frame->meter_value_3 = (value >> 8) & 0xFF;
  reset_frame->meter_value_4 = value & 0xFF;

  if (scale >= 7) {
    reset_frame->scale_2 = (uint8_t)(scale - 8);
  } else {
    reset_frame->scale_2 = 0;
  }

  *frame_length = sizeof(meter_reset_v6_frame_t);
  return SL_STATUS_OK;
}

/**
 * @brief Prepares a Meter Get Command.
 *
 * @param node          Attribute Store node to resolve
 * @param frame         Pointer at which frame data can be written.
 * @param frame_length  Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_meter_get(attribute_store_node_t node,
                                                 uint8_t *frame,
                                                 uint16_t *frame_length)
{
  attribute_store_node_t rate_type_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(RATE_TYPE));
  meter_rate_type_t rate_type = 0;
  attribute_store_get_reported(rate_type_node, &rate_type, sizeof(rate_type));
  attribute_store_node_t scale_node
    = attribute_store_get_first_parent_with_type(rate_type_node,
                                                 ATTRIBUTE(SCALE));
  meter_scale_t scale = 0;
  attribute_store_get_reported(scale_node, &scale, sizeof(scale));

  ZW_METER_GET_V5_FRAME *get_frame = (ZW_METER_GET_V5_FRAME *)frame;
  get_frame->cmdClass              = COMMAND_CLASS_METER_V6;
  get_frame->cmd                   = METER_GET;

  // Insert the rate type into the frame:
  get_frame->properties1 = (uint8_t)(rate_type << 6);
  if (scale < 7) {
    get_frame->properties1 |= ((scale & 0xFF) << 3);
    get_frame->scale2 = 0;
  } else {
    get_frame->properties1 |= ((7 & 0xFF) << 3);
    get_frame->scale2 = (uint8_t)(scale - 8);
  }

  *frame_length = sizeof(ZW_METER_GET_V5_FRAME);
  return SL_STATUS_OK;
}

/**
 * @brief Frame resolution function for meter capabilities. Will send
 * a Supported Get to v2 or newer nodes, and a Meter Get to v1 nodes.
 *
 * @param node          Attribute Store node to resolve
 * @param frame         Pointer at which frame data can be written.
 * @param frame_length  Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_meter_supported_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Make a node version verification (v1 cannot do supported get)
  zwave_cc_version_t supporting_node_version
    = zwave_command_class_get_version_from_node(node, COMMAND_CLASS_METER);

  if (supporting_node_version == 1) {
    ZW_METER_GET_V5_FRAME *get_frame = (ZW_METER_GET_V5_FRAME *)frame;
    get_frame->cmdClass              = COMMAND_CLASS_METER_V6;
    get_frame->cmd                   = METER_GET;
    get_frame->properties1           = 0;
    get_frame->scale2                = 0;
    *frame_length                    = sizeof(ZW_METER_GET_V5_FRAME);
    return SL_STATUS_OK;
  }

  // Else just issue a Supported Get:
  zwave_minimum_frame_t *get_frame = (zwave_minimum_frame_t *)frame;
  get_frame->command_class         = COMMAND_CLASS_METER_V6;
  get_frame->command               = METER_SUPPORTED_GET_V5;
  *frame_length                    = sizeof(zwave_minimum_frame_t);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Resolver callback functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Decides what to do after we've sent a Meter Reset Command.
 *
 * @param node          Attribute Store node that was resolved
 * @param rule_type     The type of rule that was applied (Set/Get)
 * @param event         The outcome of the resolution.
 */
static void on_meter_reset_send_data_complete(attribute_store_node_t node,
                                              resolver_rule_type_t rule_type,
                                              zpc_resolver_event_t event)
{
  if (rule_type == RESOLVER_GET_RULE) {
    // We just did a Meter Get. Ignore.
    return;
  }

  zwave_cc_version_t supporting_node_version
    = zwave_command_class_get_version_from_node(node, COMMAND_CLASS_METER);
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  switch (event) {
    case FRAME_SENT_EVENT_OK_SUPERVISION_WORKING:
      // Wait for more. Weird to have a working Reset
      return;

    case FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT:
      // This is strange. Just give up and avoid "Meter resets" again.
      sl_log_debug(LOG_TAG,
                   "Meter Reset is not supported. "
                   "Deleting attribute store node %d",
                   node);
      const meter_reset_supported_t reset_support = false;
      attribute_store_set_child_reported(endpoint_node,
                                         ATTRIBUTE(RESET_SUPPORTED),
                                         &reset_support,
                                         sizeof(reset_support));
      break;

    default:
      // Probe again in any other case
      if (supporting_node_version >= 6) {
        // Only the scale/value we just resetted in v6
        attribute_store_undefine_desired(node);
        attribute_store_undefine_reported(node);
      } else {
        // Everything should be probed again with < v6
        attribute_store_node_t type_node
          = attribute_store_get_first_child_by_type(endpoint_node,
                                                    ATTRIBUTE(TYPE));
        size_t type_node_index = 0;
        while (type_node != ATTRIBUTE_STORE_INVALID_NODE) {
          attribute_store_walk_tree(type_node, &undefine_meter_values);
          type_node_index += 1;
          type_node = attribute_store_get_node_child_by_type(endpoint_node,
                                                             ATTRIBUTE(TYPE),
                                                             type_node_index);
        }
      }
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute update callbacks
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Attribute callback function listening for Meter version
 *        attribute updates.
 *
 * @param updated_node    Attribute Store node that was modified.
 * @param change          Type of change applied to the node.
 */
static void zwave_command_class_meter_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(COMMAND_CLASS_METER_V6,
                                                      updated_node)
      == true) {
    return;
  }

  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_get_reported(updated_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Wait for the version to be known.
    return;
  }

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  // All versions have these attributes.
  attribute_store_add_if_missing(endpoint_node,
                                 default_attributes,
                                 COUNT_OF(default_attributes));

  // Meter Supported get first arrived in v2.
  // set default capabilities if the node is v1
  if (supporting_node_version == 1) {
    const meter_reset_supported_t reset_support = false;
    attribute_store_set_child_reported(endpoint_node,
                                       ATTRIBUTE(RESET_SUPPORTED),
                                       &reset_support,
                                       sizeof(reset_support));
    const meter_supported_rate_types_t supported_rates = 0;
    attribute_store_set_child_reported(endpoint_node,
                                       ATTRIBUTE(SUPPORTED_RATE_TYPES),
                                       &supported_rates,
                                       sizeof(supported_rates));
  }
}

/**
 * @brief Attribute callback function listening for Supported scales
 *        attribute updates.
 *
 * It will create the list of scales under the supported Meter Type.
 *
 * @param supported_scales_node     Attribute Store node that was modified.
 * @param change                    Type of change applied to the node.
 */
static void zwave_command_class_meter_on_supported_scales_attribute_update(
  attribute_store_node_t supported_scales_node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }
  if (false
      == attribute_store_is_value_defined(supported_scales_node,
                                          REPORTED_ATTRIBUTE)) {
    return;
  }

  uint8_t supported_scales_bitmask[SCALE_BITMASK_MAX_LENTGH] = {};
  attribute_store_get_reported(supported_scales_node,
                               supported_scales_bitmask,
                               sizeof(supported_scales_bitmask));

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(supported_scales_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t type_node
    = attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE(TYPE));

  for (uint8_t byte = 0; byte < SCALE_BITMASK_MAX_LENTGH; byte++) {
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (supported_scales_bitmask[byte] & (1 << bit)) {
        // This scale is supported.
        meter_scale_t scale = byte * 8 + bit;
        sl_log_debug(LOG_TAG,
                     "Creating Meter Scale %d under Attribute ID %d",
                     scale,
                     type_node);
        attribute_store_emplace(type_node,
                                ATTRIBUTE(SCALE),
                                &scale,
                                sizeof(scale));
      }
    }
  }
}

/**
 * @brief Attribute callback function listening for Supported Rate types
 *        attribute updates.
 *
 * It will create the list of Rate types under each supported Meter Scale.
 *
 * @param supported_rate_types_node     Attribute Store node that was modified.
 * @param change                        Type of change applied to the node.
 */
static void zwave_command_class_meter_on_supported_rate_types_attribute_update(
  attribute_store_node_t supported_rate_types_node,
  attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }
  if (false
      == attribute_store_is_value_defined(supported_rate_types_node,
                                          REPORTED_ATTRIBUTE)) {
    return;
  }

  meter_supported_rate_types_t supported_rate_types = 0;
  attribute_store_get_reported(supported_rate_types_node,
                               &supported_rate_types,
                               sizeof(supported_rate_types));

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(supported_rate_types_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t type_node
    = attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE(TYPE));

  attribute_store_node_t scale_node
    = attribute_store_get_first_child_by_type(type_node, ATTRIBUTE(SCALE));
  uint8_t scale_index         = 0;
  meter_rate_type_t rate_type = 0;
  while (scale_node != ATTRIBUTE_STORE_INVALID_NODE) {
    if (supported_rate_types == 0) {
      attribute_store_add_if_missing(scale_node,
                                     rate_type_attributes,
                                     COUNT_OF(rate_type_attributes));
    } else if (supported_rate_types == 1) {
      rate_type = 1;
      attribute_store_emplace(scale_node,
                              ATTRIBUTE(RATE_TYPE),
                              &rate_type,
                              sizeof(rate_type));
    } else if (supported_rate_types == 2) {
      rate_type = 2;
      attribute_store_emplace(scale_node,
                              ATTRIBUTE(RATE_TYPE),
                              &rate_type,
                              sizeof(rate_type));
    } else if (supported_rate_types == 3) {
      rate_type = 1;
      attribute_store_emplace(scale_node,
                              ATTRIBUTE(RATE_TYPE),
                              &rate_type,
                              sizeof(rate_type));
      rate_type = 2;
      attribute_store_emplace(scale_node,
                              ATTRIBUTE(RATE_TYPE),
                              &rate_type,
                              sizeof(rate_type));
    } else {
      sl_log_debug(LOG_TAG,
                   "Error: Unknown supported rate types: %d. "
                   "We will not do anything with it.",
                   supported_rate_types_node);
    }

    scale_index += 1;
    scale_node = attribute_store_get_node_child_by_type(type_node,
                                                        ATTRIBUTE(SCALE),
                                                        scale_index);
  }
}

/**
 * @brief Attribute callback function listening for Rate type
 *        attribute creations.
 *
 * It will create a value/precision set of attributes under
 *
 * @param rate_type_node     Attribute Store node that was modified.
 * @param change             Type of change applied to the node.
 */
static void zwave_command_class_meter_on_rate_types_attribute_created(
  attribute_store_node_t rate_type_node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_CREATED) {
    return;
  }

  attribute_store_add_if_missing(rate_type_node,
                                 value_attributes,
                                 COUNT_OF(value_attributes));
}

///////////////////////////////////////////////////////////////////////////////
// Command Class frame handler function
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Process an incoming Meter Report Command.
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t zwave_command_class_meter_handle_report_command(
  const zwave_controller_connection_info_t *info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  if (frame_length < 5) {
    return SL_STATUS_OK;
  }

  meter_type_t meter_type     = frame[2] & 0x1F;
  meter_rate_type_t rate_type = (frame[2] >> 5) & 0x3;
  meter_scale_t scale = ((frame[2] & 0x80) >> 5) | (frame[3] >> 3 & 0x3);

  uint8_t value_size            = frame[3] & 0x7;
  meter_precision_t precision   = frame[3] >> 5;
  meter_value_t value           = 0;
  meter_delta_time_t delta_time = 0;

  if (frame_length >= (METER_REPORT_VALUE_INDEX + value_size)) {
    const uint8_t *frame_value = &frame[METER_REPORT_VALUE_INDEX];
    value = get_signed_value_from_frame_and_size(frame_value, value_size);
  }

  if (frame_length > (METER_REPORT_VALUE_INDEX + value_size + 2)) {
    delta_time
      = (meter_delta_time_t)(frame[METER_REPORT_VALUE_INDEX + value_size] << 8);
    delta_time |= frame[METER_REPORT_VALUE_INDEX + value_size + 1];
  }

  if (scale == 7) {
    uint8_t scale_2_index = METER_REPORT_VALUE_INDEX + value_size + 2;
    if (delta_time > 0) {
      scale_2_index += value_size;
    }
    if (frame_length > scale_2_index) {
      scale = frame[scale_2_index] + 8;
    } else {
      sl_log_debug(LOG_TAG, "Missing Scale2 value. Ignoring.");
      return SL_STATUS_OK;
    }
  }

  // Make an anouncement.
  sl_log_debug(LOG_TAG,
               "Received Meter reading. Type = %d, Rate Type = %d, Scale = %d, "
               "Value = %d, precision = %d",
               meter_type,
               rate_type,
               scale,
               value,
               precision);

  // We do not care about the Delta Time / Previous value fields.
  // Now save in the attribute store.
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(info);

  attribute_store_node_t type_node
    = attribute_store_emplace(endpoint_node,
                              ATTRIBUTE(TYPE),
                              &meter_type,
                              sizeof(meter_type));
  attribute_store_node_t scale_node = attribute_store_emplace(type_node,
                                                              ATTRIBUTE(SCALE),
                                                              &scale,
                                                              sizeof(scale));

  attribute_store_node_t rate_type_node
    = attribute_store_emplace(scale_node,
                              ATTRIBUTE(RATE_TYPE),
                              &rate_type,
                              sizeof(rate_type));

  attribute_store_set_child_reported(rate_type_node,
                                     ATTRIBUTE(VALUE),
                                     &value,
                                     sizeof(value));
  attribute_store_set_child_reported(rate_type_node,
                                     ATTRIBUTE(PRECISION),
                                     &precision,
                                     sizeof(precision));

  // Just check if we had a node with no Rate type value (due to the "defaul
  // rate type") and erase it now that we know the rate type
  attribute_store_delete_node(
    attribute_store_get_node_child_by_value(scale_node,
                                            ATTRIBUTE(RATE_TYPE),
                                            REPORTED_ATTRIBUTE,
                                            NULL,
                                            0,
                                            0));

  // If this is a v1 node, save that the supported scales are just the scale
  // received in this report.
  zwave_cc_version_t supporting_node_version
    = zwave_command_class_get_version_from_node(type_node, COMMAND_CLASS_METER);
  if (supporting_node_version == 1) {
    uint8_t supported_scales[SCALE_BITMASK_MAX_LENTGH] = {};
    uint8_t byte                                       = (uint8_t)(scale / 8);
    uint8_t bit                                        = (uint8_t)(scale % 8);
    supported_scales[byte]                             = (uint8_t)(1 << bit);
    attribute_store_set_child_reported(endpoint_node,
                                       ATTRIBUTE(SUPPORTED_SCALES),
                                       supported_scales,
                                       sizeof(supported_scales));
  }

  return SL_STATUS_OK;
}

/**
 * @brief Process an incoming Meter Supported Report Command.
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t zwave_command_class_meter_handle_supported_report_command(
  const zwave_controller_connection_info_t *info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  if (frame_length < 4) {
    return SL_STATUS_OK;
  }

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(info);

  meter_reset_supported_t reset_support = frame[2] >> 7;
  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(RESET_SUPPORTED),
                                     &reset_support,
                                     sizeof(reset_support));

  meter_type_t meter_type = frame[2] & 0x1F;
  attribute_store_emplace(endpoint_node,
                          ATTRIBUTE(TYPE),
                          &meter_type,
                          sizeof(meter_type));

  /// Parse the scales before the rate type due to the tree structure
  uint8_t supported_scales_bitmask[SCALE_BITMASK_MAX_LENTGH] = {};
  supported_scales_bitmask[0]                                = frame[3];
  uint8_t bytes_to_follow                                    = 0;
  if (frame[3] >> 7 && frame_length >= 6) {  //MST Present and one more byte:
    const uint8_t BITMASK_INDEX_START = 5;
    bytes_to_follow                   = frame[4];
    uint8_t index                     = 0;
    while (index < bytes_to_follow
           && (index + BITMASK_INDEX_START) < frame_length
           && ((index + 1) < SCALE_BITMASK_MAX_LENTGH)) {
      supported_scales_bitmask[index + 1] = frame[index + BITMASK_INDEX_START];
      index += 1;
    }
  }
  // Make sure not to save the MST bit:
  supported_scales_bitmask[0] &= 0x7F;
  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(SUPPORTED_SCALES),
                                     supported_scales_bitmask,
                                     sizeof(supported_scales_bitmask));

  meter_supported_rate_types_t supported_rate_types = (frame[2] >> 5) & 0x3;
  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(SUPPORTED_RATE_TYPES),
                                     &supported_rate_types,
                                     sizeof(supported_rate_types));

  return SL_STATUS_OK;
}

/** @brief Dispatches incoming Meter Commands to individual
 *        command handlers.
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t zwave_command_class_meter_control_handler(
  const zwave_controller_connection_info_t *info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  if (frame_length < COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame[COMMAND_CLASS_INDEX] != COMMAND_CLASS_METER) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame[COMMAND_INDEX]) {
    case METER_REPORT_V5:
      return zwave_command_class_meter_handle_report_command(info,
                                                             frame,
                                                             frame_length);

    case METER_SUPPORTED_REPORT_V5:
      return zwave_command_class_meter_handle_supported_report_command(
        info,
        frame,
        frame_length);

    default:
      sl_log_debug(LOG_TAG,
                   "Just received a command of the future. "
                   "(or a controlling command). Ignoring");
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_meter_control_init()
{
  // Attribute resolver rules
  attribute_resolver_register_rule(ATTRIBUTE(VALUE),
                                   &zwave_command_class_meter_reset,
                                   &zwave_command_class_meter_get);

  attribute_resolver_register_rule(ATTRIBUTE(SUPPORTED_SCALES),
                                   NULL,
                                   &zwave_command_class_meter_supported_get);

  // Attribute Resolver event listener:
  register_send_event_handler(ATTRIBUTE(VALUE),
                              &on_meter_reset_send_data_complete);

  // Attribute store callbacks
  attribute_store_register_callback_by_type(
    zwave_command_class_meter_on_version_attribute_update,
    ATTRIBUTE(VERSION));

  attribute_store_register_callback_by_type(
    zwave_command_class_meter_on_supported_scales_attribute_update,
    ATTRIBUTE(SUPPORTED_SCALES));

  attribute_store_register_callback_by_type(
    zwave_command_class_meter_on_supported_rate_types_attribute_update,
    ATTRIBUTE(SUPPORTED_RATE_TYPES));

  attribute_store_register_callback_by_type(
    zwave_command_class_meter_on_rate_types_attribute_created,
    ATTRIBUTE(RATE_TYPE));

  // Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler         = &zwave_command_class_meter_control_handler;
  // Not supported, so this does not really matter
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_METER_V6;
  handler.version                    = METER_VERSION_V6;
  handler.command_class_name         = "Meter";
  handler.comments                   = "Partial control: <br>1. Meter Reset "
                                       "Command is not supported due to "
                                       "missing dotdot ZCL equivalent "
                                       "functionality.";

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}