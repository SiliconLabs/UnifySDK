/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "eed_node_state.h"

// EED Components
#include "eed_attribute_store_helper.h"
#include "eed_attribute_store_attribute_types.h"

// Unify components
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "unify_dotdot_attribute_store_node_state.h"
#include "attribute_timeouts.h"
#include "dotdot_mqtt.h"

#define STATE(type) DOTDOT_ATTRIBUTE_ID_STATE_##type

// Time that it takes to interview a node, in seconds
#define INTERVIEW_TIME 5 * 1000

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
static void return_to_online_functional(attribute_store_node_t endpoint_node)
{
  NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_node_t unid_node
      = attribute_store_get_first_parent_with_type(endpoint_node,
                                                   ATTRIBUTE_UNID);
  attribute_store_set_child_reported(unid_node,
                                     DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));
  attribute_store_set_child_desired(unid_node,
                                    DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                    &network_status,
                                    sizeof(network_status));
}

static void on_endpoint_created(attribute_store_node_t endpoint_node,
                                attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  dotdot_endpoint_id_t endpoint_id = 99;
  attribute_store_get_reported(endpoint_node,
                               &endpoint_id,
                               sizeof(endpoint_id));
  if (endpoint_id != 0) {
    attribute_store_node_t unid_node
      = attribute_store_get_first_parent_with_type(endpoint_node,
                                                   ATTRIBUTE_UNID);
    endpoint_id   = 0;
    endpoint_node = attribute_store_emplace(unid_node,
                                            ATTRIBUTE_ENDPOINT_ID,
                                            &endpoint_id,
                                            sizeof(endpoint_id));
  }

  // Set all the attributes with a default value
  attribute_store_create_child_if_missing(endpoint_node,
                                          STATE(ENDPOINT_ID_LIST));

  attribute_store_node_t unid_node
      = attribute_store_get_first_parent_with_type(endpoint_node,
                                                   ATTRIBUTE_UNID);
  const NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING;
  attribute_store_set_child_desired(unid_node,
                                    STATE(NETWORK_STATUS),
                                    &network_status,
                                    sizeof(network_status));
  attribute_store_set_child_reported(unid_node,
                                     STATE(NETWORK_STATUS),
                                     &network_status,
                                     sizeof(network_status));
  attribute_timeout_set_callback(endpoint_node,
                                 INTERVIEW_TIME,
                                 &return_to_online_functional);

  const NodeStateSecurity security = ZCL_NODE_STATE_SECURITY_NONE;
  attribute_store_set_child_desired(endpoint_node,
                                    STATE(SECURITY),
                                    &security,
                                    sizeof(security));
  attribute_store_set_child_reported(endpoint_node,
                                     STATE(SECURITY),
                                     &security,
                                     sizeof(security));

  const uint32_t maximum_command_delay = 1;
  attribute_store_set_child_desired(endpoint_node,
                                    STATE(MAXIMUM_COMMAND_DELAY),
                                    &maximum_command_delay,
                                    sizeof(maximum_command_delay));
  attribute_store_set_child_reported(endpoint_node,
                                     STATE(MAXIMUM_COMMAND_DELAY),
                                     &maximum_command_delay,
                                     sizeof(maximum_command_delay));

  attribute_store_node_t network_list_node
    = attribute_store_create_child_if_missing(endpoint_node,
                                              STATE(NETWORK_LIST));
  attribute_store_set_reported_string(network_list_node, "");
}

///////////////////////////////////////////////////////////////////////////////
// MQTT callback functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t ucl_node_state_interview_command(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  const uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  attribute_store_node_t unid_node = eed_attribute_store_get_unid_node(unid);

  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type) {
    NodeStateNetworkStatus network_status
      = unify_attribute_store_node_state_get_status(unid_node);

    // Interview always supported unless the node is offline/unavailable
    sl_status_t support = SL_STATUS_OK;
    if ((network_status == ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE)
        || (network_status == ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE)) {
      support = SL_STATUS_FAIL;
    }
    return support;
  }

  const dotdot_endpoint_id_t endpoint_id_0 = 0;
  attribute_store_node_t endpoint_node
    = attribute_store_emplace(unid_node,
                              ATTRIBUTE_ENDPOINT_ID,
                              &endpoint_id_0,
                              sizeof(endpoint_id_0));

  // Interview will just take 10 seconds
  NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING;
  attribute_store_set_child_reported(endpoint_node,
                                     DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));
  attribute_store_set_child_desired(endpoint_node,
                                    DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                    &network_status,
                                    sizeof(network_status));

  return attribute_timeout_set_callback(endpoint_node,
                                        INTERVIEW_TIME,
                                        &return_to_online_functional);
}

static sl_status_t ucl_node_state_remove_offline_command(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  const uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  attribute_store_node_t unid_node = eed_attribute_store_get_unid_node(unid);
  NodeStateNetworkStatus network_status
    = unify_attribute_store_node_state_get_status(unid_node);

  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type) {
    // RemoveOffline supported unless the node is unavailable
    sl_status_t support = SL_STATUS_OK;
    if (network_status == ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE) {
      support = SL_STATUS_FAIL;
    }
    return support;
  }

  // Delete the node if it is offline
  if (network_status == ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE) {
    attribute_store_delete_node(unid_node);
  }
  return SL_STATUS_OK;
}

static sl_status_t ucl_node_state_remove_command(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  const uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  attribute_store_node_t unid_node = eed_attribute_store_get_unid_node(unid);
  NodeStateNetworkStatus network_status
    = unify_attribute_store_node_state_get_status(unid_node);

  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type) {
    // RemoveOffline supported unless the node is unavailable
    sl_status_t support = SL_STATUS_OK;
    if (network_status == ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE) {
      support = SL_STATUS_FAIL;
    }
    return support;
  }

  // Delete the node no matter what
  attribute_store_delete_node(unid_node);
  return SL_STATUS_OK;
}

sl_status_t eed_node_state_init()
{
  // Automatically provide a state for UNIDs being created
  attribute_store_register_callback_by_type(&on_endpoint_created,
                                            ATTRIBUTE_ENDPOINT_ID);

  // Register command callbacks for the UCL Node state
  uic_mqtt_dotdot_state_remove_callback_set(&ucl_node_state_remove_command);

  uic_mqtt_dotdot_state_remove_offline_callback_set(
    &ucl_node_state_remove_offline_command);

  uic_mqtt_dotdot_state_interview_callback_set(
    &ucl_node_state_interview_command);

  return SL_STATUS_OK;
}
