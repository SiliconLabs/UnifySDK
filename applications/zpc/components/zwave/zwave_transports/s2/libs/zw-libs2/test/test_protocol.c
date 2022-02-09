/* © 2014 Silicon Laboratories Inc.
 */
/*
 * test_protocol.c
 *
 *  Created on: Jun 24, 2015
 *      Author: aes
 */
#include "unity.h"
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>
#include<stdarg.h>
#include<stdio.h>

#define ZW_CONTROLLER // defined type definitions match the S2 library we are unit testing
#include "../include/S2.h"
#include "../include/s2_protocol.h"
#include "../include/s2_keystore.h"
#include "../include/s2_psa.h"
#include "ctr_drbg.h"
#include "ZW_typedefs.h"
#include "ZW_classcmd.h"

#ifdef __C51__
/* Local stub implementation of ZW_WatchDogKick() when compiling for C51. */
void ZW_WatchDogKick()
{
  // Do nothing. Stub implementation.
}
#endif // __C51__

extern int verbose;

#define TIMER_STOPPED -1;  /**< Special value used by struct test_state field timeout_value */

struct test_state
{
  uint8_t frame[1280+8+4];
  uint16_t frame_len;

  int fcount; //number of frames sent

  int timeout_value;

  int s2_send_done;
  s2_tx_status_t s2_send_status;

  int send_data_return_fail; //Make send_data return fail

  uint8_t rx_frame[1280];
  uint16_t rx_frame_len;

  s2_connection_t last_trans; //The last transaction reversed

  struct {
    uint16_t count;      // Count of S2 Synchronization Events received
    uint8_t reason;      // The latest reason value received
    node_t remote_node; // Remote node ID of the latest S2 Synchronization event
    uint8_t seqno;
    node_t local_node;
  } sync_ev;
} ts;

uint8_t S2_send_frame_multi(struct S2* ctxt,s2_connection_t *peer, uint8_t* buf, uint16_t len) {


  if(verbose) {
      printf("********** MULTICAST ***************\n");
  }
  return S2_send_frame(ctxt,peer,buf,len);
}


uint8_t
S2_send_frame_no_cb(struct S2* ctxt, const s2_connection_t* dst, uint8_t* buf, uint16_t len)
{
  return S2_send_frame(ctxt,dst,buf,len);
}


uint8_t
S2_send_frame(struct S2* ctxt, const s2_connection_t* dst, uint8_t* buf, uint16_t len)
{

  int i;
  s2_connection_t swap;
  if(verbose) {
#if defined(__C51__) || defined (SINGLE_CONTEXT)
      printf("Sending  %02u -> %02u (len %04u) : ", dst->l_node, dst->r_node,len);
#else
      printf("Sending %p %02u -> %02u (len %04u) : ", ctxt, dst->l_node, dst->r_node,len);
#endif
      for (i = 0; i < len; i++)
        {
          printf("%02X", buf[i]);
        }
      printf("\n");
  }

  TEST_ASSERT(len < 1280+8+4);

  if (ts.send_data_return_fail)
    {
      if(verbose) printf("making send_data return fail\n");
      return 0;
    }

  memcpy(ts.frame, buf, len);
  ts.frame_len = len;
  ts.fcount++;

  swap = *dst;
  ts.last_trans.l_node = swap.r_node;
  ts.last_trans.r_node = swap.l_node;

  return 1;
}

void
S2_set_timeout(struct S2* ctxt, uint32_t interval)
{
  //ctxt = ctxt;
  if(verbose) printf("Timer set on interval %lu\n", (long unsigned)interval);
  ts.timeout_value = interval;
}

void
S2_stop_timeout(struct S2* ctxt)
{
  //ctxt = ctxt;
  if(verbose) printf("Timer stopped\n");
  ts.timeout_value = TIMER_STOPPED;
}

void
S2_send_done_event(struct S2* ctxt, s2_tx_status_t status)
{
  //ctxt=ctxt;
  if(verbose) printf("S2 send done status %d\n", (int)status);

  ts.s2_send_done++;
  ts.s2_send_status = status;
}

void
S2_msg_received_event(struct S2* ctxt, s2_connection_t* src, uint8_t* buf, uint16_t len)
{
  //int i;
  //ctxt = ctxt;

  if(verbose) {
      printf("Message received. from %d -> %d\n ", src->r_node, src->l_node);
      puts((char*)buf);
      /*for (i = 0; i < len; i++)
        {
          printf("%02X", buf[i]);
        }
      printf("\n");*/

  }

  memcpy(ts.rx_frame, buf, len);
  ts.rx_frame_len = len;
}

void
S2_get_hw_random(uint8_t* rnd,uint8_t len)
{
  int i;
  for (i = 0; i < len; i++)
    {
      rnd[i] = rand() % 0xFF;
    }
}

void S2_get_commands_supported(node_t lnode,uint8_t class_id, const uint8_t ** cmdClasses, uint8_t* length) {
  static uint8_t cmd_classes[] = {COMMAND_CLASS_SECURITY_2};
  *length = sizeof(cmd_classes);
  *cmdClasses = cmd_classes;
}

void S2_resynchronization_event(
    node_t remote_node,
    sos_event_reason_t reason,
    uint8_t seqno,
    node_t local_node)
{
  ts.sync_ev.count++;
  ts.sync_ev.reason = reason;
  ts.sync_ev.seqno = seqno;
  ts.sync_ev.remote_node = remote_node;
  ts.sync_ev.local_node = local_node;
}

//---------------- global test state ---------------------
s2_connection_t conn12 =
  { 1, 2 }; //Sending form node 1 to 2
s2_connection_t conn21 =
  { 2, 1 }; //Sending form node 2 to 1

s2_connection_t conn13 =
  { 1, 3 }; //Sending form node 1 to 3
s2_connection_t conn31 =
  { 3, 1 }; //Sending form node 3 to 1


/*
 * Test that we a able to send a message from once instance to another
 */
const uint8_t key[] =
  { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
      0x0, };

const uint8_t key2[] = {0xab,0xab,0xab,0xab,0xab,0xab,0xab,0xab,0xab,0xab,0xab,0xab,0xab,0xab,0xab,0xab,};

const uint8_t key3[] = {0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,0xac,};

const char hello[] = "HelloWorld";
const uint8_t nonce_get[] =
  { COMMAND_CLASS_SECURITY_2, SECURITY_2_NONCE_GET, 0 };
const uint8_t nonce_report[] =
  { COMMAND_CLASS_SECURITY_2, SECURITY_2_NONCE_REPORT };

const uint8_t commands_supported_report[] = {COMMAND_CLASS_SECURITY_2,SECURITY_2_COMMANDS_SUPPORTED_REPORT,COMMAND_CLASS_SECURITY_2};

struct S2* ctx1;
struct S2* ctx2;
struct S2* ctx3;

static void
my_setup()
{
  srand(0x44);
  //reset test state
  memset(&ts, 0, sizeof(ts));
  conn12.tx_options = 0;
  conn21.tx_options = 0;

  if(ctx1) {
      S2_destroy(ctx1);
  }
  ctx1 = S2_init_ctx( 0xAABBCCDD);

  if(ctx2) {
      S2_destroy(ctx2);
  }
  ctx2 = S2_init_ctx( 0xAABBCCDD);

  if(ctx3) {
      S2_destroy(ctx3);
  }
  ctx3 = S2_init_ctx( 0xAABBCCDD);

  S2_init_prng();

  S2_network_key_update(ctx1,ZWAVE_KEY_ID_NONE,0,key,0);
  S2_network_key_update(ctx2,ZWAVE_KEY_ID_NONE,0,key,0);
  S2_network_key_update(ctx3,ZWAVE_KEY_ID_NONE,0,key,0);

  S2_network_key_update(ctx1,ZWAVE_KEY_ID_NONE,1,key2,0);
  S2_network_key_update(ctx2,ZWAVE_KEY_ID_NONE,1,key2,0);
  S2_network_key_update(ctx3,ZWAVE_KEY_ID_NONE,1,key2,0);

  S2_network_key_update(ctx1,ZWAVE_KEY_ID_NONE,2,key3,0);
  S2_network_key_update(ctx2,ZWAVE_KEY_ID_NONE,2,key3,0);
  S2_network_key_update(ctx3,ZWAVE_KEY_ID_NONE,2,key3,0);

}

void tearDownSuite(void)
{
  if(ctx1) {
    S2_destroy(ctx1);
    ctx1 = NULL;
  }
  if(ctx2) {
    S2_destroy(ctx2);
    ctx2 = NULL;
  }
  if(ctx3) {
    S2_destroy(ctx3);
    ctx3 = NULL;
  }
}
/*In general we should valgrind to make sure there is no buffer overruns*/


void wrap_test_s2_send_data(struct S2* ctx_dest, s2_connection_t* dst)
{
  ts.fcount = 0;
  ts.s2_send_done =0;
  S2_send_data(ctx1, dst, (uint8_t*) hello, sizeof(hello));

  TEST_ASSERT_EQUAL_STRING_LEN(nonce_get, ts.frame, 2);
  TEST_ASSERT_EQUAL(3, ts.frame_len);
  TEST_ASSERT_EQUAL(1, ts.fcount);
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx_dest, &ts.last_trans, ts.frame, ts.frame_len);
  /*Expect that we get a nonce report*/
  TEST_ASSERT_EQUAL(2, ts.fcount);
  TEST_ASSERT_EQUAL(20, ts.frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(nonce_report, ts.frame, 2);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);

  S2_send_frame_done_notify(ctx_dest, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*Send the frame to ctx1*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);
  TEST_ASSERT_EQUAL(3, ts.fcount);

  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  //TEST_ASSERT_EQUAL(ts.frame[2],COMMAND_CLASS_SECURITY_2); //seq
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION_PROPERTIES1_EXTENSION_BIT_MASK, ts.frame[3]);
  TEST_ASSERT_EQUAL(18, ts.frame[4]);
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_CRITICAL_FLAG | S2_MSG_EXTHDR_TYPE_SN,
      ts.frame[5]);

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  TEST_ASSERT_EQUAL(1, ts.s2_send_done); //Check we got a send done event
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status); //Check we got a send done event

  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx_dest, &ts.last_trans, ts.frame, ts.frame_len);
  TEST_ASSERT_EQUAL(sizeof(hello), ts.rx_frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(hello, ts.rx_frame, sizeof(hello));
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}

/**
 * Positive test. Check that we can send a single cast message between two instances, without
 * having a span established first.
 *
 * Verify the frame flow
 * - Nonce_get
 * - Nonce_raport, with SOS
 * - Encap_message with SN
 *
 * Verify the status code of the s2_send_data callback
 * Verify the frame received by the ctx2
 *
 */
void test_s2_send_data()
{
  my_setup();

  wrap_test_s2_send_data(ctx2, &conn12);
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}



/**
 * Test that we can use single frame transmissions once SPAN
 * has been established.
 *
 */
void test_single_frame_transmission()
{
  const char hello[] = "HelloWorld Second Frame";

  test_s2_send_data();
  /* ----------- now send second frame with SPAN established --------- *
   */

  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;

  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  //TEST_ASSERT_EQUAL(ts.frame[2],COMMAND_CLASS_SECURITY_2); //seq
  TEST_ASSERT_EQUAL(0, ts.frame[3]); //Verify that there is no SN included
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);

  TEST_ASSERT_EQUAL(1, ts.fcount);
  TEST_ASSERT_EQUAL(sizeof(hello), ts.rx_frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(hello, ts.rx_frame, sizeof(hello));
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}



/**
 * Make send data return false in nonce get
 */
void test_send_data_fail_in_nonce_get()
{
  my_setup();
  ts.send_data_return_fail = 1;
  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));
  TEST_ASSERT_EQUAL(1, ts.s2_send_done);
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_FAIL, ts.s2_send_status);
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}

/**
 * Make send data return transmit complete no ack in nonce get
 */
void test_transmit_complete_no_ack_in_nonce_get()
{
  my_setup();
  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));
  TEST_ASSERT_EQUAL_STRING_LEN(nonce_get, ts.frame, 2);
  TEST_ASSERT_EQUAL(3, ts.frame_len);
  TEST_ASSERT_EQUAL(1, ts.fcount);
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_NO_ACK,0x42);

  TEST_ASSERT_EQUAL(1, ts.s2_send_done);
  TEST_ASSERT_NOT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status);
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}

/**
 * Make make system timeout before nonce report is sent
 */
void test_transmit_timeout_after_sending_nonce_get()
{
  my_setup();
  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));
  TEST_ASSERT_EQUAL_STRING_LEN(nonce_get, ts.frame, 2);
  TEST_ASSERT_EQUAL(3, ts.frame_len);
  TEST_ASSERT_EQUAL(1, ts.fcount);
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
  S2_timeout_notify(ctx1);

  TEST_ASSERT_EQUAL(1, ts.s2_send_done);
  TEST_ASSERT_NOT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status);
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}

/**
 * Make make system timeout before before ACK of nonce get is sent.
 */
void test_transmit_timeout_before_sending_nonce_get()
{
  my_setup();
  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));
  TEST_ASSERT_EQUAL_STRING_LEN(nonce_get, ts.frame, 2);
  TEST_ASSERT_EQUAL(3, ts.frame_len);
  TEST_ASSERT_EQUAL(1, ts.fcount);
  S2_timeout_notify(ctx1);
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  TEST_ASSERT_EQUAL(1, ts.s2_send_done);
  TEST_ASSERT_NOT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status);
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}



/* Test that duplicate nonce get frames are ignored
 * Test that duplicate nonce raport frames are ignored
 * Test that duplicate msg enap frames are ignored */
void test_dup_nonce_get()
{
  uint8_t backup_frame[sizeof(ts.frame)];
  uint8_t backup_len;

  my_setup();
  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));

  TEST_ASSERT_EQUAL_STRING_LEN(nonce_get, ts.frame, 2);
  TEST_ASSERT_EQUAL(3, ts.frame_len);
  TEST_ASSERT_EQUAL(1, ts.fcount);
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  memcpy(backup_frame,ts.frame,ts.frame_len);
  backup_len = ts.frame_len;
  /*Send the frame to ctx2 twice */
  S2_application_command_handler(ctx2, &conn21, backup_frame, backup_len);
  S2_application_command_handler(ctx2, &conn21, backup_frame, backup_len);

  /*Expect that we get a nonce report*/
  TEST_ASSERT_EQUAL(2, ts.fcount);
  TEST_ASSERT_EQUAL(20, ts.frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(nonce_report, ts.frame, 2);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);

  S2_send_frame_done_notify(ctx2, S2_TRANSMIT_COMPLETE_OK,0x42);

  memcpy(backup_frame,ts.frame,ts.frame_len);
  backup_len = ts.frame_len;
  /*Send the frame to ctx1 twice */
  S2_application_command_handler(ctx1, &conn12, backup_frame, backup_len);
  S2_application_command_handler(ctx1, &conn12, backup_frame, backup_len);

  TEST_ASSERT_EQUAL(3, ts.fcount);

  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  //TEST_ASSERT_EQUAL(ts.frame[2],COMMAND_CLASS_SECURITY_2); //seq
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION_PROPERTIES1_EXTENSION_BIT_MASK, ts.frame[3]);
  TEST_ASSERT_EQUAL(18, ts.frame[4]);
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_CRITICAL_FLAG | S2_MSG_EXTHDR_TYPE_SN,
      ts.frame[5]);

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  TEST_ASSERT_EQUAL(1, ts.s2_send_done); //Check we got a send done event
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status); //Check we got a send done event

  memcpy(backup_frame,ts.frame,ts.frame_len);
  backup_len = ts.frame_len;
  /*Send the frame to ctx2 twice */
  S2_application_command_handler(ctx2, &conn21, backup_frame, backup_len);
  S2_application_command_handler(ctx2, &conn21, backup_frame, backup_len);


  TEST_ASSERT_EQUAL(sizeof(hello), ts.rx_frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(hello, ts.rx_frame, sizeof(hello));
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}


/**
 * Test that we get transmissions complete fail when send_data
 * fails in encap_message
 *
 */
void test_send_data_fail_in_encap_message()
{
  const char hello[] = "HelloWorld Second Frame";

  test_s2_send_data();
  /* ----------- now send second frame with SPAN established --------- *
   */

  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;
  ts.send_data_return_fail=1;
  ts.s2_send_done =0;

  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));
  TEST_ASSERT_EQUAL(1, ts.s2_send_done);
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_FAIL, ts.s2_send_status);
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}

/**
 * Test that we get transmissions complete fail when send_data
 * fails in encap_message
 *
 */
void test_send_data_no_ack_in_encap_message()
{
  const char hello[] = "HelloWorld Second Frame";

  test_s2_send_data();
  /* ----------- now send second frame with SPAN established --------- *
   */

  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;
  ts.s2_send_done =0;

  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_NO_ACK,0x42);
  TEST_ASSERT_EQUAL(1, ts.s2_send_done);
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_NO_ACK, ts.s2_send_status);
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}


/* Test that sequence counter increases and test all message lengths */

  /*  loop len 0..1280
   *     send encap message with len
   *     verify message decryption
   *     verify seq increases
   */
void test_encap_seq_increase()
{
  uint8_t test_buf[1280];
  uint8_t seq;
  int i;

  for(i=0; i < sizeof(test_buf); i++) {
      test_buf[i] = rand() & 0xFF;
  }


  test_s2_send_data();
  /* ----------- now send second frame with SPAN established --------- *
   */

  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;

  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);
  seq = ts.frame[2];

  TEST_ASSERT_EQUAL(1, ts.fcount);
  TEST_ASSERT_EQUAL(sizeof(hello), ts.rx_frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(hello, ts.rx_frame, sizeof(hello));

  ts.fcount=0;
  for(i=1; i < 1280; i++) {
    S2_send_data(ctx1, &conn12, (uint8_t*) test_buf, i);
    TEST_ASSERT_EQUAL(i, ts.fcount);

    S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
    /*Send the frame to ctx2*/
    S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);
    TEST_ASSERT_EQUAL(++seq, ts.frame[2]);

    TEST_ASSERT_EQUAL(i, ts.rx_frame_len);
    TEST_ASSERT_EQUAL_STRING_LEN(test_buf, ts.rx_frame, i);
    }
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
  }

/*Validate nonce report */
void test_nonce_report()
{
  uint8_t nonce_get[] = {COMMAND_CLASS_SECURITY_2,SECURITY_2_NONCE_GET,42};
  int seq2;
  uint8_t nonce[8];
  my_setup();

  /*send nonce get */
  S2_application_command_handler(ctx2,&conn12,nonce_get,3);
  S2_send_frame_done_notify(ctx2,S2_TRANSMIT_COMPLETE_OK,0x42);

  /* verify nonce report save nonce for later */
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2,ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT,ts.frame[1]);
  seq2 = ts.frame[2];
  memcpy(nonce,&ts.frame[4],8);

  /*verify sequence number */
  nonce_get[2]++;
  S2_application_command_handler(ctx2,&conn12,nonce_get,3);
  S2_send_frame_done_notify(ctx2,S2_TRANSMIT_COMPLETE_OK,0x42);
  TEST_ASSERT_EQUAL(2,ts.fcount);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2,ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT,ts.frame[1]);
  TEST_ASSERT_EQUAL(seq2+1,ts.frame[2]);


  /* verify reserved is 0 */
  /* verify SOS = 1 and MOS 0*
   */
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK,ts.frame[3]);
  /* Verify nonce bytes */
  /* Verify nonce bytes different than in case 1 */
  TEST_ASSERT( memcmp(nonce,&ts.frame[4],8) != 0 )

  /* Verify frame length */
  TEST_ASSERT_EQUAL(4+16,ts.frame_len);

  /* send nonce get which is too short ie 2 bytes */
  /* Verify that no report is sent */
  ts.fcount =0;
  nonce_get[2]++;
  S2_application_command_handler(ctx2,&conn12,nonce_get,2);
  S2_send_done_event(ctx2,S2_TRANSMIT_COMPLETE_OK);
  TEST_ASSERT_EQUAL(0,ts.fcount);

  /* send nonce get which has some extra data appened */
  /* Verify we get a valid report*/
  nonce_get[2]++;
  S2_application_command_handler(ctx2,&conn12,nonce_get,6);
  S2_send_done_event(ctx2,S2_TRANSMIT_COMPLETE_OK);
  TEST_ASSERT_EQUAL(1,ts.fcount);
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}

/**
 * Check recover after a node has sent SOS
 *
 * Scenario:
 *
 * A sends MSG1 to B
 * B is out of sync and sends Nonce report
 * A sends MSG2 to B, with SN
 * B receives MSG 2
 *
 */
void test_sos_recover_scenario1() {

  const char hello[] = "HelloWorld Second Frame";

  test_s2_send_data();

  ts.rx_frame_len=0;
  ts.s2_send_done = 0;
  ts.fcount = 0;
  /* ----------- now send second frame with SPAN established --------- *
   */
  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
  TEST_ASSERT_EQUAL(1, ts.s2_send_done); //Check the callback

  /*Make the message invalid*/
  ts.frame[ts.frame_len-1] =0;
  ts.frame[ts.frame_len-2] =0;
  ts.frame[ts.frame_len-3] =0;
  ts.frame[ts.frame_len-4] =0;

  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);

  /*Verify that the ctx2 sends a nonce report SOS, because it could not receive the frame.*/
  TEST_ASSERT_EQUAL(2, ts.fcount); //Check the callback
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT, ts.frame[1]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);
  S2_send_frame_done_notify(ctx2, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);

  TEST_ASSERT_EQUAL(1, ts.sync_ev.count); // Test that a sync events was emitted after receiving Nonce Report SOS

  /* send a new frame */
  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));


  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION_PROPERTIES1_EXTENSION_BIT_MASK, ts.frame[3]); /*Check that SN is included */
  TEST_ASSERT_EQUAL(18, ts.frame[4]);
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_CRITICAL_FLAG | S2_MSG_EXTHDR_TYPE_SN,
      ts.frame[5]);

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);


  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);

  TEST_ASSERT_EQUAL(sizeof(hello), ts.rx_frame_len); //Verify that the frame has been received
  TEST_ASSERT_EQUAL_STRING_LEN(hello, ts.rx_frame, sizeof(hello));
  TEST_ASSERT_EQUAL(1, ts.sync_ev.count); // Test that no further sync events was emitted during test
}


/**
 * Check recover after a node has sent SOS
 *
 * Scenario:
 *
 * A sends MSG1 to B
 * B is out of sync and sends Nonce report
 * B sends MSG2, but is sending Nonce get first
 * A receives nonce get
 * A sends nonce report
 * B sends MSG 2
 * A Gets MSG 2
 *
 */
void test_sos_recover_scenario2() {

  const char hello[] = "HelloWorld Second Frame";

  test_s2_send_data();

  ts.rx_frame_len=0;
  ts.s2_send_done = 0;
  ts.fcount = 0;
  /* ----------- now send second frame with SPAN established --------- *
   */
  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
  TEST_ASSERT_EQUAL(1, ts.s2_send_done); //Check the callback

  /*Make the message invalid*/
  ts.frame[ts.frame_len-1] =0;
  ts.frame[ts.frame_len-2] =0;
  ts.frame[ts.frame_len-3] =0;
  ts.frame[ts.frame_len-4] =0;

  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);

  /*Verify that the ctx2 sends a nonce report SOS, because it could not receive the frame.*/
  TEST_ASSERT_EQUAL(2, ts.fcount); //Check the callback
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT, ts.frame[1]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);
  S2_send_frame_done_notify(ctx2, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);

  TEST_ASSERT_EQUAL(1, ts.sync_ev.count); // Test that a sync events was emitted when receiving the Nonce Report SOS while idle

  /* send a new frame from ctx 2*/
  S2_send_data(ctx2, &conn21, (uint8_t*) hello, sizeof(hello));

  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_GET, ts.frame[1]);
  S2_send_frame_done_notify(ctx2, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*Send the frame to ctx1*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);

  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT, ts.frame[1]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*Send the NONCE REPORT frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);

  /*Verify a ENCAP msg comes out */
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION_PROPERTIES1_EXTENSION_BIT_MASK, ts.frame[3]); /*Check that SN is included */
  TEST_ASSERT_EQUAL(18, ts.frame[4]);
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_CRITICAL_FLAG | S2_MSG_EXTHDR_TYPE_SN,
      ts.frame[5]);

  S2_send_frame_done_notify(ctx2, S2_TRANSMIT_COMPLETE_OK,0x42);


  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);

  /* verify that ctx1 gets the message */
  TEST_ASSERT_EQUAL(sizeof(hello), ts.rx_frame_len); //Verify that the frame has been received
  TEST_ASSERT_EQUAL_STRING_LEN(hello, ts.rx_frame, sizeof(hello));
  TEST_ASSERT_EQUAL(1, ts.sync_ev.count); // Test that no further sync events was emitted during test
}


/**Verify that message is ignored if MAC is invalid
 */

void test_encap_message_authcheck()
{
  test_s2_send_data();
  /* ----------- now send second frame with SPAN established --------- *
   */

  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;

  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*Break the mac */
  ts.frame[ts.frame_len-3] = 0x1;
  ts.frame[ts.frame_len-2] = 0x2;
  ts.frame[ts.frame_len-1] = 0x3;
  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);
  TEST_ASSERT_EQUAL(0, ts.rx_frame_len);
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}

/* Verify we dont go into an infinite loop when parsing a zero-length header extension
 * and more-to-follow is set.*/
void test_hdr_ext_zero_length_and_more_to_follow()
{
  test_s2_send_data();
  /* ----------- now send second frame with SPAN established --------- *
   */

  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;

  //uint8_t strange_frame[] = {0x9F, 0x03 , 0x17 , 0x01 , 0x02 , 0xBF , 0x00 , 0x89, 0xE , 0xDD , 0x69 , 0xFC , 0x92 , 0xA0 , 0xA5 , 0x09 , 0xF0 , 0xF4};
  uint8_t strange_frame[] = {0x9F, 0x03 , 0x17 , 0x01 , 0x02 , 0xBF , 0x00 , 0x89, 0 , 0 , 0 , 0, 0, 0, 0};

  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, strange_frame, sizeof(strange_frame));
  TEST_ASSERT_EQUAL(0, ts.rx_frame_len);
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}

/* Verify we dont go into an infinite loop when parsing a zero-length header extension
 * and more-to-follow is set.*/
void test_hdr_ext_zero_length_and_more_to_follow2()
{
  test_s2_send_data();
  /* ----------- now send second frame with SPAN established --------- *
   */

  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;

  uint8_t strange_frame[] = {0x9F, 0x03 , 0x17 , 0x01 , 0x02 , 0xBF , 0x00 , 0x89, 0xE , 0xDD , 0x69 , 0xFC , 0x92 , 0xA0 , 0xA5 , 0x09 , 0xF0 , 0xF4};

  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, strange_frame, sizeof(strange_frame));
  TEST_ASSERT_EQUAL(0, ts.rx_frame_len);
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}

void test_encap_message_oversize()
{
  /*send message which is 1281 byes */
  /* verify rejecting by transport layer */

  /*construct message which is 1281 bytes long */
  /* verify that the message is ignored by receiver*/
  TEST_IGNORE_MESSAGE("Implement me");
 ;
}

void test_encap_message_lencheck()
{
  /* loop
   *   establish span
   *   send frame frame cropped from 0.. len bytes
   *   verify that message is ignored
   */
  TEST_IGNORE_MESSAGE("Implement me");
  ;
}

void test_encap_message_extcheck()
{
  /*construct message with two extensions which are non critical. One of them must be 255 bytes in len
   * verify the message is decrypted
   */
  TEST_IGNORE_MESSAGE("Implement me");
  ;
}

void test_encap_message_critical_extcheck()
{
  /* construct message with two extensions one are non critical and one is critical, and unknown.
   verify the message is ignored
   */
  TEST_IGNORE_MESSAGE("Implement me");
  ;
}

void test_encap_message_extlen()
{
  /* construct message with an extension of len 0.
   * verify that this message is ignored.
   */
  TEST_IGNORE_MESSAGE("Implement me");
  ;
}


void test_m_enacap_send()
{
  /* generate MPAN for nodes 2,3,4 */
  /* send secure single-cast message for node 2 and 3 (SPAN is now established) */
  /* push MPAN to node 2, out of band hack */

  /* enc_send_mcast to nodes 2,3,4*/
  /* verify that all three nodes got the message after transmission has completed. */

  /* enc_send_mcast to nodes 2,3,4*/
  /* verify that all three nodes got the message after transmission has completed but this time
   * after sending fewer frames. */

  TEST_IGNORE_MESSAGE("Implement me");
  ;
}

/**
 * Test that we can use single frame transmissions once SPAN
 * has been established.
 *
 */
void test_commands_supported_report()
{
  const uint8_t commands_supported_get[] = {COMMAND_CLASS_SECURITY_2,SECURITY_2_COMMANDS_SUPPORTED_GET};

  test_s2_send_data();
  /* ----------- now send second frame with SPAN established --------- *
   */

  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;

  S2_send_data(ctx1, &conn12, (uint8_t*) commands_supported_get, sizeof(commands_supported_get));
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  //TEST_ASSERT_EQUAL(ts.frame[2],COMMAND_CLASS_SECURITY_2); //seq
  TEST_ASSERT_EQUAL(0, ts.frame[3]); //Verify that there is no SN included
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);

  S2_send_frame_done_notify(ctx2, S2_TRANSMIT_COMPLETE_OK,0x42);

  TEST_ASSERT_EQUAL(2, ts.fcount); //encap commands supported get + encap commands supported report
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);


  /*Expect ctx2 to reply with command supported report*/

  TEST_ASSERT_EQUAL(sizeof(commands_supported_report), ts.rx_frame_len);
  TEST_ASSERT_EQUAL_MEMORY( commands_supported_report, ts.rx_frame, sizeof(commands_supported_report));

  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}

/**
 * Check that we receive S2_TRANSMIT_COMPLETE_OK if we are sending with verify,
 * if the receiving controller is able to decrypt the message
 */
void test_sd_verify_tx_ok() {

  const char hello[] = "HelloWorld Second Frame";

  test_s2_send_data();
  /* ----------- now send second frame with SPAN established --------- *
   */

  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;
  ts.s2_send_status = S2_TRANSMIT_COMPLETE_NO_ACK;

  conn12.tx_options = S2_TXOPTION_VERIFY_DELIVERY;

  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  //TEST_ASSERT_EQUAL(ts.frame[2],COMMAND_CLASS_SECURITY_2); //seq
  TEST_ASSERT_EQUAL(0, ts.frame[3]); //Verify that there is no SN included

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);

  TEST_ASSERT_EQUAL(1, ts.fcount);
  TEST_ASSERT_EQUAL(sizeof(hello), ts.rx_frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(hello, ts.rx_frame, sizeof(hello));


  TEST_ASSERT_NOT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status);
  S2_timeout_notify(ctx1);
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status);
  TEST_ASSERT_FALSE(S2_is_busy(ctx1));
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}




/**
 * Check that we receive S2_TRANSMIT_COMPLETE_NO_ACK if senddata returns false in fist transmission
 * Related to ZGW-XXXX
 */
void test_sd_verify_tx_sd_false() {

  const char hello[] = "HelloWorld Second Frame";

  test_s2_send_data();
  /* ----------- now send second frame with SPAN established --------- */

  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;
  ts.s2_send_status = 42; /* Init to invalid status code, so we can verify that S2_send_data changes it */

  conn12.tx_options = S2_TXOPTION_VERIFY_DELIVERY;

  ts.send_data_return_fail = 1;

  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_FAIL, ts.s2_send_status);
  TEST_ASSERT_FALSE(S2_is_busy(ctx1));
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}




/**
 * Check that we receive S2_TRANSMIT_COMPLETE_FAIL if send data returns false
 * when trying to do nonce sync in verify delivery.
 */
void test_sd_verify_tx_sd_false_second_attempt() {
  const char hello[] = "HelloWorld Second Frame";
  uint8_t bad_frame[1500];
  test_s2_send_data();
  /* ----------- now send second frame with SPAN established --------- *
   */

  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;
  ts.s2_send_status = S2_TRANSMIT_COMPLETE_NO_ACK;

  conn12.tx_options = S2_TXOPTION_VERIFY_DELIVERY;

  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(0, ts.frame[3]); //Verify that there is no SN included

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*make a bad frame */
  memcpy(bad_frame,ts.frame,ts.frame_len);
  bad_frame[ts.frame_len-1]++; // *this* will invalidate the MAC

  /*Send the BAD frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, bad_frame, ts.frame_len);

  /*We expect ctx2 to send a nonce report */
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT, ts.frame[1]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);

  ts.send_data_return_fail=1;
  /*Make ctx1 hear the nocne report*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);

  /*Verify that the callback only comes after the timeout */
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_FAIL, ts.s2_send_status);

  //Make sure we are not stuck
  TEST_ASSERT_FALSE(S2_is_busy(ctx1));
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}


/**
 * Check that we receive S2_TRANSMIT_COMPLETE_OK if we are sending with verify,
 * if the receiving controller is able to decrypt the message after the
 * second attempt
 */
void test_sd_verify_tx_ok_second_attempt() {
  const char hello[] = "HelloWorld Second Frame";
  uint8_t bad_frame[1500];
  test_s2_send_data();
  /* ----------- now send second frame with SPAN established --------- *
   */

  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;
  ts.s2_send_status = S2_TRANSMIT_COMPLETE_NO_ACK;

  conn12.tx_options = S2_TXOPTION_VERIFY_DELIVERY;

  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(0, ts.frame[3]); //Verify that there is no SN included

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*make a bad frame */
  memcpy(bad_frame,ts.frame,ts.frame_len);
  bad_frame[ts.frame_len-1]++; // *this* will invalidate the MAC

  /*Send the BAD frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, bad_frame, ts.frame_len);

  /*We expect ctx2 to send a nonce report */
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT, ts.frame[1]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);

  /*Make ctx1 hear the nocne report*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);

  /* ctx1 should now send a MSG,SN */
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION_PROPERTIES1_EXTENSION_BIT_MASK, ts.frame[3]);
  TEST_ASSERT_EQUAL(18, ts.frame[4]);
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_CRITICAL_FLAG | S2_MSG_EXTHDR_TYPE_SN,
      ts.frame[5]);

  /*Make ctx1 hear the MSG,SN*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);

  /*Checke that Ctx2 did hear the frame*/
  TEST_ASSERT_EQUAL(sizeof(hello), ts.rx_frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(hello, ts.rx_frame, sizeof(hello));

  /*Verify that the callback only comes after the timeout */
  TEST_ASSERT_NOT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status);
  S2_timeout_notify(ctx1);
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status);
  TEST_ASSERT_FALSE(S2_is_busy(ctx1));
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}


/**
 * Check that we receive S2_TRANSMIT_COMPLETE_FIAL if we are sending with verify,
 * if the receiving controller is not able to decrypt, and sends a NONCE_REPORT -
 * after both first and second attempt.
 */
void test_sd_verify_tx_fail() {
  const char hello[] = "HelloWorld Second Frame";
  uint8_t bad_frame[1500];
  test_s2_send_data();
  /* ----------- now send second frame with SPAN established --------- *
   */

  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;
  ts.s2_send_status = S2_TRANSMIT_COMPLETE_NO_ACK;

  conn12.tx_options = S2_TXOPTION_VERIFY_DELIVERY;

  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(0, ts.frame[3]); //Verify that there is no SN included

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*make a bad frame */
  memcpy(bad_frame,ts.frame,ts.frame_len);
  bad_frame[ts.frame_len-1]++; // *this* will invalidate the MAC

  /*Send the BAD frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, bad_frame, ts.frame_len);

  /*We expect ctx2 to send a nonce report */
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT, ts.frame[1]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);

  /*Make ctx1 hear the nocne report*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);

  /* ctx1 should now send a MSG,SN */
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION_PROPERTIES1_EXTENSION_BIT_MASK, ts.frame[3]);
  TEST_ASSERT_EQUAL(18, ts.frame[4]);
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_CRITICAL_FLAG | S2_MSG_EXTHDR_TYPE_SN,
      ts.frame[5]);

  /*make a bad frame */
  memcpy(bad_frame,ts.frame,ts.frame_len);
  bad_frame[ts.frame_len-1]++; // *this* will invalidate the MAC

  /*Make ctx1 hear the bad frame*/
  S2_application_command_handler(ctx2, &ts.last_trans, bad_frame, ts.frame_len);

  /*We expect ctx2 to send a nonce report */
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT, ts.frame[1]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);


  /*Verify that the callback only comes after the second frame */
  TEST_ASSERT_NOT_EQUAL(S2_TRANSMIT_COMPLETE_FAIL, ts.s2_send_status);

  /*Make ctx1 hear the nocne report*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);

  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_FAIL, ts.s2_send_status);
  TEST_ASSERT_FALSE(S2_is_busy(ctx1));
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}



/**
 * Check that we receive S2_TRANSMIT_COMPLETE_VERIFIED if we are sending with verify,
 * if the receiving controller is able to decrypt the message,and sends a reply
 */
void test_sd_verify_tx_verifify() {
  const char ping[] = "PING";
  const char pong[] = "PONG";

  test_s2_send_data();
  /* ----------- now send second frame with SPAN established --------- *
   */

  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;
  ts.s2_send_status = S2_TRANSMIT_COMPLETE_NO_ACK;

  conn12.tx_options = S2_TXOPTION_VERIFY_DELIVERY;

  /* We just do this to make sure that the callback of ctx2 does not overwrite
   * the callback we are looking for.
    */
  conn21.tx_options = S2_TXOPTION_VERIFY_DELIVERY;

  S2_send_data(ctx1, &conn12, (uint8_t*) ping, sizeof(ping));
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(0, ts.frame[3]); //Verify that there is no SN included

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);

  TEST_ASSERT_EQUAL(1, ts.fcount);
  TEST_ASSERT_EQUAL(sizeof(ping), ts.rx_frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(ping, ts.rx_frame, sizeof(ping));

  S2_send_data(ctx2, &conn21, (uint8_t*) pong, sizeof(pong));
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(0, ts.frame[3]); //Verify that there is no SN included

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  TEST_ASSERT_NOT_EQUAL(S2_TRANSMIT_COMPLETE_VERIFIED, ts.s2_send_status);

  /*Send the frame to ctx1*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);

  TEST_ASSERT_EQUAL(2, ts.fcount);
  TEST_ASSERT_EQUAL(sizeof(pong), ts.rx_frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(pong, ts.rx_frame, sizeof(pong));

  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_VERIFIED, ts.s2_send_status);
  TEST_ASSERT_FALSE(S2_is_busy(ctx1));
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}


/**
 * Check that we receive S2_TRANSMIT_COMPLETE_VERIFIED if we are sending with verify,
 * if the receiving controller is able to decrypt the message after the
 * second attempt, and sends a reply message
 */
void test_sd_verify_tx_verify_second_attempt() {
  const char ping[] = "PING";
  const char pong[] = "PONG";

  uint8_t bad_frame[1500];
  test_s2_send_data();
  /* ----------- now send second frame with SPAN established --------- *
   */

  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;
  ts.s2_send_status = S2_TRANSMIT_COMPLETE_NO_ACK;

  conn12.tx_options = S2_TXOPTION_VERIFY_DELIVERY;
  /* We just do this to make sure that the callback of ctx2 does not overwrite
   * the callback we are looking for.
    */
  conn21.tx_options = S2_TXOPTION_VERIFY_DELIVERY;


  S2_send_data(ctx1, &conn12, (uint8_t*) ping, sizeof(ping));
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(0, ts.frame[3]); //Verify that there is no SN included

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*make a bad frame */
  memcpy(bad_frame,ts.frame,ts.frame_len);
  bad_frame[ts.frame_len-1]++; // *this* will invalidate the MAC

  /*Send the BAD frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, bad_frame, ts.frame_len);

  /*We expect ctx2 to send a nonce report */
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT, ts.frame[1]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);

  /*Make ctx1 hear the nocne report*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);

  /* ctx1 should now send a MSG,SN */
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION_PROPERTIES1_EXTENSION_BIT_MASK, ts.frame[3]);
  TEST_ASSERT_EQUAL(18, ts.frame[4]);
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_CRITICAL_FLAG | S2_MSG_EXTHDR_TYPE_SN,
      ts.frame[5]);

  /*Make ctx1 hear the MSG,SN*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);

  /*Checke that Ctx2 did hear the frame*/
  TEST_ASSERT_EQUAL(sizeof(ping), ts.rx_frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(ping, ts.rx_frame, sizeof(ping));

  /*Verify that the callback only comes after the reply */
  TEST_ASSERT_NOT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status);

  /*Now make ctx2 send reply*/

  S2_send_data(ctx2, &conn21, (uint8_t*) pong, sizeof(pong));
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(0, ts.frame[3]); //Verify that there is no SN included

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  TEST_ASSERT_NOT_EQUAL(S2_TRANSMIT_COMPLETE_VERIFIED, ts.s2_send_status);

  /*Send the frame to ctx1*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);

  TEST_ASSERT_EQUAL(4, ts.fcount);
  TEST_ASSERT_EQUAL(sizeof(pong), ts.rx_frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(pong, ts.rx_frame, sizeof(pong));

  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_VERIFIED, ts.s2_send_status);
  TEST_ASSERT_FALSE(S2_is_busy(ctx1));
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}

/**
 * Do a singlecast followup, where SPAN is ok, but node b is MOS
 */
void singlecast_followup_mos(struct S2* ctx_a, struct S2* ctx_b, s2_connection_t* conn) {
  char hello_mc[] = "Hello multicast";

  ts.fcount = 0;
  ts.s2_send_done=0;
  /* Send the single cast frame */
  S2_send_data(ctx_a, conn, (uint8_t*) hello_mc, sizeof(hello_mc));
  S2_send_frame_done_notify(ctx_a, S2_TRANSMIT_COMPLETE_OK,0x42);

  S2_application_command_handler(ctx_b, &ts.last_trans, ts.frame, ts.frame_len);
  S2_timeout_notify(ctx_b);

  /* Verify that ctx2 sends a NONCE_REPORT,MOS */
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT, ts.frame[1]);
  TEST_ASSERT_EQUAL(2, ts.frame[3]);
  TEST_ASSERT_EQUAL(4, ts.frame_len);

  S2_send_frame_done_notify(ctx_b, S2_TRANSMIT_COMPLETE_OK,0x42);

  /* Deliver NR,MOS to ctx 1*/
  S2_application_command_handler(ctx_a, &ts.last_trans, ts.frame, ts.frame_len);

  /* The CTX1 sends a MSG,MPAN */
  TEST_ASSERT_EQUAL(3, ts.fcount);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(2, ts.frame[3]); //Verify that there is a ENC, EXT included
  TEST_ASSERT_EQUAL(4 + 19 + 8, ts.frame_len);

  S2_send_frame_done_notify(ctx_a, S2_TRANSMIT_COMPLETE_OK,0x42);
  S2_timeout_notify(ctx_a);
  TEST_ASSERT_EQUAL(1, ts.s2_send_done);
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status);

  /* Deliver MSG,MPAN to ctx 2*/
  S2_application_command_handler(ctx_b, &ts.last_trans, ts.frame, ts.frame_len);
}

/**
 * Do a singlecast followup, where SPAN is ok, but node b is MOS, but
 * here its a supervision get, and receiver will send a report
 */
void supervision_singlecast_followup_mos(struct S2* ctx_a, struct S2* ctx_b, s2_connection_t* conn) {
  char supervision_get[] = "Supervision Get";
  char supervision_report[] = "Supervision Report";

  ts.fcount = 0;
  ts.s2_send_done=0;
  /* Send the single cast frame */
  S2_send_data(ctx_a, conn, (uint8_t*) supervision_get, sizeof(supervision_get));
  S2_send_frame_done_notify(ctx_a, S2_TRANSMIT_COMPLETE_OK,0x42);

  S2_application_command_handler(ctx_b, &ts.last_trans, ts.frame, ts.frame_len);

  /*Reply with supervison report */
  S2_send_data(ctx_b, &ts.last_trans, (uint8_t*) supervision_report, sizeof(supervision_report));
  S2_send_frame_done_notify(ctx_b, S2_TRANSMIT_COMPLETE_OK,0x42);

  /* Verify that ctx2 sends a message with the,MOS extension */
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(1, ts.frame[3]); //Verify that there is a ENC, EXT included
  TEST_ASSERT_EQUAL(2, ts.frame[4]); //length 2
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_TYPE_MOS, ts.frame[5]); //length 2

  S2_send_frame_done_notify(ctx_b, S2_TRANSMIT_COMPLETE_OK,0x42);
  S2_application_command_handler(ctx_a, &ts.last_trans, ts.frame, ts.frame_len);

  /* The CTX1 sends a MSG,MPAN */
  TEST_ASSERT_EQUAL(3, ts.fcount);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(2, ts.frame[3]); //Verify that there is a ENC, EXT included
  TEST_ASSERT_EQUAL(4 + 19 + 8, ts.frame_len);

  S2_send_frame_done_notify(ctx_a, S2_TRANSMIT_COMPLETE_OK,0x42);
  S2_timeout_notify(ctx_a);
  TEST_ASSERT_EQUAL(2, ts.s2_send_done); //both ctx_a and ctx_b have been sending a message
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status);

  /* Deliver MSG,MPAN to ctx 2*/
  S2_application_command_handler(ctx_b, &ts.last_trans, ts.frame, ts.frame_len);
}

/*
 * Establish MPAN sync between ctx1 and ctx2, its ctx1 which is sending
 *
 * 2) Send multicast, verify that no frames are transmitted by the source node
 * 3) Send singlecast frame, verify that the node answers NR,MOS
 * 4) Verify that sender sends EMSG,MPAN
 *
 */
void
establish_mpan(uint8_t group)
{
  uint8_t buf[1024];
  char hello_mc[] = "Hello multicast";
  s2_connection_t mc_group =
    { 1, 0x42 };
  s2_connection_t mc_group_rx =
    { 0x42, 1, 0, S2_RXOPTION_MULTICAST };

  mc_group.class_id = 2;
  mc_group.r_node = group;
  mc_group_rx.l_node = group;

  /* Send the first multicast message */

  S2_send_data_multicast(ctx1, &mc_group, (uint8_t*) hello_mc,
      sizeof(hello_mc));

  TEST_ASSERT_EQUAL(ts.fcount, 1);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(1, ts.frame[3]); //Verify that there is a MC group included
  TEST_ASSERT_EQUAL(3, ts.frame[4]);
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_CRITICAL_FLAG|S2_MSG_EXTHDR_TYPE_MGRP,
      ts.frame[5]);
  TEST_ASSERT_EQUAL(mc_group.r_node, ts.frame[6]);

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
  TEST_ASSERT_EQUAL(1, ts.s2_send_done);
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status);

  memcpy(buf,ts.frame,ts.frame_len);
  S2_application_command_handler(ctx2, &mc_group_rx, ts.frame, ts.frame_len);
  S2_application_command_handler(ctx3, &mc_group_rx, buf, ts.frame_len);


  /* Of ctx 2 */
  conn12.rx_options = 0;
  conn12.tx_options = S2_TXOPTION_VERIFY_DELIVERY | S2_TXOPTION_SINGLECAST_FOLLOWUP | S2_TXOPTION_FIRST_SINGLECAST_FOLLOWUP;

  supervision_singlecast_followup_mos(ctx1,ctx2,&conn12);

  conn13.rx_options = 0;
  conn13.tx_options = S2_TXOPTION_VERIFY_DELIVERY | S2_TXOPTION_SINGLECAST_FOLLOWUP;

  singlecast_followup_mos(ctx1,ctx3,&conn13);
}

/**
 * Send a multicast frame to a node
 *
 * 1) Establish span
 * 2) Send multicast to node 2 and 3, verify that no frames are transmitted by the source node
 * 3) Send singlecast frame, verify that the node answers NR,MOS
 * 4) Verify that sender sends EMSG,MPAN
 * 5) Send multicast again, verify that the message is received by destination
 * 6) Send singlecast, verify that we do not get NR,MOS
 */
void test_mulicast_send()
{
  uint8_t buf[1024];
  char hello_mc2[] = "Jello multicast 2";
  char hello_single[] = "hello singlecast";
  int i;
  int group_id = 2;
  s2_connection_t mc_group =
    { 1, group_id };
  s2_connection_t mc_group_rx =
    { group_id, 1, 0, S2_RXOPTION_MULTICAST };

  conn12.class_id=2;
  conn13.class_id=2;
  mc_group.class_id=2;
  /*Establish span*/
  //test_s2_send_data();
  my_setup();
  wrap_test_s2_send_data(ctx2,&conn12);
  wrap_test_s2_send_data(ctx3,&conn13);

  ts.fcount = 0;
  ts.s2_send_done = 0;
  ts.s2_send_status = S2_TRANSMIT_COMPLETE_NO_ACK;

  establish_mpan(group_id);


  for(i=0; i < 10; i++) {
    int j;
    uint8_t retval=0;

    printf("Round %i.....\n",i);

    /*Send 3 multicast frames */
    for(j=0; j<3; j++) {
      /*Send a new multicast and verify that ctx2 is able to decrypt the frame*/
      retval = S2_send_data_multicast(ctx1, &mc_group, (uint8_t*) hello_mc2,
				      sizeof(hello_mc2));
      TEST_ASSERT_EQUAL(retval,1);
      /* Negative test: we cannot send twice */
      retval = S2_send_data_multicast(ctx1, &mc_group, (uint8_t*) hello_mc2,
				      sizeof(hello_mc2));
      TEST_ASSERT_EQUAL(retval,0);
      /* Negative test: we cannot send nothing */
      retval = S2_send_data_multicast(ctx1, &mc_group, (uint8_t*) NULL,
				      sizeof(hello_mc2));
      /* Negative test: we cannot send no data */
      TEST_ASSERT_EQUAL(retval,0);
      retval = S2_send_data_multicast(ctx1, &mc_group, (uint8_t*) hello_mc2,
				      0);
      TEST_ASSERT_EQUAL(retval,0);
      /* Back to the legal path */
      S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

      memcpy(buf,ts.frame,ts.frame_len);

      memset(ts.rx_frame,0,sizeof(ts.rx_frame));
      ts.rx_frame_len=0;
      S2_application_command_handler(ctx2, &mc_group_rx, ts.frame, ts.frame_len);
      TEST_ASSERT_EQUAL_STRING(hello_mc2, ts.rx_frame);
      TEST_ASSERT_EQUAL( ts.rx_frame_len, sizeof(hello_mc2));

      memset(ts.rx_frame,0,sizeof(ts.rx_frame));
      ts.rx_frame_len = 0;
      S2_application_command_handler(ctx3, &mc_group_rx, buf, ts.frame_len);
      TEST_ASSERT_EQUAL_STRING(hello_mc2, ts.rx_frame);
      TEST_ASSERT_EQUAL( ts.rx_frame_len, sizeof(hello_mc2));
    }


    /*Verify no NR send on next single cast */
    ts.fcount = 0;

    conn12.tx_options = S2_TXOPTION_VERIFY_DELIVERY |S2_TXOPTION_SINGLECAST_FOLLOWUP | S2_TXOPTION_FIRST_SINGLECAST_FOLLOWUP;

    memset(ts.rx_frame,0,sizeof(ts.rx_frame));
    S2_send_data(ctx1,&conn12, (uint8_t*)hello_single, sizeof(hello_single));
    S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
    S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);
    TEST_ASSERT_EQUAL_STRING(hello_single, ts.rx_frame);
    TEST_ASSERT_EQUAL(1,ts.fcount);
    S2_timeout_notify(ctx1);

    conn13.tx_options = S2_TXOPTION_VERIFY_DELIVERY | S2_TXOPTION_SINGLECAST_FOLLOWUP;

    /*Verify no NR send on next single cast */
    memset(ts.rx_frame,0,sizeof(ts.rx_frame));
    S2_send_data(ctx1,&conn13, (uint8_t*)hello_single, sizeof(hello_single));
    S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
    S2_application_command_handler(ctx3, &ts.last_trans, ts.frame, ts.frame_len);
    TEST_ASSERT_EQUAL_STRING(hello_single, ts.rx_frame);
    TEST_ASSERT_EQUAL(2,ts.fcount);
    S2_timeout_notify(ctx1);
  }
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}

/*
 * Test that we are able to MOS and SOS sync in the same session.
 *
 */
void test_mulicast_send_mos_sos_sync()
{
  char hello_mc2[] = "Jello multicast 2";
  char hello_single[] = "hello singlecast";
  char hello_mc[] = "Hello multicast";
  int i;
  s2_connection_t mc_group =
    { 1, 0x42 };
  s2_connection_t mc_group_rx =
    { 0x42, 1, 0, S2_RXOPTION_MULTICAST };

  my_setup();

  ts.fcount = 0;
  ts.s2_send_done = 0;
  ts.s2_send_status = S2_TRANSMIT_COMPLETE_NO_ACK;

  mc_group.class_id=2;
  /* Send the first multicast message */

  S2_send_data_multicast(ctx1, &mc_group, (uint8_t*) hello_mc,
      sizeof(hello_mc));

  TEST_ASSERT_EQUAL(ts.fcount, 1);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(1, ts.frame[3]); //Verify that there is a MC group included
  TEST_ASSERT_EQUAL(3, ts.frame[4]);
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_CRITICAL_FLAG|S2_MSG_EXTHDR_TYPE_MGRP,
      ts.frame[5]);
  TEST_ASSERT_EQUAL(mc_group.r_node, ts.frame[6]);

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
  TEST_ASSERT_EQUAL(1, ts.s2_send_done);
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status);

  S2_application_command_handler(ctx2, &mc_group_rx, ts.frame, ts.frame_len);

  conn12.rx_options = 0;
  conn12.tx_options = S2_TXOPTION_VERIFY_DELIVERY | S2_TXOPTION_FIRST_SINGLECAST_FOLLOWUP | S2_TXOPTION_SINGLECAST_FOLLOWUP;

  /* Send the single cast frame */
  S2_send_data(ctx1, &conn12, (uint8_t*) hello_mc, sizeof(hello_mc));
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);

  /* Verify that ctx2 sends a NONCE_REPORT,SOS */
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT, ts.frame[1]);
  TEST_ASSERT_EQUAL(S2_NONCE_REPORT_SOS_FLAG, ts.frame[3]); /*SOS*/
  TEST_ASSERT_EQUAL(4+16, ts.frame_len);

  S2_send_frame_done_notify(ctx2, S2_TRANSMIT_COMPLETE_OK,0x42);

  /* Deliver NR,SOS to ctx 1*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);

  /* The CTX1 sends a MSG,MPAN */
  TEST_ASSERT_EQUAL(ts.fcount, 4);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(S2_MSG_FLAG_EXT, ts.frame[3]); //Verify that there is a non ENC EXT is included
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_MORE_FLAG, ts.frame[5] & S2_MSG_EXTHDR_MORE_FLAG); //Verify that there is a non ENC EXT is included

  TEST_ASSERT_EQUAL(4 +3+ 18+ sizeof(hello_mc) + 8, ts.frame_len); //SH=4,  MC GRP=3,  SPAN=18, PAYLOAD , AUTH=8

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  /* Deliver MSG, to ctx 2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);
  S2_timeout_notify(ctx2);

  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT, ts.frame[1]);
  TEST_ASSERT_EQUAL(S2_NONCE_REPORT_MOS_FLAG, ts.frame[3]); /*MOS*/

  /* Deliver NR, MOS to ctx 1*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);
  S2_timeout_notify(ctx2);

  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(S2_MSG_FLAG_SEXT, ts.frame[3]); //Verify that there is a ENC, SEXT included
  TEST_ASSERT_EQUAL(4 + 19 + 8 , ts.frame_len); //SH, MPAN, AUTH

  /* Deliver ENC, MPAN to ctx 2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);

  TEST_ASSERT_EQUAL(1, ts.s2_send_done);
  S2_timeout_notify(ctx1);
  TEST_ASSERT_EQUAL(2, ts.s2_send_done);
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status);

  ts.fcount = 0;
  ts.s2_send_done = 0;
  for(i =0; i < 10; i++) {

    /*Send a new multicast and verify that ctx2 is able to decrypt the frame*/
    S2_send_data_multicast(ctx1, &mc_group, (uint8_t*) hello_mc2,
        sizeof(hello_mc2));
    S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

    S2_application_command_handler(ctx2, &mc_group_rx, ts.frame, ts.frame_len);
    TEST_ASSERT_EQUAL_STRING(hello_mc2, ts.rx_frame);

    conn12.tx_options = S2_TXOPTION_VERIFY_DELIVERY | S2_TXOPTION_FIRST_SINGLECAST_FOLLOWUP | S2_TXOPTION_SINGLECAST_FOLLOWUP;

    /*Verify no NR send on next single cast */
    S2_send_data(ctx1,&conn12, (uint8_t*)hello_single, sizeof(hello_single));
    S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
    S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);

    TEST_ASSERT_EQUAL(2*i+1,ts.s2_send_done);
    S2_timeout_notify(ctx1);
    TEST_ASSERT_EQUAL(2*i+2,ts.s2_send_done);
    TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_OK,ts.s2_send_status);

    TEST_ASSERT_EQUAL_STRING(hello_single, ts.rx_frame);
    TEST_ASSERT_EQUAL((i+1)*2,ts.fcount);
  }
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}


/**
 * Send a frame with sec group 1, verify that it goes just like group 0
 */
void test_s2_send_data_second_group()
{

  const char hello2[] = "HelloWorld Second Frame";

  my_setup();

  conn12.class_id = 0;
  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));

  TEST_ASSERT_EQUAL_STRING_LEN(nonce_get, ts.frame, 2);
  TEST_ASSERT_EQUAL(3, ts.frame_len);
  TEST_ASSERT_EQUAL(1, ts.fcount);
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);
  /*Expect that we get a nonce report*/
  TEST_ASSERT_EQUAL(2, ts.fcount);
  TEST_ASSERT_EQUAL(20, ts.frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(nonce_report, ts.frame, 2);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);

  S2_send_frame_done_notify(ctx2, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*Send the frame to ctx1*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);
  TEST_ASSERT_EQUAL(3, ts.fcount);

  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  //TEST_ASSERT_EQUAL(ts.frame[2],COMMAND_CLASS_SECURITY_2); //seq
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION_PROPERTIES1_EXTENSION_BIT_MASK, ts.frame[3]);
  TEST_ASSERT_EQUAL(18, ts.frame[4]);
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_CRITICAL_FLAG | S2_MSG_EXTHDR_TYPE_SN,
      ts.frame[5]);

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  TEST_ASSERT_EQUAL(1, ts.s2_send_done); //Check we got a send done event
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status); //Check we got a send done event

  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);
  TEST_ASSERT_EQUAL(sizeof(hello), ts.rx_frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(hello, ts.rx_frame, sizeof(hello));


  /*Now verify that we can send single frames*/

  /* ----------- now send second frame with SPAN established --------- *
   */

  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;

  S2_send_data(ctx1, &conn12, (uint8_t*) hello2, sizeof(hello2));
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(0, ts.frame[3]); //Verify that there is no SN included
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);

  TEST_ASSERT_EQUAL(1, ts.fcount);
  TEST_ASSERT_EQUAL(sizeof(hello2), ts.rx_frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(hello2, ts.rx_frame, sizeof(hello2));


  /*Now change group again this should look exactly like the first transmission*/
  ts.fcount = 0; //Reset frame count
  ts.rx_frame_len = 0;
  ts.s2_send_done =0;

  conn21.class_id = 2;
  S2_send_data(ctx2, &conn21, (uint8_t*) hello, sizeof(hello));

  TEST_ASSERT_EQUAL_STRING_LEN(nonce_get, ts.frame, 2);
  TEST_ASSERT_EQUAL(3, ts.frame_len);
  TEST_ASSERT_EQUAL(1, ts.fcount);
  S2_send_frame_done_notify(ctx2, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);
  /*Expect that we get a nonce report*/
  TEST_ASSERT_EQUAL(2, ts.fcount);
  TEST_ASSERT_EQUAL(20, ts.frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(nonce_report, ts.frame, 2);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*Send the frame to ctx1*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);
  TEST_ASSERT_EQUAL(3, ts.fcount);

  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  //TEST_ASSERT_EQUAL(ts.frame[2],COMMAND_CLASS_SECURITY_2); //seq
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION_PROPERTIES1_EXTENSION_BIT_MASK, ts.frame[3]);
  TEST_ASSERT_EQUAL(18, ts.frame[4]);
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_CRITICAL_FLAG | S2_MSG_EXTHDR_TYPE_SN,
      ts.frame[5]);

  S2_send_frame_done_notify(ctx2, S2_TRANSMIT_COMPLETE_OK,0x42);

  TEST_ASSERT_EQUAL(1, ts.s2_send_done); //Check we got a send done event
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status); //Check we got a send done event

  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);
  TEST_ASSERT_EQUAL(sizeof(hello), ts.rx_frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(hello, ts.rx_frame, sizeof(hello));
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}

/**
 * Send a frame to group0, then group 1, then use the follow-up API to try to
 * follow-up on group 0
 */
void test_s2_send_data_multicast_and_follow_up_with_keyset()
{
  char hello_mc2[] = "Hello multicast 2";
  char hello_mc[] = "Hello multicast";
  s2_connection_t mc_group1 =
    { 1, 0x42 };
  s2_connection_t mc_group_rx1 =
    { 0x42, 1, 0, S2_RXOPTION_MULTICAST };
  s2_connection_t mc_group2 =
    { 1, 0x47 };
  s2_connection_t mc_group_rx2 =
    { 0x47, 1, 0, S2_RXOPTION_MULTICAST };
  my_setup();

  ts.fcount = 0;
  ts.s2_send_done = 0;
  ts.s2_send_status = S2_TRANSMIT_COMPLETE_NO_ACK;

  mc_group1.class_id=1;

  /* Send the first multicast message, group 0x42 */
  S2_send_data_multicast_with_keyset(ctx1, &mc_group1, ZWAVE_LONG_RANGE_KEYSET, (uint8_t*) hello_mc,
      sizeof(hello_mc));

  TEST_ASSERT_EQUAL(ts.fcount, 1);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(1, ts.frame[3]); //Verify that there is a MC group included
  TEST_ASSERT_EQUAL(3, ts.frame[4]);
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_CRITICAL_FLAG|S2_MSG_EXTHDR_TYPE_MGRP,
      ts.frame[5]);
  TEST_ASSERT_EQUAL(mc_group1.r_node, ts.frame[6]);
  TEST_ASSERT_EQUAL(mc_group1.class_id, 3);

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK, 0x42);
  TEST_ASSERT_EQUAL(1, ts.s2_send_done);
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status);

  S2_application_command_handler(ctx1, &mc_group_rx1, ts.frame, ts.frame_len);

  // Try with an unknown keyset. That should get rejected.
  S2_send_data_multicast_with_keyset(ctx1, &mc_group2, UNKNOWN_KEYSET, (uint8_t*) hello_mc2,
      sizeof(hello_mc2));
  TEST_ASSERT_EQUAL(ts.fcount, 1);

  /* Send the second multicast message, group 0x47 */
  mc_group2.class_id=1;
  S2_send_data_multicast_with_keyset(ctx1, &mc_group2, ZWAVE_KEYSET, (uint8_t*) hello_mc2,
      sizeof(hello_mc2));

  TEST_ASSERT_EQUAL(ts.fcount, 2);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  TEST_ASSERT_EQUAL(1, ts.frame[3]); //Verify that there is a MC group included
  TEST_ASSERT_EQUAL(3, ts.frame[4]);
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_CRITICAL_FLAG|S2_MSG_EXTHDR_TYPE_MGRP,
      ts.frame[5]);
  TEST_ASSERT_EQUAL(mc_group2.r_node, ts.frame[6]);
  TEST_ASSERT_EQUAL(mc_group2.class_id, 1);

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK, 0x47);
  TEST_ASSERT_EQUAL(2, ts.s2_send_done);
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status);

  S2_application_command_handler(ctx1, &mc_group_rx2, ts.frame, ts.frame_len);

  /* Send the single cast frame, to the first group, i.e. ID 42. */
  conn12.mgrp_group_id = 0x42;
  conn12.rx_options = 0;
  conn12.class_id = 2;
  conn12.tx_options = S2_TXOPTION_VERIFY_DELIVERY | S2_TXOPTION_FIRST_SINGLECAST_FOLLOWUP | S2_TXOPTION_SINGLECAST_FOLLOWUP;
  S2_send_data_singlecast_follow_up_with_keyset(ctx1, &conn12, ZWAVE_LONG_RANGE_KEYSET,(uint8_t*) hello_mc, sizeof(hello_mc));
  TEST_ASSERT_EQUAL(ts.fcount, 3);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_GET, ts.frame[1]);
  TEST_ASSERT_EQUAL(ctx1->mpan->group_id, 0x42); // Did the context change to the right MGRP ID ?
  TEST_ASSERT_EQUAL(ctx1->peer.class_id, 4); // Did the Keyset select the right adjusted class id 2->4?
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK, 0x47);
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);

  // Try with an unknown keyset
  TEST_ASSERT_EQUAL(ts.fcount, 4);
  S2_send_data_singlecast_follow_up_with_keyset(ctx1, &conn12, UNKNOWN_KEYSET, (uint8_t*) hello_mc, sizeof(hello_mc));
  TEST_ASSERT_EQUAL(ts.fcount, 4);

  // Abort the previous transmission and test the follow-up with Z-Wave keyset
  ctx1->fsm = IDLE;
  conn12.tx_options = S2_TXOPTION_VERIFY_DELIVERY | S2_TXOPTION_SINGLECAST_FOLLOWUP;
  conn12.class_id = 20;
  S2_send_data_singlecast_follow_up_with_keyset(ctx1, &conn12, ZWAVE_KEYSET,(uint8_t*) hello_mc, sizeof(hello_mc));
  TEST_ASSERT_EQUAL(ctx1->peer.class_id, 20); // Did the Keyset select the right adjusted class id 2->4?
  TEST_ASSERT_EQUAL(ts.fcount, 5);

  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}

void test_s2_send_data_with_keyset()
{
  my_setup();
  ts.fcount = 0;
  ts.s2_send_done =0;
  // Should not accept any unknown keysets
  S2_send_data_singlecast_with_keyset(ctx1, &conn12, UNKNOWN_KEYSET, (uint8_t*) hello, sizeof(hello));
  TEST_ASSERT_EQUAL(0, ts.fcount);

  // Accept and do not modify the class id for the Z-Wave keyset
  conn12.class_id = 1;
  S2_send_data_singlecast_with_keyset(ctx1, &conn12, ZWAVE_KEYSET, (uint8_t*) hello, sizeof(hello));
  TEST_ASSERT_EQUAL(1, ts.fcount);
  TEST_ASSERT_EQUAL(1, conn12.class_id);


  // Abort this previous transmission, go back to idle
  ctx1->fsm = IDLE;
  // Accept and modify the class id for the Z-Wave Long Range keyset
  conn12.class_id = 1;
  S2_send_data_singlecast_with_keyset(ctx1, &conn12, ZWAVE_LONG_RANGE_KEYSET, (uint8_t*) hello, sizeof(hello));
  TEST_ASSERT_EQUAL(2, ts.fcount);
  TEST_ASSERT_EQUAL(3, conn12.class_id);
}


void test_s2_free_mpan()
{
  char hello_mc[] = "Test free MPAN";
  s2_connection_t mc_group1 =
    { 1, 0x42 };

  my_setup();

  ts.fcount = 0;
  ts.s2_send_done = 0;
  ts.s2_send_status = S2_TRANSMIT_COMPLETE_NO_ACK;

  mc_group1.class_id=2;

  /* Send the first multicast message, group 0x42 */
  S2_send_data_multicast(ctx1, &mc_group1, (uint8_t*) hello_mc,
      sizeof(hello_mc));

  // MPAN should have been set up
  TEST_ASSERT_EQUAL(0x42,ctx1->mpan->group_id);
  TEST_ASSERT_EQUAL(MPAN_SET,ctx1->mpan->state);

  S2_free_mpan(ctx1,0,0x42);
  TEST_ASSERT_EQUAL(MPAN_NOT_USED,ctx1->mpan->state);
}



/**
 * Check that the sequence number check in verify delivery works
 */
void test_s2_verify_delivery_seq_no_check()
{
  my_setup();
  uint8_t dup_nr[64];
  struct S2* ctx_dest = ctx2;
  s2_connection_t* dst = &conn12;
  s2_connection_t nr_conn;
  ts.fcount = 0;
  ts.s2_send_done =0;

  dst->tx_options = S2_TXOPTION_VERIFY_DELIVERY;

  S2_send_data(ctx1, dst, (uint8_t*) hello, sizeof(hello));

  TEST_ASSERT_EQUAL_STRING_LEN(nonce_get, ts.frame, 2);
  TEST_ASSERT_EQUAL(3, ts.frame_len);
  TEST_ASSERT_EQUAL(1, ts.fcount);
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx_dest, &ts.last_trans, ts.frame, ts.frame_len);
  /*Expect that we get a nonce report*/
  TEST_ASSERT_EQUAL(2, ts.fcount);
  TEST_ASSERT_EQUAL(20, ts.frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(nonce_report, ts.frame, 2);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);

  /*Remember the nonce report */
  memcpy(dup_nr,ts.frame,ts.frame_len);
  nr_conn= ts.last_trans;
  S2_send_frame_done_notify(ctx_dest, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*Send the frame to ctx1*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);
  TEST_ASSERT_EQUAL(3, ts.fcount);

  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  //TEST_ASSERT_EQUAL(ts.frame[2],COMMAND_CLASS_SECURITY_2); //seq
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION_PROPERTIES1_EXTENSION_BIT_MASK, ts.frame[3]);
  TEST_ASSERT_EQUAL(18, ts.frame[4]);
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_CRITICAL_FLAG | S2_MSG_EXTHDR_TYPE_SN,
      ts.frame[5]);

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);


  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx_dest, &ts.last_trans, ts.frame, ts.frame_len);
  TEST_ASSERT_EQUAL(sizeof(hello), ts.rx_frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(hello, ts.rx_frame, sizeof(hello));

  /*Send retransmitted nonce report to ctx1 */
  S2_application_command_handler(ctx1, &nr_conn, dup_nr, 20);

  S2_timeout_notify(ctx1);

  TEST_ASSERT_EQUAL(1, ts.s2_send_done); //Check we got a send done event
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status); //Check we got a send done event
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}

void test_s2_exploit()
{
  my_setup();


  ts.fcount = 0;
  ts.s2_send_done =0;
  S2_send_data(ctx1, &conn12, (uint8_t*) hello, sizeof(hello));

  TEST_ASSERT_EQUAL_STRING_LEN(nonce_get, ts.frame, 2);
  TEST_ASSERT_EQUAL(3, ts.frame_len);
  TEST_ASSERT_EQUAL(1, ts.fcount);
  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);
  /*Expect that we get a nonce report*/
  TEST_ASSERT_EQUAL(2, ts.fcount);
  TEST_ASSERT_EQUAL(20, ts.frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(nonce_report, ts.frame, 2);
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);

  S2_send_frame_done_notify(ctx2, S2_TRANSMIT_COMPLETE_OK,0x42);



  uint8_t inject_frame[]=" You have been haCKed";
  inject_frame[0] = COMMAND_CLASS_SECURITY_2;
  inject_frame[1] = SECURITY_2_COMMANDS_SUPPORTED_REPORT;
  S2_application_command_handler(ctx1, &conn12, inject_frame, sizeof(inject_frame));


  /*Send the frame to ctx1*/
  S2_application_command_handler(ctx1, &conn12, ts.frame, ts.frame_len);
  TEST_ASSERT_EQUAL(3, ts.fcount);

  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, ts.frame[0]);
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION, ts.frame[1]);
  //TEST_ASSERT_EQUAL(ts.frame[2],COMMAND_CLASS_SECURITY_2); //seq
  TEST_ASSERT_EQUAL(SECURITY_2_MESSAGE_ENCAPSULATION_PROPERTIES1_EXTENSION_BIT_MASK, ts.frame[3]);
  TEST_ASSERT_EQUAL(18, ts.frame[4]);
  TEST_ASSERT_EQUAL(S2_MSG_EXTHDR_CRITICAL_FLAG | S2_MSG_EXTHDR_TYPE_SN,
      ts.frame[5]);

  S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);

  TEST_ASSERT_EQUAL(1, ts.s2_send_done); //Check we got a send done event
  TEST_ASSERT_EQUAL(S2_TRANSMIT_COMPLETE_OK, ts.s2_send_status); //Check we got a send done event

  /*Send the frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, ts.frame, ts.frame_len);
  TEST_ASSERT_EQUAL(sizeof(hello), ts.rx_frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(hello, ts.rx_frame, sizeof(hello));
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count); // Test that no sync events were emitted during test
}

/**
 * Check "happy case" for emitting a S2 Resynchronization Event with reason
 * SOS_EVENT_REASON_UNANSWERED.
 *
 * Scenario: A succesful Verify Delivery Transmit times out, returns SUCCESS
 * and then gets a Nonce Report SOS. This must generate a S2 Resynchronization Event.
 * We arbitrarily choose the case where the verify delivery succeeds on the re-transmit.
 * Another test case would be to time out Verify Delivery on first transmit and do the same.
 * In this test, the S2 context is idle when the SOS comes in.
 *
 */
void test_sos_recsync_event_1() {

  uint8_t bad_frame[1500];

  test_sd_verify_tx_ok_second_attempt();
  /* Now ctx1 has timed out verify delivery on second attempt and returned
   * S2_TRANSMIT_COMPLETE_OK. We now inject a SOS Nonce Report and expect
   * to see a resync event signalled. */

  /* Generate a Nonce Report SOS by injecting a bad frame into ctx2 */
  /*make a bad frame (there is a good frame in */
  memcpy(bad_frame,ts.frame,ts.frame_len);
  bad_frame[ts.frame_len-1]++; // *this* will invalidate the MAC
  bad_frame[2]++; /* This will increment seqno, so it doesnt get duplicate detected */
  /* Now we have a nonce report SOS in ts.frame*/

  /*Send the BAD frame to ctx2*/
  S2_application_command_handler(ctx2, &ts.last_trans, bad_frame, ts.frame_len);

  /*Expect that we get a nonce report*/
  TEST_ASSERT_EQUAL(4, ts.fcount);
  TEST_ASSERT_EQUAL(20, ts.frame_len);
  TEST_ASSERT_EQUAL_STRING_LEN(nonce_report, ts.frame, 2);
  uint8_t sos_seqno = ts.frame[2];
  TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);

  /* Expect no Sync Events so far*/
  TEST_ASSERT_EQUAL(0, ts.sync_ev.count);

  /* Invalidate ts.sync_ev so they aren't accidentally correct for the following asserts */
  memset(&ts.sync_ev, 0 ,sizeof ts.sync_ev);
  ts.sync_ev.reason = 42;

  /*Send the Nonce Rep SOS to ctx1*/
  S2_application_command_handler(ctx1, &ts.last_trans, ts.frame, ts.frame_len);

  /* Expect that we get a s2 resync event*/
  TEST_ASSERT_EQUAL(1, ts.sync_ev.count);
  TEST_ASSERT_EQUAL(2, ts.sync_ev.remote_node);
  TEST_ASSERT_EQUAL(sos_seqno, ts.sync_ev.seqno);
  TEST_ASSERT_EQUAL(1, ts.sync_ev.local_node);

  /*Expect that nothing is sent from ctx 1*/
  TEST_ASSERT_EQUAL(4, ts.fcount);
}

/**
 * Test that a resync event is emitted if SOS comes in from Node A, while sending to Node B
 *
 */
void test_sos_recsync_event_2() {

  uint8_t bad_frame[1500];
  uint8_t nonce_rep_frame[30];
  uint16_t nonce_rep_frame_len;

  const char test_msg[] = "Testing 1 2 3";

  /* Test has two flavours: With and without verify delivery for the "busy" transmission to A
   * while Nonce Rep SOS comes in from node B */
  for (uint8_t test_flavour = 0; test_flavour < 2; test_flavour++)
  {
    char flavour_msg[] = "test flavour 01";
    snprintf(flavour_msg, sizeof flavour_msg, "test flavour %u", test_flavour);

    test_sd_verify_tx_ok_second_attempt();
    /* Now ctx1 has timed out verify delivery on second attempt and returned
     * S2_TRANSMIT_COMPLETE_OK. We now inject a SOS Nonce Report and expect
     * to see a resync event signalled. */

    /* Generate a Nonce Report SOS by injecting a bad frame into ctx2 */
    /*make a bad frame (there is a good frame in */
    memcpy(bad_frame,ts.frame,ts.frame_len);
    bad_frame[ts.frame_len-1]++; // *this* will invalidate the MAC
    bad_frame[2]++; /* This will increment seqno, so it doesnt get duplicate detected */
    /* Now we have a nonce report SOS in ts.frame*/

    /*Send the BAD frame to ctx2*/
    S2_application_command_handler(ctx2, &ts.last_trans, bad_frame, ts.frame_len);

    /*Expect that we get a nonce report*/
    TEST_ASSERT_EQUAL(4, ts.fcount);
    TEST_ASSERT_EQUAL(20, ts.frame_len);
    TEST_ASSERT_EQUAL_STRING_LEN(nonce_report, ts.frame, 2);
    TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);

    memcpy(nonce_rep_frame, ts.frame, (ts.frame_len <= sizeof nonce_rep_frame) ? ts.frame_len : 0);
    nonce_rep_frame_len = ts.frame_len;

    /* Expect no Sync Events so far*/
    TEST_ASSERT_EQUAL(0, ts.sync_ev.count);

    /* Invalidate ts.sync_ev */
    ts.sync_ev.remote_node = 0;
    ts.sync_ev.reason = 0xFF;

    /* Make ctx1 busy sending to node 2 */
    if (test_flavour == 0) {
      TEST_ASSERT_EQUAL(S2_TXOPTION_VERIFY_DELIVERY, conn12.tx_options);
      S2_send_data(ctx1, &conn12, (uint8_t*) test_msg, sizeof(test_msg));
      TEST_ASSERT_EQUAL(VERIFYING_DELIVERY, ctx1->fsm);
      S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
      TEST_ASSERT_EQUAL(VERIFYING_DELIVERY, ctx1->fsm);
    } else {
      conn12.tx_options = 0; /* disable verify delivery */
      S2_send_data(ctx1, &conn12, (uint8_t*) test_msg, sizeof(test_msg));
      TEST_ASSERT_EQUAL(SENDING_MSG, ctx1->fsm);
    }

    ts.fcount = 0;

    /* Send the Nonce Rep SOS to from node 3 ctx1 */
    S2_application_command_handler(ctx1, &conn13, nonce_rep_frame, nonce_rep_frame_len);

    /* Expect that we get a s2 resync event*/
    TEST_ASSERT_EQUAL_MESSAGE(1, ts.sync_ev.count, flavour_msg);
    TEST_ASSERT_EQUAL_MESSAGE(3, ts.sync_ev.remote_node, flavour_msg);
    TEST_ASSERT_EQUAL_MESSAGE(SOS_EVENT_REASON_UNANSWERED, ts.sync_ev.reason, flavour_msg);

    /*Expect that nothing is sent from ctx 1*/
    TEST_ASSERT_EQUAL_MESSAGE(0, ts.fcount, flavour_msg);
  }
}

/**
 * Test that a resync event is emitted if SOS comes in from Node A, while WAIT_NONCE_RAPORT to Node B
 *
 */
void test_sos_recsync_event_3() {

  uint8_t bad_frame[1500];
  uint8_t nonce_rep_frame[30];
  uint16_t nonce_rep_frame_len;

  const char test_msg[] = "Testing 1 2 3";

  /* Test has only one flavour - remove this for loop */
  for (uint8_t test_flavour = 0; test_flavour < 1; test_flavour++)
  {
    char flavour_msg[] = "test flavour 01";
    snprintf(flavour_msg, sizeof flavour_msg, "test flavour %u", test_flavour);

    test_sd_verify_tx_ok_second_attempt();
    /* Now ctx1 has timed out verify delivery on second attempt and returned
     * S2_TRANSMIT_COMPLETE_OK. We now inject a SOS Nonce Report and expect
     * to see a resync event signalled. */

    /* Generate a Nonce Report SOS by injecting a bad frame into ctx2 */
    /*make a bad frame (there is a good frame in */
    memcpy(bad_frame,ts.frame,ts.frame_len);
    bad_frame[ts.frame_len-1]++; // *this* will invalidate the MAC
    bad_frame[2]++; /* This will increment seqno, so it doesnt get duplicate detected */
    /* Now we have a nonce report SOS in ts.frame*/

    /*Send the BAD frame to ctx2*/
    S2_application_command_handler(ctx2, &ts.last_trans, bad_frame, ts.frame_len);

    /*Expect that we get a nonce report*/
    TEST_ASSERT_EQUAL(4, ts.fcount);
    TEST_ASSERT_EQUAL(20, ts.frame_len);
    TEST_ASSERT_EQUAL_STRING_LEN(nonce_report, ts.frame, 2);
    TEST_ASSERT_EQUAL(SECURITY_2_NONCE_REPORT_PROPERTIES1_SOS_BIT_MASK, ts.frame[3]);

    memcpy(nonce_rep_frame, ts.frame, (ts.frame_len <= sizeof nonce_rep_frame) ? ts.frame_len : 0);
    nonce_rep_frame_len = ts.frame_len;

    /* Expect no Sync Events so far*/
    TEST_ASSERT_EQUAL(0, ts.sync_ev.count);

    /* Invalidate ts.sync_ev */
    ts.sync_ev.remote_node = 0;
    ts.sync_ev.reason = 0xFF;

    /* Get ctx1 in WAIT_NONCE_RAPPORT state by asking it to send to node 4 */
    s2_connection_t conn14 = { 1, 4 };
    S2_send_data(ctx1, &conn14, (uint8_t*) test_msg, sizeof(test_msg));
    S2_send_frame_done_notify(ctx1, S2_TRANSMIT_COMPLETE_OK,0x42);
    TEST_ASSERT_EQUAL(WAIT_NONCE_RAPORT, ctx1->fsm);

    ts.fcount = 0;

    /* Send the Nonce Rep SOS to from node 3 ctx1 */
    S2_application_command_handler(ctx1, &conn13, nonce_rep_frame, nonce_rep_frame_len);

    /* Expect that we get a s2 resync event*/
    TEST_ASSERT_EQUAL_MESSAGE(1, ts.sync_ev.count, flavour_msg);
    TEST_ASSERT_EQUAL_MESSAGE(3, ts.sync_ev.remote_node, flavour_msg);
    TEST_ASSERT_EQUAL_MESSAGE(SOS_EVENT_REASON_UNANSWERED, ts.sync_ev.reason, flavour_msg);

    /*Expect that nothing is sent from ctx 1*/
    TEST_ASSERT_EQUAL_MESSAGE(0, ts.fcount, flavour_msg);
  }
}

/* Stub function */
uint8_t s2_inclusion_set_timeout(struct S2* ctxt, uint32_t interval)
{
#ifndef SINGLE_CONTEXT
  UNUSED(ctxt);
#endif
  UNUSED(interval);
  return 0;
}

void s2_inclusion_stop_timeout(void)
{
}

void keystore_public_key_read(uint8_t *buf)
{
  UNUSED(buf);
}

void keystore_private_key_read(uint8_t *buf)
{
  UNUSED(buf);
}

bool keystore_network_key_read(uint8_t keyclass, uint8_t *buf)
{
  UNUSED(keyclass);
  UNUSED(buf);
  return true;
}

bool keystore_network_key_write(uint8_t keyclass, const uint8_t *keybuf)
{
  UNUSED(keyclass);
  UNUSED(keybuf);
  return true;
}

bool keystore_network_key_clear(uint8_t keyclass)
{
  UNUSED(keyclass);
  return true;
}

void S2_dbg_printf(const char *format, ...)
{
    va_list argptr;

    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);

}
