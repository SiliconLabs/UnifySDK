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
#include "zwave_command_class_association_internals.hpp"
#include "zwave_command_classes_utils.h"

// ZPC Includes
#include "attribute_store_defined_attribute_types.h"

// UIC includes
#include "attribute_store_helper.h"
#include "attribute.hpp"

#include "sl_log.h"
constexpr char LOG_TAG[] = "command_class_association_internals";

// Note, Multi Channel Association shares attributes with Association
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_##type

using namespace attribute_store;

///////////////////////////////////////////////////////////////////////////////
// Private functions helper functions
///////////////////////////////////////////////////////////////////////////////
static bool is_association_valid(association_t association)
{
  if (association.node_id == 0) {
    return false;
  }
  if (association.node_id > ZW_MAX_NODES) {
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

///////////////////////////////////////////////////////////////////////////////
// Functions shared with the component
///////////////////////////////////////////////////////////////////////////////
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
    = attribute_store_get_node_child_by_type(group_id_node,
                                             ATTRIBUTE(GROUP_CONTENT),
                                             0);
  return group_content_node;
}

association_group_count_t get_number_of_groups(zwave_node_id_t node_id,
                                               zwave_endpoint_id_t endpoint_id)
{
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_id_node(node_id, endpoint_id);
  attribute_store_node_t supported_groupings_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(SUPPORTED_GROUPINGS),
                                             0);

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
    = attribute_store_get_node_child_by_type(group_id_node,
                                             ATTRIBUTE(MAX_NODES_SUPPORTED),
                                             0);

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
                                          association_t association)
{
  try {
    // Read the Reported assocation group bytestream from the attribute store
    attribute group(get_group_content_node(node_id, endpoint_id, group_id));
    association_bytes bytestream
      = group.get<association_bytes>(DESIRED_OR_REPORTED_ATTRIBUTE);

    // Convert into a set of associations
    association_set associations;
    get_association_list(bytestream, associations);

    // If we got higher than the capacity, do not abort, bump somebody out !
    if (get_group_capacity(node_id, endpoint_id, group_id)
        <= associations.size()) {
      // We just remove somebody else from the associations here.
      bool somebody_was_removed = false;
      for (auto association: associations) {
        // Do not remove ourselves.
        if (association.node_id == node_id) {
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
    // add the requested association
    associations.insert(association);

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
    // Read the Reported assocation group bytestream from the attribute store
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
