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

#ifndef ZWAVE_TX_DEFINITIONS_H
#define ZWAVE_TX_DEFINITIONS_H

#include "zwave_controller_types.h"

/**
 * @defgroup zwave_tx_options_qos_priority Z-Wave TX Options QoS priority
 * @ingroup zwave_tx
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
 * @defgroup zwave_tx_interface Z-Wave TX Interface
 * @ingroup zwave_tx
 * @brief This is the public interface for the Z-Wave TX component.
 *
 * \ref zwave_tx_send_data() and \ref zwave_tx_abort_transmission()
 * are the two main functions to be used from other components.
 *
 * \ref zwave_tx_init() should not be called
 * by other components.
 *
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Handle that can be used for aborting ongoing transmissions
 * or identifying TX Queue elements.
 */
typedef void *zwave_tx_session_id_t;

/**
 * @brief Data used by Z-Wave Transports to track their "session"
 */
typedef struct zwave_tx_transport_options {
  /// zwave_tx_session_id_t of the parent frame. A parent frame is a frame
  // that caused this frame to be added to the queue. Child frames are
  // sent before their parent.
  zwave_tx_session_id_t parent_session_id;

  /// This flag indicates if the parent_session_id field is supposed to contain
  /// a valid value.
  /// If this field is set to true, qos_priority will be overwritten so that the
  /// frame is sent before the parent.
  bool valid_parent_session_id;

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
  /// Only set this to true if queuing the first follow-up frame.
  bool is_first_follow_up;

  /// This flag indicates if the frame is to be sent as a test frame
  /// Test frame was intended to be used to test link reliability, the
  /// Z-Wave API will send a test frame without any routing and with 9600 kbit/s
  /// transmission speed. The payload will also be ignored.
  bool is_test_frame;

  /// This flag indicates if the frame is to be sent even if we have incoming
  /// frames. Should be used only if the transport need a frame send/delivered
  /// at the risk of a collision.
  bool ignore_incoming_frames_back_off;

  /// This value indicates if the a test frame must be sent
  /// with a particular Tx Power. This value will be ignored if the
  /// is_test_frame flag is set to false.
  rf_power_level_t rf_power;
} zwave_tx_transport_options_t;

typedef struct zwave_tx_options {
  /// The \ref zwave_tx_process will wait for the expected responses to a frame before
  /// transmitting the next frame. It will time out and resume TX operations after a
  /// recommended backoff time for the expected number of responses
  /// It is possible to specify that multiple responses are expected.
  /// In case of Supervision with status update flag, the number_of_responses should
  /// be set to 1, as only one frame will be returned immediately for sure,
  /// the other(s) one will come later after an arbitrary time.
  uint8_t number_of_responses;

  /// Maximum time in ms this transmission is allowed to spend in queue waiting
  /// to be processed before it is dropped. Discard timeout of 0 means to never
  /// drop the frame.
  uint32_t discard_timeout_ms;

  /// Priority of transmission element. Frames with higher numbers are sent
  /// first.
  uint32_t qos_priority;

  /// This flag indicates if the message should be attempted to be sent without
  /// beaming or route resolution. This option should only be used for FL nodes.
  /// If the transmission fails, the message will be re-queued and attempted
  /// again later.
  bool fasttrack;

  /// The transports can automatically queue Follow-up messages following a
  /// multicast. If you wish to activate this functionality, set this field
  /// to true.
  bool send_follow_ups;

  /// Data for Z-Wave Transport. This should be zero'ed out by user components
  /// and used only by Z-Wave Transports.
  zwave_tx_transport_options_t transport;
} zwave_tx_options_t;

/** @} end zwave_tx_options_qos_priority */

#ifdef __cplusplus
}
#endif
#endif  //ZWAVE_TX_DEFINITION_H
