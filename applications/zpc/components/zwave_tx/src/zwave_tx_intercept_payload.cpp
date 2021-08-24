/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "zwave_tx_intercept_payload.h"
#include "zwave_tx_callbacks.h"
// Includes from other components
#include "zwapi_protocol_controller.h"
#include "zwave_controller_command_class_indices.h"

static zwave_node_id_t node_id_awaiting_nif = 0;

sl_status_t
  zwave_tx_intercept_payload(zwave_tx_queue_element_t const &tx_queue_element)
{
  // Intercepting Protocol specific commands
  if (tx_queue_element.data_length < 1
      || tx_queue_element.connection_info.remote.is_multicast == true) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Intercept NOPs and try to use the Z-Wave API for it
  if (tx_queue_element.data[COMMAND_CLASS_INDEX] == ZWAVE_NOP_COMMAND_CLASS) {
    uint8_t zwapi_tx_options = TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE
                               | TRANSMIT_OPTION_EXPLORE;
    sl_status_t status
      = zwapi_send_nop(tx_queue_element.connection_info.remote.node_id,
                       zwapi_tx_options,
                       zwave_tx_send_data_callback);
    return status;
  }

  if (tx_queue_element.data_length < 2) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Intercept Request NIFs
  if ((tx_queue_element.data[COMMAND_CLASS_INDEX]
       == ZWAVE_PROTOCOL_COMMAND_CLASS)
      && (tx_queue_element.data[COMMAND_INDEX]
          == ZWAVE_NODE_INFO_REQUEST_COMMAND)) {

    node_id_awaiting_nif = tx_queue_element.connection_info.remote.node_id;

    sl_status_t status = zwapi_request_node_info(
      tx_queue_element.connection_info.remote.node_id);
    if (status != SL_STATUS_OK) {
      node_id_awaiting_nif = 0;
    }
    return status;
  }

  return SL_STATUS_NOT_SUPPORTED;
}

bool zwave_tx_intercept_waiting_for_nif()
{
  if (node_id_awaiting_nif != 0) {
    node_id_awaiting_nif = 0;
    return true;
  }
  return false;
}
