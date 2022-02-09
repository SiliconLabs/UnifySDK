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

#ifndef SILABS_AF_MAIN_H
#define SILABS_AF_MAIN_H

#include <stdbool.h>
#define SIGNED_ENUM
#include "stack/include/ember-types.h"

/*  Refer to <GSDK>/protocol/zigbee/app/framework/util/af-main.h
    for more details.
*/

EmberStatus emAfPermitJoin(uint8_t duration, bool broadcastMgmtPermitJoin);

#endif  // SILABS_AF_MAIN_H
