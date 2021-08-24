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
#include <sstream>
#include <string>
class zigpc_counter_plugin_metric : public zigpc_diagnostics_metric
{
  public:
  zigpc_counter_plugin_metric(zigpc_diagnostics_notification &notif,
                     std::string metric_id) :
    zigpc_diagnostics_metric(notif, metric_id)
  {}

  void update_value()
  {
    size_t capacity = zigpc_gateway_get_counters_capacity();
    uint16_t counter_array [capacity];
    zigpc_gateway_get_counters_list(counter_array,capacity);

    // must look like : {"KEY" : value, "KEY" : value}
    std::stringstream payload;
    payload << "{";
    std::string sep("");
    uint8_t i = 0;
    for(auto const &counter_value : counter_array){
      payload << sep << "\"" << zigpc_gateway_get_counters_entry_label(i) << "\" : " << counter_value;
      sep = ",";
      i++;
    }
    payload << "}";
    serialized_value = payload.str();
    on_update_complete.notify(metric_id);
  }
};