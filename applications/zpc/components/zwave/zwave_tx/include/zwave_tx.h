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

#ifndef ZWAVE_TX_H
#define ZWAVE_TX_H

// Includes from other components
#include "sl_status.h"
#include "zwave_controller_connection_info.h"
#include "zwapi_protocol_transport.h"
#include "zwapi_protocol_basis.h"
#include "zwave_tx_definitions.h"

#ifndef ZWAVE_TX_QUEUE_BUFFER_SIZE
// max amount of frames that can be store int the internal zwave_tx_queue
#define ZWAVE_TX_QUEUE_BUFFER_SIZE 64
#endif

#ifndef ZWAVE_TX_INCOMING_FRAMES_BUFFER_SIZE
// Max amount of NodeID that can send us frames to track.
#define ZWAVE_TX_INCOMING_FRAMES_BUFFER_SIZE 10
#endif

#ifndef ZWAVE_TX_ROUTE_CACHE_BUFFER_SIZE
// Maximum amount of NodeIDs for which we cache the number of repeaters.
#define ZWAVE_TX_ROUTE_CACHE_BUFFER_SIZE 50
#endif

/**
 * @defgroup zwave_tx Z-Wave TX
 * @ingroup zwave_controller
 * @brief This component is responsible transmission of application frames.
 *
 * This component handles all the queuing logic regarding Z-Wave frames
 * transmissions, including back-off to wait for responses.
 *
 * The component features a prioritized queue by QoS. Frames with the highest
 * QoS are transmitted first. There is no guarantee over the order of elements
 * with identical QoS.
 * The low-level encapsulation control frames,
 * such as nonce report etc, must be given the highest priority.
 *
 * When frames need encapsulation, they are passed to the \ref zwave_transports,
 * which in turns will either send the frame (e.g. \ref zwave_api) or re-queue
 * an "encapsulated version/copy" of the original frame. The original frame is
 * called a parent frame, while the new version is a child frame.
 *
 * When a frame transmission is initiated, the frame and all its children frames
 * will be processed first, even if a higher QoS frame is placed in the queue
 * in the meantime.
 *
 * The external interface is described in \ref zwave_tx_interface.
 *
 * There are a few internal sub-modules:
 * - \ref zwave_tx_callbacks is a list of callbacks registered to other
 * components.
 * - \ref zwave_tx_process takes care of processing the queue and
 * interfacing with \ref zwave_transports.
 * - \ref zwave_tx_queue is the data model definition for the Z-Wave TX
 * queue ands its elements.
 *
 * The component interactions during frame transmission is depicted in
 * the figure below:
@startuml

' Style for the diagram
!theme plain
hide footbox
title Z-Wave frame transmission\nComponents with processes in grey

' List of participants
participant "User component" as u
participant "Z-Wave TX" as tx #DEDEDE
participant "Z-Wave Transports" as zwave_transports
participant "Z-Wave API" as zwapi
participant "Z-Wave RX" as rx #DEDEDE

u -> tx: <b>zwave_tx_send_data\n(frame data, user_callback_function)
tx -> tx: Queues the frame
tx -> u: returns\nSL_STATUS_OK, zwave_tx_session_id
tx -> tx: Process verifies\nthe TX queue
tx -> zwave_transports: <b>zwave_controller_transport_send_data\n(frame data, Z-Wave TX Callback)
zwave_transports -> zwapi: <b>zwapi_send_data\n(frame data, Z-Wave TX Callback)
zwapi -> zwave_transports : SL_STATUS_OK
zwave_transports -> tx : SL_STATUS_OK
zwapi -> zwapi: Frame being sent\non the radio
rx -> rx: continuously\nmonitors\nincoming\nmessages
rx -> zwapi: <b>zwapi_poll</b>()
zwapi -> tx: Z-Wave TX Callback\n(callback registered with last call \nto zwapi_send_data)
tx -> u: User callback
@enduml
 *
 * An example where frames need to be encapsulated or require other frames
 * to be sent first is shown in the diagram below:
 *
@startuml

' Style for the diagram
!theme plain
hide footbox

title Z-Wave TX parent frame functionality with transports (example)

' List of participants
participant "User component" as u
participant "Z-Wave TX" as tx #DEDEDE
participant "Z-Wave Transports" as zwave_transports
participant "Z-Wave\nS2" as s2 #DEDEDE
participant "Z-Wave API" as zwapi
participant "Z-Wave RX" as rx #DEDEDE

rnote left of u: I have a frame\nto be encrypted\nand then sent.
u -> tx: <b>zwave_tx_send_data\n(frame data, encapsulation=S2 Auth)

rnote right of tx: <b>Queue:\n<b>- Frame 1</b>: Encapsulation S2 Auth

tx -> zwave_transports: <b>zwave_controller_transport_send_data</b>\n(Frame 1, no parent)
zwave_transports -> s2: <b>zwave_s2_send_data</b>\n(Frame 1, no parent)
s2 -> zwave_transports: SL_STATUS_OK
zwave_transports -> tx: SL_STATUS_OK

rnote right of tx: Waits that Z-Wave\nTransports callbacks for Frame 1.

loop for as many encapsulated frames needed\n(Nonce Get, Encapsulated frame, etc) to get Frame 1 delivered
  s2 -> tx: <b>zwave_tx_send_data</b>\n(Frame 2 data, encapsulation = None, Parent = Frame 1)
  rnote right of tx: <b>Queue:\n<b>- Frame 1</b>: Encapsulation S2 Auth\n<b>- Frame 2</b>: Encapsulation None, parent=Frame 1
  tx -> zwave_transports: <b>zwave_controller_transport_send_data</b>\n(Frame 2, Parent = Frame 1)
  zwave_transports -> s2: <b>zwave_s2_send_data</b>\n(Frame 2, Parent = Frame 1)
  s2 -> zwave_transports: SL_STATUS_NOT_SUPPORTED
  zwave_transports -> zwapi: <b>zwave_s2_send_data</b>\n(Frame 2, Parent = Frame 1)
  zwapi -> zwave_transports: SL_STATUS_OK
  zwave_transports -> tx: SL_STATUS_OK

  rnote right of tx: Waits that Z-Wave\nTransports callbacks for Frame 2.

  zwapi -> zwapi: Frame being sent\non the radio
  rx -> rx: continuously\nmonitors\nincoming\nmessages
  rx -> zwapi: <b>zwapi_poll</b>()
  zwapi -> tx: <b>zwapi_send_data</b> callback\n(Frame 2 transmission results)

  tx -> s2: <b>zwave_tx_send_data</b> callback\n(frame 2 transmission results)
end
s2 -> tx: <b>zwave_controller_transport_send_data</b> callback\nFrame 1 transmission completed (no more to encapsulate)

rnote right of tx: <b>Queue:\n<b>- Frame 1</b>: Encap. S2 Auth (sent via Frame 2)
tx -> tx: Process verifies the TX queue\nSee that next frame (1)\nis to be discarded because of Frame 2 replacing it.

tx -> u: <b>zwave_tx_send_data</b> callback\n(frame 1 transmission results)

@enduml
 *
 * @{
 */

/**
 * @brief Function signature for callbacks invoked when send data has completed
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
 * @param connection          Connection object describing the source and
 *                            destination. If either the source or destination
 *                            endpoints ID are not null, the frame will
 *                            be Multi Channel encapsulated.
 * @param data_length         Length of the frame to send
 * @param data                Points to the payload to send
 * @param tx_options          Transmit options to use.
 * @param on_send_complete    Callback function that will be called when the send
 *                            operation has completed
 * @param user                User pointer passed in argument of the on_send_complete
 *                            callback function
 * @param session             Pointer to location where to write the session id of
 *                            the queued message. If this is set NULL the session id
 *                            will not be written
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
 * @param session_id            Pointer to location where to write the
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
  zwave_tx_session_id_t *session_id);

/**
 * @brief Abort a queued or ongoing transmission.
 *
 * Calling this function will attempt to cancel/abort a queued transmission.
 *
 * @param  session_id      Session id returned by zwave_tx_send_data
 *
 * @returns
 * - SL_STATUS_OK          if the frame transmission has been aborted
 *                         (removed from the queue and not sent)
 * - SL_STATUS_NOT_FOUND   if the session_id is not valid.
 */
sl_status_t zwave_tx_abort_transmission(zwave_tx_session_id_t session_id);

/**
 * @brief Returns the total number of responses that a frame (and its parent)
 *        will generate
 * *
 * @param  session_id Session id returned by zwave_tx_send_data for a queue element
 *
 * @returns The number of expected responses for the element identified by the
 *          Session ID.  0 if the Session ID does not exist or has been removed
 *          from the Tx Queue.
 */
uint8_t zwave_tx_get_number_of_responses(zwave_tx_session_id_t session_id);

/**
 * @brief Tell the Tx Queue that a remote NodeID is about to send some frames
 *        and we should back-off a little more.
 *
 * Calling this function back-to-back with the same NodeID will update the
 * number of frames that we are expecting from a given NodeID.
 *
 * @param remote_node_id             The NodeID that is about to send us something.
 * @param number_of_incoming_frames  The number of expected frames
 *
 */
void zwave_tx_set_expected_frames(zwave_node_id_t remote_node_id,
                                  uint8_t number_of_incoming_frames);

/**
 * @brief Returns a pointer to frame data for a given Z-Wave Tx session ID.
 * @param  session_id Session id returned by zwave_tx_send_data for a queue element
 *
 * @returns NULL if the session ID is not found, else a pointer to a payload.
 */
const uint8_t *zwave_tx_get_frame(zwave_tx_session_id_t session_id);

/**
 * @brief Returns the length of a frame in the Z-Wave Tx Queue.
 * @param  session_id Session id returned by zwave_tx_send_data for a queue element
 *
 * @returns 0 if the session ID is not found, the length of the frame in bytes.
 */
uint16_t zwave_tx_get_frame_length(zwave_tx_session_id_t session_id);

/**
 * @brief Verifies if we have frames in the queue to send to a NodeID.
 * @param node_id   NodeID to check for in the queue
 *
 * Note that this function will return true when we are backing off waiting
 * for an answer following a frame we have just sent, because the frame is still
 * in the queue.
 *
 * @returns true if we have frames to send to the NodeID, false otherwise.
 */
bool zwave_tx_has_frames_for_node(zwave_node_id_t node_id);

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
 * @brief Log the contents of the TX Queue.
 *
 * @param  with_contents will print out the content of each
 * queue element
 *
 */
void zwave_tx_log_queue(bool with_contents);

/**
 * @brief Log the details about an element in the Tx Queue
 *
 * @param session_id            Session id returned by zwave_tx_send_data
 * @param log_frame_payload   Display the frame payload in the logs
 */
void zwave_tx_log_element(zwave_tx_session_id_t session_id,
                          bool log_frame_payload);

/**
 * @brief Returns the current number of elements in the Tx Queue
 *
 * @returns int
 */
int zwave_tx_get_queue_size();

#ifdef __cplusplus
}
#endif
/** @} end zwave_tx */

#endif  //ZWAVE_TX_H
