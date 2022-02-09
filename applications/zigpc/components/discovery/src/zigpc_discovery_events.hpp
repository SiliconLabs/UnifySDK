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

/**
 * @file zigpc_discovery_events.hpp
 * @defgroup zigpc_discovery_events ZigPC Discovery Internal Event Handlers
 * @ingroup zigpc_discovery
 * @brief This module contains all the handlers for events received by clients
 * and zigpc_gateway.
 *
 * @{
 */

#ifndef ZIGPC_DISCOVERY_EVENTS_HPP
#define ZIGPC_DISCOVERY_EVENTS_HPP

#include <vector>

// Shared Unify includes
#include <sys/ctimer.h>
#include <sl_status.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>

// Component includes
#include "zigpc_discovery.h"

namespace zigpc_discovery::events
{
constexpr clock_time_t DISCOVERY_REQUEST_TIMEOUT = (CLOCK_SECOND * 30);

void on_request_timeout(zigbee_eui64_uint_t eui64,
                        zigpc_discovery_status_t timeout_status);

class DiscoverEvent
{
  public:
  const char *label;
  explicit DiscoverEvent(const char *label) : label(label) {}
  virtual sl_status_t handle(void) = 0;
  virtual ~DiscoverEvent()         = default;
};

class DeviceDiscoverRequest : public DiscoverEvent
{
  private:
  zigbee_eui64_uint_t eui64;

  public:
  explicit DeviceDiscoverRequest(zigbee_eui64_uint_t eui64);

  ~DeviceDiscoverRequest() override = default;

  sl_status_t handle(void) override;
};

class DeviceDiscoverResponse : public DiscoverEvent
{
  private:
  zigbee_eui64_uint_t eui64;
  std::vector<zigbee_endpoint_id_t> endpoints;

  public:
  explicit DeviceDiscoverResponse(
    zigbee_eui64_uint_t eui64, std::vector<zigbee_endpoint_id_t> &&endpoints);

  ~DeviceDiscoverResponse() override = default;

  sl_status_t handle(void) override;
};

class EndpointDiscoverRequest : public DiscoverEvent
{
  private:
  zigbee_eui64_uint_t eui64;
  zigbee_endpoint_id_t endpoint_id;

  public:
  explicit EndpointDiscoverRequest(zigbee_eui64_uint_t eui64,
                                   zigbee_endpoint_id_t endpoint_id);

  ~EndpointDiscoverRequest() override = default;

  sl_status_t handle(void) override;
};

class EndpointDiscoverResponse : public DiscoverEvent
{
  private:
  zigbee_eui64_uint_t eui64;
  zigbee_endpoint_id_t endpoint_id;
  std::vector<zcl_cluster_id_t> server_clusters;
  std::vector<zcl_cluster_id_t> client_clusters;

  public:
  explicit EndpointDiscoverResponse(
    zigbee_eui64_uint_t eui64,
    zigbee_endpoint_id_t endpoint_id,
    std::vector<zcl_cluster_id_t> &&server_clusters,
    std::vector<zcl_cluster_id_t> &&client_clusters);

  ~EndpointDiscoverResponse() override = default;

  sl_status_t handle(void) override;
};

}  // namespace zigpc_discovery::events

#endif /* ZIGPC_DISCOVERY_EVENTS_HPP */

/** @} end zigpc_discovery_events */
