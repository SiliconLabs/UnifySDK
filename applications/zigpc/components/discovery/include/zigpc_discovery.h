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
 * @file zigpc_discovery.h
 * @defgroup zigpc_discovery ZigPC Device Discovery
 * @ingroup zigpc_components
 * @brief This is responsible for starting and notify discovery of a Zigbee
 * device on the network. The currently supported messages are as follows:
 * - ActiveEndpoint Resquest + Response: For device's endpoint information
 * - SimpleDescriptor Request + Response: For Endpoint's cluster information
 *
 *
 * The following diagram shows the expected behavior for controllers and listeners:
 * @startuml
  !pragma teoz true
  participant "Listener" as p_listener
  participant "Controller" as p_controller
  participant "ZigPC Discovery" as p_discovery
  participant "ZigPC Gateway" as p_gateway
  hide footbox
  title "ZigPC Discovery Behaviour"
  == Register p_listener (for all device updates) ==
  p_listener -> p_discovery: add_listener(listener_callback)
  == Start Interview with callback (for single device updates) ==
  p_controller -> p_discovery: interview_device(EUI64_X,controller_callback)
  ...
  p_controller <- p_discovery: controller_callback(EUI64_X, DISCOVERY_START)
  p_listener <- p_discovery: listener_callback(EUI64_X, DISCOVERY_START)
  ...
  note over p_discovery
  Discover endpoints under device
  end note
  p_discovery -> p_gateway: Send DiscoverDevice Request for EUI64_X
  ...
  alt #pink DeviceDiscovery Timeout
    p_controller <- p_discovery: controller_callback(EUI64_X, DEVICE_DISCOVERY_FAIL)
    p_listener <- p_discovery: listener_callback(EUI64_X, DEVICE_DISCOVERY_FAIL)
  else #white
    ...
    p_discovery <- p_gateway: Receive DiscoverDevice Response for EUI64_X \n ActiveEndpointList: [1,2,3]
    ...
    loop for EP in ActiveEndpointList
      ...
      note over p_discovery
      Discover clusters under EP sequentially
      end note
      p_discovery -> p_gateway: Send DiscoverEndpoint Request for EUI64_X::EP
      ...
      alt #pink EndpointDiscovery Timeout
        p_controller <- p_discovery: controller_callback(EUI64_X, ENDPOINT_DISCOVERY_FAIL)
        p_listener <- p_discovery: listener_callback(EUI64_X, ENDPOINT_DISCOVERY_FAIL)
      else #white
        ...
        ...
        p_discovery <- p_gateway: Receive EndpointDiscover Response for EUI64_X::EP \n {serverClusters: [1,4,6], clientClusters: [4]}
      end alt
    end loop

    note over p_discovery
    All Endpoints Discovered, Discovery Complete
    end note
    p_controller <- p_discovery: controller_callback(EUI64_X, DISCOVERY_SUCCESS)
    p_listener <- p_discovery: listener_callback(EUI64_X, DISCOVERY_SUCCESS)
  end alt
   @enduml
 *
 * @{
 */

#ifndef ZIGPC_DISCOVERY_H
#define ZIGPC_DISCOVERY_H

// Shared Unify includes
#include <sl_status.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Discovery status types possible for a device being discovered.
 *
 */
typedef enum zigpc_discovery_status {
  DISCOVERY_START = 1,
  DISCOVERY_SUCCESS,
  DEVICE_DISCOVERY_FAIL,
  ENDPOINT_DISCOVERY_FAIL,
} zigpc_discovery_status_t;

/**
 * @brief Device discovery callback function that can be registered to be
 * notified of discovery success/failure per device.
 *
 * @param eui64   Device identifier.
 * @param status  Discovery status.
 */
typedef void (*zigpc_discovery_status_callback_t)(
  zigbee_eui64_uint_t eui64, zigpc_discovery_status_t status);

/**
 * @brief Setup fixture for zigpc discovery component.
 *
 * @return sl_status_t SL_STATUS_OK on success, error otherwise.
 */
sl_status_t zigpc_discovery_fixt_setup(void);

/**
 * @brief Shutdown fixture for zigpc discovery component.
 *
 * @return int 0 on success, error otherwise.
 */
int zigpc_discovery_fixt_shutdown(void);

/**
 * @brief Begin the interview process for a Zigbee device on the network.
 *
 * This function will kick-off the process to find:
 * - Endpoints active on a Zigbee device
 * - Client and server clusters active per Zigbee device endpoint
 *
 * @param eui64         Device identifier.
 * @return sl_status_t  SL_STATUS_OK on accepted request, error otherise.
 */
sl_status_t zigpc_discovery_interview_device(
  zigbee_eui64_uint_t eui64, const zigpc_discovery_status_callback_t callback);

/**
 * @brief Register listener to receive discovery status updates for all devices.
 *
 * @param callback      Reference to callback to register.
 * @return sl_status_t  SL_STATUS_OK on add success, error otherwise.
 */
sl_status_t zigpc_discovery_add_listener(
  const zigpc_discovery_status_callback_t callback);

/**
 * @brief Remove registered discovery listener.
 *
 * @param callback      Reference to callback to register.
 * @return sl_status_t  SL_STATUS_OK on successful removal, error otherwise.
 */
sl_status_t zigpc_discovery_remove_listener(
  const zigpc_discovery_status_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif /* ZIGPC_DISCOVERY_H */

/** @} end zigpc_discovery */
