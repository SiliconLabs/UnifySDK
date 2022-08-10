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
#include "zwave_association_toolbox.hpp"
#include "zwave_utils.h"

// ZPC Components
#include "zpc_attribute_store.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "zwave_command_class_indices.h"
#include "zwave_network_management.h"
#include "binding_cluster_mapper_helper.h"

// Unify Components
#include "attribute_store_helper.h"
#include "attribute.hpp"
#include "sl_log.h"

// Attribute definitions for Association
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_##type

using namespace attribute_store;

// Constants
constexpr char LOG_TAG[] = "zwave_association_toolbox";

///////////////////////////////////////////////////////////////////////////////
// Private functions helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Indicates if the values in an association struct are a valid Z-Wave
 * assocation the list of Endpoints attributes in our network.
 *
 * @return true if valid, false if invalid
 */
static bool is_association_valid(association_t association)
{
  if (association.type != NODE_ID && association.type != ENDPOINT) {
    return false;
  }
  if (association.node_id == 0) {
    return false;
  }
  if (association.node_id > ZW_MAX_NODES) {
    return false;
  }
  if ((association.type == ENDPOINT)
      && (association.endpoint_id > ZWAVE_MAXIMUM_ENDPOINT_ID)) {
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// C++ helper functions
///////////////////////////////////////////////////////////////////////////////
void get_association_list(association_bytes source, association_set &target)
{
  association_t association = {};
  association.type          = NODE_ID;
  association.node_id       = 0;
  association.endpoint_id   = 0;
  bool endpoint_byte        = false;

  for (auto byte: source) {
    // If we get a marker, switch to the endpoint type and continue
    if (byte == ASSOCIATION_MARKER && association.type == NODE_ID) {
      association.type = ENDPOINT;
      continue;
    }

    // Without the marker, each byte is a NodeID.
    if (association.type == NODE_ID) {
      association.node_id = byte;
      if (is_association_valid(association) == true) {
        target.insert(association);
      }
    }

    // After the marker, bytes are interpreted 2-by-2.
    if (association.type == ENDPOINT) {
      if (endpoint_byte == false) {
        association.node_id = byte;
        endpoint_byte       = true;
      } else {
        association.endpoint_id = byte;
        endpoint_byte           = false;
        if (is_association_valid(association) == true) {
          target.insert(association);
        }
      }
    }
  }
}

void get_node_id_association_list(association_bytes source,
                                  association_set &target)
{
  association_t association = {};
  association.type          = NODE_ID;
  association.node_id       = 0;
  association.endpoint_id   = 0;

  for (auto byte: source) {
    // If we get a marker, we stop
    if (byte == ASSOCIATION_MARKER) {
      return;
    }
    if (association.type == NODE_ID) {
      association.node_id = byte;
      target.insert(association);
    }
  }
}

void get_association_bytestream(association_set source,
                                association_bytes &target)
{
  // Emtpy set results in a MARKER.
  if (source.size() == 0) {
    target.push_back(ASSOCIATION_MARKER);
    return;
  }

  bool marker_inserted = false;
  for (auto association: source) {
    if (association.type == NODE_ID) {
      target.push_back((uint8_t)association.node_id);
      continue;
    }
    if (association.type == ENDPOINT) {
      // Is it the first endpoint association ?
      if (marker_inserted == false) {
        target.push_back(ASSOCIATION_MARKER);
        marker_inserted = true;
      }

      target.push_back((uint8_t)association.node_id);
      target.push_back((uint8_t)association.endpoint_id);
      continue;
    }
  }
}

void get_node_id_association_bytestream(association_set source,
                                        association_bytes &target)
{
  for (auto association: source) {
    if (association.type == NODE_ID) {
      target.push_back((uint8_t)association.node_id);
    }
  }
}

void substract_association_lists(const association_set &set_1,
                                 const association_set &set_2,
                                 association_set &result)
{
  for (auto element: set_2) {
    if (set_1.count(element) == 0) {
      result.insert(element);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Remove all association containing a NodeID from an association set
 *
 * @param [in] node_id      NodeID to find and remove from all associations
 * @param [out] set         Association Set to modify.
 *
 */
static void remove_node_id_from_association_set(zwave_node_id_t node_id,
                                                association_set &set)
{
  bool search_more = false;
  for (association_t association: set) {
    if (association.node_id == node_id) {
      // Iterator invalidated
      set.erase(association);
      search_more = true;
      break;
    }
  }

  // Make recursive calls as long as we find the NodeID in the set.
  if (search_more == true) {
    remove_node_id_from_association_set(node_id, set);
  }
}

/**
 * @brief Returns the list of Endpoints attributes in our network.
 *
 * @return std::vector<attribute>
 */
static std::vector<attribute> endpoint_id_attribute_list()
{
  std::vector<attribute> endpoint_list;
  attribute network_node(get_zpc_network_node());

  for (attribute node_id: network_node.children(ATTRIBUTE_NODE_ID)) {
    for (attribute endpoint_id: node_id.children(ATTRIBUTE_ENDPOINT_ID)) {
      endpoint_list.push_back(endpoint_id);
    }
  }
  return endpoint_list;
}

/**
 * @brief Returns groups nodes for a given set of Endpoint attributes
 *
 * @return std::vector<attribute>
 */
static std::vector<attribute>
  group_content_attribute_list(std::vector<attribute> endpoint_id_nodes)
{
  std::vector<attribute> group_content_node_list;
  for (auto endpoint_node: endpoint_id_nodes) {
    for (attribute group_id_node: endpoint_node.children(ATTRIBUTE(GROUP_ID))) {
      attribute group_content_node(
        group_id_node.child_by_type(ATTRIBUTE(GROUP_CONTENT)));
      if (group_content_node.is_valid()) {
        group_content_node_list.push_back(group_content_node);
      }
    }
  }
  return group_content_node_list;
}

///////////////////////////////////////////////////////////////////////////////
// Public functions
///////////////////////////////////////////////////////////////////////////////
attribute_store_node_t get_group_child_node(zwave_node_id_t node_id,
                                            zwave_endpoint_id_t endpoint_id,
                                            association_group_id_t group_id,
                                            attribute_store_type_t type)
{
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_id_node(node_id, endpoint_id);
  attribute_store_node_t group_id_node
    = attribute_store_get_node_child_by_value(endpoint_node,
                                              ATTRIBUTE(GROUP_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&group_id,
                                              sizeof(group_id),
                                              0);
  return attribute_store_get_first_child_by_type(group_id_node, type);
}

attribute_store_node_t get_group_content_node(zwave_node_id_t node_id,
                                              zwave_endpoint_id_t endpoint_id,
                                              association_group_id_t group_id)
{
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_id_node(node_id, endpoint_id);
  attribute_store_node_t group_id_node
    = attribute_store_get_node_child_by_value(endpoint_node,
                                              ATTRIBUTE(GROUP_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&group_id,
                                              sizeof(group_id),
                                              0);
  attribute_store_node_t group_content_node
    = attribute_store_get_first_child_by_type(group_id_node,
                                              ATTRIBUTE(GROUP_CONTENT));
  return group_content_node;
}

bool is_association_legal(zwave_node_id_t node_id,
                          zwave_endpoint_id_t endpoint_id,
                          association_group_id_t group_id,
                          association_t association)
{
  // First off, it's always legal to establish towards ourselves:
  if (association.node_id == zwave_network_management_get_node_id()) {
    return is_association_valid(association);
  } else if (association.node_id == node_id) {
    sl_log_debug(LOG_TAG,
                 "Not allowing to associate NodeID %d towards itself",
                 node_id);
    return false;
  }

  // If the destination is an NL node, it's a no go
  if (zwave_get_operating_mode(association.node_id) == OPERATING_MODE_NL) {
    sl_log_debug(LOG_TAG,
                 "Not allowing to associate towards an NL node (NodeID %d)",
                 association.node_id);
    return false;
  }

  // If either the source or destination don't operate on the same protocol, it's a no go
  // This check will also prevent to associate to non-existing nodes.
  if (zwave_get_inclusion_protocol(association.node_id) != PROTOCOL_ZWAVE
      || zwave_get_inclusion_protocol(node_id) != PROTOCOL_ZWAVE) {
    sl_log_debug(LOG_TAG,
                 "Not allowing to associate because not both nodes operate on "
                 "Z-Wave (Source NodeID %d, Destination NodeID %d)",
                 node_id,
                 association.node_id);
    return false;
  }

  if (!endpoint_exists(association.node_id, association.endpoint_id)) {
    sl_log_debug(LOG_TAG,
                 "Not allowing to associate because endpoint %d does not "
                 "exist for the destination NodeID %d)",
                 association.endpoint_id,
                 association.node_id);
    return false;
  }

  // Security level time.
  // The Source NodeID MUST be granted the highest key of the destination NodeID
  // If they both have the same highest key, easy:
  zwave_keyset_t source_granted_keys = 0;
  zwave_get_node_granted_keys(node_id, &source_granted_keys);
  zwave_controller_encapsulation_scheme_t source_highest_scheme
    = zwave_controller_get_highest_encapsulation(source_granted_keys);
  zwave_keyset_t destination_granted_keys = 0;
  zwave_get_node_granted_keys(association.node_id, &destination_granted_keys);
  zwave_controller_encapsulation_scheme_t destination_highest_scheme
    = zwave_controller_get_highest_encapsulation(destination_granted_keys);
  if (source_highest_scheme != destination_highest_scheme) {
    // Here in theory, we can check if source NodeID has security discovery
    // capabilities (association v3 or mc association v4)
    sl_log_debug(LOG_TAG,
                 "Warning: Not verifying for security discovery capabilities "
                 "(Source NodeID %d, Destination NodeID %d). Association "
                 "is not considered legal and will not be established.",
                 node_id,
                 association.node_id);
    return false;
  }

  // Check multi channel encapsulation compatibility
  // For an endpoint association, we need the source to support Multi Channel association
  // and the destination to support Multi Channel.
  if (association.type == ENDPOINT
      && ((!zwave_node_supports_command_class(COMMAND_CLASS_MULTI_CHANNEL_V4,
                                              association.node_id,
                                              0))
          || (!zwave_node_supports_command_class(
            COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
            node_id,
            endpoint_id)))) {
    sl_log_debug(LOG_TAG,
                 "Not allowing to associate due to lack of Multi Channel "
                 "capabilities (Source NodeID %d, Destination NodeID %d)",
                 node_id,
                 association.node_id);
    return false;
  }

  // Now check that the destination supports the commands sent by the source.
  attribute_store_node_t group_command_list_node
    = get_group_child_node(node_id,
                           endpoint_id,
                           group_id,
                           ATTRIBUTE(GROUP_COMMAND_LIST));
  uint8_t command_list[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  uint8_t command_list_length                                = 0;
  attribute_store_get_node_attribute_value(group_command_list_node,
                                           REPORTED_ATTRIBUTE,
                                           command_list,
                                           &command_list_length);
  bool at_least_one_mapped_command = false;
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
    // If it does not match, we assume
    if (is_command_class_and_command_mapped_to_bindings(command_class,
                                                        command)) {
      at_least_one_mapped_command = true;
      // Here it's kind of tricky to check for commands that got introduced
      // after a certain version.
      // Assume Basic is suppported by anybody.
      if ((command_class != COMMAND_CLASS_BASIC)
          && !zwave_node_supports_command_class(command_class,
                                                association.node_id,
                                                association.endpoint_id)) {
        return false;
      }
    }
  }

  return at_least_one_mapped_command;
}

association_group_count_t get_number_of_groups(zwave_node_id_t node_id,
                                               zwave_endpoint_id_t endpoint_id)
{
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_id_node(node_id, endpoint_id);
  attribute_store_node_t supported_groupings_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(SUPPORTED_GROUPINGS));

  association_group_count_t number_of_groups = 0;
  attribute_store_get_reported(supported_groupings_node,
                               &number_of_groups,
                               sizeof(number_of_groups));
  return number_of_groups;
}

association_group_capacity_t get_group_capacity(zwave_node_id_t node_id,
                                                zwave_endpoint_id_t endpoint_id,
                                                association_group_id_t group_id)
{
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_id_node(node_id, endpoint_id);
  attribute_store_node_t group_id_node
    = attribute_store_get_node_child_by_value(endpoint_node,
                                              ATTRIBUTE(GROUP_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&group_id,
                                              sizeof(group_id),
                                              0);
  attribute_store_node_t group_capacity_node
    = attribute_store_get_first_child_by_type(group_id_node,
                                              ATTRIBUTE(MAX_NODES_SUPPORTED));

  association_group_capacity_t group_capacity = 0;
  attribute_store_get_reported(group_capacity_node,
                               &group_capacity,
                               sizeof(group_capacity));
  return group_capacity;
}

sl_status_t add_desired_association(zwave_node_id_t node_id,
                                    zwave_endpoint_id_t endpoint_id,
                                    association_group_id_t group_id,
                                    association_t association)
{
  try {
    // Read the Desired assocation group bytestream from the attribute store
    attribute group(get_group_content_node(node_id, endpoint_id, group_id));
    association_bytes bytestream
      = group.get<association_bytes>(DESIRED_OR_REPORTED_ATTRIBUTE);

    // Convert into a set of associations
    association_set associations;
    get_association_list(bytestream, associations);
    // add the requested association
    associations.insert(association);

    // If we got higher than the capacity, abort
    if (get_group_capacity(node_id, endpoint_id, group_id)
        < associations.size()) {
      return SL_STATUS_FAIL;
    }

    // Put back into a bytestream:
    bytestream.clear();
    get_association_bytestream(associations, bytestream);

    // Write to the attribute store:
    group.set<association_bytes>(DESIRED_ATTRIBUTE, bytestream);
    return SL_STATUS_OK;

  } catch (const std::exception &err) {
    sl_log_warning(LOG_TAG,
                   "Failed to Add desired association: %s\n",
                   err.what());
    return SL_STATUS_FAIL;
  }
}

sl_status_t add_reported_association(zwave_node_id_t node_id,
                                     zwave_endpoint_id_t endpoint_id,
                                     association_group_id_t group_id,
                                     association_t association)
{
  try {
    // Read the Reported assocation group bytestream from the attribute store
    attribute group(get_group_content_node(node_id, endpoint_id, group_id));
    association_bytes bytestream
      = group.get<association_bytes>(REPORTED_ATTRIBUTE);

    // Convert into a set of associations
    association_set associations;
    get_association_list(bytestream, associations);
    // add the requested association
    associations.insert(association);

    // If we got higher than the capacity, abort
    if (get_group_capacity(node_id, endpoint_id, group_id)
        < associations.size()) {
      return SL_STATUS_FAIL;
    }

    // Put back into a bytestream:
    bytestream.clear();
    get_association_bytestream(associations, bytestream);

    // Write to the attribute store:
    group.set<association_bytes>(REPORTED_ATTRIBUTE, bytestream);
    return SL_STATUS_OK;

  } catch (const std::exception &err) {
    sl_log_warning(LOG_TAG,
                   "Failed to Add Reported association: %s\n",
                   err.what());
    return SL_STATUS_FAIL;
  }
}

sl_status_t force_add_desired_association(zwave_node_id_t node_id,
                                          zwave_endpoint_id_t endpoint_id,
                                          association_group_id_t group_id,
                                          association_t new_association)
{
  try {
    // Read the Reported assocation group bytestream from the attribute store
    attribute group(get_group_content_node(node_id, endpoint_id, group_id));
    association_bytes bytestream
      = group.get<association_bytes>(DESIRED_OR_REPORTED_ATTRIBUTE);

    // Convert into a set of associations
    association_set associations;
    get_association_list(bytestream, associations);

    // add the requested association, before the size check.
    associations.insert(new_association);

    // If we got higher than the capacity, do not abort, try to bump somebody out !
    if (get_group_capacity(node_id, endpoint_id, group_id)
        < associations.size()) {
      // We just remove somebody else from the associations here.
      bool somebody_was_removed = false;
      for (auto association: associations) {
        // Do not remove the association that we just added.
        if ((association.node_id == new_association.node_id)
            && (association.type == new_association.type)
            && (association.endpoint_id == new_association.endpoint_id)) {
          continue;
        }
        associations.erase(association);
        somebody_was_removed = true;
        break;
      }

      if (somebody_was_removed == false) {
        // At that point, we we are still above capacity, it means that we have only
        // associations to ourselves in that group... Abort now
        return SL_STATUS_FAIL;
      }
    }

    // Put back into a bytestream:
    bytestream.clear();
    get_association_bytestream(associations, bytestream);

    // Write to the attribute store:
    group.set<association_bytes>(DESIRED_ATTRIBUTE, bytestream);
    return SL_STATUS_OK;

  } catch (const std::exception &err) {
    sl_log_warning(LOG_TAG,
                   "Failed to Add Desired association: %s\n",
                   err.what());
    return SL_STATUS_FAIL;
  }
}

sl_status_t remove_desired_association(zwave_node_id_t node_id,
                                       zwave_endpoint_id_t endpoint_id,
                                       association_group_id_t group_id,
                                       association_t association)
{
  try {
    // Read the Desired assocation group bytestream from the attribute store
    attribute group(get_group_content_node(node_id, endpoint_id, group_id));
    association_bytes bytestream
      = group.get<association_bytes>(DESIRED_OR_REPORTED_ATTRIBUTE);

    // Convert into a set of associations
    association_set associations;
    get_association_list(bytestream, associations);
    // remove the requested association, if present
    associations.erase(association);

    // Put back into a bytestream:
    bytestream.clear();
    get_association_bytestream(associations, bytestream);

    // Write to the attribute store:
    group.set<association_bytes>(DESIRED_ATTRIBUTE, bytestream);
    return SL_STATUS_OK;

  } catch (const std::exception &err) {
    sl_log_warning(LOG_TAG,
                   "Failed to Remove desired association: %s\n",
                   err.what());
    return SL_STATUS_FAIL;
  }
}

sl_status_t remove_reported_association(zwave_node_id_t node_id,
                                        zwave_endpoint_id_t endpoint_id,
                                        association_group_id_t group_id,
                                        association_t association)
{
  try {
    // Read the Reported assocation group bytestream from the attribute store
    attribute group(get_group_content_node(node_id, endpoint_id, group_id));
    association_bytes bytestream
      = group.get<association_bytes>(REPORTED_ATTRIBUTE);

    // Convert into a set of associations
    association_set associations;
    get_association_list(bytestream, associations);
    // remove the requested association, if present
    associations.erase(association);

    // Put back into a bytestream:
    bytestream.clear();
    get_association_bytestream(associations, bytestream);

    // Write to the attribute store:
    group.set<association_bytes>(REPORTED_ATTRIBUTE, bytestream);
    return SL_STATUS_OK;

  } catch (const std::exception &err) {
    sl_log_warning(LOG_TAG,
                   "Failed to Remove reported association: %s\n",
                   err.what());
    return SL_STATUS_FAIL;
  }
}

sl_status_t remove_all_desired_associations(zwave_node_id_t node_id,
                                            zwave_endpoint_id_t endpoint_id,
                                            association_group_id_t group_id)
{
  try {
    attribute group(get_group_content_node(node_id, endpoint_id, group_id));
    association_bytes bytestream;
    association_set associations;
    get_association_bytestream(associations, bytestream);
    // Write to the attribute store:
    group.set<association_bytes>(DESIRED_ATTRIBUTE, bytestream);
    return SL_STATUS_OK;
  } catch (const std::exception &err) {
    sl_log_warning(LOG_TAG,
                   "Failed to Remove All Desired associations: %s\n",
                   err.what());
    return SL_STATUS_FAIL;
  }
}

sl_status_t remove_all_reported_associations(zwave_node_id_t node_id,
                                             zwave_endpoint_id_t endpoint_id,
                                             association_group_id_t group_id)
{
  try {
    attribute group(get_group_content_node(node_id, endpoint_id, group_id));
    association_bytes bytestream;
    association_set associations;
    get_association_bytestream(associations, bytestream);
    // Write to the attribute store:
    group.set<association_bytes>(REPORTED_ATTRIBUTE, bytestream);
    return SL_STATUS_OK;
  } catch (const std::exception &err) {
    sl_log_warning(LOG_TAG,
                   "Failed to Remove All Reported associations: %s\n",
                   err.what());
    return SL_STATUS_FAIL;
  }
}

sl_status_t
  remove_all_reported_node_id_associations(zwave_node_id_t node_id,
                                           zwave_endpoint_id_t endpoint_id,
                                           association_group_id_t group_id)
{
  try {
    // Read the Reported assocation group bytestream from the attribute store
    attribute group(get_group_content_node(node_id, endpoint_id, group_id));
    association_bytes bytestream
      = group.get<association_bytes>(REPORTED_ATTRIBUTE);

    // Convert into a set of associations
    association_set associations;
    get_association_list(bytestream, associations);
    // Find all NodeID associations in here.
    std::vector<association_t> associations_to_remove;
    for (auto association: associations) {
      if (association.type == NODE_ID) {
        associations_to_remove.push_back(association);
      }
    }
    // Then remove then.
    for (auto association: associations_to_remove) {
      associations.erase(association);
    }

    // Put back into a bytestream:
    bytestream.clear();
    get_association_bytestream(associations, bytestream);

    // Write to the attribute store:
    group.set<association_bytes>(REPORTED_ATTRIBUTE, bytestream);
    return SL_STATUS_OK;

  } catch (const std::exception &err) {
    sl_log_warning(LOG_TAG,
                   "Failed to Remove all NodeID reported associations: %s\n",
                   err.what());
    return SL_STATUS_FAIL;
  }
}

bool is_reported_group_full(zwave_node_id_t node_id,
                            zwave_endpoint_id_t endpoint_id,
                            association_group_id_t group_id)
{
  try {
    // Read the Reported assocation group bytestream from the attribute store
    attribute group(get_group_content_node(node_id, endpoint_id, group_id));
    association_bytes bytestream
      = group.get<association_bytes>(REPORTED_ATTRIBUTE);

    // Convert into a set of associations
    association_set associations;
    get_association_list(bytestream, associations);

    // Did we exeed the limit?
    if (associations.size()
        >= get_group_capacity(node_id, endpoint_id, group_id)) {
      return true;
    } else {
      return false;
    }

  } catch (const std::exception &err) {
    sl_log_warning(
      LOG_TAG,
      "Failed to evaluate if reported association group is full: %s\n",
      err.what());
    return false;
  }
}

bool is_desired_group_full(zwave_node_id_t node_id,
                           zwave_endpoint_id_t endpoint_id,
                           association_group_id_t group_id)
{
  try {
    // Read the desired assocation group bytestream from the attribute store
    attribute group(get_group_content_node(node_id, endpoint_id, group_id));
    association_bytes bytestream
      = group.get<association_bytes>(DESIRED_ATTRIBUTE);

    // Convert into a set of associations
    association_set associations;
    get_association_list(bytestream, associations);

    // Did we exeed the limit?
    if (associations.size()
        >= get_group_capacity(node_id, endpoint_id, group_id)) {
      return true;
    } else {
      return false;
    }

  } catch (const std::exception &err) {
    sl_log_warning(
      LOG_TAG,
      "Failed to evaluate if reported association group is full: %s\n",
      err.what());
    return false;
  }
}

bool is_association_in_group(zwave_node_id_t node_id,
                             zwave_endpoint_id_t endpoint_id,
                             association_group_id_t group_id,
                             association_t association)
{
  try {
    // Read the reported assocation group bytestream from the attribute store
    attribute group(get_group_content_node(node_id, endpoint_id, group_id));
    association_bytes bytestream
      = group.get<association_bytes>(DESIRED_OR_REPORTED_ATTRIBUTE);

    // Convert into a set of associations
    association_set associations;
    get_association_list(bytestream, associations);
    return associations.count(association);

  } catch (const std::exception &err) {
    sl_log_warning(LOG_TAG,
                   "Failed to evaluate if an association is in a group: %s\n",
                   err.what());
    return false;
  }
}

void remove_desired_node_id_from_all_associations_in_network(
  zwave_node_id_t node_id)
{
  for (auto group_content_node:
       group_content_attribute_list(endpoint_id_attribute_list())) {
    try {
      // Read the Desired assocation group bytestream from the attribute store
      association_bytes bytestream = group_content_node.get<association_bytes>(
        DESIRED_OR_REPORTED_ATTRIBUTE);

      // Convert into a set of associations
      association_set associations;
      get_association_list(bytestream, associations);

      // Eradicate the presence of the NodeID in this group
      remove_node_id_from_association_set(node_id, associations);

      //Convert back to Bytestream
      bytestream.clear();
      get_association_bytestream(associations, bytestream);

      // Write to the attribute store:
      group_content_node.set<association_bytes>(DESIRED_ATTRIBUTE, bytestream);

    } catch (const std::exception &err) {
      sl_log_warning(LOG_TAG,
                     "Failed to update association group content: %s\n",
                     err.what());
    }
  }
}

void add_desired_association_in_all_groups(zwave_node_id_t node_id,
                                           zwave_endpoint_id_t endpoint_id,
                                           association_t association)
{
  association_group_count_t group_count
    = get_number_of_groups(node_id, endpoint_id);

  for (association_group_id_t current_group_id = 1;
       current_group_id <= group_count;
       current_group_id++) {
    add_desired_association(node_id,
                            endpoint_id,
                            current_group_id,
                            association);
  }
}

void remove_desired_association_in_all_groups(zwave_node_id_t node_id,
                                              zwave_endpoint_id_t endpoint_id,
                                              association_t association)
{
  association_group_count_t group_count
    = get_number_of_groups(node_id, endpoint_id);

  for (association_group_id_t current_group_id = 1;
       current_group_id <= group_count;
       current_group_id++) {
    remove_desired_association(node_id,
                               endpoint_id,
                               current_group_id,
                               association);
  }
}

void remove_desired_node_id_from_all_associations(
  zwave_node_id_t node_id_to_remove,
  zwave_node_id_t node_id,
  zwave_endpoint_id_t endpoint_id)
{
  association_group_count_t group_count
    = get_number_of_groups(node_id, endpoint_id);

  for (association_group_id_t current_group_id = 1;
       current_group_id <= group_count;
       current_group_id++) {
    try {
      attribute group(
        get_group_content_node(node_id, endpoint_id, current_group_id));
      association_bytes bytestream
        = group.get<association_bytes>(DESIRED_OR_REPORTED_ATTRIBUTE);

      // Convert into a set of associations
      association_set associations;
      get_association_list(bytestream, associations);

      // Eradicate the presence of the NodeID in this group
      remove_node_id_from_association_set(node_id_to_remove, associations);

      // Convert back to Bytestream
      bytestream.clear();
      get_association_bytestream(associations, bytestream);

      // Write to the attribute store:
      group.set<association_bytes>(DESIRED_ATTRIBUTE, bytestream);

    } catch (const std::exception &err) {
      sl_log_warning(LOG_TAG,
                     "Failed to update association group content: %s\n",
                     err.what());
    }
  }
}
