/* © 2017 Silicon Laboratories Inc.
 */
#include <transport_service2.h>
#include  <S2.h> /* For s2_tx_status_t */
#include <stdint.h>

ts_param_t p;

//#define DEBUG
#ifdef DEBUG
#define v_printf(format, args...)  printf(format, ## args);
#else
#define v_printf(format, args...)
#endif
#define fail_if_nonzero(ret) if (ret) \
                            goto fail;

// This is the complete datagagram (payload of fragments 1, 2 & 3)
unsigned char test_complete_datagram[] =
{
  // frag1
  0x20, 0x01, 0x00, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 
  0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e,
  // frag2
  0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x20, 0x01,
  0x00, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
  0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
  0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
  0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21,
  0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
  // frag3
  0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
  0x31, 0x32, 0x33, 0x34,
};

unsigned char test_first_frag1[] = 
{
  0x55, // COMMAND_CLASS_TRANSPORT_SERVICE_V2
  0xc0, // COMMAND_FIRST_FRAGMENT[7..3], datagram_size_1[2..0] (size bits 10..8 = 0)
  0x6a, // datagram_size_2 (size bits 7..0 = 106)
  0x00, // session_ID [7..4], Ext[3] (=0 i.e. no ext header), Reserved[2..0]
  
  // ----- Payload from here (first 47 bytes)
  0x20, 0x01, 0x00, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 
  0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e,
  // ----- Payload to here 

  // Last two bytes are CRC-16-CCITT
  // (can be calculated with https://crccalc.com/ for test purposes
  //  - use the "CRC-16/AUG-CCITT" value)
  0x04, 0xa5
};

unsigned char test_subseq_frag2[] =
{
  0x55, // COMMAND_CLASS_TRANSPORT_SERVICE_V2
  0xe0, // COMMAND_SUBSEQUENT_FRAGMENT[7..3], datagram_size_1[2..0] (size bits 10..8 = 0)
  0x6a, // datagram_size_2 (size bits 7..0 = 106)
  0x00, // session_ID [7..4], Ext[3] (=0 i.e. no ext header), datagram_offset_1[2..0] (size bits 10..8 = 0)
  0x2f, // datagram_offset_2 (size bits 7..0 = 47)
 
  // ----- Payload from here (next 47 bytes)
  0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x20, 0x01,
  0x00, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
  0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
  0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
  0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21,
  0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
  // ----- Payload to here 
  
  // Last two bytes are CRC-16-CCITT
  0x1d, 0x69
};

// test_subseq_frag2a and 2b are test_subseq_frag2 split into two different-sized fragments
unsigned char test_subseq_frag2a[] =
{
  0x55, // COMMAND_CLASS_TRANSPORT_SERVICE_V2
  0xe0, // COMMAND_SUBSEQUENT_FRAGMENT[7..3], datagram_size_1[2..0] (size bits 10..8 = 0)
  0x6a, // datagram_size_2 (size bits 7..0 = 106)
  0x00, // session_ID [7..4], Ext[3] (=0 i.e. no ext header), datagram_offset_1[2..0] (size bits 10..8 = 0)
  0x2f, // datagram_offset_2 (size bits 7..0 = 47)
 
  // ----- Payload from here (next 20 bytes)
  0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x20, 0x01,
  0x00, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
  0x0a, 0x0b, 0x0c, 0x0d,
  // ----- Payload to here 
  
  // Last two bytes are CRC-16-CCITT
  0x8c, 0x4b
};

unsigned char test_subseq_frag2b[] =
{
  0x55, // COMMAND_CLASS_TRANSPORT_SERVICE_V2
  0xe0, // COMMAND_SUBSEQUENT_FRAGMENT[7..3], datagram_size_1[2..0] (size bits 10..8 = 0)
  0x6a, // datagram_size_2 (size bits 7..0 = 106)
  0x00, // session_ID [7..4], Ext[3] (=0 i.e. no ext header), datagram_offset_1[2..0] (size bits 10..8 = 0)
  0x43, // datagram_offset_2 (size bits 7..0 = 67)
 
  // ----- Payload from here (next 27 bytes)
  0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
  0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
  0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
  0x26, 0x27, 0x28,
  // ----- Payload to here 
  
  // Last two bytes are CRC-16-CCITT
  0x6b, 0x5a
};

unsigned char test_subseq_frag3[] =
{
  0x55, // COMMAND_CLASS_TRANSPORT_SERVICE_V2
  0xe0, // COMMAND_SUBSEQUENT_FRAGMENT[7..3], datagram_size_1[2..0] (size bits 10..8 = 0)
  0x6a, // datagram_size_2 (size bits 7..0 = 106)
  0x00, // session_ID [7..4], Ext[3] (=0 i.e. no ext header), datagram_offset_1[2..0] (size bits 10..8 = 0)
  0x5e, // datagram_offset_2 (size bits 7..0 = 94)
 
  // ----- Payload from here (last 12 bytes)
  0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
  0x31, 0x32, 0x33, 0x34,
  // ----- Payload to here 
  
  // Last two bytes are CRC-16-CCITT
  0x05, 0xd6
};

unsigned char test_frag_compl[] =
{
  0x55, // COMMAND_CLASS_TRANSPORT_SERVICE_V2
  0xE8, // COMMAND_FRAGMENT_COMPLETE
  0x00  // SessionID[7..4], Reserved[3..0]
};

unsigned char test_frag_req[] =
{
  0x55, // COMMAND_CLASS_TRANSPORT_SERVICE_V2
  0xC8, // COMMAND_FRAGMENT_REQUEST
  0x00, // SessionID[7..4], Reserved[3], datagram_offset_1[2..0] (size bits 10..8 = 0)
  0x2f  // datagram_offset_2 = 47
};

unsigned char test_frag_req1[] =
{
  0x55, // COMMAND_CLASS_TRANSPORT_SERVICE_V2
  0xC8, // COMMAND_FRAGMENT_REQUEST
  0x00, // SessionID[7..4], Reserved[3], datagram_offset_1[2..0] (size bits 10..8 = 0)
  0x43  // datagram_offset_2 = 67
};


unsigned char test_frag_req2[] =
{
  0x55, // COMMAND_CLASS_TRANSPORT_SERVICE_V2
  0xC8, // COMMAND_FRAGMENT_REQUEST
  0x00, // SessionID[7..4], Reserved[3], datagram_offset_1[2..0] (size bits 10..8 = 0)
  0x5e  // datagram_offset_2 = 94
};

unsigned char test_frag_wait_zero_pending[] = {0x55, 0xF0, 0x00};
unsigned char test_frag_wait[] = {0x55, 0xF0, 0x01};
unsigned char test_frag_wait_two_pending[] = {0x55, 0xF0, 0x02};
unsigned char test_frag_wait_three_pending[] = {0x55, 0xF0, 0x03};
unsigned char output[1024];

void ZCB_ts_senddata_cb(unsigned char status_send, TX_STATUS_TYPE* txStatus);

void fire_rx_timer();
void fire_fc_timer();
void fire_timer_btwn_2_frags(int test_type);
extern int fc_timer_counter;

uint8_t MyNodeID = 0;

/** Check a static in transport_service2.c. */
extern int check_flag_tie_broken();
extern int check_flag_fc_timer_expired_once();
extern int check_scb_current_dnode();
extern int compare_received_datagram(const uint8_t *data, uint16_t len);
extern int get_current_scb_cmn_session_id();

extern uint16_t ZW_CheckCrc16(uint16_t crc, uint8_t *pDataAddr, uint16_t bDataLen);
extern int call_with_large_value;

/* Should be s2_tx_status_t */
unsigned char global_status = 0xff;

/* For some reason, the type of ZW_TransportService_SendData() is different in gw and C51. */
#ifdef ZIPGW
void status_callback(uint8_t txStatus, TX_STATUS_TYPE *t)
#else
void status_callback(uint8_t txStatus, void *t)
#endif
{
    global_status = txStatus;
}

unsigned char raw_data [] = {0x20, 0x01, 0x00 ,0x03 ,0x04 ,0x05 ,0x06 ,0x07 ,0x08 ,0x09 ,0x0A ,0x0B ,0x0C ,0x0D ,0x0E ,0x0F ,0x10 ,0x11 ,0x12 ,0x13 ,0x14 ,0x15 ,0x16 ,0x17 ,0x18 ,0x19 ,0x1A ,0x1B ,0x1C ,0x1D ,0x1E ,0x1F ,0x20 ,0x21 ,0x22 ,0x23 ,0x24 ,0x25 ,0x26 ,0x27 ,0x28 ,0x29 ,0x2A ,0x2B ,0x2C ,0x2D ,0x2E, 0x2F ,0x30 ,0x31 ,0x32 ,0x33 ,0x34};
unsigned char raw_data2 [] = {0x20, 0x01, 0x00 ,0x03 ,0x04 ,0x05 ,0x06 ,0x07 ,0x08 ,0x09 ,0x0A ,0x0B ,0x0C ,0x0D ,0x0E ,0x0F ,0x10 ,0x11 ,0x12 ,0x13 ,0x14 ,0x15 ,0x16 ,0x17 ,0x18 ,0x19 ,0x1A ,0x1B ,0x1C ,0x1D ,0x1E ,0x1F ,0x20 ,0x21 ,0x22 ,0x23 ,0x24 ,0x25 ,0x26 ,0x27 ,0x28 ,0x29 ,0x2A ,0x2B ,0x2C ,0x2D ,0x2E, 0x2F ,0x30 ,0x31 ,0x32 ,0x33 ,0x34, 0x20, 0x01, 0x00 ,0x03 ,0x04 ,0x05 ,0x06 ,0x07 ,0x08 ,0x09 ,0x0A ,0x0B ,0x0C ,0x0D ,0x0E ,0x0F ,0x10 ,0x11 ,0x12 ,0x13 ,0x14 ,0x15 ,0x16 ,0x17 ,0x18 ,0x19 ,0x1A ,0x1B ,0x1C ,0x1D ,0x1E ,0x1F ,0x20 ,0x21 ,0x22 ,0x23 ,0x24 ,0x25 ,0x26 ,0x27 ,0x28 ,0x29 ,0x2A ,0x2B ,0x2C ,0x2D ,0x2E, 0x2F ,0x30 ,0x31 ,0x32 ,0x33 ,0x34};

#define CRC_POLY        0x1021

uint16_t zgw_crc16(uint16_t crc16, uint8_t *data_buf, unsigned long data_len)
{
  uint8_t WorkData;
  uint8_t bitMask;
  uint8_t NewBit;
  //printf("ZW_CheckCrc16: bDataLen = %u\r\n", bDataLen);
  while (data_len--)
  {
    WorkData = *data_buf++;
    for (bitMask = 0x80; bitMask != 0; bitMask >>= 1)
    {
      /* Align test bit with next bit of the message byte, starting with msb. */
      NewBit = ((WorkData & bitMask) != 0) ^ ((crc16 & 0x8000) != 0);
      crc16 <<= 1;
      if (NewBit)
      {
        crc16 ^= CRC_POLY;
      }
    } /* for (bitMask = 0x80; bitMask != 0; bitMask >>= 1) */
  }
  return crc16;
}

extern unsigned char output[];
bool TS_SEND_RAW(node_t snode, node_t dnode, uint8_t *cmd, uint8_t len, uint8_t flags, VOID_CALLBACKFUNC(completedFunc)(uint8_t, TX_STATUS_TYPE*))
{
    memcpy(output, cmd, len);

#if 0
    int i;
    for(i = 0; i < len; i++)
        printf("%x ", cmd[i]);

    printf("\n");
#endif
/*
    if (completedFunc)
        completedFunc(S2_TRANSMIT_COMPLETE_OK, NULL);
*/
    return 1;
}

typedef uint16_t clock_time_t;

struct ctimer {
    struct ctimer *next;
    clock_time_t tickCounts;
    clock_time_t startValue;
    VOID_CALLBACKFUNC(f)(void *);
    void *ptr;
};

void ctimer_set(struct ctimer *c, clock_time_t t,
                void (*f)(void *), void *ptr)
{
   return;
}
int fc_timer_counter = 0;
void ctimer_stop(struct ctimer *c)
{
    fc_timer_counter++;
}

void fire_timer_btwn_2_frags(int test_type) {
#ifdef ZIPGW
    TX_STATUS_TYPE t;
    memset(&t, 0, sizeof(TX_STATUS_TYPE));
   ZCB_ts_senddata_cb(0, &t);
#else
    ZCB_ts_senddata_cb(0, NULL);
#endif
}

extern void fc_timer_expired(void *nthing);
void fire_fc_timer()
{
    fc_timer_expired(NULL);
}

extern void test_rx_timer_expired(void *);
void fire_rx_timer(){
    test_rx_timer_expired(0);
}

void ask_TS_to_send()
{
    ZW_TransportService_SendData(&p, raw_data2, sizeof(raw_data2), status_callback);
}

void ask_TS_to_receive(unsigned char *cmd, unsigned int len)
{
    TransportService_ApplicationCommandHandler(&p, cmd, len);
}

int print_failed_if_nonzero(int ret, const char *test_name)
{   
   if( ret != 0) {
        v_printf("\t%s failed <-------------\n", test_name);
        return 1;
    } else {
        v_printf("\t%s passed\n", test_name);
        return 0;
    }
}
/* -------------- Test transport service's Receiving functionality ---------------------------------------*/

/*
    1. Send first fragment
    2. send second subseq fragment
    3. send third subseq fragment
    4. check if fragment complete is sent
*/
int simple_test()
{
    int ret = 0;
    memset(output, 0, sizeof(output));

    printf("simple_test\n");
    ask_TS_to_receive(test_first_frag1, sizeof(test_first_frag1));
    ask_TS_to_receive(test_subseq_frag2, sizeof(test_subseq_frag2));
    ask_TS_to_receive(test_subseq_frag3, sizeof(test_subseq_frag3));

    // Did the fragments get "glued" together into the expected datagram?
    ret = compare_received_datagram(test_complete_datagram, sizeof(test_complete_datagram));
    ret = print_failed_if_nonzero(ret, "Datagram re-assembly from fragments:");

    // Did the transport service module respond with COMMAND_FRAGMENT_COMPLETE?
    ret = memcmp(output, test_frag_compl, sizeof(test_frag_compl));
    ret = print_failed_if_nonzero(ret, "FRAGMENT_COMPLETE response sent:");
    fail_if_nonzero(ret);

    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);
    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;

}

/*
    1. Send first fragment
    2. Send next two subseq fragments (different sizes)
    3. Send last subseq fragment
    4. Check if fragment complete is sent
*/
int simple_test_non_uniform_fragment_sizes()
{
    int ret = 0;
    memset(output, 0, sizeof(output));

    printf("simple_test_non_uniform_fragment_sizes\n");
    ask_TS_to_receive(test_first_frag1, sizeof(test_first_frag1));
    ask_TS_to_receive(test_subseq_frag2a, sizeof(test_subseq_frag2a));
    ask_TS_to_receive(test_subseq_frag2b, sizeof(test_subseq_frag2b));
    ask_TS_to_receive(test_subseq_frag3, sizeof(test_subseq_frag3));

    // Did the fragments get "glued" together into the expected datagram?
    ret = compare_received_datagram(test_complete_datagram, sizeof(test_complete_datagram));
    ret = print_failed_if_nonzero(ret, "Datagram re-assembly from fragments:");

    // Did the transport service module respond with COMMAND_FRAGMENT_COMPLETE?
    ret = memcmp(output, test_frag_compl, sizeof(test_frag_compl));
    ret = print_failed_if_nonzero(ret, "FRAGMENT_COMPLETE response sent:");
    fail_if_nonzero(ret);

    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);
    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;

}

/*
    1. Send first fragment
    2. Send next two subseq fragments (different sizes, reverse order)
    3. Send last subseq fragment
    4. Check if fragment complete is sent
*/
int simple_test_non_sequential_fragments()
{
    int ret = 0;
    memset(output, 0, sizeof(output));

    printf("simple_test_non_sequential_fragments\n");
    ask_TS_to_receive(test_first_frag1, sizeof(test_first_frag1));
    ask_TS_to_receive(test_subseq_frag2b, sizeof(test_subseq_frag2b));
    ask_TS_to_receive(test_subseq_frag2a, sizeof(test_subseq_frag2a));
    ask_TS_to_receive(test_subseq_frag3, sizeof(test_subseq_frag3));

    // Did the fragments get "glued" together into the expected datagram?
    ret = compare_received_datagram(test_complete_datagram, sizeof(test_complete_datagram));
    ret = print_failed_if_nonzero(ret, "Datagram re-assembly from fragments:");

    // Did the transport service module respond with COMMAND_FRAGMENT_COMPLETE?
    ret = memcmp(output, test_frag_compl, sizeof(test_frag_compl));
    ret = print_failed_if_nonzero(ret, "FRAGMENT_COMPLETE response sent:");
    fail_if_nonzero(ret);

    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);
    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;

}

/*
 * Send first and third fragment and see if Transport service sends request for second fragment
 */
int miss_one_frag()
{
    int ret = 0;
    memset(output, 0, sizeof(output));
    printf("miss_one_frag\n");

    ask_TS_to_receive(test_first_frag1, sizeof(test_first_frag1));
    ask_TS_to_receive(test_subseq_frag3, sizeof(test_subseq_frag3));
    fire_rx_timer();

    ret = memcmp(output, test_frag_req, sizeof(test_frag_req));
    ret = print_failed_if_nonzero(ret, "miss_one_frag frag_req check");
    fail_if_nonzero(ret);

    ask_TS_to_receive(test_subseq_frag2, sizeof(test_subseq_frag2));

    ret = compare_received_datagram(test_complete_datagram, sizeof(test_complete_datagram));
    ret = print_failed_if_nonzero(ret, "Datagram re-assembly from fragments:");

    fire_rx_timer();
    ret = memcmp(output, test_frag_compl, sizeof(test_frag_compl));
    ret = print_failed_if_nonzero(ret, "miss_one_frag rag_compl check");
    fail_if_nonzero(ret);
    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);
    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;
}

/*
 * Send first, second and fourt fragment and see if Transport service sends request for third fragment
 */
int miss_one_frag1()
{
    int ret = 0;
    memset(output, 0, sizeof(output));
    printf("miss_one_frag1\n");

    ask_TS_to_receive(test_first_frag1, sizeof(test_first_frag1));
    ask_TS_to_receive(test_subseq_frag2a, sizeof(test_subseq_frag2a));
    ask_TS_to_receive(test_subseq_frag3, sizeof(test_subseq_frag3));
    fire_rx_timer();

    ret = memcmp(output, test_frag_req1, sizeof(test_frag_req1));
    ret = print_failed_if_nonzero(ret, "miss_one_frag frag_req check");
    fail_if_nonzero(ret);

    ask_TS_to_receive(test_subseq_frag2b, sizeof(test_subseq_frag2b));

    ret = compare_received_datagram(test_complete_datagram, sizeof(test_complete_datagram));
    ret = print_failed_if_nonzero(ret, "Datagram re-assembly from fragments:");

    fire_rx_timer();
    ret = memcmp(output, test_frag_compl, sizeof(test_frag_compl));
    ret = print_failed_if_nonzero(ret, "miss_one_frag rag_compl check");
    fail_if_nonzero(ret);
    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);
    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;
}

/*  1. send first fragment
    2. Fire rx timer which will make the transport service request remaining fragments
    3. see fragment req is sent
    4. send second fragment
    5. send third fragment
    6. Check that fragment complete is sent
*/
int dont_send_one_frag()
{
    int ret = 0;
    memset(output, 0, sizeof(output));

    printf("dont_send_one_frag\n");

    ask_TS_to_receive(test_first_frag1, sizeof(test_first_frag1));
    fire_rx_timer();

    ret = memcmp(output, test_frag_req, sizeof(test_frag_req));
    ret = print_failed_if_nonzero(ret, "dont_send_one_frag frag_req check");
    fail_if_nonzero(ret);

    ask_TS_to_receive(test_subseq_frag2, sizeof(test_subseq_frag2));
    ask_TS_to_receive(test_subseq_frag3, sizeof(test_subseq_frag3));
    ret = memcmp(output, test_frag_compl, sizeof(test_frag_compl));
    ret = print_failed_if_nonzero(ret, "dont_send_one_frag frag_compl check");
    fail_if_nonzero(ret);
    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);
    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;
}

/* 1. send subseq fragment
   2. check that fragment wait is sent with 0 pending fragments
*/
int test_dont_send_first_frag()
{
    int ret = 0;
    memset(output, 0, sizeof(output));

    printf("test_dont_send_first_frag\n");
    ask_TS_to_receive(test_subseq_frag2, sizeof(test_subseq_frag2)); /* Skip sending first frag and send subseq frag instead */

    ret = memcmp(output, test_frag_wait_zero_pending, sizeof(test_frag_wait_zero_pending)); /* Check if we get fragment wait */
    ret = print_failed_if_nonzero(ret, "test_dont_send_first_frag frag_wait");
    fail_if_nonzero(ret);

    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);

    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;
}

/* 1. Send first fragment
   2. Send subseq fragment from different source node
   3. Test if fragment wait is sent
   4. send second subseq fragment from same source node as first fragment of step.1
   5. send third subseq fragment from same source node as first fragment of step.1
   6. Check that fragment complete is sent
*/

int test_frag_wait_fn()
{
    int ret = 0;
    memset(output, 0, sizeof(output));

    printf("test_frag_wait_fn\n");
    ask_TS_to_receive(test_first_frag1, sizeof(test_first_frag1)); /* Send first frag */

    p.snode = 0xf1;
    ask_TS_to_receive(test_subseq_frag2, sizeof(test_subseq_frag2)); /* Send subseq frag from different source node */
    p.snode = 0xff;

    ret = memcmp(output, test_frag_wait_three_pending, sizeof(test_frag_wait_two_pending));
    ret = print_failed_if_nonzero(ret, "test_frag_wait receive check ");
    fail_if_nonzero(ret);

    memset(output, 0, sizeof(test_frag_compl));
    ask_TS_to_receive(test_subseq_frag2, sizeof(test_subseq_frag2)); /* Send correct second subseq frag */
    ask_TS_to_receive(test_subseq_frag3, sizeof(test_subseq_frag3)); /* Send correct third subseq frag */

    ret = memcmp(output, test_frag_compl, sizeof(test_frag_compl)); /* Check if we recive fragment complete */
    ret = print_failed_if_nonzero(ret, "frag_complete receive check");

    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);
    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;

}
/* -------------- Test transport service's Sending functionality ---------------------------------------*/

extern TRANSPORT2_ST_T current_state;
/* Purpose of this test is to test the sending functionality of TS

Steps: 
1. Ask TS to send first frag and check
2. Ask TS to send second frag and check
3. Ask TS to receive Fragment_complete and see if it has processed the fragment complete
    by checking if the scb.current_dnode == 0
*/
int test_send_whole_datagram()
{
    int ret = 0;
    memset(output, 0, sizeof(output));

    printf("test_send_whole_datagram\n");
    ask_TS_to_send();

    ret = memcmp(output, test_first_frag1, sizeof(test_first_frag1));
    ret = print_failed_if_nonzero(ret, "tets_send_whole_data_gram first fragment");
    fail_if_nonzero(ret);

    fire_timer_btwn_2_frags(0);
    ret = memcmp(output, test_subseq_frag2, sizeof(test_subseq_frag2));
    ret = print_failed_if_nonzero(ret, "tets_send_whole_data_gram first subseq fragment");
    fail_if_nonzero(ret);

    fire_timer_btwn_2_frags(0);
    ret = memcmp(output, test_subseq_frag3, sizeof(test_subseq_frag3));
    ret = print_failed_if_nonzero(ret, "tets_send_whole_data_gram second subseq fragment");
    fail_if_nonzero(ret);

    p.snode = 0xfe;
    p.dnode = 0xff;
    ask_TS_to_receive(test_frag_compl, sizeof(test_frag_compl));
    p.snode = 0xff;
    p.dnode = 0xfe;

    ret = print_failed_if_nonzero(!(global_status == S2_TRANSMIT_COMPLETE_OK), "Did transmissino OK for sending session...");
    fail_if_nonzero(ret);
    ret = print_failed_if_nonzero(check_scb_current_dnode(), "tets_send_whole_data_gram frag_compl processing");
    fail_if_nonzero(ret);
    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);
    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;
}

/* Purpose of this test is to send fragment request frame with different session ID,
    while transport service is in the middle of sending different session ID. This test checks if
    the transport service sending side fires the fragment complete timer. Which
    it should not do as this fragment has to be ignored. 

Steps:
1. Ask the TS to send first frag and check
2. ask the TS to receive frament request with different session id
3. Check if fc_timer is fired
4. ask the TS to send second frag and check
*/

int test_jakob()
{
    int ret = 0;
    unsigned backup_byte;
    memset(output, 0, sizeof(output));

    printf("test_jakob\n");
    ask_TS_to_send();

    ret = memcmp(output, test_first_frag1, sizeof(test_first_frag1));
    ret = print_failed_if_nonzero(ret, "test_jakob first fragment");
    fail_if_nonzero(ret);

    backup_byte = test_frag_req[2]; /*Change the session ID of frag_req cmd */
    test_frag_req[2] = 0xA0;
    fc_timer_counter = 0;
    p.snode = 0xfe;
    p.dnode = 0xff;
    ask_TS_to_receive(test_frag_req, sizeof(test_frag_req)); /* <-- Send fragment request with different session id */
    ret = print_failed_if_nonzero(fc_timer_counter, "test_send_frag_req_with_diff_session_id(jakob)");
    fail_if_nonzero(ret);
    test_frag_req[2] = backup_byte;

    fire_timer_btwn_2_frags(0);
    ret = memcmp(output, test_subseq_frag2, sizeof(test_subseq_frag2));
    ret = print_failed_if_nonzero(ret, "test_jakob first subseq fragment");
    fail_if_nonzero(ret);

    fire_timer_btwn_2_frags(0);
    ret = memcmp(output, test_subseq_frag3, sizeof(test_subseq_frag3));
    ret = print_failed_if_nonzero(ret, "test_jakob second subseq fragment");
    fail_if_nonzero(ret);

    p.snode = 0xfe;
    p.dnode = 0xff;
    ask_TS_to_receive(test_frag_compl, sizeof(test_frag_compl)); /* <-- Send fragment request with different session id */

    ret = print_failed_if_nonzero(!(global_status == S2_TRANSMIT_COMPLETE_OK), "Did transmissino OK for sending session...");
    fail_if_nonzero(ret);

    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);

    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;
    

}

/* Purpose of this test is to see if the sending side tie breaks on receival of segment
steps:
1. set source node as more than destination node
2. ask transport service to send 
3. set source node as less than destination node
4. ask TS to receive first frag
        At this point tie must be broken on receiving side as
        - TS is in Sending mode (step 1)
        - We, the sender of the new fragment(step 4) is receiver of current datagram transmission(step 1) from TS in progress
        - Node id of TS is less than ours
5. check the flag_tie_broken
6. ask TS to receive second frag
7. check if we recive frag_compl
*/
int test_tie_break()
{
    int ret = 0;
    memset(output, 0, sizeof(output));

    printf("test_tie_break\n");
    p.snode = 0xfe;
    p.dnode = 0xff;
    ask_TS_to_send();
    p.snode = 0xff;
    p.dnode = 0xfe;
    MyNodeID = 0xfe;

    ask_TS_to_receive(test_first_frag1, sizeof(test_first_frag1)); /* <-- ask to receive*/
    ret = print_failed_if_nonzero(!check_flag_tie_broken(), "Flag tie broken");
    fail_if_nonzero(ret);
    ask_TS_to_receive(test_subseq_frag2, sizeof(test_subseq_frag2));/* <-- ask to receive*/
    ask_TS_to_receive(test_subseq_frag3, sizeof(test_subseq_frag3));/* <-- ask to receive*/
//    fire_rx_timer();
    ret = memcmp(output, test_frag_compl, sizeof(test_frag_compl));

    ret = print_failed_if_nonzero(!(global_status == S2_TRANSMIT_COMPLETE_OK), "Did transmissino OK for sending session...");
    fail_if_nonzero(ret);

    ret = print_failed_if_nonzero(ret, "test_tie_break");
    fail_if_nonzero(ret);

    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);

    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;
}

/* Purpose of this test is to check if the on timeout of fc_timer does transport service
   send the last fragment again
steps:
1. ask TS to send first frag and check if we receive it
2. ask TS to send second frag and check if we receive it
3. ask TS to send second frag again by firing the fc_timer and check if we receive it
4. check if the flag_fc_timer_expired_once is 1
*/
int test_two_last_fragments()
{
    int ret = 0;
    memset(output, 0, sizeof(output));

    printf("test_two_last_fragments\n");
    ask_TS_to_send();

    ret = memcmp(output, test_first_frag1, sizeof(test_first_frag1));
    print_failed_if_nonzero(ret, "test_two_last_fragments first fragment");

    fire_timer_btwn_2_frags(0);/* <-- ask to send second frag */
    ret = memcmp(output, test_subseq_frag2, sizeof(test_subseq_frag2));
    ret = print_failed_if_nonzero(ret, "test_two_last_fragments second fragment");
    fail_if_nonzero(ret);

    fire_timer_btwn_2_frags(0);/* <-- ask to send second frag */
    ret = memcmp(output, test_subseq_frag3, sizeof(test_subseq_frag3));
    ret = print_failed_if_nonzero(ret, "test_two_last_fragments second fragment");
    fail_if_nonzero(ret);

    memset(output, 0, sizeof(test_subseq_frag3));
    fire_fc_timer(); /*Tell transport service that we did not receive the last fragment */
    ret = memcmp(output, test_subseq_frag3, sizeof(test_subseq_frag3));
    ret = print_failed_if_nonzero(ret, "test_two_last_fragments second fragment again");
    fail_if_nonzero(ret);

    ret = print_failed_if_nonzero(!check_flag_fc_timer_expired_once(), "fc_timer_expired_flag_check");
    fail_if_nonzero(ret);
    p.snode = 0xfe;
    p.dnode = 0xff;
    ask_TS_to_receive(test_frag_compl, sizeof(test_frag_compl));
    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    p.snode = 0xff;
    p.dnode = 0xfe;
    fail_if_nonzero(ret);
    ret = print_failed_if_nonzero(!(global_status == S2_TRANSMIT_COMPLETE_OK), "Did transmissino OK for sending session...");
    fail_if_nonzero(ret);

    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;
}

int test_send_frag_compl_from_diff_session()
{
    int ret = 0;
    memset(output, 0, sizeof(output));
    unsigned backup_byte;

    printf("test_send_frag_compl_from_diff_session\n");
    ask_TS_to_send(); /* <-- Start sending session */

    ret = memcmp(output, test_first_frag1, sizeof(test_first_frag1));
    ret = print_failed_if_nonzero(ret, "test_send_frag_compl_from_diff_session first fragment");
    fail_if_nonzero(ret);

    fire_timer_btwn_2_frags(0);
    ret = memcmp(output, test_subseq_frag2, sizeof(test_subseq_frag2));
    ret = print_failed_if_nonzero(ret, "test_send_frag_compl_from_diff_session second fragment");
    fail_if_nonzero(ret);

    fire_timer_btwn_2_frags(0);
    ret = memcmp(output, test_subseq_frag3, sizeof(test_subseq_frag3));
    ret = print_failed_if_nonzero(ret, "test_send_frag_compl_from_diff_session second fragment");
    fail_if_nonzero(ret);

    backup_byte = test_frag_compl[2];
    test_frag_compl[2] = 0xA0;
    fc_timer_counter = 0;
    p.snode = 0xfe;
    p.dnode = 0xff;
    ask_TS_to_receive(test_frag_compl, sizeof(test_frag_compl)); /* <-- Send fragment request with different session id */
    ret = print_failed_if_nonzero(fc_timer_counter, "test_send_frag_compl_from_diff_session fc_timer");
    fail_if_nonzero(ret);
    test_frag_compl[2] = backup_byte;
    ask_TS_to_receive(test_frag_compl, sizeof(test_frag_compl));
    p.snode = 0xff;
    p.dnode = 0xfe;
    ret = print_failed_if_nonzero(!(global_status == S2_TRANSMIT_COMPLETE_OK), "Did transmissino OK for sending session...");
    fail_if_nonzero(ret);
    ret = print_failed_if_nonzero(check_scb_current_dnode(), "test_send_frag_compl_from_diff_session frag_compl processing");
    fail_if_nonzero(ret); 
    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);
    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;
}
static void test_add_crc(uint8_t *buf, uint8_t len)
{
        uint8_t *tmp_buf = buf;
#ifdef ZGW
        uint16_t crc = ZW_CheckCrc16(0x1D0F, tmp_buf, len);
#else
        uint16_t crc = zgw_crc16(0x1D0F, tmp_buf, len);
#endif

        tmp_buf+=len;
        *tmp_buf++ =  (crc>>8)&0xff;
        *tmp_buf=(crc)&0xff;
}

void regenerate_crc(unsigned char *array, unsigned int len, unsigned char *crc)
{
        crc[0] = array[len-2];
        crc[1] = array[len-1];
        test_add_crc(array, len - 2);
}

void restore_crc(unsigned char *array, unsigned int len, unsigned char *crc)
{
        array[len-2] = crc[0];
        array[len-1] = crc[1];
}

int test_abort_transmission()
{
        int ret = 0;
        unsigned char backup_byte;
        unsigned char crc[2];

        printf("test_abort_transmission\n");
        p.snode = 0xfe;
        p.dnode = 0xff;
        ask_TS_to_send(); /* <-- Start sending session */
        p.snode = 0xff;
        p.dnode = 0xfe;
        MyNodeID = 0xfe;
        backup_byte = test_first_frag1[3];
        test_first_frag1[3] = 0xA0;

        regenerate_crc(test_first_frag1, sizeof(test_first_frag1), crc);
        ask_TS_to_receive(test_first_frag1, sizeof(test_first_frag1)); /* <-- ask to receive*/
        restore_crc(test_first_frag1, sizeof(test_first_frag1), crc);
        test_first_frag1[3] = backup_byte;

        ret = print_failed_if_nonzero(!check_flag_tie_broken(), "Flag tie broken");
        fail_if_nonzero(ret);

        fc_timer_counter = 0;
        ask_TS_to_receive(test_frag_req, sizeof(test_frag_req)); /* <-- Send fragment request with different session id */
        ret = print_failed_if_nonzero(fc_timer_counter, "test_abort_transmission frag_req should be ignored");

        test_subseq_frag2[3] = 0xA0;
        regenerate_crc(test_subseq_frag2, sizeof(test_subseq_frag2), crc);
        ask_TS_to_receive(test_subseq_frag2, sizeof(test_subseq_frag2));/* <-- ask to receive*/
        restore_crc(test_subseq_frag2, sizeof(test_subseq_frag2), crc);
        test_subseq_frag2[3] = backup_byte;

        test_subseq_frag3[3] = 0xA0;
        regenerate_crc(test_subseq_frag3, sizeof(test_subseq_frag3), crc);
        ask_TS_to_receive(test_subseq_frag3, sizeof(test_subseq_frag3));/* <-- ask to receive*/
        restore_crc(test_subseq_frag3, sizeof(test_subseq_frag3), crc);
        test_subseq_frag3[3] = backup_byte;

        backup_byte = test_frag_compl[2];
        test_frag_compl[2] = 0xA0;
        ret = memcmp(output, test_frag_compl, sizeof(test_frag_compl));
        ret = print_failed_if_nonzero(ret, "frag_complete receive check");
        test_frag_compl[2] = backup_byte; 
        fail_if_nonzero(ret);
        ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
        fail_if_nonzero(ret);
        printf("passed\n");
        return 0;
fail:
        printf("failed\n");
        return 1;
}

int test_fc_timer_after_frag_req()
{   
        int ret = 0;
        memset(output, 0, sizeof(output));

        printf("test_fc_timer_after_frag_req\n");
        ask_TS_to_send(); /* <-- ask to send first frag */

        ret = memcmp(output, test_first_frag1, sizeof(test_first_frag1));
        print_failed_if_nonzero(ret, "test_fc_timer_after_frag_req first fragment");

        fire_timer_btwn_2_frags(0);/* <-- ask to send second frag */
        ret = memcmp(output, test_subseq_frag2, sizeof(test_subseq_frag2));
        ret = print_failed_if_nonzero(ret, "test_fc_timer_after_frag_req second fragment");
        fail_if_nonzero(ret);

        fire_timer_btwn_2_frags(0);/* <-- ask to send second frag */
        ret = memcmp(output, test_subseq_frag3, sizeof(test_subseq_frag3));
        ret = print_failed_if_nonzero(ret, "test_fc_timer_after_frag_req second fragment");
        fail_if_nonzero(ret);

        memset(output, 0, sizeof(test_subseq_frag3));

        p.snode = 0xfe;
        p.dnode = 0xff;
        ask_TS_to_receive(test_frag_req, sizeof(test_frag_req));
        fire_fc_timer();
        ret = print_failed_if_nonzero(!(global_status == S2_TRANSMIT_COMPLETE_FAIL), "Did transmissino fail...");
        fail_if_nonzero(ret);
        ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
        fail_if_nonzero(ret);

        printf("passed\n");
        return 0;
fail:
        printf("failed\n");
        return 1;


}

int test_fc_timer_after_frag_compl_of_aborted_transmission()
{
        int ret = 0;
        unsigned char backup_byte;
        unsigned char crc[2];
        memset(output, 0, sizeof(output));

        printf("test_fc_timer_after_frag_compl_of_aborted_transmission\n");
        p.snode = 0xfe;
        p.dnode = 0xff;
        ask_TS_to_send(); /* <-- ask to send */

        ret = memcmp(output, test_first_frag1, sizeof(test_first_frag1));
        ret = print_failed_if_nonzero(ret, "test_fc_timer_after_frag_compl_of_aborted_transmission first fragment");
        fail_if_nonzero(ret); 

        fire_timer_btwn_2_frags(0); /* ask to send subseq frag */
        ret = memcmp(output, test_subseq_frag2, sizeof(test_subseq_frag2));
        ret = print_failed_if_nonzero(ret, "test_fc_timer_after_frag_compl_of_aborted_transmission second fragment");
        fail_if_nonzero(ret); 

        fire_timer_btwn_2_frags(0); /* ask to send subseq frag */
        ret = memcmp(output, test_subseq_frag3, sizeof(test_subseq_frag3));
        ret = print_failed_if_nonzero(ret, "test_fc_timer_after_frag_compl_of_aborted_transmission second fragment");
        fail_if_nonzero(ret); 

        p.snode = 0xff;  /* setup to do tie break */
        p.dnode = 0xfe;
        MyNodeID = 0xfe;

        backup_byte = test_first_frag1[3];
        test_first_frag1[3] = 0xA0;
        regenerate_crc(test_first_frag1, sizeof(test_first_frag1), crc);
        ask_TS_to_receive(test_first_frag1, sizeof(test_first_frag1)); /* <-- ask to receive with session id:10 */
        restore_crc(test_first_frag1, sizeof(test_first_frag1), crc);
        test_first_frag1[3] = backup_byte;

        ret = print_failed_if_nonzero(!check_flag_tie_broken(), "Flag tie broken");
        fail_if_nonzero(ret);

        fire_fc_timer();
        ret = print_failed_if_nonzero(!(global_status == S2_TRANSMIT_COMPLETE_FAIL), "Did transmissino fail for sending session...");
        fail_if_nonzero(ret);

        test_subseq_frag2[3] = 0xA0;
        regenerate_crc(test_subseq_frag2, sizeof(test_subseq_frag2), crc);
        ask_TS_to_receive(test_subseq_frag2, sizeof(test_subseq_frag2));/* <-- ask to receive subseq frag with session id:10 */
        restore_crc(test_subseq_frag2, sizeof(test_subseq_frag2), crc);
        test_subseq_frag2[3] = backup_byte;

        test_subseq_frag3[3] = 0xA0;
        regenerate_crc(test_subseq_frag3, sizeof(test_subseq_frag3), crc);
        ask_TS_to_receive(test_subseq_frag3, sizeof(test_subseq_frag3));/* <-- ask to receive subseq frag with session id:10 */
        restore_crc(test_subseq_frag3, sizeof(test_subseq_frag3), crc);
        test_subseq_frag3[3] = backup_byte;

        backup_byte = test_frag_compl[2]; /* Check if frag compl received for session id:10 */
        test_frag_compl[2] = 0xA0;
        ret = memcmp(output, test_frag_compl, sizeof(test_frag_compl));
        ret = print_failed_if_nonzero(ret, "test_fc_timer_after_frag_compl_of_aborted_transmission frag_compl received check");
        test_frag_compl[2] = backup_byte;
        fail_if_nonzero(ret);

        ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
        fail_if_nonzero(ret);

        p.snode = 0xff;
        p.dnode = 0xfe;
        printf("passed\n");
        return 0;
fail:
        printf("failed\n");
        return 1;
}

int test_fc_timer_after_last_frag_twice()
{
    int ret = 0;
    memset(output, 0, sizeof(output));

    printf("test_fc_timer_after_last_frag_twice\n");
    ask_TS_to_send(); /* <-- ask to send first frag */

    ret = memcmp(output, test_first_frag1, sizeof(test_first_frag1));
    print_failed_if_nonzero(ret, "test_fc_timer_after_last_frag_twice first fragment");

    fire_timer_btwn_2_frags(0);/* <-- ask to send second frag */
    ret = memcmp(output, test_subseq_frag2, sizeof(test_subseq_frag2));
    ret = print_failed_if_nonzero(ret, "test_fc_timer_after_last_frag_twice second fragment");
    fail_if_nonzero(ret);

    fire_timer_btwn_2_frags(0);/* <-- ask to send second frag */
    ret = memcmp(output, test_subseq_frag3, sizeof(test_subseq_frag3));
    ret = print_failed_if_nonzero(ret, "test_fc_timer_after_last_frag_twice second fragment");
    fail_if_nonzero(ret);

    memset(output, 0, sizeof(test_subseq_frag3));

    fire_fc_timer(); /*Tell transport service that we did not receive the last fragment */
    ret = memcmp(output, test_subseq_frag3, sizeof(test_subseq_frag3));
    ret = print_failed_if_nonzero(ret, "test_fc_timer_after_last_frag_twice second fragment again");
    fail_if_nonzero(ret);

    ret = print_failed_if_nonzero(!check_flag_fc_timer_expired_once(), "fc_timer_expired_flag_check");
    fail_if_nonzero(ret);

    fire_fc_timer(); /*Tell transport service that we did not receive the last fragment */
    ret = print_failed_if_nonzero(!(global_status == S2_TRANSMIT_COMPLETE_FAIL), "Did transmissino fail for sending session...");
    fail_if_nonzero(ret);

    ret = print_failed_if_nonzero(check_scb_current_dnode(), "current_dnode is set to 0 check");
    fail_if_nonzero(ret); 
    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);
    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;
}

int test_frag_wait_for_completed_session()
{
    int ret = 0;
    memset(output, 0, sizeof(output));

    printf("test_frag_wait_for_completed_session\n");
    ask_TS_to_send();

    ret = memcmp(output, test_first_frag1, sizeof(test_first_frag1));
    ret = print_failed_if_nonzero(ret, "test_frag_wait_for_completed_session first fragment");
    fail_if_nonzero(ret);

    fire_timer_btwn_2_frags(0);
    ret = memcmp(output, test_subseq_frag2, sizeof(test_subseq_frag2));
    ret = print_failed_if_nonzero(ret, "test_frag_wait_for_completed_session second fragment");
    fail_if_nonzero(ret);

    fire_timer_btwn_2_frags(0);
    ret = memcmp(output, test_subseq_frag3, sizeof(test_subseq_frag3));
    ret = print_failed_if_nonzero(ret, "test_frag_wait_for_completed_session second fragment");
    fail_if_nonzero(ret);

    p.snode = 0xfe;
    p.dnode = 0xff;
    ask_TS_to_receive(test_frag_compl, sizeof(test_frag_compl));
    ask_TS_to_receive(test_frag_wait, sizeof(test_frag_wait));
    p.snode = 0xff;
    p.dnode = 0xfe;
    ret = print_failed_if_nonzero(!(global_status == S2_TRANSMIT_COMPLETE_OK), "Did transmissino OK for sending session...");
    fail_if_nonzero(ret);
    ret = print_failed_if_nonzero(check_scb_current_dnode(), "tets_send_whole_data_gram frag_compl processing");
    fail_if_nonzero(ret);
    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);
    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;
   
}

int three_node_test()
{
    int ret = 0;
    global_status = 0xff;
    memset(output, 0, sizeof(output));

    printf("three_node_stress_test\n");
    ask_TS_to_send();

    ret = memcmp(output, test_first_frag1, sizeof(test_first_frag1));
    ret = print_failed_if_nonzero(ret, "first fragment");
    fail_if_nonzero(ret);

    p.snode = 0xf1;
    ask_TS_to_receive(test_first_frag1, sizeof(test_first_frag1));

    fire_timer_btwn_2_frags(0);
    ret = memcmp(output, test_frag_wait_two_pending, sizeof(test_frag_wait_two_pending)); /* Check if we get fragment wait */
    ret = print_failed_if_nonzero(ret, "received fragment_wait check");
    fail_if_nonzero(ret);

    fire_timer_btwn_2_frags(0);
    ret = memcmp(output, test_subseq_frag2, sizeof(test_subseq_frag2));
    ret = print_failed_if_nonzero(ret, "first subseq fragment");
    fail_if_nonzero(ret);

    ask_TS_to_receive(test_first_frag1, sizeof(test_first_frag1));

    fire_timer_btwn_2_frags(0);
    ret = memcmp(output, test_frag_wait, sizeof(test_frag_wait)); /* Check if we get fragment wait */
    ret = print_failed_if_nonzero(ret, "received fragment_wait again check");
    fail_if_nonzero(ret);

    fire_timer_btwn_2_frags(0);
    ret = memcmp(output, test_subseq_frag3, sizeof(test_subseq_frag3));
    ret = print_failed_if_nonzero(ret, "second subseq fragment");
    fail_if_nonzero(ret);

    p.snode = 0xfe;
    p.dnode = 0xff;
    ask_TS_to_receive(test_frag_compl, sizeof(test_frag_compl));
    ret = print_failed_if_nonzero(!(global_status == S2_TRANSMIT_COMPLETE_OK), "Did transmissino OK for sending session...");
    fail_if_nonzero(ret);
    p.snode = 0xf1;

    ask_TS_to_receive(test_first_frag1, sizeof(test_first_frag1)); /* <-- ask to receive*/
    ask_TS_to_receive(test_subseq_frag2, sizeof(test_subseq_frag2));/* <-- ask to receive*/
    ask_TS_to_receive(test_subseq_frag3, sizeof(test_subseq_frag3));/* <-- ask to receive*/
//    fire_rx_timer();
    ret = memcmp(output, test_frag_compl, sizeof(test_frag_compl));
    ret = print_failed_if_nonzero(ret, "fragment complete");
    fail_if_nonzero(ret);

    return 0;
fail:
    return 1;
}

int send_big_datagram()
{
    int ret = 0;
    unsigned char backup[2];
    unsigned char crc[2];
    memset(output, 0, sizeof(output));

    printf("send_big_datagram\n");
    ask_TS_to_receive(test_first_frag1, sizeof(test_first_frag1));
    ask_TS_to_receive(test_subseq_frag2, sizeof(test_subseq_frag2));
    backup[0] = test_subseq_frag3[3];
    backup[1] = test_subseq_frag3[4];

    test_subseq_frag3[3] = ((DATAGRAM_SIZE_MAX+1) & 0x700) >> 8;
    test_subseq_frag3[4] = ((DATAGRAM_SIZE_MAX+1) & 0xff);

    regenerate_crc(test_subseq_frag3, sizeof(test_subseq_frag3), crc);
    ask_TS_to_receive(test_subseq_frag3, sizeof(test_subseq_frag3));
    restore_crc(test_subseq_frag3, sizeof(test_subseq_frag3), crc);
    test_subseq_frag3[3] = backup[0];
    test_subseq_frag3[4] = backup[1];
    fire_rx_timer();

    ret = memcmp(output, test_frag_req2, sizeof(test_frag_req2));
    ret = print_failed_if_nonzero(ret, "send_big_datagram frag_req received");
    fail_if_nonzero(ret);

    ask_TS_to_receive(test_subseq_frag3, sizeof(test_subseq_frag3));
    ret = memcmp(output, test_frag_compl, sizeof(test_frag_compl));
    ret = print_failed_if_nonzero(ret, "send_big_datagram frag_compl");
    fail_if_nonzero(ret);
    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);
    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;

}

int send_big_datagram2()
{
    int ret = 0;
    unsigned char backup[2];
    unsigned char crc[2];
    memset(output, 0, sizeof(output));

    printf("send_big_datagram2\n");
    backup[0] = test_first_frag1[1];
    backup[1] = test_first_frag1[2];
    test_first_frag1[1] = 0xC0 + (((DATAGRAM_SIZE_MAX+1) & 0x700) >> 8);
    test_first_frag1[2] = ((DATAGRAM_SIZE_MAX+1) & 0xff);
    regenerate_crc(test_first_frag1, sizeof(test_first_frag1), crc);
    ask_TS_to_receive(test_first_frag1, sizeof(test_first_frag1));
    restore_crc(test_first_frag1, sizeof(test_first_frag1), crc);
    test_first_frag1[1] = backup[0];
    test_first_frag1[2] = backup[1];

    backup[0] = test_subseq_frag2[1];
    backup[1] = test_subseq_frag2[2];
    test_subseq_frag2[1] = 0xE0 + (((DATAGRAM_SIZE_MAX+1) & 0x700) >> 8);
    test_subseq_frag2[2] = ((DATAGRAM_SIZE_MAX+1) & 0xff);
    regenerate_crc(test_subseq_frag2, sizeof(test_subseq_frag2), crc);
    ask_TS_to_receive(test_subseq_frag2, sizeof(test_subseq_frag2));
    restore_crc(test_subseq_frag2, sizeof(test_subseq_frag2), crc);
    test_subseq_frag2[1] = backup[0];
    test_subseq_frag2[2] = backup[1];

#if 0
    backup[0] = test_subseq_frag3[3];
    backup[1] = test_subseq_frag3[4];

    test_subseq_frag3[3] = ((DATAGRAM_SIZE_MAX+1) & 0x700) >> 8;
    test_subseq_frag3[4] = ((DATAGRAM_SIZE_MAX+1) & 0xff);

    regenerate_crc(test_subseq_frag3, sizeof(test_subseq_frag3), crc);
    ask_TS_to_receive(test_subseq_frag3, sizeof(test_subseq_frag3));
    restore_crc(test_subseq_frag3, sizeof(test_subseq_frag3), crc);
    test_subseq_frag3[3] = backup[0];
    test_subseq_frag3[4] = backup[1];
    fire_rx_timer();

    ret = memcmp(output, test_frag_req2, sizeof(test_frag_req2));
    ret = print_failed_if_nonzero(ret, "send_big_datagram frag_req received");
    fail_if_nonzero(ret);

    ask_TS_to_receive(test_subseq_frag3, sizeof(test_subseq_frag3));
    ret = memcmp(output, test_frag_wait_zero_pending, sizeof(test_frag_wait_zero_pending)); /* Check if we get fragment wait */
    ret = print_failed_if_nonzero(ret, "test_frag_wait_zero_pending");
    fail_if_nonzero(ret);
#endif
    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);
    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;

}
int check_output_generated(void)
/* Verify no output has been generated.
 * Assumes that output buffer has been zeroed before with command
 * Return false if no output generated since output buffer was cleared.
 * Return true  if output was generated.
 *
 * Usage:
 *   memset(output, 0, sizeof(output));
 *   ... Do something that should not generate output...
 *   fail_if_nonzero(!check_no_output());
 *
 *   */
{
  int i;
  for (i=0; i<sizeof(output); i++)
  {
    if(output[i])
    {
      return true;
    }
  }
  return false;
}

int send_first_frag_with_big_size()
/* Steps:
 *  1. Send a first fragment with too long total frame size
 *  2. Check that we ignore the segment and remain in state idle
 *  3. Fire rx_timer
 *  4. Check that no segment request is sent
 */
{
    int ret = 0;
    unsigned char backup[2];
    unsigned char crc[2];

    printf("send_first_frag_with_big_size\n");
    backup[0] = test_first_frag1[1];
    backup[1] = test_first_frag1[2];
    test_first_frag1[1] = COMMAND_FIRST_FRAGMENT | ((DATAGRAM_SIZE_MAX+1) & 0x700) >> 8;
    test_first_frag1[2] = ((DATAGRAM_SIZE_MAX+1) & 0xff);
    regenerate_crc(test_first_frag1, sizeof(test_first_frag1), crc);

    ask_TS_to_receive(test_first_frag1, sizeof(test_first_frag1));

    test_first_frag1[1] = backup[0];
    test_first_frag1[2] = backup[1];
    regenerate_crc(test_first_frag1, sizeof(test_first_frag1), crc);

    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);

    memset(output, 0, sizeof(output));
    fire_rx_timer();

    ret = check_output_generated();
    ret = print_failed_if_nonzero(ret, "sending segment request after too long first segment");
    fail_if_nonzero(ret);

    memset(output, 0, sizeof(output));
    ask_TS_to_receive(test_subseq_frag2, sizeof(test_subseq_frag2));
    ret = memcmp(output, test_frag_wait_zero_pending, sizeof(test_frag_wait_zero_pending)); /* Check if we get fragment wait */
    ret = print_failed_if_nonzero(ret, "correct subsequent without first triggers data");
    fail_if_nonzero(ret);

    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);

    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;


}

int call_ask_TS_to_receive_with_large_size()
{
    int ret = 0;
    
    printf("call_ask_TS_to_receive_with_large_size\n");
    ask_TS_to_receive(test_first_frag1, DATAGRAM_SIZE_MAX+1);
    ret = print_failed_if_nonzero(!call_with_large_value, "call_ask_TS_to_receive_with_large_size");
    fail_if_nonzero(ret);
    ret = print_failed_if_nonzero(!(current_state == ST_IDLE), "current state has to be ST_IDLE...");
    fail_if_nonzero(ret);
    printf("passed\n");
    return 0;
fail:
    printf("failed\n");
    return 1;
}

int main()
{
    memset(&p, 0, sizeof(ts_param_t));
    p.snode = 0xff;
    p.dnode = 0xfe;
    fail_if_nonzero(simple_test());
    fail_if_nonzero(simple_test_non_uniform_fragment_sizes());
    fail_if_nonzero(simple_test_non_sequential_fragments());
    fail_if_nonzero(dont_send_one_frag());
    fail_if_nonzero(miss_one_frag());
    fail_if_nonzero(miss_one_frag1());
    fail_if_nonzero(test_dont_send_first_frag());

    fail_if_nonzero(test_frag_wait_fn());
    fail_if_nonzero(test_send_whole_datagram());
    fail_if_nonzero(test_jakob());

    fail_if_nonzero(test_tie_break());
    fail_if_nonzero(test_two_last_fragments());
    fail_if_nonzero(test_send_frag_compl_from_diff_session());
    fail_if_nonzero(test_abort_transmission());
    fail_if_nonzero(test_fc_timer_after_frag_req());
    fail_if_nonzero(test_fc_timer_after_frag_compl_of_aborted_transmission());
    fail_if_nonzero(test_fc_timer_after_last_frag_twice());
    fail_if_nonzero(test_frag_wait_for_completed_session());

    fail_if_nonzero(three_node_test());
    fail_if_nonzero(send_big_datagram());
    fail_if_nonzero(send_first_frag_with_big_size());
    fail_if_nonzero(call_ask_TS_to_receive_with_large_size());
    fail_if_nonzero(send_big_datagram2());

    printf("All test passed\n");
    return 0;
fail:
    printf("Above test failed\n");
    return 1;
}
