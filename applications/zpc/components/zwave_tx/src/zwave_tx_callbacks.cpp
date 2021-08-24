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
#include "zwave_tx_callbacks.h"
#include "zwave_tx_process.h"
#include "zwave_tx_queue.h"
#include "zwave_tx_intercept_payload.h"
#include "zwave_controller_internal.h"

// Includes from other components
#include "zwapi_protocol_transport.h"
#include "sl_log.h"
#include <string.h>

#define LOG_TAG "zwave_tx_callbacks"

// Shared variables from the Z-Wave TX Process
extern zwave_tx_queue tx_queue;
extern zwave_tx_session_id_t current_tx_session_id;

// Static variables
zwave_controller_connection_info_t last_received_connection_info;
zwapi_tx_report_t last_received_tx_report;

void zwave_tx_on_frame_received(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Copy the last received frame data into our local variable.
  last_received_connection_info = *connection_info;
  // We just need to look at the connection_info. Pass this pointer
  process_post(&zwave_tx_process,
               ZWAVE_TX_FRAME_RECEIVED,
               &last_received_connection_info);
}

void zwave_tx_send_data_callback(uint8_t status, zwapi_tx_report_t *tx_status)
{
  // Copy the callbacks data into the element
  sl_status_t queue_status
    = tx_queue.set_transmissions_results(current_tx_session_id,
                                         status,
                                         tx_status);

  if (SL_STATUS_OK != queue_status) {
    sl_log_warning(LOG_TAG,
                   "Callback for a non-existing queue element. Probably a "
                   "zwave_tx_abort_transmission() colliding with the Z-Wave "
                   "API callback. Frame was transmitted but the caller will "
                   "get no callback. Session ID: %p\n",
                   current_tx_session_id);
    tx_queue.log(false);
  }
  // We want to notify the Z-Wave Controller about transmission status of a frame.
  // This enables the controller to track when a given node is not reachable.
  zwave_tx_queue_element_t completed_element = {0};
  tx_queue.get_by_id(&completed_element, current_tx_session_id);
  if (completed_element.connection_info.remote.is_multicast == false) {
    if ((status == TRANSMIT_COMPLETE_OK)
        || (status == TRANSMIT_COMPLETE_VERIFIED)) {
      zwave_controller_on_frame_transmission_success(
        completed_element.connection_info.remote.node_id);

    } else if (completed_element.zwave_tx_options.fasttrack == false) {
      zwave_controller_on_frame_transmission_failed(
        completed_element.connection_info.remote.node_id);
    }
  }

  process_post(&zwave_tx_process, ZWAVE_TX_SEND_OPERATION_COMPLETE, 0);
}

void on_zwave_transport_send_data_complete(uint8_t status,
                                           const zwapi_tx_report_t *tx_status,
                                           void *user)
{
  if (tx_status != nullptr) {
    last_received_tx_report = *tx_status;
  } else {
    memset(&last_received_tx_report, 0, sizeof(zwapi_tx_report_t));
  }

  // Copy the transmission results to the parent session_id (user pointer)
  sl_status_t queue_status
    = tx_queue.set_transmissions_results(user,
                                         status,
                                         &last_received_tx_report);

  if (SL_STATUS_OK != queue_status) {
    sl_log_warning(LOG_TAG,
                   "Transport Callback for a non-existing queue element. "
                   "Session ID: %p\n",
                   user);
    tx_queue.log(false);
    return;
  }

  // No reason to back-off for the parent frame, which will not be sent.
  tx_queue.set_expected_responses(user, 0);
  // Get TX to look at the queue again, now that we are done.
  zwave_tx_process_check_queue();
}

void zwave_tx_send_data_multi_callback(uint8_t status)
{
  // Same as for regular send data, exept that we get no TX report.
  zwave_tx_send_data_callback(status, nullptr);
}

void zwave_tx_send_test_frame_callback(uint8_t status)
{  // Same as for regular send data, exept that we get no TX report.
  zwave_tx_send_data_callback(status, nullptr);
}

// Callbacks from zwave_controller
void zwave_tx_on_node_info_req_failed(zwave_node_id_t node_id)
{
  if (zwave_tx_intercept_waiting_for_nif()) {
    zwave_tx_send_data_callback(TRANSMIT_COMPLETE_FAIL, nullptr);
  }
}

void zwave_tx_on_node_information(zwave_node_id_t node_id,
                                  const zwave_node_info_t *node_info)
{
  if (zwave_tx_intercept_waiting_for_nif()) {
    zwave_tx_send_data_callback(TRANSMIT_COMPLETE_OK, nullptr);

    // Additionally shortcut the 1 second back-off, by pretending that the node
    // replied... Which it did.
    zwave_controller_connection_info_t info = {};
    info.remote.node_id                     = node_id;
    info.remote.endpoint_id                 = 0;
    zwave_tx_on_frame_received(&info, nullptr, nullptr, 0);
  }
}
