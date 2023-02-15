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
#include "binding_cluster_mapper_helper.h"

// ZPC components
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"

// Interfaces
#include "ZW_classcmd.h"
#include "dotdot_cluster_id_definitions.h"
#include "dotdot_cluster_command_id_definitions.h"
#include "zwave_command_class_indices.h"

// Unify components
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "sl_log.h"

// Standard includes
#include <map>
#include <set>
// Types for the attribute store.
constexpr attribute_store_type_t GROUP_ID_TYPE
  = ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID;
constexpr attribute_store_type_t GROUP_COMMAND_LIST_TYPE
  = ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST;
constexpr attribute_store_type_t GROUP_COUNT_TYPE
  = ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_SUPPORTED_GROUPINGS;

// Main map between Cluster and AGI commands class/command pairs
const std::map<std::pair<zwave_command_class_t, zwave_command_t>,
               dotdot_cluster_id_t>
  agi_commands_cluster_map = {
    // clang-format off
    // OnOff Cluster
    {{COMMAND_CLASS_BASIC, BASIC_SET}, DOTDOT_ON_OFF_CLUSTER_ID},
    {{COMMAND_CLASS_SWITCH_BINARY, SWITCH_BINARY_SET}, DOTDOT_ON_OFF_CLUSTER_ID},
    // Level Cluster
    {{COMMAND_CLASS_SWITCH_MULTILEVEL, SWITCH_MULTILEVEL_SET}, DOTDOT_LEVEL_CLUSTER_ID},
    {{COMMAND_CLASS_SWITCH_MULTILEVEL, SWITCH_MULTILEVEL_START_LEVEL_CHANGE}, DOTDOT_LEVEL_CLUSTER_ID},
    {{COMMAND_CLASS_SWITCH_MULTILEVEL, SWITCH_MULTILEVEL_STOP_LEVEL_CHANGE}, DOTDOT_LEVEL_CLUSTER_ID},
    // Idenfity Cluster
    {{COMMAND_CLASS_INDICATOR, INDICATOR_SET}, DOTDOT_IDENTIFY_CLUSTER_ID},
    // DoorLock Cluster
    {{COMMAND_CLASS_DOOR_LOCK, DOOR_LOCK_OPERATION_SET}, DOTDOT_DOOR_LOCK_CLUSTER_ID},
    {{COMMAND_CLASS_DOOR_LOCK, DOOR_LOCK_CONFIGURATION_SET}, DOTDOT_DOOR_LOCK_CLUSTER_ID},
    // WindowCovering Cluster
    {{COMMAND_CLASS_WINDOW_COVERING, WINDOW_COVERING_SET}, DOTDOT_WINDOW_COVERING_CLUSTER_ID},
    {{COMMAND_CLASS_WINDOW_COVERING, WINDOW_COVERING_START_LEVEL_CHANGE}, DOTDOT_WINDOW_COVERING_CLUSTER_ID},
    {{COMMAND_CLASS_WINDOW_COVERING, WINDOW_COVERING_STOP_LEVEL_CHANGE}, DOTDOT_WINDOW_COVERING_CLUSTER_ID},
    // BarrierControl Cluster
    {{COMMAND_CLASS_BARRIER_OPERATOR, BARRIER_OPERATOR_SET}, DOTDOT_BARRIER_CONTROL_CLUSTER_ID},
    // Thermostat Cluster
    {{COMMAND_CLASS_THERMOSTAT_MODE, THERMOSTAT_MODE_SET}, DOTDOT_THERMOSTAT_CLUSTER_ID},
    {{COMMAND_CLASS_THERMOSTAT_SETPOINT, THERMOSTAT_SETPOINT_SET}, DOTDOT_THERMOSTAT_CLUSTER_ID},
    // FanControl Cluster
    {{COMMAND_CLASS_THERMOSTAT_FAN_MODE, THERMOSTAT_FAN_MODE_SET}, DOTDOT_FAN_CONTROL_CLUSTER_ID},
    // DehumidificationControl Cluster
    {{COMMAND_CLASS_HUMIDITY_CONTROL_MODE, HUMIDITY_CONTROL_MODE_SET}, DOTDOT_DEHUMIDIFICATION_CONTROL_CLUSTER_ID},
    // ColorControl Cluster
    {{COMMAND_CLASS_SWITCH_COLOR, SWITCH_COLOR_SET}, DOTDOT_COLOR_CONTROL_CLUSTER_ID},
    // Scenes Cluster
    {{COMMAND_CLASS_SCENE_ACTIVATION, SCENE_ACTIVATION_SET}, DOTDOT_SCENES_CLUSTER_ID},
    // clang-format on
};

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
//////////////////////////////////////////////////////////////////////////////
/**
 * @brief Checks if a Cluster ID is represented by some of the Command Classes
 * sent by an Association group.
 *
 * @param cluster_id                The ClusterID to match from the Command Class
 *                                  group command list
 * @param group_command_list_node   The attribute store node of the command class
 *                                  group command list.
 * @returns true if the cluster ID is matched, false otherwise
 */
static bool is_cluster_in_group_command_list(
  dotdot_cluster_id_t cluster_id,
  attribute_store_node_t group_command_list_node)
{
  uint8_t command_list[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  uint8_t command_list_length                                = 0;
  attribute_store_get_node_attribute_value(group_command_list_node,
                                           REPORTED_ATTRIBUTE,
                                           command_list,
                                           &command_list_length);

  for (uint8_t i = 0; i < command_list_length;) {
    if ((i + 1) >= command_list_length) {
      break;
    }
    // Assemble the command class / Command pair
    zwave_command_class_t command_class = command_list[i];
    zwave_command_t command             = command_list[i + 1];
    if (command_class >= EXTENDED_COMMAND_CLASS_IDENTIFIER_START) {
      if ((i + 2) >= command_list_length) {
        break;
      }
      command_class = (command_class << 8) | command_list[i + 1];
      command       = command_list[i + 2];
      i += 3;
    } else {
      i += 2;
    }

    // Check if it matches our cluster:
    if (agi_commands_cluster_map.count({command_class, command})
        && agi_commands_cluster_map.at({command_class, command})
             == cluster_id) {
      return true;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Public functions
//////////////////////////////////////////////////////////////////////////////
bool is_command_class_and_command_mapped_to_bindings(
  zwave_command_class_t command_class, zwave_command_t command)
{
  return agi_commands_cluster_map.count({command_class, command});
}

void binding_mapper_helper_get_association_group_list_from_cluster(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  dotdot_cluster_id_t cluster_id,
  association_group_id_t *group_id_list,
  uint8_t *number_of_groups)
{
  uint8_t matched_groups = 0;
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  // Iterate through all association groups
  attribute_store_node_t group_id_node
    = attribute_store_get_first_child_by_type(endpoint_node, GROUP_ID_TYPE);
  uint32_t group_id_index = 0;

  while (group_id_node != ATTRIBUTE_STORE_INVALID_NODE
         && matched_groups < MAXIMUM_NUMBER_OF_ASSOCIATION_GROUPS) {
    association_group_id_t group_id = 0;
    attribute_store_get_reported(group_id_node, &group_id, sizeof(group_id));

    if (group_id != 0
        && is_cluster_in_group_command_list(
          cluster_id,
          attribute_store_get_first_child_by_type(group_id_node,
                                                  GROUP_COMMAND_LIST_TYPE))) {
      group_id_list[matched_groups] = group_id;
      matched_groups += 1;
    }

    group_id_index += 1;
    group_id_node = attribute_store_get_node_child_by_type(endpoint_node,
                                                           GROUP_ID_TYPE,
                                                           group_id_index);
  }

  *number_of_groups = matched_groups;
  return;
}

void binding_mapper_helper_get_cluster_list_from_association_group(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  association_group_id_t group_id,
  dotdot_cluster_id_t *cluster_id_list,
  uint8_t *number_of_clusters)
{
  std::set<dotdot_cluster_id_t> matched_clusters;
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  // Iterate through all association groups
  attribute_store_node_t group_id_node
    = attribute_store_get_node_child_by_value(endpoint_node,
                                              GROUP_ID_TYPE,
                                              REPORTED_ATTRIBUTE,
                                              &group_id,
                                              sizeof(group_id),
                                              0);

  attribute_store_node_t group_command_list_node
    = attribute_store_get_first_child_by_type(group_id_node,
                                              GROUP_COMMAND_LIST_TYPE);

  uint8_t command_list[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  uint8_t command_list_length                                = 0;
  attribute_store_get_node_attribute_value(group_command_list_node,
                                           REPORTED_ATTRIBUTE,
                                           command_list,
                                           &command_list_length);

  for (uint8_t i = 0; i < command_list_length;) {
    if ((i + 1) >= command_list_length) {
      break;
    }
    // Assemble the command class / Command pair
    zwave_command_class_t command_class = command_list[i];
    zwave_command_t command             = command_list[i + 1];
    if (command_class >= EXTENDED_COMMAND_CLASS_IDENTIFIER_START) {
      if ((i + 2) >= command_list_length) {
        break;
      }
      command_class = (command_class << 8) | command_list[i + 1];
      command       = command_list[i + 2];
      i += 3;
    } else {
      i += 2;
    }
    // We have a command class / command pair, check if it matches a cluster:
    if (agi_commands_cluster_map.count({command_class, command})) {
      matched_clusters.insert(
        agi_commands_cluster_map.at({command_class, command}));
    }
  }

  // Copy back into user pointers:
  uint8_t set_index = 0;
  for (dotdot_cluster_id_t cluster: matched_clusters) {
    if (set_index < MAXIMUM_NUMBER_OF_BINDABLE_CLUSTERS) {
      cluster_id_list[set_index] = cluster;
      set_index += 1;
    }
  }

  *number_of_clusters = set_index;
  return;
}

void binding_mapper_helper_get_bindable_cluster_list_from_node(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  dotdot_cluster_id_t *cluster_id_list,
  uint8_t *number_of_clusters)
{
  std::set<dotdot_cluster_id_t> matched_clusters;
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  // Iterate through all association groups
  attribute_store_node_t group_count_node
    = attribute_store_get_first_child_by_type(endpoint_node, GROUP_COUNT_TYPE);
  association_group_count_t group_count = 0;
  attribute_store_get_reported(group_count_node,
                               &group_count,
                               sizeof(group_count));

  for (association_group_id_t group_id = 1; group_id <= group_count;
       group_id++) {
    binding_mapper_helper_get_cluster_list_from_association_group(
      unid,
      endpoint,
      group_id,
      cluster_id_list,
      number_of_clusters);

    // temporarily copy in a set:
    for (uint8_t i = 0; i < *number_of_clusters; i++) {
      matched_clusters.insert(cluster_id_list[i]);
    }
  }

  // Copy back into user pointers:
  uint8_t set_index = 0;
  for (dotdot_cluster_id_t cluster: matched_clusters) {
    if (set_index < MAXIMUM_NUMBER_OF_BINDABLE_CLUSTERS) {
      cluster_id_list[set_index] = cluster;
      set_index += 1;
    }
  }

  *number_of_clusters = set_index;
  return;
}