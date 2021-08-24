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
 * @defgroup zwave_command_class_supervision Supervision Command Class
 * @ingroup command_classes
 * @brief A Z-Wave Supervision Command Class handlers and control functions
 *
 * This module provides both support and control of the Supervision Command Class.
 * It is composed of the following two submodules:
 * - @ref zwave_command_class_supervision_handler
 * - @ref zwave_command_class_supervision_process
 *
 * The handler parses incoming Get and Report Command
 * and provides an API to send Supervision encapsulated frames.
 *
 * The process keeps track of controlled Supervision Sessions
 * and time out waiting for status reports.
 *
 * A frame transmission is illustrated in the following figure:
 * \image html zwave_command_class_supervision.png "Supervision send data example"
 *
 * And a frame transmission with a missing Supervision report is
 * illustrated in the following figure:
 * \image html zwave_command_class_supervision_timeout.png "Supervision send data with timeout example"
 *
 */

/**
 * @defgroup zwave_command_class_supervision_handler Supervision Command Class
 * @ingroup zwave_command_class_supervision
 * @brief A Z-Wave Supervision Command Class handlers
 *
 * This module provides both support and control of the Supervision Command Class.
 * It provides an API to send data, which will encapsulate the frame and send out.
 * It also provides a Supervision Command Class handler that is registered to the
 * @ref zwave_command_handler
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_SUPERVISION_H
#define ZWAVE_COMMAND_CLASS_SUPERVISION_H

#include "sl_status.h"
#include "zwave_controller_connection_info.h"
#include "zwave_rx.h"
#include "zwave_tx.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * @brief Setup fixture for the Supervision Command Class.
 *
 * This setup will register the Supervision command handler
 * to the Z-Wave CC framework,
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_command_class_supervision_init(void);

/**
 * @brief Handle incoming Supervision encapsulated commands.
 *
 * @param connection_info Info about the connection properties of this frame.
 * @param frame_data The data payload of this frame.
 * @param frame_length The length of this frame.
 * @returns sl_status_t representing the outcome of receiving the frame
 *          In case where the command is controlled, it should be set to SL_STATUS_OK,
 *          even when encountering some parsing errors
 *          In case where the command is supported, it should be set to the @ref sl_status_t
 *          corresponding to the correct Supervision Status code. Refer to @ref zwave_command_handler_t
 */
sl_status_t zwave_command_class_supervision_support_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length);

/**
 * @brief Take a frame and create a Supervision Session.
 *
 * This function is used to transmit Z-Wave frames using Supervision
 * encapsulation. The provided payload will be encapsulated in a
 * Supervision session, then sent out.
 *
 * The Supervision module will wait for a Supervision Report
 * before it callbacks the application.
 *
 *
 * @param connection       Refer to @ref zwave_tx_send_data
 * @param data_length      Refer to @ref zwave_tx_send_data
 * @param data             Refer to @ref zwave_tx_send_data
 * @param tx_options       Refer to @ref zwave_tx_send_data
 * @param on_supervision_complete Refer to @ref zwave_tx_send_data.
 *                                Note: The status parameter
 *                                in the on_supervision_complete callback
 *                                will be the Supervision Report Status
 *                                nd not the Transmit Status.
 * @param user             Refer to @ref zwave_tx_send_data
 * @param session          Refer to @ref zwave_tx_send_data
 *
 * @returns
 * - SUCCESS The transmission request has been accepted and callback will be
 *           triggered when the operation is completed.
 * - FAILURE The transmission request has been rejected, callback will not
 *           be called.
 */
sl_status_t zwave_command_class_supervision_send_data(
  const zwave_controller_connection_info_t *connection,
  uint16_t data_length,
  const uint8_t *data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_supervision_complete,
  void *user,
  zwave_tx_session_id_t *session);

/**
 * @brief Abort a queued or ongoing transmission.
 *
 * Refer to @ref zwave_tx_abort_transmission for a
 * detailed description
 *
 * @param  session Refer to @ref zwave_tx_abort_transmission
 *
 * @returns refer to @ref zwave_tx_abort_transmission
 */
sl_status_t zwave_command_class_supervision_abort_send_data(
  zwave_tx_session_id_t session);

/**
 * @brief Request that a node is "Waked" Up on demand
 * at the next communication.
 *
 * Note that the Supervision module will only ensure that
 * the node supports Supervision Command Class, version 2
 * It will not verify that it supports the Wake Up Command Class,
 * version 3 or the Wake-Up on demand capability.
 *
 * @param node_id The node ID of the node to Wake-Up on demand
 *
 * @returns sl_status indicating if the request was accepted.
 */
sl_status_t
  zwave_command_class_supervision_wake_on_demand(zwave_node_id_t node_id);

/**
 * @brief Cancel a request that a node is "Waked" Up on demand
 * at the next communication.
 *
 * @param node_id The node ID of the node that no longer is requested
 * to Wake-Up on demand
 *
 * @returns sl_status
 * - SL_STATUS_OK indicating if the node is not (or no longer) part
 *   of the Wake On Demand node list.
 */
sl_status_t
  zwave_command_class_supervision_stop_wake_on_demand(zwave_node_id_t node_id);

/**
 * @brief Close a supervision session based on the Z-Wave Tx session ID
 *
 * @param tx_session_id   The Z-Wave TX session ID associated to the Supervision
 *                        Session that must be closed.
 *
 * @returns sl_status
 * - SL_STATUS_OK if the Session ID was closed.
 * - SL_STATUS_NOT_FOUND if there was no session ID found for the
 *                       Z-Wave TX Session ID.
 */
sl_status_t zwave_command_class_supervision_close_session_by_tx_session(
  zwave_tx_session_id_t tx_session_id);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_SUPERVISION_H
/** @} end zwave_command_class_supervision_handler */
