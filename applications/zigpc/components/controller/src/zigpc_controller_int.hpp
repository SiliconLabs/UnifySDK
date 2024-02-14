/*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef ZIGPC_CONTROLLER_INT_HPP
#define ZIGPC_CONTROLLER_INT_HPP

#include <vector>

// ZigPC includes
#include <zigpc_discovery.h>
#include <zigpc_common_zigbee.h>

namespace zigpc_ctrl
{
constexpr char LOG_TAG[] = "zigpc_ctrl";

/**
 * @brief Event handler when a device has joined the network and has
 * not been interviewed. This function will start the device interview
 * process.
 *
 * @param eui64         Device identifier.
 * @return sl_status_t  SL_STATUS_OK on successfully completing
 * operations, error otherwise.
 */
sl_status_t on_device_announce(const zigbee_eui64_t eui64);

/**
 * @brief Event handler when a device has left the network.
 * process.
 *
 * @param eui64         Device identifier.
 * @return sl_status_t  SL_STATUS_OK on successfully completing
 * post-remove operations, error otherwise.
 */
sl_status_t on_device_leave(const zigbee_eui64_t eui64);

/**
 * @brief Event handled when a device fails to interview on the network.
 * This function will update the node state topic.
 *
 *
 * @param eui64         Device identifier.
 * @return sl_status_t  SL_STATUS_OK on successfully completing
 * operations, error otherwise.
 */
sl_status_t on_device_interview_failed(const zigbee_eui64_t eui64);

/**
 * @brief Event handled when a device is included on the network.
 * This function will setup endpoint based configurations,
 * configure OTA support, and invoke UCL supported command list to
 * be published.
 *
 * NOTE: This function always calls \ref update_endpoint_capabilities per endpoint
 * supported.
 *
 * @param eui64                 Device identifier.
 * @param configure_endpoint   Flag to indicate if endpoint needs
 *                              configuration.
 * @return sl_status_t      SL_STATUS_OK on successfully completing
 * operations, error otherwise.
 */
sl_status_t on_device_interviewed(const zigbee_eui64_t eui64,
                                  bool configure_endpoint);

/**
 * @brief Configure endpoint under a device. This function
 * will setup bindings and configure attribute reporting for each
 * cluster under the endpoint.
 *
 * @param eui64         Device identifier.
 * @param ep_id         Endpoint identifier.
 * @return sl_status_t  SL_STATUS_OK on successfully configuring the endpoint,
 * error otherwise.
 */
sl_status_t perform_endpoint_configuration(const zigbee_eui64_t eui64,
                                           zigbee_endpoint_id_t ep_id);

/**
 * @brief Update device capabilities. This function will update the active
 * endpoints published to MQTT.
 *
 * NOTE: This function is always called by \ref on_device_interviewed.
 *
 * @param eui64         Device identifier.
 * @param endpoint_list Endpoint list.
 * @return sl_status_t  SL_STATUS_OK on successfully updating capabilities,
 * error otherwise.
 */
sl_status_t update_device_capabilities(
  const zigbee_eui64_t eui64,
  const std::vector<zigbee_endpoint_id_t> &endpoint_list);

/**
 * @brief Update endpoint capabilities for a device. This function
 * will update the cluster capabilities and configure any OTA capabilities.
 *
 * NOTE: This function is always called by \ref on_device_interviewed.
 *
 * @param eui64         Device identifier.
 * @param ep_id         Endpoint identifier.
 * @return sl_status_t  SL_STATUS_OK on successfully updating capabilities,
 * error otherwise.
 */
sl_status_t update_endpoint_capabilities(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t ep_id);

/**
 * @brief Callback handler for ZigPC device discovery updates.
 *
 * @param eui64            Device identifier.
 * @param discovery_status Discovery status type.
 */
void zigpc_discovery_update_callback(zigbee_eui64_uint_t eui64,
                                     zigpc_discovery_status discovery_status);

/**
 * @brief Event handler called when ZigPC is initializing.
 * This handler will update all device states to "Online functional" before
 * publishing known device capabilities.
 *
 */
void on_startup(void);

/**
 * @brief Event handler for when ZigPC is shutting down.
 * This handler will update all device states to "Offline" before removing all
 * non-node-state based topics for all nodes serviced from the broker.
 *
 */
void on_shutdown(void);


}  // namespace zigpc_ctrl

#endif  // ZIGPC_CONTROLLER_INT_HPP
