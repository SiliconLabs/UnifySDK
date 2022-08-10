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
#include "zwave_tx_callbacks.h"
#include "zwave_tx_fixt.h"
#include "zwave_tx_process.h"
#include "zwave_tx_queue.hpp"

// Includes from other components
#include "zwapi_protocol_transport.h"
#include "zwave_controller_internal.h"
#include "zwave_controller_callbacks.h"

// Unify includes
#include "sl_log.h"

// Generic includes
#include <string.h>

#define LOG_TAG "zwave_tx_callbacks"

// Shared variables from the Z-Wave TX Process
extern zwave_tx_queue tx_queue;
extern zwave_tx_session_id_t current_tx_session_id;

// Static variables
zwave_controller_connection_info_t last_received_connection_info;
zwapi_tx_report_t last_received_tx_report;

void zwave_tx_on_frame_received(zwave_node_id_t node_id)
{
  zwave_tx_process_inspect_received_frame(node_id);
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

  // Copy the transmission results to the session_id (user pointer)
  sl_status_t queue_status
    = tx_queue.set_transmissions_results(user,
                                         status,
                                         &last_received_tx_report);

  if (SL_STATUS_OK != queue_status) {
    sl_log_warning(LOG_TAG,
                   "Transport Callback for a non-existing queue element: id=%p",
                   user);
    tx_queue.log(false);
    return;
  }

  // Get TX to look at the queue again, now that we are done.
  process_post(&zwave_tx_process, ZWAVE_TX_SEND_OPERATION_COMPLETE, user);
}

void zwave_tx_on_new_network_entered(zwave_home_id_t,
                                     zwave_node_id_t,
                                     zwave_keyset_t,
                                     zwave_kex_fail_type_t)
{
  // This will re-init and unlock the Tx Queue to accept frames again.
  zwave_tx_process_open_tx_queue();
}