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
#include "zwave_command_class_door_lock_control.h"
#include "zwave_command_classes_utils.h"
#include "zwave_COMMAND_CLASS_DOOR_LOCK_attribute_id.h"
#include "zwave_COMMAND_CLASS_DOOR_LOCK_handlers.h"

// Generic includes
#include <stdlib.h>
#include <assert.h>

// Includes from other ZPC Components
#include "zwave_command_class_indices.h"
#include "zwave_command_handler.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zpc_attribute_resolver.h"

// Includes from other Unify Components
#include "dotdot_mqtt.h"
#include "dotdot_mqtt_generated_commands.h"
#include "attribute_store_helper.h"
#include "attribute_resolver.h"
#include "attribute_timeouts.h"
#include "sl_log.h"
// Log tag
constexpr const char *LOG_TAG = "zwave_command_class_door_lock_control";
constexpr int OPERATION_REPORT_V1_MINIMUM_LENGTH     = 7;
constexpr int OPERATION_REPORT_V3_MINIMUM_LENGTH     = 9;
constexpr int CONFIGURATION_REPORT_V1_MINIMUM_LENGTH = 6;
constexpr int CONFIGURATION_REPORT_V4_MINIMUM_LENGTH = 11;
constexpr int CAPABILITIES_REPORT_V4_MINIMUM_LENGTH  = 7;

// Version 1-2 attributes under the state
static const attribute_store_type_t v1_2_sub_attributes[]
  = {ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_LOCK_MODE,
     ATTRIBUTE_DOOR_LOCK_OPERATION_INSIDE_DOOR_HANDLES_MODE,
     ATTRIBUTE_DOOR_LOCK_OPERATION_OUTSIDE_DOOR_HANDLES_MODE,
     ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_CONDITION,
     ATTRIBUTE_DOOR_LOCK_OPERATION_LOCK_TIMEOUT_MINUTES,
     ATTRIBUTE_DOOR_LOCK_OPERATION_LOCK_TIMEOUT_SECONDS,
     ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OPERATION_TYPE,
     ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE,
     ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE,
     ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_MINUTES,
     ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_SECONDS};

// Version 3 attributes under the state
static const attribute_store_type_t v3_sub_attributes[]
  = {ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_TARGET_MODE,
     ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_DURATION};

// Version 4 attributes under the state
static const attribute_store_type_t v4_conf_sub_attributes[]
  = {ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK,
     ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_HOLD_RELEASE_TIME,
     ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_BTB,
     ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_TA};

// Version 4 attributes under the state
static const attribute_store_type_t v4_capab_sub_attributes[] = {
  ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_OPERATION_TYPE_BITMASK,
  ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_LOCK_MODE,
  ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_OUTSIDE_HANDLE_MODES_BITMASK,
  ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_INSIDE_HANDLE_MODES_BITMASK,
  ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_DOOR_COMPONENT_BITMASK,
  ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_ARS,
  ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_HRS,
  ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_TAS,
  ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_BTBS};

sl_status_t zwave_command_class_door_lock_operation_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  ZW_DOOR_LOCK_OPERATION_SET_V4_FRAME *set_frame
    = (ZW_DOOR_LOCK_OPERATION_SET_V4_FRAME *)frame;

  set_frame->cmdClass = COMMAND_CLASS_DOOR_LOCK_V4;
  set_frame->cmd      = DOOR_LOCK_OPERATION_SET_V4;

  attribute_store_node_t parent_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  attribute_store_node_t type_node = attribute_store_get_first_child_by_type(
    parent_node,
    ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_LOCK_MODE);

  uint32_t doorLockMode = 0;
  attribute_store_read_value(type_node,
                             DESIRED_OR_REPORTED_ATTRIBUTE,
                             &doorLockMode,
                             sizeof(int32_t));

  set_frame->doorLockMode = doorLockMode;
  *frame_length           = sizeof(ZW_DOOR_LOCK_OPERATION_SET_V4_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_door_lock_operation_get(
  [[maybe_unused]] attribute_store_node_t node,
  uint8_t *frame,
  uint16_t *frame_length)
{
  auto *get_frame          = (zwave_minimum_frame_t *)frame;
  get_frame->command_class = COMMAND_CLASS_DOOR_LOCK_V4;
  get_frame->command       = DOOR_LOCK_OPERATION_GET_V4;
  *frame_length            = sizeof(zwave_minimum_frame_t);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_door_lock_handle_operation_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < OPERATION_REPORT_V1_MINIMUM_LENGTH) {
    return SL_STATUS_FAIL;
  }

  // Find the Endpoint
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  int32_t mode = frame_data[2];
  //Set Current Door Lock Mode
  attribute_store_node_t state_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_LOCK_MODE);
  attribute_store_set_reported(state_node, &mode, sizeof(mode));

  //Set Outside Handle Modes && Set Inside Handle Modes
  int32_t outside_handles = frame_data[3] >> 4;
  state_node              = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_OPERATION_OUTSIDE_DOOR_HANDLES_MODE);
  attribute_store_set_reported(state_node,
                               &outside_handles,
                               sizeof(outside_handles));

  int32_t inside_handles = frame_data[3] & 0b1111;
  state_node             = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_OPERATION_INSIDE_DOOR_HANDLES_MODE);
  attribute_store_set_reported(state_node,
                               &inside_handles,
                               sizeof(inside_handles));

  int32_t condition = frame_data[4];
  //Door Condition
  state_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_CONDITION);
  attribute_store_set_reported(state_node, &condition, sizeof(condition));

  int32_t minutes = frame_data[5];
  //Remaining Lock Time Minutes
  state_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_OPERATION_LOCK_TIMEOUT_MINUTES);
  attribute_store_set_reported(state_node, &minutes, sizeof(minutes));

  int32_t seconds = frame_data[6];
  //Remaining Lock Time Seconds
  state_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_OPERATION_LOCK_TIMEOUT_SECONDS);
  attribute_store_set_reported(state_node, &seconds, sizeof(seconds));

  if (frame_length < OPERATION_REPORT_V3_MINIMUM_LENGTH) {
    return SL_STATUS_OK;
  }

  attribute_store_node_t operation_node
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT);

  //Target Door Lock Mode
  state_node = attribute_store_get_first_child_by_type(
    operation_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_TARGET_MODE);
  attribute_store_set_reported(state_node, &frame_data[7], sizeof(uint8_t));

  //Duration
  state_node = attribute_store_get_first_child_by_type(
    operation_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_DURATION);
  attribute_store_set_reported(state_node, &frame_data[8], sizeof(uint8_t));
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_door_lock_configuration_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  ZW_DOOR_LOCK_CONFIGURATION_SET_V4_FRAME *set_frame
    = (ZW_DOOR_LOCK_CONFIGURATION_SET_V4_FRAME *)frame;

  set_frame->cmdClass = COMMAND_CLASS_DOOR_LOCK_V4;
  set_frame->cmd      = DOOR_LOCK_CONFIGURATION_SET_V4;

  attribute_store_node_t parent_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  int32_t operationType            = 0;
  attribute_store_node_t type_node = attribute_store_get_first_child_by_type(
    parent_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OPERATION_TYPE);
  attribute_store_read_value(type_node,
                             DESIRED_OR_REPORTED_ATTRIBUTE,
                             &operationType,
                             sizeof(operationType));
  set_frame->operationType = (uint8_t)operationType;

  int32_t outside_handles = 0;
  type_node               = attribute_store_get_first_child_by_type(
    parent_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE);
  attribute_store_read_value(type_node,
                             DESIRED_OR_REPORTED_ATTRIBUTE,
                             &outside_handles,
                             sizeof(outside_handles));

  int32_t inside_handles = 0;
  type_node              = attribute_store_get_first_child_by_type(
    parent_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE);
  attribute_store_read_value(type_node,
                             DESIRED_OR_REPORTED_ATTRIBUTE,
                             &inside_handles,
                             sizeof(inside_handles));

  set_frame->properties1 = (uint8_t)((outside_handles << 4) | inside_handles);

  int32_t lockTimeoutMinutes = 0;
  type_node                  = attribute_store_get_first_child_by_type(
    parent_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_MINUTES);
  attribute_store_read_value(type_node,
                             DESIRED_OR_REPORTED_ATTRIBUTE,
                             &lockTimeoutMinutes,
                             sizeof(lockTimeoutMinutes));
  set_frame->lockTimeoutMinutes = (uint8_t)lockTimeoutMinutes;

  int32_t lockTimeoutSeconds = 0;
  type_node                  = attribute_store_get_first_child_by_type(
    parent_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_SECONDS);
  attribute_store_read_value(type_node,
                             DESIRED_OR_REPORTED_ATTRIBUTE,
                             &lockTimeoutSeconds,
                             sizeof(lockTimeoutSeconds));
  set_frame->lockTimeoutSeconds = (uint8_t)lockTimeoutSeconds;

  attribute_store_node_t config_node = attribute_store_get_first_child_by_type(
    parent_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION);

  uint16_t auto_relock = 0;
  type_node            = attribute_store_get_first_child_by_type(
    config_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK);
  attribute_store_read_value(type_node,
                             DESIRED_OR_REPORTED_ATTRIBUTE,
                             &auto_relock,
                             sizeof(auto_relock));

  set_frame->autoRelockTime1 = (uint8_t)(auto_relock >> 8);
  set_frame->autoRelockTime2 = (uint8_t)(auto_relock & 0xFF);

  uint16_t hold_release = 0;
  type_node             = attribute_store_get_first_child_by_type(
    config_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_HOLD_RELEASE_TIME);
  attribute_store_read_value(type_node,
                             DESIRED_OR_REPORTED_ATTRIBUTE,
                             &hold_release,
                             sizeof(hold_release));

  set_frame->holdAndReleaseTime1 = (uint8_t)(hold_release >> 8);
  set_frame->holdAndReleaseTime2 = (uint8_t)(hold_release & 0xFF);

  uint8_t btb = 0;
  type_node   = attribute_store_get_first_child_by_type(
    config_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_BTB);
  attribute_store_read_value(type_node,
                             DESIRED_OR_REPORTED_ATTRIBUTE,
                             &btb,
                             sizeof(btb));

  uint8_t ta = 0;
  type_node  = attribute_store_get_first_child_by_type(
    config_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_TA);
  attribute_store_read_value(type_node,
                             DESIRED_OR_REPORTED_ATTRIBUTE,
                             &ta,
                             sizeof(ta));

  set_frame->properties2 = (uint8_t)((btb << 1) | ta);

  *frame_length = sizeof(ZW_DOOR_LOCK_CONFIGURATION_SET_V4_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_door_lock_configuration_get(
  [[maybe_unused]] attribute_store_node_t node,
  uint8_t *frame,
  uint16_t *frame_length)
{
  auto *get_frame          = (zwave_minimum_frame_t *)frame;
  get_frame->command_class = COMMAND_CLASS_DOOR_LOCK_V4;
  get_frame->command       = DOOR_LOCK_CONFIGURATION_GET_V4;
  *frame_length            = sizeof(zwave_minimum_frame_t);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_door_lock_handle_configuration_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < CONFIGURATION_REPORT_V1_MINIMUM_LENGTH) {
    return SL_STATUS_FAIL;
  }

  // Find the Endpoint
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  //Set Current Door Lock Mode
  int32_t oper_type = frame_data[2];
  attribute_store_node_t state_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OPERATION_TYPE);
  attribute_store_set_reported(state_node, &oper_type, sizeof(oper_type));

  //Set Outside Handle Modes && Set Inside Handle Modes
  int32_t outside_handles = frame_data[3] >> 4;
  state_node              = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE);
  attribute_store_set_reported(state_node, &outside_handles, sizeof(outside_handles));

  int32_t inside_handles = frame_data[3] & 0b1111;
  state_node             = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE);
  attribute_store_set_reported(state_node, &inside_handles, sizeof(inside_handles));

  //Lock Time Minutes
  int32_t minutes = frame_data[4];
  state_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_MINUTES);
  attribute_store_set_reported(state_node, &minutes, sizeof(minutes));

  //Lock Time Seconds
  int32_t seconds = frame_data[5];
  state_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_SECONDS);
  attribute_store_set_reported(state_node, &seconds, sizeof(seconds));

  if (frame_length < CONFIGURATION_REPORT_V4_MINIMUM_LENGTH) {
    return SL_STATUS_OK;
  }

  attribute_store_node_t configuration_node
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION);

  //Auto-relock time (MSB + LSB)
  uint16_t auto_relock = frame_data[6] << 8 | frame_data[7];
  state_node           = attribute_store_get_first_child_by_type(
    configuration_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK);
  attribute_store_set_reported(state_node, &auto_relock, sizeof(auto_relock));

  //Hold and release time (MSB + LSB)
  uint16_t hold_release = frame_data[8] << 8 | frame_data[9];
  state_node            = attribute_store_get_first_child_by_type(
    configuration_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_HOLD_RELEASE_TIME);
  attribute_store_set_reported(state_node, &hold_release, sizeof(hold_release));

  //Set BTB
  uint8_t btb = frame_data[10] >> 1;
  state_node  = attribute_store_get_first_child_by_type(
    configuration_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_BTB);
  attribute_store_set_reported(state_node, &btb, sizeof(btb));

  //Set TA
  uint8_t ta = frame_data[10] & 0b1;
  state_node = attribute_store_get_first_child_by_type(
    configuration_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_TA);
  attribute_store_set_reported(state_node, &ta, sizeof(ta));

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_door_lock_capabilities_get(
  [[maybe_unused]] attribute_store_node_t node,
  uint8_t *frame,
  uint16_t *frame_length)
{
  auto *get_frame          = (zwave_minimum_frame_t *)frame;
  get_frame->command_class = COMMAND_CLASS_DOOR_LOCK_V4;
  get_frame->command       = DOOR_LOCK_CAPABILITIES_GET_V4;
  *frame_length            = sizeof(zwave_minimum_frame_t);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_door_lock_handle_capabilities_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // We expect to have at least 1 byte of value
  if (frame_length < CAPABILITIES_REPORT_V4_MINIMUM_LENGTH) {
    return SL_STATUS_FAIL;
  }

  // Find the Endpoint
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  //Set Operation Type Bit Mask
  attribute_store_node_t capabilities_node
    = attribute_store_create_child_if_missing(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES);
  attribute_store_node_t state_node = attribute_store_get_first_child_by_type(
    capabilities_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_OPERATION_TYPE_BITMASK);
  uint8_t type_length = frame_data[2];

  if (type_length == 0) {
    const uint8_t bitmask = 0;
    attribute_store_set_reported(state_node, &bitmask, sizeof(bitmask));
  } else if (type_length > 0
             && (frame_length
                 >= CAPABILITIES_REPORT_V4_MINIMUM_LENGTH + type_length)) {
    attribute_store_set_reported(state_node, &frame_data[3], type_length);
  } else
    return SL_STATUS_FAIL;

  //Set Door Lock Mode
  state_node = attribute_store_get_first_child_by_type(
    capabilities_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_LOCK_MODE);
  uint8_t mode_length = frame_data[3 + type_length];

  if (mode_length == 0) {
    const uint8_t mode = 0;
    attribute_store_set_reported(state_node, &mode, sizeof(mode));
  } else if (mode_length > 0
             && (frame_length >= CAPABILITIES_REPORT_V4_MINIMUM_LENGTH
                                   + type_length + mode_length)) {
    attribute_store_set_reported(state_node,
                                 &frame_data[4 + type_length],
                                 mode_length);
  } else
    return SL_STATUS_FAIL;

  uint8_t handle_modes = frame_data[4 + type_length + mode_length];

  //Set Outside Handle Modes
  uint8_t outside_handle_mode = handle_modes >> 4;
  state_node                  = attribute_store_get_first_child_by_type(
    capabilities_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_OUTSIDE_HANDLE_MODES_BITMASK);
  attribute_store_set_reported(state_node,
                               &outside_handle_mode,
                               sizeof(uint8_t));

  //Set Inside Handle Modes
  uint8_t inside_handle_mode = handle_modes & 0b1111;
  state_node                 = attribute_store_get_first_child_by_type(
    capabilities_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_INSIDE_HANDLE_MODES_BITMASK);
  attribute_store_set_reported(state_node,
                               &inside_handle_mode,
                               sizeof(uint8_t));

  //Set Door Components
  state_node = attribute_store_get_first_child_by_type(
    capabilities_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_DOOR_COMPONENT_BITMASK);
  attribute_store_set_reported(state_node,
                               &frame_data[5 + type_length + mode_length],
                               sizeof(uint8_t));

  uint8_t functionality = frame_data[6 + type_length + mode_length];

  //Set ARS
  uint8_t ars = (functionality & 0b1000) >> 3;
  state_node  = attribute_store_get_first_child_by_type(
    capabilities_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_ARS);
  attribute_store_set_reported(state_node, &ars, sizeof(uint8_t));

  //Set HRS
  uint8_t hrs = (functionality & 0b100) >> 2;
  state_node  = attribute_store_get_first_child_by_type(
    capabilities_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_HRS);
  attribute_store_set_reported(state_node, &hrs, sizeof(uint8_t));

  //Set TAS
  uint8_t tas = (functionality & 0b10) >> 1;
  state_node  = attribute_store_get_first_child_by_type(
    capabilities_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_TAS);
  attribute_store_set_reported(state_node, &tas, sizeof(uint8_t));

  //Set BTBS
  uint8_t btbs = functionality & 0b1;
  state_node   = attribute_store_get_first_child_by_type(
    capabilities_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_BTBS);
  attribute_store_set_reported(state_node, &btbs, sizeof(uint8_t));

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Store callback functions
///////////////////////////////////////////////////////////////////////////////
void zwave_command_class_door_lock_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(
        COMMAND_CLASS_DOOR_LOCK_V4,
        updated_node)
      == true) {
    return;
  }

  // Confirm that we have a version attribute update
  assert(ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_VERSION
         == attribute_store_get_node_type(updated_node));

  // Do not create the attributes until we are sure of the version
  uint8_t supporting_node_version = 0;
  attribute_store_get_reported(updated_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Wait that the version becomes non-zero.
    return;
  }

  // Now we know we have a DoorLock supporting endpoint.
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  if (supporting_node_version >= 1) {
    attribute_store_add_if_missing(endpoint_node,
                                   v1_2_sub_attributes,
                                   COUNT_OF(v1_2_sub_attributes));

    if (supporting_node_version >= 3) {
      attribute_store_node_t state_node
        = attribute_store_get_first_child_by_type(
          endpoint_node,
          ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT);
      if (state_node == ATTRIBUTE_STORE_INVALID_NODE) {
        state_node = attribute_store_add_node(
          ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT,
          endpoint_node);
      }

      attribute_store_add_if_missing(state_node,
                                     v3_sub_attributes,
                                     COUNT_OF(v3_sub_attributes));
    }

    if (supporting_node_version >= 4) {
      attribute_store_node_t state_node
        = attribute_store_get_first_child_by_type(
          endpoint_node,
          ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION);
      if (state_node == ATTRIBUTE_STORE_INVALID_NODE) {
        state_node = attribute_store_add_node(
          ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION,
          endpoint_node);
      }

      attribute_store_add_if_missing(state_node,
                                     v4_conf_sub_attributes,
                                     COUNT_OF(v4_conf_sub_attributes));

      state_node = attribute_store_get_first_child_by_type(
        endpoint_node,
        ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES);
      if (state_node == ATTRIBUTE_STORE_INVALID_NODE) {
        state_node = attribute_store_add_node(
          ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES,
          endpoint_node);
      }

      attribute_store_add_if_missing(state_node,
                                     v4_capab_sub_attributes,
                                     COUNT_OF(v4_capab_sub_attributes));
    }
  }
}

sl_status_t zwave_command_class_door_lock_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  assert(frame_data[COMMAND_CLASS_INDEX] == COMMAND_CLASS_DOOR_LOCK_V4);

  switch (frame_data[COMMAND_INDEX]) {
    case DOOR_LOCK_OPERATION_REPORT_V4:
      return zwave_command_class_door_lock_handle_operation_report(
        connection_info,
        frame_data,
        frame_length);
    case DOOR_LOCK_CONFIGURATION_REPORT_V4:
      return zwave_command_class_door_lock_handle_configuration_report(
        connection_info,
        frame_data,
        frame_length);
    case DOOR_LOCK_CAPABILITIES_REPORT_V4:
      return zwave_command_class_door_lock_handle_capabilities_report(
        connection_info,
        frame_data,
        frame_length);
    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_door_lock_init()
{
  // // Attribute store callbacks
  attribute_store_register_callback_by_type(
    zwave_command_class_door_lock_on_version_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_VERSION);

  //Opearation
  attribute_resolver_register_rule(ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_LOCK_MODE,
                                   zwave_command_class_door_lock_operation_set,
                                   zwave_command_class_door_lock_operation_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_DOOR_LOCK_OPERATION_INSIDE_DOOR_HANDLES_MODE,
    nullptr,
    zwave_command_class_door_lock_operation_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_DOOR_LOCK_OPERATION_OUTSIDE_DOOR_HANDLES_MODE,
    nullptr,
    zwave_command_class_door_lock_operation_get);
  attribute_resolver_register_rule(ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_CONDITION,
                                   nullptr,
                                   zwave_command_class_door_lock_operation_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_DOOR_LOCK_OPERATION_LOCK_TIMEOUT_MINUTES,
    nullptr,
    zwave_command_class_door_lock_operation_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_DOOR_LOCK_OPERATION_LOCK_TIMEOUT_SECONDS,
    nullptr,
    zwave_command_class_door_lock_operation_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_TARGET_MODE,
    nullptr,
    zwave_command_class_door_lock_operation_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_DURATION,
    nullptr,
    zwave_command_class_door_lock_operation_get);

  //Configuration
  attribute_resolver_register_rule(
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OPERATION_TYPE,
    zwave_command_class_door_lock_configuration_set,
    zwave_command_class_door_lock_configuration_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE,
    zwave_command_class_door_lock_configuration_set,
    zwave_command_class_door_lock_configuration_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE,
    zwave_command_class_door_lock_configuration_set,
    zwave_command_class_door_lock_configuration_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_MINUTES,
    zwave_command_class_door_lock_configuration_set,
    zwave_command_class_door_lock_configuration_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_SECONDS,
    zwave_command_class_door_lock_configuration_set,
    zwave_command_class_door_lock_configuration_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK,
    zwave_command_class_door_lock_configuration_set,
    zwave_command_class_door_lock_configuration_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_HOLD_RELEASE_TIME,
    zwave_command_class_door_lock_configuration_set,
    zwave_command_class_door_lock_configuration_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_BTB,
    zwave_command_class_door_lock_configuration_set,
    zwave_command_class_door_lock_configuration_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_TA,
    zwave_command_class_door_lock_configuration_set,
    zwave_command_class_door_lock_configuration_get);

  //Capabilities
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_OPERATION_TYPE_BITMASK,
    nullptr,
    zwave_command_class_door_lock_capabilities_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_LOCK_MODE,
    nullptr,
    zwave_command_class_door_lock_capabilities_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_OUTSIDE_HANDLE_MODES_BITMASK,
    nullptr,
    zwave_command_class_door_lock_capabilities_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_INSIDE_HANDLE_MODES_BITMASK,
    nullptr,
    zwave_command_class_door_lock_capabilities_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_DOOR_COMPONENT_BITMASK,
    nullptr,
    zwave_command_class_door_lock_capabilities_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_ARS,
    nullptr,
    zwave_command_class_door_lock_capabilities_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_HRS,
    nullptr,
    zwave_command_class_door_lock_capabilities_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_TAS,
    nullptr,
    zwave_command_class_door_lock_capabilities_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_BTBS,
    nullptr,
    zwave_command_class_door_lock_capabilities_get);

  // The support side of things: Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {nullptr};
  handler.support_handler         = nullptr;
  handler.control_handler    = &zwave_command_class_door_lock_control_handler;
  handler.command_class_name = "Door Lock";
  handler.comments           = "";
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  handler.command_class      = COMMAND_CLASS_DOOR_LOCK;
  handler.version            = DOOR_LOCK_VERSION_V4;
  handler.manual_security_validation = false;

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}
