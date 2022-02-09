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

#ifndef ZWAVE_COMMAND_HANDLER_CALLBACKS_H
#define ZWAVE_COMMAND_HANDLER_CALLBACKS_H

#include "zwave_controller_connection_info.h"
#include "zwave_controller_types.h"

/**
 * @defgroup zwave_command_handler_callbacks Callbacks for the Z-Wave Command Handler
 * @ingroup zwave_command_handler
 * @brief List of callback functions that are registered to the @ref zwave_controller
 *
 * List of callbacks to register to the @ref zwave_controller.
 * Refer to @ref zwave_controller_callbacks_t for details.
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Callback for entering a new network.
 * Refer to @ref zwave_controller_callbacks_t
 * @ref zwave_controller_callbacks_t.on_new_network_entered for parameter description
 */
void zwave_command_handler_on_new_network_entered(
  zwave_home_id_t home_id,
  zwave_node_id_t node_id,
  zwave_keyset_t granted_keys,
  zwave_kex_fail_type_t kex_fail_type);

/**
 * Callback for receiving a Z-Wave frame.
 * Refer to @ref zwave_controller_callbacks_t
 * @ref zwave_controller_callbacks_t.on_frame_received for parameter description
 */
void zwave_command_handler_on_frame_received(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length);

#ifdef __cplusplus
}
#endif
/** @} end zwave_command_handler_callbacks */

#endif  //ZWAVE_COMMAND_HANDLER_CALLBACKS_H