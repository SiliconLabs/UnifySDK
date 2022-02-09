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

#include "zigpc_diagnostics_metric.hpp"
#include "zigpc_gateway.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

class zigpc_counter_plugin_metric : public zigpc_diagnostics_metric
{
  private:
  static constexpr char LOG_TAG[] = "diagnostics_metric_counters";

  public:
  zigpc_counter_plugin_metric(zigpc_diagnostics_notification &notif,
                              std::string metric_id) :
    zigpc_diagnostics_metric(notif, metric_id)
  {}

  void update_value()
  {
    size_t capacity = zigpc_gateway_get_counters_capacity();
    uint16_t counter_array[capacity];

    sl_status_t status
      = zigpc_gateway_get_counters_list(counter_array, capacity);

    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG, "Failed to retrieve counters list: 0x%X", status);
      return;
    }

    // payload should be as a JSON object: {"KEY" : value, "KEY" : value}
    nlohmann::json jsn;

    for (uint8_t i = 0U; i < capacity; i++) {
      const char *label_cstr = zigpc_gateway_get_counters_entry_label(i);
      if (label_cstr == nullptr) {
        continue;
      }

      jsn[std::string(label_cstr)] = counter_array[i];
    }

    serialized_value = jsn.dump();
    on_update_complete.notify(metric_id);
  }
};