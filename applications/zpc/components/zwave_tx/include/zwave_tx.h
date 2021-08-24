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

#ifndef ZWAVE_TX_H
#define ZWAVE_TX_H

// Includes from other components
#include "sl_status.h"
#include "zwave_controller_connection_info.h"
#include "zwapi_protocol_transport.h"
#include "zwapi_protocol_basis.h"

/**
 * @defgroup zwave_tx Z-Wave TX
 * @ingroup components
 * @brief This component is responsible transmission of application frames.
 *
 * This component handles all the queuing logic regarding Z-Wave frames
 * transmission, back-off, and some frame encapsulation such as Security 0,
 * Security 2, Transport service, Multi Channel (endpoints) and
 * CRC16 encapsulation.
 *
 * The component features a prioritized queue by QoS. Frames with the highest
 * QoS are transmitted first. There is no guarantee over the order of elements
 * with identical QoS.
 * The low-level encapsulation control frames,
 * such as nonce report etc, must be given the highest priority.
 *
 * The external interface is described in \ref zwave_tx_interface.
 *
 * There are 4 internal sub-modules:
 * - \ref zwave_tx_callbacks is a list of callbacks registered to other
 * components.
 * - \ref zwave_tx_process takes care of processing the queue and
 * interfacing with \ref ZWAPI.
 * - \ref zwave_tx_intercept intercepts the Z-Wave Frame that can not be sent
 * using zwapi_send_data command and send the frame using other zwapi transport
 * commands.
 * - \ref zwave_tx_queue is the data model definition for the Z-Wave TX
 * queue ands its elements.
 *
 * The component interactions during frame transmission is depicted in
 * the figure below:
 * \image html zwave_tx_send_frame_diagram.png "Z-Wave TX component interaction"
 *
 * The \ref zwave_tx component also makes use of the S2 engine to encrypt incoming frames
 * It leverages the parent frame functionality described in \ref zwave_tx_options_t
 * \image html zwave_tx_parent_frames_functionality.png "Z-Wave TX encrypting a frame"
 */

/**
 * @defgroup zwave_tx_options_qos_priority Z-Wave TX Options QoS priority
 * @ingroup zwave_tx_interface
 * @brief QoS Priority for Z-Wave TX Options
 *
 * The QoS Priority is used to determine the order of frames being sent.
 * It is a 32 bits value, with the normal ranging from 0 to 0xFFFFFFFF. (32 bits)
 *
 * Inclusion and other timing critical frames should high priority.
 * Frames that can wait should have a low priority.
 * ZWAVE_TX_QOS_MAX_PRIORITY should not be used, unless really in an emergency
 * situation.
 * @{
 */
/// Maximum QoS priority for \ref zwave_tx_options_t qos_priority.
/// Nothing is more important than these frames.
#define ZWAVE_TX_QOS_MAX_PRIORITY 0xFFFFFF00
/// Recommended QoS priority for \ref zwave_tx_options_t qos_priority
/// when sending timing critical frames.
#define ZWAVE_TX_QOS_RECOMMENDED_TIMING_CRITICAL_PRIORITY 0x0FFFFFFF
/// Recommended QoS priority for \ref zwave_tx_options_t qos_priority
/// for replying to incoming Get type commands.
#define ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY 0x00FFFFFF
/// Recommended QoS priority for \ref zwave_tx_options_t qos_priority
/// for issuing probing commands (node interview).
#define ZWAVE_TX_QOS_RECOMMENDED_NODE_INTERVIEW_PRIORITY 0x0000FFFF
/// Recommended QoS priority for \ref zwave_tx_options_t qos_priority
/// for state polling of a node (e.g. verifying its On/off state
/// if it does not report to the Lifeline).
#define ZWAVE_TX_QOS_RECOMMENDED_POLLING_PRIORITY 0x00000FFF
/// Minimum QoS priority for \ref zwave_tx_options_t qos_priority.
#define ZWAVE_TX_QOS_MIN_PRIORITY 0x00000000

/// Recommended QoS gap between frames to accomodate for parent/child frames
#define ZWAVE_TX_RECOMMENDED_QOS_GAP 10
/** @} end zwave_tx_options_qos_priority */

/**
 * @defgroup zwave_tx_interface Z-Wave TX interface
 * @ingroup zwave_tx
 * @brief This is the public interface for the Z-Wave TX component.
 *
 * \ref zwave_tx_send_data() and \ref zwave_tx_abort_transmission()
 * are the two main functions to be used from other components.
 *
 * \ref zwave_tx_init() and \ref zwave_tx_shutdown should not be called
 * by other components.
 *
 * @{
 */

/**
 * @brief Handle that can be used for aborting ongoing transmissions
 * or identifying TX Queue elements.
 */
typedef void *zwave_tx_session_id_t;

typedef struct zwave_tx_options {
  /// The \ref zwave_tx_process will wait for the expected responses to a frame before
  /// transmitting the next frame. It will time out and resume TX operations after a
  /// recommended backoff time for the expected number of responses
  /// If S2 uses the VERIFY_DELIVERY mechanism, the number of expected responses should be
  /// equal or greater than 1.
  /// If is possible to specify that multiple responses are expected.
  /// In case of Supervision with status update flag, the number_of_responses should
  /// be set to 1, as only one frame will be returned immediately for sure, the other(s) one will
  /// come later after an arbitrary time.
  /// If the use_parent_frame_options and valid_parent_session_id fields are set to true
  /// and a valid parent_session_id is provided in the parent_session_id, this value will
  /// be overwritten with the number_of_responses value of the parent.
  uint8_t number_of_responses;

  /// Maximum time in ms this transmission is allowed to spend in queue waiting
  /// to be processed before it is dropped. Discard timeout of 0 means to never
  /// drop the frame.
  /// If the use_parent_frame_options and valid_parent_session_id fields are set to true
  /// and a valid parent_session_id is provided in the parent_session_id, this value will
  /// be overwritten with the discard_timeout_ms value of the parent.
  uint32_t discard_timeout_ms;

  /// Priority of transmission element. Frames with higher numbers are sent
  /// first.
  /// If the valid_parent_session_id field is set to true and a valid parent_session_id is
  /// provided in the parent_session_id, this value will be overwritten with the
  /// qos_priority value of the parent + 1.
  uint32_t qos_priority;

  /// This flag indicates if the message should be attempted to be sent without
  /// beaming or route resolution. This option should only be used for FL nodes.
  /// If the transmission fails, the message will be re-queued and attempted
  /// again later.
  bool fasttrack;

  /// zwave_tx_session_id_t of the parent frame. A parent frame is a frame that caused
  /// this frame to be added to the queue. Child frames are sent before their parent.
  zwave_tx_session_id_t parent_session_id;

  /// This flag indicates if the parent_session_id field is supposed to contain
  /// a valid value.
  /// If this field is set to true, qos_priority will be overwritten so that the
  /// frame is sent before the parent.
  bool valid_parent_session_id;

  /// This flag indicates if the number_of_responses and discard_timeout_ms
  /// fields should be determined from the parent_session_id
  /// If the valid_parent_session_id field is additionally set to true,
  /// the number_of_responses and discard_timeout_ms and will be overwritten.
  bool use_parent_frame_options;

  /// This flag indicates if the frame is to be sent as a test frame
  /// Test frame was intended to be used to test link reliability, the
  /// Z-Wave API will send a test frame without any routing and with 9600 kbit/s
  /// transmission speed. The payload will also be ignored.
  bool is_test_frame;

  /// This value indicates if the a test frame must be sent
  /// with a particular Tx Power. This value will be ignored if the
  /// is_test_frame flag is set to false.
  rf_power_level_t rf_power;

  /// This flag can be used for tracking multicast/singlecast follow-ups
  /// transmissions.
  /// For Singlecast messages (remote.is_multicast = false), this must be set
  /// to ZWAVE_TX_INVALID_GROUP if the frame is not a follow-up frame, else to
  /// the group_id for which it is a follow-up.
  /// For Multicast messages (remote.is_multicast = true), if this field is
  /// set with a group_id different than 0, the Tx Queue will use the
  /// TRANSMIT_OPTION_MULTICAST_AS_BROADCAST and the multicast will be
  /// sent as a "singlecast to the broadcast destination"
  zwave_multicast_group_id_t group_id;

  /// Is this the first Singlecast follow-up frame ?
  /// Only set this to true if queuing the first follow-up frame. User components
  /// SHOULD always set this to false and let the Tx Queue handle the singlecast
  /// follow-ups.
  bool is_first_follow_up;

  /// The TX Queue can automatically queue Follow-up messages following a
  /// multicast. If you wish to activate this functionality, set this field
  /// to true.
  bool send_follow_ups;

} zwave_tx_options_t;

/**
 * @brief Callback function of zwave_tx_send_data(), indicating the result of the operation
 *
 * This function is called when an element in the TX Queue has been transmitted
 * (or attempted to be transmitted) and indicates the status of the operation.
 *
 * @param status  Indicates how the transmission operation was completed.
 *                Refer for \ref zwapi_transmit_complete_codes for details.
 * @param tx_info zwapi_tx_report_t reported by the \ref ZWAPI. It
 *                contains transmission details, refer to \ref zwapi_tx_report_t.
 * @param user    User pointer provided in \ref zwave_tx_send_data()
 */
typedef void (*on_zwave_tx_send_data_complete_t)(
  uint8_t status, const zwapi_tx_report_t *tx_info, void *user);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Queue and send frames to Z-Wave nodes.
 *
 * This function is used to transmitting Z-Wave frames. The provided payload
 * will be encapsulated into the format described in connection->encapsulation.
 * This module does not verify if the destination actually supports the requested
 * encapsulation, or if the destination exists at all (NodeID / Endpoint).
 *
 * This module will put all frames on a queue. The frames with the highest
 * qos_priority will be send first. the tx_options and connection parameters
 * allow all kind of frames to be transmitted, including multicast/broadcast.
 *
 *
 * @param connection       Connection object describing the source and
 *                         destination. If either the source or destination
 *                         endpoints ID are not null, the frame will
 *                         be Multi Channel encapsulated.
 * @param data_length      Length of the frame to send
 * @param data             Points to the payload to send
 * @param tx_options       Transmit options to use.
 * @param on_send_complete Callback function that will be called when the send
 *                         operation has completed
 * @param user             User pointer passed in argument of the on_send_complete
 *                         callback function
 * @param session          Pointer to location where to write the session id of
 *                         the queued message. If this is set NULL the session id
 *                         will not be written
 *
 * @returns
 * - SL_STATUS_OK The transmission request has been accepted and callback will be
 *                    triggered when the operation is completed.
 * - SL_STATUS_FAIL   The transmission request has been rejected, callback will not
 *                    be called.
 */
sl_status_t
  zwave_tx_send_data(const zwave_controller_connection_info_t *connection,
                     uint16_t data_length,
                     const uint8_t *data,
                     const zwave_tx_options_t *tx_options,
                     const on_zwave_tx_send_data_complete_t on_send_complete,
                     void *user,
                     zwave_tx_session_id_t *session);

/**
 * @brief Queues and send a test frame to a Z-Wave node.
 *
 * This function is used for sending a special test frame that will be
 * testing the direct range connectivity of a remote Z-Wave Node.
 *
 * Test frame are automatically send with the lowest priority.
 *
 *
 * @param destination_node_id   Z-Wave NodeID of the destination.
 * @param power_level           Powerlevel setting to apply for
 *                              the transmission. Refer to \ref rf_power_level_t
 * @param on_send_complete      Callback function that will be called when
 *                              the send operation is completed
 * @param user                  User pointer passed in argument of
 *                              the on_send_complete callback function
 * @param session               Pointer to location where to write the
 *                              session id of the queued message. If
 *                              this is set NULL the session id will
 *                              not be written
 *
 * @returns
 * - SL_STATUS_OK The transmission request has been accepted and callback will be
 *                    triggered when the operation is completed.
 * - SL_STATUS_FAIL   The transmission request has been rejected, callback will not
 *                    be called.
 */
sl_status_t zwave_tx_send_test_frame(
  zwave_node_id_t destination_node_id,
  rf_power_level_t power_level,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user,
  zwave_tx_session_id_t *session);

/**
 * @brief Abort a queued or ongoing transmission.
 *
 * Calling this function will attempt to cancel/abort a queued transmission.
 * If the transmission data has been passed to the Z-Wave module, it means that
 * no further route resolution will be made, however the frame may still get
 * transmitted and the callback of \ref zwave_tx_send_data will still be called.
 * As a consequence the transmit status of on_zwave_tx_send_data_complete_t can
 * be both TRANSMIT_COMPLETE_OK and TRANSMIT_COMPLETE_NO_ACK
 *
 * @param  session Session id returned by zwave_tx_send_data
 *
 * @returns
 * - SL_STATUS_OK          if the frame transmission has been aborted
 *                         (removed from the queue and not sent)
 * - SL_STATUS_NOT_FOUND   if the session_id is not valid.
 * - SL_STATUS_IN_PROGRESS if the frame transmission was initiated and
 *                         aborted. The on_send_complete callback will
 *                         be called.
 */
sl_status_t zwave_tx_abort_transmission(zwave_tx_session_id_t session);

/**
 * @brief Initialize the zwave_tx component.
 *
 * Z-Wave Tx will register an on_frame_received() callback to the
 * \ref zwave_controller
 *
 * @returns SL_STATUS_OK on success.
 */
sl_status_t zwave_tx_init();

/**
 * @brief Stop the tx component.
 *
 * This will stop ongoing transmission and flush all queues. No callbacks
 * will be called when queues are emptied due to a shutdown.
 * It will also unregister the on_frame_received() callback to the
 * \ref zwave_controller
 *
 */
void zwave_tx_shutdown();

/**
 * @brief Log the contents of the TX Queue.
 *
 * @param  with_contents will print out the content of each
 * queue element
 *
 */
void zwave_tx_log_queue(bool with_contents);

#ifdef __cplusplus
}
#endif
/** @} end zwave_tx */

#endif  //ZWAVE_TX_H
