/* © 2017 Silicon Laboratories Inc.
 */

/*
 * s2.h
 *
 *  Created on: Jun 25, 2015
 *      Author: aes
 */

#ifndef INCLUDE_S2_H_
#define INCLUDE_S2_H_

#include<stdint.h>
#include<platform.h>

#ifndef DllExport
#define DllExport extern
#endif

#include "ctr_drbg.h"

//#include <ZW_transport_api.h>

#define S2_MSG_FLAG_EXT      0x1
#define S2_MSG_FLAG_SEXT     0x2

#define S2_NONCE_REPORT_SOS_FLAG 1
#define S2_NONCE_REPORT_MOS_FLAG 2

#define S2_MSG_EXTHDR_CRITICAL_FLAG 0x40
#define S2_MSG_EXTHDR_MORE_FLAG 0x80
#define S2_MSG_EXTHDR_TYPE_MASK 0x3F
#define S2_MSG_EXTHDR_TYPE_SN 1
#define S2_MSG_EXTHDR_TYPE_MPAN 2
#define S2_MSG_EXTHDR_TYPE_MGRP 3
#define S2_MSG_EXTHDR_TYPE_MOS  4

/* TODO move above into ZW_classcmd.h */


/**
 * \defgroup S2trans S2 transport system
 * The S2 transport system defines all the interfaces which are needed for
 * normal S2 singlecast and multicast frame transmission. This module will
 * interface the the underlying Z-Wave system calls such as ZW_SendData and
 * timer functions. The Z-Wave application command handler will also hook into
 * this module to provide frame input to the S2 layer.
 *
 * @{
 **/


/**
 * This flag will activate frame delivery.
 *
 * In this transmission mode the S2_send_data will try
 * to verify that the receiver understood the sent message.
 * This is done by waiting a little to see if the node will
 * respond nonce report to the encrypted message. If the node
 * does respond with a nonce report then the S2_send_data
 * call will automatically cause the system to re-sync the node,
 * and deliver the message
 *
 */
#define S2_TXOPTION_VERIFY_DELIVERY 1

/**
 * This flag must be present on all single cast followup messages.
 */
#define S2_TXOPTION_SINGLECAST_FOLLOWUP 2

/**
 * This flag must be present on the first, and only the first single
 * cast followup message in a multicast transmission.
 */
#define S2_TXOPTION_FIRST_SINGLECAST_FOLLOWUP 4

/**
 * Set this flag if the received message was a multicast message.
 */
#define S2_RXOPTION_MULTICAST 1

/**
 * Set this flag if the received message was a multicast message.
 */
#define S2_RXOPTION_FOLLOWUP 2

/* parameter used by the send function */

/**
 * The node type.
 */
typedef uint16_t node_t;

/**
 * Security class which is used, in S2_send_data and is set in S2_network_key_update
 * The class must be less than S2_NUM_KEY_CLASSES
 */
typedef uint8_t security_class_t;

/**
 * Key sets: Sets of keys that are different for the same Security Class.
 * We have 2 at the moment, one for Z-Wave, one for Z-Wave Long Range
 */
typedef enum {
  ///> Z-Wave keyset
  ZWAVE_KEYSET = 0x00,
  ///> Z-Wave Long Range keyset
  ZWAVE_LONG_RANGE_KEYSET = 0x01,
  ///> Unknown keyset. This is a reserved value for errors.
  UNKNOWN_KEYSET = 0xFF,
} zwave_s2_keyset_t;


/**
 * The structure holds information about the sender and receiver in a S2
 * Transmission.
 */
typedef struct peer {
  /**
   * Local node, if sending this a the node sending the message, when receiving this is the node receiving the message.
   */
  node_t l_node; //local   node

  /**
   * Remote node, when sending this is the node that should receive the message. When receiving a message, this is the
   * node for whom the message is targeted. In the special case of multicast transmission it must be set to the 8-bit multicast
   * group id.
   */
  node_t r_node;

  /**
   * This is the S2 tx flags see \ref s2txopt
   */
  uint8_t tx_options;
  /**
   * This is the S2 rx flags see \ref s2rxopt
   */
  uint8_t rx_options;

  /**
   * These is just passed transparent through the S2 transport system.
   */
  uint8_t zw_tx_options;
  uint8_t zw_rx_status;
  uint8_t zw_rx_RSSIval;
  /**
   * Use this to indicate for which GroupID a singlecast follow-up frame is.
   * This field will be ignored for multicast or regular singlecast frames.
   */
  uint8_t mgrp_group_id;
  //uint8_t zw_multicast_nodelist[29];

  /**
   * The security class which is used to send this frame, or the the class used to decrypt this frame.
   */
  security_class_t class_id;
} s2_connection_t;

/**
 * Status codes for \ref S2_send_data
 */
typedef enum {
  /**
   * Frame was sent to node, we the node should have been able to decrypt the frame
   * but we don't know for sure.
   */
  S2_TRANSMIT_COMPLETE_OK = 0x00,

  /**
   * Frame was not sent due to a MAC layer failure
   */
  S2_TRANSMIT_COMPLETE_NO_ACK = 0x01,

  /**
   * Frame was not sent because some other failure, i.e the node does not support S2
   */
  S2_TRANSMIT_COMPLETE_FAIL = 0x02,


  /**
   * We are sure that the node has decrypted the frame, because it has sent a message
   * the other way which it would only be able to send if it could decrypt this frame
   */
  S2_TRANSMIT_COMPLETE_VERIFIED = 0x5, /*TODO allocate code in ZW_transport_api.h*/

} s2_tx_status_t;


/**
 * Anonymous declaration of the S2 context, the data inside S2 context must not be
 * modified outside libs2 */
struct S2;

/**
 *  Network key type used in S2
 */
typedef uint8_t network_key_t[16];

/**
 * Send singlecast security s2 encrypted frame. Upon completion this call will call \ref S2_send_done_event. Only one transmission
 * may be active at a time. If this function is called twice without waiting to the S2_send_done_event it will return false
 *
 * \param ctxt   the security context.
 * \param peer   transmit parameters, destination node, ack req etc. see \ref s2_connection_t
 * \param buf   plaintext to which is going to be sent. The the data in this pointer must be valid until the S2_send_done_event has been called.
 * \param len   length of data to be sent.
 * \return true if the frame transmission has been initiated. false the transmission has not begun and \ref S2_send_done_event will not be called.
 *
 *
 */
uint8_t S2_send_data(struct S2* ctxt, s2_connection_t* peer ,const uint8_t* buf, uint16_t len);

/**
* Check if S2 is ready to receive a new frame for transmission
*
* Use to ensure S2_send_data will accept a frame before calling it.
* \param[in] ctxt The S2 context
* \return False if the S2_send_data method will accept a new frame for transmission
*/
uint8_t S2_is_send_data_busy(struct S2* ctxt);

/**
 * Send multicast security s2 encrypted frame.
 * This will only send the multicast frame itself. There will be no single cast follow ups.
 * a S2_send_done_event will be emitted when the transmission is done.
 *
 * \param ctxt           the security context.
 * \param dst           connection handle a handle for the mulicast group to use, this group id will also be used when calling S2_send_frame_multi
 * \param buf           plaintext to which is going to be sent.
 * \param len           length of data to be sent.
 *
 */
uint8_t S2_send_data_multicast(struct S2* ctxt,  const s2_connection_t* dst, const uint8_t* buf, uint16_t len);


/**
 * A new A new (improved) version of S2_send_data that allows to select a
 * keyset with wich the command is to be sent.
 *
 * \param ctxt            The S2 context.
 * \param connection      connection handle a handle for the singlecast
 *                        transmission
 * \param keyset          The keyset to use for the transmission.
 * \param payload         plaintext payload to send
 * \param payload_length  length of plaintext payload
 *
 * \returns 1 on success, 0 on failure
 */
uint8_t S2_send_data_singlecast_with_keyset(struct S2* ctxt,
                                            s2_connection_t* connection,
                                            zwave_s2_keyset_t keyset,
                                            const uint8_t* payload,
                                            uint16_t payload_length);

/**
 * A new (improved) version of S2_send_data_multicast that allows to select a
 * keyset with wich the command is to be sent
 * This will only send the multicast frame itself. There will be no singlecast follow ups.
 * a S2_send_done_event will be emitted when the transmission is done.
 *
 * \param ctxt            The S2 context.
 * \param connection      connection handle a handle for the mulicast group to
 *                        use.
 * \param keyset          The keyset to use for the transmission.
 * \param payload         plaintext payload to send
 * \param payload_length  length of plaintext payload
 *
 * \returns 1 on success, 0 on failure
 */
uint8_t S2_send_data_multicast_with_keyset(struct S2* ctxt,
                                           s2_connection_t* connection,
                                           zwave_s2_keyset_t keyset,
                                           const uint8_t* payload,
                                           uint16_t payload_length);


/**
 * Send a singlecast follow-up frame where we selected the MGRP ID.
 *
 * \param ctxt            The S2 context.
 * \param connection      connection handle for the destination, including a valid
 *                        mgrp_group_id field.
 * \param keyset          The keyset to use for the transmission.
 * \param payload         plaintext payload to send
 * \param payload_length  length of plaintext payload
 *
 * \returns 1 on success, 0 on failure
 */
uint8_t S2_send_data_singlecast_follow_up_with_keyset(
                                          struct S2* p_context,
                                          s2_connection_t* connection,
                                          zwave_s2_keyset_t keyset,
                                          const uint8_t* payload,
                                          uint16_t payload_length);

/**
* Check if S2 is ready to receive a new multicast frame for transmission
*
* Use to ensure S2_send_data_multicast will accept a frame before calling it.
* \param[in] ctxt The S2 context
* \return False if the S2_send_data_multicast method will accept a new frame for transmission
*/
uint8_t S2_is_send_data_multicast_busy(struct S2* ctxt);

/**
 * Allocates and Initializes a context. This must be done on every power up, or when the homeID changes.
 * The allocated context must be freed when its not longer needed, ie. when home ID changes. See \ref S2_destroy.
 * This function will also read in the network keys using the S2_keysotre. See \ref keystore.
 *
 * \param home HomeID of the context. Must use platform byte order.
 *
 * \return pointer to the allocated context. If libs2 has been compiled with SINGLE_CONTEXT the context is statically allocated.
 * libs2 in multi-context mode is only strictly need in test mode.

 * \callgraph
 */
struct S2*
S2_init_ctx(uint32_t home);


/**
 * Free's resources associated with context, the context will be invalid after a call to this function
 *
 * \param ctxt the S2 context
 */
void S2_destroy(struct S2* ctxt);

/**
 * Command handler for all incoming COMMAND_CLASS_SECURITY2 frames.
 * \param ctxt the S2 context
 * \param peer Information about the frame transaction. Rember to fill in the rx_options.
 * \param buf  pointing to the received data. The S2 machine may alter the data in this buffer.
 * \param len  The length of the received data.
 */
void S2_application_command_handler(struct S2* ctxt, s2_connection_t* peer , uint8_t* buf, uint16_t len);

/**
 * This must be called when the timer set by \ref S2_set_timeout has expired.
 * \param ctxt the S2 context
 */
void S2_timeout_notify(struct S2* ctxt);

/**
 * Notify the security stack that sent with \ref S2_send_frame has completed.
 * \param ctxt the S2 context
 * \param status  status code of the transmission
 * \param tx_time the time used for this transmission i milliseconds.
 */
void S2_send_frame_done_notify(struct S2* ctxt, s2_tx_status_t status,uint16_t tx_time);

/**
* Check if the S2 FSM or inclusion FSM are busy.
*
* \param ctxt the S2 context
* \return False if the FSM and inclusion FSM are both idle
*/
uint8_t S2_is_busy(struct S2* ctxt);

/**
* Free an MPAN entry.
* Marks an MPAN entry matching the (owner_id, group_id) tupple as free.
* This should be called prior to re-using the group_id for a group where the new
* members are not a superset of the old members. If this is not done, the old
* members will react to one single message not intended for them before leaving
* the group permanently.
*
* \param ctxt         the S2 context
* \param owner_id     The Sendind NodeID for the Group ID
* \param group_id     The Group ID to erase.
*/
void S2_free_mpan(struct S2* p_context, node_t owner_id, uint8_t group_id);

#include "S2_external.h"
#include "s2_inclusion.h"

/**
* Initialize the S2 PRNG. Must be called on powerup.
* Do not call when starting S2 inclusion. Disables the radio briefly while running.
*/
void S2_init_prng(void);

/**
 * The random number generator used by S2
 */
extern CTR_DRBG_CTX s2_ctr_drbg;


/**
 * @}
 */
#endif /* INCLUDE_S2_H_ */
