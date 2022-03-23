/******************************************************************************
* # License
* <b>Copyright 2021  Silicon Laboratories Inc. www.silabs.com</b>
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
#include "zwave_command_class_central_scene.h"
#include "zwave_command_classes_utils.h"
#include "zwave_command_class_agi.h"

// Generic includes
#include <stdbool.h>

// Interfaces
#include "zwave_command_class_central_scene_types.h"
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"

// Includes from other ZPC Components
#include "zwave_controller_command_class_indices.h"
#include "zwave_command_handler.h"
#include "zwave_controller_connection_info.h"
#include "zwave_controller_utils.h"
#include "zpc_attribute_store_network_helper.h"
#include "zwave_unid.h"

// Includes from UIC Components
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_resolver.h"
#include "attribute_timeouts.h"
#include "dotdot_mqtt.h"
#include "dotdot_mqtt_supported_generated_commands.h"

#include "sl_log.h"
#define LOG_TAG "zwave_command_class_central_scene"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_##type

// Central Scene Attributes. For all versions.
static const attribute_store_type_t attribute_list[]
  = {ATTRIBUTE(NUMBER_OF_SCENES),
     ATTRIBUTE(MAX_KEY_ATTRIBUTE),
     ATTRIBUTE(SLOW_REFRESH),
     ATTRIBUTE(ACTIVE_SCENE)};

// v1 nodes support only 3 key attributes max (pressed 1 time, held down, released)
#define V1_KEY_ATTRIBUTES 3
// v2/v3 nodes support only 7 key attributes max
#define V2_KEY_ATTRIBUTES 7
#define V3_KEY_ATTRIBUTES 7

// Consider slow refresh to be inactive by default.
#define SLOW_REFRESH_DEFAULT_VALUE false

// Value reprensenting no active scene.
#define NO_SCENE_ACTIVE 0

// Scene timeout values.
#define DEFAULT_SCENE_HELD_DOWN_TIMEOUT_VALUE (5 * CLOCK_SECOND)
#define SLOW_REFRESH_SCENE_TIMEOUT_VALUE      (60 * CLOCK_SECOND)

//Frame parsing constants
#define SUPPORTED_REPORT_SUPPORTED_SCENES_INDEX     2
#define SUPPORTED_REPORT_SLOW_REFRESH_SUPPORT_INDEX 3
#define SUPPORTED_REPORT_BITMASKS_INDEX             4
#define SUPPORTED_REPORT_SLOW_REFRESH_MASK          1 << 7
#define SUPPORTED_REPORT_NUMBER_OF_BITMASK_MASK     3 << 1
#define SUPPORTED_REPORT_IDENTICAL_MASK             1

#define NOTIFICATION_SEQ_NUMBER_INDEX    2
#define NOTIFICATION_KEY_ATTRIBUTE_INDEX 3
#define NOTIFICATION_SCENE_NUMBER_INDEX  4
#define NOTIFICATION_SLOW_REFRESH_MASK   (1 << 7)
#define NOTIFICATION_KEY_ATTRIBUTE_MASK  0x7

#define CONFIGURATION_REPORT_SLOW_REFRESH_SUPPORT_INDEX 2
#define CONFIGURATION_REPORT_SLOW_REFRESH_MASK          (1 << 7)

/**
 * @brief Full state of a Central Scene supporting node.
 */
typedef struct central_scene_state {
  central_scene_scene_t scene;
  central_scene_sequence_number_t scene_seq_number;
} central_scene_state_t;

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Finds the highest bit set in a byte
 * Bits will be denote as 8..1 with 8 the MSB and 1 the LSB.
 *
 * @param byte the byte to inspect
 * @returns Number of the highest bit set.
 */
static uint8_t get_byte_highest_bit(uint8_t byte)
{
  for (uint8_t bit = 8; bit > 0; bit--) {
    if (byte >> (bit - 1)) {
      return bit;
    }
  }
  return 0;
}

/**
 * @brief Computes what default max key attribute a node can support based on its
 *        version.
 *
 * @param endpoint_node Endpoint ID node for the endpoint supporting Central
 *                      Scene
 * @returns central_scene_key_attribute_t value of the maximum possible key
 *          attribute.
 */
static central_scene_key_attribute_t
  get_default_max_key_attribute(attribute_store_node_t endpoint_node)
{
  // Read the version of the supporting node
  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_node_t version_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(VERSION),
                                             0);
  attribute_store_get_reported(version_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  // Device how many key attributes they have by default.
  central_scene_key_attribute_t max_supported_key_attribute = 0;
  if (supporting_node_version == 1) {
    max_supported_key_attribute = V1_KEY_ATTRIBUTES;
  } else if (supporting_node_version > 1) {
    max_supported_key_attribute = V2_KEY_ATTRIBUTES;
  }

  return max_supported_key_attribute;
}

/**
 * @brief Reads from the attribute store the maximum Key attribute used by a node
 *
 * @param endpoint_node Endpoint ID node for the endpoint supporting Central
 *                      Scene
 */
static central_scene_key_attribute_t
  get_supported_attributes(attribute_store_node_t endpoint_node)
{
  central_scene_key_attribute_t value = V3_KEY_ATTRIBUTES;
  attribute_store_node_t supported_attributes_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(MAX_KEY_ATTRIBUTE),
                                             0);

  attribute_store_get_reported(supported_attributes_node,
                               &value,
                               sizeof(value));

  return value;
}

/**
 * @brief Calcuates from a received frame maximum Key attribute used by a node
 *
 * @param frame                     Pointer pointing at the frame bitmask
 *                                  masks of the Supported Report
 * @param scene_bitmasks            How many scenes bitmasks are included
 *                                  in the frame
 * @param number_of_bitmasks_bytes  How many bitmask byte are included per
 *                                  scene in the frame.
 * @returns Calculated maximum possible key attribute.
 */
static central_scene_key_attribute_t
  calculate_max_supported_attributes(const uint8_t *frame,
                                     central_scene_scene_t scene_bitmasks,
                                     uint8_t number_of_bitmasks_bytes)
{
  central_scene_key_attribute_t max_attribute = 0;

  for (central_scene_scene_t scene = 0; scene < scene_bitmasks; scene++) {
    // Go directly at the highest byte, we want the highest possible value
    uint8_t byte = frame[(scene * number_of_bitmasks_bytes)
                         + number_of_bitmasks_bytes - 1];

    // Find the highest set bit in the byte
    uint8_t bit = get_byte_highest_bit(byte);

    central_scene_key_attribute_t scene_maximum
      = bit + 8 * (number_of_bitmasks_bytes - 1);

    if (scene_maximum > max_attribute) {
      max_attribute = scene_maximum;
    }
  }

  return max_attribute;
}

/**
 * @brief Reads the state (last active scene) of a supporting node.
 *
 * @param endpoint_node Endpoint ID node for the endpoint supporting Central
 *                      Scene
 * @param [out] state   Pointer to a state struct that will be filled with the
 *                      values retrieved from the attribute store.
 */
static void get_state(attribute_store_node_t endpoint_node,
                      central_scene_state_t *state)
{
  attribute_store_node_t active_scene_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(ACTIVE_SCENE),
                                             0);

  attribute_store_get_reported(active_scene_node,
                               &(state->scene),
                               sizeof(state->scene));

  attribute_store_node_t sequence_number_node
    = attribute_store_get_node_child_by_type(
      active_scene_node,
      ATTRIBUTE(ACTIVE_SCENE_SEQUENCE_NUMBER),
      0);

  attribute_store_get_reported(sequence_number_node,
                               &(state->scene_seq_number),
                               sizeof(state->scene_seq_number));
}

/**
 * @brief Save the state (last active scene) of a supporting node in the
 *        attribute store.
 *
 * @param endpoint_node Endpoint ID node for the endpoint supporting Central
 *                      Scene
 * @param state         Pointer to a state struct that will be used to save
 *                      values in the attribute store.
 */
static void set_state(attribute_store_node_t endpoint_node,
                      const central_scene_state_t *state)
{
  attribute_store_node_t active_scene_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(ACTIVE_SCENE),
                                             0);

  attribute_store_set_reported(active_scene_node,
                               &(state->scene),
                               sizeof(state->scene));

  attribute_store_set_child_reported(active_scene_node,
                                     ATTRIBUTE(ACTIVE_SCENE_SEQUENCE_NUMBER),
                                     &(state->scene_seq_number),
                                     sizeof(state->scene_seq_number));
}

/**
 * @brief Maps the received scene notification / key attribute to a unique
 *        active scene.
 *
 * @param endpoint_node           Endpoint ID node for the endpoint supporting
 *                                Central Scene
 * @param received_scene          Scene value received in the Central Scene
 *                                Notification
 * @param received_key_attribute  Key attribute value received in the
 *                                Central Scene Notification
 *
 * @returns central_scene_scene_t Unique active scene number.
 */
static central_scene_scene_t
  get_active_scene(attribute_store_node_t endpoint_node,
                   central_scene_scene_t received_scene,
                   central_scene_key_attribute_t received_key_attribute)
{
  central_scene_key_attribute_t max_attributes
    = get_supported_attributes(endpoint_node);

  // Scene has to be >0. Else we consider the Scene inactive.
  if (received_scene < 1) {
    return NO_SCENE_ACTIVE;
  }
  // Validate the Key attributes value.
  if (received_key_attribute > max_attributes) {
    sl_log_debug(
      LOG_TAG,
      "Received an invalid key attribute %d for endpoint node %d. Only %d key "
      "attributes are supposed to be supported. Ignoring.",
      received_key_attribute,
      endpoint_node,
      max_attributes);
    return NO_SCENE_ACTIVE;
  }

  // Map key attribute / scene number into a unique scene number.
  central_scene_scene_t active_scene
    = ((received_scene - 1) * max_attributes) + (received_key_attribute + 1);

  return active_scene;
}

/**
 * @brief Sets back the current scene to IDLE.
 *
 * @param endpoint_node           Endpoint ID node for the endpoint supporting
 *                                Central Scene
 */
static void return_scene_to_idle(attribute_store_node_t endpoint_node)
{
  const central_scene_state_t idle_state
    = {.scene = NO_SCENE_ACTIVE, .scene_seq_number = 0};
  set_state(endpoint_node, &idle_state);
}

///////////////////////////////////////////////////////////////////////////////
// Command Handler functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Handle incoming Central Scene Notification Commands
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t
  zwave_command_class_central_scene_handle_notification_command(
    const zwave_controller_connection_info_t *info,
    const uint8_t *frame,
    uint16_t frame_length)
{
  if (frame_length <= NOTIFICATION_SCENE_NUMBER_INDEX) {
    return SL_STATUS_OK;
  }

  // Find the Endpoint
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(info);

  // Get the data from the frame
  central_scene_sequence_number_t received_seq_number
    = frame[NOTIFICATION_SEQ_NUMBER_INDEX];
  central_scene_slow_refresh_t received_slow_refresh
    = frame[NOTIFICATION_KEY_ATTRIBUTE_INDEX] & NOTIFICATION_SLOW_REFRESH_MASK;
  central_scene_key_attribute_t received_key_attribute
    = frame[NOTIFICATION_KEY_ATTRIBUTE_INDEX] & NOTIFICATION_KEY_ATTRIBUTE_MASK;

  sl_log_debug(LOG_TAG,
               "Received Scene %d, key attribute %d and sequence number "
               "%d from NodeID %d:%d",
               frame[NOTIFICATION_SCENE_NUMBER_INDEX],
               received_key_attribute,
               received_seq_number,
               info->remote.node_id,
               info->remote.endpoint_id);

  // Convert frame data into "an active scene"
  central_scene_scene_t received_active_scene
    = get_active_scene(endpoint_node,
                       frame[NOTIFICATION_SCENE_NUMBER_INDEX],
                       received_key_attribute);

  sl_log_debug(LOG_TAG, "Calculated active scene: %d.", received_active_scene);

  // Get the previous data from the attribute store
  central_scene_state_t current_state = {};
  get_state(endpoint_node, &current_state);

  // Check for duplicates, using the sequence number.
  if (current_state.scene == received_active_scene
      && current_state.scene_seq_number == received_seq_number) {
    return SL_STATUS_OK;
  }

  // Accept the scene and save it.
  current_state.scene            = received_active_scene;
  current_state.scene_seq_number = received_seq_number;
  set_state(endpoint_node, &current_state);

  // Check what timeout to apply to that scene.
  if ((received_key_attribute
       == CENTRAL_SCENE_NOTIFICATION_KEY_ATTRIBUTES_KEY_HELD_DOWN_V3)
      && (received_slow_refresh == true)) {
    attribute_timeout_set_callback(endpoint_node,
                                   SLOW_REFRESH_SCENE_TIMEOUT_VALUE,
                                   &return_scene_to_idle);
  } else if ((received_key_attribute
              == CENTRAL_SCENE_NOTIFICATION_KEY_ATTRIBUTES_KEY_HELD_DOWN_V3)
             && (received_slow_refresh == false)) {
    attribute_timeout_set_callback(endpoint_node,
                                   DEFAULT_SCENE_HELD_DOWN_TIMEOUT_VALUE,
                                   &return_scene_to_idle);
  }

  // Generate a received command to MQTT (Recall Scene):
  unid_t node_unid;
  zwave_unid_from_node_id(info->remote.node_id, node_unid);
  if (received_active_scene > NO_SCENE_ACTIVE) {
    uic_mqtt_dotdot_scenes_command_recall_scene_fields_t fields = {};
    fields.groupid                                              = 0;
    fields.sceneid         = received_active_scene - 1;
    fields.transition_time = 0;
    uic_mqtt_dotdot_scenes_publish_generated_recall_scene_command(
      node_unid,
      (dotdot_endpoint_id_t)info->remote.endpoint_id,
      &fields);
  }

  return SL_STATUS_OK;
}

/**
 * @brief Handle incoming Central Scene Supported Report Commands
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t
  zwave_command_class_central_scene_handle_supported_report_command(
    const zwave_controller_connection_info_t *info,
    const uint8_t *frame,
    uint16_t frame_length)
{
  // Find the Endpoint
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(info);

  // We expect to have at least 1 byte of value.
  // First save the number of supported scenes.
  central_scene_scene_t supported_scenes = 0;
  if (frame_length > SUPPORTED_REPORT_SUPPORTED_SCENES_INDEX) {
    supported_scenes = frame[SUPPORTED_REPORT_SUPPORTED_SCENES_INDEX];
  }

  // If slow refresh is supported, we want to activate it.
  if ((frame_length > SUPPORTED_REPORT_SLOW_REFRESH_SUPPORT_INDEX)
      && ((frame[SUPPORTED_REPORT_SLOW_REFRESH_SUPPORT_INDEX]
           & SUPPORTED_REPORT_SLOW_REFRESH_MASK)
          > 0)) {
    central_scene_slow_refresh_t desired_slow_refresh = true;
    attribute_store_set_child_desired(endpoint_node,
                                      ATTRIBUTE(SLOW_REFRESH),
                                      &desired_slow_refresh,
                                      sizeof(desired_slow_refresh));
  }

  // Investigate the max key attributes that the node supports
  central_scene_key_attribute_t max_attribute
    = get_default_max_key_attribute(endpoint_node);
  if (frame_length > SUPPORTED_REPORT_SLOW_REFRESH_SUPPORT_INDEX) {
    uint8_t number_of_bitmasks_bytes
      = (frame[SUPPORTED_REPORT_SLOW_REFRESH_SUPPORT_INDEX]
         & SUPPORTED_REPORT_NUMBER_OF_BITMASK_MASK)
        >> 1;

    // We get only 1 scene advertised if identical bitmasks
    central_scene_scene_t scene_bitmasks = supported_scenes;
    if (frame[SUPPORTED_REPORT_SLOW_REFRESH_SUPPORT_INDEX]
        & SUPPORTED_REPORT_IDENTICAL_MASK) {
      scene_bitmasks = 1;
    }

    // Check that the length of the frame is long enough to parse those bytes.
    if (frame_length > ((number_of_bitmasks_bytes * scene_bitmasks)
                        + SUPPORTED_REPORT_SLOW_REFRESH_SUPPORT_INDEX)) {
      max_attribute = calculate_max_supported_attributes(
        &frame[SUPPORTED_REPORT_BITMASKS_INDEX],
        scene_bitmasks,
        number_of_bitmasks_bytes);
    } else {
      sl_log_debug(LOG_TAG,
                   "Too short Central Scene Supported Report received. "
                   "Ignoring the supported key attributes.");
    }
  }
  // Save this in the attribute store.
  sl_log_debug(LOG_TAG,
               "NodeID %d:%d will use maximum key attribute %d.",
               info->remote.node_id,
               info->remote.endpoint_id,
               max_attribute);
  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(MAX_KEY_ATTRIBUTE),
                                     &max_attribute,
                                     sizeof(max_attribute));

  // Finally, compute how many combinations we can have
  // 3 scenes with 7 key attributes gives us 21 combinations.
  central_scene_scene_t supported_active_scenes
    = supported_scenes * max_attribute;
  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(NUMBER_OF_SCENES),
                                     &supported_active_scenes,
                                     sizeof(supported_active_scenes));

  return SL_STATUS_OK;
}

/**
 * @brief Handle incoming Central Scene Configuration Report Commands
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t
  zwave_command_class_central_scene_handle_configuration_report_command(
    const zwave_controller_connection_info_t *info,
    const uint8_t *frame,
    uint16_t frame_length)
{
  if (frame_length <= CONFIGURATION_REPORT_SLOW_REFRESH_SUPPORT_INDEX) {
    return SL_STATUS_OK;
  }

  // Find the Endpoint
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(info);

  // Get the frame data
  central_scene_slow_refresh_t received_slow_refresh
    = frame[CONFIGURATION_REPORT_SLOW_REFRESH_SUPPORT_INDEX]
      & CONFIGURATION_REPORT_SLOW_REFRESH_MASK;

  // Save in the attribute store.
  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(SLOW_REFRESH),
                                     &received_slow_refresh,
                                     sizeof(received_slow_refresh));

  return SL_STATUS_OK;
}

/**
 * @brief Dispatches incoming Central Scene Commands to individual
 *        command handlers.
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t zwave_command_class_central_scene_control_handler(
  const zwave_controller_connection_info_t *info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  if (frame_length < COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame[COMMAND_CLASS_INDEX] != COMMAND_CLASS_CENTRAL_SCENE_V3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame[COMMAND_INDEX]) {
    case CENTRAL_SCENE_NOTIFICATION_V3:
      return zwave_command_class_central_scene_handle_notification_command(
        info,
        frame,
        frame_length);

    case CENTRAL_SCENE_SUPPORTED_REPORT_V3:
      return zwave_command_class_central_scene_handle_supported_report_command(
        info,
        frame,
        frame_length);

    case CENTRAL_SCENE_CONFIGURATION_REPORT_V3:
      return zwave_command_class_central_scene_handle_configuration_report_command(
        info,
        frame,
        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute resolution functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Generates a Central Scene Supported Get Command.
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_central_scene_supported_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Create a frame for the attribute resolver. All versions support this frame.
  zwave_minimum_frame_t *get_frame = (zwave_minimum_frame_t *)frame;
  get_frame->command_class         = COMMAND_CLASS_CENTRAL_SCENE_V3;
  get_frame->command               = CENTRAL_SCENE_SUPPORTED_GET_V3;
  *frame_length                    = sizeof(zwave_minimum_frame_t);
  return SL_STATUS_OK;
}

/**
 * @brief Generates a Central Scene Configuration Get Command.
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_central_scene_configuration_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // This frame is supported from V3, verify that the supporting node can handle it.
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_node_t version_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(VERSION),
                                             0);
  attribute_store_get_reported(version_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  // Nodes for version 1 / version 2 will be assumed to have no Slow refresh enabled.
  if (supporting_node_version < 3) {
    central_scene_slow_refresh_t slow_refresh = SLOW_REFRESH_DEFAULT_VALUE;
    attribute_store_set_reported(node, &slow_refresh, sizeof(slow_refresh));
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  // Else just send a get command.
  zwave_minimum_frame_t *get_frame = (zwave_minimum_frame_t *)frame;
  get_frame->command_class         = COMMAND_CLASS_CENTRAL_SCENE_V3;
  get_frame->command               = CENTRAL_SCENE_CONFIGURATION_GET_V3;
  *frame_length                    = sizeof(zwave_minimum_frame_t);
  return SL_STATUS_OK;
}

/**
 * @brief Generates a Central Scene Configuration Set Command.
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_central_scene_configuration_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // This frame is supported from V3, verify that the supporting node can handle it.
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_node_t version_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(VERSION),
                                             0);
  attribute_store_get_reported(version_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  // Nodes for version 1 / version 2 cannot change this value.
  if (supporting_node_version < 3) {
    sl_log_warning(LOG_TAG,
                   "Slow refresh desired value was changed for a supporting "
                   "node with version < 3. This should not happen.");
    attribute_store_undefine_desired(node);
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  // Now we know that the node can take it.
  central_scene_slow_refresh_t slow_refresh = false;
  attribute_store_get_desired(node, &slow_refresh, sizeof(slow_refresh));

  ZW_CENTRAL_SCENE_CONFIGURATION_SET_V3_FRAME *set_frame
    = (ZW_CENTRAL_SCENE_CONFIGURATION_SET_V3_FRAME *)frame;
  set_frame->cmdClass    = COMMAND_CLASS_CENTRAL_SCENE_V3;
  set_frame->cmd         = CENTRAL_SCENE_CONFIGURATION_SET_V3;
  set_frame->properties1 = 0;
  set_frame->properties1 |= (slow_refresh == true) ? (1 << 7) : 0;

  *frame_length = sizeof(ZW_CENTRAL_SCENE_CONFIGURATION_SET_V3_FRAME);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute update callbacks
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Attribute callback function listening for Central Scene version attribute
 *        updates.
 *
 * @param updated_node    Attribute Store node that was modified.
 * @param change          Type of change applied to the node.
 */
static void zwave_command_class_central_scene_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(
        COMMAND_CLASS_CENTRAL_SCENE_V3,
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
                                 attribute_list,
                                 COUNT_OF(attribute_list));

  // No scene is active by default, until we receive a notification.
  // No need to add the sequence number under the scene just yet.
  central_scene_scene_t active_scene = NO_SCENE_ACTIVE;
  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(ACTIVE_SCENE),
                                     &active_scene,
                                     sizeof(active_scene));

  // Advertise that such nodes will generate RecallScene commands
  zwave_endpoint_id_t endpoint_id = 0;
  unid_t unid;
  if (SL_STATUS_OK
      == attribute_store_network_helper_get_unid_endpoint_from_node(
        endpoint_node,
        unid,
        &endpoint_id)) {
    uic_mqtt_dotdot_scenes_supported_commands_t commands
      = {.recall_scene = true};
    uic_mqtt_dotdot_scenes_publish_supported_generated_commands(unid,
                                                                endpoint_id,
                                                                &commands);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_central_scene_init()
{
  // Attribute resolver rules
  attribute_resolver_register_rule(
    ATTRIBUTE(NUMBER_OF_SCENES),
    NULL,
    &zwave_command_class_central_scene_supported_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(SLOW_REFRESH),
    &zwave_command_class_central_scene_configuration_set,
    &zwave_command_class_central_scene_configuration_get);

  // Attribute store callbacks
  attribute_store_register_callback_by_type(
    zwave_command_class_central_scene_on_version_attribute_update,
    ATTRIBUTE(VERSION));

  // Tell AGI that we want to receive Central Scene Notifications via assocation
  // groups
  zwave_command_class_agi_request_to_establish_association(
    COMMAND_CLASS_CENTRAL_SCENE_V3,
    CENTRAL_SCENE_NOTIFICATION_V3);

  // Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler = &zwave_command_class_central_scene_control_handler;
  // Not supported, so this does not really matter
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_CENTRAL_SCENE_V3;
  handler.version                    = CENTRAL_SCENE_VERSION_V3;
  handler.command_class_name         = "Central Scene";
  handler.comments                   = "Partial control: key attributes are not displayed in the UI.";

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}