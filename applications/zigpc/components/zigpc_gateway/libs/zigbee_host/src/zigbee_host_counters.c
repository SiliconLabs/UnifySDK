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
#include "app/framework/include/af.h"
#include "app/framework/plugin/counters/counters.h"
#include "app/util/ezsp/ezsp-protocol.h"
#include "app/util/ezsp/ezsp.h"

/* external includes */
#include "zigbee_host.h"

EmberStatus zigbeeHostGetCountersList(uint16_t *const list, size_t count)
{
  if ((list == NULL) || (count == 0U) || (count > EMBER_COUNTER_TYPE_COUNT)) {
    return EMBER_BAD_ARGUMENT;
  }

  ezspReadCounters(emberCounters);

  /*
   * NOTE: emberCounters is populated by EZSP handlers. It is safe to assume
   * that this buffer is accessed synchonously due to the EmberAf Tick
   * mechanism.
   */
  memcpy(list, emberCounters, sizeof(uint16_t) * count);

  return EMBER_SUCCESS;
}

void zigbeeHostClearCounters(void)
{
  emberAfPluginCountersClear();
}
