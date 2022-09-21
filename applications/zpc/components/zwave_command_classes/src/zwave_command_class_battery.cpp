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
// Generic includes
#include <assert.h>
#include <stdlib.h>

#include "attribute_resolver.h"
#include "zwave_command_classes_utils.h"
#include "attribute_store_defined_attribute_types.h"
#include "zwave_command_class_battery.h"
#include "zwave_COMMAND_CLASS_BATTERY_attribute_id.h"
#include "zwave_COMMAND_CLASS_BATTERY_handlers.h"
#include "attribute.hpp"
#include "zpc_attribute_store_network_helper.h"
#include "zwave_unid.h"
#include "zwave_command_class_indices.h"
#include "sl_log.h"

// Log tag
constexpr const char *LOG_TAG = "battery_control";

using namespace attribute_store;

// Frame parsing indices
constexpr uint8_t TEMPERATURE_REPORT_VALUE_INDEX = 4;

sl_status_t zwave_BATTERY_HEALTH_REPORT_handle_report_command_override(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  (void)frame_length;
  try {
    // Find out the unid of the sending node:
    unid_t supporting_node_unid;
    attribute node;

    zwave_unid_from_node_id(connection_info->remote.node_id,
                            supporting_node_unid);
    // Get the corresponding Attribute Store entry for this unid / endpoint:
    attribute parent = attribute_store_network_helper_get_endpoint_node(
      supporting_node_unid,
      connection_info->remote.endpoint_id);
    // Get the Maximum Capacity of the Battery
    uint8_t max_capacity = frame[COMMAND_INDEX + 1];
    node = parent.child_by_type(ATTRIBUTE_BATTERY_HEALTH_MAXIMUM_CAPACITY);
    if (!node.is_valid()) {
      sl_log_debug(
        LOG_TAG,
        "Unable to locate BATTERY_HEALTH_MAXIMUM_CAPACITY attribute.");
    } else {
      node.set_reported<uint8_t>(max_capacity);
      node.clear_desired();
    }
    //Get the reported battery temperature size and read the value
    uint8_t size = frame[COMMAND_INDEX + 2] & 0x07;
    node = parent.child_by_type(ATTRIBUTE_BATTERY_HEALTH_BATTERY_TEMPERATURE);
    if (!node.is_valid()) {
      sl_log_debug(
        LOG_TAG,
        "Unable to locate BATTERY_HEALTH_BATTERY_TEMPERATURE attribute.");
    } else {
      if (size > 0) {
        const uint8_t *frame_value = &frame[TEMPERATURE_REPORT_VALUE_INDEX];
        int32_t value = get_signed_value_from_frame_and_size(frame_value, size);
        node.set_reported<int32_t>(value);
        node.clear_desired();
      } else {
        sl_log_debug(LOG_TAG,
                     "Unable to parse the battery temperature value since teh "
                     "value size is [%i] ",
                     size);
      }
    }
    //Get the reported battery temperature scale
    uint8_t scale = frame[COMMAND_INDEX + 2] & 0x18;
    node          = parent.child_by_type(ATTRIBUTE_BATTERY_HEALTH_SCALE);
    if (!node.is_valid()) {
      sl_log_debug(LOG_TAG, "Unable to locate BATTERY_HEALTH_SCALE attribute.");
    } else {
      node.set_reported<uint8_t>(scale);
      node.clear_desired();
    }
    //Get the reported battery temperature precision
    uint8_t precision = frame[COMMAND_INDEX + 2] & 0xE0;
    node = parent.child_by_type(ATTRIBUTE_BATTERY_HEALTH_PRECISION);
    if (!node.is_valid()) {
      sl_log_debug(LOG_TAG,
                   "Unable to locate BATTERY_HEALTH_PRECISION attribute.");
    } else {
      node.set_reported<uint8_t>(precision);
      node.clear_desired();
    }
    return SL_STATUS_OK;
  } catch (const std::exception &e) {
    sl_log_debug(LOG_TAG, "%s: %s", __func__, e.what());
    return SL_STATUS_FAIL;
  }
}

sl_status_t zwave_command_class_battery_control_init()
{
  return zwave_COMMAND_CLASS_BATTERY_init();
}