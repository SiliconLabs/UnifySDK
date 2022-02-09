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

#ifndef ZWAVE_TX_CALLBACKS_H
#define ZWAVE_TX_CALLBACKS_H

// Includes from other components
#include "zwave_controller_connection_info.h"
#include "zwave_controller_types.h"
#include "zwave_controller_keyset.h"
#include "zwapi_protocol_transport.h"
#include "zwave_network_management.h"
#include "zwave_rx.h"

/**
 * @defgroup zwave_tx_callbacks Z-Wave TX Callbacks
 * @ingroup zwave_tx
 * @brief Z-Wave TX Callbacks registered to other components.
 *
 * These functions are callbacks that are registered to other components.
 * When called, they post an event in the \ref zwave_tx_process
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Callback for Z-Wave TX, for transport encapsulation.
 */
void on_zwave_transport_send_data_complete(uint8_t status,
                                           const zwapi_tx_report_t *tx_info,
                                           void *user);

// zwave_controller callback functions
/**
 * @brief Callback for Z-Wave TX, for incoming Z-Wave frames.
 *
 * Refer to \ref zwave_controller_callbacks_t on_frame_received()
 * for the functions parameter descriptions.
 */
void zwave_tx_on_frame_received(zwave_node_id_t node_id);

/**
 * @brief Callback function invoked when we are in a new network.
 *
 * See \ref zwave_controller_callbacks_t for details on the callback.
 */
void zwave_tx_on_new_network_entered(zwave_home_id_t home_id,
                                     zwave_node_id_t node_id,
                                     zwave_keyset_t granted_keys,
                                     zwave_kex_fail_type_t kex_fail_type);

#ifdef __cplusplus
}
#endif
/** @} end of zwave_tx_callbacks */

#endif  // ZWAVE_TX_CALLBACKS_H
