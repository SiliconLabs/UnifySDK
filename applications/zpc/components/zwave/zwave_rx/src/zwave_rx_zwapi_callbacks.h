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

/**
 * @defgroup zwave_rx_zwapi_callbacks Z-Wave RX Z-Wave API Callbacks
 * @ingroup zwave_rx
 * @brief List of Z-Wave RX callbacks that are registered to the @ref zwave_api
 * and used to call Z-Wave RX callbacks ( @ref zwave_rx ).
 *
 * These callbacks are an intermediate step between the @ref zwave_api and the
 * next application layers. Some mapping is done and data is validated/filtered,
 * so that the right handlers are called based on what was received from the
 * @ref zwave_api
 *
 * @{
 */

#ifndef ZWAVE_RX_ZWAPI_CALLBACKS_H
#define ZWAVE_RX_ZWAPI_CALLBACKS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief application_command_handler function for @ref zwapi_callbacks.
 * @ref zwave_api controller libraries will call this function for incoming
 * frames from the Z-Wave nodes (e.g. Basic Report) will be dispatched
 * to this function.
 *
 * Parameters are described in @ref application_command_handler_function
 */
void zwave_rx_application_command_handler(uint8_t rx_status,
                                          zwave_node_id_t destination_node_id,
                                          zwave_node_id_t source_node_id,
                                          const uint8_t *zwave_command_payload,
                                          uint8_t zwave_command_payload_length,
                                          int8_t rssi_value);

/**
 * @brief application_command_handler_bridge function for @ref zwapi_callbacks.
 * @ref zwave_api Bridge libraries will call this function instead of
 * application_command_handler(). Incoming frames from the Z-Wave nodes
 * (e.g. Basic Report) will be dispatched to this function.
 *
 * Parameters are described in @ref application_command_handler_function
 */
void zwave_rx_application_command_handler_bridge(
  uint8_t rx_status,
  zwave_node_id_t destination_node_id,
  zwave_node_id_t source_node_id,
  const uint8_t *zwave_command_payload,
  uint8_t zwave_command_payload_length,
  int8_t rssi_value);

/**
 * @brief application_controller_update function for @ref zwapi_callbacks.
 * Incoming NIF from the Z-Wave nodes will be dispatched to this
 * function.
 *
 * Parameters are described in @ref application_controller_update_function
 */
void zwave_rx_application_controller_update(uint8_t status,
                                            zwave_node_id_t node_id,
                                            const uint8_t *zwave_nif,
                                            uint8_t zwave_nif_length,
                                            zwave_home_id_t nwi_home_id);

/**
 * @brief functions handling when the Z-Wave API is ready to use. This
 * is called after soft reset. See @ref zwapi_callbacks
 *
 * @param buffer
 * Contains the payload of the currently configured NIF for the Z-Wave module
 * @param buffer_length
 * Indicates the length in bytes of the data contained in the buffer pointer.
 */
void zwave_rx_zwave_api_started(const uint8_t *buffer, uint8_t buffer_length);

/**
 * @brief Request call zwapi_poll again
 *
 */
void zwave_rx_poll_request();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_RX_ZWAPI_CALLBACKS_H
        /** @} end zwave_rx_zwapi_callbacks */
