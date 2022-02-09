/* © 2017 Silicon Laboratories Inc.
 */
/*
 * s2_event.h
 *
 *  Created on: Oct 16, 2015
 *      Author: trasmussen
 */

#ifndef _S2_EVENT_H_
#define _S2_EVENT_H_

/**
 * \defgroup s2_event libs2 inclusion events.
 *
 * @{
 */

#include "S2.h"
#include "platform.h"

/**
 * This enum contains codes for all type of S2 events.
 */
typedef enum
{
  S2_NODE_INCLUSION_INITIATED_EVENT,            ///< Security 2 event, a KEX Get has been received from including node, thus indicating that Security 2 inclusion is now in progress..
  S2_NODE_INCLUSION_KEX_REPORT_EVENT,           ///< Security 2 event, a node is in progress of being included to the network. This event contains the requested key set from the KEX report. This event MUST be followed by a call to \ref s2_inclusion_key_grant.
  S2_NODE_INCLUSION_PUBLIC_KEY_CHALLENGE_EVENT, ///< Security 2 event, a node is in progress of being included to the network. This event is a request to the system to verify the challenge and progress with inclusion. This event MUST be followed by a call to \ref s2_inclusion_challenge_response.
  S2_NODE_INCLUSION_COMPLETE_EVENT,             ///< Security 2 event, inclusion of the node  to the network has been completed.
  S2_NODE_JOINING_COMPLETE_EVENT,               ///< Security 2 event, the node has completed its inclusion.
  S2_NODE_INCLUSION_FAILED_EVENT,               ///< Security 2 event, inclusion of the node  to the network has failed. See the details
  S2_JOINING_COMPLETE_NEVER_STARTED_EVENT,      ///< Security2 event. Bootstrapping never started and the node is non-securely included. Emitted after timing out waiting for Kex Get from including controller.
} zwave_event_codes_t;

typedef struct{
  struct S2     * s2_handle;      ///< Handle reference to the S2 structure to which this event relates.
  uint8_t         security_keys;  ///< Security keys requested by the joining node.
  uint8_t         csa;            ///< Client side authentication was requested
}s2_node_inclusion_request_t;

typedef struct{
  struct S2     * s2_handle;     ///< Handle reference to the S2 structure to which this event relates.
  uint8_t         granted_keys;  ///< Security keys granted earlier in inclusion process.
  uint8_t         dsk_length;    ///< Number of missing bytes in the DSK which is requested by the protocol and must be returned with \ref s2_inclusion_challenge_response.
  uint16_t        length;        ///< Length of public key following below.
  uint8_t         public_key[1]; ///< Public key, length of key is describe in length field.
}s2_node_inclusion_challenge_t;

typedef struct{
  uint8_t         exchanged_keys; ///< Keys actually given/granted by/to node
} s2_node_inclusion_complete_t;

typedef struct{
  uint8_t        kex_fail_type;
} s2_node_inclusion_fail_t;

typedef struct
{
  s2_connection_t peer;
  union {
    s2_node_inclusion_request_t      kex_report;
    s2_node_inclusion_challenge_t    challenge_req;
    s2_node_inclusion_complete_t     inclusion_complete;
    s2_node_inclusion_fail_t         inclusion_fail;
  } s2_data;

//  uint16_t        length;
//  uint8_t         buffer[1]; // Variable byte array at end to allow for different packets. How to free the memory ?
}s2_event_t;


typedef struct __attribute__((__packed__))
{
  uint32_t event_type;
  // The below struct is inspired from SKB in Linux. Do we need it ?
  // Discussed with Anders. We should be concerned not to expose the linked list externally, but
  // maybe just keep it internally so that app cannot mess up/invalidate the list.
  // This also means the app must call an event_get every time we have sent a notification.
  /*
  struct
  {
    struct event_t * next;
    struct event_t * prev;
    uint32_t         timestamp;
  };
  */
  union
  {
    s2_event_t s2_event;
  }evt;
  // TODO: should we put the buffer here ?
  // Some events may not need a buffer, or at least a small one, in which case having the variable
  // buffer at end of struct in union allow dynamic growth.
  // Linux has a bit more flexibility for free form data, but in z-wave we might decide that we
  // know all supported form to ensure better name checking.
} zwave_event_t; // TODO: Should this be named zwave_event_t ?

/** @brief S2 event handler.
 * 
 *  @details The event handler is used for notifying upper layers about events from S2 securoity layers.
 *           Such events may be, but are not limited to, public key exchange during inclusions,
 *           transmission reports.
 *
 * @param[in] zwave_event_t  Z-Wave event identifying the event and associated data.
 */
typedef void (*s2_event_handler_t)(zwave_event_t *);

/**
 * @}
 */
#endif // _S2_EVENT_H_
