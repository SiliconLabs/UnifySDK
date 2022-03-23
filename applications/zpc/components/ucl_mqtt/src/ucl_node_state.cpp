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
#include "ucl_node_state.h"
#include "ucl_mqtt_node_interview.h"
#include "ucl_network_management.h"

//Include from other components
#include "attribute_store.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_store_helper.h"
#include "zpc_attribute_store.h"
#include "zpc_attribute_store_network_helper.h"
#include "sl_log.h"
#include "sl_status.h"
#include "uic_mqtt.h"
#include "attribute.hpp"
#include "zwave_command_class_wake_up_types.h"
#include "zwave_utils.h"
#include "dotdot_mqtt.h"

// Interfaces (only using typedefs from here)
#include "zwave_controller_keyset.h"
#include "ucl_definitions.h"

//Generic include
#include <stddef.h>
#include <map>
#include <vector>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <string>

#include <boost/algorithm/string.hpp>

// Includes from auto-generated files
#include "dotdot_mqtt.h"

using namespace attribute_store;
constexpr char LOG_TAG[] = "ucl_node_state_topic";

/**
 * @brief Name the of @ref contiki process for the Ucl Network Management.
 *
 * This is used to register the name of the Ucl Network Management Process.
 */
PROCESS_NAME(ucl_network_management_process);

////////////////////////////////////////////////////////////////////////////////
// Defines and types
////////////////////////////////////////////////////////////////////////////////

// Theese strings must follow definitions in node_state_topic_state_t
static const char *state_names[] = {
  "Online interviewing",  // NODE_STATE_TOPIC_STATE_NODEID_ASSIGNED
  "Online functional",    // NODE_STATE_TOPIC_STATE_INCLUDED
  "Offline",              // NODE_STATE_TOPIC_STATE_INTERVIEW_FAIL
  "Online interviewing",  // NODE_STATE_TOPIC_INTERVIEWING
  "Offline",              // NODE_STATE_TOPIC_STATE_OFFLINE
  "Unavailable",          // NODE_STATE_TOPIC_STATE_UNAVAILABLE
};

static_assert(sizeof(state_names) / sizeof(char *) == NODE_STATE_TOPIC_LAST,
              "state_names and node_state_topic_states_t shall be same length");

// Theese strings must follow definitions in node_state_topic_security_t
static const char *security_names[] = {
  "None",
  "Z-Wave S2 Unauthenticated",
  "Z-Wave S2 Authenticated",
  "Z-Wave S2 Access Control",
  "Z-Wave S0",
};

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////
static node_state_topic_security_t
  get_node_state_security_value(zwave_keyset_t keys)
{
  if (keys & ZWAVE_CONTROLLER_S2_ACCESS_KEY) {
    return NODES_STATE_TOPIC_SECURITY_S2_ACCESS;
  }
  if (keys & ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY) {
    return NODES_STATE_TOPIC_SECURITY_S2_AUTHENTICATED;
  }
  if (keys & ZWAVE_CONTROLLER_S2_UNAUTHENTICATED_KEY) {
    return NODES_STATE_TOPIC_SECURITY_S2_UNAUTHENTICATED;
  }
  if (keys & ZWAVE_CONTROLLER_S0_KEY) {
    return NODES_STATE_TOPIC_SECURITY_S0;
  }

  return NODES_STATE_TOPIC_SECURITY_NONE;
}

static_assert(
  sizeof(security_names) / sizeof(char *) == NODES_STATE_TOPIC_SECURITY_LAST,
  "security_names and node_state_topic_security_t shall be same length");

static node_state_topic_security_t
  get_node_security(attribute_store_node_t node_id_node)
{
  attribute_store_node_t granted_keys_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                             0);

  zwave_keyset_t granted_keys = 0;
  attribute_store_read_value(granted_keys_node,
                             REPORTED_ATTRIBUTE,
                             &granted_keys,
                             sizeof(zwave_keyset_t));

  return get_node_state_security_value(granted_keys);
}

static wake_up_interval_t
  get_nl_node_maximum_command_delay(attribute_store_node_t node_id_node)
{
  zwave_endpoint_id_t root_device = 0;
  attribute_store_node_t endpoint_node
    = attribute_store_get_node_child_by_value(node_id_node,
                                              ATTRIBUTE_ENDPOINT_ID,
                                              REPORTED_ATTRIBUTE,
                                              &root_device,
                                              sizeof(zwave_endpoint_id_t),
                                              0);
  attribute_store_node_t wake_up_setting_node
    = attribute_store_get_node_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_WAKE_UP_SETTING,
      0);

  attribute_store_node_t wake_up_interval_node
    = attribute_store_get_node_child_by_type(
      wake_up_setting_node,
      ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL,
      0);

  if (wake_up_interval_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return MAX_COMMAND_DELAY_UNKNOWN;
  }

  wake_up_interval_t wake_up_interval = 0;
  if (SL_STATUS_OK
      != attribute_store_read_value(wake_up_interval_node,
                                    REPORTED_ATTRIBUTE,
                                    &wake_up_interval,
                                    sizeof(wake_up_interval_t))) {
    return MAX_COMMAND_DELAY_UNKNOWN;
  }

  if (wake_up_interval == 0) {
    return MAX_COMMAND_DELAY_INFINITE;
  } else {
    return wake_up_interval;
  }
}

static uint32_t
  get_node_maximum_command_delay(attribute_store_node_t node_id_node)
{
  // ZPC has no command delay. Lightning fast :-)
  if (node_id_node == get_zpc_node_id_node()) {
    return 0;
  }

  zwave_node_id_t node_id = 0;
  attribute_store_read_value(node_id_node,
                             REPORTED_ATTRIBUTE,
                             &node_id,
                             sizeof(zwave_node_id_t));

  if (node_id == 0) {
    return MAX_COMMAND_DELAY_UNKNOWN;
  }

  switch (zwave_get_operating_mode(node_id)) {
    case OPERATING_MODE_AL:
      return 1;
    case OPERATING_MODE_FL:
      return 5;
    case OPERATING_MODE_NL:
      return get_nl_node_maximum_command_delay(node_id_node);

    default:
      return MAX_COMMAND_DELAY_UNKNOWN;
  }
}

///////////////////////////////////////////////////////////////////////////////
// MQTT callback functions
//////////////////////////////////////////////////////////////////////////////
static sl_status_t ucl_node_state_interview_command(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  const uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  if (true == is_zpc_unid(unid)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type) {
    return SL_STATUS_OK;
  }

  return ucl_mqtt_initiate_node_interview(unid);
}

static sl_status_t ucl_node_state_remove_offline_command(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  const uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  if (true == is_zpc_unid(unid)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type) {
    return SL_STATUS_OK;
  }

  zwave_node_id_t node_id = 0x00;
  if (SL_STATUS_OK == zwave_unid_to_node_id(unid, &node_id)) {
    return ucl_network_management_remove_offline_node(node_id);
  }

  return SL_STATUS_OK;
}

static sl_status_t ucl_node_state_discover_neighbors_command(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  const uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  if (true == is_zpc_unid(unid)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type) {
    return SL_STATUS_OK;
  }

  zwave_node_id_t node_id = 0x00;
  if (SL_STATUS_OK == zwave_unid_to_node_id(unid, &node_id)) {
    return ucl_network_management_request_node_neighbor_update(node_id);
  }
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// MQTT Publication functions
//////////////////////////////////////////////////////////////////////////////
static sl_status_t publish_node_state(attribute_store_node_t node_id_node,
                                      bool available)
{
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_UNAVAILABLE;
  if (available == true) {
    network_status = get_node_network_status(node_id_node);
  }
  node_state_topic_security_t security = get_node_security(node_id_node);
  uint32_t maximum_command_delay = get_node_maximum_command_delay(node_id_node);

  std::string topic = "ucl/by-unid/";
  unid_t unid;
  attribute_store_network_helper_get_unid_from_node(node_id_node, unid);
  topic.append(unid);
  topic.append("/State");
  std::string cmd_delay;

  if (network_status >= NODE_STATE_TOPIC_LAST) {
    sl_log_debug(LOG_TAG, "Invalid topic state\n");
    return SL_STATUS_FAIL;
  }
  if (security >= NODES_STATE_TOPIC_SECURITY_LAST) {
    sl_log_debug(LOG_TAG, "Invalid topic security\n");
    return SL_STATUS_FAIL;
  }

  std::string message = std::string(R"({"NetworkStatus": ")");
  message.append(state_names[network_status]);
  message.append(std::string(R"(", "Security": ")"));
  message.append(security_names[security]);
  message.append(std::string(R"(", "MaximumCommandDelay": )"));
  if (maximum_command_delay == MAX_COMMAND_DELAY_UNKNOWN) {
    message.append(R"("unknown")");
  } else if (maximum_command_delay == MAX_COMMAND_DELAY_INFINITE) {
    message.append(R"("infinite")");
  } else {
    message.append(std::to_string(maximum_command_delay));
  }
  message.append("}");

  uic_mqtt_publish(topic.c_str(), message.c_str(), message.size(), true);
  return SL_STATUS_OK;
}

// Publish the endpoint list per node when the node state topic is
// NODE_STATE_TOPIC_STATE_INCLUDED
static void publish_endpoint_list(attribute_store_node_t node_id_node)
{
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_UNAVAILABLE;
  network_status = get_node_network_status(node_id_node);

  if (network_status >= NODE_STATE_TOPIC_LAST) {
    sl_log_error(LOG_TAG, "Invalid topic state\n");
    return;
  }
  if (network_status != NODE_STATE_TOPIC_STATE_INCLUDED) {
    return;
  }
  std::string topic_basic = "ucl/by-unid/";
  unid_t unid;
  attribute_store_network_helper_get_unid_from_node(node_id_node, unid);
  topic_basic.append(unid);
  // Get endpoint lists
  std::vector<zwave_endpoint_id_t> endpoint_list;
  attribute attr_node(node_id_node);
  for (attribute endpoint_node: attr_node.children(ATTRIBUTE_ENDPOINT_ID)) {
    if (endpoint_node.reported_exists()) {
      try {
        zwave_endpoint_id_t ep = endpoint_node.reported<zwave_endpoint_id_t>();
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

static void unretain_node_publications(attribute_store_node_t node)
{
  unid_t unid;
  attribute_store_network_helper_get_unid_from_node(node, unid);
  std::string pattern = "ucl/by-unid/";
  pattern.append(unid);
  pattern.append("/State");
  uic_mqtt_unretain(pattern.data());
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Store Callback functions
//////////////////////////////////////////////////////////////////////////////
/**
 * @brief callback function for attributes that affect the node Network State topic
 *
 * The following attributes will trigger this function
 * - ATTRIBUTE_GRANTED_SECURITY_KEYS
 * - ATTRIBUTE_NETWORK_STATUS
 * - ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL
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
                                                 ATTRIBUTE_NODE_ID);

  if (node_id_node != ATTRIBUTE_STORE_INVALID_NODE) {
    publish_node_state(node_id_node, true);
    publish_endpoint_list(node_id_node);
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
  if (attribute_store_is_value_defined(node_id_node, REPORTED_ATTRIBUTE)) {
    publish_node_state(node_id_node, true);
  }
}

///////////////////////////////////////////////////////////////////////////////
//  Init and teardown functions.
//////////////////////////////////////////////////////////////////////////////
sl_status_t ucl_node_state_init()
{
  attribute_store_register_callback_by_type_and_state(on_pan_node_update,
                                                      ATTRIBUTE_NODE_ID,
                                                      REPORTED_ATTRIBUTE);

  attribute_store_register_callback_by_type_and_state(
    on_pan_node_state_attribute_update,
    ATTRIBUTE_GRANTED_SECURITY_KEYS,
    REPORTED_ATTRIBUTE);

  attribute_store_register_callback_by_type_and_state(
    on_pan_node_state_attribute_update,
    ATTRIBUTE_NETWORK_STATUS,
    REPORTED_ATTRIBUTE);

  attribute_store_register_callback_by_type_and_state(
    on_pan_node_state_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL,
    REPORTED_ATTRIBUTE);

  uic_mqtt_dotdot_state_interview_callback_set(
    ucl_node_state_interview_command);

  uic_mqtt_dotdot_state_remove_offline_callback_set(
    ucl_node_state_remove_offline_command);

  uic_mqtt_dotdot_state_discover_neighbors_callback_set(
    ucl_node_state_discover_neighbors_command);

  return SL_STATUS_OK;
}

void ucl_node_state_teardown()
{
  // get node list and publish the node state unavailable
  attribute_store_node_t home_id_node = get_zpc_network_node();
  attribute_store_node_t node_id_node = ATTRIBUTE_STORE_INVALID_NODE;
  size_t num_child = attribute_store_get_node_child_count(home_id_node);
  for (size_t i = 0; i < num_child; i++) {
    node_id_node = attribute_store_get_node_child_by_type(home_id_node,
                                                          ATTRIBUTE_NODE_ID,
                                                          i);
    publish_node_state(node_id_node, false);
  }

  //Remove all retained topics except ucl/by-unid/<xxxxx>/State
  uic_mqtt_unretain_by_regex("^(?!ucl\\/by-unid\\/.*\\/State$).*");
}

///////////////////////////////////////////////////////////////////////////////
//  Public/shared functions.
//////////////////////////////////////////////////////////////////////////////
node_state_topic_state_t
  get_node_network_status(attribute_store_node_t node_id_node)
{
  // Default to UNAVAILABLE if the value is undefined in the attribute store
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_UNAVAILABLE;

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
// Contiki process
//////////////////////////////////////////////////////////////////////////////

PROCESS(ucl_node_state_process, "ucl_node_state_process");

PROCESS_THREAD(ucl_node_state_process, ev, data)
{
  PROCESS_BEGIN();
  while (1) {
    try {
      switch (ev) {
        case PROCESS_EVENT_INIT:
          ucl_node_state_init();
          break;
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
