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

#include PLATFORM_HEADER
#include EMBER_AF_API_EMBER_TYPES
#include EMBER_AF_API_COUNTERS

/* external includes */
#include "z3gateway.h"

EmberStatus z3gatewayGetCountersList(uint16_t list[EMBER_COUNTER_TYPE_COUNT])
{
  /*
   * NOTE: emberCounters is populated by EZSP handlers. It is safe to assume
   * that this buffer is accessed synchonously due to the EmberAf Tick
   * mechanism.
   */
  memcpy(list, emberCounters, sizeof(uint16_t) * EMBER_COUNTER_TYPE_COUNT);

  return EMBER_SUCCESS;
}

void z3gatewayClearCounters(){
  emberAfPluginCountersClear();
}


