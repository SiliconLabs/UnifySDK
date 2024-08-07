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
#include <array>
#include <vector>
#include <algorithm>
#include <string>

#include "zigpc_gateway.h"
#include "sl_status.h"

#include "zigpc_gateway_int.h"
#include <unordered_map>

/**
 * @brief Raw strings exported from EmberAf in stack/include/sl_zigbee_types.h
 *
 */
const std::vector<const char*> COUNTER_LABELS_RAW = {SL_ZIGBEE_COUNTER_STRINGS};

const static std::array<std::string, SL_ZIGBEE_COUNTER_TYPE_COUNT > COUNTER_LABELS_WO_SPACES = [](){
    // Transform raw counter labels to remove included spaces
    // i.e. "Mac Rx Bcast" -> "MacRxBcast"
    std::array<std::string, SL_ZIGBEE_COUNTER_TYPE_COUNT > tmp;
    for(size_t i = 0U; i < SL_ZIGBEE_COUNTER_TYPE_COUNT ; ++i)
    {
      std::string tmpStr = std::string(COUNTER_LABELS_RAW[i]);
      tmpStr.erase( std::remove_if(tmpStr.begin(),
                                  tmpStr.end(),
                                  [](unsigned char x){ return std::isspace(x); }));

      tmp[i] = tmpStr;
    }

    return tmp;
  }();

const char *zigpc_gateway_get_counters_entry_label(size_t offset)
{
  if (offset < SL_ZIGBEE_COUNTER_TYPE_COUNT ) {
    return COUNTER_LABELS_WO_SPACES[offset].c_str();
  } else {
    return nullptr;
  }
}

sl_status_t zigpc_gateway_get_counters_list(uint16_t *list, size_t count)
{
  sl_status_t status = SL_STATUS_OK;
  if (list == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else if (count > SL_ZIGBEE_COUNTER_TYPE_COUNT ) {
    status = SL_STATUS_WOULD_OVERFLOW;
  } else if (count == 0U) {
    status = SL_STATUS_EMPTY;
  } else {
    status = zigbeeHostGetCountersList(list, count);
  }
  return status;
}

void zigpc_gateway_clear_counters()
{
  zigbeeHostClearCounters();
}

size_t zigpc_gateway_get_counters_capacity()
{
  return SL_ZIGBEE_COUNTER_TYPE_COUNT ;
}
