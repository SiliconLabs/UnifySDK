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
#include "binding_cluster_mapper.h"

// ZPC Components
#include "attribute_store_defined_attribute_types.h"
#include "zpc_attribute_store_network_helper.h"
#include "zpc_attribute_store.h"
#include "zwave_utils.h"
#include "zwave_unid.h"

// Unify Components
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "dotdot_mqtt_supported_generated_commands.h"
#include "sl_log.h"

// Interfaces
#include "ucl_definitions.h"

#define LOG_TAG "binding_cluster_mapper"

///////////////////////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////////////////////
/**
 * @brief Publishes the Supported Generated Commands based on AGI data for an Endpoint
 *
 * @param endpoint_node Attribute store node for the endpoint.
 */
static void
  publish_supported_generated_commands(attribute_store_node_t endpoint_node)
{
  // Get the Node UNID/Endpoint
  unid_t node_unid;
  zwave_endpoint_id_t endpoint_id = 0;
  if (SL_STATUS_OK
      != attribute_store_network_helper_get_unid_endpoint_from_node(
        endpoint_node,
        node_unid,
        &endpoint_id)) {
    sl_log_warning(LOG_TAG,
                   "Cannot read UNID from endpoint node %d, "
                   "no publications will be made.",
                   endpoint_node);
    return;
  }

  if (is_zpc_unid(node_unid)) {
    return;
  }

  // Prepare a list of commands:
  uic_mqtt_dotdot_on_off_supported_commands_t on_off_commands = {};
  uic_mqtt_dotdot_level_supported_commands_t level_commands   = {};
  uic_mqtt_dotdot_scenes_supported_commands_t scenes_commands = {};

  uint32_t group_id_index              = 0;
  attribute_store_node_t group_id_node = attribute_store_get_node_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
    group_id_index);
  group_id_index += 1;

  while (ATTRIBUTE_STORE_INVALID_NODE != group_id_node) {
    attribute_store_node_t group_command_list_node
      = attribute_store_get_first_child_by_type(
        group_id_node,
        ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST);

    uint8_t command_list[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
    uint8_t command_list_length                                = 0;
    attribute_store_get_node_attribute_value(group_command_list_node,
                                             REPORTED_ATTRIBUTE,
                                             command_list,
                                             &command_list_length);

    // Search for OnOff first.
    if (is_command_in_array(COMMAND_CLASS_BASIC,
                            BASIC_SET,
                            command_list,
                            command_list_length)
        || is_command_in_array(COMMAND_CLASS_SWITCH_BINARY,
                               SWITCH_BINARY_SET,
                               command_list,
                               command_list_length)) {
      on_off_commands.on  = true;
      on_off_commands.off = true;
    }

    // Level Cluster
    if (is_command_in_array(COMMAND_CLASS_SWITCH_MULTILEVEL,
                            SWITCH_MULTILEVEL_SET_V2,
                            command_list,
                            command_list_length)) {
      level_commands.move_to_level = true;
    }
    if (is_command_in_array(COMMAND_CLASS_SWITCH_MULTILEVEL,
                            SWITCH_MULTILEVEL_START_LEVEL_CHANGE,
                            command_list,
                            command_list_length)) {
      level_commands.move = true;
    }
    if (is_command_in_array(COMMAND_CLASS_SWITCH_MULTILEVEL,
                            SWITCH_MULTILEVEL_STOP_LEVEL_CHANGE,
                            command_list,
                            command_list_length)) {
      level_commands.stop = true;
    }
    // Search for Central Scene Notifications
    if (is_command_in_array(COMMAND_CLASS_CENTRAL_SCENE,
                            CENTRAL_SCENE_NOTIFICATION,
                            command_list,
                            command_list_length)) {
      scenes_commands.recall_scene = true;
    }

    // Move to the next group
    group_id_node = attribute_store_get_node_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
      group_id_index);
    group_id_index += 1;
  }

  // Publish the list of supported generated commands:
  uic_mqtt_dotdot_on_off_publish_supported_generated_commands(node_unid,
                                                              endpoint_id,
                                                              &on_off_commands);
  uic_mqtt_dotdot_level_publish_supported_generated_commands(node_unid,
                                                             endpoint_id,
                                                             &level_commands);
  uic_mqtt_dotdot_scenes_publish_supported_generated_commands(node_unid,
                                                              endpoint_id,
                                                              &scenes_commands);
}

/**
 * @brief Publishes the supported Generated commands when the network status
 * is moved to online functional
 *
 * @param network_status_node     Attribute store node for the network status
 * @param change                  Update that was performed in the attribute store
 */
static void on_network_status_update(attribute_store_node_t network_status_node,
                                     attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_UNAVAILABLE;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  if (NODE_STATE_TOPIC_STATE_INCLUDED != network_status) {
    return;
  }

  // Iterate through the endpoints and publish their supported generated commands
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(network_status_node,
                                                 ATTRIBUTE_NODE_ID);

  uint32_t child_index = 0;
  attribute_store_node_t endpoint_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ENDPOINT_ID,
                                             child_index);
  child_index += 1;
  while (ATTRIBUTE_STORE_INVALID_NODE != endpoint_node) {
    publish_supported_generated_commands(endpoint_node);
    endpoint_node
      = attribute_store_get_node_child_by_type(node_id_node,
                                               ATTRIBUTE_ENDPOINT_ID,
                                               child_index);
    child_index += 1;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public functions
//////////////////////////////////////////////////////////////////////////////
bool binding_cluster_mapper_init(void)
{
  sl_log_debug(LOG_TAG, "Binding Cluster Mapper initialization");

  // The binding cluster mapper server takes care of processing incoming commands
  // and publishing attributes.

  // Here we listen to AGI information to publish which commands are generated
  // by nodes, in case we have a binding/association towards them.
  attribute_store_register_callback_by_type_and_state(&on_network_status_update,
                                                      ATTRIBUTE_NETWORK_STATUS,
                                                      REPORTED_ATTRIBUTE);

  return true;
}