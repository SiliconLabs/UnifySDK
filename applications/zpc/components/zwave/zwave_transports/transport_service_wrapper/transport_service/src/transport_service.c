/******************************************************************************
* # License
* <b>Copyright 2021  Silicon Laboratories Inc. www.silabs.com</b>
******************************************************************************
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
*****************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "transport_service.h"
#include "zwave_tx.h"
#include "ctimer.h"
#include "transport_service_command_class_def.h"
#include "zwave_controller_crc16.h"
#include "sl_log.h"

#define LOG_TAG "transport_service"

#define PAYLOAD_SIZE_MAX    (200)
#define FIRST_HDR_LEN       4 /* Cmd class, cmd, size, seqno */
#define SUBSEQ_HDR_LEN      5 /* Cmd class, cmd, size, seqno + offset 1, offset 2*/
#define FRAGMENT_FC_TIMEOUT 1000 /*ms*/
#define FRAGMENT_RX_TIMEOUT 800  /*ms*/

#define RESET_TIME 5000 /* ms */
// TODO saeed: uncomment (re-defined)
// /opt/silabs/toolchain_arm_gnu/10.2_2020q4/lib/gcc/arm-none-eabi/10.2.1/include/stddef.h:406:
//                                        note: this is the location of the previous definition
//#define offsetof(s, m) (uint16_t)((uint8_t *)&(((s *)0)->m) - (uint8_t *)0)
static on_transport_service_send_data_complete_t on_send_complete_cb = 0;

/* See send_data() in zwave_transport_service_wrapper.c */
static send_data_t lower_layer_send_data = 0;

static upper_layer_command_handler_t upper_layer_command_handler;

static uint16_t my_node_id;
static uint16_t max_fragment_size = 0;
static uint16_t offset_to_request = 0;

static uint8_t flag_tie_broken = 0;
static uint8_t flag_initialize_once
  = 1;  // to initialize the receive session stuff once
static void reply_frag_req();
static uint8_t send_frag_wait_cmd();
static void send_subseq_frag(void *nthing);
void fc_timer_expired(void *nthing);
static void send_first_frag();
static uint8_t get_next_missing_offset();
static void rx_timer_expired(void *ss);
static void send_last_frag(void);

#define log_debug(f, ...) sl_log_debug(LOG_TAG, f, ##__VA_ARGS__)
#define log_error(f, ...) sl_log_debug(LOG_TAG, f, ##__VA_ARGS__)
void transport_service_init(ts_node_id_t node_id,
                            const upper_layer_command_handler_t command_handler,
                            const send_data_t send_data)
{
  lower_layer_send_data       = send_data;
  upper_layer_command_handler = command_handler;
  my_node_id                  = node_id;
}

uint8_t t2_txBuf[PAYLOAD_SIZE_MAX
                 + sizeof(ZW_COMMAND_SUBSEQUENT_FRAGMENT_1BYTE_FRAME) - 1];
ZW_COMMAND_FIRST_FRAGMENT_1BYTE_FRAME *first_frag
  = (ZW_COMMAND_FIRST_FRAGMENT_1BYTE_FRAME *)t2_txBuf;
ZW_COMMAND_SUBSEQUENT_FRAGMENT_1BYTE_FRAME *subseq_frag
  = (ZW_COMMAND_SUBSEQUENT_FRAGMENT_1BYTE_FRAME *)t2_txBuf;

#define FIRST_FRAG_NONPAYLOAD_LENGTH \
  (sizeof(ZW_COMMAND_FIRST_FRAGMENT_1BYTE_FRAME) - 1)
#define SUBSEQ_FRAG_NONPAYLOAD_LENGTH \
  (sizeof(ZW_COMMAND_SUBSEQUENT_FRAGMENT_1BYTE_FRAME) - 1)

typedef enum TRANSPORT2_STATES {
  ST_IDLE,

  /* Receive state machine states */
  ST_RECEIVING,
  ST_SEND_FRAG_COMPLETE,
  ST_SEND_FRAG_REQ,
  ST_SEND_FRAG_WAIT,
  ST_FIND_MISS_FRAG,

  /* Send state machine states */
  ST_SEND_FRAG,
  ST_SEND_LAST_FRAG,
  ST_WAIT_ACK,

} TRANSPORT2_ST_T;

typedef enum TRANSPORT2_EVENTS {
  /* Receive state machine events */
  EV_START_RECV,
  EV_RECV_NEW_FRAG,
  EV_NO_SESSION_ID,
  EV_SEND_FRAG_COMPLETE,
  EV_NO_SEND_FRAG_COMPLETE,
  EV_SUCCESS,
  EV_SUCCESS2,
  EV_SUCCESS3,
  EV_FRAG_RX_TIMER,
  EV_MISSING_FRAG,
  EV_MISSING_FRAG_BCAST,
  EV_SCAST_DIFF_NODE,
  EV_BCAST_DIFF_NODE,
  EV_DIFF_SESSION,
  EV_SUBSEQ_DIFF_SESSION,
  EV_FRAG_REQ_COMPL_WAIT_DIFF_SESSION,
  EV_FRAG_REQ_COMPL_WAIT_DIFF_NODE,
  EV_RECV_FRAG_REQ,
  EV_RECV_LAST_FRAG,
  EV_RECV_FRAG_COMPLETE,
  EV_TIE_BREAK,
  EV_DUPL_FRAME,
  EV_FRAG_REQ_OR_COMPL,

  /* Send state machine events */
  EV_START_SEND,
  EV_SEND_NEW_FRAG,
  EV_SEND_NXT_MISS_FRAG,
  EV_SENT_MISS_FRAG,
  EV_RECV_FRAG_WAIT,
  EV_SEND_LAST_FRAG,
  EV_SEND_LAST_MISS_FRAG,
  EV_FRAG_COMPL_TIMER,
  EV_FRAG_COMPL_TIMER2,
  EV_FRAG_COMPL_TIMER_REQ,
  EV_FAILURE_LAST_FRAG2,
  EV_RECV_FRAG_COMPL,
  EV_REPLY_FRAG_REQ_DONE,
  EV_FAIL,
} TRANSPORT2_EV_T;

typedef struct {
  TRANSPORT2_ST_T st; /* Current state */
  TRANSPORT2_EV_T ev; /* Incoming event */
  TRANSPORT2_ST_T next_st;
  char *fun_name; /* Action function returning next state */
} transition_t;

/* State transition table
 * This is where the state machine is defined.
 *
 * Each transition can optionally trigger a call to an action function.
 * An action function defines the actions that must occur when a particular
 * transition occurs.
 *
 * Format: {Current state, incoming event, next state, action_function} */
static const transition_t trans[] = {
  /* ========================*/
  /* Receiving State diagram */
  /* ========================*/

  /* First event when on receiving end. */
  {ST_IDLE, EV_START_RECV, ST_RECEIVING, "start receive"},
  {ST_IDLE, EV_START_SEND, ST_SEND_FRAG, "send_first_frag"},

  /* This happens when there is a fragment received from different session or node, which
            will be ignored. Then the state machine lands up in ST_IDLE */
  {ST_IDLE, EV_SEND_NEW_FRAG, ST_SEND_FRAG, "keep sending"},

  /* Send new frame. Call the same funciton i.e. */
  {ST_SEND_FRAG, EV_SEND_NEW_FRAG, ST_SEND_FRAG, "send_subseq_frag"},

  {ST_RECEIVING, EV_SEND_NEW_FRAG, ST_SEND_FRAG, "send"},
  {ST_WAIT_ACK, EV_SEND_NEW_FRAG, ST_SEND_FRAG, "send"},

  /* If the current state is ST_RECEIVING and new fragment arrives */
  /* If the fragment length is wrong */
  /* If the crc is bad */
  {ST_RECEIVING, EV_RECV_NEW_FRAG, ST_RECEIVING, "process new fram"},

  /* If duplicate frame arrives after frag completion move back to idle state */
  {ST_RECEIVING, EV_DUPL_FRAME, ST_IDLE, "discard the fragment"},

  /* If the fragment received is corrupt and there is no session ID. Receiving node
         * has to send Fragment wait command to sending node */
  {ST_RECEIVING, EV_NO_SESSION_ID, ST_SEND_FRAG_WAIT, "send_frag_wait_cmd"},

  /* If the fragment received is singlecast fragment while receiving another fragmented
         * datagram. Receiving node has to send Fragment wait command to sending node */
  {ST_RECEIVING, EV_SCAST_DIFF_NODE, ST_SEND_FRAG_WAIT, "send_frag_wait_cmd"},

  /* If the fragment received is singlecast fragment while sending another fragmented
         * datagram. */
  {ST_SEND_FRAG, EV_SCAST_DIFF_NODE, ST_SEND_FRAG_WAIT, "send_frag_wait_cmd"},

  /* If the fragment received is singlecast fragment while waiting for ack {frag req or frag compl}
        */
  {ST_WAIT_ACK, EV_SCAST_DIFF_NODE, ST_SEND_FRAG_WAIT, "send_frag_wait_cmd"},

  /* If the fragment received is singlecast fragment while receiving fragmented
         * datagram from different session. Receiving node has to discard the fragment */
  {ST_RECEIVING, EV_DIFF_SESSION, ST_IDLE, "discard the fragment"},

  {ST_RECEIVING, EV_SUBSEQ_DIFF_SESSION, ST_SEND_FRAG_WAIT, "send frag_wait"},

  /* Fragment request/complete/wait from different node id*/
  {ST_RECEIVING,
   EV_FRAG_REQ_COMPL_WAIT_DIFF_NODE,
   ST_IDLE,
   "discard the fragment"},

  /* Fragment request/complete/wait from different session */
  {ST_RECEIVING,
   EV_FRAG_REQ_COMPL_WAIT_DIFF_SESSION,
   ST_IDLE,
   "discard the fragment"},

  /* Go back to receiving state if Fragment wait command is sent successfully */
  {ST_SEND_FRAG_WAIT, EV_SUCCESS, ST_RECEIVING, NULL},
  /* Go back to sending state as the fragment wait was sent in sending session */
  {ST_SEND_FRAG_WAIT, EV_SUCCESS2, ST_SEND_FRAG, NULL},
  /* Go back to ST_WAIT_ACK as the fragment wait was sent in sending session */
  {ST_SEND_FRAG_WAIT, EV_SUCCESS3, ST_WAIT_ACK, NULL},

  /* What if sending fragment wait command fails ?*/

  /* If Fragment complete has been sent to sending node successfully, Go back to receiving
         * state */
  {ST_SEND_FRAG_COMPLETE, EV_SUCCESS, ST_IDLE, NULL},
  /*After sending one fragment request go back to receiving state, either we will receive
        another fragment request of fragment complete */
  {ST_SEND_FRAG_REQ, EV_SUCCESS, ST_RECEIVING, NULL},

  /* If RX timer event happens while ST_RECEIVING */
  {ST_RECEIVING, EV_FRAG_RX_TIMER, ST_FIND_MISS_FRAG, "find_missing"},
  /* If there is Fragment Rx timer event whiel sending Fragment Req command discard all the previous
         * Discard received fragmets in the datagram and go to Idle state */
  {ST_IDLE, EV_FRAG_RX_TIMER, ST_IDLE, "discard_all_received_fragments"},

  /* If the fragment received is broadcast fragment while receiving another fragmented datagram,
         * Receiving node needs to drop that fragment */
  {ST_RECEIVING, EV_BCAST_DIFF_NODE, ST_RECEIVING, "drop_fragment"},

  {ST_RECEIVING, EV_RECV_LAST_FRAG, ST_FIND_MISS_FRAG, "find_missing"},
  {ST_WAIT_ACK, EV_SEND_FRAG_COMPLETE, ST_SEND_FRAG_COMPLETE, NULL},
  {ST_RECEIVING, EV_RECV_FRAG_COMPL, ST_IDLE, NULL},

  /* Stay in the same state ST_WAIT_ACK until we know if its REQUEST OR COMPLETE cmd */
  {ST_WAIT_ACK, EV_FRAG_REQ_OR_COMPL, ST_WAIT_ACK, NULL},

  /* If the rx timer expires after sending Fragment Request, discard all fragments and go to IDLE state */
  {ST_WAIT_ACK, EV_FRAG_RX_TIMER, ST_IDLE, "discard all fragments"},

  /* Need to move to ST_RECEIVING state to process the FRAGMENT_REQUEST or FRAGMENT_COMPLETE received while sending */
  {ST_RECEIVING, EV_FRAG_REQ_OR_COMPL, ST_RECEIVING, "process received"},
  {ST_SEND_FRAG, EV_FRAG_REQ_OR_COMPL, ST_RECEIVING, "process received"},

  /* If there are missing frames, receiving node needs to send Fragment request command to
         * sending node */
  {ST_FIND_MISS_FRAG, EV_MISSING_FRAG, ST_SEND_FRAG_REQ, "send_frag_req_cmd"},
  /* If the broacast fragment is being received and rx timer times out of there are some missing fragments*/
  {ST_FIND_MISS_FRAG, EV_MISSING_FRAG_BCAST, ST_IDLE, NULL},
  /* If all the fragments of the datagram are receivied receiving node needs to send
         * Fragment complete command to sending node. */
  {ST_FIND_MISS_FRAG,
   EV_SEND_FRAG_COMPLETE,
   ST_SEND_FRAG_COMPLETE,
   "send_frag_complete_cmd"},

  /* NO need to send frag complete if it was bcast datagram which was being recived */
  {ST_FIND_MISS_FRAG, EV_NO_SEND_FRAG_COMPLETE, ST_IDLE, NULL},

  /* ======================*/
  /* Sending State diagram */
  /* ======================*/

  {ST_SEND_FRAG, EV_RECV_FRAG_WAIT, ST_SEND_FRAG, "wait_restart_from_first"},

  /* ST_SEND_FRAG ->{EV_TIE_BREAK} -> ST_RECEIVING ->{EV_RECV_NEW_FRAG} ->{ EV_RECV_FRAG_WAIT }*/
  {ST_RECEIVING, EV_RECV_FRAG_WAIT, ST_SEND_FRAG, "wait_restart_from_first"},

  {ST_SEND_FRAG, EV_SEND_LAST_FRAG, ST_SEND_LAST_FRAG, "send_last_frag"},
  {ST_SEND_FRAG, EV_SEND_LAST_MISS_FRAG, ST_SEND_LAST_FRAG, "send_last_frag"},

  {ST_SEND_LAST_FRAG, EV_SUCCESS, ST_WAIT_ACK, NULL},
  /* If the Fragment completion timer even happens send the last fragment again */
  {ST_WAIT_ACK, EV_FRAG_COMPL_TIMER, ST_SEND_LAST_FRAG, "send_last_frag"},

  /*  fc_timer_expired in reply_frag_req*/
  {ST_WAIT_ACK,
   EV_FRAG_COMPL_TIMER_REQ,
   ST_IDLE,
   "send failure to application"},

  /* If the Fragment completion timer event happens again send error to application */
  {ST_WAIT_ACK, EV_FRAG_COMPL_TIMER2, ST_IDLE, NULL},

  /* Failure in sending last fragment second time after one Fragment completion timer event */
  /*P.S. This is not failure in sending the last fragment at first time */
  {ST_WAIT_ACK, EV_FAILURE_LAST_FRAG2, ST_IDLE, NULL},

  /* If received Fragment request command */
  {ST_WAIT_ACK, EV_RECV_FRAG_REQ, ST_SEND_FRAG, "reply_frag_req"},
  {ST_RECEIVING, EV_RECV_FRAG_REQ, ST_SEND_FRAG, "reply_frag_req"},

  /* If received Fragment complete command, disable the fragment complete timer*/
  {ST_WAIT_ACK, EV_RECV_FRAG_COMPL, ST_IDLE, NULL},

  {ST_SEND_FRAG, EV_SEND_NXT_MISS_FRAG, ST_SEND_FRAG, "reply_frag_req"},
  {ST_SEND_FRAG, EV_FRAG_COMPL_TIMER, ST_IDLE, "wait"},

  /* This happens when the FC_complete timer happens before  reply_frag_req{} gets a callback
           state machine is still in ST_SEND_FRAG as it goes in ST_WAIT_ACK in the callback*/
  {ST_SEND_FRAG, EV_FRAG_COMPL_TIMER_REQ, ST_IDLE, "wait"},

  {ST_SEND_FRAG, EV_SENT_MISS_FRAG, ST_WAIT_ACK, NULL},
  {ST_SEND_FRAG, EV_TIE_BREAK, ST_RECEIVING, "receive"},
  {ST_SEND_LAST_FRAG, EV_TIE_BREAK, ST_RECEIVING, "receive"},
  {ST_WAIT_ACK, EV_RECV_FRAG_WAIT, ST_SEND_FRAG, NULL},

  /* If the ZW_SendData{} function itself fails we go back to IDLE state
           The other side will send fragment request command. But we need to be in IDLE
            state to atleast receive and process it */
  {ST_SEND_LAST_FRAG, EV_FAIL, ST_IDLE, NULL},
  {ST_WAIT_ACK, EV_TIE_BREAK, ST_RECEIVING, "receive"},
};

TRANSPORT2_ST_T current_state = ST_IDLE;

uint8_t find_transition(TRANSPORT2_ST_T cstate, TRANSPORT2_EV_T event)
{
  uint8_t num_trans;

  num_trans = sizeof(trans) / sizeof(trans[0]);
  while (num_trans--) {
    if ((cstate == trans[num_trans].st) && (event == trans[num_trans].ev))
      return num_trans + 1;
  }

  return 0;
}

void t2_sm_post_event(TRANSPORT2_EV_T ev)
{
  uint8_t i;

  i = find_transition(current_state, ev);
  if (i) {
    current_state = trans[i - 1].next_st;
  }
}

/* Structure desribing the inflight fragment. */
typedef struct cb {
  ts_node_id_t source;
  ts_node_id_t dest;
  uint8_t session_id;
  uint8_t pending_segments;
} control_block_t;

typedef struct sending_cntrl_blk {
  uint16_t
    datalen_to_send;  // this is set to FragmentMaxPayload or remaining data less than FragmentMaxPayload
  uint16_t
    missing_offset;  // this is set to missing offset received in FRAMENT_REQUEST command and used to resend that fragment
  uint16_t offset;   // this is used in sending side
  uint16_t
    remaining_data_len;  // this records the len of remaining data to be sent
  control_block_t
    cmn; /* Common fields, necessary for both sending and receiving */
  uint8_t datagram[PAYLOAD_SIZE_MAX];
  uint16_t datagram_len;
  uint8_t sending;
  uint8_t flag_replied_frag_req;
  uint8_t transmission_aborted; /*Initializing to out of range session id */

  struct ctimer reset_timer;

  /* When fragment wait is received with pending segments this timer is used to halt the restart of datagram sending
  by 100ms * no_of_pending_segments */
  struct ctimer wait_restart_timer;

  /*Timer used to keep delay between two fragments sent typically 15ms */
  struct ctimer timer_btwn_2_frags;

  /* Array to mark the receival of Fragment Completion command for each sending session */
  /* As there could be max 16 sessions (4 bits for the session id) */
  uint8_t frag_compl_list[15];

  /* If the session is sending one then this flag is set to TRUE to mark that next send is going to be
  fragment wait. If the session is receiving send_frag_wait_cmd() is called directly */
  uint8_t flag_send_frag_wait;
  uint8_t flag_reply_frag_req;
  ts_node_id_t frag_wait_source;
  ts_node_id_t frag_wait_dest;

  clock_time_t round_trip_first_frag;

  /* This flag is used to mark that there have been mutliple fc_timer expiration
   * for this fragment complete
   * So that transport service can report error to the application layer */
  uint8_t flag_fc_timer_expired_once;

  /* this stores the current destination node where the send session is going on. If a fragment
  request from different node is received this helps identifying it */
  uint8_t current_dnode;

  /* Used to determine if the send_last_frag() function is called from send_first_frag()
   * or ZCB_send_subseq_frag()/reply_frag_req */
  uint8_t does_not_fit_in_first_frag;

} sending_cntrl_blk_t;

struct rx_timer_expired_data {
  uint8_t state; /* Rx timer expired after sending SEG_REQ or after sending */
};

typedef struct receiving_cntrl_blk {
  control_block_t
    cmn; /* Common fields, necessary for both sending and receiving */
  const uint8_t *fragment;
  uint8_t fragment_len;
  /* Fragment Completion timer */
  struct ctimer fc_timer;
  /*RX timer */
  struct ctimer rx_timer;

  struct rx_timer_expired_data rx_data;

  /* Array to mark the receival of whole Fragment (sending of fragment complete) command for each receiving session */
  /* As there could be max 16 sessions (4 bits for the session id) */
  uint8_t recv_frag_compl_list[16];

  /* Buffer for incoming re-assembled datagrams */
  uint8_t datagramData[PAYLOAD_SIZE_MAX];

  /* To copy the data to destination buffer */
  uint8_t cur_recvd_data_size;

  /* this is set to the source node id on receiving FIRST_FRAG, and used in checking
  all subseq fragments and (first fragments if any before the transmissin of current
  datagram is done). This variable is set back to 0 on sending FRAG_COMPL or when the
  datagram is discarded */
  /*Used on receiving side */
  uint8_t current_snode;

  /* Mark each byte recived in a bit Then used to find missing
   * fragments/offsets if any */
  uint8_t bytes_recvd_bitmask[(PAYLOAD_SIZE_MAX / 8)
                              + 1];  //Bit for each byte received

  uint16_t datagram_size;
  receive_type rx_type;

} receiving_cntrl_blk_t;

sending_cntrl_blk_t scb;
receiving_cntrl_blk_t rcb;

static uint8_t recv_or_send()
{
  switch (current_state) {
    case ST_IDLE:
      return 2; /*Neither sending nor receiving */

    /* Receive state machine states */
    case ST_RECEIVING:
    case ST_SEND_FRAG_COMPLETE:
    case ST_SEND_FRAG_REQ:
    case ST_SEND_FRAG_WAIT:
    case ST_FIND_MISS_FRAG:
      log_debug("Sending 2: FALSE\n\n");
      return 1;

    /* Send state machine states */
    case ST_SEND_FRAG:
    case ST_SEND_LAST_FRAG:
    case ST_WAIT_ACK:
      log_debug("Sending 2: TRUE\n\n");
      return 0;
    default:
      log_debug("Unkonwn current_state: %d\n", current_state);
      break;
  }
  return 3;
}

uint8_t is_transport_service_receiving()
{
  return (recv_or_send() == 1) ? 1 : 0;
}

uint8_t is_transport_service_sending()
{
  return (recv_or_send() == 0) ? 1 : 0;
}

void transport_service_senddata_cb(uint8_t status, void *user)
{
  (void)user;
  /* : May be, this should be part of the specs
  Find out how long it took for the callback of FIRST_FRAG,
  wait that much before sending second FRAG. if the receiving
  node wants to send FRAG_WAIT, this will give the receiving node little
  time to breath - Anders Esbensen*/
  if (scb.round_trip_first_frag) {
    scb.round_trip_first_frag = clock_time() - scb.round_trip_first_frag;
    /* 300 below is added to ease the receiving side to send fragment wait if
     * it wants to */
    scb.round_trip_first_frag += 50;
    log_debug(
      "Adding delay of scb.round_trip_first_frag: %lu ms before sending "
      "second fragment\n",
      scb.round_trip_first_frag);
  }

  if (status != 0) {
    log_debug("Transmission status is not TRANSMIT_COMPLETE_OK\n\n");
  }

  if (scb.flag_reply_frag_req) {
    t2_sm_post_event(EV_SEND_NXT_MISS_FRAG);
    scb.flag_reply_frag_req = 0;
    reply_frag_req(NULL);
    return;
  }

  if (scb.flag_send_frag_wait) {
    scb.flag_send_frag_wait = 0;
    log_debug("Send Frag_wait now\n");
    send_frag_wait_cmd();
    return;
  }
  /*In order not to congest the Z-Wave network,
    large data transfers MUST leave transmit opportunities for other
    nodes in the network. If sending a command longer than two frames,
    a node MUST implement a delay between every transmitted frame. The
    mninimum required time delay and number of frames before delay MUST
    be inserted depends on the actual bit rate.

   40 kbit/s: At least 35 ms if sending more than 2 frames back-to-back
   100 kbit/s: At least 15 ms if sending more than 2 frames back-to-back
  */
  if (scb.transmission_aborted == scb.cmn.session_id) {
    log_debug("stopping tranmission for session: %d\n",
              scb.transmission_aborted);
    ctimer_stop(&scb.timer_btwn_2_frags);
  } else {
    log_debug("Calling send_subseq_frag\n");
    t2_sm_post_event(EV_SEND_NEW_FRAG); /*ZCB_send_subseq_frag*/
    log_debug("delay: %lu\n", 15 + scb.round_trip_first_frag);
    ctimer_set(&scb.timer_btwn_2_frags,
               15 + (scb.round_trip_first_frag),
               send_subseq_frag,
               0);
  }
  scb.round_trip_first_frag = 0;
}

static uint8_t discard_all_received_fragments(void)
{
  memset(rcb.datagramData, 0, sizeof(rcb.datagramData));

  memset((uint8_t *)&rcb.cmn, 0, sizeof(control_block_t));
  rcb.cmn.session_id = 0x10;
  rcb.current_snode  = 0;
  memset(rcb.bytes_recvd_bitmask, 0, sizeof(rcb.bytes_recvd_bitmask));
  return 0;
}

static void reset_transport_service(void *user)
{
  (void)user;
  log_debug("reset_timer expired going back to ST_IDLE state \n");
  ctimer_stop(&scb.reset_timer);
  current_state = ST_IDLE;
  discard_all_received_fragments();
  scb.sending = 0;
}

static uint8_t send_frag_wait_cmd()
{
  uint8_t ret = 0;
  ZW_COMMAND_SEGMENT_WAIT_V2_FRAME frag_wait;

  frag_wait.cmdClass     = COMMAND_CLASS_TRANSPORT_SERVICE_V2;
  frag_wait.cmd_reserved = (COMMAND_SEGMENT_WAIT_V2 & 0xf8);
  ctimer_set(&scb.reset_timer, RESET_TIME, reset_transport_service, 0);
  /* If there is a sending session going on FRAG_WAIT will be queed for next callback*/
  if (scb.sending) {
    log_debug("Sending fragment wait command. Pending segments: %d\n",
              scb.cmn.pending_segments);
    frag_wait.pendingFragments = scb.cmn.pending_segments;
    log_debug("Sending FRAG_WAIT from sending session snode: %d -> dnode: %d\n",
              scb.frag_wait_source,
              scb.frag_wait_dest);
    ret = lower_layer_send_data(scb.frag_wait_source,
                                scb.frag_wait_dest,
                                (uint8_t *)&frag_wait,
                                sizeof(frag_wait),
                                0,
                                transport_service_senddata_cb);
    t2_sm_post_event(EV_SUCCESS2); /* Go back to ST_SEND_FRAG state in */
  } else {
    if (rcb.cmn.session_id == 0x10) {
      rcb.cmn.pending_segments = 0;
    } else {
      // this is approximate. If we have variable frame size
      rcb.cmn.pending_segments = rcb.datagram_size / rcb.cur_recvd_data_size;
      (rcb.datagram_size % rcb.cur_recvd_data_size) ? rcb.cmn.pending_segments++
                                                    : 0;
      log_debug("datagram size: %d, cur recv size: %d\n\n",
                rcb.datagram_size,
                rcb.cur_recvd_data_size);
    }

    log_debug("Sending fragment wait command. Pending segments: %d\n",
              rcb.cmn.pending_segments);
    frag_wait.pendingFragments = rcb.cmn.pending_segments;
    log_debug(
      "Sending FRAG_WAIT from receiving session snode: %d -> dnode: %d\n",
      scb.frag_wait_source,
      scb.frag_wait_dest);
    ret = lower_layer_send_data(scb.frag_wait_source,
                                scb.frag_wait_dest,
                                (uint8_t *)&frag_wait,
                                sizeof(frag_wait),
                                0,
                                NULL);

    t2_sm_post_event(
      EV_SUCCESS); /* Go back to ST_RECEIVING state in receive() funciton */
  }

  /* What to do if sending frag wait fails*/
  if (ret == 1) {
    log_debug("send_data failed\n\n");
  }
  return 0;
}

static uint8_t mark_frag_received(const uint16_t offset, const uint8_t size)
{
  log_debug("Received offset: %d\n", (int)offset);

  if ((offset != 0) && !(rcb.bytes_recvd_bitmask[0] & 1)) {
    log_debug("Received subseq fragment without first fragment.\n");
    t2_sm_post_event(EV_SUBSEQ_DIFF_SESSION);
    if (scb.sending) {
      scb.flag_send_frag_wait = 1;
    } else {
      send_frag_wait_cmd();
    }
    return 1;
  }
  for (int i = offset; i < (offset + size); i++) {
    // set the (i%8)th bit in (i/8)th byte in bitmask, where i is the byte received

    // if 9th byte is received following formula becomes
    //                rcb.bytes_recvd_bitmask[1] |= ( 1 << 1)
    // if 11th byte is received following formula becomes rcb.bytes_recvd_bitmask[1] |= 4
    //                rcb.bytes_recvd_bitmask[1] |= ( 1 << 3)

    rcb.bytes_recvd_bitmask[i / 8] |= (1 << (i % 8));
    if (i > PAYLOAD_SIZE_MAX) {  // Prevent the array over run
      break;
    }
  }
  return 0;
}

static uint8_t send_frag_complete_cmd()
{
  uint8_t ret = 0;

  log_debug("Sending COMMAND_FRAGMENT_COMPLETE\n");
  ZW_COMMAND_SEGMENT_COMPLETE_V2_FRAME frag_compl;

  if (rcb.cmn.session_id > 0x0f) { /* Session ID has only 4 bits for it.*/
    log_debug("Session id is more than 15\n");
    return 0;
  }

  frag_compl.cmdClass     = COMMAND_CLASS_TRANSPORT_SERVICE_V2;
  frag_compl.cmd_reserved = (COMMAND_SEGMENT_COMPLETE_V2 & 0xf8);

  frag_compl.properties2 = ((rcb.cmn.session_id & 0x0f) << 4);
  ctimer_set(&scb.reset_timer, RESET_TIME, reset_transport_service, 0);
  ret = lower_layer_send_data(rcb.cmn.dest,
                              rcb.cmn.source,
                              (uint8_t *)&frag_compl,
                              sizeof(frag_compl),
                              0,
                              NULL);
  if (ret == 1) {
    log_debug(
      "send_data failed\n"); /* What to do of sending Frag Compl fails */
  }

  upper_layer_command_handler(rcb.cmn.source,
                              rcb.cmn.dest,
                              rcb.datagramData,
                              rcb.datagram_size); /**/

  /* Resetting for next session */
  rcb.recv_frag_compl_list[rcb.cmn.session_id] = 1;
  rcb.cmn.session_id                           = 0x10;
  rcb.current_snode                            = 0;
  ctimer_stop(&rcb.rx_timer);
  t2_sm_post_event(EV_SUCCESS); /* just change the state to ST_RECEIVING */
  return 0;
}

static uint8_t send_frag_req_cmd()
{
  ZW_COMMAND_SEGMENT_REQUEST_V2_FRAME *frag_req
    = (ZW_COMMAND_SEGMENT_REQUEST_V2_FRAME *)t2_txBuf;
  uint8_t ret = 0;

  offset_to_request = get_next_missing_offset();
  if (!offset_to_request) {
    log_debug("No offset_to_request is missing\n");
    t2_sm_post_event(EV_SUCCESS); /* Just change the state to ST_RECEIVING */
    return 0;
  }

  if (rcb.cmn.session_id > 0x0f) { /* Session ID has only 4 bits for it.*/
    log_debug("Session id is more than %d\n", 0x0f);
    return 0;
  }

  frag_req->cmdClass     = COMMAND_CLASS_TRANSPORT_SERVICE_V2;
  frag_req->cmd_reserved = (COMMAND_SEGMENT_REQUEST_V2)&0xf8;
  frag_req->properties2  = (rcb.cmn.session_id & 0x0f) << 4;

  /* Get 9th, 10th and 11th MSB */
  frag_req->properties2 |= ((offset_to_request & 0x700) >> 8);
  frag_req->datagramOffset2 = (offset_to_request & 0xff);

  /*Got to wait here some time or wait for ACK */
  /* after sending frag req, as we need to discard fragments in
   * rx_timer_expired */
  rcb.rx_data.state = 1;

  for (int i = 0; i < 2; i++) {
    log_debug("Sending fragment request command for offset: %d in "
              "session id: %d\n",
              offset_to_request,
              rcb.cmn.session_id);

    ret = lower_layer_send_data(rcb.cmn.dest,
                                rcb.cmn.source,
                                t2_txBuf,
                                sizeof(*frag_req),
                                1,  //expect one frame in response
                                NULL);

    if (ret == 0) {
      break;
    } else {
      /* what to do if frag req cmd fails */
      log_debug("send_data failed \n");
      log_debug("Retrying sending fragment request\n");
      if (i)
        rx_timer_expired(&rcb.rx_data);
    }
  }

  /* should this be in the call back? Just change the state to ST_RECEIVING */
  t2_sm_post_event(EV_SUCCESS);
  log_debug("rx_timer\n");
  ctimer_set(&rcb.rx_timer,
             FRAGMENT_RX_TIMEOUT,
             rx_timer_expired,
             &rcb.rx_data);
  return 0;
}

static void find_missing()
{
  uint8_t missing_frag = get_next_missing_offset();

  if (missing_frag) {
    if (rcb.rx_type == BROADCAST) {
      log_debug("There are missing fragments, but in broadcast datagram. Not "
                "sending fragment request command to sender\n");
      discard_all_received_fragments();
      return;
    }
    t2_sm_post_event(EV_MISSING_FRAG); /* send_frag_req_cmd */
    send_frag_req_cmd();
  } else {
    /* No need to send Fragment complete in case of Broadcast */
    if (rcb.rx_type == BROADCAST) {
      log_debug("Fragment transfer has compoleted, but in broadcast datagram. "
                "Not sending fragment complete command to sender\n");
      return;
    }
    t2_sm_post_event(EV_SEND_FRAG_COMPLETE);
    send_frag_complete_cmd();
  }
}

static void add_crc(uint8_t *buf, uint16_t len)
{
  uint8_t *tmp_buf = buf;
  uint16_t crc     = zwave_controller_crc16(0x1D0F, tmp_buf, len);
  tmp_buf += len;
  *tmp_buf++ = (crc >> 8) & 0xff;
  *tmp_buf   = (crc)&0xff;
}

static void temp_callback_last_frag(uint8_t status, void *user)
{
  (void)user;
  if (status != 0) {
    log_debug(
      "Transmission status is not TRANSMIT_COMPLETE_OK for last_frag\n");
    if (!scb.flag_fc_timer_expired_once) {
      scb.flag_fc_timer_expired_once++;
      send_last_frag();
    } else {
      if (on_send_complete_cb) {
        on_send_complete_cb(1, 0);
        on_send_complete_cb = 0;
      }
      t2_sm_post_event(EV_FAILURE_LAST_FRAG2);
      log_debug("reset_timer\n");
      ctimer_set(&scb.reset_timer, RESET_TIME, reset_transport_service, 0);
    }
  } else {
    log_debug("last fragment sent. Starting fc_timer\n");
    ctimer_set(&rcb.fc_timer, FRAGMENT_FC_TIMEOUT, fc_timer_expired, 0);
    t2_sm_post_event(EV_SUCCESS); /* Go to ST_WAIT_ACK state */
  }
}

static void send_last_frag(void)
{
  uint8_t ret = 0;
  log_debug("send_last_frag\n");
  ctimer_stop(&rcb.fc_timer);
  scb.sending = 0;
  /* this is last fragment being sent, so pending_segments are 0 now */
  scb.cmn.pending_segments = 0;
  if (scb.does_not_fit_in_first_frag) {
    ret = lower_layer_send_data(scb.cmn.source,
                                scb.cmn.dest,
                                t2_txBuf,
                                sizeof(*subseq_frag) + scb.datalen_to_send - 1,
                                1,  //expect one frame in response
                                temp_callback_last_frag);

  } else {
    ret = lower_layer_send_data(scb.cmn.source,
                                scb.cmn.dest,
                                t2_txBuf,
                                sizeof(*first_frag) + scb.datalen_to_send - 1,
                                1,  //expect one frame in response
                                temp_callback_last_frag);
  }

  if (ret == 1) {
    log_debug("lower_layer_send_data failed\n");
    if (scb.flag_fc_timer_expired_once) {
      if (on_send_complete_cb) {
        log_debug("lower_layer_send_data failed twice. Aborting the send\n");
        on_send_complete_cb(1, 0);
        on_send_complete_cb = 0;
      }
      t2_sm_post_event(EV_FAILURE_LAST_FRAG2);
      ctimer_set(&scb.reset_timer, RESET_TIME, reset_transport_service, 0);
      return;
    } else {
      scb.flag_fc_timer_expired_once++;
    }
  }
  return;
}

static void send_subseq_frag(void *nthing)
{
  (void)nthing;
  uint8_t ret = 0;

  log_debug("send_subseq_frag\n");
  ctimer_stop(&rcb.fc_timer);
  if (scb.remaining_data_len == 0)
    scb.remaining_data_len = scb.datagram_len;

  if (scb.remaining_data_len >= max_fragment_size) {
    scb.datalen_to_send = max_fragment_size;
  } else {
    scb.datalen_to_send = scb.remaining_data_len;
  }

  if (scb.frag_compl_list[scb.cmn.session_id] == 1) {
    log_debug("Already received frag complete command for this session. "
              "Aborting any more fragment sending\n");
    return;
  }
  scb.offset = scb.datagram_len - scb.remaining_data_len;
  log_debug("Sending Subsequent Fragment scb.offset: %d\n", scb.offset);
  subseq_frag->cmdClass = COMMAND_CLASS_TRANSPORT_SERVICE_V2;
  subseq_frag->cmd_datagramSize1
    = (COMMAND_SUBSEQUENT_FRAGMENT) | ((scb.datagram_len >> 8) & 0x07);
  subseq_frag->datagramSize2 = scb.datagram_len & 0xff;
#if 0  //t

    if (frag_wait_once) {
        subseq_frag->properties2  = ((scb.cmn.session_id+1) << 3) | ((scb.offset>>8)&0x07);
            frag_wait_once = 0;
    } else
#endif
  /* properties2 4 MSBs are session id 4th LSB is reserved and 3 LSBs are 3 MSBs of scb.offset */
  subseq_frag->properties2
    = ((scb.cmn.session_id & 0x0F) << 4) | ((scb.offset >> 8) & 0x07);
  /* datagramOffset2 is 8 LSBs of scb.offset */
  subseq_frag->datagramOffset2 = scb.offset & 0xff;

  memcpy(&subseq_frag->payload1,
         (scb.datagram + scb.offset),
         scb.datalen_to_send);

  /*5 is size of ZW_COMMAND_SUBSEQUENT_FRAGMENT_1BYTE_FRAME till payload field */
  add_crc(t2_txBuf, scb.datalen_to_send + 5);

  if (scb.remaining_data_len <= max_fragment_size) {
    scb.does_not_fit_in_first_frag = 1;
    t2_sm_post_event(EV_SEND_LAST_FRAG); /* send_last_frag() */
    send_last_frag();
    return;
  }
  ctimer_set(&scb.reset_timer, RESET_TIME, reset_transport_service, 0);
  scb.remaining_data_len -= scb.datalen_to_send;
  log_debug("remaining_data_len: %d\n", scb.remaining_data_len);
  log_debug("data_len: %d\n", sizeof(*subseq_frag) + scb.datalen_to_send - 1);

  ret = lower_layer_send_data(scb.cmn.source,
                              scb.cmn.dest,
                              t2_txBuf,
                              sizeof(*subseq_frag) + scb.datalen_to_send - 1,
                              0,
                              transport_service_senddata_cb);

  if (ret == 1) {
    log_debug("lower_layer_send_data failed\n\n");
  } else {
    if (scb.remaining_data_len >= max_fragment_size) {
      scb.remaining_data_len = scb.remaining_data_len - max_fragment_size;
    }

    scb.cmn.pending_segments = scb.remaining_data_len / scb.datalen_to_send;
    if (scb.remaining_data_len % scb.datalen_to_send) {
      scb.cmn.pending_segments++;
    }
  }
  t2_sm_post_event(EV_SEND_NEW_FRAG); /*ZCB_send_subseq_frag*/
}

void temp_callback_reply_frag_req(unsigned char status, void *user)
{
  (void)user;
  //    memcpy((uint8_t*)&scb.cmn.tx_status, ts, sizeof(TX_STATUS_TYPE));
  if ((status != 0) && (on_send_complete_cb)) {
    on_send_complete_cb(status, 0);
    on_send_complete_cb = 0;
  }
  t2_sm_post_event(EV_SENT_MISS_FRAG);
}

void fc_timer_expired(void *nthing)
{
  (void)nthing;
  if (scb.flag_replied_frag_req) {
    scb.transmission_aborted  = scb.cmn.session_id;
    scb.flag_replied_frag_req = 0;
    scb.current_dnode         = 0;
    log_debug("FC timer expired after reply_frag_req()\n");
    log_debug("Sending failure to application\n");
    t2_sm_post_event(EV_FRAG_COMPL_TIMER_REQ);
    if (on_send_complete_cb) {
      on_send_complete_cb(1, 0);
      on_send_complete_cb = 0;
    }
    ctimer_set(&scb.reset_timer, RESET_TIME, reset_transport_service, 0);
    return;
  }

  /* This happens when in Tie break we abort a transmission and then FC completion timer
      event happens. Need to ignore it to make sure we dont send the last fragment again
      and make the state machine end up in weird state */
  /* Tested in test_fc_timer_after_frag_compl_of_aborted_transmission() */
  if (scb.transmission_aborted == scb.cmn.session_id) {
    log_debug(
      "FC timer expired for aborted transmission. Ignoring the timer event\n");
    log_debug("Sending failure to application\n");
    if (on_send_complete_cb) {
      on_send_complete_cb(1, 0);
      on_send_complete_cb = 0;
    }
    return;
  }

  /* Tested in test_fc_timer_after_last_frag_twice() */
  if (scb.flag_fc_timer_expired_once) {
    log_debug("Frag completion timer event happened twice \n\n");
    log_debug("Sending failure to application\n");
    scb.flag_fc_timer_expired_once = 0;
    scb.current_dnode              = 0;
    if (on_send_complete_cb) {
      on_send_complete_cb(1, 0);
      on_send_complete_cb = 0;
    }
    t2_sm_post_event(EV_FRAG_COMPL_TIMER2);
    ctimer_stop(&rcb.fc_timer);
    return;
  }
  log_debug("fc_timer_expired once. Sending last fragment again\n");
  t2_sm_post_event(EV_FRAG_COMPL_TIMER); /*send_last_frag() */
  scb.flag_fc_timer_expired_once++;
  send_last_frag();
}

/*this has to be aligned in sending session similarly to send_frag_wait_cmd() */
static void reply_frag_req()
{
  uint8_t ret;

  if (scb.frag_compl_list[scb.cmn.session_id] == 1) {
    log_debug("Already received frag complete command for this session."
              "Aborting any more fragment sending\n");
    return;
  }
  scb.datalen_to_send = max_fragment_size;
  log_debug("Resending offset: %d\n", scb.missing_offset);

  subseq_frag->cmdClass = COMMAND_CLASS_TRANSPORT_SERVICE_V2;

  if ((scb.missing_offset + scb.datalen_to_send)
      > scb.datagram_len) { /*last fragment */
    scb.datalen_to_send = scb.datagram_len - scb.missing_offset;
  }

  subseq_frag->cmd_datagramSize1
    = (COMMAND_SUBSEQUENT_FRAGMENT) | ((scb.datagram_len >> 8) & 0x07);
  subseq_frag->datagramSize2 = scb.datagram_len & 0xff;
  subseq_frag->properties2
    = ((scb.cmn.session_id & 0x0F) << 4) | ((scb.missing_offset >> 8) & 0x07);
  subseq_frag->datagramOffset2 = scb.missing_offset & 0xff;
  memcpy(&subseq_frag->payload1,
         (scb.datagram + scb.missing_offset),
         scb.datalen_to_send);

  /* 5 is size of ZW_COMMAND_SUBSEQUENT_FRAGMENT_1BYTE_FRAME till payload
   * field */
  add_crc(t2_txBuf, scb.datalen_to_send + 5);
  if ((scb.missing_offset + scb.datalen_to_send)
      == scb.datagram_len) { /*last fragment */
    log_debug("Resending last fragmnet\n");
    scb.does_not_fit_in_first_frag = 1;
    t2_sm_post_event(EV_SEND_LAST_MISS_FRAG); /* send_last_frag() */
    scb.flag_replied_frag_req = 1;
    send_last_frag();
    return;
  }

  if (scb.sending) {
    ret = lower_layer_send_data(scb.cmn.source,
                                scb.cmn.dest,
                                t2_txBuf,
                                sizeof(*subseq_frag) + scb.datalen_to_send - 1,
                                0,
                                transport_service_senddata_cb);
  } else {
    ret = lower_layer_send_data(scb.cmn.source,
                                scb.cmn.dest,
                                t2_txBuf,
                                sizeof(*subseq_frag) + scb.datalen_to_send - 1,
                                0,
                                temp_callback_reply_frag_req);
  }
  if (ret == 1) {
    log_debug("send_data failed\n");
  }
  scb.flag_replied_frag_req = 1;
  ctimer_set(&rcb.fc_timer, (FRAGMENT_FC_TIMEOUT + 500), fc_timer_expired, 0);
}

static uint8_t get_next_missing_offset()
{
  uint8_t missing_offset = 0;

  for (uint8_t i = 0; i < (sizeof(rcb.bytes_recvd_bitmask)); i++) {
    for (uint8_t j = 0; j < 8; j++) {
      if ((rcb.bytes_recvd_bitmask[i] & (1 << j)) == 0) {
        missing_offset = ((i * 8) + j);
        if (missing_offset == 0) {
          continue;
        }
        log_debug("missing_offset: %d\n", missing_offset);
        if (missing_offset >= rcb.datagram_size) {
          return 0;
        }
        return missing_offset;
      }
    }
  }
  return 0;
}

static void rx_timer_expired(void *ss)
{
  log_debug("rx_timer_expired\n");
  const struct rx_timer_expired_data *rdata = ss;
  uint8_t state                             = 0;
  if (rdata) {
    state = rdata->state;
  }

  ctimer_stop(&rcb.rx_timer);

#if 0 /* Following code is just for information purpose */
    /* If the rx_timer event happened while sending frag request */
    if (current_state == ST_SEND_FRA_REQ) {
            t2_sm_post_event(EV_FRAG_RX_TIMER); /* discard_all_received_fragments() */
    } else {
            t2_sm_post_event(EV_FRAG_RX_TIMER); /* send_frag_req_cmd */
    }
#endif
  /* There could be two functions called after this depending on current
     * state. See code above */
  t2_sm_post_event(EV_FRAG_RX_TIMER);
  if (state && (get_next_missing_offset(/*rcb.datagram_size*/))) {
    log_debug("rx timer expired after sending Fragment Request\n");
    log_debug("Discarding all fragments\n");
    discard_all_received_fragments();
  } else {
    find_missing();
  }
}

static void wait_restart_from_first(void *nthing)
{
  (void)nthing;
  send_first_frag();
  return;
}

static bool receive(void)
{
  const uint8_t byte1 = rcb.fragment[1];
  const uint8_t byte2 = rcb.fragment[2];
  const uint8_t byte3 = rcb.fragment[3];
  const uint8_t byte4 = rcb.fragment[4];

  uint16_t datagram_offset
    = 0; /*It has to fit 11 bits so need to be two uint8_t */
  uint8_t recvd_session_id = 0;
  uint16_t datagram_size_tmp;

  if (rcb.fragment[0] != COMMAND_CLASS_TRANSPORT_SERVICE_V2) {
    log_debug("Command class is not COMMAND_CLASS_TRANSPORT_SERVICE\n");
    return false;
  }

  switch (byte1 & 0xf8) {
    case COMMAND_FIRST_FRAGMENT:
      log_debug("Received First Fragment\n");
      if (flag_tie_broken) {
        scb.transmission_aborted = scb.cmn.session_id;
      }

      if (rcb.fragment_len <= FIRST_FRAG_NONPAYLOAD_LENGTH) {
        log_debug("Length of received fragment is less than %zu\n",
                  FIRST_FRAG_NONPAYLOAD_LENGTH);
        t2_sm_post_event(EV_RECV_NEW_FRAG);
        return false;
      }

      /* If first fragment received is corrupt send fragment wait command */
      if (zwave_controller_crc16(0x1D0F, rcb.fragment, rcb.fragment_len) != 0) {
        log_debug("CRC error. Discarding fragment\n");
        t2_sm_post_event(EV_RECV_NEW_FRAG);
        return false;
      }

      recvd_session_id = (byte3 & 0xf0) >> 4;
      log_debug("recvd_sesion_id is %d\n", recvd_session_id);
      if ((recvd_session_id != rcb.cmn.session_id)
          && (rcb.cmn.session_id != 0x10)) { /*Refer 10.1.3.1.5 */
        log_debug("Current session is %d but received session id is %d. "
                  "Ignoring the fragment\n",
                  rcb.cmn.session_id,
                  recvd_session_id);
        t2_sm_post_event(EV_DIFF_SESSION);
        return false;
      }
      datagram_size_tmp = ((byte1)&0x07) << 8;
      datagram_size_tmp += byte2;
      log_debug("Datagram size: %d\n", datagram_size_tmp);

      if (datagram_size_tmp > PAYLOAD_SIZE_MAX) {
        log_debug("datagram size is more than PAYLOAD_SIZE_MAX. "
                  "Ignoring the fragment\n\n");
        t2_sm_post_event(EV_DIFF_SESSION);
        return false;
      }
      // Tell the Tx Queue to take it easy, we have incoming frames.
      zwave_tx_set_expected_frames(rcb.cmn.source, 1);
      ctimer_set(&rcb.rx_timer,
                 FRAGMENT_RX_TIMEOUT,
                 rx_timer_expired,
                 &rcb.rx_data);
      rcb.cmn.session_id = recvd_session_id;
      if (recvd_session_id < sizeof(rcb.recv_frag_compl_list)) {
        rcb.recv_frag_compl_list[recvd_session_id]
          = 0; /* Setting this session id as "havent received FRAG_COMPLETE for it"*/
      } else {
        log_debug("Session id must have been less than %d but its: %d\n",
                  sizeof(rcb.recv_frag_compl_list),
                  recvd_session_id);
        return false;
      }

      rcb.cur_recvd_data_size = rcb.fragment_len - FIRST_FRAG_NONPAYLOAD_LENGTH;

      rcb.datagram_size = datagram_size_tmp;
      memset(rcb.bytes_recvd_bitmask, 0, sizeof(rcb.bytes_recvd_bitmask));

      rcb.current_snode = rcb.cmn.source;

      memcpy(rcb.datagramData,
             rcb.fragment + FIRST_HDR_LEN,
             rcb.cur_recvd_data_size);
      if (mark_frag_received(0, rcb.cur_recvd_data_size))
        return 0;

      /* The current fragment had all the data needed for the datagram */
      if (rcb.cur_recvd_data_size == rcb.datagram_size) {
        t2_sm_post_event(EV_SEND_FRAG_COMPLETE); /* csend_frag_complete_cmd */
        send_frag_complete_cmd();
        return 0;
      }

      if (rcb.cur_recvd_data_size > rcb.datagram_size) {
        log_debug("Something went wrong. Current fragment has more data than "
                  "needed in this datagram\n");
      }
      rcb.rx_data.state = 0; /*not after sending req cmd */
      break;

    case COMMAND_SUBSEQUENT_FRAGMENT:
      /* Stay in the same function and handle fragment */
      log_debug("Received Subsequent Fragment\n");
      if (flag_tie_broken) {
        scb.transmission_aborted = scb.cmn.session_id;
      }

      if (rcb.fragment_len <= SUBSEQ_FRAG_NONPAYLOAD_LENGTH) {
        log_debug(
          "Length of received subseq fragment is less than %i. Ignoring "
          "the fragment\n",
          (int)SUBSEQ_FRAG_NONPAYLOAD_LENGTH);
        t2_sm_post_event(EV_RECV_NEW_FRAG);
        return false;
      }
      /* If subseq fragment received is corrupt just ignore it */
      if (zwave_controller_crc16(0x1D0F, rcb.fragment, rcb.fragment_len) != 0) {
        log_debug("CRC error. Ignoring\n");
        t2_sm_post_event(EV_RECV_NEW_FRAG);
        return false;
      }

      recvd_session_id = (byte3 & 0xf0) >> 4;

      if (rcb.recv_frag_compl_list[recvd_session_id] == 1) {
        log_debug(
          "Already received Fragment Complete command for this session: "
          "%d. Looks like duplicate frame\n",
          recvd_session_id);
        if (current_state == ST_RECEIVING) {
          t2_sm_post_event(EV_DUPL_FRAME);
        } else {
          log_debug("Strange current state: %d(%d)\n",
                    current_state,
                    current_state);
        }
        return false;
      }
      /* session ID of new received fragment is different from the one being
     * assembled */
      if ((recvd_session_id != rcb.cmn.session_id)
          && (rcb.cmn.session_id != 0x10)) {
        log_debug("Current session is %d but recived session id is %d. "
                  "Ignoring fragment\n",
                  rcb.cmn.session_id,
                  recvd_session_id);
        t2_sm_post_event(EV_DIFF_SESSION);
        return false;
      }

      datagram_offset = ((byte3 & 0x07) << 8); /* last three bits of 2nd byte */
      datagram_offset += byte4;

      // Also checks if first fragment for this datagram is missing and we
      // received subsequent fragment.
      // Sends FRAG_WAIT as well

      log_debug("offset: %d\n", datagram_offset);
      rcb.cur_recvd_data_size
        = rcb.fragment_len - SUBSEQ_FRAG_NONPAYLOAD_LENGTH;

      if ((datagram_offset > sizeof(rcb.datagramData))
          || (rcb.cur_recvd_data_size > sizeof(rcb.datagramData))
          || ((datagram_offset + rcb.cur_recvd_data_size)
              > sizeof(rcb.datagramData))) {
        log_debug("Offset of fragment received is more than PAYLOAD_SIZE_MAX. "
                  "Ignoring fragment\n");
        if (current_state == ST_RECEIVING) {
          t2_sm_post_event(EV_DUPL_FRAME);
        }
        return false;
      }
      datagram_size_tmp = (byte1)&0x07;
      datagram_size_tmp = (datagram_size_tmp << 8) + byte2;

      if (datagram_size_tmp > PAYLOAD_SIZE_MAX) {
        log_debug("datagram size is more than PAYLOAD_SIZE_MAX. Ignoring the "
                  "fragment\n\n");
        t2_sm_post_event(EV_DIFF_SESSION);
        return false;
      }
      ctimer_set(&rcb.rx_timer,
                 FRAGMENT_RX_TIMEOUT,
                 rx_timer_expired,
                 &rcb.rx_data);
      if (mark_frag_received(datagram_offset, rcb.cur_recvd_data_size))
        break;

      log_debug("Pending Segments: %d\n", rcb.cmn.pending_segments);
      rcb.datagram_size = datagram_size_tmp;
      if (rcb.cmn.pending_segments > 0) {
        zwave_tx_set_expected_frames(rcb.cmn.source, 1);
      } else {
        zwave_tx_set_expected_frames(rcb.cmn.source, 0);
      }

      if (datagram_offset < sizeof(rcb.datagramData)
          && ((datagram_offset + rcb.cur_recvd_data_size)
              < sizeof(rcb.datagramData))) {
        memcpy(rcb.datagramData + datagram_offset,
               rcb.fragment + SUBSEQ_HDR_LEN,
               rcb.cur_recvd_data_size);
      } else {
        log_debug("Offset and size of of fragment received is more than"
                  "PAYLOAD_SIZE_MAX: %d. Ignoring fragment\n",
                  PAYLOAD_SIZE_MAX);
        return false;
      }

      if ((datagram_offset + rcb.cur_recvd_data_size)
          >= rcb.datagram_size) {            /*last fragment? */
        t2_sm_post_event(EV_RECV_LAST_FRAG); /*find_missing()*/
        find_missing();
        return 0;
      }
      rcb.rx_data.state = 0; /*not after sending req cmd */
      break;

    case COMMAND_SEGMENT_REQUEST_V2:
      t2_sm_post_event(EV_FRAG_REQ_OR_COMPL);
      log_debug("Received Fragment Request Command\n");
      //log_debug("byte2: %x\n", byte2)
      recvd_session_id = (byte2 & 0xf0) >> 4;
      /*Fragment request is not from the same session in which we were sending */

      if (recvd_session_id == scb.transmission_aborted) {
        log_debug("COMMAND_FRAGMENT_REQUEST: for aborted transmionss "
                  "session:%d. Igoring... \n",
                  recvd_session_id);
        t2_sm_post_event(EV_FRAG_REQ_COMPL_WAIT_DIFF_SESSION);
        return false;
      }
      if (recvd_session_id != scb.cmn.session_id) {
        log_debug("Current session is %d but recived session id is %d. "
                  "Ignoring...\n",
                  scb.cmn.session_id,
                  recvd_session_id);
        t2_sm_post_event(EV_FRAG_REQ_COMPL_WAIT_DIFF_SESSION);
        return false;
      }
      if ((rcb.cmn.source != scb.current_dnode) && (rcb.current_snode != 0)) {
        /* Check if the FRAG REQ is from the destination node where we were
       * sending data to */
        log_debug("Session id of Fragment request received is not same as "
                  "session_id of fragment being sent, "
                  "recvd_session_id: %d, scb.cmn.session_id: %d. "
                  "Ignoring the Frag request command\n",
                  recvd_session_id,
                  scb.cmn.session_id);
        t2_sm_post_event(EV_FRAG_REQ_COMPL_WAIT_DIFF_NODE);
        return false;
      }

      ctimer_stop(&rcb.fc_timer);
      scb.missing_offset = ((byte2 & 0x7) << 8);
      scb.missing_offset |= byte3;

      t2_sm_post_event(EV_RECV_FRAG_REQ);
      if (scb.sending) {
        scb.flag_reply_frag_req = 1;
      } else {
        reply_frag_req();
      }
      break;

    case COMMAND_SEGMENT_COMPLETE_V2:
      t2_sm_post_event(EV_FRAG_REQ_OR_COMPL);
      log_debug("Received Fragment Complete Command\n");
      recvd_session_id = (byte2 & 0xf0) >> 4;
      if (recvd_session_id == scb.transmission_aborted) {
        log_debug("COMMAND_FRAGMENT_COMPLETE: for aborted transmionss "
                  "session:%d. Igoring... \n",
                  recvd_session_id);
        t2_sm_post_event(EV_FRAG_REQ_COMPL_WAIT_DIFF_SESSION);
        return false;
      }
      /*Fragment complete is not from the same session in which we were sending */
      if (recvd_session_id != scb.cmn.session_id) {
        log_debug("Current session is %d but recived session id is %d\n",
                  recvd_session_id,
                  rcb.cmn.session_id);
        t2_sm_post_event(EV_FRAG_REQ_COMPL_WAIT_DIFF_SESSION);
        return false;
      }
      if ((rcb.cmn.source != scb.current_dnode) && (rcb.current_snode != 0)) {
        /* Check if the FRAG complete is from the destination node where we
       * were sending data to */
        log_debug("Session id of Fragment request received is not same as "
                  "session_id of fragment being sent, recvd_session_id: %d, "
                  "scb.cmn.session_id: %d. Ignoring the Frag request command\n",
                  recvd_session_id,
                  scb.cmn.session_id);
        t2_sm_post_event(EV_FRAG_REQ_COMPL_WAIT_DIFF_NODE);
        return false;
      }
      ctimer_stop(&rcb.fc_timer);
      log_debug("recvd_session_id : %d\n\n", recvd_session_id);
      if (scb.cmn.session_id == recvd_session_id) {
        scb.frag_compl_list[recvd_session_id] = 1;
        scb.current_dnode                     = 0;
        if (on_send_complete_cb) {
          log_debug("Sending back TRANSMIT_COMPLETE_OK to client\n");
          on_send_complete_cb(0, 0);
          on_send_complete_cb = 0;
        }
      } else {
        log_debug("Fragment complete session id is %d while current session id "
                  "is %d\n",
                  recvd_session_id,
                  scb.cmn.session_id);
      }

      t2_sm_post_event(EV_RECV_FRAG_COMPL); /* Go back to ST_IDLE state */
      break;

    case COMMAND_SEGMENT_WAIT_V2:
      /* Though the code flow is in receive() function. Current state is still
     * be ST_SEND_FRAG */
      log_debug("Received Fragment wait Command\n");
      if (scb.frag_compl_list[scb.cmn.session_id] == 1) {
        log_debug(
          "Already received Fragment Complete command for this session: "
          "%d\n",
          scb.cmn.session_id);
        t2_sm_post_event(EV_DUPL_FRAME);
        return false;
      }
      t2_sm_post_event(EV_RECV_FRAG_WAIT);
      scb.transmission_aborted = scb.cmn.session_id;
      /* call transport_service_senddata_cb() here that will halt the next
       * fragment send
       * function
       */
      transport_service_senddata_cb(1, 0);
      /* Refer 10.1.3.5.3 */
      rcb.cmn.pending_segments = byte2;
      log_debug("Pending fragments: %d\n", rcb.cmn.pending_segments);
      /* Shall we increment the scb.sending session id here or should we send it
     * in same session id */
      /* If the pending segments are 0 then the sending side is going to bombard
     * the receiving side with new fragments
     * so added a delay of 100ms regardless of number of pending segments
     */
      ctimer_set(&scb.wait_restart_timer,
                 (100 + 100 * rcb.cmn.pending_segments),
                 wait_restart_from_first,
                 NULL);
      break;
    default:
      log_debug("Unknown command type: 0X%02X\n", byte1 & 0xf8);
      break;
  }
  return 0;
}

static bool transport_service_command_handler(ts_node_id_t source,
                                              ts_node_id_t dest,
                                              receive_type rx_type,
                                              const uint8_t *frame_data,
                                              uint8_t frame_length)
{
  uint8_t cmd_type;
  uint16_t datagram_size_tmp = 0;

  ctimer_set(&scb.reset_timer, RESET_TIME, reset_transport_service, 0);
  log_debug("Received data: Source node:%d, Destination node: %d\n",
            source,
            dest);
  /* Tie break check */
  /* 1. The receiving node is currently transmitting a datagram.
   * 2. The recipient of the datagram being transmitted is also the
          originator of the received fragment
   * 3. The receiving node has a lower NodeID than the originator */
  if (is_transport_service_sending() && (scb.cmn.dest == source)
      && (my_node_id < source)) {
    log_debug("Tie breaking.  Ready to receive\n");
    log_debug("scb.cmn.p.dest: %d, source: %d, my_node_id: %d\n",
              scb.cmn.dest,
              source,
              my_node_id);
    t2_sm_post_event(EV_TIE_BREAK);
    if (frame_data[1] != COMMAND_SEGMENT_COMPLETE_V2) {
      log_debug("Failing the send session\n");
      if (on_send_complete_cb) {
        on_send_complete_cb(1, 0);
        on_send_complete_cb = 0;
      }
    }
    flag_tie_broken = 1;
  }

  log_debug("my_node_id: %d, Source :%d, Destination : %d\n",
            my_node_id,
            source,
            dest);

  /* There are some garbage retranmissions where the source and destination ids are messed up */
  if (source == dest) {
    log_debug("ERROR: source and destination is same node id? Ignoring the "
              "frame\n");
    return true;
  }

  log_debug("frame_length: %d\n", frame_length);
  if (frame_length > PAYLOAD_SIZE_MAX) {
    log_debug("Length of command received is more than PAYLOAD_SIZE_MAX. "
              "Ignorning the frame\n");
    return true;
  }

  /* incase FRAG_WAIT has to be sent backup the source, dest received */
  scb.frag_wait_source = dest;
  scb.frag_wait_dest   = source;

  if (
    ((source != rcb.current_snode) && (rcb.current_snode))
    || /* received frame from third node while receiving from second node */
    ((source != scb.current_dnode)
     && (scb
           .current_dnode))) { /* received frame from third node while sending to second node */
    if (rx_type == SINGLECAST) {
      log_debug(
        "Current Receiving source node is %d but received source node id is "
        "%d, session_id: %d\n",
        rcb.current_snode,
        source,
        (frame_data[3] & 0xf0) >> 4);
      log_debug(
        "Current Sending dest node is %d but received source node id is "
        "%d, session_id : %d\n",
        scb.current_dnode,
        source,
        (frame_data[3] & 0xf0) >> 4);

      if (current_state == ST_SEND_FRAG_WAIT) {
        return false;
      }
      t2_sm_post_event(EV_SCAST_DIFF_NODE); /*send_frag_wait_cmd */
      if (scb.sending) {
        log_debug("Next fragment sent will be FRAG_WAIT to %d\n", source);
        scb.flag_send_frag_wait = 1;
      } else {
        send_frag_wait_cmd();
      }
      return false;
    }

    if (rx_type == BROADCAST) {
      t2_sm_post_event(EV_BCAST_DIFF_NODE); /*drop_fragment */
      return true;
    }
  }

  cmd_type = frame_data[1] & 0xf8;

  if ((rcb.cmn.session_id == 0x10)
      && (cmd_type == COMMAND_SUBSEQUENT_FRAGMENT)) {
    datagram_size_tmp = (frame_data[1] & 0x07) << 8;
    datagram_size_tmp += frame_data[2];

    if (datagram_size_tmp > PAYLOAD_SIZE_MAX) {
      log_debug("datagram size is more than PAYLOAD_SIZE_MAX. Ignoring the "
                "fragment\n");
      t2_sm_post_event(EV_DIFF_SESSION);
      return true;
    }
    log_debug("Received subseq fragment without first fragment. "
              "session_id:%d\n",
              ((frame_data[3] & 0xf0) >> 4));
    t2_sm_post_event(EV_SUBSEQ_DIFF_SESSION);
    if (scb.sending) {
      scb.flag_send_frag_wait = 1;
    } else {
      send_frag_wait_cmd();
    }
    return true;
  }

  switch (current_state) {
    case ST_IDLE:
      t2_sm_post_event(EV_START_RECV);
      break;
    case ST_RECEIVING:
      t2_sm_post_event(EV_RECV_NEW_FRAG);
      break;
    case ST_WAIT_ACK:
      /*Next state is decided in receive() */
      /*Only fragment complete, fragment request or fragment wait are
      * expected */
      break;
    default:
      log_debug("Received a fragment in unexpected state %d. Processing it, "
                "incase we need to send FRAG_WAIT.\n",
                current_state);
      /*no break no return*/
      break;
  }

  if (flag_initialize_once) {
    log_debug("Initializing rcb.cmn.session_id to 0x10\n");
    flag_initialize_once = 0;
    memset((uint8_t *)&rcb.cmn, 0, sizeof(control_block_t));
    rcb.cmn.session_id      = 0x10;
    rcb.cur_recvd_data_size = 0;
    rcb.current_snode       = 0;
    memset(rcb.datagramData, 0, PAYLOAD_SIZE_MAX);
    memset(rcb.bytes_recvd_bitmask, 0, sizeof(rcb.bytes_recvd_bitmask));
  }
  rcb.fragment = frame_data;
  /*need to memcpy because the (ts_param_t*)p pointer is not valid when
   the rx_timer_expired is called by the timer*/
  rcb.cmn.source   = source;
  rcb.cmn.dest     = dest;
  rcb.fragment_len = frame_length;
  rcb.rx_type      = rx_type;

  return receive();
}

bool transport_service_on_frame_received(ts_node_id_t source,
                                         ts_node_id_t dest,
                                         receive_type rx_type,
                                         const uint8_t *frame_data,
                                         uint8_t frame_length)
{
  return transport_service_command_handler(source,
                                           dest,
                                           rx_type,
                                           frame_data,
                                           frame_length);
}

static void send_first_frag()
{
  uint8_t ret = 0;
  /* session id begins with random number and then keeps incrementing */
  if (!scb.cmn.session_id)
    scb.cmn.session_id = 0;
  else
    scb.cmn.session_id++;

  log_debug("Sending First Fragment\n");
  if (scb.cmn.session_id > 0xf) /*scb.cmn.session_id has only 4 bits for it */
    scb.cmn.session_id = 0;     /* Being back from 0 */

  scb.frag_compl_list[scb.cmn.session_id] = 0;
  log_debug("Max payload size: %d\n", max_fragment_size);

  if (scb.remaining_data_len == 0)
    scb.remaining_data_len = scb.datagram_len;

  if (scb.datagram_len > max_fragment_size) {
    scb.remaining_data_len = scb.datagram_len - max_fragment_size;
    scb.datalen_to_send    = max_fragment_size;
  } else {
    scb.remaining_data_len = scb.datagram_len;
    scb.datalen_to_send    = scb.datagram_len;
  }

  first_frag->cmdClass = COMMAND_CLASS_TRANSPORT_SERVICE_V2;

  /* Take 8th, 9th and 10th bit of scb.datagram_len */
  first_frag->cmd_datagramSize1
    = (COMMAND_FIRST_FRAGMENT) | ((scb.datagram_len >> 8) & 0x07);

  /* Take 0th-7th bit of scb.datagram_len */
  first_frag->datagramSize2 = scb.datagram_len & 0xff;
  first_frag->properties2   = scb.cmn.session_id << 4;
  log_debug("Packing session id %d\n", first_frag->properties2 >> 4);

  log_error("Length of payload being sent: %d\n", scb.datalen_to_send);
  memcpy(&first_frag->payload1, scb.datagram, scb.datalen_to_send);

  /*4 is size of ZW_COMMAND_FIRST_FRAGMENT_1BYTE_FRAME till payload field */
  add_crc(t2_txBuf, scb.datalen_to_send + 4);

  scb.cmn.pending_segments = scb.remaining_data_len / scb.datalen_to_send;
  if (scb.remaining_data_len % scb.datalen_to_send)
    scb.cmn.pending_segments++;

  if (scb.datagram_len <= max_fragment_size) { /* If it was only one fragment */
    scb.does_not_fit_in_first_frag = 0;
    t2_sm_post_event(EV_SEND_LAST_FRAG); /* send_last_frag() */
    send_last_frag();
    return;
  }

  log_debug("data_len: %d\n", sizeof(*first_frag) + scb.datalen_to_send - 1);
  ctimer_set(&scb.reset_timer, RESET_TIME, reset_transport_service, 0);
  ret = lower_layer_send_data(scb.cmn.source,
                              scb.cmn.dest,
                              t2_txBuf,
                              sizeof(*first_frag) + scb.datalen_to_send - 1,
                              0,
                              transport_service_senddata_cb);
  if (ret == 1) {
    log_debug("send_data failed\n\n");
    return;
  }

  scb.round_trip_first_frag = clock_time();

  scb.sending = 1;
}

transport_service_send_data_return_code_t
  transport_service_send_data(ts_node_id_t source,
                              ts_node_id_t dest,
                              const uint8_t *payload,
                              uint16_t payload_len,
                              uint16_t max_frame_len,
                              const on_transport_service_send_data_complete_t c)
{
  on_send_complete_cb = c;
  max_fragment_size   = max_frame_len - 7;
  if (payload_len > PAYLOAD_SIZE_MAX) {
    log_debug("Payload size is more than PAYLOAD_SIZE_MAX. "
              "Ignoring the fragment\n\n");
    return TRANSPORT_SERVICE_WILL_OVERFLOW;
  }
  memcpy(scb.datagram, payload, payload_len);
  ctimer_set(&scb.reset_timer, RESET_TIME, reset_transport_service, 0);
  log_debug("Request for Sending data: dataLength: %d, MyNodeid: %d "
            "Source node:%d, Destination node: %d\n",
            payload_len,
            (int)my_node_id,
            (int)source,
            (int)dest);
  if (is_transport_service_sending()) {
    log_debug("Another TX session is in progress. session id: %d\n",
              scb.cmn.session_id);
    log_debug("Sending buffer %p, while new request to send of buffer: %p\n",
              scb.datagram,
              payload);
    return TRANSPORT_SERVICE_BUSY;
  }
  if (is_transport_service_receiving()) {
    log_debug("Another RX session is in progress. session id: %d\n",
              rcb.cmn.session_id);
    log_debug("Sending buffer %p, while new request to send of buffer: %p\n",
              scb.datagram,
              payload);
    return TRANSPORT_SERVICE_BUSY;
  }

  scb.cmn.source            = source;
  scb.cmn.dest              = dest;
  scb.datagram_len          = payload_len;
  scb.sending               = 0;
  scb.flag_replied_frag_req = 0;
  scb.transmission_aborted  = 0x11; /*Initializing to out of range session id */
  scb.flag_send_frag_wait   = 0;
  scb.flag_reply_frag_req   = 0;
  scb.round_trip_first_frag = 0;
  scb.flag_fc_timer_expired_once = 0;
  scb.remaining_data_len         = 0;
  scb.current_dnode              = 0;

  log_debug("MyNodeid: %d Source node:%d, Destination node: %d\n",
            (int)my_node_id,
            (int)source,
            (int)dest);
  scb.current_dnode = dest;
  switch (current_state) {
    case ST_IDLE:
      log_debug("Current state: ST_IDLE\n");
      t2_sm_post_event(EV_START_SEND); /* send() */
      break;
    case ST_SEND_FRAG:
      log_debug("Current state: ST_SEND_FRAG\n");
      t2_sm_post_event(EV_SEND_NEW_FRAG); /* send() */
      break;
    default:
      log_debug("Trying to send fragment from wrong state: %d\n",
                current_state);
#if 0
      scb.cmn.completedFunc(S2_TRANSMIT_COMPLETE_FAIL, 0); /*FIXME: Need to decide what to do if we are trying to send while receiving */
      return 0;
#endif
      return TRANSPORT_SERVICE_BUSY;
  }
  send_first_frag();
  return TRANSPORT_SERVICE_SEND_SUCCESS;
}
