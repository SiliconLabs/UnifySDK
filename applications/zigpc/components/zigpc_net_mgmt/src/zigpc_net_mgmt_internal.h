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

#ifndef ZIGPC_NET_MGMT_FSM_H
#define ZIGPC_NET_MGMT_FSM_H

/* Contiki includes */
#include "sys/clock.h"
#include "sys/etimer.h"
#include "sl_status.h"

#include "zigpc_common_zigbee.h"
#include <zigpc_discovery.h>
#include "zigpc_gateway_notify.h"

#include "zigpc_net_mgmt.h"

/**
 * @brief Initialize Network Management
 * The function expected to be called when the gateway completes setting
 * up the network
 *
 * @param zigpc_eui64 - The EUI64 of the gateway radio
 * @param zigpc_endpoint_id - the main endpoint used by the radio
 * @param zigpc_panid - the PANID of the Zigbee network
 * @param zigpc_ext_panid - extended PANID
 * @param zigpc_radio_channel - the channel used by the Zigbee network
 *
 * @return SL_STATUS_OK if the network was successfuly initialized
 *
 */
sl_status_t zigpc_net_mgmt_init_complete(
  const zigbee_eui64_t zigpc_eui64,                 /**< Gateway EUI64 */
  zigbee_endpoint_id_t zigpc_endpoint_id,     /**< Gateway Endpoint ID */
  const zigbee_panid_t zigpc_panid,                 /**< Network PAN ID */
  const zigbee_ext_panid_t zigpc_ext_panid,         /**< Network extended Pan ID */
  zigbee_radio_channel_t zigpc_radio_channel /**<  Network radio channel */);

/**
 * @brief zigpc_net_mgmt_hdl_idle_add_request
 * Handle a request to add a new node to the network
 *
 * @param eui64 - EUI64 of the node to add
 * @param install_code - install code used to encrypt messages to specified device
 * @param install_code_length - length of install code (bytes)
 * @param is_well_known_key_add if True : use well known joining method, if false : use install code method
 *
 * @return SL_STATUS_OK if the node was added to added to TrustZone succesffuly
 *
 * */
sl_status_t zigpc_net_mgmt_hdl_idle_add_request( 
  const zigbee_eui64_t eui64,
  const zigbee_install_code_t install_code,
  uint8_t install_code_length,
  bool is_well_known_key_add);

/**
 * @brief zigpc_net_mgmt_hdl_idle_remove_request
 * Handle a request to remove a device from the network
 *
 * @param eui64 - EUI64 of the node to remove
 *
 * @return SL_STATUS_OK if the request to remove the device from the network
 * was sent successfully
 */ 
sl_status_t zigpc_net_mgmt_hdl_idle_remove_request(const zigbee_eui64_t eui64);

/**
 * @brief zigpc_net_mgmt_node_add_complete
 * Callback from the gateway when a node is added
 *
 * @param node_eui64 - EUI64 of the added node
 *
 * @SL_STATUS_OK if able to succseffuly manage the newly added node
 */ 
sl_status_t zigpc_net_mgmt_node_add_complete(const zigbee_eui64_t node_eui64);

/**
 * @brief zigpc_net_mgmt_publish_interview_status
 * On an response from a node interview message, publish the status
 *
 * @param eui64 - the EUI64 of the node
 * @param discovery_status - the status of the discovery process
 *
 * @return SL_STATUS_OK if the message was properly handled
 */
sl_status_t zigpc_net_mgmt_publish_interview_status(
  const zigbee_eui64_t eui64,
  zigpc_discovery_status_t discovery_status);

/**
 * @brief zigpc_net_mgmt_node_remove_complete
 * On a response from a node remove attempt
 *
 * @param eui64 - EUI64 of the node removed
 *
 * @return SL_STATUS_OK if the node removal could be properly handled
 */ 
sl_status_t zigpc_net_mgmt_node_remove_complete(
  const zigbee_eui64_t eui64);

#endif /* ZIGPC_NET_MGMT_FSM_H */
