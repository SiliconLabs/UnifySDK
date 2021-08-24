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
#define SIGNED_ENUM
#include "stack/include/ember-types.h"
#include "app/framework/util/config.h"
#include "app/framework/include/af-types.h"

/**
 * Refer to <GSDK>/protocol/zigbee/app/framework/include/af.h
 * for more details.
 */

#define EMBER_AF_ZCL_OVERHEAD 3

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

#endif  // SILABS_APP_FRAMEWORK_INCLUDE_AF_H
