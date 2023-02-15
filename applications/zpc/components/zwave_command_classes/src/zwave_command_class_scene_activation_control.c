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
#include "zwave_command_class_scene_activation_control.h"
#include "zwave_command_classes_utils.h"
#include "zwave_command_class_switch_color.h"

// Includes from other ZPC Components
#include "zwave_command_class_indices.h"
#include "zwave_controller_connection_info.h"
#include "zwave_command_handler.h"
#include "zpc_attribute_store_network_helper.h"
#include "zpc_attribute_resolver.h"
#include "zwave_unid.h"

// Interfaces
#include "zwave_command_class_local_scenes_types.h"
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"

// Includes from Unify Components
#include "attribute_store_helper.h"
#include "attribute_resolver.h"
#include "dotdot_mqtt.h"
#include "dotdot_mqtt_generated_commands.h"
#include "sl_log.h"

#define LOG_TAG "zwave_command_class_scene_activation"
// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_SCENE_##type

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Pauses the resolution of Binary Switch, Color and Multilevel Switch CC
 * until the scene activation has been made
 *
 * @param endpoint_node
 */
static void
  pause_actuator_command_class_resolution(attribute_store_node_t endpoint_node)
{
  attribute_store_node_t state_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_STATE);
  attribute_resolver_pause_node_resolution(state_node);
  state_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_STATE);
  attribute_resolver_pause_node_resolution(state_node);
  state_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_STATE);
  attribute_resolver_pause_node_resolution(state_node);
}

/**
 * @brief Resumes resolution and undefines the reported values for the
 * Binary Switch, Color and Multilevel Switch CC
 *
 * @param endpoint_node
 */
static void
  resume_actuator_command_class_resolution(attribute_store_node_t endpoint_node)
{
  attribute_store_node_t state_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_STATE);
  attribute_store_undefine_desired(state_node);
  attribute_store_undefine_reported(state_node);
  attribute_resolver_resume_node_resolution(state_node);
  state_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_STATE);
  attribute_store_undefine_desired(state_node);
  attribute_store_undefine_reported(state_node);
  attribute_resolver_resume_node_resolution(state_node);
  state_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_STATE);
  zwave_command_class_switch_color_invoke_on_all_attributes(
    state_node,
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
    &attribute_store_undefine_reported);
  zwave_command_class_switch_color_invoke_on_all_attributes(
    state_node,
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
    &attribute_store_undefine_desired);
  zwave_command_class_switch_color_invoke_on_all_attributes(
    state_node,
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_DURATION,
    &attribute_store_undefine_reported);
  zwave_command_class_switch_color_invoke_on_all_attributes(
    state_node,
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_DURATION,
    attribute_store_undefine_desired);
  attribute_resolver_resume_node_resolution(state_node);
}

///////////////////////////////////////////////////////////////////////////////
// Attribute resolution functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Prepares a Scene Activation Set Command.
 *
 * @param node          Attribute Store node to resolve
 * @param frame         Pointer at which frame data can be written.
 * @param frame_length  Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t scene_activation_set(attribute_store_node_t node,
                                        uint8_t *frame,
                                        uint16_t *frame_length)
{
  local_scene_id_t desired_scene = 1;
  attribute_store_get_desired_else_reported(node,
                                            &desired_scene,
                                            sizeof(desired_scene));

  attribute_store_node_t duration_node
    = attribute_store_get_first_child_by_type(
      node,
      ATTRIBUTE(ACTIVATION_SETTING_DIMMING_DURATION));
  local_scene_dimming_duration_t duration = 0;
  attribute_store_get_desired_else_reported(duration_node,
                                            &duration,
                                            sizeof(duration));

  ZW_SCENE_ACTIVATION_SET_FRAME *set_frame
    = (ZW_SCENE_ACTIVATION_SET_FRAME *)frame;
  set_frame->cmdClass        = COMMAND_CLASS_SCENE_ACTIVATION;
  set_frame->cmd             = SCENE_ACTIVATION_SET;
  set_frame->sceneId         = desired_scene;
  set_frame->dimmingDuration = duration;

  *frame_length = sizeof(ZW_SCENE_ACTIVATION_SET_FRAME);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute update callbacks
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Attribute callback function listening for Scene Activation version
 *        attribute updates.
 *
 * @param updated_node    Attribute Store node that was modified.
 * @param change          Type of change applied to the node.
 */
static void on_version_attribute_update(attribute_store_node_t updated_node,
                                        attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(
        COMMAND_CLASS_SCENE_ACTIVATION,
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

  // Create Local Scenes -> Active Scene -> Dimming duration.
  attribute_store_node_t local_scenes_node
    = attribute_store_create_child_if_missing(endpoint_node,
                                              ATTRIBUTE(LOCAL_SCENES));
  attribute_store_node_t active_scene_node
    = attribute_store_create_child_if_missing(
      local_scenes_node,
      ATTRIBUTE(ACTIVATION_ACTIVE_SCENE));

  if (false == attribute_store_is_reported_defined(active_scene_node)) {
    const local_scene_id_t active_scene = 1;
    attribute_store_set_reported(active_scene_node,
                                 &active_scene,
                                 sizeof(active_scene));
  }

  attribute_store_node_t dimming_duration_node
    = attribute_store_create_child_if_missing(
      active_scene_node,
      ATTRIBUTE(ACTIVATION_SETTING_DIMMING_DURATION));
  if (false == attribute_store_is_reported_defined(dimming_duration_node)) {
    const local_scene_dimming_duration_t dimming_duration = 0;
    attribute_store_set_reported(dimming_duration_node,
                                 &dimming_duration,
                                 sizeof(dimming_duration));
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Resolver callback functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Decides what to do after we've sent a Scene Activation Set Command.
 *
 * @param node          Attribute Store node that was resolved
 * @param rule_type     The type of rule that was applied (Set/Get)
 * @param event         The outcome of the resolution.
 */
static void
  on_scene_activation_set_send_data_complete(attribute_store_node_t node,
                                             resolver_rule_type_t rule_type,
                                             zpc_resolver_event_t event)
{
  if (rule_type == RESOLVER_GET_RULE) {
    // This should not happen. Scream!
    sl_log_error(LOG_TAG,
                 "Resolver just executed a Get Rule on an Active Scene "
                 "Attribute, where we have no Get rule registered. "
                 "This should not happen.");
    return;
  }

  resume_actuator_command_class_resolution(
    attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID));

  attribute_store_node_t duration_node
    = attribute_store_get_first_child_by_type(
      node,
      ATTRIBUTE(ACTIVATION_SETTING_DIMMING_DURATION));

  switch (event) {
    case FRAME_SENT_EVENT_OK_SUPERVISION_WORKING:
      // Wait for more.
      return;

    case FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT:
    case FRAME_SENT_EVENT_OK_SUPERVISION_FAIL:
      attribute_store_undefine_desired(node);
      attribute_store_set_desired_as_reported(duration_node);
      break;

    default:
      // Any other case, we assume it worked
      attribute_store_set_reported_as_desired(node);
      attribute_store_set_reported_as_desired(duration_node);
      // Do we need to start a timer for the scene duration ? ...
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Command Class frame handler function
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Process an incoming Scene Activation Set Command.
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t
  handle_scene_activation_set(const zwave_controller_connection_info_t *info,
                              const uint8_t *frame,
                              uint16_t frame_length)
{
  // Generate a received command to MQTT (Recall Scene):
  unid_t node_unid;
  zwave_unid_from_node_id(info->remote.node_id, node_unid);

  uic_mqtt_dotdot_scenes_command_recall_scene_fields_t fields = {};

  if (frame_length > 2) {
    fields.sceneid = frame[2];
  }
  if (frame_length > 3) {
    fields.transition_time = frame[3];
    if (0xFF == fields.transition_time) {
      // Use instantaneous if the node requested 0xFF
      fields.transition_time = 0;
    } else if (0x80 <= fields.transition_time) {
      // Multiply by 60 if we received a duration in minutes
      fields.transition_time *= 60;
    }
  }

  uic_mqtt_dotdot_scenes_publish_generated_recall_scene_command(
    node_unid,
    (dotdot_endpoint_id_t)info->remote.endpoint_id,
    &fields);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Incoming commands handler
///////////////////////////////////////////////////////////////////////////////
static sl_status_t scene_activation_control_handler(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case SCENE_ACTIVATION_SET:
      return handle_scene_activation_set(connection, frame_data, frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Dotdot mqtt callbacks for incoming Scene commands
///////////////////////////////////////////////////////////////////////////////
static sl_status_t
  scenes_cluster_recall_scene(dotdot_unid_t unid,
                              dotdot_endpoint_id_t endpoint,
                              uic_mqtt_dotdot_callback_call_type_t call_type,
                              SGroupId groupid,
                              uint8_t sceneid,
                              uint16_t transition_time)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    // Another function is in charge of answering this correctly.
    return SL_STATUS_FAIL;
  }

  local_scene_id_t scene_id = sceneid + 1;
  if (scene_id == 0) {
    sl_log_debug(LOG_TAG, "Cannot map ZCL Scene ID 255 to Z-Wave. Ignoring.");
    return SL_STATUS_OK;
  }

  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  attribute_store_node_t local_scenes_node
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_SCENE_LOCAL_SCENES);
  attribute_store_node_t active_scene_node
    = attribute_store_get_first_child_by_type(
      local_scenes_node,
      ATTRIBUTE(ACTIVATION_ACTIVE_SCENE));

  attribute_store_set_desired(active_scene_node, &scene_id, sizeof(scene_id));
  const local_scene_id_t scene_0 = 0;
  attribute_store_set_reported(active_scene_node, &scene_0, sizeof(scene_0));

  attribute_store_node_t duration_node
    = attribute_store_get_first_child_by_type(
      active_scene_node,
      ATTRIBUTE(ACTIVATION_SETTING_DIMMING_DURATION));
  local_scene_dimming_duration_t duration = 0;
  if (transition_time == 1) {
    duration = 0xFF;
  } else if (transition_time <= 1270) {
    duration = transition_time / 10;
  } else {
    duration = 0x7F + transition_time / 600;
  }
  attribute_store_set_desired(duration_node, &duration, sizeof(duration));

  // Activating a Scene, making sure that the resolver does not try other actuators first
  if (active_scene_node != ATTRIBUTE_STORE_INVALID_NODE) {
    pause_actuator_command_class_resolution(endpoint_node);
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_scene_activation_control_init()
{
  // Get to know when we received a RecallScene command, so we can save
  // the desired Scene/TransitionTime.
  uic_mqtt_dotdot_scenes_recall_scene_callback_set(
    &scenes_cluster_recall_scene);

  // Attribute Resolver rules
  attribute_resolver_register_rule(ATTRIBUTE(ACTIVATION_ACTIVE_SCENE),
                                   &scene_activation_set,
                                   NULL);

  // Attribute Store callbacks
  attribute_store_register_callback_by_type(&on_version_attribute_update,
                                            ATTRIBUTE(ACTIVATION_VERSION));

  // Attribute Resolver event listener:
  register_send_event_handler(ATTRIBUTE(ACTIVATION_ACTIVE_SCENE),
                              &on_scene_activation_set_send_data_complete);

  // Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler         = &scene_activation_control_handler;
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  handler.command_class      = COMMAND_CLASS_SCENE_ACTIVATION;
  handler.version            = 1;
  handler.command_class_name = "Scene Activation";
  handler.manual_security_validation = false;

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}