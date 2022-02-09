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
 * @defgroup zwave_s0_network Security 0 Network
 * @ingroup zwave_s0_transport
 * @brief Interface for requesting S0 bootstrapping of a node and resistering
 *        callbacks to get notified when the S0 bootstrapping has failed or
 *        succeeded.
 *
 * @{
 */

#ifndef ZWAVE_S0_NETWORK_H
#define ZWAVE_S0_NETWORK_H

#include <stdint.h>
#include "zwave_network_management.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Bootstrapping S0 complete event.
 * @param granted_keys
 * @param kex_fail_code
 *
 * @returns true if success, false if failure
 */
typedef void (*s0_on_bootstrapping_complete_cb)(
  zwave_keyset_t granted_keys, zwave_kex_fail_type_t kex_fail_code);
/**
 * @brief Initialize the S0 network layer.
 *
 * This function must be called when ever the home ID or node ID of the
 * controller changes.
 *
 */
sl_status_t zwave_s0_network_init();
/**
 * @brief Set callback function for the S0 inclusion. This function ptr is saved
 * internally in s0.
 *
 * @param cb
 */
void zwave_s0_set_network_callbacks(const s0_on_bootstrapping_complete_cb cb);

/**
 * @brief Start the S0 add node process.
 *
 * Calling this function will make the S0 FSM start the inclusion process of a
 * newly added node. This must be called right after ADD_NODE_DONE event for
 * nodes supporting COMMAND_CLASS_SECURITY0
 *
 * @param node_id         The node we wish to S0 bootstrap
 * @param is_controller   Set to 1 if the node to bootstrap
 *                        is a controller, 0 for an end node.
 */
void zwave_s0_start_bootstrapping(zwave_node_id_t node_id,
                                  uint8_t is_controller);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_S0_NETWORK_H
/** @} end zwave_s0_network */
