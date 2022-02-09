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
#include <algorithm>
#include <string>

#include "zigpc_gateway.h"
#include "sl_status.h"

#include "zigpc_gateway_int.h"
#include <unordered_map>

/**
 * @brief Raw strings exported from EmberAf in stack/include/ember-types.h
 *
 */
constexpr std::string_view COUNTER_LABELS_RAW[EMBER_COUNTER_TYPE_COUNT + 1]
  = {EMBER_COUNTER_STRINGS};

const char *zigpc_gateway_get_counters_entry_label(size_t offset)
{
  static std::array<std::string, EMBER_COUNTER_TYPE_COUNT> counter_labels;

  // Transform raw counter labels to remove included spaces
  // i.e. "Mac Rx Bcast" -> "MacRxBcast"
  if (counter_labels.at(0) == "") {
    for (size_t i = 0U; i < EMBER_COUNTER_TYPE_COUNT; i++) {
      if (COUNTER_LABELS_RAW[i] == nullptr) {
        break;
      }
      std::string label(COUNTER_LABELS_RAW[i]);
      label.erase(std::remove(label.begin(), label.end(), ' '), label.end());
      counter_labels.at(i) = label;
    }
  }

  if (offset < EMBER_COUNTER_TYPE_COUNT) {
    return counter_labels[offset].c_str();
  } else {
    return nullptr;
  }
}

sl_status_t zigpc_gateway_get_counters_list(uint16_t *list, size_t count)
{
  sl_status_t status = SL_STATUS_OK;
  if (list == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else if (count > EMBER_COUNTER_TYPE_COUNT) {
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
  return EMBER_COUNTER_TYPE_COUNT;
}