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
#include "zwave_command_class_scene_actuator_configuration_control.h"
#include "zwave_command_classes_utils.h"

// Includes from other ZPC Components
#include "zwave_command_class_indices.h"
#include "zwave_controller_connection_info.h"
#include "zwave_command_handler.h"
#include "zpc_attribute_resolver.h"
#include "zpc_attribute_store_network_helper.h"

// Interfaces
#include "zwave_command_class_local_scenes_types.h"
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"

// Includes from Unify Components
#include "attribute_store_helper.h"
#include "attribute_resolver.h"
#include "dotdot_mqtt.h"
#include "sl_log.h"

#define LOG_TAG "zwave_command_class_scene_actuator_configuration"
// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_SCENE_ACTUATOR_##type

// Frame parsing defines
#define CONFIGURATION_REPORT_SCENE_ID_INDEX         2
#define CONFIGURATION_REPORT_LEVEL_INDEX            3
#define CONFIGURATION_REPORT_DIMMING_DURATION_INDEX 4

///////////////////////////////////////////////////////////////////////////////
// Attribute resolution functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Prepares a Scene Actuator Configuration Set Command.
 *
 * @param scene_id_node   Attribute Store node to resolve (Actuator Scene ID)
 * @param frame           Pointer at which frame data can be written.
 * @param frame_length    Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t scene_actuator_configuration_set(
  attribute_store_node_t scene_id_node, uint8_t *frame, uint16_t *frame_length)
{
  // Get the Scene ID
  local_scene_id_t scene_id = 0;
  attribute_store_get_reported(scene_id_node, &scene_id, sizeof(scene_id));

  if (scene_id == 0) {
    *frame_length = 0;
    return SL_STATUS_FAIL;
  }

  // Get the Duration
  attribute_store_node_t duration_node
    = attribute_store_get_first_child_by_type(
      scene_id_node,
      ATTRIBUTE(SETTING_DIMMING_DURATION));
  local_scene_dimming_duration_t duration = 0;
  attribute_store_get_desired_else_reported(duration_node,
                                            &duration,
                                            sizeof(duration));

  // Get the Level
  attribute_store_node_t level_node
    = attribute_store_get_first_child_by_type(scene_id_node,
                                              ATTRIBUTE(SETTING_LEVEL));
  local_scene_level_t level = 0;
  attribute_store_get_desired_else_reported(level_node, &level, sizeof(level));

  // Get the override:
  attribute_store_node_t override_node
    = attribute_store_get_first_child_by_type(scene_id_node,
                                              ATTRIBUTE(SETTING_OVERRIDE));
  local_scene_override_t override = 0;
  attribute_store_get_desired_else_reported(override_node,
                                            &override,
                                            sizeof(override));

  ZW_SCENE_ACTUATOR_CONF_SET_FRAME *set_frame
    = (ZW_SCENE_ACTUATOR_CONF_SET_FRAME *)frame;
  set_frame->cmdClass        = COMMAND_CLASS_SCENE_ACTUATOR_CONF;
  set_frame->cmd             = SCENE_ACTUATOR_CONF_SET;
  set_frame->sceneId         = scene_id;
  set_frame->dimmingDuration = duration;
  set_frame->level           = level;
  set_frame->level2          = override ? 0x80 : 0;  // Override | Reserved byte

  *frame_length = sizeof(ZW_SCENE_ACTUATOR_CONF_SET_FRAME);
  return SL_STATUS_OK;
}

/**
 * @brief Prepares a Scene Actuator Configuration Get Command.
 *
 * @param node          Attribute Store node to resolve (Actuator Scene Level, duration)
 * @param frame         Pointer at which frame data can be written.
 * @param frame_length  Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t scene_actuator_configuration_get(attribute_store_node_t node,
                                                    uint8_t *frame,
                                                    uint16_t *frame_length)
{
  // Get the Association Group ID.
  attribute_store_node_t scene_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(ID));
  local_scene_id_t scene_id = 0;
  attribute_store_get_reported(scene_id_node, &scene_id, sizeof(scene_id));

  if (scene_id == 0) {
    *frame_length = 0;
    return SL_STATUS_FAIL;
  }

  // Check if we do not want to set here. We set before get.
  // This will work because the Set rule is registered on a different attribute
  // type than the get rule, and the resolver won't know it's the same.
  local_scene_id_t desired_scene_id = scene_id;
  attribute_store_get_desired(scene_id_node,
                              &desired_scene_id,
                              sizeof(desired_scene_id));
  sl_log_debug(LOG_TAG, "Scene ID %d, desired %d", scene_id, desired_scene_id);
  if (desired_scene_id != scene_id) {
    *frame_length = 0;
    return SL_STATUS_IS_WAITING;
  }

  // We just need to get.
  ZW_SCENE_ACTUATOR_CONF_GET_FRAME *get_frame
    = (ZW_SCENE_ACTUATOR_CONF_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_SCENE_ACTUATOR_CONF;
  get_frame->cmd      = SCENE_ACTUATOR_CONF_GET;
  get_frame->sceneId  = scene_id;

  *frame_length = sizeof(ZW_SCENE_ACTUATOR_CONF_GET_FRAME);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute update callbacks
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Attribute callback function listening for Scene Actuator Configuration
 *        version attribute updates.
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
        COMMAND_CLASS_SCENE_ACTUATOR_CONF,
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

  // Create Local Scenes -> Configuration
  attribute_store_node_t local_scenes_node
    = attribute_store_create_child_if_missing(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_SCENE_LOCAL_SCENES);

  attribute_store_create_child_if_missing(local_scenes_node,
                                          ATTRIBUTE(CONFIGURATION));
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Resolver callback functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Decides what to do after we've sent a Scene Actuator Configuration
 *        Set Command.
 *
 * @param scene_id_node     Attribute Store node that was resolved
 * @param rule_type         The type of rule that was applied (Set/Get)
 * @param event             The outcome of the resolution.
 */
static void on_scene_actuator_configuration_set_send_data_complete(
  attribute_store_node_t scene_id_node,
  resolver_rule_type_t rule_type,
  zpc_resolver_event_t event)
{
  if (rule_type == RESOLVER_GET_RULE) {
    // This should not happen. Scream!
    sl_log_error(LOG_TAG,
                 "Resolver just executed a Get Rule on an Actuator Scene ID "
                 "configuration, where we have no Get rule registered. "
                 "This should not happen.");
    return;
  }

  // Get the children nodes:
  attribute_store_node_t duration_node
    = attribute_store_create_child_if_missing(
      scene_id_node,
      ATTRIBUTE(SETTING_DIMMING_DURATION));

  attribute_store_node_t level_node
    = attribute_store_create_child_if_missing(scene_id_node,
                                              ATTRIBUTE(SETTING_LEVEL));

  attribute_store_node_t override_node
    = attribute_store_create_child_if_missing(scene_id_node,
                                              ATTRIBUTE(SETTING_OVERRIDE));

  // If we set with override = 0, just probe again.
  local_scene_override_t override = 1;
  attribute_store_get_desired(override_node, &override, sizeof(override));
  if (override == 0) {
    // Always just undefine the scene desired value.
    attribute_store_undefine_desired(scene_id_node);
    // Undefine everything in the children, it will trigger a Get resolution.
    attribute_store_undefine_desired(duration_node);
    attribute_store_undefine_desired(level_node);
    attribute_store_undefine_desired(override_node);
    attribute_store_undefine_reported(duration_node);
    attribute_store_undefine_reported(level_node);
    attribute_store_undefine_reported(override_node);
    return;
  }

  switch (event) {
    case FRAME_SENT_EVENT_OK_SUPERVISION_WORKING:
      // Wait for more.
      return;

    case FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS:
      // Always just undefine the scene desired value.
      attribute_store_undefine_desired(scene_id_node);
      // Children nodes
      attribute_store_set_reported_as_desired(duration_node);
      attribute_store_set_reported_as_desired(level_node);
      override = 0;
      attribute_store_set_reported(override_node, &override, sizeof(override));
      attribute_store_undefine_desired(override_node);
      break;

    default:
      // Always just undefine the scene desired value.
      attribute_store_undefine_desired(scene_id_node);
      // Any other case than success, we just probe again
      attribute_store_undefine_desired(duration_node);
      attribute_store_undefine_desired(level_node);
      attribute_store_undefine_desired(override_node);
      attribute_store_undefine_reported(duration_node);
      attribute_store_undefine_reported(level_node);
      attribute_store_undefine_reported(override_node);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Command Class frame handler function
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Process an incoming Scene Actuator Configuration Report Command.
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t handle_scene_actuator_configuration_report(
  const zwave_controller_connection_info_t *info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  if (frame_length <= CONFIGURATION_REPORT_SCENE_ID_INDEX) {
    // We need at least a Scene ID to do something
    return SL_STATUS_OK;
  }
  local_scene_id_t scene_id = frame[CONFIGURATION_REPORT_SCENE_ID_INDEX];
  if (scene_id == 0) {
    // Scene ID has to be in range 1..255. We cannot guess anything if 0.
    return SL_STATUS_OK;
  }

  local_scene_dimming_duration_t duration = 0;
  if (frame_length > CONFIGURATION_REPORT_DIMMING_DURATION_INDEX) {
    duration = frame[CONFIGURATION_REPORT_DIMMING_DURATION_INDEX];
  }

  local_scene_level_t level = 0;
  if (frame_length > CONFIGURATION_REPORT_LEVEL_INDEX) {
    level = frame[CONFIGURATION_REPORT_LEVEL_INDEX];
  }

  // Given that we do not try to interview all 255 scene settings
  // Here we are resilient and if we received an unsolicited report, we
  // accept it and create all attributes even if they should have been there
  // before
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(info);

  attribute_store_node_t local_scenes_node
    = attribute_store_create_child_if_missing(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_SCENE_LOCAL_SCENES);
  attribute_store_node_t actuator_configuration_node
    = attribute_store_create_child_if_missing(local_scenes_node,
                                              ATTRIBUTE(CONFIGURATION));

  attribute_store_node_t scene_id_node
    = attribute_store_emplace(actuator_configuration_node,
                              ATTRIBUTE(ID),
                              &scene_id,
                              sizeof(scene_id));
  attribute_store_undefine_desired(scene_id_node);

  attribute_store_node_t duration_node
    = attribute_store_create_child_if_missing(
      scene_id_node,
      ATTRIBUTE(SETTING_DIMMING_DURATION));
  attribute_store_set_reported(duration_node, &duration, sizeof(duration));
  attribute_store_undefine_desired(duration_node);

  attribute_store_node_t level_node
    = attribute_store_create_child_if_missing(scene_id_node,
                                              ATTRIBUTE(SETTING_LEVEL));
  attribute_store_set_reported(level_node, &level, sizeof(level));
  attribute_store_undefine_desired(level_node);

  const local_scene_override_t override = 0;
  attribute_store_node_t override_node
    = attribute_store_create_child_if_missing(scene_id_node,
                                              ATTRIBUTE(SETTING_OVERRIDE));
  attribute_store_set_reported(override_node, &override, sizeof(override));
  attribute_store_undefine_desired(override_node);

  sl_log_debug(LOG_TAG,
               "Node ID %d:%d Scene Actuator Configuration for Scene %d : "
               "Level %d / Duration %d",
               info->remote.node_id,
               info->remote.endpoint_id,
               scene_id,
               duration,
               level);

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Incoming commands handler
///////////////////////////////////////////////////////////////////////////////
static sl_status_t scene_actuator_configuration_control_handler(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case SCENE_ACTUATOR_CONF_REPORT:
      return handle_scene_actuator_configuration_report(connection,
                                                        frame_data,
                                                        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Dotdot mqtt callbacks for incoming Scene commands
///////////////////////////////////////////////////////////////////////////////
static sl_status_t
  scenes_cluster_view_scene(dotdot_unid_t unid,
                            dotdot_endpoint_id_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t call_type,
                            SGroupId groupid,
                            uint8_t sceneid)
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

  // Make sure to create the Local scene settings to trigger a Get.
  attribute_store_node_t local_scenes_node
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_SCENE_LOCAL_SCENES);
  attribute_store_node_t actuator_configuration_node
    = attribute_store_get_first_child_by_type(local_scenes_node,
                                              ATTRIBUTE(CONFIGURATION));

  attribute_store_node_t scene_id_node
    = attribute_store_emplace(actuator_configuration_node,
                              ATTRIBUTE(ID),
                              &scene_id,
                              sizeof(scene_id));
  attribute_store_undefine_desired(scene_id_node);
  attribute_store_create_child_if_missing(scene_id_node,
                                          ATTRIBUTE(SETTING_DIMMING_DURATION));
  attribute_store_create_child_if_missing(scene_id_node,
                                          ATTRIBUTE(SETTING_LEVEL));
  return SL_STATUS_OK;
}

static sl_status_t
  scenes_cluster_store_scene(dotdot_unid_t unid,
                             dotdot_endpoint_id_t endpoint,
                             uic_mqtt_dotdot_callback_call_type_t call_type,
                             SGroupId groupid,
                             uint8_t sceneid)
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
  attribute_store_node_t actuator_configuration_node
    = attribute_store_get_first_child_by_type(local_scenes_node,
                                              ATTRIBUTE(CONFIGURATION));

  // Now create the Scene ID if it does not exist, and configure it.
  attribute_store_node_t scene_id_node
    = attribute_store_emplace(actuator_configuration_node,
                              ATTRIBUTE(ID),
                              &scene_id,
                              sizeof(scene_id));
  // Set the desired to 0, so it triggers a resolution.
  scene_id = 0;
  attribute_store_set_desired(scene_id_node, &sceneid, sizeof(sceneid));

  // Set the override to 0, to use the current node's settngs.
  const uint8_t override = 0;
  attribute_store_set_child_desired(scene_id_node,
                                    ATTRIBUTE(SETTING_OVERRIDE),
                                    &override,
                                    sizeof(override));

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Init function
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_scene_actuator_configuration_control_init()
{
  // DotDot MQTT callbacks:
  uic_mqtt_dotdot_scenes_view_scene_callback_set(&scenes_cluster_view_scene);
  uic_mqtt_dotdot_scenes_store_scene_callback_set(&scenes_cluster_store_scene);

  // Attribute Resolver rules
  // Set on the top level Scene ID
  // Set the desired of a Scene ID to 0 to trigger a configuration
  attribute_resolver_register_rule(ATTRIBUTE(ID),
                                   &scene_actuator_configuration_set,
                                   NULL);

  // Get on the individual parameters for a Scene ID
  attribute_resolver_register_rule(ATTRIBUTE(SETTING_DIMMING_DURATION),
                                   NULL,
                                   &scene_actuator_configuration_get);
  attribute_resolver_register_rule(ATTRIBUTE(SETTING_LEVEL),
                                   NULL,
                                   &scene_actuator_configuration_get);
  // Attribute Store callbacks
  attribute_store_register_callback_by_type(&on_version_attribute_update,
                                            ATTRIBUTE(CONF_VERSION));

  // Attribute Resolver event listener:
  register_send_event_handler(
    ATTRIBUTE(ID),
    &on_scene_actuator_configuration_set_send_data_complete);

  // Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler    = &scene_actuator_configuration_control_handler;
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  handler.command_class      = COMMAND_CLASS_SCENE_ACTUATOR_CONF;
  handler.version            = 1;
  handler.command_class_name = "Scene Actuator Configuration";
  handler.manual_security_validation = false;
  handler.comments                   = "No interview performed automatically.";

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}