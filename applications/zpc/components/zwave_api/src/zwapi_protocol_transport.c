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
// Includes from this component
#include "zwapi_protocol_transport.h"
#include "zwapi_func_ids.h"
#include "zwapi_session.h"
#include "zwapi_init.h"
#include "zwapi_internal.h"
#include "zwapi_utils.h"

// Generic includes
#include <assert.h>
#include <stddef.h>

// UIC Includes
#include "sl_log.h"

#define LOG_TAG "zwapi_protocol_transport"

sl_status_t
  zwapi_send_data_multi_bridge(zwave_node_id_t source_node_id,
                               zwave_nodemask_t destination_node_mask,
                               const uint8_t *data,
                               uint8_t data_length,
                               uint8_t tx_options,
                               void (*callback_function)(uint8_t tx_status))
{
  uint8_t number_of_nodes = 0, number_of_nodes_index = 0;
  uint8_t func_id = 0, response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};

  zwapi_write_node_id(request_buffer, &index, source_node_id);

  // The number of nodes should precede the list of nodes in the data. But
  // we must first unpack the nodemask to learn how many nodes it contain.
  // We'll save the position for now (and increment idx) and fill in the
  // count when we have processed the nodemask.
  number_of_nodes_index = index++;

  /* Unpack nodemask in byte array into array of node ids */
  for (zwave_node_id_t i = ZW_MIN_NODE_ID; i <= ZW_LR_MAX_NODE_ID; i++) {
    if (ZW_IS_NODE_IN_MASK(i, destination_node_mask)) {
      zwapi_write_node_id(request_buffer, &index, i);
      number_of_nodes += 1;
    }
  }
  request_buffer[number_of_nodes_index] = number_of_nodes;
  request_buffer[index++]               = data_length;
  for (uint8_t i = 0; i < data_length; i++) {
    request_buffer[index++] = data[i];
  }
  request_buffer[index++] = tx_options;

  func_id                 = (callback_function == NULL ? 0 : 0x03);
  request_buffer[index++] = func_id;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_SEND_DATA_MULTI_BRIDGE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    // Activate the callback function only if the Z-Wave module accepted the command
    zwapi_send_data_multi_bridge_callback = callback_function;
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_lock_route(bool lock)
{
  uint8_t index                               = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0};
  request_buffer[index++]                     = lock ? 0x01 : 0x00;
  return zwapi_send_command(FUNC_ID_LOCK_ROUTE_RESPONSE, request_buffer, index);
}

sl_status_t zwapi_send_data_bridge(
  zwave_node_id_t source_node_id,
  zwave_node_id_t destination_node_id,
  const uint8_t *data,
  uint8_t data_length,
  uint8_t tx_options,
  void (*callback_function)(uint8_t, zwapi_tx_report_t *))
{
  uint8_t func_id = 0, response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};

  if ((data_length + 2) > sizeof(request_buffer)) {
    sl_log_error(LOG_TAG, "The frame is too long\n");
    return SL_STATUS_FAIL;
  }
  zwapi_write_node_id(request_buffer, &index, source_node_id);
  zwapi_write_node_id(request_buffer, &index, destination_node_id);
  request_buffer[index++] = data_length;
  for (uint8_t i = 0; i < data_length; i++) {
    request_buffer[index++] = data[i];
  }
  request_buffer[index++] = tx_options;
  request_buffer[index++] = 0;
  request_buffer[index++] = 0;
  request_buffer[index++] = 0;
  request_buffer[index++] = 0;
  func_id                 = (callback_function == NULL ? 0 : 1);
  request_buffer[index++] = func_id;

  zwapi_send_data_bridge_callback = callback_function;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_SEND_DATA_BRIDGE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    // Activate the callback function only if the Z-Wave module accepted the command
    zwapi_send_data_bridge_callback = callback_function;
    return SL_STATUS_OK;
  }
  return SL_STATUS_FAIL;
}

sl_status_t zwapi_send_data_multi(zwave_nodemask_t destination_node_mask,
                                  const uint8_t *data,
                                  uint8_t data_length,
                                  uint8_t tx_options,
                                  void (*callback_function)(uint8_t tx_status))
{
  // If the current API is a Bridge, just call the bridge version.
  if (zwapi_get_library_type() == ZWAVE_LIBRARY_TYPE_CONTROLLER_BRIDGE) {
    return zwapi_send_data_multi_bridge(0xFF,
                                        destination_node_mask,
                                        data,
                                        data_length,
                                        tx_options,
                                        callback_function);
  }

  uint8_t number_of_nodes = 0, number_of_nodes_index = 0;
  uint8_t func_id = 0, response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};

  // The number of nodes should precede the list of nodes in the data. But
  // we must first unpack the nodemask to learn how many nodes it contain.
  // We'll save the position for now (and increment idx) and fill in the
  // count when we have processed the nodemask.
  number_of_nodes_index = index++;

  /* Unpack nodemask in byte array into array of node ids */
  for (zwave_node_id_t i = ZW_MIN_NODE_ID; i <= ZW_LR_MAX_NODE_ID; i++) {
    if (ZW_IS_NODE_IN_MASK(i, destination_node_mask)) {
      zwapi_write_node_id(request_buffer, &index, i);
      number_of_nodes += 1;
    }
  }
  request_buffer[number_of_nodes_index] = number_of_nodes;
  request_buffer[index++]               = data_length;
  for (uint8_t i = 0; i < data_length; i++) {
    request_buffer[index++] = data[i];
  }
  request_buffer[index++] = tx_options;

  func_id                 = (callback_function == NULL ? 0 : 0x03);
  request_buffer[index++] = func_id;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_SEND_DATA_MULTI,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    // Activate the callback function only if the Z-Wave module accepted the command
    zwapi_send_data_multi_callback = callback_function;
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_send_data(zwave_node_id_t destination_node_id,
                            const uint8_t *data,
                            uint8_t data_length,
                            uint8_t tx_options,
                            void (*callback_function)(uint8_t,
                                                      zwapi_tx_report_t *))
{
  // If the current API is a Bridge, just call the bridge version.
  if (zwapi_get_library_type() == ZWAVE_LIBRARY_TYPE_CONTROLLER_BRIDGE) {
    return zwapi_send_data_bridge(0xFF,
                                  destination_node_id,
                                  data,
                                  data_length,
                                  tx_options,
                                  callback_function);
  }

  uint8_t func_id = 0, response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  // Check for too long payloads
  if ((data_length + 2) > sizeof(request_buffer)) {
    sl_log_error(LOG_TAG,
                 "The frame is too long, therefore, the frame is discarded\n");
    assert(0);
    return SL_STATUS_FAIL;
  }

  // Fill up the buffer:  HOST->ZW: REQ | 0x13 | destination_node_id | data_length | data[] | tx_options | func_id
  zwapi_write_node_id(request_buffer, &index, destination_node_id);
  request_buffer[index++] = data_length;
  for (uint8_t i = 0; i < data_length; i++) {
    request_buffer[index++] = data[i];
  }
  request_buffer[index++] = tx_options;
  func_id                 = (callback_function == NULL ? 0 : 1);
  request_buffer[index++] = func_id;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_SEND_DATA,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    // Activate the callback function only if the Z-Wave module accepted the command
    zwapi_send_data_callback = callback_function;
    return SL_STATUS_OK;
  }
  return SL_STATUS_FAIL;
}

sl_status_t zwapi_abort_send_data(void)
{
  return zwapi_send_command(FUNC_ID_ZW_SEND_DATA_ABORT, NULL, 0);
}

sl_status_t zwapi_send_nop(zwave_node_id_t destination_node_id,
                           uint8_t tx_options,
                           void (*callback_function)(uint8_t,
                                                     zwapi_tx_report_t *))
{
  uint8_t response_length                     = 0;
  uint8_t index                               = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0};
  uint8_t response_buffer[FRAME_LENGTH_MAX]   = {0};

  zwapi_write_node_id(request_buffer, &index, destination_node_id);
  request_buffer[index++] = tx_options;
  request_buffer[index++] = (callback_function == NULL ? 0 : 1);

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_SEND_NOP,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    // Activate the callback function only if the Z-Wave module accepted the command
    zwapi_send_nop_callback = callback_function;
    return SL_STATUS_OK;
  } else if (send_command_status == SL_STATUS_NOT_SUPPORTED) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  return SL_STATUS_FAIL;
}
