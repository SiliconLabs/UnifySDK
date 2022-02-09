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

#include "stack/include/ember-types.h"
#include "app/framework/include/af-types.h"
#include "app/framework/service-function/sl_service_function.h"

/**
 * Refer to <GSDK>/protocol/zigbee/app/framework/include/af.h
 * for more details.
 */

#define EMBER_AF_ZCL_OVERHEAD 3

#define ZCL_PROFILE_WIDE_COMMAND 0
#define ZCL_GLOBAL_COMMAND       (ZCL_PROFILE_WIDE_COMMAND)

#define ZCL_CLUSTER_SPECIFIC_COMMAND BIT(0)

#define ZCL_FRAME_CONTROL_CLIENT_TO_SERVER 0

/**
 * @brief Use this macro to retrieve the current command. This
 * macro may only be used within the command parsing context. For instance
 * Any of the command handling callbacks may use this macro. If this macro
 * is used outside the command context, the returned EmberAfClusterCommand pointer
 * will be null.
 */
EmberAfClusterCommand *emberAfCurrentCommand();

/**
 * @brief populates the passed EUI64 with the local EUI64 MAC address.
 */
void emberAfGetEui64(EmberEUI64 returnEui64);

/**
 * @brief Returns the current network state.  This call caches the results
 *   on the host to prevent frequent EZSP transactions.
 */
EmberNetworkStatus emberAfNetworkState(void);

/**
  * @brief Returns the current network parameters.
  */
EmberStatus emberAfGetNetworkParameters(EmberNodeType *nodeType,
                                        EmberNetworkParameters *parameters);

/**
 * @brief Gets a Security Key based on the passed key type
 *
 * @param keyType See EmberKeyType
 * @param keyStruct Struct pointer to populate key data
 * @return EmberStatus
 */
EmberStatus emberGetKey(EmberKeyType keyType, EmberKeyStruct *keyStruct);

/** @brief Returns the primary endpoint of the current network index or 0xFF if
 * no endpoints belong to the current network.
 */
uint8_t emberAfPrimaryEndpointForCurrentNetworkIndex(void);

/**
 * @brief Set the source and destination endpoints in the client API APS frame.
 */
void emberAfSetCommandEndpoints(uint8_t sourceEndpoint,
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
uint8_t emberAfNextSequence(void);

/**
 * @brief Sends unicast.
 */
EmberStatus emberAfSendUnicast(EmberOutgoingMessageType type,
                               uint16_t indexOrDestination,
                               EmberApsFrame *apsFrame,
                               uint16_t messageLength,
                               uint8_t *message);
/**
 * @brief Sends multicast.
 */
EmberStatus emberAfSendMulticast(EmberMulticastId multicastId,
                                 EmberApsFrame *apsFrame,
                                 uint16_t messageLength,
                                 uint8_t *message);

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
 * @param shortId The node id of the remote device or ::EMBER_UNKNOWN_NODE_ID
 * if the node id is currently unknown.
 * @return The index of the address table entry for this remove device or
 * ::EMBER_NULL_ADDRESS_TABLE_INDEX if an error occurred (e.g., the address
 * table is full).
 */
uint8_t emberAfAddAddressTableEntry(EmberEUI64 longId, EmberNodeId shortId);

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
EmberStatus emberAfFindClustersByDeviceAndEndpoint(
  EmberNodeId target,
  uint8_t targetEndpoint,
  EmberAfServiceDiscoveryCallback *callback);

/**
  * @brief Initiate an Active Endpoint request ZDO message to the target node ID.
  */
EmberStatus
  emberAfFindActiveEndpoints(EmberNodeId target,
                             EmberAfServiceDiscoveryCallback *callback);

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
EmberStatus emberAfSendResponse(void);

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
uint16_t emberAfFillExternalBuffer(uint8_t frameControl,
                                   EmberAfClusterId clusterId,
                                   uint8_t commandId,
                                   const char *format,
                                   ...);

#endif  // SILABS_APP_FRAMEWORK_INCLUDE_AF_H
