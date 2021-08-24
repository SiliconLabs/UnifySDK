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

#ifndef ZWAVE_COMMAND_CLASS_SUPERVISION_PROCESS_H
#define ZWAVE_COMMAND_CLASS_SUPERVISION_PROCESS_H

// Contiki includes
#include "process.h"
#include "etimer.h"

/**
 * @defgroup zwave_command_class_supervision_process Z-Wave Command Class Supervision process
 * @ingroup zwave_command_class_supervision
 * @brief Process taking care of monitoring Supervision sessions that the ZPC
 * initiated.
 *
 * @{
 */

/// Default duration (ms) for a node to return a Supervision Report
#define SUPERVISION_REPORT_TIMEOUT            8000
#define SUPERVISION_SEND_DATA_EMERGENCY_TIMER 60000

/**
 * @brief Name the of \ref contiki process for the Supervision Command Class.
 *
 * This is used to register the name of the Supervision Command Clas Process.
 */
PROCESS_NAME(zwave_command_class_supervision_process);

/**
 * @brief Event definitions for the Z-Wave Command Class Supervision Process.
 */
typedef enum {
  /// Start a timer for a Supervised session.
  ZWAVE_COMMAND_CLASS_SUPERVISION_START_TIMER
} zwave_command_class_supervision_events_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Object containing tracking information for a Supervision Session
 * that the ZPC initiated and supervises
 */
typedef struct supervised_session {
  ///> Session information with the remote node.
  supervision_session_t session;
  ///> Callback for the application that initiated this supervision session
  on_zwave_tx_send_data_complete_t callback;
  /// User pointer for the callback function
  void *user;
  ///> Used to indicate if the data contained in tx_info is valid
  bool tx_info_valid;
  /// used to forward the tx_info to the user callback.
  zwapi_tx_report_t tx_info;
  ///> The Supervision Status value (SUPERVISION_REPORT_SUCCESS, SUPERVISION_REPORT_FAIL, etc.)
  uint8_t status;
  ///> Timeout after which we consider the supervision to have failed.
  clock_time_t expiry_time;
  ///> Indicates if the value in tx_session_id is valid.
  bool tx_session_valid;
  ///> Z-Wave Tx session ID used to transmit the Supervision Get (follow-up)
  zwave_tx_session_id_t tx_session_id;
} supervised_session_t;

/**
 * @brief Retrieves a supervised session based on its ID.
 *
 * @param session_id The Supervision Session ID to find among the
 *                   supervised sessions.
 *
 * @returns
 * - supervised_session_t Pointer of the found supervised session if the ID
 *                        is found
 * - NULL                 If the Session ID is not supervised.
 *
 * @note: be careful with the pointer, do not call
 * @ref zwave_command_class_supervision_create_session or
 * @ref zwave_command_class_supervision_close_session while
 * reading or writing using the returned pointer.
 */
supervision_id_t zwave_command_class_supervision_find_session(
  uint8_t session_id, zwave_node_id_t node_id, zwave_endpoint_id_t endpoint_id);

supervised_session_t *zwave_command_class_supervision_find_session_by_unique_id(
  supervision_id_t supervision_id);

/**
 * @brief Creates a new supe a supervised session
 *
 * @param connection    Connection info received for the supervision session
 * @param tx_options    Tx options received for the supervision session
 * @param callback      User callback function for when the
 *                      supervision reports statuses. Refer
 *                      to @ref on_zwave_tx_send_data_complete_t
 * @param user          User pointer to pass to the callback function.
 *
 * @returns supervision_id_t Unique handle for the Session ID
 *          INVALID_SUPERVISION_ID in case of error / multicast transmission
 */

supervision_id_t zwave_command_class_supervision_create_session(
  const zwave_controller_connection_info_t *connection,
  const zwave_tx_options_t *tx_options,
  on_zwave_tx_send_data_complete_t callback,
  void *user);

/**
 * @brief Closes a supervised session
 *
 * @param supervision_id The unique Supervision Session ID to find among the
 *                        supervised sessions.
 *
 * @returns sl_status_t indicating if the session was closed.
 * - SL_STATUS_OK if the session was found and closed
 * - SL_STATUS_NOT_FOUND if the session was not found
 */
sl_status_t zwave_command_class_supervision_close_session(
  supervision_id_t supervision_id);

/**
 * @brief Assigns a Z-Wave TX Session ID to a supervision Session ID.
 *
 * @param supervision_id  The Supervision ID to find among the
 *                        supervised sessions.
 * @param tx_session_id   The Z-Wave Tx session ID to associate to the
 *                        Supervision Session ID
 *
 * @returns sl_status_t indicating if the session ID was found and associated.
 * - SL_STATUS_OK if the session was found and associated
 * - SL_STATUS_NOT_FOUND if the session was not found
 */
sl_status_t zwave_command_class_supervision_assign_session_tx_id(
  supervision_id_t supervision_id, zwave_tx_session_id_t tx_session_id);

#ifdef __cplusplus
}
#endif

/** @} end of zwave_cc_supervision_process */

#endif  // ZWAVE_COMMAND_CLASS_SUPERVISION_PROCESS_H
