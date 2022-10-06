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
#include "zcl_binding_cluster_server.h"
#include "zcl_cluster_servers_helpers.hpp"

// Unify includes
#include "attribute.hpp"
#include "sl_log.h"

// Interfaces
#include "zwave_command_class_version_types.h"
#include "ucl_definitions.h"

// ZPC includes
#include "binding_cluster_mapper_helper.h"
#include "zpc_attribute_store.h"
#include "zwave_utils.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "zwave_command_class_generic_types.h"
#include "zwave_network_management.h"
#include "zwave_association_toolbox.hpp"

// Includes from auto-generated files
#include "dotdot_mqtt.h"
#include "zap-types.h"
#include "dotdot_cluster_id_definitions.h"

// Generic includes
#include <array>
#include <vector>
#include <string.h>

using namespace attribute_store;

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_##type

// Constants
constexpr char LOG_TAG[] = "zcl_binding_cluster_server";
// Maximum number of characters for a Cluster Name (e.g. ProtocolController/RFTelemetry)
constexpr uint8_t MAXIMUM_CLUSTER_NAME_LENGTH = 40;
// Cluster revision
constexpr uint16_t BINDING_CLUSTER_REVISION = 1;
// Endpoint ID used for the local bindings to protocol controller
constexpr dotdot_endpoint_id_t PROTOCOL_CONTROLLER_ENDPOINT_ID = 0;

// Private variables
namespace
{
// Cache used to retrieve Cluster Lists.
std::array<dotdot_cluster_id_t, MAXIMUM_NUMBER_OF_BINDABLE_CLUSTERS>
  cluster_list;
// Number of elements in the Cluster List cache
uint8_t cluster_list_size = 0;

// List of Association Group ID matching a Cluster.
std::array<association_group_id_t, MAXIMUM_NUMBER_OF_ASSOCIATION_GROUPS>
  group_id_list;
// Number of elements in the Association Group ID cache
uint8_t number_of_groups = 0;
}  // namespace

// Operator allowing to compare if 2 BindingObjects are the same
struct binding_object_compare {
  bool operator()(const BindingObject &lhs, const BindingObject &rhs) const
  {
    if (strcmp(lhs.DestinationUnid, rhs.DestinationUnid) > 0) {
      return true;
    } else if (strcmp(lhs.DestinationUnid, rhs.DestinationUnid) < 0) {
      return false;
    } else {
      if (lhs.DestinationEp > rhs.DestinationEp) {
        return true;
      } else if (lhs.DestinationEp < rhs.DestinationEp) {
        return false;
      } else {
        return strcmp(lhs.ClusterName, rhs.ClusterName) > 0;
      }
    }
  }
};

//////////////////////////////////////////////////////////////////////////////
//  Private helper functions.
//////////////////////////////////////////////////////////////////////////////
/**
 * @brief Verifies if the Binding cluster is provided on behalf a node or not
 * (i.e. if it supports Association)
 *
 * @param node_id         Z-Wave NodeID of the node
 * @param endpoint_id     Z-Wave Endpoint ID.
 * @returns true if binding is supported, false otherwise.
 */
bool node_supports_binding_cluster(zwave_node_id_t node_id,
                                   zwave_endpoint_id_t endpoint_id)
{
  // Decide if Binding is supported based on Association/Multi channel Association
  return (zwave_node_supports_command_class(COMMAND_CLASS_ASSOCIATION,
                                            node_id,
                                            endpoint_id))
         || (zwave_node_supports_command_class(
           COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V2,
           node_id,
           endpoint_id));
}
/**
 * @brief Unallocate memory allocated for the binding table.
 *
 * @param binding_table     Binding table to be cleaned up / freed..
 */
static void free_binding_table(const std::vector<BindingObject> &binding_table)
{
  // Get a list of unique pointers:
  std::set<const char *> pointers_to_free;
  for (const auto &binding: binding_table) {
    pointers_to_free.insert(binding.DestinationUnid);
  }

  for (auto pointer: pointers_to_free) {
    free((void *)pointer);
  }
}

/**
 * @brief Iterates via all association groups and fills up a binding table
 * vector object
 *
 * NOTE: this function mallocs strings for the UNIDs and they need to be freed
 * when no longer needed
 *
 * @param unid                 UNID of the node for which we want to publish
 * @param endpoint_id          Endpoint ID for which we want to publish
 * @param [out] binding_table  Vector of BindingObject to fill up.
 * @param value_state          Desired or Reported value
 */
static void get_binding_table(const std::string &unid,
                              zwave_endpoint_id_t endpoint_id,
                              std::vector<BindingObject> &binding_table,
                              attribute_store_node_value_state_t value_state)
{
  attribute_store_node_t endpoint_id_node
    = attribute_store_network_helper_get_endpoint_node(unid.c_str(),
                                                       endpoint_id);

  // Create a set of bindings, so we avoid adding duplicates in the vector
  std::set<BindingObject, binding_object_compare> binding_table_set;
  std::set<const char *> allocated_pointers;

  // Now go around and read the associations
  for (attribute group_id_node:
       attribute(endpoint_id_node).children(ATTRIBUTE(GROUP_ID))) {
    // Get the group content node.
    attribute group_content_node
      = group_id_node.child_by_type(ATTRIBUTE(GROUP_CONTENT));

    // Get the Group ID:
    association_group_id_t group_id = 0;
    attribute_store_get_reported(group_id_node, &group_id, sizeof(group_id));

    // Get the list of bindable clusters from the group:
    BindingObject binding = {};
    association_set association_list;
    try {
      association_bytes assocation_bytes
        = group_content_node.get<association_bytes>(value_state);
      get_association_list(assocation_bytes, association_list);
    } catch (const std::exception &e) {
      sl_log_debug(LOG_TAG,
                   "Warning: Could not read association data. (%s)"
                   "No publications will happen",
                   e.what());
    }

    // Get a list of ClusterNames for that association group
    cluster_list_size = 0;
    binding_mapper_helper_get_cluster_list_from_association_group(
      unid.c_str(),
      endpoint_id,
      group_id,
      cluster_list.data(),
      &cluster_list_size);

    if (cluster_list_size == 0) {
      // No mapping for this group, move to the next.
      continue;
    }

    // For each association, create a binding object:
    for (auto association: association_list) {
      // Fill up destination UNID/Endpoint
      unid_t destination_unid = {};
      zwave_unid_from_node_id(association.node_id, destination_unid);

      //Check if we already have a string for this UNID:
      char *destination_unid_pointer = nullptr;
      for (const auto &pointer: allocated_pointers) {
        if (strcmp(pointer, destination_unid) == 0) {
          destination_unid_pointer = (char *)pointer;
          break;
        }
      }

      // No pointer, allocate one
      if (destination_unid_pointer == nullptr) {
        destination_unid_pointer = (char *)malloc(sizeof(unid_t));
        allocated_pointers.insert(destination_unid_pointer);
        zwave_unid_from_node_id(association.node_id, destination_unid_pointer);
      }

      binding.DestinationUnid = destination_unid_pointer;
      binding.DestinationEp   = association.endpoint_id;

      // List of clusters:
      for (uint8_t i = 0; i < cluster_list_size; i++) {
        binding.ClusterName = uic_dotdot_get_cluster_name(cluster_list[i]);
        binding_table_set.insert(binding);
      }
    }
  }

  // Derive a vector from the unique set.
  for (const auto &binding: binding_table_set) {
    binding_table.push_back(binding);
  }
}

/**
 * @brief Publishes the Binding Cluster bindable cluster attributes
 *
 * @param unid           UNID of the node for which we need to publish
 * @param endpoint_id    Endpoint ID of the node for which we need to publish
 */
static void publish_binding_cluster_bindable_cluster_list_attribute(
  const std::string &unid, zwave_endpoint_id_t endpoint_id)
{
  // Do not publish any state supported commands for ourselves.
  if (true == is_zpc_unid(unid.c_str())) {
    return;
  }

  // Build the base topic and pass it on to DotDot MQTT.
  std::string base_topic
    = "ucl/by-unid/" + std::string(unid) + "/ep" + std::to_string(endpoint_id);

  // Get the bindable cluster list from the mapper helper
  std::array<dotdot_cluster_id_t, MAXIMUM_NUMBER_OF_BINDABLE_CLUSTERS>
    cluster_id_list;
  uint8_t number_of_clusters = 0;
  binding_mapper_helper_get_bindable_cluster_list_from_node(
    unid.c_str(),
    endpoint_id,
    cluster_id_list.data(),
    &number_of_clusters);

  char **bindable_clusters = nullptr;
  if (number_of_clusters > 0) {
    bindable_clusters = (char **)malloc(sizeof(char *) * number_of_clusters);
  }

  for (uint8_t i = 0; i < number_of_clusters; i++) {
    bindable_clusters[i] = (char *)malloc(MAXIMUM_CLUSTER_NAME_LENGTH);
    snprintf(bindable_clusters[i],
             MAXIMUM_CLUSTER_NAME_LENGTH,
             "%s",
             uic_dotdot_get_cluster_name(cluster_id_list.at(i)));
    bindable_clusters[i][MAXIMUM_CLUSTER_NAME_LENGTH - 1] = '\0';
  }

  uic_mqtt_dotdot_binding_bindable_cluster_list_publish(
    base_topic.c_str(),
    number_of_clusters,
    (const char **)bindable_clusters,
    UCL_MQTT_PUBLISH_TYPE_ALL);

  // Clear the malloc'ed memory again
  if (number_of_clusters > 0) {
    for (uint8_t i = 0; i < number_of_clusters; i++) {
      free(bindable_clusters[i]);
    }
    free(bindable_clusters);
  }
}

/**
 * @brief Publishes the Binding Cluster Cluster revision attribute
 *
 * @param unid           UNID of the node for which we need to publish
 * @param endpoint_id    Endpoint ID of the node for which we need to publish

 */
static void
  publish_binding_cluster_revision_attribute(const std::string &unid,
                                             zwave_endpoint_id_t endpoint_id)
{
  // Do not publish any state supported commands for ourselves.
  if (true == is_zpc_unid(unid.c_str())) {
    return;
  }

  // Build the base topic and pass it on to DotDot MQTT.
  std::string base_topic
    = "ucl/by-unid/" + std::string(unid) + "/ep" + std::to_string(endpoint_id);

  uic_mqtt_dotdot_binding_publish_cluster_revision(base_topic.c_str(),
                                                   BINDING_CLUSTER_REVISION);
}

/**
 * @brief Publishes the Binding Table attribute
 *
 * @param unid           UNID of the node for which we need to publish
 * @param endpoint_id    Endpoint ID of the node for which we need to publish
 */
static void publish_binding_table_attribute(const std::string &unid,
                                            zwave_endpoint_id_t endpoint_id)
{
  // Do not publish anything for ourselves
  if (true == is_zpc_unid(unid.c_str())) {
    return;
  }

  // Build the base topic and pass it on to DotDot MQTT.
  std::string base_topic
    = "ucl/by-unid/" + std::string(unid) + "/ep" + std::to_string(endpoint_id);

  // Build the binding table based on the association groups
  std::vector<BindingObject> binding_table;
  get_binding_table(unid,
                    endpoint_id,
                    binding_table,
                    DESIRED_OR_REPORTED_ATTRIBUTE);
  uic_mqtt_dotdot_binding_binding_table_publish(base_topic.c_str(),
                                                binding_table.size(),
                                                binding_table.data(),
                                                UCL_MQTT_PUBLISH_TYPE_DESIRED);

  free_binding_table(binding_table);
  binding_table.clear();
  get_binding_table(unid, endpoint_id, binding_table, REPORTED_ATTRIBUTE);
  uic_mqtt_dotdot_binding_binding_table_publish(base_topic.c_str(),
                                                binding_table.size(),
                                                binding_table.data(),
                                                UCL_MQTT_PUBLISH_TYPE_REPORTED);
  free_binding_table(binding_table);
}

/**
 * @brief Publishes the Binding Table Full attribute
 *
 * @param unid           UNID of the node for which we need to publish
 * @param endpoint_id    Endpoint ID of the node for which we need to publish
 */
static void
  publish_binding_table_full_attribute(const std::string &unid,
                                       zwave_endpoint_id_t endpoint_id)
{
  // Do not publish anything for ourselves
  if (true == is_zpc_unid(unid.c_str())) {
    return;
  }

  // Build the base topic and pass it on to DotDot MQTT.
  std::string base_topic
    = "ucl/by-unid/" + std::string(unid) + "/ep" + std::to_string(endpoint_id);

  attribute_store_node_t endpoint_id_node
    = attribute_store_network_helper_get_endpoint_node(unid.c_str(),
                                                       endpoint_id);
  zwave_node_id_t node_id = 0;
  attribute_store_network_helper_get_node_id_from_node(endpoint_id_node,
                                                       &node_id);

  // Go through the associations and check if all groups that have a bindable
  // cluster have no more space available.
  bool binding_table_full = true;
  for (attribute group_id_node:
       attribute(endpoint_id_node).children(ATTRIBUTE(GROUP_ID))) {
    // Get the Group ID:
    association_group_id_t group_id = 0;
    attribute_store_get_reported(group_id_node, &group_id, sizeof(group_id));

    // Check if there is a bindable cluster:
    cluster_list_size = 0;
    binding_mapper_helper_get_cluster_list_from_association_group(
      unid.c_str(),
      endpoint_id,
      group_id,
      cluster_list.data(),
      &cluster_list_size);
    if (cluster_list_size == 0) {
      continue;
    }

    if (false == is_desired_group_full(node_id, endpoint_id, group_id)) {
      binding_table_full = false;
      break;
    }
  }

  uic_mqtt_dotdot_binding_binding_table_full_publish(
    base_topic.c_str(),
    binding_table_full,
    UCL_MQTT_PUBLISH_TYPE_DESIRED);

  // Once more for the reported value:
  binding_table_full = true;
  for (attribute group_id_node:
       attribute(endpoint_id_node).children(ATTRIBUTE(GROUP_ID))) {
    // Get the Group ID:
    association_group_id_t group_id = 0;
    attribute_store_get_reported(group_id_node, &group_id, sizeof(group_id));

    // Check if there is a bindable cluster:
    cluster_list_size = 0;
    binding_mapper_helper_get_cluster_list_from_association_group(
      unid.c_str(),
      endpoint_id,
      group_id,
      cluster_list.data(),
      &cluster_list_size);
    if (cluster_list_size == 0) {
      continue;
    }

    // We only publish the reported is full, not desired.
    if (false == is_reported_group_full(node_id, endpoint_id, group_id)) {
      binding_table_full = false;
      break;
    }
  }
  uic_mqtt_dotdot_binding_binding_table_full_publish(
    base_topic.c_str(),
    binding_table_full,
    UCL_MQTT_PUBLISH_TYPE_REPORTED);
}

//////////////////////////////////////////////////////////////////////////////
// DotDot commands callback functions.
//////////////////////////////////////////////////////////////////////////////
sl_status_t bind_callback(dotdot_unid_t unid,
                          dotdot_endpoint_id_t endpoint,
                          uic_mqtt_dotdot_callback_call_type_t call_type,
                          const char *cluster_name,
                          const char *destination_unid,
                          uint8_t destination_ep)
{
  attribute_store_node_t endpoint_id_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);
  zwave_node_id_t node_id = 0;
  attribute_store_network_helper_get_node_id_from_node(endpoint_id_node,
                                                       &node_id);

  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    return node_supports_binding_cluster(node_id, endpoint) ? SL_STATUS_OK
                                                            : SL_STATUS_FAIL;
  }

  // Go around and check for Association groups.
  zwave_node_id_t destination_node_id = 0;
  zwave_unid_to_node_id(destination_unid, &destination_node_id);

  association_t new_association = {};
  new_association.node_id       = destination_node_id;
  new_association.endpoint_id   = destination_ep;
  new_association.type
    = ((destination_ep == 0) && (endpoint == 0)) ? NODE_ID : ENDPOINT;

  number_of_groups = 0;
  binding_mapper_helper_get_association_group_list_from_cluster(
    unid,
    endpoint,
    uic_dotdot_get_cluster_id(cluster_name),
    group_id_list.data(),
    &number_of_groups);

  for (uint8_t i = 0; i < number_of_groups; i++) {
    association_group_id_t group_id = group_id_list[i];

    // Check if we can associate without trouble?
    if (false
        == is_association_legal(node_id, endpoint, group_id, new_association)) {
      continue;
    }

    // All checks out, we establish an association.
    sl_log_debug(LOG_TAG,
                 "Adding %s association towards the NodeID %d:%d "
                 "for NodeID %d:%d Group ID %d. (Non-forceful).",
                 new_association.type == NODE_ID ? "NodeID" : "Endpoint",
                 new_association.node_id,
                 new_association.endpoint_id,
                 node_id,
                 endpoint,
                 group_id);
    add_desired_association(node_id, endpoint, group_id, new_association);
  }

  return SL_STATUS_OK;
}

sl_status_t unbind_callback(dotdot_unid_t unid,
                            dotdot_endpoint_id_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t call_type,
                            const char *cluster_name,
                            const char *destination_unid,
                            uint8_t destination_ep)
{
  attribute_store_node_t endpoint_id_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);
  zwave_node_id_t node_id = 0;
  attribute_store_network_helper_get_node_id_from_node(endpoint_id_node,
                                                       &node_id);

  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    return node_supports_binding_cluster(node_id, endpoint) ? SL_STATUS_OK
                                                            : SL_STATUS_FAIL;
  }

  // Go around and check for Association groups.
  zwave_node_id_t destination_node_id = 0;
  zwave_unid_to_node_id(destination_unid, &destination_node_id);

  association_t old_association = {};
  old_association.node_id       = destination_node_id;
  old_association.endpoint_id   = destination_ep;
  old_association.type
    = ((destination_ep == 0) && (endpoint == 0)) ? NODE_ID : ENDPOINT;

  binding_mapper_helper_get_association_group_list_from_cluster(
    unid,
    endpoint,
    uic_dotdot_get_cluster_id(cluster_name),
    group_id_list.data(),
    &number_of_groups);

  // Just remove, no check needed,
  for (uint8_t i = 0; i < number_of_groups; i++) {
    sl_log_debug(LOG_TAG,
                 "Removing %s association towards the NodeID %d:%d "
                 "for NodeID %d:%d Group ID %d. (Non-forceful).",
                 old_association.type == NODE_ID ? "NodeID" : "Endpoint",
                 old_association.node_id,
                 old_association.endpoint_id,
                 node_id,
                 endpoint,
                 group_id_list[i]);
    remove_desired_association(node_id,
                               endpoint,
                               group_id_list[i],
                               old_association);
  }

  return SL_STATUS_OK;
}

sl_status_t bind_to_protocol_controller_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  const char *cluster_name)
{
  // Find our UNID/Ep, and treat that as a regular binding operation.
  zwave_node_id_t my_node_id = zwave_network_management_get_node_id();
  unid_t destination_unid;
  zwave_unid_from_node_id(my_node_id, destination_unid);

  return bind_callback(unid,
                       endpoint,
                       call_type,
                       cluster_name,
                       destination_unid,
                       PROTOCOL_CONTROLLER_ENDPOINT_ID);
}

sl_status_t unbind_from_protocol_controller_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  const char *cluster_name)
{
  // Find our UNID/Ep, and treat that as a regular un-binding operation.
  zwave_node_id_t my_node_id = zwave_network_management_get_node_id();
  unid_t destination_unid;
  zwave_unid_from_node_id(my_node_id, destination_unid);

  return unbind_callback(unid,
                         endpoint,
                         call_type,
                         cluster_name,
                         destination_unid,
                         PROTOCOL_CONTROLLER_ENDPOINT_ID);
}

//////////////////////////////////////////////////////////////////////////////
//  Attribute Store callback functions.
//////////////////////////////////////////////////////////////////////////////
/**
 * @brief Verifies if something needs to be published after a node is moved
 * to Online Functional.
 *
 * @param network_status_node    Attribute Store node that was modified.
 * @param change          Type of change applied to the node.
 */
static void on_network_status_update(attribute_store_node_t network_status_node,
                                     attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  // Read the network status, do not publish if it is not online functional:
  if (get_network_status(network_status_node)
      != NODE_STATE_TOPIC_STATE_INCLUDED) {
    return;
  }

  // Go up and find the UNID/Endpoint and its network status.
  unid_t unid;
  if (SL_STATUS_OK
      != attribute_store_network_helper_get_unid_from_node(network_status_node,
                                                           unid)) {
    return;
  }

  // Go around all the endpoints and ask for a Configuration parameter attribute
  // publications:
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(network_status_node,
                                                 ATTRIBUTE_NODE_ID);

  for (attribute endpoint_id_node:
       attribute(node_id_node).children(ATTRIBUTE_ENDPOINT_ID)) {
    zwave_endpoint_id_t endpoint_id = 0;
    attribute_store_get_reported(endpoint_id_node,
                                 &endpoint_id,
                                 sizeof(endpoint_id));

    // All looks good, publish the configuration parameters and cluster revision
    publish_binding_cluster_bindable_cluster_list_attribute(std::string(unid),
                                                            endpoint_id);

    publish_binding_cluster_revision_attribute(std::string(unid), endpoint_id);
    publish_binding_table_attribute(std::string(unid), endpoint_id);
    publish_binding_table_full_attribute(std::string(unid), endpoint_id);
  }
}

/**
 * @brief Listens to updates to the Association group command list and
 * publishes the changes accordingly
 *
 * @param updated_node    Attribute Store node that was modified.
 * @param change          Type of change applied to the node.
 */
static void on_group_command_list_udpate(attribute_store_node_t updated_node,
                                         attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  // Go up and find the UNID/Endpoint and its network status.
  unid_t unid;
  zwave_endpoint_id_t endpoint_id = 0;
  if (SL_STATUS_OK
      != attribute_store_network_helper_get_unid_endpoint_from_node(
        updated_node,
        unid,
        &endpoint_id)) {
    return;
  }

  // Read the network status, do not publish if it is not online functional:
  if (get_network_status(updated_node) != NODE_STATE_TOPIC_STATE_INCLUDED) {
    return;
  }

  publish_binding_cluster_bindable_cluster_list_attribute(std::string(unid),
                                                          endpoint_id);
}

/**
 * @brief Listens to updates to the Association group content and
 * publishes the changes accordingly
 *
 * @param updated_node    Attribute Store node that was modified.
 * @param change          Type of change applied to the node.
 */
static void on_group_content_udpate(attribute_store_node_t updated_node,
                                    attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  // Go up and find the UNID/Endpoint and its network status.
  unid_t unid;
  zwave_endpoint_id_t endpoint_id = 0;
  if (SL_STATUS_OK
      != attribute_store_network_helper_get_unid_endpoint_from_node(
        updated_node,
        unid,
        &endpoint_id)) {
    return;
  }

  // Read the network status, do not publish if it is not online functional:
  if (get_network_status(updated_node) != NODE_STATE_TOPIC_STATE_INCLUDED) {
    return;
  }

  publish_binding_table_attribute(std::string(unid), endpoint_id);
  publish_binding_table_full_attribute(std::string(unid), endpoint_id);
}

///////////////////////////////////////////////////////////////////////////////
//  Init and teardown functions.
//////////////////////////////////////////////////////////////////////////////
sl_status_t binding_cluster_server_init()
{
  sl_log_debug(LOG_TAG, "Initialization of the Binding Cluster Server");

  // Register the callback for handling commands from IoT services
  uic_mqtt_dotdot_binding_bind_callback_set(&bind_callback);
  uic_mqtt_dotdot_binding_unbind_callback_set(&unbind_callback);
  uic_mqtt_dotdot_binding_bind_to_protocol_controller_callback_set(
    &bind_to_protocol_controller_callback);
  uic_mqtt_dotdot_binding_unbind_from_protocol_controller_callback_set(
    &unbind_from_protocol_controller_callback);

  // Register attribute updates
  attribute_store_register_callback_by_type_and_state(&on_network_status_update,
                                                      ATTRIBUTE_NETWORK_STATUS,
                                                      REPORTED_ATTRIBUTE);

  attribute_store_register_callback_by_type_and_state(
    &on_group_command_list_udpate,
    ATTRIBUTE(GROUP_COMMAND_LIST),
    REPORTED_ATTRIBUTE);

  attribute_store_register_callback_by_type(&on_group_content_udpate,
                                            ATTRIBUTE(GROUP_CONTENT));

  return SL_STATUS_OK;
}