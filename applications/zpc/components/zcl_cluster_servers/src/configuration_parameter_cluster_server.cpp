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
#include "configuration_parameter_cluster_server.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_store_helper.h"
#include "zwave_command_class_configuration_types.h"

// Interfaces
#include "zwave_command_class_version_types.h"
#include "ucl_definitions.h"

// ZPC includes
#include "zpc_attribute_store.h"
#include "zwave_command_class_generic_types.h"

// Includes from Unify shared components
#include "attribute.hpp"
#include "sl_log.h"

// Includes from auto-generated files
#include "dotdot_mqtt.h"
#include "zap-types.h"

// Generic includes
#include <string>
#include <vector>
#include <stdlib.h>

using namespace attribute_store;
using attribute_store_get_value_function_t
  = sl_status_t (*)(attribute_store_node_t, void *, size_t);
using attribute_store_get_string_function_t
  = sl_status_t (*)(attribute_store_node_t, char *, size_t);

// Setup Log ID
constexpr char LOG_TAG[] = "configuration_parameter_cluster_server";

// Constants
constexpr uint16_t CONFIGURATION_PARAMETERS_CLUSTER_REVISION = 1;

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_##type

// List of attributes for each configuration parameter
static const attribute_store_type_t configuration_parameter_attributes[]
  = {ATTRIBUTE(PARAMETER_FORMAT),
     ATTRIBUTE(PARAMETER_NAME),
     ATTRIBUTE(PARAMETER_INFO),
     ATTRIBUTE(PARAMETER_VALUE),
     ATTRIBUTE(PARAMETER_MIN_VALUE),
     ATTRIBUTE(PARAMETER_MAX_VALUE),
     ATTRIBUTE(PARAMETER_DEFAULT_VALUE),
     ATTRIBUTE(PARAMETER_ADVANCED),
     ATTRIBUTE(PARAMETER_READ_ONLY),
     ATTRIBUTE(PARAMETER_ALTERING_CAPABILITIES)};

// Forward declarations
void on_next_parameter_id_update(attribute_store_node_t updated_node,
                                 attribute_store_change_t change);

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
//////////////////////////////////////////////////////////////////////////////
/**
 * @brief Returns the version of the Configuration Command Class
 *
 * @param endpoint_node     Attribute Store node for the Endpoint ID.
 * @returns Version of the Configuration COmmand Class. 0 if undefined or not supported.
 */
static zwave_cc_version_t
  get_configuration_command_class_version(attribute_store_node_t endpoint_node)
{
  attribute_store_node_t configuration_version_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(VERSION),
                                             0);
  zwave_cc_version_t configuration_cc_version = 0;
  attribute_store_get_reported(configuration_version_node,
                               &configuration_cc_version,
                               sizeof(configuration_cc_version));

  return configuration_cc_version;
}

/**
 * @brief Unallocate memory from a configuration parameter
 *
 * @param parameter     Configuration parameter to be cleaned up / freed..
 */
static void free_configuration_parameter(ConfigurationParameter &parameter)
{
  if (parameter.Info != nullptr) {
    free((void *)parameter.Info);
    parameter.Info = nullptr;
  }
  if (parameter.Name != nullptr) {
    free((void *)parameter.Name);
    parameter.Name = nullptr;
  }
}

/**
 * @brief Unallocate memory from a configuration parameter vector
 *
 * @param configuration     Configuration parameter vector to be cleaned up / freed..
 */
static void clear_configuration_parameters_vector(
  std::vector<ConfigurationParameter> &configuration)
{
  for (ConfigurationParameter parameter: configuration) {
    free_configuration_parameter(parameter);
  }
  configuration.clear();
}

/**
 * @brief Loads the configuration parameters from the attribute store to a
 * ConfigurationParameter vector.
 *
 * NOTE: this function mallocs 2 strings per parameter and they need to be freed
 * when no longer needed
 *
 * @param endpoint_id_node     Attribute Store node for the Endpoint ID.
 * @param [out] configuration  Vector of ConfigurationParameters to fill up.
 * @param value_state     Attribute Store node for the Endpoint ID.
 */
static void get_configuration_parameters(
  attribute_store_node_t endpoint_id_node,
  std::vector<ConfigurationParameter> &configuration,
  attribute_store_node_value_state_t value_state)
{
  // Process the value state: assign get functions based on its value:
  attribute_store_get_value_function_t get_value_function   = nullptr;
  attribute_store_get_string_function_t get_string_function = nullptr;

  // Select the function that we use to read the values:
  if (value_state == DESIRED_ATTRIBUTE) {
    get_value_function  = &attribute_store_get_desired;
    get_string_function = &attribute_store_get_desired_string;
  } else if (value_state == DESIRED_OR_REPORTED_ATTRIBUTE) {
    get_value_function  = &attribute_store_get_desired_else_reported;
    get_string_function = &attribute_store_get_desired_else_reported_string;
  } else {
    get_value_function  = &attribute_store_get_reported;
    get_string_function = &attribute_store_get_reported_string;
  }

  // Now go around and read the configuration parameters:
  for (attribute parameter_id_node:
       attribute(endpoint_id_node).children(ATTRIBUTE(PARAMETER_ID))) {
    sl_status_t status               = SL_STATUS_OK;
    ConfigurationParameter parameter = {};

    status |= get_value_function(parameter_id_node,
                                 &parameter.ParameterId,
                                 sizeof(parameter.ParameterId));

    attribute_store_node_t parameter_value_node
      = attribute_store_get_node_child_by_type(parameter_id_node,
                                               ATTRIBUTE(PARAMETER_VALUE),
                                               0);
    status |= get_value_function(parameter_value_node,
                                 &parameter.Value,
                                 sizeof(parameter.Value));

    attribute_store_node_t parameter_min_value_node
      = attribute_store_get_node_child_by_type(parameter_id_node,
                                               ATTRIBUTE(PARAMETER_MIN_VALUE),
                                               0);
    status |= get_value_function(parameter_min_value_node,
                                 &parameter.MinimumValue,
                                 sizeof(parameter.MinimumValue));

    attribute_store_node_t parameter_max_value_node
      = attribute_store_get_node_child_by_type(parameter_id_node,
                                               ATTRIBUTE(PARAMETER_MAX_VALUE),
                                               0);
    status |= get_value_function(parameter_max_value_node,
                                 &parameter.MaximumValue,
                                 sizeof(parameter.MaximumValue));

    attribute_store_node_t parameter_default_value_node
      = attribute_store_get_node_child_by_type(
        parameter_id_node,
        ATTRIBUTE(PARAMETER_DEFAULT_VALUE),
        0);
    status |= get_value_function(parameter_default_value_node,
                                 &parameter.DefaultValue,
                                 sizeof(parameter.DefaultValue));

    attribute_store_node_t parameter_format_node
      = attribute_store_get_node_child_by_type(parameter_id_node,
                                               ATTRIBUTE(PARAMETER_FORMAT),
                                               0);
    status |= get_value_function(parameter_format_node,
                                 &parameter.DisplayFormat,
                                 sizeof(parameter.DisplayFormat));

    attribute_store_node_t parameter_read_only_node
      = attribute_store_get_node_child_by_type(parameter_id_node,
                                               ATTRIBUTE(PARAMETER_READ_ONLY),
                                               0);
    status |= get_value_function(parameter_read_only_node,
                                 &parameter.ReadOnly,
                                 sizeof(parameter.ReadOnly));

    attribute_store_node_t parameter_advanced_node
      = attribute_store_get_node_child_by_type(parameter_id_node,
                                               ATTRIBUTE(PARAMETER_ADVANCED),
                                               0);
    status |= get_value_function(parameter_advanced_node,
                                 &parameter.Advanced,
                                 sizeof(parameter.Advanced));

    attribute_store_node_t parameter_altering_capabilities_node
      = attribute_store_get_node_child_by_type(
        parameter_id_node,
        ATTRIBUTE(PARAMETER_ALTERING_CAPABILITIES),
        0);
    status |= get_value_function(parameter_altering_capabilities_node,
                                 &parameter.AlteringCapabilities,
                                 sizeof(parameter.AlteringCapabilities));

    // Parameter name
    char *name_string = nullptr;
    name_string       = (char *)malloc(ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
    if (name_string != nullptr) {
      attribute_store_node_t parameter_name_node
        = attribute_store_get_node_child_by_type(parameter_id_node,
                                                 ATTRIBUTE(PARAMETER_NAME),
                                                 0);
      status |= get_string_function(parameter_name_node,
                                    name_string,
                                    ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
    }
    parameter.Name = name_string;

    // Parameter Info
    char *info_string = nullptr;
    info_string       = (char *)malloc(ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
    if (info_string != nullptr) {
      attribute_store_node_t parameter_info_node
        = attribute_store_get_node_child_by_type(parameter_id_node,
                                                 ATTRIBUTE(PARAMETER_INFO),
                                                 0);
      status |= get_string_function(parameter_info_node,
                                    info_string,
                                    ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
    }
    parameter.Info = info_string;

    if (status == SL_STATUS_OK) {
      configuration.push_back(parameter);
    } else {
      free_configuration_parameter(parameter);
    }
  }
}

/**
 * @brief Reads the UCL network status of a node, starting from any attribute
 * under the NodeID in the attribute store.
 *
 * @param node     Attribute store node under the NodeID for which we want
 * to read the network status.
 */
node_state_topic_state_t get_network_status(attribute_store_node_t node)
{
  // Default to UNAVAILABLE if the value is undefined in the attribute store
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_UNAVAILABLE;

  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_NODE_ID);

  attribute_store_node_t network_status_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_NETWORK_STATUS,
                                             0);
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  return network_status;
}

///////////////////////////////////////////////////////////////////////////////
// DotDot MQTT incoming commands handling functions
//////////////////////////////////////////////////////////////////////////////
static sl_status_t configuration_parameter_discover_parameter_command(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint16_t parameter_id)
{
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);
  uint8_t version = get_configuration_command_class_version(endpoint_node);
  if (version == 0 || version > 2) {
    // Regardless of the call type, do not go furhter if the node is not
    // supposed to support this command
    return SL_STATUS_FAIL;
  }

  // Now that we know that the command is supported, return here if it is
  // a support check type of call.
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    return SL_STATUS_OK;
  }

  // Do not try to discover parameter ID 0.
  if (parameter_id == 0) {
    return SL_STATUS_FAIL;
  }

  // Store the "wish" for parameter discover under the PARAMETERS_TO_DISCOVER attribute
  attribute_store_node_t parameter_list_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(PARAMETERS_TO_DISCOVER),
                                             0);
  if (parameter_list_node == ATTRIBUTE_STORE_INVALID_NODE) {
    parameter_list_node
      = attribute_store_add_node(ATTRIBUTE(PARAMETERS_TO_DISCOVER),
                                 endpoint_node);
  }

  // Check if we parameter ID that is requested already exists
  configuration_parameter_id_t parameter_id_to_discover
    = (configuration_parameter_id_t)parameter_id;
  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_value(
      parameter_list_node,
      ATTRIBUTE(PARAMETER_ID),
      REPORTED_ATTRIBUTE,
      (uint8_t *)&parameter_id_to_discover,
      sizeof(parameter_id_to_discover),
      0);

  if (parameter_id_node != ATTRIBUTE_STORE_INVALID_NODE) {
    // Nothing to do, it's alread there!
    return SL_STATUS_OK;
  }

  // Add the parameter and check if we want to push it to the Next parameter ID:
  parameter_id_node
    = attribute_store_add_node(ATTRIBUTE(PARAMETER_ID), parameter_list_node);
  attribute_store_set_reported(parameter_id_node,
                               &parameter_id_to_discover,
                               sizeof(parameter_id_to_discover));

  attribute_store_node_t next_parameter_id_node
    = attribute_store_get_node_child_by_type(
      endpoint_node,
      ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID),
      0);
  on_next_parameter_id_update(next_parameter_id_node, ATTRIBUTE_UPDATED);

  return SL_STATUS_OK;
}

static sl_status_t configuration_parameter_set_parameter_command(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint16_t parameter_id,
  int64_t value)
{
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    if (get_configuration_command_class_version(endpoint_node) != 0) {
      return SL_STATUS_OK;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  attribute_store_node_t configuration_parameter_id
    = attribute_store_get_node_child_by_value(
      endpoint_node,
      ATTRIBUTE(PARAMETER_ID),
      REPORTED_ATTRIBUTE,
      reinterpret_cast<uint8_t *>(&parameter_id),
      sizeof(parameter_id),
      0);

  attribute_store_node_t configuration_parameter_value_node
    = attribute_store_get_node_child_by_type(configuration_parameter_id,
                                             ATTRIBUTE(PARAMETER_VALUE),
                                             0);
  configuration_parameter_value_t new_value
    = (configuration_parameter_value_t)value;

  attribute_store_set_desired(configuration_parameter_value_node,
                              &new_value,
                              sizeof(new_value));

  return SL_STATUS_OK;
}

static sl_status_t configuration_parameter_default_reset_command(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type)
{
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  uint8_t version = get_configuration_command_class_version(endpoint_node);
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == call_type) {
    if (version >= 4) {
      return SL_STATUS_OK;
    } else {
      return SL_STATUS_FAIL;
    }
  }
  attribute_store_node_t configuration_default_reset_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(DEFAULT_RESET_REQUESTED),
                                             0);
  const command_status_values_t reset_requested = NEEDS_ONE_COMMAND;
  attribute_store_set_desired(configuration_default_reset_node,
                              &reset_requested,
                              sizeof(reset_requested));

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute publication functions
//////////////////////////////////////////////////////////////////////////////
void publish_configuration_parameters_cluster_cluster_revision(
  const std::string &unid, zwave_endpoint_id_t endpoint_id)
{
  // Do not publish any state supported commands for ourselves.
  if (true == is_zpc_unid(unid.c_str())) {
    return;
  }

  // Build the base topic and pass it on to DotDot MQTT.
  std::string base_topic
    = "ucl/by-unid/" + std::string(unid) + "/ep" + std::to_string(endpoint_id);

  uic_mqtt_dotdot_configuration_parameters_publish_cluster_revision(
    base_topic.c_str(),
    CONFIGURATION_PARAMETERS_CLUSTER_REVISION);
}

/**
 * @brief Publishes the Configuration Cluster Server attributes for Configuration
 *
 * @param endpoint_id_node    Endpoint ID for which we want to publish the
 *                            Configuration attributes.
 */
static void
  publish_configuration_parameters_cluster_configuration_parameters_attributes(
    const std::string &unid, zwave_endpoint_id_t endpoint_id)
{
  // Do not publish any state supported commands for ourselves.
  if (true == is_zpc_unid(unid.c_str())) {
    return;
  }

  // Build the base topic and pass it on to DotDot MQTT.
  std::string base_topic
    = "ucl/by-unid/" + std::string(unid) + "/ep" + std::to_string(endpoint_id);

  // Make an attribute store walkthrough and fill up our data.
  attribute_store_node_t endpoint_id_node
    = attribute_store_network_helper_get_endpoint_node(unid.c_str(),
                                                       endpoint_id);
  std::vector<ConfigurationParameter> configuration;

  // Get and publish the Desired, else reported value.
  get_configuration_parameters(endpoint_id_node,
                               configuration,
                               DESIRED_OR_REPORTED_ATTRIBUTE);

  uic_mqtt_dotdot_configuration_parameters_configuration_parameters_publish(
    base_topic.c_str(),
    configuration.size(),
    configuration.data(),
    UCL_MQTT_PUBLISH_TYPE_DESIRED);
  clear_configuration_parameters_vector(configuration);

  // Do it again for the Reported value
  get_configuration_parameters(endpoint_id_node,
                               configuration,
                               REPORTED_ATTRIBUTE);
  uic_mqtt_dotdot_configuration_parameters_configuration_parameters_publish(
    base_topic.c_str(),
    configuration.size(),
    configuration.data(),
    UCL_MQTT_PUBLISH_TYPE_REPORTED);
  clear_configuration_parameters_vector(configuration);
}

///////////////////////////////////////////////////////////////////////////////
// Attribute store callback functions
//////////////////////////////////////////////////////////////////////////////
/**
 * @brief Listens to updates to the next parameter id, checks if we have pending
 * parameters to discover and update the next parameter id attribute to search
 * for the next parameter
 *
 * @param updated_node    Attribute Store node that was modified.
 * @param change          Type of change applied to the node.
 */
void on_next_parameter_id_update(attribute_store_node_t updated_node,
                                 attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  // When the reported value gets defined, check if we have some pending parameters
  // to discover.
  if (false
      == attribute_store_is_value_defined(updated_node, REPORTED_ATTRIBUTE)) {
    return;
  }

  // value is defined, pick the next parameter to discover:
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t parameter_list_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(PARAMETERS_TO_DISCOVER),
                                             0);

  // Just pick the first parameter from the wishlist.
  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_type(parameter_list_node,
                                             ATTRIBUTE(PARAMETER_ID),
                                             0);
  configuration_parameter_id_t parameter_id = 0;
  attribute_store_get_reported(parameter_id_node,
                               &parameter_id,
                               sizeof(parameter_id));
  attribute_store_delete_node(parameter_id_node);

  if (parameter_id != 0) {
    attribute_store_set_desired(updated_node,
                                &parameter_id,
                                sizeof(parameter_id));
    attribute_store_undefine_reported(updated_node);
  }
}

/**
 * @brief Listens to updates to the configuration parameters or Network Status
 * for a given node, then publishes the attributes.
 *
 * @param updated_node    Attribute Store node that was modified.
 * @param change          Type of change applied to the node.
 */
void on_configuration_update(attribute_store_node_t updated_node,
                             attribute_store_change_t change)
{
  if (change == ATTRIBUTE_CREATED) {
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

  // All looks good, publish the configuration parameters:
  publish_configuration_parameters_cluster_configuration_parameters_attributes(
    std::string(unid),
    endpoint_id);
}

/**
 * @brief Verifies if something needs to be published after a node is moved
 * to Online Functional.
 *
 * @param network_status_node    Attribute Store node that was modified.
 * @param change          Type of change applied to the node.
 */
void on_network_status_update(attribute_store_node_t network_status_node,
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

    // Check that this endpoint supports Configuration, else just skip
    if (get_configuration_command_class_version(endpoint_id_node) == 0) {
      continue;
    }

    // All looks good, publish the configuration parameters and cluster revision
    publish_configuration_parameters_cluster_configuration_parameters_attributes(
      std::string(unid),
      endpoint_id);

    publish_configuration_parameters_cluster_cluster_revision(std::string(unid),
                                                              endpoint_id);
  }
}

///////////////////////////////////////////////////////////////////////////////
//  Init and teardown functions.
//////////////////////////////////////////////////////////////////////////////
sl_status_t configuration_parameter_cluster_server_init(void)
{
  sl_log_debug(LOG_TAG,
               "Configuration Parameter Commands handler initialization");
  // Register the callback for handling commands from IoT service
  uic_mqtt_dotdot_configuration_parameters_discover_parameter_callback_set(
    configuration_parameter_discover_parameter_command);
  uic_mqtt_dotdot_configuration_parameters_set_parameter_callback_set(
    configuration_parameter_set_parameter_command);
  uic_mqtt_dotdot_configuration_parameters_default_reset_all_parameters_callback_set(
    configuration_parameter_default_reset_command);

  // Register attribute updates
  attribute_store_register_callback_by_type_and_state(&on_network_status_update,
                                                      ATTRIBUTE_NETWORK_STATUS,
                                                      REPORTED_ATTRIBUTE);

  attribute_store_register_callback_by_type_to_array(
    &on_configuration_update,
    configuration_parameter_attributes,
    COUNT_OF(configuration_parameter_attributes));

  attribute_store_register_callback_by_type_and_state(
    &on_next_parameter_id_update,
    ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID),
    REPORTED_ATTRIBUTE);

  return SL_STATUS_OK;
}