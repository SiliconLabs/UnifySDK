#include "string.h"
#include "transport_service.h"
#include "stdio.h"
#include "unity.h"
#include "zwave_tx_scheme_selector_mock.h"
#include "sl_log.h"

#define MAX_MSDU_SIZE   158
#define SMALL_MSDU_SIZE 47

ts_node_id_t my_node_id     = 1;
ts_node_id_t source_node_id = 2;

/* -------------- ctimer stub begin -------------------------*/
typedef uint16_t clock_time_t;
struct ctimer {
  struct ctimer *next;
  clock_time_t tickCounts;
  clock_time_t startValue;
  void (*f)(void *);
  void *ptr;
};

uint32_t clock_time(void)
{
  return 0;
}
typedef void (*timer_callback_t)(void *nthing);
// Call this function when timer needs to be off and the callback function
// needs to be called. For e.g. if TS has sent
//  ctimer_set(&scb.reset_timer, RESET_TIME, reset_transport_service, 0);
//  Calling test_timer_callback() will call reset_transport_service()
static timer_callback_t test_timer_callback;
static void *ctimer_user_ptr;
void ctimer_set(struct ctimer *c, clock_time_t t, void (*f)(void *), void *ptr)
{
  test_timer_callback = f;
  ctimer_user_ptr     = ptr;
  return;
}
int fc_timer_counter = 0;
void ctimer_stop(struct ctimer *c) {}
/* -------------- ctimer stub end -------------------------*/

// This is the complete datagagram (payload of fragments 1, 2 & 3)
unsigned char test_complete_datagram[] = {
  // frag1
  0x20,
  0x01,
  0x00,
  0x03,
  0x04,
  0x05,
  0x06,
  0x07,
  0x08,
  0x09,
  0x0a,
  0x0b,
  0x0c,
  0x0d,
  0x0e,
  0x0f,
  0x10,
  0x11,
  0x12,
  0x13,
  0x14,
  0x15,
  0x16,
  0x17,
  0x18,
  0x19,
  0x1a,
  0x1b,
  0x1c,
  0x1d,
  0x1e,
  0x1f,
  0x20,
  0x21,
  0x22,
  0x23,
  0x24,
  0x25,
  0x26,
  0x27,
  0x28,
  0x29,
  0x2a,
  0x2b,
  0x2c,
  0x2d,
  0x2e,
  // frag2
  0x2f,
  0x30,
  0x31,
  0x32,
  0x33,
  0x34,
  0x20,
  0x01,
  0x00,
  0x03,
  0x04,
  0x05,
  0x06,
  0x07,
  0x08,
  0x09,
  0x0a,
  0x0b,
  0x0c,
  0x0d,
  0x0e,
  0x0f,
  0x10,
  0x11,
  0x12,
  0x13,
  0x14,
  0x15,
  0x16,
  0x17,
  0x18,
  0x19,
  0x1a,
  0x1b,
  0x1c,
  0x1d,
  0x1e,
  0x1f,
  0x20,
  0x21,
  0x22,
  0x23,
  0x24,
  0x25,
  0x26,
  0x27,
  0x28,
  // frag3
  0x29,
  0x2a,
  0x2b,
  0x2c,
  0x2d,
  0x2e,
  0x2f,
  0x30,
  0x31,
  0x32,
  0x33,
  0x34,
};

// test_subseq_frag2a and 2b are test_subseq_frag2 split into two
// different-sized fragments
unsigned char test_subseq_frag2a[]
  = {0x55,  // COMMAND_CLASS_TRANSPORT_SERVICE_V2
     0xe0,  // COMMAND_SUBSEQUENT_FRAGMENT[7..3], datagram_size_1[2..0]
            // (size bits 10..8 = 0)
     0x6a,  // datagram_size_2 (size bits 7..0 = 106)
     0x00,  // session_ID [7..4], Ext[3] (=0 i.e. no ext header),
            // datagram_offset_1[2..0] (size bits 10..8 = 0)
     0x28,  // datagram_offset_2 (size bits 7..0 = 47)

     // ----- Payload from here (next 20 bytes)
     0x28,
     0x29,
     0x2a,
     0x2b,
     0x2c,
     0x2d,
     0x2e,
     0x2f,
     0x30,
     0x31,
     0x32,
     0x33,
     0x34,
     0x20,
     0x01,
     0x00,
     0x03,
     0x04,
     0x05,
     0x06,
     // ----- Payload to here

     // Last two bytes are CRC-16-CCITT
     0xd1,
     0x80};

unsigned char test_subseq_frag2b[]
  = {0x55,  // COMMAND_CLASS_TRANSPORT_SERVICE_V2
     0xe0,  // COMMAND_SUBSEQUENT_FRAGMENT[7..3], datagram_size_1[2..0]
            // (size bits 10..8 = 0)
     0x6a,  // datagram_size_2 (size bits 7..0 = 106)
     0x00,  // session_ID [7..4], Ext[3] (=0 i.e. no ext header),
            // datagram_offset_1[2..0] (size bits 10..8 = 0)
     0x3c,  // datagram_offset_2 (size bits 7..0 = 67)

     // ----- Payload from here (next 20 bytes)
     0x07,
     0x08,
     0x09,
     0x0a,
     0x0b,
     0x0c,
     0x0d,
     0x0e,
     0x0f,
     0x10,
     0x11,
     0x12,
     0x13,
     0x14,
     0x15,
     0x16,
     0x17,
     0x18,
     0x19,
     0x1a,
     // ----- Payload to here

     // Last two bytes are CRC-16-CCITT
     0xa6,
     0x17};

unsigned char test_first_frag1[]
  = {0x55,  // COMMAND_CLASS_TRANSPORT_SERVICE_V2
     0xc0,  // COMMAND_FIRST_FRAGMENT[7..3], datagram_size_1[2..0]
            // (size bits 10..8 = 0)
     0x6a,  // datagram_size_2 (size bits 7..0 = 106)
     0x00,  // session_ID [7..4], Ext[3] (=0 i.e. no ext header), Reserved[2..0]

     // ----- Payload from here (first 40 bytes)
     0x20,
     0x01,
     0x00,
     0x03,
     0x04,
     0x05,
     0x06,
     0x07,
     0x08,
     0x09,
     0x0a,
     0x0b,
     0x0c,
     0x0d,
     0x0e,
     0x0f,
     0x10,
     0x11,
     0x12,
     0x13,
     0x14,
     0x15,
     0x16,
     0x17,
     0x18,
     0x19,
     0x1a,
     0x1b,
     0x1c,
     0x1d,
     0x1e,
     0x1f,
     0x20,
     0x21,
     0x22,
     0x23,
     0x24,
     0x25,
     0x26,
     0x27,
     // ----- Payload to here

     // Last two bytes are CRC-16-CCITT
     // (can be calculated with https://crccalc.com/ for test purposes
     //  - use the "CRC-16/AUG-CCITT" value)
     0xEA,
     0x0D};

unsigned char test_subseq_frag2[]
  = {0x55,  // COMMAND_CLASS_TRANSPORT_SERVICE_V2
     0xe0,  // COMMAND_SUBSEQUENT_FRAGMENT[7..3], datagram_size_1[2..0]
            // (size bits 10..8 = 0)
     0x6a,  // datagram_size_2 (size bits 7..0 = 106)
     0x00,  // session_ID [7..4], Ext[3] (=0 i.e. no ext header),
            // datagram_offset_1[2..0] (size bits 10..8 = 0)
     0x28,  // datagram_offset_2 (size bits 7..0 = 47)

     // ----- Payload from here (next 40 bytes)
     0x28,
     0x29,
     0x2a,
     0x2b,
     0x2c,
     0x2d,
     0x2e,
     0x2f,
     0x30,
     0x31,
     0x32,
     0x33,
     0x34,
     0x20,
     0x01,
     0x00,
     0x03,
     0x04,
     0x05,
     0x06,
     0x07,
     0x08,
     0x09,
     0x0a,
     0x0b,
     0x0c,
     0x0d,
     0x0e,
     0x0f,
     0x10,
     0x11,
     0x12,
     0x13,
     0x14,
     0x15,
     0x16,
     0x17,
     0x18,
     0x19,
     0x1A,
     // ----- Payload to here

     // Last two bytes are CRC-16-CCITT
     0x19,
     0xd9};

unsigned char test_subseq_frag3[]
  = {0x55,  // COMMAND_CLASS_TRANSPORT_SERVICE_V2
     0xe0,  // COMMAND_SUBSEQUENT_FRAGMENT[7..3], datagram_size_1[2..0]
            // (size bits 10..8 = 0)
     0x6a,  // datagram_size_2 (size bits 7..0 = 106)
     0x00,  // session_ID [7..4], Ext[3] (=0 i.e. no ext header),
            // datagram_offset_1[2..0] (size bits 10..8 = 0)
     0x50,  // datagram_offset_2 (size bits 7..0 = 94)

     // ----- Payload from here (last 27 bytes)
     0x1b,
     0x1c,
     0x1d,
     0x1e,
     0x1f,
     0x20,
     0x21,
     0x22,
     0x23,
     0x24,
     0x25,
     0x26,
     0x27,
     0x28,
     0x29,
     0x2a,
     0x2b,
     0x2c,
     0x2d,
     0x2e,
     0x2f,
     0x30,
     0x31,
     0x32,
     0x33,
     0x34,
     // ----- Payload to here

     // Last two bytes are CRC-16-CCITT
     0x0e,
     0x07};

unsigned char test_frag_compl[] = {
  0x55,  // COMMAND_CLASS_TRANSPORT_SERVICE_V2
  0xE8,  // COMMAND_FRAGMENT_COMPLETE
  0x00   // SessionID[7..4], Reserved[3..0]
};

unsigned char test_frag_req[] = {
  0x55,  // COMMAND_CLASS_TRANSPORT_SERVICE_V2
  0xC8,  // COMMAND_FRAGMENT_REQUEST
  0x00,  // SessionID[7..4], Reserved[3], datagram_offset_1[2..0]
         // (size bits 10..8 = 0)
  0x28   // datagram_offset_2 = 47
};

unsigned char test_frag_req1[] = {
  0x55,  // COMMAND_CLASS_TRANSPORT_SERVICE_V2
  0xC8,  // COMMAND_FRAGMENT_REQUEST
  0x00,  // SessionID[7..4], Reserved[3], datagram_offset_1[2..0]
         //(size bits 10..8 = 0)
  0x3C   // datagram_offset_2 = 67
};
unsigned char test_frag_req_final[] = {
  0x55,  // COMMAND_CLASS_TRANSPORT_SERVICE_V2
  0xC8,  // COMMAND_FRAGMENT_REQUEST
  0x00,  // SessionID[7..4], Reserved[3], datagram_offset_1[2..0]
         //(size bits 10..8 = 0)
  0x50,  // datagram_offset_2 (size bits 7..0 = 94)
};

unsigned char test_frag_wait_zero_pending[]  = {0x55, 0xF0, 0x00};
unsigned char test_frag_wait_two_pending[]   = {0x55, 0xF0, 0x02};
unsigned char test_frag_wait_three_pending[] = {0x55, 0xF0, 0x03};

void stubbed_upper_layer_command_handler(ts_node_id_t source,
                                         ts_node_id_t dest,
                                         const uint8_t *frame,
                                         uint16_t frame_length)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(frame,
                               test_complete_datagram,
                               sizeof(test_complete_datagram));
  TEST_ASSERT_EQUAL(source, source);
  TEST_ASSERT_EQUAL(dest, my_node_id);
  TEST_ASSERT_EQUAL(frame_length, sizeof(test_complete_datagram));
}

uint8_t send_data_frag_compl(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_frag_compl,
                               sizeof(test_frag_compl));
  return 0;
}
uint8_t send_data_frag_req_rx_timeout(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload, test_frag_req, sizeof(test_frag_req));
  test_timer_callback(ctimer_user_ptr);
  return 1;
}

uint8_t send_data_frag_req_miss_one_frag(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload, test_frag_req, sizeof(test_frag_req));

  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_frag_compl);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag2,
                                        sizeof(test_subseq_frag2)),
    false);
  return 0;
}

uint8_t send_data_frag_req(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_frag_req_final,
                               sizeof(test_frag_req_final));

  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_frag_compl);
  //contiki_test_helper_run(5000); //Reset the State machine
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag3,
                                        sizeof(test_subseq_frag3)),
    false);
  return 0;
}

uint8_t send_data_frag_req1(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload, test_frag_req1, sizeof(test_frag_req1));
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_frag_compl);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag2b,
                                        sizeof(test_subseq_frag2b)),
    false);
  return 0;
}

uint8_t send_data_frag_wait_zero_pending(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_frag_wait_zero_pending,
                               sizeof(test_frag_wait_zero_pending));
  return 0;
}
uint8_t send_data_frag_wait_two_pending(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_frag_wait_two_pending,
                               sizeof(test_frag_wait_two_pending));
  return 0;
}

uint8_t send_data_frag_wait_three_pending(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_frag_wait_three_pending,
                               sizeof(test_frag_wait_three_pending));
  return 0;
}

/* -------------- Test transport service's Receiving functionality ---------------------------------------*/

/*
    1. Send first fragment
    2. send second subseq fragment
    3. send third subseq fragment
    4. check if fragment complete is sent
*/

void test_simple_test()
{
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_frag_compl);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_first_frag1,
                                        sizeof(test_first_frag1)),
    false);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag2,
                                        sizeof(test_subseq_frag2)),
    false);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag3,
                                        sizeof(test_subseq_frag3)),
    false);
}

/*
 * Broadcast
    1. Send first fragment
    2. send second subseq fragment
    3. send third subseq fragment
    4. check if fragment complete is sent
*/

void test_simple_test_broadcast()
{
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_frag_compl);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        BROADCAST,
                                        test_first_frag1,
                                        sizeof(test_first_frag1)),
    false);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        BROADCAST,
                                        test_subseq_frag2,
                                        sizeof(test_subseq_frag2)),
    false);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        BROADCAST,
                                        test_subseq_frag3,
                                        sizeof(test_subseq_frag3)),
    false);
}

/*
    1. Send first fragment
    2. Send next two subseq fragments (different sizes)
    3. Send last subseq fragment
    4. Check if fragment complete is sent
*/

void test_non_uniform_fragment_sizes()
{
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_frag_compl);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_first_frag1,
                                        sizeof(test_first_frag1)),
    false);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag2a,
                                        sizeof(test_subseq_frag2a)),
    false);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag2b,
                                        sizeof(test_subseq_frag2b)),
    false);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag3,
                                        sizeof(test_subseq_frag3)),
    false);
}
/*
    1. Send first fragment
    2. Send next two subseq fragments (different sizes, reverse order)
    3. Send last subseq fragment
    4. Check if fragment complete is sent
*/

void test_non_sequential_fragments()
{
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_frag_compl);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_first_frag1,
                                        sizeof(test_first_frag1)),
    false);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag2b,
                                        sizeof(test_subseq_frag2b)),
    false);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag2a,
                                        sizeof(test_subseq_frag2a)),
    false);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag3,
                                        sizeof(test_subseq_frag3)),
    false);
}
/*  1. send first fragment
    2. Fire rx timer which will make the transport service request remaining fragments
    3. see fragment req is sent
    4. send second fragment
    5. send third fragment
    6. Check that fragment complete is sent
*/

void test_dont_send_one_frag()
{
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_frag_req);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_first_frag1,
                                        sizeof(test_first_frag1)),
    false);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag2,
                                        sizeof(test_subseq_frag2)),
    false);
  test_timer_callback(ctimer_user_ptr);
}

void test_dont_send_one_frag_rx_timeout()
{
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_frag_req_rx_timeout);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_first_frag1,
                                        sizeof(test_first_frag1)),
    false);

  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag3,
                                        sizeof(test_subseq_frag3)),
    false);
}

/*
 * Send first and third fragment and see if Transport service sends request for second fragment
 */
void test_miss_one_frag()
{
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_frag_req_miss_one_frag);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_first_frag1,
                                        sizeof(test_first_frag1)),
    false);

  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag3,
                                        sizeof(test_subseq_frag3)),
    false);

  //contiki_test_helper_run(5000); //Reset the State machine
}

///*
// * Send first and third fragment and see if Transport service sends request for second fragment
// */
//void test_miss_one_frag_broadcast()
//{
//  transport_service_init(my_node_id,
//                         stubbed_upper_layer_command_handler,
//                         0);
//  TEST_ASSERT_EQUAL(
//    transport_service_on_frame_received(source_node_id,
//                                        my_node_id,
//                                        BROADCAST,
//                                        test_first_frag1,
//                                        sizeof(test_first_frag1)),
//    false);
//
//  TEST_ASSERT_EQUAL(
//    transport_service_on_frame_received(source_node_id,
//                                        my_node_id,
//                                        BROADCAST,
//                                        test_subseq_frag3,
//                                        sizeof(test_subseq_frag3)),
//    false);
//
//  //contiki_test_helper_run(5000); //Reset the State machine
//}
//
/*
 * Send first, second and fourt fragment and see if Transport service sends request for third fragment
 */
void test_miss_one_frag_variant()
{
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_frag_req1);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_first_frag1,
                                        sizeof(test_first_frag1)),
    false);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag2a,
                                        sizeof(test_subseq_frag2a)),
    false);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag3,
                                        sizeof(test_subseq_frag3)),
    false);
}

/* 1. send subseq fragment
   2. check that fragment wait is sent with 0 pending fragments
 */
void test_dont_send_first_frag()
{
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_frag_wait_zero_pending);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag2a,
                                        sizeof(test_subseq_frag2a)),
    false);
}

/* 1. Send first fragment
   2. Send subseq fragment from different source node
   3. Test if fragment wait is sent
   4. send second subseq fragment from same source node as first fragment of step.1
   5. send third subseq fragment from same source node as first fragment of step.1
   6. Check that fragment complete is sent
*/
void test_frag_wait_fn()
{
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_frag_wait_three_pending);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_first_frag1,
                                        sizeof(test_first_frag1)),
    false);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(3,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag2,
                                        sizeof(test_subseq_frag2)),
    false);
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_frag_compl);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag2,
                                        sizeof(test_subseq_frag2)),
    false);
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag3,
                                        sizeof(test_subseq_frag3)),
    false);
}
/* -- Test transport service's Sending functionality -------------------------*/
unsigned char raw_data2[]
  = {0x20, 0x01, 0x00, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
     0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
     0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
     0x30, 0x31, 0x32, 0x33, 0x34, 0x20, 0x01, 0x00, 0x03, 0x04, 0x05, 0x06,
     0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12,
     0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E,
     0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A,
     0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34};

/* Purpose of this test is to test the sending functionality of TS
*/
uint8_t send_data_test_subseq_frag3(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_subseq_frag3,
                               sizeof(test_subseq_frag3));

  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_frag_compl,
                                        sizeof(test_frag_compl)),
    false);
  return 0;
}

uint8_t send_data_test_subseq_frag2(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_subseq_frag2,
                               sizeof(test_subseq_frag2));

  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_subseq_frag3);
  on_lower_layer_send_data_complete(0, 0);
  test_timer_callback(0);
  return 0;
}

uint8_t send_data_test_first_frag1(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_first_frag1,
                               sizeof(test_first_frag1));
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_subseq_frag2);
  on_lower_layer_send_data_complete(0, 0);
  test_timer_callback(0);
  return 0;
}

void stubbed_on_transport_service_send_data_complete(uint8_t status, void *user)
{
  TEST_ASSERT_EQUAL(status, 0);
}

/* stubbed_failed_on_transport_service_send_data_complete() will check the
 * return code passed to it to make sure its FAILURE */
void stubbed_failed_on_transport_service_send_data_complete(uint8_t status,
                                                            void *user)
{
  TEST_ASSERT_EQUAL(status, 1);
}

void test_send_whole_payload()
{
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_first_frag1);

  transport_service_send_data(my_node_id,
                              2,
                              raw_data2,
                              sizeof(raw_data2),
                              SMALL_MSDU_SIZE,
                              stubbed_on_transport_service_send_data_complete);
  test_timer_callback(0);
  return;
}

uint8_t send_data_test_first_frag11(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  uint8_t backup_byte;
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_first_frag1,
                               sizeof(test_first_frag1));
  on_lower_layer_send_data_complete(0, 0);
  backup_byte = test_frag_req[2]; /*Change the session ID of frag_req cmd */
  test_frag_req[2] = 0xA0;
  TEST_ASSERT_EQUAL(transport_service_on_frame_received(2,
                                                        my_node_id,
                                                        SINGLECAST,
                                                        test_frag_req,
                                                        sizeof(test_frag_req)),
                    false);
  test_frag_req[2] = backup_byte;
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_subseq_frag2);
  on_lower_layer_send_data_complete(0, 0);
  test_timer_callback(0);
  return 0;
}

void test_send_frag_req_from_different_session_id()
{
  my_node_id = 3;
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_first_frag11);
  transport_service_send_data(my_node_id,
                              2,
                              raw_data2,
                              sizeof(raw_data2),
                              SMALL_MSDU_SIZE,
                              stubbed_on_transport_service_send_data_complete);
  test_timer_callback(0);
}

/*--------------------------------------------------------------------------*/
/* Purpose of this test is to see if the sending side tie breaks on receival of
 * segment
steps:
1. set source node as more than destination node
2. ask transport service to send 
3. set source node as less than destination node
4. ask TS to receive first frag
        At this point tie must be broken on receiving side as
        - TS is in Sending mode (step 1)
        - We, the sender of the new fragment(step 4) is receiver of 
          current datagram transmission(step 1) from TS in progress
        - Node id of TS is less than ours
5. check the flag_tie_broken
6. ask TS to receive second frag
7. check if we recive frag_compl
*/
uint8_t send_data_test_first_frag_tie_break(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  // Check that TS has sent test_first_frag1
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_first_frag1,
                               sizeof(test_first_frag1));
  on_lower_layer_send_data_complete(0, 0);  // Tell TS that first frag is sent

  //Set the send_data function to send_data_frag_compl so that when TS sends
  //frag complete test can verify it.

  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_frag_compl);

  //Send first frag
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(2,
                                        my_node_id,
                                        SINGLECAST,
                                        test_first_frag1,
                                        sizeof(test_first_frag1)),
    false);
  //Send second frag
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(2,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag2,
                                        sizeof(test_subseq_frag2)),
    false);
  //Send third frag
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(2,
                                        my_node_id,
                                        SINGLECAST,
                                        test_subseq_frag3,
                                        sizeof(test_subseq_frag3)),
    false);
  return 0;
}

void test_tie_break()
{
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_first_frag_tie_break);

  //2. ask transport service to send
  transport_service_send_data(my_node_id,
                              2,
                              raw_data2,
                              sizeof(raw_data2),
                              SMALL_MSDU_SIZE,
                              stubbed_on_transport_service_send_data_complete);
  test_timer_callback(0);
}
/*--------------------------------------------------------------------------*/
/* Purpose of this test is to check if the on_lower_layer_send_data_complete
 * failure makes transport service
   send the last fragment again
steps:
*/

uint8_t send_data_test_subseq_frag3_variant(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_subseq_frag3,
                               sizeof(test_subseq_frag3));
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_subseq_frag3_variant);
  on_lower_layer_send_data_complete(1, 0);
  return 0;
}

uint8_t send_data_test_subseq_frag2_variant(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_subseq_frag2,
                               sizeof(test_subseq_frag2));

  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_subseq_frag3_variant);
  on_lower_layer_send_data_complete(0, 0);
  test_timer_callback(0);
  return 0;
}

uint8_t send_data_test_first_frag1_variant(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_first_frag1,
                               sizeof(test_first_frag1));
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_subseq_frag2_variant);
  on_lower_layer_send_data_complete(0, 0);
  test_timer_callback(0);
  return 0;
}

void test_two_last_fragments_failed_callback()
{
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_first_frag1_variant);

  transport_service_send_data(
    my_node_id,
    2,
    raw_data2,
    sizeof(raw_data2),
    SMALL_MSDU_SIZE,
    stubbed_failed_on_transport_service_send_data_complete);
  test_timer_callback(0);
  return;
}

/*--------------------------------------------------------------------------*/
/* Purpose of this test is to check if the on timeout of fc_timer does 
 * transport service
   send the last fragment again
*/
uint8_t send_data_test_subseq_frag3_final(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_subseq_frag3,
                               sizeof(test_subseq_frag3));
  transport_service_init(my_node_id, stubbed_upper_layer_command_handler, 0);
  on_lower_layer_send_data_complete(0, 0);
  return 0;
}

uint8_t send_data_test_subseq_frag3_fc_timer_expired_again(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  /* Check that TS sent third fragment
   */
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_subseq_frag3,
                               sizeof(test_subseq_frag3));
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_subseq_frag3_final);
  /* Send success callback for the fragment TS tried to send 
   */
  on_lower_layer_send_data_complete(0, 0);
  /* Ask TS to send next fragment*/
  test_timer_callback(0);
  return 0;
}

uint8_t send_data_test_subseq_frag3_fc_timer_expired(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  /* Check that TS sent third fragment
   */
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_subseq_frag3,
                               sizeof(test_subseq_frag3));
  /* Resister send_data_test_subseq_frag3_fc_timer_expired_again() as the next 
   * function
   * which will be called 
   */
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_subseq_frag3_fc_timer_expired_again);
  /* Send success callback for the third fragment TS tried to send 
   */
  on_lower_layer_send_data_complete(0, 0);
  /* Tell TS that fc_timer expired */
  test_timer_callback(0);
  return 0;
}

uint8_t send_data_test_subseq_frag2_fc_timer_expired(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  /* Check that TS sent second fragment
   */
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_subseq_frag2,
                               sizeof(test_subseq_frag2));

  /* Resister send_data_test_subseq_frag3_fc_timer_expired() as the next 
   * function
   * which will be called 
   */
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_subseq_frag3_fc_timer_expired);
  /* Send success callback for the second fragment TS tried to send 
   */
  on_lower_layer_send_data_complete(0, 0);
  /* Ask TS to send third fragment */
  test_timer_callback(0);
  return 0;
}

uint8_t send_data_test_first_fc_timer_expired(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  /* Check that TS send first fragment
   */
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_first_frag1,
                               sizeof(test_first_frag1));
  /* Resister send_data_test_subseq_frag2_fc_timer_expired() as the next 
   * function
   * which will be called 
   */
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_subseq_frag2_fc_timer_expired);
  /* Send success callback for the first fragment TS tried to send 
   */
  on_lower_layer_send_data_complete(0, 0);
  /* Ask TS to send second fragment */
  test_timer_callback(0);
  return 0;
}

void test_two_last_fragments_fc_timer_expired()
{
  /* Resister send_data_test_first_fc_timer_expired() as the next function
   * which will be called 
   */
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_first_fc_timer_expired);

  /*As TS to send raw_data2[] */
  transport_service_send_data(
    my_node_id,
    2,
    raw_data2,
    sizeof(raw_data2),
    SMALL_MSDU_SIZE,
    stubbed_failed_on_transport_service_send_data_complete);

  test_timer_callback(0);
  test_timer_callback(0);  //Reset TS
  return;
}
/*--------------------------------------------------------------------------*/
/* Purpose of this test is to check if the transport service
   sends TRANSPORT_SERVICE_BUSY for second send data attempt
*/

uint8_t send_data_test_first_frag_transport_service_busy_sending(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  /* Check that TS sent first fragment
   */
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_first_frag1,
                               sizeof(test_first_frag1));

  /* Resister send_data_test_subseq_frag2() as the next function
   * which will be called 
   */
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_subseq_frag2);  //<- continue

  /* Try sending another raw data attempt and check if TS returns 
   * TRANSPORT_SERVICE_BUSY
   */
  TEST_ASSERT_EQUAL(transport_service_send_data(my_node_id,
                                                2,
                                                raw_data2,
                                                sizeof(raw_data2),
                                                SMALL_MSDU_SIZE,
                                                0),
                    TRANSPORT_SERVICE_BUSY);

  /* Send success callback for the first fragment TS tried to send 
   */
  on_lower_layer_send_data_complete(0, 0);

  /* Ask TS to send second fragment */
  test_timer_callback(0);
  return 0;
}

void test_transport_service_busy_sending()
{
  /* Resister send_data_test_first_frag_transport_service_busy_sending() as 
   * the next function which will be called 
   */
  transport_service_init(
    my_node_id,
    stubbed_upper_layer_command_handler,
    send_data_test_first_frag_transport_service_busy_sending);  // <----

  transport_service_send_data(my_node_id,
                              2,
                              raw_data2,
                              sizeof(raw_data2),
                              SMALL_MSDU_SIZE,
                              stubbed_on_transport_service_send_data_complete);
  test_timer_callback(0);  //Reset TS
  return;
}
/*--------------------------------------------------------------------------*/
/* Purpose of this test is to check if the on transport service
   sends TRANSPORT_SERVICE_BUSY for send request in the middle of another
   TS receiving session 
*/

void test_transport_service_busy_receiving()
{
  /*As TS to be a reciving side */
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_first_frag1,
                                        sizeof(test_first_frag1)),
    false);

  /* Ask TS now to send in the middle of receiving. And check if this request is
   * denied as TRANSPORT_SERVICE_BUSY */
  TEST_ASSERT_EQUAL(transport_service_send_data(
                      my_node_id,
                      2,
                      raw_data2,
                      sizeof(raw_data2),
                      SMALL_MSDU_SIZE,
                      stubbed_on_transport_service_send_data_complete),
                    TRANSPORT_SERVICE_BUSY);

  test_timer_callback(0);  //Reset TS
  return;
}

/*--------------------------------------------------------------------------*/
/* Purpose of this test is to check if the on transport service
   aborts the session when it recieves a weird long frame of 201 bytes
*/

void test_transport_service_long_frame()
{
  TEST_ASSERT_EQUAL(transport_service_on_frame_received(source_node_id,
                                                        my_node_id,
                                                        SINGLECAST,
                                                        test_first_frag1,
                                                        201),
                    true);
  return;
}

/*--------------------------------------------------------------------------*/
/* Purpose of this test is to test the sending functionality of TS on 
 * receiving fragment request for a missing fragment
*/
uint8_t send_data_test_subseq_frag2_fc_timer_after_frag_req_resend(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  /* Check that TS sent missing test_subseq_frag2[] */
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_subseq_frag2,
                               sizeof(test_subseq_frag2));

  /* Send success callback for the second fragment TS tried to send 
   */
  on_lower_layer_send_data_complete(0, 0);
  test_timer_callback(0);
  return 0;
}

uint8_t send_data_test_subseq_frag3_fc_timer_after_frag_req(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  /* Check that TS sent test_subseq_frag3[] */
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_subseq_frag3,
                               sizeof(test_subseq_frag3));

  /* Resister send_data_test_subseq_frag2_fc_timer_after_frag_req_resend() as 
   * the next function which will be called 
   */
  transport_service_init(
    my_node_id,
    stubbed_upper_layer_command_handler,
    send_data_test_subseq_frag2_fc_timer_after_frag_req_resend);
  /* Notify TS that it received test_frag_req[] which tells that second
   * fragment is missing*/
  TEST_ASSERT_EQUAL(transport_service_on_frame_received(source_node_id,
                                                        my_node_id,
                                                        SINGLECAST,
                                                        test_frag_req,
                                                        sizeof(test_frag_req)),
                    false);
  /* Send success callback for the third fragment TS tried to send 
   */
  on_lower_layer_send_data_complete(0, 0);

  test_timer_callback(0);
  return 0;
}

uint8_t send_data_test_subseq_frag2_fc_timer_after_frag_req(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  /* Check that TS sent test_subseq_frag2[] */
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_subseq_frag2,
                               sizeof(test_subseq_frag2));

  /* Resister send_data_test_subseq_frag3_fc_timer_after_frag_req() as the next 
   * function
   * which will be called 
   */
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_subseq_frag3_fc_timer_after_frag_req);
  /* Send success callback for the second fragment TS tried to send 
   */
  on_lower_layer_send_data_complete(0, 0);
  /* Ask TS to send third fragment */
  test_timer_callback(0);
  return 0;
}

uint8_t send_data_test_first_frag_fc_timer_after_frag_req(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  /* Check that TS sent test_first_frag1[] */
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_first_frag1,
                               sizeof(test_first_frag1));
  /* Resister send_data_test_subseq_frag2_fc_timer_after_frag_req() as the next 
   * function
   * which will be called 
   */
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_subseq_frag2_fc_timer_after_frag_req);
  /* Send success callback for the first fragment TS tried to send 
   */
  on_lower_layer_send_data_complete(0, 0);
  /* Ask TS to send second fragment */
  test_timer_callback(0);
  return 0;
}

void test_fc_timer_after_frag_req()
{
  /* Resister send_data_test_first_frag_fc_timer_after_frag_req() as the next 
   * function
   * which will be called 
   */
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_first_frag_fc_timer_after_frag_req);

  /* Ask TS to send raw data and make sure send data is failure in
   * callback stubbed_failed_on_transport_service_send_data_complete()*/
  transport_service_send_data(
    my_node_id,
    2,
    raw_data2,
    sizeof(raw_data2),
    SMALL_MSDU_SIZE,
    stubbed_failed_on_transport_service_send_data_complete);
  /* Reset the TS */
  test_timer_callback(0);
  return;
}
/*--------------------------------------------------------------------------*/
/* Purpose of this test is to check if the on_lower_layer_send_data_complete
  failure makes transport service
   send the last fragment again
*/

uint8_t send_data_test_subseq_frag3_failed_send_data(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  /* Check that TS sent test_subseq_frag3[] */
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_subseq_frag3,
                               sizeof(test_subseq_frag3));
  // Return 1 means send_data called by TS has failed
  return 1;
}

uint8_t send_data_test_subseq_frag2_failed_send_data(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  /* Check that TS sent test_subseq_frag2[] */
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_subseq_frag2,
                               sizeof(test_subseq_frag2));

  /* Resister send_data_test_subseq_frag3_failed_send_data() as the next function
   * which will be called 
   */
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_subseq_frag3_failed_send_data);
  /* Send success callback for the subseq fragment TS tried to send 
   */
  on_lower_layer_send_data_complete(0, 0);
  /* Ask TS to send third fragment */
  test_timer_callback(0);
  return 0;
}

uint8_t send_data_test_first_frag1_failed_send_data(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  /* Check that TS sent test_first_frag[] */
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_first_frag1,
                               sizeof(test_first_frag1));

  /* Resister send_data_test_subseq_frag2_failed_send_data() as the next function
   * which will be called 
   */
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_subseq_frag2_failed_send_data);

  /* Send success callback for the first fragment TS tried to send 
   */
  on_lower_layer_send_data_complete(0, 0);
  /* Ask TS to send second fragment */
  test_timer_callback(0);
  return 0;
}

void test_two_last_fragments_failed_send_data()
{
  /* Resister send_data_test_first_frag1_failed_send_data() as the next function
   * which will be called 
   */
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_first_frag1_failed_send_data);

  /* Expect the send to fail.*/
  transport_service_send_data(
    my_node_id,
    2,
    raw_data2,
    sizeof(raw_data2),
    SMALL_MSDU_SIZE,
    stubbed_failed_on_transport_service_send_data_complete);

  test_timer_callback(0);
  /* Reset the TS */
  test_timer_callback(0);
  return;
}

/*--------------------------------------------------------------------------*/

/* Purpose of this test is to test the Fragment wait frame sending 
 * functionality of TS
*/
uint8_t send_data_test_subseq_frag2_with_one_wait(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  /* Check that TS sent second fragment
   */
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_subseq_frag2,
                               sizeof(test_subseq_frag2));

  /* Send success callback for the second fragment TS tried to send 
   */
  on_lower_layer_send_data_complete(0, 0);
  /* Resister send_data_test_first_frag1() as the next function
   * which will be called 
   */
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_first_frag1);

  /* Tell TS that it received test_frag_wait_two_pending[] frame
   */
  TEST_ASSERT_EQUAL(
    transport_service_on_frame_received(source_node_id,
                                        my_node_id,
                                        SINGLECAST,
                                        test_frag_wait_two_pending,
                                        sizeof(test_frag_wait_two_pending)),
    false);
  return 0;
}

uint8_t send_data_test_first_frag1_with_one_wait(
  ts_node_id_t source,
  ts_node_id_t dest,
  const uint8_t *payload,
  const uint16_t payload_len,
  const uint8_t no_of_expected_responses,
  const on_lower_layer_send_data_complete_t on_lower_layer_send_data_complete)
{
  /* Check that TS send first fragment
   */
  TEST_ASSERT_EQUAL_HEX8_ARRAY(payload,
                               test_first_frag1,
                               sizeof(test_first_frag1));
  /* Resister send_data_test_subseq_frag2_with_one_wait() as the next function
   * which will be called 
   */
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_subseq_frag2_with_one_wait);
  /* Send success callback for the first fragment TS tried to send 
   */
  on_lower_layer_send_data_complete(0, 0);
  /* Ask TS to send second fragment */
  test_timer_callback(0);
  return 0;
}

void test_send_whole_payload_with_one_wait()
{
  /* Resister send_data_test_first_frag1_with_one_wait() as the next function
   * which will be called 
   */
  transport_service_init(my_node_id,
                         stubbed_upper_layer_command_handler,
                         send_data_test_first_frag1_with_one_wait);

  /* Ask TS to send raw data and make sure send data is success in
   * callback stubbed_on_transport_service_send_data_complete()*/
  transport_service_send_data(my_node_id,
                              2,
                              raw_data2,
                              sizeof(raw_data2),
                              SMALL_MSDU_SIZE,
                              stubbed_on_transport_service_send_data_complete);
  test_timer_callback(0);
  return;
}
