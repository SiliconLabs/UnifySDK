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

#ifndef SILABS_AF_NETWORK_CREATOR_H
#define SILABS_AF_NETWORK_CREATOR_H

#include <stdbool.h>
#define SIGNED_ENUM
#include "stack/include/ember-types.h"

/**
 * Refer to <GSDK>/protocol/zigbee/app/framework/plugin/network-creator/network-creator.h
 * for more details.
 */

/** @brief Commands the network creator to form a network with the following qualities.
 *
 *
 *  @param centralizedNetwork Whether or not to form a network using
 *  centralized security. If this argument is false, a network with
 *  distributed security will be formed.
 *
 *  @return Status of the commencement of the network creator process.
 */
EmberStatus emberAfPluginNetworkCreatorStart(bool centralizedNetwork);
EmberStatus emberAfPluginNetworkCreatorNetworkForm(bool centralizedNetwork,
                                                   EmberPanId panId,
                                                   int8_t radioTxPower,
                                                   uint8_t channel);


#endif  // SILABS_AF_NETWORK_CREATOR_H
