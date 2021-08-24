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

/**
 * @defgroup zwave_controller_internal Z-Wave Controller Internal definitions
 * @ingroup zwave_controller
 * @brief TODO: Write brief for zwave_controller_internal
 *
 * TODO: Write component description for zwave_controller_internal
 *
 * @{
 */

#ifndef ZWAVE_CONTROLLER_INTERNAL_H
#define ZWAVE_CONTROLLER_INTERNAL_H

#include "zwave_controller_callbacks.h"
#include "zwave_controller_keyset.h"
#include "zwave_controller.h"
#include "zwave_tx.h"

#ifdef __cplusplus
extern "C" {
#endif

/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_state_updated(zwave_network_management_state_t state);
/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_error(zwave_network_management_error_t error);
/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_node_id_assigned(zwave_node_id_t node_id,
                                          bool included_by_us);
/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_node_deleted(zwave_node_id_t node_id);
/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_node_info_req_failed(zwave_node_id_t node_id);
/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_frame_transmission_failed(zwave_node_id_t node_id);
/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_frame_transmission_success(zwave_node_id_t node_id);
/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_node_added(sl_status_t status,
                                    const zwave_node_info_t *nif,
                                    zwave_node_id_t node_id,
                                    zwave_dsk_t dsk,
                                    zwave_keyset_t granted_keys,
                                    zwave_kex_fail_type_t kex_fail_type,
                                    zwave_protocol_t inclusion_protocol);
/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_new_network_entered(
  zwave_home_id_t home_id,
  zwave_node_id_t node_id,
  zwave_keyset_t granted_keys,
  zwave_kex_fail_type_t kex_fail_type);
/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_keys_report(bool csa, zwave_keyset_t keys);
/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_dsk_report(uint8_t input_length,
                                    zwave_dsk_t dsk,
                                    zwave_keyset_t keys);
/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_frame_received(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length

);
/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_smart_start_inclusion_request(
  uint32_t home_id, bool already_included, const zwave_node_info_t *node_info, zwave_protocol_t inclusion_protocol);
/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_node_information(zwave_node_id_t node_id,
                                          const zwave_node_info_t *node_info);
/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_new_suc(zwave_node_id_t suc_node_id);

/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_multicast_group_deleted(
  zwave_multicast_group_id_t group_id);

/// see @ref zwave_controller_callbacks_t
void zwave_controller_on_request_neighbor_update(uint8_t status);
/**
 * @brief Send a frame with transport plugin system
 *
 * This function sends a frame with the transport system. It will return
 * SL_STATUS_OK if the frame is consumed
 *
 * @param connection see @ref  zwave_tx_send_data
 * @param data_length see @ref  zwave_tx_send_data
 * @param data  see @ref  zwave_tx_send_data
 * @param tx_options see @ref  zwave_tx_send_data
 * @param on_send_complete see @ref  zwave_tx_send_data
 * @param user see @ref  zwave_tx_send_data
 * @param session see @ref  zwave_tx_send_data
 * @returns sl_status_t         Indicating the processing of the frame.
 *  - SL_STATUS_OK              indicates that the frame was accepted and
 *                              encapsulated
 *  - SL_STATUS_NOT_SUPPORTED   indicates that transport has nothing to
 *                              encapsulate, the frame is ready the Z-Wave API.
 *  - SL_STATUS_WOULD_OVERFLOW  indicates that transport should encapsulate the
 *                              frame but cannot. Frame should be dropped.
 *  - Any other error code      (SL_STATUS_BUSY, SL_STATUS_FAIL, etc.) indicating
 *                              that the frame should be encapsulated,
 *                              but could not due to an error.
 */
sl_status_t zwave_controller_transport_send_data(
  const zwave_controller_connection_info_t *connection,
  uint16_t data_length,
  const uint8_t *data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user,
  zwave_tx_session_id_t session);

#ifdef __cplusplus
}
#endif
/** @} end zwave_controller_internal */

#endif  //ZWAVE_CONTROLLER_INTERNAL_H