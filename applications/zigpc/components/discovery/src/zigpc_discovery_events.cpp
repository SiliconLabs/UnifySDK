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

#include <algorithm>
#include <cinttypes>
#include <cstring>
#include <functional>
#include <list>

// Shared Unify includes
#include <sl_log.h>
#include <sl_status.h>

// ZigPC includes
#include <zigpc_datastore.h>
#include <zigpc_gateway.h>
#include <zigpc_common_zigbee.h>

// Component includes
#include "zigpc_discovery.h"
#include "zigpc_discovery.hpp"
#include "zigpc_discovery_events.hpp"
#include "zigpc_discovery_callbacks.hpp"
#include "zigpc_discovery_timers.hpp"

static constexpr char LOG_TAG[] = "zigpc_discovery";

static constexpr char LOG_FMT_DEVICE_FAIL[]
  = "%s: Failed to %s for Eui64:%016" PRIX64 ": 0x%X";
static constexpr char LOG_FMT_ENDPOINT_FAIL[]
  = "%s: Failed to %s for Endpoint:%u EUI64:%016" PRIX64 ": 0x%X";
static constexpr char LOG_FMT_CLUSTER_FAIL[]
  = "%s: Failed to %s for Cluster:0x%04X Endpoint:%u EUI64:%016" PRIX64
    ": 0x%X";

namespace zigpc_discovery::events
{
void on_request_timeout(zigbee_eui64_uint_t eui64,
                        zigpc_discovery_status timeout_status)
{
  sl_log_info(LOG_TAG,
              "%s: Timeout reached for Eui64:%016" PRIX64 "",
              "DiscoveryTimeout",
              eui64);

  zigbee_eui64_t eui64_a;
  zigbee_uint_to_eui64(eui64, eui64_a);

  zigpc_device_data_t device_data;
  sl_status_t status = zigpc_datastore_read_device(eui64_a, &device_data);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 LOG_FMT_DEVICE_FAIL,
                 "DiscoveryTimeout",
                 "read state",
                 eui64,
                 status);
  }

  device_data.network_status = ZIGBEE_NODE_STATUS_INTERVIEW_FAIL;

  status = zigpc_datastore_write_device(eui64_a, &device_data);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 LOG_FMT_DEVICE_FAIL,
                 "DiscoveryTimeout",
                 "update discovery-fail status",
                 eui64,
                 status);
  }

  zigpc_discovery::callbacks::notify(eui64, timeout_status);
  zigpc_discovery::callbacks::remove_for_device(eui64);

  status = timers::remove(eui64);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   LOG_FMT_DEVICE_FAIL,
                   "DiscoveryTimeout",
                   "remove discovery timeout",
                   eui64,
                   status);
  }
}

/***********************/
/* Discovery Events   **/
/***********************/

DeviceDiscoverRequest::DeviceDiscoverRequest(zigbee_eui64_uint_t eui64) :
  DiscoverEvent("DeviceDiscoverRequest"), eui64(eui64)
{}
sl_status_t DeviceDiscoverRequest::handle(void)
{
  zigbee_eui64_t eui64_a;
  zigbee_uint_to_eui64(eui64, eui64_a);

  sl_status_t status = zigpc_datastore_remove_device_children(eui64_a);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 LOG_FMT_DEVICE_FAIL,
                 this->label,
                 "remove previous discovery information",
                 eui64,
                 status);
    return SL_STATUS_NOT_FOUND;
  }

  zigpc_device_data_t device_data;
  status = zigpc_datastore_read_device(eui64_a, &device_data);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 LOG_FMT_DEVICE_FAIL,
                 this->label,
                 "read device-discover state",
                 eui64,
                 status);
    return SL_STATUS_OBJECT_READ;
  }

  device_data.network_status = ZIGBEE_NODE_STATUS_INTERVIEWING;
  status = zigpc_datastore_write_device(eui64_a, &device_data);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 LOG_FMT_DEVICE_FAIL,
                 this->label,
                 "update device-discover state",
                 eui64,
                 status);
    return SL_STATUS_OBJECT_WRITE;
  }

  status = zigpc_gateway_discover_device(eui64_a);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 LOG_FMT_DEVICE_FAIL,
                 this->label,
                 "request device-discover",
                 eui64,
                 status);
    return status;
  }

  // Inform listeners of discovery starting
  zigpc_discovery::callbacks::notify(eui64,
                                     zigpc_discovery_status::DISCOVERY_START);

  // Setup discovery timeout callback
  // NOTE: timer removed in DeviceDiscoverResponse::handle()
  status = timers::create(eui64,
                          DISCOVERY_REQUEST_TIMEOUT,
                          zigpc_discovery_status::DEVICE_DISCOVERY_FAIL,
                          on_request_timeout);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 LOG_FMT_DEVICE_FAIL,
                 this->label,
                 "add device-discovery timer",
                 eui64,
                 status);
  }

  return status;
}

DeviceDiscoverResponse::DeviceDiscoverResponse(
  zigbee_eui64_uint_t eui64, std::vector<zigbee_endpoint_id_t> &&endpoints) :
  DiscoverEvent("DeviceDiscoverResponse"),
  eui64(eui64),
  endpoints(std::move(endpoints))
{}

sl_status_t DeviceDiscoverResponse::handle(void)
{
  sl_status_t status = timers::remove(eui64);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   LOG_FMT_DEVICE_FAIL,
                   this->label,
                   "remove device-discovery timer",
                   eui64,
                   status);
  }

  zigbee_eui64_t eui64_a;
  zigbee_uint_to_eui64(eui64, eui64_a);

  // Create endpoint entities for those discovered
  for (const zigbee_endpoint_id_t &endpoint_id: endpoints) {
    zigpc_datastore_remove_endpoint(eui64_a, endpoint_id);
    status = zigpc_datastore_create_endpoint(eui64_a, endpoint_id);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   LOG_FMT_ENDPOINT_FAIL,
                   this->label,
                   "create entity",
                   endpoint_id,
                   eui64,
                   status);
    }
  }

  zigpc_device_data_t device_data;
  status = zigpc_datastore_read_device(eui64_a, &device_data);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 LOG_FMT_DEVICE_FAIL,
                 this->label,
                 "read discover-response state",
                 eui64,
                 status);
    return status;
  }

  // Set endpoint discovery information
  // NOTE: size() returns size_t, ensure endpoints up to limit is supported
  if (endpoints.size() > ZIGBEE_MAX_ENDPOINT_COUNT) {
    sl_log_warning(LOG_TAG,
                   "Cannot support more than %u max endpoints. Accepting "
                   "endpoints up to max",
                   ZIGBEE_MAX_ENDPOINT_COUNT);
    device_data.endpoint_total_count = ZIGBEE_MAX_ENDPOINT_COUNT;
  } else {
    device_data.endpoint_total_count = endpoints.size();
  }
  device_data.endpoint_discovered_count = 0;

  status = zigpc_datastore_write_device(eui64_a, &device_data);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 LOG_FMT_DEVICE_FAIL,
                 this->label,
                 "update discover-response state",
                 eui64,
                 status);
    return status;
  }

  if (device_data.endpoint_total_count > 0U) {
    // NOTE: Kick-off first endpoint discovery
    status = zigpc_discovery_partial_interview_endpoint(eui64, endpoints[0]);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   LOG_FMT_ENDPOINT_FAIL,
                   this->label,
                   "request endpoint-interview event",
                   endpoints[0],
                   eui64,
                   status);
    }
  }

  return status;
}

EndpointDiscoverRequest::EndpointDiscoverRequest(
  zigbee_eui64_uint_t eui64, zigbee_endpoint_id_t endpoint_id) :
  DiscoverEvent("EndpointDiscoverRequest"),
  eui64(eui64),
  endpoint_id(endpoint_id)
{}

sl_status_t EndpointDiscoverRequest::handle(void)
{
  zigbee_eui64_t eui64_a;
  zigbee_uint_to_eui64(eui64, eui64_a);

  zigpc_device_data_t device_data;
  sl_status_t status = zigpc_datastore_read_device(eui64_a, &device_data);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 LOG_FMT_DEVICE_FAIL,
                 this->label,
                 "read endpoint-discover-request state",
                 eui64,
                 status);
    return status;
  }

  // Reset discovered count if completed discovery before
  bool all_endpoints_discovered = (device_data.endpoint_discovered_count
                                   == device_data.endpoint_total_count);

  if (all_endpoints_discovered == true) {
    device_data.endpoint_discovered_count = 0;
  }

  device_data.network_status = ZIGBEE_NODE_STATUS_INTERVIEWING;

  status = zigpc_datastore_write_device(eui64_a, &device_data);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 LOG_FMT_DEVICE_FAIL,
                 this->label,
                 "update endpoint-discover-request state",
                 eui64,
                 status);
    return status;
  }

  status = zigpc_gateway_discover_endpoint(eui64_a, endpoint_id);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 LOG_FMT_ENDPOINT_FAIL,
                 this->label,
                 "request endpoint-discover",
                 endpoint_id,
                 eui64_a,
                 status);
    return status;
  }

  // NOTE: timer removed in EndpointDiscoverResponse::handle()
  status = timers::create(eui64,
                          DISCOVERY_REQUEST_TIMEOUT,
                          zigpc_discovery_status::ENDPOINT_DISCOVERY_FAIL,
                          on_request_timeout);
  if ((status != SL_STATUS_OK) && (status != SL_STATUS_ALREADY_EXISTS)) {
    sl_log_error(LOG_TAG,
                 LOG_FMT_DEVICE_FAIL,
                 this->label,
                 "add endpoint-discovery timer",
                 eui64,
                 status);
  }

  return status;
}

EndpointDiscoverResponse::EndpointDiscoverResponse(
  zigbee_eui64_uint_t eui64,
  zigbee_endpoint_id_t endpoint_id,
  std::vector<zcl_cluster_id_t> &&server_clusters,
  std::vector<zcl_cluster_id_t> &&client_clusters) :
  DiscoverEvent("EndpointDiscoverResponse"),
  eui64(eui64),
  endpoint_id(endpoint_id),
  server_clusters(std::move(server_clusters)),
  client_clusters(std::move(client_clusters))
{}

sl_status_t EndpointDiscoverResponse::handle(void)
{
  sl_status_t status = timers::remove(eui64);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   LOG_FMT_DEVICE_FAIL,
                   this->label,
                   "remove endpoint-discovery timer",
                   eui64,
                   status);
  }

  zigbee_eui64_t eui64_a;
  zigbee_uint_to_eui64(eui64, eui64_a);

  zigpc_device_data_t device_data;
  status = zigpc_datastore_read_device(eui64_a, &device_data);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 LOG_FMT_DEVICE_FAIL,
                 this->label,
                 "read endpoint-discover-response state",
                 eui64,
                 status);
    return status;
  }

  // Populate server clusters under endpoint
  for (const zcl_cluster_id_t &cluster_id: server_clusters) {
    status = zigpc_datastore_create_cluster(eui64_a,
                                            endpoint_id,
                                            ZCL_CLUSTER_SERVER_SIDE,
                                            cluster_id);
    sl_log_debug(LOG_TAG, 
                "Added server cluster: 0x%04X", cluster_id);

    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   LOG_FMT_CLUSTER_FAIL,
                   this->label,
                   "create server-side entity",
                   cluster_id,
                   endpoint_id,
                   eui64,
                   status);
    }
  }

  // Populate client clusters under endpoint
  for (const zcl_cluster_id_t &cluster_id: client_clusters) {
    status = zigpc_datastore_create_cluster(eui64_a,
                                            endpoint_id,
                                            ZCL_CLUSTER_CLIENT_SIDE,
                                            cluster_id);
    
    sl_log_debug(LOG_TAG, 
                "Added client cluster: 0x%04X", cluster_id);
    
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   LOG_FMT_CLUSTER_FAIL,
                   this->label,
                   "create client-side entity",
                   cluster_id,
                   endpoint_id,
                   eui64,
                   status);
    }
  }

  device_data.endpoint_discovered_count++;

  sl_log_info(LOG_TAG,
              "Discovered %u out of %u endpoints",
              device_data.endpoint_discovered_count,
              device_data.endpoint_total_count);

  bool all_endpoints_discovered = (device_data.endpoint_discovered_count
                                   == device_data.endpoint_total_count);

  if (all_endpoints_discovered == true) {
    device_data.network_status = ZIGBEE_NODE_STATUS_INCLUDED;

  } else {
    // Request next endpoint to be discovered

    zigbee_endpoint_id_t next_endpoint_id;
    status = zigpc_datastore_find_endpoint_by_index(
      eui64_a,
      device_data.endpoint_discovered_count,
      &next_endpoint_id);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   LOG_FMT_DEVICE_FAIL,
                   this->label,
                   "find next endpoint",
                   eui64,
                   status);
      return status;
    }

    status
      = zigpc_discovery_partial_interview_endpoint(eui64, next_endpoint_id);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   LOG_FMT_ENDPOINT_FAIL,
                   this->label,
                   "request endpoint-interview event",
                   next_endpoint_id,
                   eui64,
                   status);
    }

    device_data.network_status = ZIGBEE_NODE_STATUS_INTERVIEWING;
  }

  status = zigpc_datastore_write_device(eui64_a, &device_data);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 LOG_FMT_DEVICE_FAIL,
                 this->label,
                 "update endpoint-discover-response state",
                 eui64,
                 status);
    return status;
  }

  if (all_endpoints_discovered == true) {
    zigpc_discovery::callbacks::notify(
      eui64,
      zigpc_discovery_status::DISCOVERY_SUCCESS);
    zigpc_discovery::callbacks::remove_for_device(eui64);
  }

  return status;
}

}  // namespace zigpc_discovery::events
