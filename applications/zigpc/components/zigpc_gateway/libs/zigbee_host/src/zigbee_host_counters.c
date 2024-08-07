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

sl_status_t zigbeeHostGetCountersList(uint16_t *const list, size_t count)
{
  if ((list == NULL) || (count == 0U) || (count > SL_ZIGBEE_COUNTER_TYPE_COUNT )) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_zigbee_ezsp_read_counters(sl_zigbee_counters);

  /*
   * NOTE: sli_zigbee_counters is populated by EZSP handlers. It is safe to assume
   * that this buffer is accessed synchonously due to the EmberAf Tick
   * mechanism.
   */
  memcpy(list, sl_zigbee_counters, sizeof(uint16_t) * count);

  return SL_STATUS_OK;
}

void zigbeeHostClearCounters(void)
{
  sl_zigbee_af_counters_clear();
}
