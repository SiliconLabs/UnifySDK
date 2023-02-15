/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "zigpc_common_zigbee.h"
#include "zigpc_gateway.h"
#include <nlohmann/json.hpp>
#include "vector"
#include "array"

class zigpc_neighbor_metric : public zigpc_diagnostics_metric
{
  private:
  uint8_t value;
  static constexpr char LOG_TAG[] = "diagnostics_neighbor_table";

  public:
  zigpc_neighbor_metric(zigpc_diagnostics_notification &notif,
                       std::string metric_id) :
    zigpc_diagnostics_metric(notif, metric_id), value {0}
  {}

  void update_value()
  {
    size_t count = zigpc_gateway_get_neighbor_count();
    nlohmann::json jsn;
    std::vector<std::array<uint8_t, ZIGBEE_EUI64_SIZE>> neighbor_eui64_v;

    for (uint8_t i = 0U; i < count; i++) {
      zigbee_eui64_t neighbor;
      zigpc_gateway_get_neighbor_eui64(i,neighbor);
      
      //Convert to std::array
      std::array<uint8_t, ZIGBEE_EUI64_SIZE> neighbor_array;
      std::copy(neighbor, neighbor + ZIGBEE_EUI64_SIZE, neighbor_array.begin());

      neighbor_eui64_v.push_back(neighbor_array);
    }
    
    // Serialize to JSON
     nlohmann::json j_vec(neighbor_eui64_v);
    
    // Export
    serialized_value = j_vec.dump();

    on_update_complete.notify(metric_id);
  }
};