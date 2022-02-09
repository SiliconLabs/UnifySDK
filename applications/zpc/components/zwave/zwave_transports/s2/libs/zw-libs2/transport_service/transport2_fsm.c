/* © 2014 Silicon Laboratories Inc.
 */

 /* Author: mdumbare */

#include <stddef.h>
#include "transport2_fsm.h"
#ifdef GRAPH /* This is set by Makefile.transport2_fsm while generating graphs */
#define 
#define uint16_t unsigned short
#define uint8_t unsigned char

#define VOID_CALLBACKFUNC(completedFunc)  void ( *completedFunc)
#else
#include "transport_service2.h"
//#include <TYPES.H>
#endif

#ifdef __C51__
#define slash /
#define printf  slash/

#define ST_DEFINE(a, b, c, d) {a, b, c}

#else

const char *T2_STATES_STRING[]  = {
        "ST_IDLE",

        /* Receive state machine states */
        "ST_RECEIVING",
        "ST_SEND_FRAG_COMPLETE",
        "ST_SEND_FRAG_REQ",
        "ST_SEND_FRAG_WAIT",
        "ST_FIND_MISS_FRAG",

        /* Send state machine states */
        "ST_SEND_FRAG",
        "ST_SEND_LAST_FRAG",
        "ST_WAIT_ACK",

};
const char *T2_EVENTS_STRING[] = {
        /* Receive state machine events */
        "EV_START_RECV",
        "EV_RECV_NEW_FRAG",
        "EV_NO_SESSION_ID",
        "EV_SEND_FRAG_COMPLETE",
        "EV_NO_SEND_FRAG_COMPLETE",
        "EV_SUCCESS",
        "EV_SUCCESS2",
        "EV_SUCCESS3",
        "EV_FRAG_RX_TIMER",
        "EV_MISSING_FRAG",
        "EV_MISSING_FRAG_BCAST",
        "EV_SCAST_DIFF_NODE",
        "EV_BCAST_DIFF_NODE",
        "EV_DIFF_SESSION",
        "EV_SUBSEQ_DIFF_SESSION",
        "EV_FRAG_REQ_COMPL_DIFF_SESSION",
        "EV_FRAG_REQ_COMPL_WAIT_DIFF_NODE",
        "EV_RECV_FRAG_REQ",
        "EV_RECV_LAST_FRAG",
        "EV_RECV_FRAG_COMPLETE",
        "EV_TIE_BREAK",
        "EV_DUPL_FRAME",
        "EV_FRAG_REQ_OR_COMPL",

        /* Send state machine events */
        "EV_START_SEND",
        "EV_SEND_NEW_FRAG",
        "EV_SEND_NXT_MISS_FRAG",
        "EV_SENT_MISS_FRAG",
        "EV_RECV_FRAG_WAIT",
        "EV_SEND_LAST_FRAG",
        "EV_SEND_LAST_MISS_FRAG",
        "EV_FRAG_COMPL_TIMER",
        "EV_FRAG_COMPL_TIMER2",
        "EV_FRAG_COMPL_TIMER_REQ",
        "EV_FAILURE_LAST_FRAG2",
        "EV_RECV_FRAG_COMPL",
        "EV_REPLY_FRAG_REQ_DONE",
        "EV_FAIL",
};

#define ST_DEFINE(a, b, c, d) {a, b, c, d}

#endif

#define STATE enum TRANSPORT2_STATES
#define EVENT enum TRANSPORT2_EVENTS

typedef struct {
        STATE st;             /* Current state */
        EVENT ev;             /* Incoming event */
        STATE next_st;
        char *fun_name;    /* Action function returning next state */
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
         ST_DEFINE(ST_IDLE, EV_START_RECV, ST_RECEIVING, "start receive"),
         ST_DEFINE(ST_IDLE, EV_START_SEND, ST_SEND_FRAG, "send_first_frag"),

        /* This happens when there is a fragment received from different session or node, which
            will be ignored. Then the state machine lands up in ST_IDLE */
         ST_DEFINE(ST_IDLE, EV_SEND_NEW_FRAG, ST_SEND_FRAG, "keep sending"),

        /* Send new frame. Call the same funciton i.e. send(); */
         ST_DEFINE(ST_SEND_FRAG, EV_SEND_NEW_FRAG, ST_SEND_FRAG, "send_subseq_frag"),

         ST_DEFINE(ST_RECEIVING, EV_SEND_NEW_FRAG, ST_SEND_FRAG, "send"),
         ST_DEFINE(ST_WAIT_ACK, EV_SEND_NEW_FRAG, ST_SEND_FRAG, "send"),

        /* If the current state is ST_RECEIVING and new fragment arrives */
        /* If the fragment length is wrong */
        /* If the crc is bad */
         ST_DEFINE(ST_RECEIVING, EV_RECV_NEW_FRAG, ST_RECEIVING, "process new fram"),

        /* If duplicate frame arrives after frag completion move back to idle state */
         ST_DEFINE(ST_RECEIVING, EV_DUPL_FRAME, ST_IDLE, "discard the fragment"),

        /* If the fragment received is corrupt and there is no session ID. Receiving node
         * has to send Fragment wait command to sending node */
         ST_DEFINE(ST_RECEIVING, EV_NO_SESSION_ID, ST_SEND_FRAG_WAIT, "send_frag_wait_cmd"),

        /* If the fragment received is singlecast fragment while receiving another fragmented
         * datagram. Receiving node has to send Fragment wait command to sending node */
         ST_DEFINE(ST_RECEIVING, EV_SCAST_DIFF_NODE, ST_SEND_FRAG_WAIT, "send_frag_wait_cmd"),

        /* If the fragment received is singlecast fragment while sending another fragmented
         * datagram. */
         ST_DEFINE(ST_SEND_FRAG, EV_SCAST_DIFF_NODE, ST_SEND_FRAG_WAIT, "send_frag_wait_cmd"),

        /* If the fragment received is singlecast fragment while waiting for ack (frag req or frag compl)
        */
        ST_DEFINE(ST_WAIT_ACK, EV_SCAST_DIFF_NODE, ST_SEND_FRAG_WAIT, "send_frag_wait_cmd"),


        /* If the fragment received is singlecast fragment while receiving fragmented
         * datagram from different session. Receiving node has to discard the fragment */
         ST_DEFINE(ST_RECEIVING, EV_DIFF_SESSION, ST_IDLE, "discard the fragment"),

         ST_DEFINE(ST_RECEIVING, EV_SUBSEQ_DIFF_SESSION, ST_SEND_FRAG_WAIT, "send frag_wait"),

        /* Fragment request/complete/wait from different node id*/
         ST_DEFINE(ST_RECEIVING, EV_FRAG_REQ_COMPL_WAIT_DIFF_NODE, ST_IDLE, "discard the fragment"),

        /* Fragment request/complete/wait from different session */
         ST_DEFINE(ST_RECEIVING, EV_FRAG_REQ_COMPL_WAIT_DIFF_SESSION, ST_IDLE, "discard the fragment"),

        /* Go back to receiving state if Fragment wait command is sent successfully */
         ST_DEFINE(ST_SEND_FRAG_WAIT, EV_SUCCESS, ST_RECEIVING, NULL),
        /* Go back to sending state as the fragment wait was sent in sending session */
         ST_DEFINE(ST_SEND_FRAG_WAIT, EV_SUCCESS2, ST_SEND_FRAG, NULL),
        /* Go back to ST_WAIT_ACK as the fragment wait was sent in sending session */
         ST_DEFINE(ST_SEND_FRAG_WAIT, EV_SUCCESS3, ST_WAIT_ACK, NULL),

        /* TODO. What if sending fragment wait command fails ?*/

        /* If Fragment complete has been sent to sending node successfully, Go back to receiving
         * state */
         ST_DEFINE(ST_SEND_FRAG_COMPLETE, EV_SUCCESS, ST_IDLE, NULL),
        /*After sending one fragment request go back to receiving state, either we will receive
        another fragment request of fragment complete */
         ST_DEFINE(ST_SEND_FRAG_REQ, EV_SUCCESS, ST_RECEIVING, NULL),

        /* If RX timer event happens while ST_RECEIVING */
         ST_DEFINE(ST_RECEIVING, EV_FRAG_RX_TIMER, ST_FIND_MISS_FRAG, "find_missing"),
        /* If there is Fragment Rx timer event whiel sending Fragment Req command discard all the previous
         * Discard received fragmets in the datagram and go to Idle state */
         ST_DEFINE(ST_IDLE, EV_FRAG_RX_TIMER, ST_IDLE, "discard_all_received_fragments"),

        /* If the fragment received is broadcast fragment while receiving another fragmented datagram,
         * Receiving node needs to drop that fragment */
         ST_DEFINE(ST_RECEIVING, EV_BCAST_DIFF_NODE, ST_RECEIVING, "drop_fragment"),

         ST_DEFINE(ST_RECEIVING, EV_RECV_LAST_FRAG, ST_FIND_MISS_FRAG, "find_missing"),
         ST_DEFINE(ST_WAIT_ACK, EV_SEND_FRAG_COMPLETE, ST_SEND_FRAG_COMPLETE, NULL),
         ST_DEFINE(ST_RECEIVING, EV_RECV_FRAG_COMPL, ST_IDLE, NULL),

        /* Stay in the same state ST_WAIT_ACK until we know if its REQUEST OR COMPLETE cmd */
         ST_DEFINE(ST_WAIT_ACK, EV_FRAG_REQ_OR_COMPL, ST_WAIT_ACK, NULL),

        /* If the rx timer expires after sending Fragment Request, discard all fragments and go to IDLE state */
         ST_DEFINE(ST_WAIT_ACK, EV_FRAG_RX_TIMER, ST_IDLE, "discard all fragments"),

        /* Need to move to ST_RECEIVING state to process the FRAGMENT_REQUEST or FRAGMENT_COMPLETE received while sending */
         ST_DEFINE(ST_RECEIVING, EV_FRAG_REQ_OR_COMPL, ST_RECEIVING, "process received"),
         ST_DEFINE(ST_SEND_FRAG, EV_FRAG_REQ_OR_COMPL, ST_RECEIVING, "process received"),

        /* If there are missing frames, receiving node needs to send Fragment request command to
         * sending node */
         ST_DEFINE(ST_FIND_MISS_FRAG, EV_MISSING_FRAG, ST_SEND_FRAG_REQ, "send_frag_req_cmd"),
        /* If the broacast fragment is being received and rx timer times out of there are some missing fragments*/
         ST_DEFINE(ST_FIND_MISS_FRAG, EV_MISSING_FRAG_BCAST, ST_IDLE, NULL),
        /* If all the fragments of the datagram are receivied receiving node needs to send
         * Fragment complete command to sending node. */
         ST_DEFINE(ST_FIND_MISS_FRAG, EV_SEND_FRAG_COMPLETE, ST_SEND_FRAG_COMPLETE, "send_frag_complete_cmd"),

        /* NO need to send frag complete if it was bcast datagram which was being recived */
         ST_DEFINE(ST_FIND_MISS_FRAG, EV_NO_SEND_FRAG_COMPLETE, ST_IDLE, NULL),

        /* ======================*/
        /* Sending State diagram */
        /* ======================*/


         ST_DEFINE(ST_SEND_FRAG, EV_RECV_FRAG_WAIT, ST_SEND_FRAG, "wait_restart_from_first"),

        /* ST_SEND_FRAG ->(EV_TIE_BREAK) -> ST_RECEIVING ->(EV_RECV_NEW_FRAG) ->( EV_RECV_FRAG_WAIT )*/
         ST_DEFINE(ST_RECEIVING, EV_RECV_FRAG_WAIT, ST_SEND_FRAG, "wait_restart_from_first"),

         ST_DEFINE(ST_SEND_FRAG, EV_SEND_LAST_FRAG, ST_SEND_LAST_FRAG, "send_last_frag"),
         ST_DEFINE(ST_SEND_FRAG, EV_SEND_LAST_MISS_FRAG, ST_SEND_LAST_FRAG, "send_last_frag"),

         ST_DEFINE(ST_SEND_LAST_FRAG, EV_SUCCESS, ST_WAIT_ACK, NULL),
        /* If the Fragment completion timer even happens send the last fragment again */
         ST_DEFINE(ST_WAIT_ACK, EV_FRAG_COMPL_TIMER, ST_SEND_LAST_FRAG, "send_last_frag"),

        /*  fc_timer_expired in reply_frag_req*/
         ST_DEFINE(ST_WAIT_ACK, EV_FRAG_COMPL_TIMER_REQ, ST_IDLE, "send failure to application"),

        /* If the Fragment completion timer event happens again send error to application */
         ST_DEFINE(ST_WAIT_ACK, EV_FRAG_COMPL_TIMER2, ST_IDLE, NULL),

        /* Failure in sending last fragment second time after one Fragment completion timer event */
        /*P.S. This is not failure in sending the last fragment at first time */
         ST_DEFINE(ST_WAIT_ACK, EV_FAILURE_LAST_FRAG2, ST_IDLE, NULL),

        /* If received Fragment request command */
         ST_DEFINE(ST_WAIT_ACK, EV_RECV_FRAG_REQ, ST_SEND_FRAG, "reply_frag_req"),
         ST_DEFINE(ST_RECEIVING, EV_RECV_FRAG_REQ, ST_SEND_FRAG, "reply_frag_req"),

        /* If received Fragment complete command, disable the fragment complete timer*/
         ST_DEFINE(ST_WAIT_ACK, EV_RECV_FRAG_COMPL, ST_IDLE, NULL),

         ST_DEFINE(ST_SEND_FRAG, EV_SEND_NXT_MISS_FRAG, ST_SEND_FRAG, "reply_frag_req"),
         ST_DEFINE(ST_SEND_FRAG, EV_FRAG_COMPL_TIMER, ST_IDLE, "wait"),

        /* This happens when the FC_complete timer happens before  reply_frag_req() gets a callback
           state machine is still in ST_SEND_FRAG as it goes in ST_WAIT_ACK in the callback*/
         ST_DEFINE(ST_SEND_FRAG, EV_FRAG_COMPL_TIMER_REQ, ST_IDLE, "wait"),

         ST_DEFINE(ST_SEND_FRAG, EV_SENT_MISS_FRAG, ST_WAIT_ACK, NULL),
         ST_DEFINE(ST_SEND_FRAG, EV_TIE_BREAK, ST_RECEIVING, "receive"),
         ST_DEFINE(ST_SEND_LAST_FRAG, EV_TIE_BREAK, ST_RECEIVING, "receive"),
         ST_DEFINE(ST_WAIT_ACK, EV_RECV_FRAG_WAIT, ST_SEND_FRAG, NULL),

        /* If the ZW_SendData() function itself fails we go back to IDLE state
           The other side will send fragment request command. But we need to be in IDLE
            state to atleast receive and process it */
         ST_DEFINE(ST_SEND_LAST_FRAG, EV_FAIL, ST_IDLE, NULL),
         ST_DEFINE(ST_WAIT_ACK, EV_TIE_BREAK, ST_RECEIVING, "receive"),
};

TRANSPORT2_ST_T current_state = ST_IDLE;
uint8_t find_transition(TRANSPORT2_ST_T cstate, TRANSPORT2_EV_T event)
{
        uint8_t num_trans;

        num_trans = sizeof(trans) / sizeof(trans[0]); /* FIXME better to move it inside some init function */
//        printf("Transprot2: Current State: %s(%d), Event: %s(%d)\n", T2_STATES_STRING[current_state], current_state, T2_EVENTS_STRING[event], event);
        while(num_trans--) {        
                if ((cstate == trans[num_trans].st) && (event == trans[num_trans].ev))
                        return num_trans+1;
        }

//        printf("Transport2: Transition not found.");
        return 0;
}

void t2_sm_post_event(TRANSPORT2_EV_T ev)
{
/*
        function fn;
*/
        uint8_t i;

        i = find_transition(current_state, ev);
        if (i) {
                current_state = trans[i-1].next_st;
        } else {
        /* TODO FIXME: can not find the transtion. probably stay in the same state */
        /* print error for debugging */
        }
}
