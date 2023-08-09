/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>

// Unify shared includes
#include <sl_log.h>
#include <sl_status.h>

// ZigPC includes
#include <zigpc_datastore.h>
#include <zigpc_discovery.h>
#include <zigpc_gateway.h>

#include "zigpc_net_mgmt_notify_int.h"
#include "zigpc_net_mgmt_internal.h"

static const char LOG_TAG[] = "zigpc_net_mgmt_internal";

sl_status_t zigpc_net_mgmt_init_complete(
  const zigbee_eui64_t zigpc_eui64,                 /**< Gateway EUI64 */
  zigbee_endpoint_id_t zigpc_endpoint_id,     /**< Gateway Endpoint ID */
  const zigbee_panid_t zigpc_panid,                 /**< Network PAN ID */
  const zigbee_ext_panid_t zigpc_ext_panid,         /**< Network extended Pan ID */
  zigbee_radio_channel_t zigpc_radio_channel) /**<  Network radio channel */
{
  sl_status_t status = SL_STATUS_OK;


    zigpc_network_data_t nwk;

    memcpy(nwk.gateway_eui64, zigpc_eui64, sizeof(zigbee_eui64_t));
    nwk.panid = zigpc_panid;
    memcpy(nwk.ext_panid, zigpc_ext_panid, sizeof(zigbee_ext_panid_t));
    nwk.radio_channel = zigpc_radio_channel;

    zigpc_datastore_create_network();
    status = zigpc_datastore_write_network(&nwk);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG, "Unable to store network data: 0x%X", status);
    }

  // Create a device entity for the gateway radio
  if (status == SL_STATUS_OK) {
    // Remove any persisted stale device data, if exists
    zigpc_datastore_remove_device(zigpc_eui64);
    status = zigpc_datastore_create_device(zigpc_eui64);
    if (status == SL_STATUS_OK) {
      // Since there is no discovery needed for knowing endpoints supported
      // on the gateway, set the total and discovered fields to 1
      const uint8_t zigpc_endpoint_count = 1;

      zigpc_device_data_t dev = {
        .network_status            = ZIGBEE_NODE_STATUS_INCLUDED,
        .max_cmd_delay             = 0,
        .endpoint_total_count      = zigpc_endpoint_count,
        .endpoint_discovered_count = zigpc_endpoint_count,
      };
      status = zigpc_datastore_write_device(zigpc_eui64, &dev);
    }
    if (status == SL_STATUS_OK) {
      status = zigpc_datastore_create_endpoint(zigpc_eui64,
                                               zigpc_endpoint_id);
    }
  }

  if (status == SL_STATUS_OK) {
    zigpc_datastore_log_network(LOG_TAG, SL_LOG_INFO, "Network info");

    sl_log_debug(LOG_TAG, "PC PANID: 0x%04X", zigpc_panid);
    sl_log_debug(LOG_TAG, "PC Channel: 0x%02X", zigpc_radio_channel);

    zigpc_net_mgmt_notify_network_init(zigpc_eui64);

    enum zigpc_net_mgmt_fsm_state next_supported_states[] = 
    {
        ZIGPC_NET_MGMT_FSM_STATE_IDLE,
        ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD,
        ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE,
        ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL
    };

    zigpc_net_mgmt_notify_state_update(ZIGPC_NET_MGMT_FSM_STATE_IDLE, next_supported_states);

  }

  return status;
}

sl_status_t zigpc_net_mgmt_hdl_idle_add_request( 
  const zigbee_eui64_t eui64,
  const zigbee_install_code_t install_code,
  uint8_t install_code_length,
  bool is_well_known_key_add)
{
    sl_status_t status = 
        zigpc_gateway_add_node(
            eui64,
            install_code,
            install_code_length,
            is_well_known_key_add);

    enum zigpc_net_mgmt_fsm_state next_supported_states[] = 
    {
        ZIGPC_NET_MGMT_FSM_STATE_IDLE,
        ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL
    };

    zigpc_net_mgmt_notify_state_update(
            ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD, 
            next_supported_states);

  return status;
}

sl_status_t zigpc_net_mgmt_node_add_complete(const zigbee_eui64_t node_eui64)
{
  sl_status_t status                      = SL_STATUS_OK;


    // Remove any persisted stale device data, if exists
    zigpc_datastore_remove_device(node_eui64);
    status = zigpc_datastore_create_device(node_eui64);
  

  if (status == SL_STATUS_OK) {
    zigpc_device_data_t dev = {
      .network_status            = ZIGBEE_NODE_STATUS_NODEID_ASSIGNED,
      .max_cmd_delay             = 1,
      .endpoint_total_count      = 0,
      .endpoint_discovered_count = 0,
    };
    status = zigpc_datastore_write_device(node_eui64, &dev);
  }

  if (status == SL_STATUS_OK) {
    zigpc_datastore_log_device(LOG_TAG,
                               SL_LOG_DEBUG,
                               "Device Added (Pre-Interview)",
                               node_eui64);

    zigpc_net_mgmt_notify_node_added(node_eui64);
    
    enum zigpc_net_mgmt_fsm_state next_supported_states[] = 
    {
        ZIGPC_NET_MGMT_FSM_STATE_IDLE,
        ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD,
        ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE,
        ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL
    };

    zigpc_net_mgmt_notify_state_update(ZIGPC_NET_MGMT_FSM_STATE_IDLE, next_supported_states);
    }

  return status;
}

sl_status_t zigpc_net_mgmt_publish_interview_status(
  const zigbee_eui64_t eui64,
  zigpc_discovery_status_t discovery_status)
{

  if(DISCOVERY_START != discovery_status)
  {
      bool success = (DISCOVERY_SUCCESS == discovery_status);
      zigpc_net_mgmt_notify_node_interview_status(eui64, success);
  }
    
  sl_log_info(
          LOG_TAG, 
          "Discovery status received %d",
          discovery_status);

  return SL_STATUS_OK;
}

sl_status_t zigpc_net_mgmt_hdl_idle_remove_request(const zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;

    zigpc_device_data_t dev;

    status = zigpc_datastore_read_device(eui64, &dev);
    if (status == SL_STATUS_OK) {
      dev.network_status = ZIGBEE_NODE_STATUS_UNAVAILABLE;
      status             = zigpc_datastore_write_device(eui64, &dev);
    }
  

  if (status == SL_STATUS_OK) {
    zigpc_gateway_remove_node(eui64);
    
    enum zigpc_net_mgmt_fsm_state next_supported_states[] = 
    {
        ZIGPC_NET_MGMT_FSM_STATE_IDLE,
        ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL
    };

    
    zigpc_net_mgmt_notify_state_update(
            ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE, 
            next_supported_states);
  }

  return status;
}

sl_status_t zigpc_net_mgmt_node_remove_complete(
  const zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;

    zigpc_datastore_log_device(LOG_TAG,
                               SL_LOG_INFO,
                               "Removed Device",
                               eui64);

    status = zigpc_datastore_remove_device(eui64);

  if (status == SL_STATUS_OK) {

    zigpc_net_mgmt_notify_node_removed(eui64);
    enum zigpc_net_mgmt_fsm_state next_supported_states[] = 
    {
        ZIGPC_NET_MGMT_FSM_STATE_IDLE,
        ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD,
        ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE,
        ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL
    };

    zigpc_net_mgmt_notify_state_update(ZIGPC_NET_MGMT_FSM_STATE_IDLE, next_supported_states);

  }

  return status;
}

