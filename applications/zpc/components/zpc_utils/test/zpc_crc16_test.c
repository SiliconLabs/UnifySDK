/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "zpc_crc16.h"
#include "unity.h"
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
};

void test_zpc_crc16()
{
  uint16_t crc
    = zpc_crc16(0x1D0F, test_subseq_frag2, sizeof(test_subseq_frag2));
  TEST_ASSERT_EQUAL_HEX16(crc, 0x1d69);
}

