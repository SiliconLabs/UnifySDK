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
#include "zwave_command_class_scene_controller_configuration_control.h"
#include "zwave_command_classes_utils.h"

// Includes from other ZPC Components
#include "zwave_command_class_indices.h"
#include "zwave_controller_connection_info.h"
#include "zwave_command_handler.h"
#include "zwave_unid.h"
#include "zwave_association_toolbox.h"
#include "zpc_attribute_store_network_helper.h"
#include "zpc_attribute_resolver.h"

// Interfaces
#include "zwave_command_class_local_scenes_types.h"
#include "zwave_command_class_association_types.h"
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"

// Includes from Unify Components
#include "attribute_store_helper.h"
#include "attribute_resolver.h"
#include "sl_log.h"

#define LOG_TAG "zwave_command_class_scene_controller_configuration"
// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_SCENE_##type

// Frame parsing defines
#define CONFIGURATION_REPORT_GROUP_ID_INDEX         2
#define CONFIGURATION_REPORT_SCENE_ID_INDEX         3
#define CONFIGURATION_REPORT_DIMMING_DURATION_INDEX 4

///////////////////////////////////////////////////////////////////////////////
// Attribute resolution functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Prepares a Scene Controller Configuration Set Command.
 *
 * @param node          Attribute Store node to resolve (Controller configuration)
 * @param frame         Pointer at which frame data can be written.
 * @param frame_length  Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t scene_controller_configuration_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Get the Association Group ID.
  attribute_store_node_t group_id_node
    = attribute_store_get_first_parent_with_type(
      node,
      ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID);
  association_group_id_t group_id = 0;
  attribute_store_get_reported(group_id_node, &group_id, sizeof(group_id));
  if (group_id == 0) {
    *frame_length = 0;
    return SL_STATUS_FAIL;
  }

  // Get the Scene ID
  attribute_store_node_t scene_id_node
    = attribute_store_get_first_child_by_type(node,
                                              ATTRIBUTE(CONTROLLER_SETTING_ID));

  local_scene_id_t desired_scene = 1;
  attribute_store_get_desired_else_reported(scene_id_node,
                                            &desired_scene,
                                            sizeof(desired_scene));
  // Get the Duration
  attribute_store_node_t duration_node
    = attribute_store_get_first_child_by_type(
      node,
      ATTRIBUTE(CONTROLLER_SETTING_DIMMING_DURATION));
  local_scene_dimming_duration_t duration = 0;
  attribute_store_get_desired_else_reported(duration_node,
                                            &duration,
                                            sizeof(duration));

  ZW_SCENE_CONTROLLER_CONF_SET_FRAME *set_frame
    = (ZW_SCENE_CONTROLLER_CONF_SET_FRAME *)frame;
  set_frame->cmdClass        = COMMAND_CLASS_SCENE_CONTROLLER_CONF;
  set_frame->cmd             = SCENE_CONTROLLER_CONF_SET;
  set_frame->groupId         = group_id;
  set_frame->sceneId         = desired_scene;
  set_frame->dimmingDuration = duration;

  *frame_length = sizeof(ZW_SCENE_CONTROLLER_CONF_SET_FRAME);
  return SL_STATUS_OK;
}

static sl_status_t scene_controller_configuration_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Get the Association Group ID.
  attribute_store_node_t group_id_node
    = attribute_store_get_first_parent_with_type(
      node,
      ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID);
  association_group_id_t group_id = 0;
  attribute_store_get_reported(group_id_node, &group_id, sizeof(group_id));

  if (group_id == 0) {
    *frame_length = 0;
    return SL_STATUS_FAIL;
  }

  ZW_SCENE_CONTROLLER_CONF_GET_FRAME *get_frame
    = (ZW_SCENE_CONTROLLER_CONF_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_SCENE_CONTROLLER_CONF;
  get_frame->cmd      = SCENE_CONTROLLER_CONF_GET;
  get_frame->groupId  = group_id;

  *frame_length = sizeof(ZW_SCENE_CONTROLLER_CONF_GET_FRAME);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Store callback functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Reacts to desired values updates under the Controller Configuration
 * attribute
 *
 * @param updated_node  The child node of the Controller configuration node
 * @param change        Type of attribute store change that was applied.
 */
static void on_desired_scene_controller_configuration_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (ATTRIBUTE_UPDATED != change) {
    return;
  }

  if (attribute_store_is_reported_defined(updated_node)
      && attribute_store_is_desired_defined(updated_node)
      && (false == attribute_store_is_value_matched(updated_node))) {
    set_command_status_value(attribute_store_get_first_parent_with_type(
                               updated_node,
                               ATTRIBUTE(CONTROLLER_CONFIGURATION)),
                             FINAL_STATE,
                             NEEDS_ONE_COMMAND);
  }
}

/**
 * @brief Reacts to Association Group ID updates and
 *
 * @param group_id_node
 * @param change
 */
static void on_association_group_id_update(attribute_store_node_t group_id_node,
                                           attribute_store_change_t change)
{
  if (ATTRIBUTE_UPDATED != change) {
    return;
  }

  zwave_node_id_t node_id         = 0;
  zwave_endpoint_id_t endpoint_id = 0;
  attribute_store_network_helper_get_zwave_ids_from_node(group_id_node,
                                                         &node_id,
                                                         &endpoint_id);

  if (zwave_node_supports_command_class(COMMAND_CLASS_SCENE_CONTROLLER_CONF,
                                        node_id,
                                        endpoint_id)) {
    attribute_store_node_t configuration_node
      = attribute_store_create_child_if_missing(
        group_id_node,
        ATTRIBUTE(CONTROLLER_CONFIGURATION));
    attribute_store_create_child_if_missing(configuration_node,
                                            ATTRIBUTE(CONTROLLER_SETTING_ID));
    attribute_store_create_child_if_missing(
      configuration_node,
      ATTRIBUTE(CONTROLLER_SETTING_DIMMING_DURATION));
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Resolver callback functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Decides what to do after we've sent a Scene Controller Configuration
 * Set Command.
 *
 * @param node          Attribute Store node that was resolved
 * @param rule_type     The type of rule that was applied (Set/Get)
 * @param event         The outcome of the resolution.
 */
static void on_scene_controller_configuration_set_send_data_complete(
  attribute_store_node_t node,
  resolver_rule_type_t rule_type,
  zpc_resolver_event_t event)
{
  if (rule_type == RESOLVER_GET_RULE) {
    return;
  }

  attribute_store_node_t scene_id_node
    = attribute_store_get_first_child_by_type(node,
                                              ATTRIBUTE(CONTROLLER_SETTING_ID));
  attribute_store_node_t duration_node
    = attribute_store_get_first_child_by_type(
      node,
      ATTRIBUTE(CONTROLLER_SETTING_DIMMING_DURATION));

  switch (event) {
    case FRAME_SENT_EVENT_OK_SUPERVISION_WORKING:
      // Wait for more.
      return;

    case FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS:
      attribute_store_set_reported_as_desired(scene_id_node);
      attribute_store_set_reported_as_desired(duration_node);
      set_command_status_value(node, FINAL_STATE, FINAL_STATE);
      return;

    default:
      // It it did not go well, probe again.
      attribute_store_undefine_desired(scene_id_node);
      attribute_store_undefine_desired(duration_node);
      attribute_store_undefine_reported(node);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Command Class frame handler function
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Process an incoming Scene Controller Configuration Report Command.
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t handle_scene_controller_configuration_report(
  const zwave_controller_connection_info_t *info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  if (frame_length <= CONFIGURATION_REPORT_DIMMING_DURATION_INDEX) {
    // Frame too short, we do not bother saving partially.
    return SL_STATUS_OK;
  }

  association_group_id_t group_id = frame[CONFIGURATION_REPORT_GROUP_ID_INDEX];
  local_scene_id_t scene_id       = frame[CONFIGURATION_REPORT_SCENE_ID_INDEX];
  local_scene_dimming_duration_t duration
    = frame[CONFIGURATION_REPORT_DIMMING_DURATION_INDEX];

  attribute_store_node_t scene_configuration_node
    = get_group_child_node(info->remote.node_id,
                           info->remote.endpoint_id,
                           group_id,
                           ATTRIBUTE(CONTROLLER_CONFIGURATION));

  attribute_store_node_t scene_id_node
    = attribute_store_get_first_child_by_type(scene_configuration_node,
                                              ATTRIBUTE(CONTROLLER_SETTING_ID));
  attribute_store_undefine_desired(scene_id_node);
  attribute_store_set_reported(scene_id_node, &scene_id, sizeof(scene_id));

  attribute_store_node_t dimming_duration_node
    = attribute_store_get_first_child_by_type(
      scene_configuration_node,
      ATTRIBUTE(CONTROLLER_SETTING_DIMMING_DURATION));
  attribute_store_undefine_desired(dimming_duration_node);
  attribute_store_set_reported(dimming_duration_node,
                               &duration,
                               sizeof(duration));

  set_command_status_value(scene_configuration_node, FINAL_STATE, FINAL_STATE);

  sl_log_debug(LOG_TAG,
               "Node ID %d:%d Scene Configuration for Association "
               "Group ID %d : Scene %d / Duration %d",
               info->remote.node_id,
               info->remote.endpoint_id,
               group_id,
               scene_id,
               duration);

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
    case SCENE_CONTROLLER_CONF_REPORT:
      return handle_scene_controller_configuration_report(connection,
                                                          frame_data,
                                                          frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

sl_status_t zwave_command_class_scene_controller_configuration_control_init()
{
  // Attribute Resolver rules
  attribute_resolver_register_rule(ATTRIBUTE(CONTROLLER_CONFIGURATION),
                                   &scene_controller_configuration_set,
                                   &scene_controller_configuration_get);

  // Attribute Store callbacks
  // We do not react to version updates. Though, when association groups are
  // created, we will look if this CC is supported to decide if we should do something.
  attribute_store_register_callback_by_type(
    &on_association_group_id_update,
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID);

  // Listening to updates for the nodes under the command_status_t configuration.
  attribute_store_register_callback_by_type_and_state(
    &on_desired_scene_controller_configuration_update,
    ATTRIBUTE(CONTROLLER_SETTING_ID),
    DESIRED_ATTRIBUTE);

  attribute_store_register_callback_by_type_and_state(
    &on_desired_scene_controller_configuration_update,
    ATTRIBUTE(CONTROLLER_SETTING_DIMMING_DURATION),
    DESIRED_ATTRIBUTE);

  // Attribute Resolver event listener:
  register_send_event_handler(
    ATTRIBUTE(CONTROLLER_CONFIGURATION),
    &on_scene_controller_configuration_set_send_data_complete);

  // Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler         = &scene_activation_control_handler;
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  handler.command_class      = COMMAND_CLASS_SCENE_CONTROLLER_CONF;
  handler.version            = 1;
  handler.command_class_name = "Scene Controller Configuration";
  handler.manual_security_validation = false;

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}