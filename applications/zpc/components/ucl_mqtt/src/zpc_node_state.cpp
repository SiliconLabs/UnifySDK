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

//Include from this component
#include "unify_dotdot_attribute_store_node_state.h"
#include "ucl_mqtt_node_interview.h"
#include "ucl_network_management.h"
#include "zpc_node_state.h"

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
#include "zwave_utils.h"
#include "dotdot_mqtt.h"

///////////////////////////////////////////////////////////////////////////////
// MQTT callback functions
//////////////////////////////////////////////////////////////////////////////
/**
 * @brief Performs a neighbor discovery for a Unid/Endpoint
 *
 * @param unid          UNID of the node to perform neighbor discovery
 * @param endpoint      Endpoint of the node. The State topic does not support
 *                      endpoints so this variable will be ignored.
 * @param callback_type  Dotdot MQTT Callback type
 *
 * @returns SL_STATUS_OK/SL_STATUS_FAIL to indicate support if callback_type is set to UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK
 * sl_status_t code indicating the outcome of applying the command if callback_type is set to UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL
*/
static sl_status_t ucl_node_state_discover_neighbors_command(
  const dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  if (true == is_zpc_unid(unid)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type) {
    // DiscoverNeighbors always supported unless the node is offline/unavailale
    NodeStateNetworkStatus network_status = unify_attribute_store_node_state_get_status(
      attribute_store_network_helper_get_node_id_node(unid)
    );

    sl_status_t support                     = SL_STATUS_OK;
    if ((network_status == ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE)
        || (network_status == ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE)) {
      support = SL_STATUS_FAIL;
    }
    return support;
  }

  zwave_node_id_t node_id = 0x00;
  if (SL_STATUS_OK == zwave_unid_to_node_id(unid, &node_id)) {
    return ucl_network_management_request_node_neighbor_update(node_id);
  }
  return SL_STATUS_OK;
}

/**
 * @brief Performs a remove offline operation for a Unid/Endpoint
 *
 * @param unid          UNID of the node to attempt to remove due to being offline
 * @param endpoint      Endpoint of the node. The State topic does not support
 *                      endpoints so this variable will be ignored.
 * @param callback_type  Dotdot MQTT Callback type
 *
 * @returns SL_STATUS_OK/SL_STATUS_FAIL to indicate support if callback_type is set to UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK
 * sl_status_t code indicating the outcome of applying the command if callback_type is set to UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL
*/
static sl_status_t ucl_node_state_remove_offline_command(
  const dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  if (true == is_zpc_unid(unid)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type) {
    // RemoveOffline supported unless the node is unavailable
    NodeStateNetworkStatus network_status = unify_attribute_store_node_state_get_status(
      attribute_store_network_helper_get_node_id_node(unid)
    );

    sl_status_t support                     = SL_STATUS_OK;
    if (network_status == ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE) {
      support = SL_STATUS_FAIL;
    }
    return support;
  }

  zwave_node_id_t node_id = 0x00;
  if (SL_STATUS_OK == zwave_unid_to_node_id(unid, &node_id)) {
    return ucl_network_management_remove_offline_node(node_id);
  }

  return SL_STATUS_OK;
}

/**
 * @brief Performs a new interview operation for a Unid/Endpoint
 *
 * @param unid          UNID of the node to attempt to remove due to being offline
 * @param endpoint      Endpoint of the node. The State topic does not support
 *                      endpoints so this variable will be ignored.
 *                      is performed on a NodeID level, as all endpoints share the same radio.
 * @param callback_type  Dotdot MQTT Callback type
 *
 * @returns SL_STATUS_OK/SL_STATUS_FAIL to indicate support if callback_type is set to UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK
 * sl_status_t code indicating the outcome of applying the command if callback_type is set to UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL
*/
static sl_status_t ucl_node_state_interview_command(
  const dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  if (true == is_zpc_unid(unid)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type) {
      NodeStateNetworkStatus network_status = unify_attribute_store_node_state_get_status(
      attribute_store_network_helper_get_node_id_node(unid)
    );

    // Interview always supported unless the node is offline/unavailable
    sl_status_t support                     = SL_STATUS_OK;
    if ((network_status == ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE)
        || (network_status == ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE)) {
      support = SL_STATUS_FAIL;
    }
    return support;
  }

  return ucl_mqtt_initiate_node_interview(unid);
}

/**
 * @brief Perform an S0/S2 key discovery for a Unid/Endpoint
 *
 * @param unid          UNID of the node to attempt to remove due to being offline
 * @param endpoint      Endpoint of the node. The State topic does not support
 *                      endpoints so this variable will be ignored.
 *                      is performed on a NodeID level, as all endpoints share the same radio.
 * @param callback_type  Dotdot MQTT Callback type
 *
 * @returns SL_STATUS_OK/SL_STATUS_FAIL to indicate support if callback_type is set to UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK
 * sl_status_t code indicating the outcome of applying the command if callback_type is set to UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL
*/
static sl_status_t ucl_node_state_discover_security_command(
  const dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  if (true == is_zpc_unid(unid)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  attribute_store_node_t node_id_node
    = attribute_store_network_helper_get_node_id_node(unid);

  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type) {
    NodeStateNetworkStatus network_status
      = unify_attribute_store_node_state_get_status(node_id_node);

    // Interview always supported unless the node is offline/unavailable
    sl_status_t support = SL_STATUS_OK;
    if ((network_status == ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE)
        || (network_status == ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE)) {
      support = SL_STATUS_FAIL;
    }
    return support;
  }

  // Undefine the granted keys, so they get discovered again.
  attribute_store_node_t granted_keys
    = attribute_store_get_first_child_by_type(node_id_node,
                                              ATTRIBUTE_GRANTED_SECURITY_KEYS);
  attribute_store_undefine_reported(granted_keys);
  return attribute_store_node_exists(granted_keys) ? SL_STATUS_OK
                                                   : SL_STATUS_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// Init function shared with the component.
//////////////////////////////////////////////////////////////////////////////
sl_status_t zpc_node_state_init()
{
  uic_mqtt_dotdot_state_remove_offline_callback_set(
    &ucl_node_state_remove_offline_command);

  uic_mqtt_dotdot_state_discover_neighbors_callback_set(
    &ucl_node_state_discover_neighbors_command);

  uic_mqtt_dotdot_state_interview_callback_set(
    &ucl_node_state_interview_command);

  uic_mqtt_dotdot_state_discover_security_callback_set(
    &ucl_node_state_discover_security_command);

  return SL_STATUS_OK;
}
