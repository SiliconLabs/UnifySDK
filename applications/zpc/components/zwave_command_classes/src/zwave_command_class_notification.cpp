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
#include "zwave_command_class_notification.h"
#include "zwave_command_class_notification_types.h"
#include "notification_command_class_defined_notifications.h"
#include "zwave_command_class_indices.h"
#include "zwave_command_classes_utils.h"
#include "zwave_command_class_agi.h"

// Includes from other components
#include "sl_log.h"
#include "zwave_unid.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_command_handler.h"
#include "attribute_resolver.h"
#include "zwave_controller_utils.h"
#include "zwave_utils.h"

// Generic includes
#include <assert.h>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <attribute.hpp>

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_##type

// Log tag
static constexpr char LOG_TAG[] = "zwave_command_class_notification";
static constexpr unsigned int MAX_SUPPORTED_NOTIFICATION_TYPES  = 20;
static constexpr unsigned int MAX_SUPPORTED_NOTIFICATION_STATES = 256;
static constexpr unsigned int NOTIFICATION_REPORT_EVENT_STATE_PARAMETER_OFFSET
  = 9;

using namespace attribute_store;

#include "zwave_command_class_notification_types.inc"

static sl_status_t zwave_command_class_notification_update_state_event(
  const attribute &notification_type_node,
  uint8_t state,
  const ZW_NOTIFICATION_REPORT_1BYTE_V3_FRAME *frame,
  uint16_t frame_length)
{
  int32_t notification_event = frame->mevent;
  if (notification_event == 0xFE) {
    notification_event = 0;
  }

  attribute notification_event_node
    = attribute_store_get_node_child_by_value(notification_type_node,
                                              ATTRIBUTE(STATE),
                                              REPORTED_ATTRIBUTE,
                                              &state,
                                              sizeof(state),
                                              0);
  notification_event_node
    = notification_event_node.child_by_type(ATTRIBUTE(EVENT), 0);

  if (!notification_event_node.is_valid()) {
    sl_log_debug(LOG_TAG,
                 "Failed to find the State -> Event node under Notification "
                 "Type Attribute %d, for State value %d",
                 notification_type_node,
                 state);
    return SL_STATUS_OK;
  }
  notification_event_node.set_reported(notification_event);

  uint8_t state_event_param_len
    = frame->properties1
      & NOTIFICATION_REPORT_PROPERTIES1_EVENT_PARAMETERS_LENGTH_MASK_V4;
  attribute notification_event_param
    = notification_event_node.child_by_type(ATTRIBUTE(EVENT_PARAMETERS), 0);
  if (!notification_event_param.is_valid()) {
    sl_log_debug(
      LOG_TAG,
      "Failed to find the Event Parameter attribute under attribute %d",
      notification_event_node);
    return SL_STATUS_OK;
  }
  if (state_event_param_len > 0
      && frame_length >= (NOTIFICATION_REPORT_EVENT_STATE_PARAMETER_OFFSET
                          + state_event_param_len)) {
    // Update the parameters based on the size, also if the event parameter
    // Length (state_event_param_len) is zero (attribute back to undefined)
    int32_t state_event = frame->eventParameter1;
    notification_event_param.set_reported(state_event);
  } else {
    // No Event/state parameter byte added to the payload, we just undefine the
    // value again.
    notification_event_param.clear_reported();
  }
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_notification_report_cmd_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 3) {
    return SL_STATUS_OK;
  }

  const auto *frame
    = reinterpret_cast<const ZW_NOTIFICATION_REPORT_1BYTE_V3_FRAME *>(
      frame_data);

  // Get the unid of the sending node
  unid_t sending_node_unid;
  zwave_unid_from_node_id(connection_info->remote.node_id, sending_node_unid);

  // Get the corresponding attribute store id for the sending node endpoint
  try {
    attribute ep_node = attribute_store_network_helper_get_endpoint_node(
      sending_node_unid,
      connection_info->remote.endpoint_id);

    // Handling V1 Alarm Type and Level, they are proprietary Type and Level.
    // The values are usually specified in the product manual.
    // If the V1 Alarm Type is not supported, these fields MUST be set to 0x00.
    // ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_V1_ALARM_TYPE should be added under
    // endpoint attribute and the level shall be under TYPE attribute:
    // EP --> ..V1_ALARAM_TYPE-->..V1_ALARM_LEVEL
    const uint8_t v1_alarm_type = frame->v1AlarmType;
    attribute v1_alarm_type_node;
    if (v1_alarm_type != 0) {
      v1_alarm_type_node
        = attribute_store_get_node_child_by_value(ep_node,
                                                  ATTRIBUTE(V1_ALARM_TYPE),
                                                  REPORTED_ATTRIBUTE,
                                                  &v1_alarm_type,
                                                  sizeof(v1_alarm_type),
                                                  0);
      if (v1_alarm_type_node.is_valid()) {
        v1_alarm_type_node.child_by_type(ATTRIBUTE(V1_ALARM_LEVEL))
          .set_reported(frame->v1AlarmLevel);
      } else {
        v1_alarm_type_node = ep_node.add_node(ATTRIBUTE(V1_ALARM_TYPE));
        v1_alarm_type_node.set_reported(v1_alarm_type);
        attribute vl_alarm_level_node
          = v1_alarm_type_node.add_node(ATTRIBUTE(V1_ALARM_LEVEL));
        vl_alarm_level_node.set_reported(frame->v1AlarmLevel);
      }
    }

    const uint8_t notification_type = frame->notificationType;
    attribute notification_type_node
      = attribute_store_get_node_child_by_value(ep_node,
                                                ATTRIBUTE(TYPE),
                                                REPORTED_ATTRIBUTE,
                                                &notification_type,
                                                sizeof(notification_type),
                                                0);
    if (!notification_type_node.is_valid()) {
      sl_log_warning(LOG_TAG, "Failed to lookup attribute %d", __LINE__);
      return SL_STATUS_OK;
    }

    uint8_t event = frame->mevent;
    // If event is 0 it we find the event to use for state lookup in the event
    // parameters, as due to spec the "IDLE" shall have the event to set to idle
    // as paramter.
    if (((event == 0)
         && (frame->properties1
             & NOTIFICATION_REPORT_PROPERTIES1_EVENT_PARAMETERS_LENGTH_MASK_V8)
              == 0)
        || (event == 0xFE)) {
      sl_log_debug(LOG_TAG,
                   "Notification Idle Event without any event parameters or "
                   "Event 0xFE received. Setting all states to idle.");
      std::set<unsigned int> updated_states;
      for (auto const &elem:
           notification_event_state_map.at(frame->notificationType)) {
        try {
          if (updated_states.find(elem.second) != updated_states.end()) {
            // State is already set to idle
            continue;
          }
          updated_states.insert(elem.second);
          zwave_command_class_notification_update_state_event(
            notification_type_node,
            elem.second,
            frame,
            frame_length);
        } catch (const std::exception &ex) {
          // continue with next state in case of erorrs
          sl_log_warning(
            LOG_TAG,
            "Failed to write Notification Event to attribute store: %s",
            ex.what());
        }
      }

      // We always return OK here, even for errors
      return SL_STATUS_OK;
    }

    if (event == 0) {
      event = frame->eventParameter1;
    }

    uint8_t state;
    try {
      // Lookup if the event is a state change
      state = static_cast<uint8_t>(
        notification_event_state_map.at(frame->notificationType).at(event));
    } catch (const std::exception &e) {
      // If no state is looked up it is an event, write to LAST_EVENT attribute
      state = NOTIFICATION_STATE_LAST_EVENT;
    }
    zwave_command_class_notification_update_state_event(notification_type_node,
                                                        state,
                                                        frame,
                                                        frame_length);

  } catch (const std::exception &ex) {
    // do nothing
    sl_log_warning(LOG_TAG, "%s", ex.what());
  }
  return SL_STATUS_OK;
}

static sl_status_t
  zwave_command_class_supported_notification_types_report_cmd_handler(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  if (frame_length < 3) {
    return SL_STATUS_OK;
  }

  // Get the unid of the sending node
  unid_t sending_node_unid;
  zwave_unid_from_node_id(connection_info->remote.node_id, sending_node_unid);
  // Get the corresponding attribute store id for the sending node endpoint
  attribute_store_node_t ep_node
    = attribute_store_network_helper_get_endpoint_node(
      sending_node_unid,
      connection_info->remote.endpoint_id);

  zwave_cc_version_t version
    = zwave_node_get_command_class_version(COMMAND_CLASS_NOTIFICATION_V8,
                                           connection_info->remote.node_id,
                                           connection_info->remote.endpoint_id);
  // Extracting supported notification types for Notification_CC_V4
  uint16_t notification_types_bits               = 0x0000;
  uint16_t notification_type_mask                = 0x0000;
  uint8_t number_of_supported_notification_types = 0;
  uint8_t supported_notification_types[MAX_SUPPORTED_NOTIFICATION_TYPES];
  uint8_t number_of_bit_masks
    = frame_data[2]
      & NOTIFICATION_SUPPORTED_REPORT_PROPERTIES1_NUMBER_OF_BIT_MASKS_MASK_V4;
  if (number_of_bit_masks == 1) {
    notification_types_bits = frame_data[3];
  } else if (number_of_bit_masks == 2) {
    notification_types_bits = (frame_data[4] << 8) | frame_data[3];
  } else {
    sl_log_debug(LOG_TAG,
                 "Supported notification types Bit Masks length is wrong\n");
  }

  for (size_t i = 1; i <= MAX_SUPPORTED_NOTIFICATION_TYPES; i++) {
    notification_type_mask = 1 << i;
    notification_type_mask &= notification_types_bits;
    if (notification_type_mask) {
      switch (notification_type_mask) {
        case 0x0002:
          supported_notification_types[number_of_supported_notification_types]
            = NOTIFICATION_REPORT_SMOKE_V4;
          break;
        case 0x0004:
          supported_notification_types[number_of_supported_notification_types]
            = NOTIFICATION_REPORT_CO_V4;
          break;
        case 0x0008:
          supported_notification_types[number_of_supported_notification_types]
            = NOTIFICATION_REPORT_CO2_V4;
          break;
        case 0x0010:
          supported_notification_types[number_of_supported_notification_types]
            = NOTIFICATION_REPORT_HEAT_V4;
          break;
        case 0x0020:
          supported_notification_types[number_of_supported_notification_types]
            = NOTIFICATION_REPORT_WATER_V4;
          break;
        case 0x0040:
          supported_notification_types[number_of_supported_notification_types]
            = NOTIFICATION_REPORT_ACCESS_CONTROL_V4;
          break;
        case 0x0080:
          supported_notification_types[number_of_supported_notification_types]
            = NOTIFICATION_REPORT_HOME_SECURITY_V4;
          break;
        case 0x0100:
          supported_notification_types[number_of_supported_notification_types]
            = NOTIFICATION_REPORT_POWER_MANAGEMENT_V4;
          break;
        case 0x0200:
          supported_notification_types[number_of_supported_notification_types]
            = NOTIFICATION_REPORT_SYSTEM_V4;
          break;
        case 0x0400:
          supported_notification_types[number_of_supported_notification_types]
            = NOTIFICATION_REPORT_EMERGENCY_V4;
          break;
        case 0x0800:
          supported_notification_types[number_of_supported_notification_types]
            = NOTIFICATION_REPORT_CLOCK_V4;
          break;
        case 0x1000:
          supported_notification_types[number_of_supported_notification_types]
            = NOTIFICATION_REPORT_APPLIANCE_V4;
          break;
        case 0x2000:
          supported_notification_types[number_of_supported_notification_types]
            = NOTIFICATION_REPORT_HOME_HEALTH_V4;
          break;
        case 0x4000:
          supported_notification_types[number_of_supported_notification_types]
            = NOTIFICATION_GET_SIREN_V6;
          break;
        case 0x8000:
          supported_notification_types[number_of_supported_notification_types]
            = NOTIFICATION_GET_WATER_VALVE_V7;
          break;
      }
      number_of_supported_notification_types++;
    }
  }

  // Update the notification types reported values in the Attribute Store
  attribute_store_node_t supported_notification_types_node
    = attribute_store_get_first_child_by_type(
      ep_node,
      ATTRIBUTE(SUPPORTED_NOTIFICATION_TYPES));
  attribute_store_set_node_attribute_value(
    supported_notification_types_node,
    REPORTED_ATTRIBUTE,
    supported_notification_types,
    number_of_supported_notification_types);

  for (uint8_t i = 0; i < number_of_supported_notification_types; i++) {
    attribute_store_node_t notification_type
      = attribute_store_get_node_child_by_value(
        ep_node,
        ATTRIBUTE(TYPE),
        REPORTED_ATTRIBUTE,
        &supported_notification_types[i],
        sizeof(supported_notification_types[i]),
        0);
    if (notification_type == ATTRIBUTE_STORE_INVALID_NODE) {
      notification_type = attribute_store_add_node(ATTRIBUTE(TYPE), ep_node);

      attribute_store_set_node_attribute_value(
        notification_type,
        REPORTED_ATTRIBUTE,
        &supported_notification_types[i],
        sizeof(supported_notification_types[i]));
      if (version > 2) {
        attribute_store_add_node(ATTRIBUTE(SUPPORTED_STATES_OR_EVENTS),
                                 notification_type);
      }
    }
  }

  return SL_STATUS_OK;
}

static sl_status_t
  zwave_command_class_supported_state_event_types_report_cmd_handler(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  if (frame_length < 4) {
    sl_log_debug(LOG_TAG,
                 "Received an Event Supported Report Command with "
                 "too small frame length (%d). Ignoring",
                 frame_length);
    return SL_STATUS_OK;
  }

  const auto *report_frame
    = reinterpret_cast<const ZW_EVENT_SUPPORTED_REPORT_1BYTE_V3_FRAME *>(
      frame_data);
  unsigned int number_of_bytes
    = (report_frame->properties1
       & EVENT_SUPPORTED_REPORT_PROPERTIES1_NUMBER_OF_BIT_MASKS_MASK_V3);
  if (number_of_bytes == 0) {
    sl_log_debug(LOG_TAG,
                 "The Notification Type is not supported. It seems that we "
                 "sent a wrong Get Command or that the supporting node "
                 "did not report correct data.");
    return SL_STATUS_OK;
  }
  // Get the unid of the sending node
  unid_t sending_node_unid;
  zwave_unid_from_node_id(connection_info->remote.node_id, sending_node_unid);
  // Get the corresponding attribute store id for the sending node endpoint
  attribute_store_node_t ep_node
    = attribute_store_network_helper_get_endpoint_node(
      sending_node_unid,
      connection_info->remote.endpoint_id);
  if (ATTRIBUTE_STORE_INVALID_NODE == ep_node) {
    sl_log_error(LOG_TAG,
                 "Failed to lookup Endpoint (%d)",
                 connection_info->remote.endpoint_id);
    return SL_STATUS_OK;
  }

  attribute_store_node_t notification_node
    = attribute_store_get_node_child_by_value(
      ep_node,
      ATTRIBUTE(TYPE),
      REPORTED_ATTRIBUTE,
      &report_frame->notificationType,
      sizeof(report_frame->notificationType),
      0);
  if (ATTRIBUTE_STORE_INVALID_NODE == notification_node) {
    sl_log_debug(LOG_TAG,
                 "Failed to lookup Notification Type (%d) under Endpoint (%d)",
                 report_frame->notificationType,
                 connection_info->remote.endpoint_id);
    return SL_STATUS_OK;
  }

  attribute_store_node_t supported_state_or_event_node
    = attribute_store_get_first_child_by_type(
      notification_node,
      ATTRIBUTE(SUPPORTED_STATES_OR_EVENTS));
  if (ATTRIBUTE_STORE_INVALID_NODE == supported_state_or_event_node) {
    sl_log_debug(LOG_TAG,
                 "Failed to lookup Notification Supported Event State "
                 "Attribute under Notification Type (%d)",
                 report_frame->notificationType);
    return SL_STATUS_OK;
  }

  std::vector<uint8_t> supported_events;
  bool create_last_event_attribute = false;
  // Here we loop over all bits in the bitmask and lookup if the event is
  // actually a state change. If it's a state change we create an attribute for
  // the state.
  for (size_t byte_idx = 0; byte_idx < number_of_bytes; byte_idx++) {
    const uint8_t byte = (&(report_frame->bitMask1))[byte_idx];
    for (size_t bit_idx = 0; bit_idx < 8; bit_idx++) {
      if (((byte >> bit_idx) & 1) == 1) {
        const uint8_t event = byte_idx * 8 + bit_idx;
        supported_events.push_back(event);
        try {
          // Lookup if the event is a state change
          uint8_t state
            = notification_event_state_map.at(report_frame->notificationType)
                .at(event);
          // Lookup if the state node is already created in the attribute store
          attribute_store_node_t notification_state_node
            = attribute_store_get_node_child_by_value(notification_node,
                                                      ATTRIBUTE(STATE),
                                                      REPORTED_ATTRIBUTE,
                                                      &state,
                                                      sizeof(state),
                                                      0);
          if (notification_state_node != ATTRIBUTE_STORE_INVALID_NODE) {
            // Node already exists, continue with next bit
            continue;
          }
          notification_state_node
            = attribute_store_add_node(ATTRIBUTE(STATE), notification_node);
          if (notification_state_node == ATTRIBUTE_STORE_INVALID_NODE) {
            sl_log_error(LOG_TAG,
                         "Failed to create attribute for the Notification "
                         "State under attribute %d.",
                         notification_state_node);
            assert(false);
            continue;
          }
          const attribute_store_node_t event_type
            = attribute_store_add_node(ATTRIBUTE(EVENT),
                                       notification_state_node);
          if (event_type == ATTRIBUTE_STORE_INVALID_NODE) {
            sl_log_error(LOG_TAG,
                         "Failed to create attribute for the Notification "
                         "Event Type under attribute %d.",
                         notification_state_node);
            assert(false);
            continue;
          }
          if (ATTRIBUTE_STORE_INVALID_NODE
              == attribute_store_add_node(ATTRIBUTE(EVENT_PARAMETERS),
                                          event_type)) {
            sl_log_error(LOG_TAG,
                         "Failed to create attribute for the Notification "
                         "Event Parameters under attribute %d.",
                         event_type);
            assert(false);
            continue;
          }
          attribute_store_set_node_attribute_value(notification_state_node,
                                                   REPORTED_ATTRIBUTE,
                                                   &state,
                                                   sizeof(state));
        } catch (const std::exception &e) {
          // Event is not a state change, this is okay just continue.
          // Set that last_event attribute shall be created
          create_last_event_attribute = true;
        }
      }
    }
  }
  // Store supported events as array in attribute store
  attribute_store_set_node_attribute_value(supported_state_or_event_node,
                                           REPORTED_ATTRIBUTE,
                                           supported_events.data(),
                                           supported_events.size());
  if (create_last_event_attribute) {
    // Create attribute for last received event, if any non-state events exist
    // for the notification
    uint8_t state = NOTIFICATION_STATE_LAST_EVENT;
    // Lookup if the state node is already created in the attribute store
    attribute_store_node_t notification_state_node
      = attribute_store_get_node_child_by_value(notification_node,
                                                ATTRIBUTE(STATE),
                                                REPORTED_ATTRIBUTE,
                                                &state,
                                                sizeof(state),
                                                0);
    if (notification_state_node != ATTRIBUTE_STORE_INVALID_NODE) {
      // Node already exists, continue with next bit
    } else {
      notification_state_node
        = attribute_store_add_node(ATTRIBUTE(STATE), notification_node);
      if (notification_state_node == ATTRIBUTE_STORE_INVALID_NODE) {
        sl_log_error(LOG_TAG,
                     "Failed to create attribute for the Notification "
                     "State under attribute %d.",
                     notification_state_node);
        assert(false);
      }
      attribute_store_set_node_attribute_value(notification_state_node,
                                               REPORTED_ATTRIBUTE,
                                               &state,
                                               sizeof(state));
    }
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_notification_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_FAIL;
  }
  if (frame_data[COMMAND_CLASS_INDEX] != COMMAND_CLASS_NOTIFICATION_V8) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case NOTIFICATION_REPORT_V4:
      return zwave_command_class_notification_report_cmd_handler(
        connection_info,
        frame_data,
        frame_length);

    case NOTIFICATION_SUPPORTED_REPORT_V4:
      return zwave_command_class_supported_notification_types_report_cmd_handler(
        connection_info,
        frame_data,
        frame_length);

    case EVENT_SUPPORTED_REPORT_V4:
      return zwave_command_class_supported_state_event_types_report_cmd_handler(
        connection_info,
        frame_data,
        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

/**
 * @brief A function that detects the support of the Notification CC and
 * creates the required Attributes for the Notification Command Class
 * sub-component.
 *
 * @param updated_node The Attribute Store node that just got updated.
 * @param change       The Attribute Store change that the node underwent.
 */
void zwave_command_class_notification_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(
        COMMAND_CLASS_NOTIFICATION_V8,
        updated_node)
      == true) {
    return;
  }
  // Check that we have the right type of attribute.
  assert(ATTRIBUTE(VERSION) == attribute_store_get_node_type(updated_node));

  zwave_node_id_t node_id         = 0;
  zwave_endpoint_id_t endpoint_id = 0;

  if (SL_STATUS_OK
      != attribute_store_network_helper_get_zwave_ids_from_node(updated_node,
                                                                &node_id,
                                                                &endpoint_id)) {
    assert(false && "Should always be able to get command class version here");
    return;
  }
  zwave_cc_version_t version
    = zwave_node_get_command_class_version(COMMAND_CLASS_NOTIFICATION_V8,
                                           node_id,
                                           endpoint_id);

  // For version 1 Notification Supported Get is not available
  if (version > 1) {
    // Here we know that a node supported Notification Command Class.
    attribute_store_node_t parent_node
      = attribute_store_get_node_parent(updated_node);

    // Verify if there is already supported notification types attribute.
    // Note that updated_node is the ENDPOINT node in the Attribute Store.
    attribute_store_node_t supported_notification_type_node
      = attribute_store_get_first_child_by_type(
        parent_node,
        ATTRIBUTE(SUPPORTED_NOTIFICATION_TYPES));
    // If not, create it
    if (supported_notification_type_node == ATTRIBUTE_STORE_INVALID_NODE) {
      attribute_store_add_node(ATTRIBUTE(SUPPORTED_NOTIFICATION_TYPES),
                               parent_node);
    }
  }
}

static sl_status_t zwave_command_class_notification_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_len)
{
  auto notification_get_frame
    = reinterpret_cast<ZW_NOTIFICATION_GET_V4_FRAME *>(frame);
  notification_get_frame->cmdClass    = COMMAND_CLASS_NOTIFICATION_V8;
  notification_get_frame->cmd         = NOTIFICATION_GET_V8;
  notification_get_frame->v1AlarmType = 0x00;
  // read the Notification type
  attribute_store_node_t type_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(TYPE));
  uint8_t notification_type;
  attribute_store_get_reported(type_node,
                               &notification_type,
                               sizeof(notification_type));
  notification_get_frame->notificationType = notification_type;

  // Get Notification Event / State from attribute store
  uint8_t notification_state;
  attribute_store_get_reported(
    attribute_store_get_first_parent_with_type(node, ATTRIBUTE(STATE)),
    &notification_state,
    sizeof(notification_state));
  uint8_t supported_notification_states[MAX_SUPPORTED_NOTIFICATION_STATES];
  uint8_t number_of_supported_states;
  attribute_store_node_t supported_states_node
    = attribute_store_get_first_child_by_type(
      type_node,
      ATTRIBUTE(SUPPORTED_STATES_OR_EVENTS));
  attribute_store_get_node_attribute_value(supported_states_node,
                                           REPORTED_ATTRIBUTE,
                                           supported_notification_states,
                                           &number_of_supported_states);
  if (!number_of_supported_states) {
    notification_get_frame->mevent = 0x00;
  } else {
    notification_get_frame->mevent
      = supported_notification_states[notification_state];
  }
  *frame_len = sizeof(ZW_NOTIFICATION_GET_V4_FRAME);

  // Just in case the supporting node does not answer Notification
  // Get commands, we set the reported value of all events for the type to 0
  // to prevent more resolutions.
  const int32_t notification_event = 0;
  attribute_store_set_all_children_reported(type_node,
                                            ATTRIBUTE(EVENT),
                                            &notification_event,
                                            sizeof(notification_event));

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_supported_event_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_len)
{
  ZW_EVENT_SUPPORTED_GET_V4_FRAME *supported_event_get_frame
    = (ZW_EVENT_SUPPORTED_GET_V4_FRAME *)frame;
  supported_event_get_frame->cmdClass = COMMAND_CLASS_NOTIFICATION_V8;
  supported_event_get_frame->cmd      = EVENT_SUPPORTED_GET_V8;
  // read the Notification type
  uint8_t notification_type;
  attribute_store_read_value(attribute_store_get_node_parent(node),
                             REPORTED_ATTRIBUTE,
                             &notification_type,
                             sizeof(notification_type));
  supported_event_get_frame->notificationType = notification_type;
  *frame_len = sizeof(ZW_EVENT_SUPPORTED_GET_V4_FRAME);
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_supported_notification_types_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_len)
{
  ZW_NOTIFICATION_SUPPORTED_GET_V4_FRAME *supported_notification_get_frame
    = (ZW_NOTIFICATION_SUPPORTED_GET_V4_FRAME *)frame;
  supported_notification_get_frame->cmdClass = COMMAND_CLASS_NOTIFICATION_V8;
  supported_notification_get_frame->cmd      = NOTIFICATION_SUPPORTED_GET_V8;
  *frame_len = sizeof(ZW_NOTIFICATION_SUPPORTED_GET_V4_FRAME);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////

sl_status_t zwave_command_class_notification_init()
{
  attribute_resolver_register_rule(
    ATTRIBUTE(SUPPORTED_NOTIFICATION_TYPES),
    nullptr,
    zwave_command_class_supported_notification_types_get);
  attribute_resolver_register_rule(ATTRIBUTE(EVENT),
                                   nullptr,
                                   zwave_command_class_notification_get);

  // Supported event reported handler is not implemented so we do not use the rule for now,
  // register the rule when the report handler is implemented.
  attribute_resolver_register_rule(ATTRIBUTE(SUPPORTED_STATES_OR_EVENTS),
                                   nullptr,
                                   zwave_command_class_supported_event_get);

  // Here the module is listening for the creation of
  // ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_VERSION attribute.
  attribute_store_register_callback_by_type(
    zwave_command_class_notification_on_version_attribute_update,
    ATTRIBUTE(VERSION));

  // Tell AGI that we want to receive Notification Reports
  // via assocation groups
  zwave_command_class_agi_request_to_establish_association(
    COMMAND_CLASS_NOTIFICATION_V8,
    NOTIFICATION_REPORT_V8);

  // Register Notification CC handler to the Z-Wave CC framework
  zwave_command_handler_t handler = {};
  handler.support_handler         = nullptr;
  handler.control_handler = &zwave_command_class_notification_control_handler;
  handler.minimal_scheme  = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.command_class   = COMMAND_CLASS_NOTIFICATION_V8;
  handler.version         = NOTIFICATION_VERSION_V8;
  handler.manual_security_validation = false;
  handler.command_class_name         = "Notification";
  handler.comments                   = "Partial Control: <br>"
                                       "1. No Push/Pull discovery is done.<br>"
                                       "2. No Pull sensor support. <br>"
                                       "3. Unknown types are not supported. <br>"
                                       "4. No Regular probing is done. ";

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}