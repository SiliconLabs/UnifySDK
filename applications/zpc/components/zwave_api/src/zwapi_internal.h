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
 * @file zwapi_internal.h
 * @brief Private definitions for the Z-Wave API
 *
 */

#ifndef ZWAPI_INTERNAL_H
#define ZWAPI_INTERNAL_H

#include "zwapi_init.h"
#include "zwapi_protocol_transport.h"  //zwapi_tx_report_t definition

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
struct LEARN_INFO;

extern void (*zwapi_send_data_callback)(uint8_t, zwapi_tx_report_t *);
extern void (*zwapi_send_nop_callback)(uint8_t, zwapi_tx_report_t *);
extern void (*zwapi_send_data_multi_callback)(uint8_t);
extern void (*zwapi_send_test_frame_callback)(uint8_t);
extern void (*zwapi_send_data_bridge_callback)(uint8_t, zwapi_tx_report_t *);
extern void (*zwapi_send_data_multi_bridge_callback)(uint8_t);
extern void (*zwapi_send_node_information_callback)(uint8_t txStatus);
extern void (*zwapi_memory_put_buffer_callback)(void);
extern void (*zwapi_set_default_callback)(void);
extern void (*zwapi_create_new_primarly_ctrl_callback)(struct LEARN_INFO *);
extern void (*zwapi_remove_node_from_network_callback)(struct LEARN_INFO *);
extern void (*zwapi_add_node_to_network_callback)(struct LEARN_INFO *);
extern void (*zwapi_transfer_primary_ctrl_role_callback)(struct LEARN_INFO *);
extern void (*zwapi_send_controller_replication_callback)(uint8_t txStatus);
extern void (*zwapi_assign_return_route_callback)(uint8_t bStatus);
extern void (*zwapi_assign_suc_return_route_callback)(uint8_t bStatus);
extern void (*zwapi_delete_suc_return_route_callback)(uint8_t bStatus);
extern void (*zwapi_delete_return_route_callback)(uint8_t bStatus);
extern void (*zwapi_set_learn_mode_callback)(struct LEARN_INFO *);
extern void (*zwapi_set_virtual_node_to_learn_mode_callback)(
  uint8_t bStatus, zwave_node_id_t orgID, zwave_node_id_t newID);
extern void (*zwapi_request_neighbor_update_callback)(uint8_t bStatus);
extern void (*zwapi_set_suc_node_id_callback)(uint8_t bStatus);
extern void (*zwapi_request_network_update_callback)(uint8_t txStatus);
extern void (*zwapi_remove_failed_node_callback)(uint8_t txStatus);
extern void (*zwapi_replace_failed_node_callback)(uint8_t txStatus);

/**
 * Size of rx and tx buffers used with the serial API
 *
 * Note: Value is taken from SERIAL_BUFFER_SIZE in zwapi_connection.h
 *       The ZGW has a modified version of zwapi_connection.h where
 *       SERIAL_BUFFER_SIZE is defined differently.
 */
#define REQUEST_BUFFER_SIZE 180
#define SERIAL_BUFFER_SIZE  0xFF
#define FRAME_LENGTH_MIN    3
#define FRAME_LENGTH_MAX    SERIAL_BUFFER_SIZE

// Some commands have a Return Value given by the Z-Wave Module
#define ZW_COMMAND_RETURN_VALUE_FALSE \
  0  ///< Command was not executed without errors
#define ZW_COMMAND_RETURN_VALUE_TRUE 1  ///< Command was executed successfully

/**
 * @brief Execute a callback based on a received frame from the Z-Wave module.
 *
 * @param pData    Pointer to data frame (without SOF)
 * @param len    Length of data frame
 *
 * @note Must only be called locally in the Z-Wave API module.
 */
void zwave_api_protocol_rx_dispatch(uint8_t *pData, uint16_t len);

/**
 * @brief Initialize functions pointers for Z-Wave module callbacks.
 *
 * These callbacks are handlers for incoming REQ frames from the Z-Wave module
 */
void zwave_api_protocol_init_callbacks(void);

/**
 * @brief Verifies that the command is supported by the Z-Wave module.
 *
 * @param command               Z-Wave Serial API Command (zwapi_func_ids.h)
 * @param payload_buffer        Byte array with serial API command parameters
 * @param payload_buffer_length Length in bytes of parameter array
 * @returns SL_STATUS_OK if the frame is supported by the Z-Wave module,
 * sent and Ack'ed by the module. SL_STATUS_NOT_SUPPORTED if the particular
 * command is not supported by the Z-Wave API. SL_STATUS_FAIL in all other
 * error cases.
 *
 * Sends the command data frame to the Z-Wave module and waits for ACK.
 */
sl_status_t zwapi_send_command(uint8_t command,
                               const uint8_t *payload_buffer,
                               uint8_t payload_buffer_length);

/**
 * @brief Send a command to the Z-Wave module and waits for an ACK.
 *
 * @param command               Z-Wave Serial API Command (zwapi_func_ids.h)
 * @param payload_buffer        Byte array with serial API command parameters
 * @param payload_buffer_length Length in bytes of parameter array
 * @param response_buffer       Output Buffer to hold response (buffer MUST be
 * large enough to hold SERIAL_BUFFER_SIZE bytes).
 * @param response_length       Ouput Length of response copied to response_buf.
 * @returns SL_STATUS_OK if the frame is supported by the Z-Wave module, sent,
 * Ack'ed by the module and the corresponding Response frame has been received.
 * SL_STATUS_NOT_SUPPORTED if the particular command is not supported
 * by the Z-Wave API.
 * SL_STATUS_FAIL if an error occurred.
 *
 * Verifies that the command is supported by the Z-Wave module, sends the
 * command data frame to the Z-Wave module, wait for ACK and a Response frame
 * returning a status of the requested operation/command.
 */
sl_status_t zwapi_send_command_with_response(uint8_t command,
                                             const uint8_t *payload_buffer,
                                             uint8_t payload_buffer_length,
                                             uint8_t *response_buffer,
                                             uint8_t *response_length);

/**
 * @brief Send a command to the Z-Wave module without waiting for any ACK.
 *
 * @param command               Z-Wave Serial API Command (zwapi_func_ids.h)
 * @param payload_buffer        Byte array with serial API command parameters
 * @param payload_buffer_length Length in bytes of parameter array
 * @returns SL_STATUS_OK if the frame is supported by the Z-Wave module.
 * SL_STATUS_FAIL otherwise
 *
 * Verifies that the command is supported by the Z-Wave module, sends the
 * command data frame to the Z-Wave module but does not wait for any ACK
 */
sl_status_t zwapi_send_command_no_ack(uint8_t command,
                                      const uint8_t *payload_buffer,
                                      uint8_t payload_buffer_length);

#ifdef __cplusplus
}
#endif

#endif  // ZWAPI_INTERNAL_H
