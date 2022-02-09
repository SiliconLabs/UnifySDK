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
#include <algorithm>
#include <sstream>

#include <uic_mqtt.h>
#include <nlohmann/json.hpp>

#include "zigpc_diagnostics_mqtt_helper.hpp"
#include "zigpc_diagnostics_manager.hpp"

/**
 * @brief Stores and manages metrics and handle information requests from mqtt helper / diagnostic process
 * */
zigpc_diagnostics_manager::zigpc_diagnostics_manager(/* args */) {}

zigpc_diagnostics_manager::~zigpc_diagnostics_manager() {}

zigpc_diagnostics_manager &zigpc_diagnostics_manager::get_instance()
{
  static zigpc_diagnostics_manager instance;
  return instance;
}

void zigpc_diagnostics_manager::notify(std::string metric_id)
{
  auto search = metric_map.find(metric_id);
  if (search != metric_map.end()) {
    std::stringstream value_ss;
    value_ss << R"({"value":)" << search->second->get_serialized_value() << "}";
    std::string full_topic = "ucl/by-unid/" + unid + ZIGPC_DIAGNOSTIC_TOPIC  
                             + search->second->get_metric_id();

    const std::string metric_value(value_ss.str());

    uic_mqtt_publish(full_topic.c_str(),
                     metric_value.c_str(),
                     metric_value.size(),
                     false);
  }
}

void zigpc_diagnostics_manager::publish_metric_list()
{
  // if the metric map is not empty publish entire list
  if (!metric_map.empty()) {
    nlohmann::json jsn;
    jsn["value"] = nlohmann::json::array();

    for (auto const &metric: metric_map) {
      jsn["value"].push_back(metric.first);
    }

    std::string full_topic = "ucl/by-unid/" + unid + ZIGPC_DIAGNOSTIC_TOPIC + "SupportedMetrics";
    std::string payload = jsn.dump();

    uic_mqtt_publish(full_topic.c_str(), payload.c_str(), payload.size(), true);
  }
}

/**
 * @brief Handle a metric ID request by calling the relevant metrics update function
 *
 * @param metric_id_vector vector of MetricID strings
 */
void zigpc_diagnostics_manager::handle_metricID_request(
  std::vector<std::string> metric_id_vector)
{
  //remove duplicates
  sort(metric_id_vector.begin(), metric_id_vector.end());
  metric_id_vector.erase(
    unique(metric_id_vector.begin(), metric_id_vector.end()),
    metric_id_vector.end());

  // for each string in the vector, find the corresponding metric and call its update function
  for (auto const &metric_id: metric_id_vector) {
    auto search = metric_map.find(metric_id);
    if (search != metric_map.end()) {
      search->second->update_value();
    }
  }
}
