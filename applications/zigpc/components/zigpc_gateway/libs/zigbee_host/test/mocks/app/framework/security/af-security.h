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

#ifndef SILABS_AF_SECURITY_H
#define SILABS_AF_SECURITY_H

#include <stdbool.h>
#define SIGNED_ENUM
#include "stack/include/ember-types.h"

#define EMBER_INSTALL_CODE_CRC_SIZE 2

/*  Refer to <GSDK>/protocol/zigbee/app/framework/security/af-security.h
    for more details.
*/

EmberStatus emAfInstallCodeToKey(uint8_t *installCode,  // includes CRC
                                 uint8_t length,        // includes CRC length
                                 EmberKeyData *key);

EmberStatus zaTrustCenterSetJoinPolicy(EmberJoinDecision decision);

#endif  // AF_SECURITY_H
