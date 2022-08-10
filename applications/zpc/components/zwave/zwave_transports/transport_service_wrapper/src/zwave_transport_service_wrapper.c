/******************************************************************************
* # License
* <b>Copyright 2021  Silicon Laboratories Inc. www.silabs.com</b>
******************************************************************************
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
*****************************************************************************/
#include "transport_service.h"

#include <stdint.h>

#include "zwave_controller_internal.h"
#include "zwave_controller_callbacks.h"
#include "zwave_controller_connection_info.h"
#include "zwave_controller_transport.h"
#include "zwave_controller_storage.h"
#include "zwave_tx.h"
#include "zwave_command_class_indices.h"
#include "zwave_controller_utils.h"
#include "zwave_tx_scheme_selector.h"

#include "ZW_classcmd.h"

#include "sl_log.h"
#define LOG_TAG "zwave_transport_service_wrapper"

//Send data stuff
static bool transport_service_busy = false;
static on_zwave_tx_send_data_complete_t
  intercepted_on_zwave_tx_send_data_complete
  = 0;
static on_lower_layer_send_data_complete_t
  intercepted_on_lower_layer_send_data_complete
  = 0;

static zwave_tx_session_id_t zwave_tx_parent_session_id = NULL;
static bool zwave_tx_valid_parent_session_id            = false;

static void upper_layer_command_handler(ts_node_id_t source,
                                        ts_node_id_t dest,
                                        const uint8_t *frame,
                                        uint16_t frame_len)
{
  zwave_controller_connection_info_t conn = {};
  zwave_rx_receive_options_t rx_options   = {};

  conn.local.node_id  = dest;
  conn.remote.node_id = source;
  conn.encapsulation  = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  zwave_controller_on_frame_received(&conn, &rx_options, frame, frame_len);
}

static void on_zwave_tx_send_data_complete(uint8_t status,
                                           const zwapi_tx_report_t *tx_info,
                                           void *user)
{
  (void)tx_info;
  (void)user;
  if (!intercepted_on_lower_layer_send_data_complete) {
    sl_log_debug(LOG_TAG,
                 "No callback resistered by transport service for "
                 "this transmission\n");
    return;
  }
  if (status == TRANSMIT_COMPLETE_OK) {
    intercepted_on_lower_layer_send_data_complete(0, 0);
  } else {
    intercepted_on_lower_layer_send_data_complete(1, 0);
  }
}

static uint8_t send_data(ts_node_id_t source,
                         ts_node_id_t dest,
                         const uint8_t *payload,
                         const uint16_t payload_len,
                         uint8_t no_of_expected_responses,
                         const on_lower_layer_send_data_complete_t cb)
{
  zwave_controller_connection_info_t conn = {};
  zwave_tx_options_t options              = {};
  /* Transport service is asking to send a frame. Save the callback sent
    * by Transport service here in wrapper at
    * intercepted_on_lower_layer_send_data_complete
    * and send new function on_zwave_tx_send_data_complete() as callback to
    * zwave_tx_send_data(). When on_zwave_tx_send_data_complete() called by
    * zwave_tx_send_data() wrapper will call
    * intercepted_on_lower_layer_send_data_complete()
    */
  intercepted_on_lower_layer_send_data_complete = cb;
  conn.local.node_id                            = source;
  conn.remote.node_id                           = dest;
  conn.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;

  options.number_of_responses               = no_of_expected_responses;
  options.transport.valid_parent_session_id = zwave_tx_valid_parent_session_id;
  options.transport.parent_session_id       = zwave_tx_parent_session_id;

  // FIXME: Here the concept of parent session id is quite dangerous
  // in this context, the thing is that tranport service can
  // start receiving instead of sending due to the tie breaking rules.
  if (transport_service_busy) {
    options.transport.valid_parent_session_id = true;
    options.transport.parent_session_id       = zwave_tx_parent_session_id;
  } else {
    // Frames sent by Transport service as part of the protocol should have more
    // priority than responses to get frames which transport service assembled.
    // For e.g.
    // frames like Transport service segment complete need higher priority than
    // response to the (fragmented) get command sent by Transport Service from
    // other side.
    options.qos_priority = ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY
                           + (ZWAVE_TX_RECOMMENDED_QOS_GAP * 2);
  }

  if (zwave_tx_send_data(&conn,
                         payload_len,
                         payload,
                         &options,
                         on_zwave_tx_send_data_complete,
                         0,
                         0)
      == SL_STATUS_OK) {
    return 0;
  } else {
    return 1;
  }
}

static void on_transport_service_send_data_complete(uint8_t status, void *user)
{
  (void)user;
  transport_service_busy = false;
  if (!intercepted_on_zwave_tx_send_data_complete) {
    return;
  }
  if (status == 0) {
    intercepted_on_zwave_tx_send_data_complete(TRANSMIT_COMPLETE_OK,
                                               0,
                                               zwave_tx_parent_session_id);
  } else {
    intercepted_on_zwave_tx_send_data_complete(TRANSMIT_COMPLETE_FAIL,
                                               0,
                                               zwave_tx_parent_session_id);
  }
}

sl_status_t zwave_transport_service_send_data(
  const zwave_controller_connection_info_t *conn_info,
  uint16_t data_length,
  const uint8_t *cmd_data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_zwave_tx_send_data_complete,
  void *user,
  zwave_tx_session_id_t parent_session_id)
{
  (void)user;
  (void)tx_options;

  if (conn_info->remote.is_multicast == true

      || IS_BROADCAST_NODE_ID(conn_info->remote.node_id)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  uint16_t maximum_payload
    = zwave_tx_scheme_get_max_payload(conn_info->remote.node_id);
  if (data_length <= maximum_payload) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Use Transport Service only if at least one is true:
  // 1. The node supports S2
  // 2. The node supports at least v2 of Transport Service.
  if (!zwave_controller_storage_is_node_s2_capable(conn_info->remote.node_id)
      && (zwave_controller_storage_get_cc_version(
            COMMAND_CLASS_TRANSPORT_SERVICE_V2,
            conn_info->remote.node_id,
            0)
          < TRANSPORT_SERVICE_VERSION_V2)) {
    sl_log_info(LOG_TAG,
                "NodeID %d does not support S2 or Transport Service, version 2 "
                "or higher. Frame (id=%p) will not be segmented.\n",
                conn_info->remote.node_id,
                parent_session_id);
    return SL_STATUS_NOT_SUPPORTED;
  }

  transport_service_send_data_return_code_t ret;
  if (transport_service_busy) {
    sl_log_warning(
      LOG_TAG,
      "Transport service is busy. Cannot service frame (parent_id=%p)",
      parent_session_id);
    return SL_STATUS_BUSY;
  }

  /* This is a request for Transport service to send payload
   * Intercept the callback (intercepted_on_zwave_tx_send_data_complete)
   * and send on_transport_service_send_data_complete() as new callback to
   * transport service. When transport service will call
   * on_transport_service_send_data_complete(), the wrapper will call
   * intercepted_on_zwave_tx_send_data_complete()
   */
  intercepted_on_zwave_tx_send_data_complete = on_zwave_tx_send_data_complete;
  zwave_tx_parent_session_id                 = parent_session_id;
  zwave_tx_valid_parent_session_id           = true;

  ret = transport_service_send_data(conn_info->local.node_id,
                                    conn_info->remote.node_id,
                                    cmd_data,
                                    data_length,
                                    maximum_payload,
                                    on_transport_service_send_data_complete);

  if (ret == TRANSPORT_SERVICE_SEND_SUCCESS) {
    transport_service_busy = true;
    return SL_STATUS_OK;
  } else if (ret == TRANSPORT_SERVICE_WILL_OVERFLOW) {
    return SL_STATUS_WOULD_OVERFLOW;
  } else {
    return SL_STATUS_FAIL;
  }
}

// On frame received stuff
sl_status_t zwave_transport_service_on_frame_received(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  receive_type rx_type = 0;

  if (frame_data[COMMAND_CLASS_INDEX] != COMMAND_CLASS_TRANSPORT_SERVICE_V2) {
    sl_log_error(
      LOG_TAG,
      "Received frame dispatched to Transport Service does not start with the "
      "Transport Service Command Class. Ignoring.\n");
    return SL_STATUS_NOT_SUPPORTED;
  }

  uint8_t cmd_type = frame_data[COMMAND_INDEX] & 0xf8;
  if ((cmd_type != COMMAND_FIRST_SEGMENT_V2)
      && (cmd_type != COMMAND_SEGMENT_COMPLETE_V2)
      && (cmd_type != COMMAND_SEGMENT_REQUEST_V2)
      && (cmd_type != COMMAND_SEGMENT_WAIT_V2)
      && (cmd_type != COMMAND_SUBSEQUENT_SEGMENT_V2)) {
    sl_log_info(LOG_TAG,
                "Received an unknown command for Transport Service Command "
                "Class. Ignoring");
    return SL_STATUS_NOT_FOUND;
  }

  switch (rx_options->status_flags) {
    case RECEIVE_STATUS_TYPE_SINGLE:
      rx_type = SINGLECAST;
      break;
    case RECEIVE_STATUS_TYPE_BROAD:
      rx_type = BROADCAST;
      break;
    case RECEIVE_STATUS_TYPE_MULTI:
      rx_type = MULTICAST;
      break;
    default:
      sl_log_warning(LOG_TAG,
                     "Unknown rx_options: %d. "
                     "Assuming frame was received using Singlecast.",
                     rx_options->status_flags);
  }

  if (transport_service_on_frame_received(connection_info->remote.node_id,
                                          connection_info->local.node_id,
                                          rx_type,
                                          frame_data,
                                          frame_length)
      == true) {
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_FAIL;
  }
}

sl_status_t zwave_transport_service_transport_init()
{
  transport_service_init(1, upper_layer_command_handler, send_data);
  static zwave_controller_transport_t transport = {
    .priority          = 1,  // see zwave_controller_transport->priority
    .command_class     = COMMAND_CLASS_TRANSPORT_SERVICE_V2,
    .version           = 2,
    .send_data         = zwave_transport_service_send_data,
    .on_frame_received = zwave_transport_service_on_frame_received,
  };

  transport_service_busy = false;
  return zwave_controller_transport_register(&transport);
}
