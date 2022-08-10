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
#include "zwave_s0_network.h"
#include "zwave_s0_internal.h"
#include "zwave_s0_sm.h"
#include "zwave_s0_transport.h"
#include "sl_log.h"
#include "s2_keystore.h"

#define LOG_TAG "zwave_s0_network"

void zwave_s0_start_bootstrapping(zwave_node_id_t node_id,
                                  uint8_t is_controller)
{
  zwave_controller_connection_info_t conn = {};
  conn.local.node_id  = zwave_network_management_get_node_id();
  conn.remote.node_id = node_id;

  s0_bootstrapping_start(&conn, is_controller);
}

sl_status_t zwave_s0_network_init()
{
  uint8_t key[16];
  sl_log_debug(LOG_TAG, "Initializing Security0\n");
  s0_abort_all_tx_sessions();
  free_all_rx_session();
  free_all_tx_sessions();
  free_nonce_block_list();

  reset_block_next_elem();
  reset_s0_timers();
  if (keystore_network_key_read(KEY_CLASS_S0, key) == false) {
    sl_log_warning(LOG_TAG, "Could not read KEY_CLASS_S0 from keystore\n");
    return SL_STATUS_OK;
  } else {
    s0_set_key(key);
    return SL_STATUS_OK;
  }
}

void zwave_s0_set_network_callbacks(const s0_on_bootstrapping_complete_cb cb)
{
  s0_set_callback(cb);
}


void zwave_s0_stop_bootstrapping() {
  s0_bootstrapping_stop();
}


void zwave_s0_start_learn_mode(zwave_node_id_t our_nodeid) {
  (void)(our_nodeid);
  sl_log_warning(LOG_TAG,"S0 learn mode bootstrapping is not implemented ");
}
