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
#include "zwave_command_class_indicator_control.h"
#include "zwave_command_class_version_types.h"
#include "attribute_resolver.h"
#include "zwave_command_classes_utils.h"
#include "zwave_COMMAND_CLASS_INDICATOR_handlers.h"
#include "zwave_COMMAND_CLASS_INDICATOR_attribute_id.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute.hpp"
#include "attribute_timeouts.h"
#include "zwave_utils.h"
#include "sl_log.h"
#include <map>
#include <set>
using namespace attribute_store;

constexpr const char *LOG_TAG = "indicator_control";

constexpr int IDENTICATOR_NODE_IDENTIFY = 0x50;
constexpr int PROPERTY_ON_CYCLES        = 0x3;

typedef uint8_t indicator_id_t;
typedef uint8_t indicator_property_t;
typedef int32_t indicator_value_t;

/**
 * @brief add default attributes
 *
 * This code has been copied from the auto handlers, the reason is the
 * the autocode handlers thinks that the property id is a manetory attribute
 * under ATTRIBUTE_INDICATOR_INDICATOR_ID which is not quite true, because
 * it needs to be discovered by the indicator supported frame.
 */
static void
  __zwave_COMMAND_CLASS_INDICATOR_add_INDICATOR_INDICATOR_ID(attribute ep,
                                                             uint8_t val)
{
  attribute version
    = ep.child_by_type(ATTRIBUTE_COMMAND_CLASS_INDICATOR_VERSION);
  attribute node
    = ep.emplace_node<uint8_t>(ATTRIBUTE_INDICATOR_INDICATOR_ID, val);

  if (version.reported<uint8_t>() >= 2) {
    attribute value_node = node.child_by_type(
      ATTRIBUTE_INDICATOR_SUPPORTED_PROPERTY_SUPPORTED_BIT_MASK);
    if (!value_node.is_valid()) {
      node.add_node(ATTRIBUTE_INDICATOR_SUPPORTED_PROPERTY_SUPPORTED_BIT_MASK);
    }
  }
}

/**
 * This override prevents the resolver from sending indicator get from
 * the probing of the capabilities get
 */
sl_status_t zwave_INDICATOR_GET_override(attribute_store_node_t _node,
                                         uint8_t *frame,
                                         uint16_t *frame_len)
{
  try {
    attribute indicator
      = attribute(_node).first_parent(ATTRIBUTE_INDICATOR_INDICATOR_ID);
    // If we have properties under the indicator id.
    if (indicator.is_valid()
        && (indicator.children(ATTRIBUTE_INDICATOR_PROPERTY_ID).size() > 0)) {
      frame[0]   = 0x87;
      frame[1]   = 0x02;
      frame[2]   = indicator.reported<indicator_id_t>();
      *frame_len = 3;
      return SL_STATUS_OK;
    }
  } catch (const std::invalid_argument &) {
    //Fall through
  }

  *frame_len = 1;
  return SL_STATUS_IS_WAITING;
}

/**
 * This override prevents the resolver from sending indicator get from
 * the probing of the capabilities get
 */
sl_status_t zwave_INDICATOR_SUPPORTED_GET_override(attribute_store_node_t _node,
                                                   uint8_t *frame,
                                                   uint16_t *frame_len)
{
  try {
    attribute indicator
      = attribute(_node).first_parent(ATTRIBUTE_INDICATOR_INDICATOR_ID);
    if (indicator.is_valid()) {
      frame[0]   = 0x87;
      frame[1]   = 0x04;
      frame[2]   = indicator.reported<indicator_id_t>();
      *frame_len = 3;
      return SL_STATUS_OK;
    }
  } catch (const std::invalid_argument &) {
    //Fall through
  }
  *frame_len = 1;
  return SL_STATUS_IS_WAITING;
}

sl_status_t zwave_INDICATOR_SET_override(attribute_store_node_t _node,
                                         uint8_t *frame,
                                         uint16_t *frame_len)
{
  attribute node(_node);
  attribute indicator  = node.first_parent(ATTRIBUTE_INDICATOR_INDICATOR_ID);
  uint8_t offset       = 0;
  uint8_t object_count = 0;
  frame[offset++]      = 0x87;
  frame[offset++]      = 0x01;  // INDICATOR_SET
  frame[offset++]      = 0x00;  // Value 0

  offset++;  //skip the object count field for nor

  for (attribute indicator_property:
       indicator.children(ATTRIBUTE_INDICATOR_PROPERTY_ID)) {
    try {
      frame[offset++] = indicator.reported<indicator_id_t>();
      frame[offset++] = indicator_property.reported<indicator_property_t>();
      indicator_value_t value
        = indicator_property.child_by_type(ATTRIBUTE_INDICATOR_INDICATOR_VALUE)
            .desired_or_reported<indicator_value_t>();
      frame[offset++] = value;
      // For V1 compatiblity we set the value to FF if there are any properties which has non zero value
      if (value != 0) {
        frame[2] = 0xff;
      }
      object_count++;
    } catch (std::invalid_argument &) {
      //pass
    }
  }
  //Update the object count
  frame[3]   = object_count;
  *frame_len = offset;

  return SL_STATUS_OK;
}

sl_status_t zwave_INDICATOR_REPORT_handle_report_command_override(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  // Find out the endpoint node of the sending node:
  attribute endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  indicator_value_t indicator0_value = frame[2];
  uint8_t object_count               = frame[3] & 0x1f;
  indicator_id_t indicator_id        = frame[4];

  //Handle V1 report
  if (frame_length == 3) {
    endpoint_node.emplace_node(ATTRIBUTE_INDICATOR_INDICATOR_ID)
      .emplace_node(ATTRIBUTE_INDICATOR_PROPERTY_ID)
      .emplace_node(ATTRIBUTE_INDICATOR_INDICATOR_VALUE)
      .set_reported<indicator_value_t>(indicator0_value);
    return SL_STATUS_OK;
  }

  // validate frame
  if (frame_length < (4 + object_count * 3)) {
    return SL_STATUS_OK;
  }

  attribute indicator = endpoint_node.child_by_type_and_value<indicator_id_t>(
    ATTRIBUTE_INDICATOR_INDICATOR_ID,
    indicator_id);

  if (!indicator.is_valid()) {
    sl_log_debug(LOG_TAG, "Unknown indicator id");
    return SL_STATUS_OK;
  }

  std::map<indicator_property_t, indicator_value_t> props;
  uint8_t offset = 4;
  for (uint8_t i = 0; i < object_count; i++) {
    if (frame[offset++] != indicator_id) {
      sl_log_debug(LOG_TAG,
                   "All indicator objects MUST carry the same Indicator ID");
      offset += 2;
      continue;
    }
    props.insert(std::make_pair(frame[offset], frame[offset + 1]));
    offset += 2;
  }

  for (attribute property:
       indicator.children(ATTRIBUTE_INDICATOR_PROPERTY_ID)) {
    if (property.reported_exists()) {
      indicator_value_t value = 0;
      indicator_property_t property_id
        = property.reported<indicator_property_t>();
      if (props.count(property_id)) {
        value = props.at(property_id);
      }
      property.emplace_node(ATTRIBUTE_INDICATOR_INDICATOR_VALUE)
        .set_reported<indicator_value_t>(value);
    }
  }
  return SL_STATUS_OK;
}

sl_status_t zwave_INDICATOR_SUPPORTED_REPORT_handle_report_command_override(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  // Find out the endpoint node of the sending node:
  attribute endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  // Validate data
  if (frame_length < 4) {
    return SL_STATUS_OK;
  }

  uint8_t indicator_id      = frame[2];
  uint8_t next_indicator_id = frame[3];
  uint8_t bit_mask_length   = (frame[4] & 0x1f);
  const uint8_t *bit_mask   = &frame[5];

  // Validate data
  if (frame_length < (bit_mask_length + 4)) {
    return SL_STATUS_OK;
  }

  // Check if we need to add more indicators
  if (next_indicator_id != 0) {
    __zwave_COMMAND_CLASS_INDICATOR_add_INDICATOR_INDICATOR_ID(
      endpoint_node,
      next_indicator_id);
  }

  attribute indicator
    = endpoint_node.child_by_type_and_value<indicator_property_t>(
      ATTRIBUTE_INDICATOR_INDICATOR_ID,
      indicator_id);

  // Check if we have inserted indicator 0, then we need to overwrite this with the
  // ID from this report
  if (!indicator.is_valid()) {
    indicator = endpoint_node.child_by_type_and_value<indicator_id_t>(
      ATTRIBUTE_INDICATOR_INDICATOR_ID,
      0);
    if (indicator.is_valid()) {
      indicator.set_reported<indicator_id_t>(indicator_id);
    } else {
      sl_log_warning(LOG_TAG, "Unable to find the reported indicator id");
      return SL_STATUS_OK;
    }
  }

  // fill out the supported mask such we dont really need this again
  indicator
    .emplace_node(ATTRIBUTE_INDICATOR_SUPPORTED_PROPERTY_SUPPORTED_BIT_MASK)
    .set_reported(std::vector<uint8_t>(bit_mask, bit_mask + bit_mask_length));

  //Clean out indicators which has not reported value
  for (auto property_id: indicator.children(ATTRIBUTE_INDICATOR_PROPERTY_ID)) {
    if (!property_id.reported_exists()) {
      property_id.delete_node();
    }
  }

  // Note: Bit 0 in Bit Mask 1 is not allocated to any property and MUST be set to zero
  for (int bit = 1; (bit < (bit_mask_length * 8)); bit++) {
    if (bit_mask[bit / 8] & (1 << (bit & 7))) {
      indicator.emplace_node<uint8_t>(ATTRIBUTE_INDICATOR_PROPERTY_ID, bit)
        .emplace_node(ATTRIBUTE_INDICATOR_INDICATOR_VALUE);
    }
  }

  //Don't use the auto handler
  return SL_STATUS_OK;
}

static void on_timeout_zero_properties(attribute_store_node_t indicator)
{
  //We do not want to zero out the Low power property
  const std::set<indicator_property_t> exceptions = {0x10};
  //Set all values to 0
  attribute(indicator).visit([=](attribute &n, int) -> sl_status_t {
    if (n.type() == ATTRIBUTE_INDICATOR_PROPERTY_ID) {
      // Check id property is in exception list
      if (n.reported_exists()
          && exceptions.count(n.reported<indicator_property_t>())) {
        return SL_STATUS_SUSPENDED;
      }
    } else if (n.type() == ATTRIBUTE_INDICATOR_INDICATOR_VALUE) {
      n.set_reported<indicator_value_t>(0);
    }
    return SL_STATUS_OK;
  });
}

/**
 * @brief Check if an indicator property is volatile and needs to be updated
 *
 * @param _updated_node
 * @param change
 */
static void on_indicator_timeout_updated(attribute_store_node_t updated_node,
                                         attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  attribute timeout   = updated_node;
  attribute indicator = timeout.parent();

  if (timeout.reported_exists()) {
    try {
      attribute_timeout_set_callback(indicator,
                                     timeout.reported<int32_t>(),
                                     on_timeout_zero_properties);
    } catch (const std::invalid_argument &e) {
      sl_log_warning(LOG_TAG,
                     "Could not process the Indicator Timeout value for "
                     "node %d. No update will be made",
                     timeout);
      attribute_store_log_node(timeout, true);
    }
  }
}

static void on_version_attribute_update(attribute_store_node_t _updated_node,
                                        attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  uint8_t version = 0;
  attribute updated_node(_updated_node);
  attribute endpoint_node = updated_node.parent();

  try {
    version = updated_node.reported<zwave_cc_version_t>();
  } catch (const std::invalid_argument &e) {
    return;
  }

  if (!endpoint_node.child_by_type(ATTRIBUTE_INDICATOR_INDICATOR_ID)
         .is_valid()) {
    if (version == 1) {
      //For V1 we fake a property, the choice of PROPERTY_ON_CYCLES is kind of random.
      endpoint_node
        .emplace_node<indicator_id_t>(ATTRIBUTE_INDICATOR_INDICATOR_ID,
                                      IDENTICATOR_NODE_IDENTIFY)
        .emplace_node<indicator_property_t>(ATTRIBUTE_INDICATOR_PROPERTY_ID,
                                            PROPERTY_ON_CYCLES)
        .emplace_node(ATTRIBUTE_INDICATOR_INDICATOR_VALUE);
    } else {
      //Add indicator 0 which is always there
      __zwave_COMMAND_CLASS_INDICATOR_add_INDICATOR_INDICATOR_ID(endpoint_node,
                                                                 0);
    }
  }
}

sl_status_t zwave_command_class_indicator_control_init()
{
  attribute_store_register_callback_by_type(
    on_version_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_INDICATOR_VERSION);

  attribute_store_register_callback_by_type(
    on_indicator_timeout_updated,
    ATTRIBUTE_COMMAND_CLASS_INDICATOR_TIMEOUT);

  attribute_resolver_set_attribute_depth(ATTRIBUTE_INDICATOR_INDICATOR_VALUE,
                                         2);
  return zwave_COMMAND_CLASS_INDICATOR_init();
}
