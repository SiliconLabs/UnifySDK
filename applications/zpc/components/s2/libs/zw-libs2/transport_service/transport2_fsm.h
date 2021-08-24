/* © 2017 Silicon Laboratories Inc.
 */
/*
 * transport2_fsm.h
 *
 *      Author: mdumbare
 */

#ifndef TRANSPORT2_H_
#define TRANSPORT2_H_

/* States */
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

void t2_sm_post_event(TRANSPORT2_EV_T ev);

extern const char *T2_EVENTS_STRING[];
extern const char *T2_STATES_STRING[];
#endif
