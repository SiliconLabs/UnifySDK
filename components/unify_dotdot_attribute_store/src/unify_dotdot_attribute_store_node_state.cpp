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
//Include from this component
#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_attribute_store_node_state.h"
#include "unify_dotdot_attribute_store_configuration.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "uic_attribute_definitions.h"

//Include from other components
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "sl_log.h"
#include "sl_status.h"
#include "uic_mqtt.h"
#include "attribute.hpp"
#include "dotdot_mqtt.h"
#include "process.h"

//Generic include
#include <stddef.h>
#include <string>
#include <iomanip>

#include <nlohmann/json.hpp>
#include <boost/algorithm/string.hpp>

using namespace attribute_store;
constexpr char LOG_TAG[] = "unify_dotdot_attribute_store_node_state";

////////////////////////////////////////////////////////////////////////////////
// Defines and types
////////////////////////////////////////////////////////////////////////////////

// Theese strings must follow definitions in NodeStateNetworkStatus
static const char *state_names[] = {
  "Online functional",    // ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL
  "Online interviewing",  // ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING
  "Online non-functional",              // ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_NON_FUNCTIONAL
  "Unavailable",          // ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE
  "Offline",              // ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE
  "Online interviewing",  // ZCL_NODE_STATE_NETWORK_STATUS_COMMISIONING_STARTED

};

#define NODE_STATE_TOPIC_LAST (sizeof(state_names) / sizeof(char*))

// Theese strings must follow definitions in NodeStateSecurity
static const char *security_names[] = {
  "None",
  "Z-Wave S0",
  "Z-Wave S2 Unauthenticated",
  "Z-Wave S2 Authenticated",
  "Z-Wave S2 Access Control",
  "Zigbee Z3",
  "Matter"
};

#define NODES_STATE_TOPIC_SECURITY_LAST (sizeof(security_names) / sizeof(char *))
#define STATE(attribute) DOTDOT_ATTRIBUTE_ID_STATE_##attribute

///////////////////////////////////////////////////////////////////////////////
// MQTT Publication functions
//////////////////////////////////////////////////////////////////////////////
static sl_status_t publish_node_state(attribute_store_node_t node_id_node,
                                      bool available)
{
  dotdot_endpoint_id_t ep;
  nlohmann::json jsn;
  char unid[MAXIMUM_UNID_SIZE]    = {};

  // Get the endpoint 0, where is stored the state attributes.
  attribute ep_node
    = attribute(node_id_node)
        .child_by_type_and_value(unify_dotdot_attributes_endpoint_attribute(),
                                 0);
  if (SL_STATUS_OK
      != unify_dotdot_attributes_get_unid_endpoint()(ep_node, unid, &ep)) {
    sl_log_warning(LOG_TAG,
                   "Cannot derive UNID/EP from Attribute ID %d. "
                   "Node state will not be published.",
                   ep_node);
    return SL_STATUS_FAIL;
  }

  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE;
  if (available == true) {
    network_status = unify_attribute_store_node_state_get_status(node_id_node);
  }

  NodeStateSecurity security_level = ZCL_NODE_STATE_SECURITY_NONE;
  attribute_store_get_child_reported(ep_node,
                                     STATE(SECURITY),
                                     &security_level,
                                     sizeof(NodeStateSecurity));

  uint32_t maximum_command_delay = 0;
  attribute_store_get_child_reported(ep_node,
                                     STATE(MAXIMUM_COMMAND_DELAY),
                                     &maximum_command_delay,
                                     sizeof(uint32_t));

  char networkList[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  attribute_store_get_reported_string(
            attribute_store_get_first_child_by_type(node_id_node, STATE(NETWORK_LIST)),
            networkList, ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);

  std::string topic = "ucl/by-unid/";
  topic.append(unid);
  topic.append("/State");
  std::string cmd_delay;

  if (network_status >= NODE_STATE_TOPIC_LAST) {
    sl_log_debug(LOG_TAG, "Invalid topic state\n");
    return SL_STATUS_FAIL;
  }
  jsn["NetworkStatus"] = state_names[network_status];

  if (security_level >= NODES_STATE_TOPIC_SECURITY_LAST) {
    sl_log_debug(LOG_TAG, "Invalid topic state\n");
    return SL_STATUS_FAIL;
  }
  jsn["Security"] = security_names[security_level];

  if (maximum_command_delay == MAX_COMMAND_DELAY_UNKNOWN) {
    jsn["MaximumCommandDelay"] = R"("unknown")";
  } else if (maximum_command_delay == MAX_COMMAND_DELAY_INFINITE) {
    jsn["MaximumCommandDelay"] = R"("infinite")";
  } else {
    jsn["MaximumCommandDelay"] = maximum_command_delay;
  }

  jsn["NetworkList"] = nlohmann::json::array();
  std::vector<std::string> network_list;
  boost::algorithm::split(network_list, networkList, boost::is_any_of(","));
  for (auto network : network_list) {
    jsn["NetworkList"].push_back(network);
  }

  std::string message;
  try {
    // Payload contains data from end nodes, which we cannot control, thus we handle if there are non-utf8 characters
    message = jsn.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace);
  } catch (const nlohmann::json::exception &e) {
    sl_log_error(LOG_TAG, "State publish: Unable to parse JSON payload: %s", e.what());
    return SL_STATUS_FAIL;
  }

  uic_mqtt_publish(topic.c_str(), message.c_str(), message.size(), true);
  return SL_STATUS_OK;
}

// Publish the endpoint list per node when the node state topic is
// ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL
static void publish_endpoint_list(attribute_store_node_t node_id_node)
{
  char unid[MAXIMUM_UNID_SIZE]    = {};
  if (SL_STATUS_OK != unify_dotdot_attributes_get_unid()(node_id_node, unid)) {
    sl_log_warning(LOG_TAG,
                   "Cannot derive UNID from Attribute Store node %d. "
                   "Endpoint list will not be published.",
                   node_id_node);
    return;
  }

  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE;
  network_status = unify_attribute_store_node_state_get_status(node_id_node);

  if (network_status >= NODE_STATE_TOPIC_LAST) {
    sl_log_error(LOG_TAG, "Invalid topic state\n");
    return;
  }
  if (network_status != ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL) {
    return;
  }
  std::string topic_basic = "ucl/by-unid/";
  topic_basic.append(unid);

  // Get endpoint lists
  std::vector<dotdot_endpoint_id_t> endpoint_list;
  attribute attr_node(node_id_node);
  for (attribute endpoint_node: attr_node.children(unify_dotdot_attributes_endpoint_attribute())) {
    if (endpoint_node.is_valid() && endpoint_node.reported_exists()) {
      try {
        dotdot_endpoint_id_t ep = endpoint_node.reported<dotdot_endpoint_id_t>();
        endpoint_list.push_back(ep);
      } catch (std::exception &e) {
        sl_log_warning(
          LOG_TAG,
          "Failed to read the endpoint reported value for attribute  %s",
          e.what());
      }
    }
  }
  // Publish the EndpointidList
  uic_mqtt_dotdot_state_endpoint_id_list_publish(topic_basic.c_str(),
                                                 endpoint_list.size(),
                                                 &endpoint_list[0],
                                                 UCL_MQTT_PUBLISH_TYPE_ALL);
}

// Publish the endpoint list per node when the node state topic is
// ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL
static void publish_network_list(attribute_store_node_t node_id_node)
{
  char unid[MAXIMUM_UNID_SIZE]    = {};
  if (SL_STATUS_OK != unify_dotdot_attributes_get_unid()(node_id_node, unid)) {
    sl_log_warning(LOG_TAG,
                   "Cannot derive UNID from Attribute Store node %d. "
                   "Endpoint list will not be published.",
                   node_id_node);
    return;
  }

  std::string topic_basic = "ucl/by-unid/";
  topic_basic.append(unid);

  // Get network lists
  std::vector<const char *> network_list;
  attribute attr_node(node_id_node);
  char networkList[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  attribute_store_get_reported_string(
            attribute_store_get_first_child_by_type(node_id_node, STATE(NETWORK_LIST)),
            networkList, ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  std::vector<std::string> network_list_str;
  boost::algorithm::split(network_list_str, networkList, boost::is_any_of(","));
  for (auto network : network_list_str) {
    network_list.push_back(network.data());
  }

  // Publish the EndpointidList
  uic_mqtt_dotdot_state_network_list_publish(topic_basic.c_str(),
                                                 network_list.size(),
                                                 &network_list[0],
                                                 UCL_MQTT_PUBLISH_TYPE_ALL);
}

/**
 * @brief Helper function unretaining everything that has been published for a
 * UNID
 *
 * @param node  Attribute Store node for the Endpoint ID
 */
static void unretain_node_publications(attribute_store_node_t node)
{
  char unid[MAXIMUM_UNID_SIZE]    = {};

  if (SL_STATUS_OK != unify_dotdot_attributes_get_unid()(node, unid)) {
    sl_log_warning(LOG_TAG,
                   "Could not read UNID from Attribute Store Node %d. "
                   "Node publications will not be unretained.",
                   node);
    return;
  }

  std::string pattern = "ucl/by-unid/";
  pattern.append(unid);
  uic_mqtt_unretain(pattern.data());
}

/**
 * @brief Helper function unretaining everything that has been published for a Endpoint
 *
 * @param node  Attribute Store node for the Endpoint ID
 */
static void unretain_endpoint_publications(attribute_store_node_t endpoint_node)
{
  std::ostringstream pattern;
  dotdot_endpoint_id_t ep;
  char unid[MAXIMUM_UNID_SIZE]    = {};

  if (SL_STATUS_OK
      != unify_dotdot_attributes_get_unid_endpoint()(endpoint_node,
                                                     unid,
                                                     &ep)) {
    sl_log_warning(LOG_TAG,
                   "Could not read UNID/Endpoint ID from Attribute Store "
                   " Node %d. Node/Endpoint publications will not "
                   "be unretained.",
                   endpoint_node);
    return;
  }
  pattern << "ucl/by-unid/" << unid << "/ep"
          << static_cast<unsigned int>(ep);
  uic_mqtt_unretain(pattern.str().c_str());
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Store Callback functions
//////////////////////////////////////////////////////////////////////////////
/**
 * @brief callback function for attributes that affect the node Network State topic
 *
 * The following attributes will trigger this function
 * - DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS
 * - DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS
 * - DOTDOT_ATTRIBUTE_ID_STATE_MAXIMUM_COMMAND_DELAY
 * - DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_LIST
 *
 * Any update will trigger a new MQTT publication.
 */
static void
  on_pan_node_state_attribute_update(attribute_store_node_t updated_node,
                                     attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }
  // Same treatment for any of these attributes types
  // find the parent NodeID and republish its state
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 unify_dotdot_attributes_node_attribute());

  if (node_id_node != ATTRIBUTE_STORE_INVALID_NODE) {
    publish_node_state(node_id_node, true);
    publish_endpoint_list(node_id_node);
  }

  if (attribute_store_get_node_type(updated_node) == STATE(NETWORK_LIST))
  {
    publish_network_list(node_id_node);
  }
}

/**
 * @brief callback function for new NodeIDs updates in the network
 */
static void on_pan_node_update(attribute_store_node_t node_id_node,
                               attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    unretain_node_publications(node_id_node);
    return;
  }
  if (attribute_store_is_reported_defined(node_id_node)) {
    publish_node_state(node_id_node, true);
  }
}

/**
 * @brief callback function for if Endpoints ID are deleted
 */
static void on_endpoint_id_node_update(attribute_store_node_t endpoint_id_node,
                                       attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    unretain_endpoint_publications(endpoint_id_node);
    publish_endpoint_list(
      attribute_store_get_first_parent_with_type(endpoint_id_node,
                                                 unify_dotdot_attributes_node_attribute()));
  }
}

/**
 * @brief callback function for network status
 *
 * Listens to the Network Status and decides if we should publish
 * (updated) SupportedCommands and Node state topics.
 */
static void on_pan_node_network_status_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  // Check on the network status:
  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE;
  attribute_store_get_reported(updated_node,
                               &network_status,
                               sizeof(network_status));

  attribute_store::attribute node_id_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 unify_dotdot_attributes_node_attribute());


  dotdot_endpoint_id_t ep;
  char unid[MAXIMUM_UNID_SIZE]    = {};
  if (SL_STATUS_OK != unify_dotdot_attributes_get_unid()(node_id_node, unid)) {
    sl_log_warning(LOG_TAG,
                   "Cannot derive UNID from Attribute ID %d - "
                   "Supported Commands and Node States won't be published.",
                   node_id_node);
    return;
  }

  for (attribute_store::attribute endpoint_node:
       node_id_node.children(unify_dotdot_attributes_endpoint_attribute())) {
    if (SL_STATUS_OK
        != unify_dotdot_attributes_get_unid_endpoint()(endpoint_node,
                                                       unid,
                                                       &ep)) {
      return;
    }

    if ((network_status == ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_NON_FUNCTIONAL)
        || (network_status == ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING)
        || (network_status == ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE)) {
      // Clear the SupportedCommands
      uic_mqtt_dotdot_publish_empty_supported_commands(unid, ep);
    } else if (network_status == ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL) {
      uic_mqtt_dotdot_publish_supported_commands(unid, ep);
    }
  }

  uic_mqtt_dotdot_state_publish_supported_commands(unid, 0);
  on_pan_node_state_attribute_update(updated_node, change);
}

///////////////////////////////////////////////////////////////////////////////
// Functions shared within the component.
//////////////////////////////////////////////////////////////////////////////
/**
 * @brief Callback function for configuration updates
 *
 * It will register callbacks to the ENDPOINT / NODE attributes types in the
 * attribute store.
 */
void on_unify_dotdot_attribute_store_configuration_update()
{
  if (unify_dotdot_attributes_node_attribute()
      != ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE) {
    attribute_store_register_callback_by_type_and_state(
      &on_pan_node_update,
      unify_dotdot_attributes_node_attribute(),
      REPORTED_ATTRIBUTE);
  }

  if (unify_dotdot_attributes_endpoint_attribute()
      != ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE) {
    attribute_store_register_callback_by_type_and_state(
      &on_endpoint_id_node_update,
      unify_dotdot_attributes_endpoint_attribute(),
      REPORTED_ATTRIBUTE);
  }
}

///////////////////////////////////////////////////////////////////////////////
//  Init and teardown functions.
//////////////////////////////////////////////////////////////////////////////
static sl_status_t ucl_node_state_init()
{
  // Registers callbacks to Endpoint/Node attributes.
  unify_dotdot_attribute_store_set_configuration_update_callback(
    &on_unify_dotdot_attribute_store_configuration_update);
  on_unify_dotdot_attribute_store_configuration_update();

  attribute_store_register_callback_by_type_and_state(
    &on_pan_node_network_status_attribute_update,
    STATE(NETWORK_STATUS),
    REPORTED_ATTRIBUTE);

  attribute_store_register_callback_by_type_and_state(
    &on_pan_node_state_attribute_update,
    STATE(SECURITY),
    REPORTED_ATTRIBUTE);

  attribute_store_register_callback_by_type_and_state(
    &on_pan_node_state_attribute_update,
    STATE(MAXIMUM_COMMAND_DELAY),
    REPORTED_ATTRIBUTE);
  attribute_store_register_callback_by_type_and_state(
    &on_pan_node_state_attribute_update,
    STATE(NETWORK_LIST),
    REPORTED_ATTRIBUTE);

  return SL_STATUS_OK;
}

static void ucl_node_state_teardown()
{
  // get node list and publish the node state unavailable
  attribute::root().visit(
    [](attribute node, int depth) {
      if( node.type() == unify_dotdot_attributes_node_attribute() ) {
        publish_node_state(node, false);
        return SL_STATUS_FAIL;
      }
      return SL_STATUS_OK;
    }
  );

  //Remove all retained topics except ucl/by-unid/<xxxxx>/State and ucl/by-mqtt-client
  uic_mqtt_unretain_by_regex(REGEX_NOT_STATE_OR_MQTT_CLIENT_TOPICS);
}

///////////////////////////////////////////////////////////////////////////////
//  Public/shared functions.
//////////////////////////////////////////////////////////////////////////////
NodeStateNetworkStatus
  unify_attribute_store_node_state_get_status(attribute_store_node_t node_id_node)
{
  // Default to UNAVAILABLE if the value is undefined in the attribute store
  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE;

  node_id_node = attribute(node_id_node).first_parent_or_self(unify_dotdot_attributes_node_attribute());
  attribute_store_get_child_reported(node_id_node,
                                     STATE(NETWORK_STATUS),
                                     &network_status,
                                     sizeof(network_status));

  return network_status;
}

///////////////////////////////////////////////////////////////////////////////
// Contiki process
//////////////////////////////////////////////////////////////////////////////
PROCESS(unify_dotdot_attribute_store_node_state_process, "ucl_node_state_process");

sl_status_t unify_dotdot_attribute_store_node_state_init()
{
  ucl_node_state_init();
  process_start(&unify_dotdot_attribute_store_node_state_process,NULL);
  return SL_STATUS_OK;
}

PROCESS_THREAD(unify_dotdot_attribute_store_node_state_process, ev, data)
{
  PROCESS_BEGIN();
  while (1) {
    try {
      switch (ev) {
        case PROCESS_EVENT_EXIT:
          ucl_node_state_teardown();
          break;
        default:
          break;
      }
    } catch (const std::exception &exc) {
      sl_log_error(LOG_TAG, "Caught exception: %s", exc.what());
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
