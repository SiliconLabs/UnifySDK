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

#ifndef SILABS_APP_FRAMEWORK_INCLUDE_AF_H
#define SILABS_APP_FRAMEWORK_INCLUDE_AF_H

#include <stdbool.h>

#include "stack/include/sl_zigbee_types.h"
#include "app/framework/include/af-types.h"
#include "app/framework/service-function/sl_service_function.h"

/**
 * Refer to <GSDK>/protocol/zigbee/app/framework/include/af.h
 * for more details.
 */

#define SL_ZIGBEE_AF_ZCL_OVERHEAD 3

#define ZCL_PROFILE_WIDE_COMMAND 0u
#define ZCL_GLOBAL_COMMAND       (ZCL_PROFILE_WIDE_COMMAND)

#define ZCL_CLUSTER_SPECIFIC_COMMAND BIT(0)

#define ZCL_FRAME_CONTROL_CLIENT_TO_SERVER 0u
#define ZCL_DISABLE_DEFAULT_RESPONSE_MASK     BIT(4)

/**
 * @brief Use this macro to retrieve the current command. This
 * macro may only be used within the command parsing context. For instance
 * Any of the command handling callbacks may use this macro. If this macro
 * is used outside the command context, the returned sl_zigbee_af_cluster_command_t pointer
 * will be null.
 */
sl_zigbee_af_cluster_command_t *sl_zigbee_af_current_command();

/**
 * @brief populates the passed EUI64 with the local EUI64 MAC address.
 */
void sl_zigbee_af_get_eui64(sl_802154_long_addr_t returnEui64);

/**
 * @brief Returns the current network state.  This call caches the results
 *   on the host to prevent frequent EZSP transactions.
 */
sl_zigbee_network_status_t sl_zigbee_af_network_state(void);

/**
  * @brief Returns the current network parameters.
  */
sl_status_t sl_zigbee_af_get_network_parameters(sl_zigbee_node_type_t *nodeType,
                                        sl_zigbee_network_parameters_t *parameters);

/**
 * @brief Gets a Security Key based on the passed key type
 *
 * @param keyType See sl_zigbee_key_type_t
 * @param keyStruct Struct pointer to populate key data
 * @return sl_status_t
 */
sl_status_t emberGetKey(sl_zigbee_key_type_t keyType, sl_zigbee_key_struct_t *keyStruct);

/** @brief Returns the primary endpoint of the current network index or 0xFF if
 * no endpoints belong to the current network.
 */
uint8_t sl_zigbee_af_primary_endpoint_for_current_network_index(void);

/**
 * @brief Set the source and destination endpoints in the client API APS frame.
 */
void sl_zigbee_af_set_command_endpoints(uint8_t sourceEndpoint,
                                uint8_t destinationEndpoint);

/**
* @brief Increments the ZCL sequence number and returns the value.
*
* ZCL messages have sequence numbers so that they can be matched up with other
* messages in the transaction.  To avoid conflicts with sequence numbers
* generated independently by the application, this API returns sequence
* numbers with the high bit clear.  If the application generates its own
* sequence numbers, it should use numbers with the high bit set.
*
* @return The next ZCL sequence number.
*/
uint8_t sl_zigbee_af_next_sequence(void);

/**
 * @brief Sends unicast.
 */
sl_status_t emberAfSendUnicast(sl_zigbee_outgoing_message_type_t type,
                               uint16_t indexOrDestination,
                               sl_zigbee_aps_frame_t *apsFrame,
                               uint16_t messageLength,
                               uint8_t *message);
/**
 * @brief Sends multicast.
 */
sl_status_t sl_zigbee_af_send_multicast(sl_zigbee_multicast_id_t multicastId,
                                        sl_802154_short_addr_t alias,
                                        uint8_t sequence,
                                        sl_zigbee_aps_frame_t *apsFrame,
                                        uint16_t messageLength,
                                        uint8_t* message);

/**
 * @brief Use this function to add an entry for a remote device to the address
 * table.
 *
 * If the EUI64 already exists in the address table, the index of the existing
 * entry will be returned.  Otherwise, a new entry will be created and the new
 * new index will be returned.  The framework will remember how many times the
 * returned index has been referenced.  When the address table entry is no
 * longer needed, the application should remove its reference by calling
 * ::emberAfRemoveAddressTableEntry.
 *
 * @param longId The EUI64 of the remote device.
 * @param shortId The node id of the remote device or ::SL_ZIGBEE_UNKNOWN_NODE_ID
 * if the node id is currently unknown.
 * @return The index of the address table entry for this remove device or
 * ::SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX if an error occurred (e.g., the address
 * table is full).
 */
uint8_t sl_zigbee_af_add_address_table_entry(sl_802154_long_addr_t longId, sl_802154_short_addr_t shortId);

/**
 * @brief Use this function to find all of the given in and out clusters
 *   implemented on a devices given endpoint. Target should only be the
 *   short address of a specific device.
 *
 * With this function a single service discovery is initiated and the response
 * is passed back to the passed callback.
 *
 * @param target The destination node ID for the discovery. This should be a
 *  specific node's ID and should not be a broadcast address.
 * @param targetEndpoint The endpoint to target with the discovery process.
 * @param callback Function pointer for the callback function triggered when
 *  the discovery is returned.
 */
sl_status_t sl_zigbee_af_find_clusters_by_device_and_endpoint(
  sl_802154_short_addr_t target,
  uint8_t targetEndpoint,
  sl_zigbee_af_service_discovery_callback_t *callback);

/**
  * @brief Initiate an Active Endpoint request ZDO message to the target node ID.
  */
sl_status_t
  sl_zigbee_af_find_active_endpoints(sl_802154_short_addr_t target,
                             sl_zigbee_af_service_discovery_callback_t *callback);

/**
 * @brief This function sends a ZCL response, based on the information
 * that is currently in the outgoing buffer. It is expected that a complete
 * ZCL message is present, including header.  The application may use
 * this method directly from within the message handling function
 * and associated callbacks.  However this will result in the
 * response being sent before the APS Ack is sent which is not
 * ideal.
 *
 * NOTE:  This will overwrite the ZCL sequence number of the message
 * to use the LAST received sequence number.
 */
sl_status_t sl_zigbee_af_send_response(void);

/**
 * @brief Function that fills in the buffer with command.
 *
 * NOTE: This decleration is originally in app/framework/util/client-api.h
 *
 * Fills the outgoing ZCL buffer and returns the number
 * of bytes used in a buffer. Uses the buffers that
 * were previously set with emberAfSetExternalBuffer.
 *
 * @param frameControl Byte used for frame control.
 * @param clusterId Cluster ID of message.
 * @param commandId Command ID of message.
 */
uint16_t sl_zigbee_af_fill_external_buffer(uint8_t frameControl,
                                   sl_zigbee_af_cluster_id_t clusterId,
                                   uint8_t commandId,
                                   const char *format,
                                   ...);

#endif  // SILABS_APP_FRAMEWORK_INCLUDE_AF_H
