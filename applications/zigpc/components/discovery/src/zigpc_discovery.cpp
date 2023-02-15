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

#include <memory>
#include <queue>

// Shared Unify includes
#include <sl_log.h>
#include <sl_status.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>
#include <zigpc_datastore.h>
#include <zigpc_gateway_notify.h>

// Component includes
#include "zigpc_discovery.h"
#include "zigpc_discovery.hpp"
#include "zigpc_discovery_callbacks.hpp"
#include "zigpc_discovery_events.hpp"
#include "zigpc_discovery_process.hpp"
#include "zigpc_discovery_timers.hpp"

static constexpr char LOG_TAG[] = "zigpc_discovery";

sl_status_t zigpc_discovery_interview_device(
  zigbee_eui64_uint_t eui64, const zigpc_discovery_status_callback_t callback)
{
  sl_status_t status = zigpc_discovery::callbacks::add(eui64, callback);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Failed to register callback for Eui64:%016" PRIX64 ": 0x%X",
                 eui64,
                 status);
    return status;
  }

  status = zigpc_discovery::process::enqueue_event(
    std::make_unique<zigpc_discovery::events::DeviceDiscoverRequest>(eui64));

  return status;
}

sl_status_t
  zigpc_discovery_partial_interview_endpoint(zigbee_eui64_uint_t eui64,
                                             zigbee_endpoint_id_t endpoint_id)
{
  sl_status_t status = zigpc_discovery::process::enqueue_event(
    std::make_unique<zigpc_discovery::events::EndpointDiscoverRequest>(
      eui64,
      endpoint_id));

  return status;
}

sl_status_t
  zigpc_discovery_add_listener(const zigpc_discovery_status_callback_t callback)
{
  return zigpc_discovery::callbacks::add(
    zigpc_discovery::callbacks::WILDCARD_EUI64,
    callback);
}

sl_status_t zigpc_discovery_remove_listener(
  const zigpc_discovery_status_callback_t callback)
{
  return zigpc_discovery::callbacks::remove(
    zigpc_discovery::callbacks::WILDCARD_EUI64,
    callback);
}

void zigpc_discovery_on_device_discovered(void *data)
{
  if (data == nullptr) {
    sl_log_debug(LOG_TAG, "Ignoring empty gateway device discovered event");
    return;
  }

  zigpc_gateway_on_node_discovered_t &ev
    = *static_cast<zigpc_gateway_on_node_discovered_t *>(data);

  zigpc_discovery::process::enqueue_event(
    std::make_unique<zigpc_discovery::events::DeviceDiscoverResponse>(
      zigbee_eui64_to_uint(ev.eui64),
      std::vector<zigbee_endpoint_id_t>(ev.endpoint_list,
                                        ev.endpoint_list + ev.endpoint_count)));
}

void zigpc_discovery_on_endpoint_discovered(void *data)
{
  if (data == nullptr) {
    sl_log_debug(LOG_TAG, "Ignoring empty gateway endpoint discovered event");
    return;
  }

  zigpc_gateway_on_node_endpoint_discovered_t &ev
    = *static_cast<zigpc_gateway_on_node_endpoint_discovered_t *>(data);

  std::vector<zcl_cluster_id_t> server_clusters(ev.endpoint.cluster_count);
  for (size_t i = 0U; i < ev.endpoint.cluster_count; i++) {
    server_clusters[i] = ev.endpoint.cluster_list[i].cluster_id;
    
    sl_log_debug(LOG_TAG, 
                "Discovered server cluster: 0x%04X", server_clusters[i]);
  }

  std::vector<zcl_cluster_id_t> client_clusters(
    ev.endpoint.client_cluster_count);
  for (size_t i = 0U; i < ev.endpoint.client_cluster_count; i++) {
    client_clusters[i] = ev.endpoint.client_cluster_list[i].cluster_id;
    
    sl_log_debug(LOG_TAG, 
                "Discovered client cluster: 0x%04X", client_clusters[i]);
  }

  zigpc_discovery::process::enqueue_event(
    std::make_unique<zigpc_discovery::events::EndpointDiscoverResponse>(
      zigbee_eui64_to_uint(ev.eui64),
      ev.endpoint.endpoint_id,
      std::move(server_clusters),
      std::move(client_clusters)));
}

sl_status_t zigpc_discovery_register_gateway_callbacks(void)
{
  sl_status_t status
    = zigpc_gateway_register_observer(ZIGPC_GATEWAY_NOTIFY_NODE_DISCOVERED,
                                      zigpc_discovery_on_device_discovered);

  if (status == SL_STATUS_OK) {
    status = zigpc_gateway_register_observer(
      ZIGPC_GATEWAY_NOTIFY_NODE_ENDPOINT_DISCOVERED,
      zigpc_discovery_on_endpoint_discovered);
  }

  return status;
}
