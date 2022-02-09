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
 * @file zigpc_discovery.hpp
 * @defgroup zigpc_discovery_int ZigPC Discovery Internal API
 * @ingroup zigpc_discovery
 * @brief
 *
 * @{
 */

#ifndef ZIGPC_DISCOVERY_HPP
#define ZIGPC_DISCOVERY_HPP

// ZigPC includes
#include <zigpc_common_zigbee.h>

/**
 * @brief Begin partial interview process for a Zigbee device endpoint on the
 * network. This function is used internally to transition from requesting
 * cluster discovery from endpoint discovery response.
 *
 * This function will kick-off the process to find:
 * - Client and server clusters active per Zigbee device endpoint
 *
 * NOTE: This will be called as part of the \ref zigpc_discoveryerview_endpoint
 * behaviour.
 *
 * NOTE: This function is only used internally.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @return sl_status_t  SL_STATUS_OK on accepted request, error otherise.
 */
sl_status_t
  zigpc_discovery_partial_interview_endpoint(zigbee_eui64_uint_t eui64,
                                             zigbee_endpoint_id_t endpoint_id);

/**
 * @brief Callback registered with zigpc_gateway to receive device discover
 * response messages. This function will create a
 * \ref zigpc_discovery::events::DeviceDiscoverResponse event to be processed
 * within zigpc_discovery.
 *
 * NOTE: This function is only used internally.
 *
 * @param data Device discovery information sent from zigpc_gateway.
 */
void zigpc_discovery_on_device_discovered(void *data);

/**
 * @brief Callback registered with zigpc_gateway to receive endpoint discover
 * response messages. This function will create a
 * \ref zigpc_discovery::events::EndpointDiscoverResponse event to be processed
 * within zigpc_discovery.
 *
 * NOTE: This function is only used internally.
 *
 * @param data Endpoint discovery information sent from zigpc_gateway.
 */
void zigpc_discovery_on_endpoint_discovered(void *data);

/**
 * @brief Setup callbacks to receive messages from zigpc_gateway.
 *
 * @return sl_status_t SL_STATUS_OK, or error otherwise.
 */
sl_status_t zigpc_discovery_register_gateway_callbacks(void);

#endif /* ZIGPC_DISCOVERY_HPP */

/** @} end zigpc_discovery_int */
