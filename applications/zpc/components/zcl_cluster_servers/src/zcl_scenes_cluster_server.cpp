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
#include "zcl_scenes_cluster_server.h"
#include "zcl_cluster_servers_helpers.hpp"

// ZPC Components
#include "attribute_store_defined_attribute_types.h"
#include "zpc_attribute_store_network_helper.h"

// Unify components
#include "unify_dotdot_defined_attribute_types.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_timeouts.h"
#include "attribute.hpp"
#include "attribute_mapper.h"
#include "uic_mqtt.h"
#include "sl_log.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_attribute_store_group_cluster.h"
#include "dotdot_attribute_id_definitions.h"
#include "dotdot_mqtt_parsing_helpers.hpp"

// Includes from auto-generated files
#include "dotdot_cluster_id_definitions.h"
#include "dotdot_mqtt.h"
#include "dotdot_mqtt_helpers.hpp"
#include "dotdot_cluster_id_definitions.h"

// Generic includes
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <boost/algorithm/string.hpp>

using namespace attribute_store;

constexpr char LOG_TAG[]             = "zcl_scenes_cluster_server";
constexpr char EMPTY_STRING[]        = "";
constexpr char SCENES_CLUSTER_NAME[] = "Scenes";

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) DOTDOT_ATTRIBUTE_ID_SCENES_##type

// List of attributes that if supported, will trigger that we provide the
// Scene Cluster server functionality for that node.
// See the sceneRequired=true in the XML files
static constexpr attribute_store_type_t scene_actuator_attribute_list[]
  = {DOTDOT_ATTRIBUTE_ID_BARRIER_CONTROL_BARRIER_POSITION,
     // This is normaly not in the scenes but it makes little sense
     // to have saturation without HUE
     DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE,
     DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION,
     DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENTX,
     DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENTY,
     // Same here, we kind of need to save the color mode else
     // we don't know if Temperature/color should be applied
     DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_ENHANCED_COLOR_MODE,
     DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_TEMPERATURE_MIREDS,
     DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_ENHANCED_CURRENT_HUE,
     DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_LOOP_ACTIVE,
     DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_LOOP_DIRECTION,
     DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_LOOP_TIME,
     DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
     DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_FREQUENCY,
     DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
     DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OCCUPIED_COOLING_SETPOINT,
     DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OCCUPIED_HEATING_SETPOINT,
     DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_CURRENT_POSITION_LIFT_PERCENTAGE,
     DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_CURRENT_POSITION_TILT_PERCENTAGE};

// List of attributes for each configuration parameter
// Whenever any of them is updated, we re-publish the scene table.
static constexpr attribute_store_type_t scene_table_attributes[]
  = {ATTRIBUTE(SCENE_TABLE),
     ATTRIBUTE(SCENE_ID_GROUP_ID),
     ATTRIBUTE(SCENE_NAME),
     ATTRIBUTE(SCENE_FIELD_SETS),
     ATTRIBUTE(SCENE_TRANSITION_TIME),
     ATTRIBUTE(SCENE_TRANSITION_TIME_100MS)};

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Updates the value of the Scene Count attribute based on the
 * Number of scenes present in the Scene Table.
 *
 * @param scene_table_node    Attribute Store Node for the Scene Table.
 */
static void set_scene_count(attribute_store_node_t scene_table_node)
{
  uint8_t scene_count = (uint8_t)attribute_store_get_node_child_count_by_type(
    scene_table_node,
    ATTRIBUTE(SCENE_ID_GROUP_ID));

  attribute_store_node_t endpoint_id_node
    = attribute_store_get_first_parent_with_type(scene_table_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(SCENE_COUNT),
                                     &scene_count,
                                     sizeof(scene_count));
}

/**
 * @brief Helper function retrieving the Scene Table Attribute Store node based
 * on UNID/Endpoint
 *
 * @param unid          Node's UNID
 * @param endpoint      Endpoint ID under which the Scene table is searched for.
 * @return attribute_store_node_t for the Scene Table node, ATTRIBUTE_STORE_INVALID_NODE if it does not exist.
 */
static attribute_store_node_t
  get_scene_table_node(dotdot_unid_t unid, dotdot_endpoint_id_t endpoint)
{
  attribute_store_node_t endpoint_id_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);
  return attribute_store_get_first_child_by_type(endpoint_id_node,
                                                 ATTRIBUTE(SCENE_TABLE));
}

/**
 * @brief Checks if an attribute type is supported by an endpoint.
 *
 * @param attribute_type Attribute type to search for under the endpoint id.
 * @param scene_extension_fieldset_node Attribute Store node under the endpoint ID node
 *
 * @return true if the attribute type exists under the endpoint node
 * @return false otherwise
 */
static bool
  attribute_type_is_supported_by_endpoint(attribute_store_type_t attribute_type,
                                          attribute_store_node_t node)
{
  attribute_store_node_t endpoint_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  return (ATTRIBUTE_STORE_INVALID_NODE
          != attribute_store_get_first_child_by_type(endpoint_id_node,
                                                     attribute_type));
}

/**
 * @brief Checks if an attribute type is part of the types allowed to go in scenes.
 *
 * @param attribute_type Attribute Store type to check.
 * @return true If the attribute type can be used in scenes.
 * @return false otherwise.
 */
static bool
  attribute_type_is_part_of_scenes(attribute_store_type_t attribute_type)
{
  for (size_t i = 0; i < COUNT_OF(scene_actuator_attribute_list); i++) {
    if (scene_actuator_attribute_list[i] == attribute_type) {
      return true;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
// JSON helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Get the extension Fills up the JSON payload from a Extension Fieldset
 * Attribute
 *
 * @param scene_extension_fieldset_node Attribute store node for the extension
 *                                      fieldset
 * @return nlohmann::json object with the data.
 */
static nlohmann::json get_extension_fieldset_json(
  attribute_store_node_t scene_extension_fieldset_node)
{
  nlohmann::json json_data = nlohmann::json::object();

  // For each ZCL Attribute under the Scene Table.
  for (size_t index = 0; index < attribute_store_get_node_child_count(
                           scene_extension_fieldset_node);
       index++) {
    attribute_store_node_t attribute_node
      = attribute_store_get_node_child(scene_extension_fieldset_node, index);

    if (false == attribute_store_is_reported_defined(attribute_node)) {
      continue;
    }

    dotdot_cluster_id_t cluster_id
      = (attribute_store_get_node_type(attribute_node) >> 16) & 0xFFFF;
    dotdot_attribute_id_t attribute_id
      = attribute_store_get_node_type(attribute_node) & 0xFFFF;

    std::string cluster_name = uic_dotdot_get_cluster_name(cluster_id);
    if (json_data.find(cluster_name.c_str()) == json_data.end()) {
      json_data[cluster_name.c_str()] = nlohmann::json::object();
    }

    std::string attribute_name
      = uic_dotdot_get_attribute_name(cluster_id, attribute_id);

    switch (uic_dotdot_get_attribute_id_json_type(cluster_id, attribute_id)) {
      case JSON_TYPE_NUMBER:
        if (uic_dotdot_attribute_is_enum(cluster_id, attribute_id)) {
          json_data[cluster_name.c_str()][attribute_name.c_str()]
            = get_enum_value_name(
              cluster_id,
              attribute_id,
              static_cast<int>(
                attribute_store_get_reported_number(attribute_node)));
        } else {
          json_data[cluster_name.c_str()][attribute_name.c_str()]
            = static_cast<int>(
              attribute_store_get_reported_number(attribute_node));
        }
        break;
      case JSON_TYPE_BOOL:
        json_data[cluster_name.c_str()][attribute_name.c_str()]
          = static_cast<bool>(
            attribute_store_get_reported_number(attribute_node));
        break;
      case JSON_TYPE_STRING: {
        char attribute_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
        attribute_store_get_reported_string(
          attribute_node,
          attribute_value,
          ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
        json_data[cluster_name.c_str()][attribute_name.c_str()]
          = attribute_value;
      } break;
      case JSON_TYPE_OBJECT:
        sl_log_warning(LOG_TAG,
                       "Publishing objects in the Scene Table not "
                       "implemented. Returning empty object.");
        json_data[cluster_name.c_str()][attribute_name.c_str()]
          = nlohmann::json::object();
        break;
      case JSON_TYPE_ARRAY:
        sl_log_warning(LOG_TAG,
                       "Publishing arrays in the Scene Table not "
                       "implemented. Returning empty array.");
        json_data[cluster_name.c_str()][attribute_name.c_str()]
          = nlohmann::json::array();
        break;
      default:
        sl_log_warning(LOG_TAG,
                       "Unkonwn JSON type for Cluster ID %d Attribute ID %d. "
                       "Writing NULL in the payload.",
                       cluster_id,
                       attribute_id);
        json_data[cluster_name.c_str()][attribute_name.c_str()] = nullptr;
    }
  }
  return json_data;
}

/**
 * @brief Saves the Scene extension fieldset from a JSON object in the
 * Attribute Store
 *
 * @param scene_extension_fieldset_node Attribute Store Node for the Scene Extension Fieldset
 * @param json_extension_field_set      JSON object from the AddScene Command
 */
static void save_extension_fieldset_to_attribute_store(
  attribute_store_node_t scene_extension_fieldset_node,
  nlohmann::json &json_extension_field_set)
{
  try {
    for (auto &cluster_object: json_extension_field_set.items()) {
      std::string cluster_name = cluster_object.key();
      dotdot_cluster_id_t cluster_id
        = uic_dotdot_get_cluster_id(cluster_name.c_str());

      // Take the value of the JSON object {}.
      nlohmann::json attribute_objects = cluster_object.value();

      for (auto &attribute_object: attribute_objects.items()) {
        std::string attribute_name = attribute_object.key();
        dotdot_attribute_id_t attribute_id
          = uic_dotdot_get_attribute_id(cluster_id, attribute_name.c_str());

        if (DOTDOT_INVALID_ATTRIBUTE_ID == attribute_id) {
          sl_log_debug(LOG_TAG,
                       "Unknown Cluster/attribute "
                       "combination : %s/%s. Ignoring",
                       cluster_name.c_str(),
                       attribute_name.c_str());
          continue;
        }

        attribute_store_type_t attribute_type
          = ((cluster_id << 16) | attribute_id);
        if (false
              == attribute_type_is_supported_by_endpoint(
                attribute_type,
                scene_extension_fieldset_node)
            || false == attribute_type_is_part_of_scenes(attribute_type)) {
          sl_log_debug(LOG_TAG,
                       "Attribute Type %d either not supported or not allowed "
                       "for Scenes. Ignoring.",
                       attribute_type);
          continue;
        }

        // Passed all the checks, let's save this in the Attribute Store
        attribute_mapper_pause_mapping();
        attribute_store_node_t attribute_node
          = attribute_store_create_child_if_missing(
            scene_extension_fieldset_node,
            attribute_type);
        attribute_mapper_resume_mapping();
        // This attribute config should never trigger evaluations
        attribute_mapper_pause_reactions_to_attribute_updates(attribute_node);

        switch (
          uic_dotdot_get_attribute_id_json_type(cluster_id, attribute_id)) {
          case JSON_TYPE_NUMBER:
          case JSON_TYPE_BOOL: {
            if (nlohmann::json::value_t::string
                == attribute_object.value().type()) {
              std::string json_value_name = attribute_object.value();
              number_t json_value         = get_enum_name_value(cluster_id,
                                                        attribute_id,
                                                        json_value_name);
              attribute_store_set_reported_number(attribute_node, json_value);
            } else if (nlohmann::json::value_t::boolean
                       == attribute_object.value().type()) {
              bool json_value = attribute_object.value();
              attribute_store_set_reported_number(attribute_node,
                                                  (number_t)json_value);
            } else {
              // Try as a number:
              number_t json_value = attribute_object.value();
              attribute_store_set_reported_number(attribute_node, json_value);
            }
          } break;
          case JSON_TYPE_STRING: {
            std::string json_value = attribute_object.value();
            attribute_store_set_reported_string(attribute_node,
                                                json_value.c_str());
          } break;
          case JSON_TYPE_OBJECT:
            sl_log_warning(LOG_TAG,
                           "Saving objects in the Scene Table not "
                           "implemented. Ignoring.");
            break;
          case JSON_TYPE_ARRAY:
            sl_log_warning(LOG_TAG,
                           "Saving arrays in the Scene Table not "
                           "implemented. Ignoring.");
            break;
          default:
            sl_log_warning(LOG_TAG,
                           "Unknown JSON type for the Scene Table for "
                           "Cluster %s Attribute %s. Ignoring.",
                           cluster_name.c_str(),
                           attribute_name.c_str());
        }
        // Align both Desired and reported, so that we can copy the Desired
        // Value into ZCL attributes when activating the scene
        attribute_store_set_desired_as_reported(attribute_node);
      }
    }
  } catch (const nlohmann::json::exception &e) {
    // Catch JSON object field parsing errors
    sl_log_debug(LOG_TAG,
                 "Error parsing Extension Field Set from "
                 "AddScene Command: %s. Ignoring",
                 e.what());
    return;
  } catch (const std::exception &e) {
    sl_log_debug(LOG_TAG,
                 "Error parsing Extension Field Set from "
                 "AddScene Command. Ignoring");
    return;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Publications functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Publishes Scene Table state to MQTT
 *
 * @param scene_table_node   The Attribute ID for the Scene Table to publish
 */
static void publish_scene_table(attribute_store_node_t scene_table_node)
{
  if (false == attribute_store_node_exists(scene_table_node)) {
    return;
  }

  sl_log_debug(LOG_TAG,
               "Publishing the Scene Table for Attribute ID %d",
               scene_table_node);
  // Read the network status, do not publish if it is not online functional:
  if (unify_attribute_store_node_state_get_status(scene_table_node) != ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL) {
    sl_log_debug(LOG_TAG,
                 "Not online functional for Attribute ID %d",
                 scene_table_node);
    return;
  }
  attribute_store_node_t endpoint_id_node
    = attribute_store_get_first_parent_with_type(scene_table_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  // Retrieve node's addresses.
  std::string unid;
  zwave_endpoint_id_t endpoint_id = 0;
  if (SL_STATUS_OK
      != zcl_cluster_servers_get_unid_endpoint(endpoint_id_node,
                                               endpoint_id,
                                               unid)) {
    sl_log_debug(LOG_TAG,
                 "Cannot retrieve UNID/Endpoint from Attribute Store Node %d, "
                 "no Scene Table publications will be made",
                 scene_table_node);
    return;
  }
  // Prepare desired/reported strings.
  std::string reported_topic = "ucl/by-unid/" + std::string(unid) + "/ep"
                               + std::to_string(endpoint_id)
                               + "/Scenes/Attributes/SceneTable/Reported";
  std::string desired_topic = "ucl/by-unid/" + std::string(unid) + "/ep"
                              + std::to_string(endpoint_id)
                              + "/Scenes/Attributes/SceneTable/Desired";

  // Create a SceneTable JSON object.
  nlohmann::json json_payload;
  json_payload["value"] = nlohmann::json::array();

  for (size_t index = 0; index < attribute_store_get_node_child_count_by_type(
                           scene_table_node,
                           ATTRIBUTE(SCENE_ID_GROUP_ID));
       index++) {
    nlohmann::json json_scene = nlohmann::json::object();

    scene_id_group_id_t scene_id_group_id = {};
    attribute_store_node_t scene_id_node
      = attribute_store_get_node_child_by_type(scene_table_node,
                                               ATTRIBUTE(SCENE_ID_GROUP_ID),
                                               index);
    attribute_store_get_reported(scene_id_node,
                                 &scene_id_group_id,
                                 sizeof(scene_id_group_id));
    json_scene["SceneID"] = scene_id_group_id.scene_id;
    json_scene["GroupID"] = scene_id_group_id.group_id;

    attribute_store_node_t scene_name_node
      = attribute_store_get_first_child_by_type(scene_id_node,
                                                ATTRIBUTE(SCENE_NAME));
    char scene_name[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
    attribute_store_get_reported_string(scene_name_node,
                                        scene_name,
                                        ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
    json_scene["SceneName"] = scene_name;

    uint8_t transition_time_100ms = 0;
    attribute_store_get_child_reported(scene_id_node,
                                       ATTRIBUTE(SCENE_TRANSITION_TIME_100MS),
                                       &transition_time_100ms,
                                       sizeof(transition_time_100ms));
    json_scene["TransitionTime100ms"] = transition_time_100ms;

    uint16_t transition_time = 0;
    attribute_store_get_child_reported(scene_id_node,
                                       ATTRIBUTE(SCENE_TRANSITION_TIME),
                                       &transition_time,
                                       sizeof(transition_time));
    json_scene["TransitionTime"] = transition_time;

    attribute_store_node_t scene_extension_fieldset_node
      = attribute_store_get_first_child_by_type(scene_id_node,
                                                ATTRIBUTE(SCENE_FIELD_SETS));

    json_scene["SceneTableExtensions"]
      = get_extension_fieldset_json(scene_extension_fieldset_node);
    json_payload["value"].push_back(json_scene);
  }

  std::string payload
    = json_payload.dump(-1,
                        ' ',
                        false,
                        nlohmann::detail::error_handler_t::replace);
  // Publish the SceneTable. Desired values first.
  uic_mqtt_publish(desired_topic.c_str(),
                   payload.c_str(),
                   payload.size(),
                   true);
  uic_mqtt_publish(reported_topic.c_str(),
                   payload.c_str(),
                   payload.size(),
                   true);

  // Make sure the Scene Count is still accurate.
  set_scene_count(scene_table_node);
  return;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Store callback functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Listens on updates/creations of scene compatible actuator attributes
 * and creates a Scene table if missing
 *
 * @param updated_node   The node that was updated
 * @param change         The type of change applied to the node.
 */
static void
  on_scene_actuator_attribute_updated(attribute_store_node_t updated_node,
                                      attribute_store_change_t change)
{
  // Check if we are reacting to our own Attribute Table update
  attribute_store_node_t scene_table_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE(SCENE_TABLE));
  if (ATTRIBUTE_STORE_INVALID_NODE != scene_table_node) {
    // Make sure the mapper does not map for this Scene Table configuration.
    if (change != ATTRIBUTE_DELETED) {
      attribute_mapper_pause_reactions_to_attribute_updates(updated_node);
    }
    return;
  }

  attribute_store_node_t endpoint_id_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  // Check if the attribute was deleted, in this case check if the
  // SceneTable should also disappear:
  if (change == ATTRIBUTE_DELETED) {
    bool at_least_one_actuator_attribute = false;
    for (uint32_t i = 0; i < COUNT_OF(scene_actuator_attribute_list); i++) {
      if (attribute_store_node_exists(attribute_store_get_first_child_by_type(
            endpoint_id_node,
            scene_actuator_attribute_list[i]))) {
        at_least_one_actuator_attribute = true;
        break;
      }
    }

    if (at_least_one_actuator_attribute == false) {
      attribute_store_delete_node(
        attribute_store_get_first_child_by_type(endpoint_id_node,
                                                ATTRIBUTE(SCENE_TABLE)));
    }
    return;
  }

  // At that point, we need a scene Table:
  attribute_store_create_child_if_missing(endpoint_id_node,
                                          ATTRIBUTE(SCENE_TABLE));

  if ((change == ATTRIBUTE_UPDATED)
      && attribute_store_is_value_defined(updated_node, DESIRED_ATTRIBUTE)
      && (false == attribute_store_is_value_matched(updated_node))) {
    // Any actuator update with a reported/desired mismatch,
    // we will consider that the node got "out of the scene"
    std::string unid;
    zwave_endpoint_id_t endpoint_id = 0;
    if (SL_STATUS_OK
        == zcl_cluster_servers_get_unid_endpoint(endpoint_id_node,
                                                 endpoint_id,
                                                 unid)) {
      dotdot_set_scenes_scene_valid(unid.c_str(),
                                    endpoint_id,
                                    DESIRED_ATTRIBUTE,
                                    false);
    }
  }
}

/**
 * @brief Listens on updates/creations/deletions in the Scene Table and schedules
 * an MQTT publication
 *
 * @param updated_node   The node that was updated under the scene table.
 * @param change         The type of change applied to the node.
 */
static void on_scene_data_updated(attribute_store_node_t updated_node,
                                  attribute_store_change_t change)
{
  // When something in a scene gets updated, it's likely not going to be a single
  // attribute. We want to avoid to do the publication logic many times,
  // so we set a callback in the very near future.
  attribute_store_node_t endpoint_id_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t scene_table_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(SCENE_TABLE));

  if (false == attribute_store_node_exists(scene_table_node)) {
    return;
  }
  attribute_timeout_set_callback(scene_table_node, 15, &publish_scene_table);
}

/**
 * @brief Verifies if something needs to be published after a node is moved
 * to Online Functional.
 *
 * @param network_status_node     Attribute Store node that was modified.
 * @param change                  Type of change applied to the node.
 */
static void on_network_status_update(attribute_store_node_t network_status_node,
                                     attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  // Read the network status, do not publish if it is not online functional:
  if (unify_attribute_store_node_state_get_status(network_status_node)
      != ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL) {
    return;
  }
  // Go up and find the UNID/Endpoint and its network status.
  unid_t unid;
  if (SL_STATUS_OK
      != attribute_store_network_helper_get_unid_from_node(network_status_node,
                                                           unid)) {
    return;
  }

  // Go around all the endpoints and ask for a Scene Table publications:
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(network_status_node,
                                                 ATTRIBUTE_NODE_ID);
  for (attribute endpoint_id_node:
       attribute(node_id_node).children(ATTRIBUTE_ENDPOINT_ID)) {
    publish_scene_table(endpoint_id_node.child_by_type(ATTRIBUTE(SCENE_TABLE)));
  }
}

///////////////////////////////////////////////////////////////////////////////
// DotDot MQTT callback functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t scenes_cluster_mapper_add_scene(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  SGroupId groupid,
  uint8_t sceneid,
  uint16_t transition_time,
  SSceneName scene_name,
  uint8_t extension_field_sets_count,
  const SExtensionFieldSetList *extension_field_sets,
  uint8_t transition_time100ms)
{
  attribute_store_node_t scene_table_node
    = get_scene_table_node(unid, endpoint);

  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return (scene_table_node == ATTRIBUTE_STORE_INVALID_NODE) ? SL_STATUS_FAIL
                                                              : SL_STATUS_OK;
  }

  // UIC dotdot mqtt is unable to parse the Scene extension fieldset,
  // so we have a separate MQTT callback here. See on_add_scene_command_received
  return SL_STATUS_OK;
}

static sl_status_t scenes_cluster_mapper_view_scene(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  SGroupId groupid,
  uint8_t sceneid)
{
  attribute_store_node_t scene_table_node
    = get_scene_table_node(unid, endpoint);

  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return (scene_table_node == ATTRIBUTE_STORE_INVALID_NODE) ? SL_STATUS_FAIL
                                                              : SL_STATUS_OK;
  }

  sl_log_debug(LOG_TAG,
               "ViewScene Command received. "
               "Publishing the Scene Table again.");
  on_scene_data_updated(scene_table_node, ATTRIBUTE_UPDATED);
  return SL_STATUS_OK;
}

static sl_status_t scenes_cluster_mapper_remove_scene(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  SGroupId groupid,
  uint8_t sceneid)
{
  attribute_store_node_t scene_table_node
    = get_scene_table_node(unid, endpoint);

  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return (scene_table_node == ATTRIBUTE_STORE_INVALID_NODE) ? SL_STATUS_FAIL
                                                              : SL_STATUS_OK;
  }

  // Matter specs here say to ignore removing scenes if the node/ep is not part
  // of the group. ZCL spec says nothing.

  // Remove the scene only if both the Scene ID and Group ID are matching.
  scene_id_group_id_t scene_id_group_id = {};
  scene_id_group_id.scene_id            = sceneid;
  scene_id_group_id.group_id            = groupid;

  attribute_store_node_t scene_id_node
    = attribute_store_get_node_child_by_value(scene_table_node,
                                              ATTRIBUTE(SCENE_ID_GROUP_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&scene_id_group_id,
                                              sizeof(scene_id_group_id),
                                              0);
  // Calling attribute_store_delete on an invalid node is just a no-op.
  attribute_store_delete_node(scene_id_node);

  // Adjust current Scene, Current Group and Scene valid attributes if needed
  SGroupId current_group
    = dotdot_get_scenes_current_group(unid, endpoint, REPORTED_ATTRIBUTE);
  uint8_t current_scene
    = dotdot_get_scenes_current_scene(unid, endpoint, REPORTED_ATTRIBUTE);
  if ((groupid == current_group) && (current_scene == sceneid)) {
    attribute_mapper_pause_mapping();
    dotdot_set_scenes_scene_valid(unid, endpoint, DESIRED_ATTRIBUTE, false);
    dotdot_set_scenes_scene_valid(unid, endpoint, REPORTED_ATTRIBUTE, false);
    dotdot_set_scenes_current_scene(unid, endpoint, DESIRED_ATTRIBUTE, 0);
    dotdot_set_scenes_current_scene(unid, endpoint, REPORTED_ATTRIBUTE, 0);
    dotdot_set_scenes_current_group(unid, endpoint, DESIRED_ATTRIBUTE, 0);
    dotdot_set_scenes_current_group(unid, endpoint, REPORTED_ATTRIBUTE, 0);
    attribute_mapper_resume_mapping();
  }

  return SL_STATUS_OK;
}

static sl_status_t scenes_cluster_mapper_remove_all_scenes(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  SGroupId groupid)
{
  attribute_store_node_t scene_table_node
    = get_scene_table_node(unid, endpoint);

  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return (scene_table_node == ATTRIBUTE_STORE_INVALID_NODE) ? SL_STATUS_FAIL
                                                              : SL_STATUS_OK;
  }

  // Matter specs here say to ignore removing scenes if the node/ep is not part
  // of the group. ZCL spec says nothing.

  std::set<attribute_store_node_t> scene_nodes_to_remove;
  for (size_t i = 0; i < attribute_store_get_node_child_count_by_type(
                       scene_table_node,
                       ATTRIBUTE(SCENE_ID_GROUP_ID));
       i++) {
    attribute_store_node_t scene_id_node
      = attribute_store_get_node_child_by_type(scene_table_node,
                                               ATTRIBUTE(SCENE_ID_GROUP_ID),
                                               i);
    scene_id_group_id_t scene_id_group_id = {};
    attribute_store_get_reported(scene_id_node,
                                 &scene_id_group_id,
                                 sizeof(scene_id_group_id));

    if (groupid == scene_id_group_id.group_id) {
      scene_nodes_to_remove.insert(scene_id_node);
    }
  }

  for (attribute_store_node_t node: scene_nodes_to_remove) {
    attribute_store_delete_node(node);
  }

  SGroupId current_group
    = dotdot_get_scenes_current_group(unid, endpoint, REPORTED_ATTRIBUTE);
  if (groupid == current_group) {
    attribute_mapper_pause_mapping();
    dotdot_set_scenes_scene_valid(unid, endpoint, DESIRED_ATTRIBUTE, false);
    dotdot_set_scenes_scene_valid(unid, endpoint, REPORTED_ATTRIBUTE, false);
    dotdot_set_scenes_current_scene(unid, endpoint, DESIRED_ATTRIBUTE, 0);
    dotdot_set_scenes_current_scene(unid, endpoint, REPORTED_ATTRIBUTE, 0);
    dotdot_set_scenes_current_group(unid, endpoint, DESIRED_ATTRIBUTE, 0);
    dotdot_set_scenes_current_group(unid, endpoint, REPORTED_ATTRIBUTE, 0);
    attribute_mapper_resume_mapping();
  }

  return SL_STATUS_OK;
}

static sl_status_t scenes_cluster_mapper_store_scene(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  SGroupId groupid,
  uint8_t sceneid)
{
  attribute_store_node_t scene_table_node
    = get_scene_table_node(unid, endpoint);

  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return (scene_table_node == ATTRIBUTE_STORE_INVALID_NODE) ? SL_STATUS_FAIL
                                                              : SL_STATUS_OK;
  } else if (scene_table_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_FAIL;
  }

  // Check if we are a member of the group before storing the scene:
  // ZCL spec does not really tell to ignore here, but the matter spec does... hmm
  if ((groupid != 0) && (!is_unid_endpoint_in_group(unid, endpoint, groupid))) {
    sl_log_debug(LOG_TAG,
                 "Ignoring StoreScene Command as %s/ep%d "
                 "is not part of GroupID %d.",
                 unid,
                 endpoint,
                 groupid);
    return SL_STATUS_FAIL;
  }

  scene_id_group_id_t scene_id_group_id = {};
  scene_id_group_id.scene_id            = sceneid;
  scene_id_group_id.group_id            = groupid;

  attribute_store_node_t scene_id_node
    = attribute_store_emplace(scene_table_node,
                              ATTRIBUTE(SCENE_ID_GROUP_ID),
                              &scene_id_group_id,
                              sizeof(scene_id_group_id));

  attribute_store_node_t scene_name_node
    = attribute_store_create_child_if_missing(scene_id_node,
                                              ATTRIBUTE(SCENE_NAME));
  if (attribute_store_is_reported_defined(scene_name_node) == false) {
    attribute_store_set_reported_string(scene_name_node, EMPTY_STRING);
  }

  const uint16_t transition_time = 0;
  attribute_store_set_child_reported_only_if_missing(
    scene_id_node,
    ATTRIBUTE(SCENE_TRANSITION_TIME),
    &transition_time,
    sizeof(transition_time));
  const uint8_t transition_time_100ms = 0;
  attribute_store_set_child_reported_only_if_missing(
    scene_id_node,
    ATTRIBUTE(SCENE_TRANSITION_TIME_100MS),
    &transition_time_100ms,
    sizeof(transition_time_100ms));

  attribute_store_node_t scene_extension_fieldset_node
    = attribute_store_create_child_if_missing(scene_id_node,
                                              ATTRIBUTE(SCENE_FIELD_SETS));

  // Here we assume that we know the state of the device.
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(scene_table_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  for (size_t i = 0; i < COUNT_OF(scene_actuator_attribute_list); i++) {
    // Check if the actuator setting exists.
    attribute_store_node_t attribute_node
      = attribute_store_get_first_child_by_type(
        endpoint_node,
        scene_actuator_attribute_list[i]);

    if (ATTRIBUTE_STORE_INVALID_NODE == attribute_node) {
      continue;
    }
    attribute_mapper_pause_mapping();
    attribute_store_node_t scene_table_attribute_node
      = attribute_store_create_child_if_missing(
        scene_extension_fieldset_node,
        scene_actuator_attribute_list[i]);
    attribute_mapper_resume_mapping();
    // This attribute config should never trigger evaluations
    attribute_mapper_pause_reactions_to_attribute_updates(
      scene_table_attribute_node);

    attribute_store_copy_value(attribute_node,
                               scene_table_attribute_node,
                               REPORTED_ATTRIBUTE);
    attribute_store_set_desired_as_reported(scene_table_attribute_node);
  }

  // Publish the Scene Table again (Could be that the scene already existed,
  // so automatic publication may not kick in)
  on_scene_data_updated(scene_table_node, ATTRIBUTE_UPDATED);

  // Finally, if we just stored a scene from the current settings, we can also
  // assume that the current scene is valid
  attribute_mapper_pause_mapping();
  dotdot_set_scenes_scene_valid(unid, endpoint, DESIRED_ATTRIBUTE, true);
  dotdot_set_scenes_scene_valid(unid, endpoint, REPORTED_ATTRIBUTE, true);
  dotdot_set_scenes_current_scene(unid, endpoint, DESIRED_ATTRIBUTE, sceneid);
  dotdot_set_scenes_current_scene(unid, endpoint, REPORTED_ATTRIBUTE, sceneid);
  dotdot_set_scenes_current_group(unid, endpoint, DESIRED_ATTRIBUTE, groupid);
  dotdot_set_scenes_current_group(unid, endpoint, REPORTED_ATTRIBUTE, groupid);
  attribute_mapper_resume_mapping();

  return SL_STATUS_OK;
}

static sl_status_t scenes_cluster_mapper_recall_scene(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  SGroupId groupid,
  uint8_t sceneid,
  uint16_t transition_time)
{
  attribute_store_node_t scene_table_node
    = get_scene_table_node(unid, endpoint);

  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return (scene_table_node == ATTRIBUTE_STORE_INVALID_NODE) ? SL_STATUS_FAIL
                                                              : SL_STATUS_OK;
  }

  // Matter specs here say to ignore recalling scenes if the node/ep is not part
  // of the group. ZCL spec says nothing.

  // Remove the scene only if both the Scene ID and Group ID are matching.
  scene_id_group_id_t scene_id_group_id = {};
  scene_id_group_id.scene_id            = sceneid;
  scene_id_group_id.group_id            = groupid;

  attribute_store_node_t scene_id_node
    = attribute_store_get_node_child_by_value(scene_table_node,
                                              ATTRIBUTE(SCENE_ID_GROUP_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&scene_id_group_id,
                                              sizeof(scene_id_group_id),
                                              0);
  attribute_store_node_t scene_extension_fieldset_node
    = attribute_store_get_first_child_by_type(scene_id_node,
                                              ATTRIBUTE(SCENE_FIELD_SETS));
  if (ATTRIBUTE_STORE_INVALID_NODE == scene_extension_fieldset_node) {
    sl_log_debug(LOG_TAG,
                 "Scene %d Group %d not found. Ignoring RecallScene Command.",
                 scene_id_group_id.scene_id,
                 scene_id_group_id.group_id);
    return SL_STATUS_FAIL;
  }

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(scene_table_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  for (size_t index = 0; index < attribute_store_get_node_child_count(
                           scene_extension_fieldset_node);
       index++) {
    attribute_store_node_t attribute_node
      = attribute_store_get_node_child(scene_extension_fieldset_node, index);

    attribute_store_node_t corresponding_zcl_node
      = attribute_store_get_first_child_by_type(
        endpoint_node,
        attribute_store_get_node_type(attribute_node));

    attribute_store_copy_value(attribute_node,
                               corresponding_zcl_node,
                               DESIRED_ATTRIBUTE);
  }

  // Now Special handling, check if we have a transition time: (this field is
  // optional in Matter/ZCL; but unify mandates all fields in all clusters.
  // if the TransitionTime is set to 0, we look up the Scene Transition Time from the Scene Table)
  if (transition_time == 0) {
    attribute_store_get_child_reported(scene_id_node,
                                       ATTRIBUTE(SCENE_TRANSITION_TIME),
                                       &transition_time,
                                       sizeof(transition_time));
  }
  uint8_t transition_time_100ms = 0;
  attribute_store_get_child_reported(scene_id_node,
                                     ATTRIBUTE(SCENE_TRANSITION_TIME_100MS),
                                     &transition_time_100ms,
                                     sizeof(transition_time_100ms));

  // Copy into the clusters that have Timing elements:
  uint16_t level_transition_time = 0;
  if (transition_time < 6552) {
    level_transition_time = transition_time * 10 + transition_time_100ms;
  } else {
    level_transition_time = 0xFFFF;
  }
  attribute_store_set_child_desired_only_if_exists(
    endpoint_node,
    DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
    &level_transition_time,
    sizeof(level_transition_time));

  // Scene exists and was recalled, we update the desired value of the
  // current scene
  sl_status_t status
    = dotdot_set_scenes_scene_valid(unid, endpoint, DESIRED_ATTRIBUTE, true);
  status |= dotdot_set_scenes_current_scene(unid,
                                            endpoint,
                                            DESIRED_ATTRIBUTE,
                                            sceneid);
  status |= dotdot_set_scenes_current_group(unid,
                                            endpoint,
                                            DESIRED_ATTRIBUTE,
                                            groupid);
  return status;
}

static sl_status_t scenes_cluster_mapper_get_scene_membership(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  SGroupId groupid)
{
  attribute_store_node_t scene_table_node
    = get_scene_table_node(unid, endpoint);

  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return (scene_table_node == ATTRIBUTE_STORE_INVALID_NODE) ? SL_STATUS_FAIL
                                                              : SL_STATUS_OK;
  }

  // Does it make sense to publish the Scene table again here?
  sl_log_debug(LOG_TAG,
               "GetSceneMembershipCommand received. "
               "Publishing the Scene Table again.");
  on_scene_data_updated(scene_table_node, ATTRIBUTE_UPDATED);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// MQTT subscription functions
///////////////////////////////////////////////////////////////////////////////
static void on_add_scene_command_received(const char *topic,
                                          const char *message,
                                          const size_t message_length)
{
  if (message_length == 0) {
    return;
  }

  // Get the UNID/Endpoint from the topic.
  std::string unid;
  uint8_t endpoint_id = 0;
  if (false == uic_dotdot_mqtt::parse_topic(topic, unid, endpoint_id)) {
    sl_log_debug(LOG_TAG,
                 "Could not parse UNID/Endpoint information. "
                 "Ignoring Command.");
    return;
  }

  // Check if it is supported by looking up the SceneTable node:
  attribute_store_node_t scene_table_node
    = get_scene_table_node(unid.c_str(), endpoint_id);
  if (ATTRIBUTE_STORE_INVALID_NODE == scene_table_node) {
    // No Scene table for this UNID/ Endpoint. Ignore the command
    return;
  }

  // Parse the payload.
  nlohmann::json json_payload;
  nlohmann::json json_extension_field_set;
  SGroupId groupid              = {};
  uint8_t sceneid               = {};
  uint16_t transition_time      = {};
  uint8_t transition_time_100ms = {};
  std::string scene_name;
  try {
    json_payload = nlohmann::json::parse(std::string(message));

    if (json_payload.find("GroupID") == json_payload.end()) {
      sl_log_debug(LOG_TAG, "Missing GroupID for AddScene Command. Ignoring");
      return;
    } else if (json_payload.find("SceneID") == json_payload.end()) {
      sl_log_debug(LOG_TAG, "Missing SceneID for AddScene Command. Ignoring");
      return;
    } else if (json_payload.find("TransitionTime") == json_payload.end()) {
      sl_log_debug(LOG_TAG,
                   "Missing TransitionTime for AddScene Command. Ignoring");
      return;
    } else if (json_payload.find("TransitionTime100ms") == json_payload.end()) {
      sl_log_debug(LOG_TAG,
                   "Missing TransitionTime for AddScene Command. Ignoring");
      return;
    } else if (json_payload.find("SceneName") == json_payload.end()) {
      sl_log_debug(LOG_TAG, "Missing SceneName for AddScene Command. Ignoring");
      return;
    } else if (json_payload.find("ExtensionFieldSets") == json_payload.end()) {
      sl_log_debug(LOG_TAG,
                   "Missing ExtensionFieldSets for AddScene Command. Ignoring");
      return;
    }

    // Retrieve the values:
    groupid         = json_payload.at("GroupID").get<SGroupId>();
    sceneid         = json_payload.at("SceneID").get<uint8_t>();
    transition_time = json_payload.at("TransitionTime").get<uint16_t>();
    transition_time_100ms
      = json_payload.at("TransitionTime100ms").get<uint8_t>();
    scene_name               = json_payload.at("SceneName").get<std::string>();
    json_extension_field_set = json_payload.at("ExtensionFieldSets");

    // Start saving the scene.
    scene_id_group_id_t scene_id_group_id = {};
    scene_id_group_id.scene_id            = sceneid;
    scene_id_group_id.group_id            = groupid;

    attribute_store_node_t scene_id_node
      = attribute_store_emplace(scene_table_node,
                                ATTRIBUTE(SCENE_ID_GROUP_ID),
                                &scene_id_group_id,
                                sizeof(scene_id_group_id));
    // Set the Scene Name
    attribute_store_node_t scene_name_node
      = attribute_store_create_child_if_missing(scene_id_node,
                                                ATTRIBUTE(SCENE_NAME));
    attribute_store_set_reported_string(scene_name_node, scene_name.c_str());

    // Set the Scene Transition Times
    attribute_store_set_child_reported(scene_id_node,
                                       ATTRIBUTE(SCENE_TRANSITION_TIME),
                                       &transition_time,
                                       sizeof(transition_time));
    attribute_store_set_child_reported(scene_id_node,
                                       ATTRIBUTE(SCENE_TRANSITION_TIME_100MS),
                                       &transition_time_100ms,
                                       sizeof(transition_time_100ms));

    // Extension field set:
    attribute_store_node_t extension_fieldset_node
      = attribute_store_create_child_if_missing(scene_id_node,
                                                ATTRIBUTE(SCENE_FIELD_SETS));

    // Make sure that the previous scene data is wiped off, if any:
    attribute_store_delete_all_children(extension_fieldset_node);
    // Then save the new data:
    save_extension_fieldset_to_attribute_store(extension_fieldset_node,
                                               json_extension_field_set);

  } catch (const nlohmann::json::exception &e) {
    // Catch JSON object field parsing errors
    sl_log_debug(LOG_TAG,
                 "Error parsing AddScene Command: %s. Ignoring",
                 e.what());
    return;
  } catch (const std::exception &e) {
    sl_log_debug(LOG_TAG, "Error parsing AddScene Command. Ignoring");
    return;
  }

  // Make sure to publish in case only attributes under the
  // extension field set node were modified
  on_scene_data_updated(scene_table_node, ATTRIBUTE_UPDATED);
}

static void on_group_add_scene_command_received(const char *topic,
                                                const char *message,
                                                const size_t message_length)
{
  if (message_length == 0) {
    return;
  }
  // Get help from the Group Dispatch component to re-inject the command
  // to individual endpoints within the group:

  try {
    std::string topic_string(topic);
    std::vector<std::string> topic_elements;
    boost::algorithm::split(topic_elements,
                            topic_string,
                            boost::is_any_of("/"));

    uint16_t group_id = 0;
    if (0 == sscanf(topic_elements[2].c_str(), "%" SCNu16, &group_id)) {
      sl_log_debug(LOG_TAG,
                   "Error parsing Group ID from topic %s. Ignoring",
                   topic);
      return;
    }

    // Leverage the service provided by our the UIC MQTT group dispatch.
    group_dispatch_t group_dispatch
      = uic_mqtt_dotdot_get_group_dispatch_callback();
    if (group_dispatch != nullptr) {
      group_dispatch(group_id,
                     SCENES_CLUSTER_NAME,
                     topic_elements.back().c_str(),
                     message,
                     message_length,
                     &on_add_scene_command_received);
    } else {
      sl_log_debug(LOG_TAG,
                   "No MQTT dispatch function available. Add Scene "
                   "command will not be dispatched to individual "
                   "members automatically.");
    }

  } catch (const std::exception &e) {
    sl_log_debug(LOG_TAG,
                 "Unable to parse MQTT Topic. Exception: %s\n",
                 e.what());
    return;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zcl_scenes_cluster_server_init()
{
  sl_log_debug(LOG_TAG, "Init of the Scenes Cluster Server");

  // Register all callbacks to the Attribute Store
  sl_status_t status = attribute_store_register_callback_by_type_to_array(
    &on_scene_actuator_attribute_updated,
    scene_actuator_attribute_list,
    COUNT_OF(scene_actuator_attribute_list));

  status |= attribute_store_register_callback_by_type_to_array(
    &on_scene_data_updated,
    scene_table_attributes,
    COUNT_OF(scene_table_attributes));

  // Make sure to publish the scene table when a node becomes available.
  attribute_store_register_callback_by_type_and_state(&on_network_status_update,
                                                      DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                                      REPORTED_ATTRIBUTE);

  // Callback for Dotdot MQTT commands
  uic_mqtt_dotdot_scenes_add_scene_callback_set(
    &scenes_cluster_mapper_add_scene);

  uic_mqtt_dotdot_scenes_view_scene_callback_set(
    &scenes_cluster_mapper_view_scene);

  uic_mqtt_dotdot_scenes_remove_scene_callback_set(
    &scenes_cluster_mapper_remove_scene);

  uic_mqtt_dotdot_scenes_remove_all_scenes_callback_set(
    &scenes_cluster_mapper_remove_all_scenes);

  uic_mqtt_dotdot_scenes_store_scene_callback_set(
    &scenes_cluster_mapper_store_scene);

  uic_mqtt_dotdot_scenes_recall_scene_callback_set(
    &scenes_cluster_mapper_recall_scene);

  uic_mqtt_dotdot_scenes_get_scene_membership_callback_set(
    &scenes_cluster_mapper_get_scene_membership);

  // MQTT callbacks for the Add Scene command
  uic_mqtt_subscribe("ucl/by-unid/+/+/Scenes/Commands/AddScene",
                     &on_add_scene_command_received);

  uic_mqtt_subscribe("ucl/by-group/+/Scenes/Commands/AddScene",
                     &on_group_add_scene_command_received);

  return status;
}

void zcl_scenes_cluster_server_teardown()
{
  return;
}
