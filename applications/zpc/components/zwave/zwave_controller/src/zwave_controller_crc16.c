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
#define CRC_POLY 0x1021
uint16_t zwave_controller_crc16(uint16_t crc16,
                                const uint8_t *data_buf,
                                unsigned long data_len)
{
  uint8_t WorkData;
  uint8_t NewBit;
  while (data_len--) {
    WorkData = *data_buf++;
    for (uint8_t bitMask = 0x80; bitMask != 0; bitMask >>= 1) {
      /* Align test bit with next bit of the message byte, starting with msb. */
      NewBit = ((WorkData & bitMask) != 0) ^ ((crc16 & 0x8000) != 0);
      crc16 <<= 1;
      if (NewBit) {
        crc16 ^= CRC_POLY;
      }
    }
  }
  return crc16;
}
