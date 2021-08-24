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

#include "zigpc_diagnostics_mqtt_helper.hpp"

#include <cstring>
#include <list>
#include <sstream>
#include <algorithm>

#include <stdlib.h>
#include <stdlib.h>
#include <memory>

#include "sl_status.h"
#include "uic_mqtt.h"
#include "zigpc_common_unid.h"
#include "zigpc_common_zigbee.h"
#include "sl_log.h"
#include "sl_status.h"
#include "process.h"

#include "zigpc_net_mgmt_notify.h"
#include "zigpc_diagnostics_manager.hpp"
#include "zigpc_uptime.hpp"
#include "zigpc_total_virtual_ram.hpp"
#include "zigpc_used_virtual_ram.hpp"
#include "zigpc_total_physical_ram.hpp"
#include "zigpc_used_physical_ram.hpp"
#include "zigpc_cpu_usage.hpp"
#include "zigpc_example_metric.hpp"
#include "zigpc_counter_plugin_metric.hpp"
#include "zigpc_gateway.h"

#include "zigpc_diagnostics.h"

constexpr char LOG_TAG[] = "zigpc_diagnostics";

// get unid here :: note this should be a common stuff, this is code replication
std::string zigpc_diagnostics_get_unid(const zigbee_eui64_t eui64)
{
  std::string unid = "";
  size_t unid_len  = ZIGBEE_EUI64_HEX_STR_LENGTH + strlen(ZIGPC_UNID_PREFIX);
  std::vector<char> unid_cstr(unid_len);

  sl_status_t status
    = zigpc_common_eui64_to_unid(eui64, unid_cstr.data(), unid_len);

  if (status == SL_STATUS_OK) {
    unid = unid_cstr.data();
  } else {
    sl_log_warning(LOG_TAG, "Unable to convert EUI64 to UNID: 0x%X", status);
  }

  return unid;
}

// This is what should call the setup / initialisation of diagnostic manager
// Because we need the unid to be able to publish / subscribe correctly
void zigpc_diagnostics_on_network_init(void *event_data)
{
  sl_log_debug("Diagnostics",
               "Initialising diagnostics on network init event\n");

  const struct zigpc_net_mgmt_on_network_init *net_init
    = (struct zigpc_net_mgmt_on_network_init *)event_data;

  std::string unid = zigpc_diagnostics_get_unid(net_init->pc_eui64);
  if (!unid.empty()) {
    zigpc_diagnostics_manager &manager
      = zigpc_diagnostics_manager::get_instance();
    manager.set_unid(unid);

    uic_diagnostics_setup(unid);
  }
}

//Entry point that sets up the observer
sl_status_t zigpc_diagnostics_init_fixt(void)
{
  sl_status_t status;
  sl_log_debug("Diagnostics", "Initialising diagnostic observer\n");
  status = zigpc_net_mgmt_register_observer(ZIGPC_NET_MGMT_NOTIFY_NETWORK_INIT,
                                            zigpc_diagnostics_on_network_init);
  return status;
}

//TODO primeaum : Change the REQUEST event type, im not sure what to put here
typedef enum { UIC_DIAGNOSTICS_REQUEST_EVENT_MQTT } diagnostic_process_event_t;

struct uic_diagnostics_request_mqtt_payload {
  unsigned int size;
  char *topic;
  char *payload;
};

PROCESS(uic_diagnostics_process, "UIC Diagnostics process");

// Diagnostic process implementation
PROCESS_THREAD(uic_diagnostics_process, ev, data)
{
  PROCESS_BEGIN()

  while (true) {
    switch (ev) {
      case UIC_DIAGNOSTICS_REQUEST_EVENT_MQTT: {
        auto *payload = static_cast<std::string *>(data);
        zigpc_diagnostics_manager &manager
          = zigpc_diagnostics_manager::get_instance();
        manager.handle_metricID_request(
          uic_diagnostic_request_to_strings(payload));
        delete payload;
      }
      default:
        break;
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
/**
 * @brief Callback for the diagnostics uic diagnostics request message
 */
void uic_diagnostics_mqtt_callback(const char *topic,
                                   const char *payload,
                                   size_t payload_size)
{
  process_post(&uic_diagnostics_process,
               UIC_DIAGNOSTICS_REQUEST_EVENT_MQTT,
               new std::string(payload));
}

/**
 * @brief Used to setup the diagnostic process and related subscriptions
 */
sl_status_t uic_diagnostics_setup(std::string unid)
{
  printf("Diagnostic setup started");
  // Start the diagnostics process
  process_start(&uic_diagnostics_process, nullptr);

  std::string request_topic
    = "ucl/by-unid/" + unid + "/ProtocolController/Diagnostics/Request";
  // Subscribe to an MQTT-topic
  uic_mqtt_subscribe(request_topic.c_str(), uic_diagnostics_mqtt_callback);

  //Initialise metrics
  uic_metric_init();
  return SL_STATUS_OK;
}

void uic_metric_init()
{
  zigpc_diagnostics_manager &manager
    = zigpc_diagnostics_manager::get_instance();
  zigpc_diagnostics_notification &m_notify = manager;

  std::list<std::shared_ptr<zigpc_diagnostics_metric>> metrics_to_add = {
    std::make_shared<zigpc_counter_plugin_metric>(m_notify, "Counters"),
    std::make_shared<zigpc_uptime_metric>(m_notify, "Uptime"),
    std::make_shared<zigpc_total_physical_ram_metric>(m_notify, "TotalPhysRam"),
    std::make_shared<zigpc_used_physical_ram_metric>(m_notify, "UsedPhysRam"),
    std::make_shared<zigpc_total_virtual_ram_metric>(m_notify,
                                                     "TotalVirtualRam"),
    std::make_shared<zigpc_used_virtual_ram_metric>(m_notify, "UsedVirtualRam"),
    std::make_shared<zigpc_cpu_usage>(m_notify, "CpAvgUsage1Min", one_minute),
    std::make_shared<zigpc_cpu_usage>(m_notify, "CpuAvgUsage5Min", five_minute),
    std::make_shared<zigpc_cpu_usage>(m_notify,
                                      "CpuAvgUsage15Min",
                                      fifthteen_minute),
  };

  for (auto &metric: metrics_to_add) {
    manager.add_metric(metric);
  }
  manager.publish_metric_list();
}

/**
 * @brief Used to transform the received json payload of a diagnostic request
 *
 * @param payload_string the received mqtt payload containing an array of string
 * @return A C++ vector of strings containing parsed metric_id
 */
std::vector<std::string>
  uic_diagnostic_request_to_strings(std::string *payload_string)
{
  // Remove [ ] "
  payload_string->erase(
    remove(payload_string->begin(), payload_string->end(), '['),
    payload_string->end());
  payload_string->erase(
    remove(payload_string->begin(), payload_string->end(), ']'),
    payload_string->end());
  payload_string->erase(
    remove(payload_string->begin(), payload_string->end(), '\"'),
    payload_string->end());

  // Split
  std::vector<std::string> metric_ids;

  std::stringstream string_stream(payload_string->c_str());
  std::string split_metric_id;

  while (getline(string_stream, split_metric_id, ',')) {
    metric_ids.push_back(split_metric_id);
  }

  return metric_ids;
}