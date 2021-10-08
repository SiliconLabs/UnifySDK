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

// ZigPC includes
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
sl_status_t on_device_announced(const zigbee_eui64_t eui64);

/**
 * @brief Event handled when a device is included on the network.
 * This function will setup endpoint based configurations,
 * configure OTA support, and invoke UCL supported command list to
 * be published.
 *
 * NOTE: This function always calls \ref on_endpoint_interviewed per endpoint
 * supported.
 *
 * @param eui64         Device identifier.
 * @return sl_status_t  SL_STATUS_OK on successfully completing
 * operations, error otherwise.
 */
sl_status_t on_device_interviewed(const zigbee_eui64_t eui64);

/**
 * @brief Event handler when an endpoint is included. This function
 * will setup bindings and configure attribute reporting for each
 * cluster under the endpoint.
 *
 * NOTE: This function is always called by \ref on_device_interviewed.
 *
 * @param eui64         Device identifier.
 * @param ep_id         Endpoint identifier.
 * @return sl_status_t  SL_STATUS_OK on successfully completing
 * operations, error otherwise.
 */
sl_status_t on_endpoint_interviewed(const zigbee_eui64_t eui64,
                                    zigbee_endpoint_id_t ep_id);

/**
 * @brief Event handler for when ZigPC is shutting down.
 *
 */
void on_shutdown(void);

}  // namespace zigpc_ctrl

#endif  // ZIGPC_CONTROLLER_INT_HPP