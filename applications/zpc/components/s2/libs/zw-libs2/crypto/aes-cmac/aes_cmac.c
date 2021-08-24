/* © 2017 Silicon Laboratories Inc.
 */
/**
 * @file aes_cmac.c
 * @author Christian Salmony Olsen
 * @date 22/06/2015
 * @brief Serves functions for calculating and verifying an AES-CMAC.
 * @details Based on the source code by Brian Gladman found in
 * https://code.google.com/p/kirk-engine/source/browse/trunk/cmac.c?r=9.
 *
 * The following licensing is taken from Brian's cmac header file.
 *
 * ############################################################################
 * ---------------------------------------------------------------------------
 * Copyright (c) 1998-2008, Brian Gladman, Worcester, UK. All rights reserved.
 *
 * LICENSE TERMS
 *
 * The redistribution and use of this software (with or without changes)
 * is allowed without the payment of fees or royalties provided that:
 *
 *  1. source code distributions include the above copyright notice, this
 *     list of conditions and the following disclaimer;
 *
 *  2. binary distributions include the above copyright notice, this list
 *     of conditions and the following disclaimer in their documentation;
 *
 *  3. the name of the copyright holder is not used to endorse products
 *     built using this software without specific written permission.
 *
 * DISCLAIMER
 *
 * This software is provided 'as is' with no explicit or implied warranties
 * in respect of its properties, including, but not limited to, correctness
 * and/or fitness for purpose.
 * ---------------------------------------------------------------------------
 * Issue Date: 6/10/2008
 *
 * ############################################################################
 *
 */

#include <stdint.h>
#include "aes.h"
#include "aes_cmac.h"

#define AES_128 0

static void xor_128(const uint8_t * a, const uint8_t * b, uint8_t * out)
{
  uint8_t count;
  for (count = 0; count < 16; count++)
  {
    out[count] = a[count] ^ b[count];
  }
}

static void leftshift_onebit(const uint8_t * input, uint8_t * output)
{
  int8_t count;
  uint8_t overflow = 0;

  for (count = 15; count >= 0; count--)
  {
    output[count] = input[count] << 1;
    output[count] |= overflow;
    overflow = (input[count] & 0x80) ? 1 : 0;
  }
}

/**
 * @brief Generates two sub keys based on a given key.
 * @param key 128-bit key.
 * @param K1 128-bit first sub key.
 * @param K2 128-bit second sub key.
 */
static void generate_subkey(const uint8_t * key, uint8_t * K1, uint8_t * K2)
{
  uint8_t L[16];
  uint8_t tmp[16];

  const uint8_t const_Zero[16] = {
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  const uint8_t const_Rb[16] = {
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87
  };

  /*
   * L := AES-128(Key, const_Zero);
   */
  AES128_ECB_encrypt((uint8_t *)const_Zero, key, L);

  if (0 == (L[0] & 0x80)) // if MSB(L) is equal to 0 then K1 := L << 1;
  {
    leftshift_onebit(L, K1);
  }
  else
  { // else K1 := (L << 1) XOR const_Rb;
    leftshift_onebit(L, tmp);
    xor_128(tmp,const_Rb, K1);
  }

  if (0 == (K1[0] & 0x80)) // if MSB(K1) is equal to 0 then K2 := K1 << 1;
  {
    leftshift_onebit(K1, K2);
  }
  else // else K2 := (K1 << 1) XOR const_Rb;
  {
    leftshift_onebit(K1, tmp);
    xor_128(tmp,const_Rb, K2);
  }
}

//void padding(unsigned char *lastb, unsigned char *pad, int length)
static void padding(const uint8_t * lastb, uint8_t * pad, const uint8_t length)
{
  int j;

  /* original last block */
  for (j = 0; j < 16; j++)
  {
    if (j < length)
    {
      pad[j] = lastb[j];
    }
    else if (j == length)
    {
      pad[j] = 0x80;
    }
    else
    {
      pad[j] = 0x00;
    }
  }
}

//void AES_CMAC ( unsigned char *key, unsigned char *input, int length, unsigned char *mac )
void aes_cmac_calculate(
        const uint8_t * key,
        const uint8_t * message,
        const uint16_t message_length,
        uint8_t * mac)
{
  uint8_t X[16];
  uint8_t Y[16];
  uint8_t M_last[16];
  uint8_t padded[16];
  uint8_t K1[16];
  uint8_t K2[16];
  uint8_t flag;
  uint8_t n; //int n;
  uint8_t i; //int i;

  generate_subkey(key, K1, K2);

  n = (message_length + 15) / 16; // n is number of rounds

  if (0 == n)
  {
    n = 1;
    flag = 0;
  }
  else
  {
    if (0 == (message_length % 16))  /* last block is a complete block */
    {
      flag = 1;
    }
    else
    { /* last block is not complete block */
      flag = 0;
    }
  }

  if (flag) /* last block is complete block */
  {
    xor_128(&message[16 * (n - 1)], K1, M_last);
  }
  else
  {
    padding(&message[16 * (n - 1)], padded, message_length % 16);
    xor_128(padded, K2, M_last);
  }

  for (i = 0; i < 16; i++)
  {
    X[i] = 0;
  }

  for (i = 0; i < (n-1); i++)
  {
    xor_128(X, &message[16 * i], Y); /* Y := Mi (+) X  */
    AES128_ECB_encrypt(Y, key, X); // X := AES-128(key, Y);
  }

  xor_128(X, M_last, Y);
  AES128_ECB_encrypt(Y, key, X); // X := AES-128(key, Y);

  for (i = 0; i < 16; i++)
  {
    mac[i] = X[i];
  }
}

CMAC_VERIFY_T aes_cmac_verify(
        const uint8_t * key,
        const uint8_t * message,
        const uint16_t message_length,
        const uint8_t * mac)
{
  uint8_t calculated_mac[16];
  uint8_t count;

  aes_cmac_calculate(key, message, message_length, calculated_mac);

  for (count = 0; count < 16; count++)
  {
    if (mac[count] != calculated_mac[count])
    {
      return CMAC_INVALID;
    }
  }

  return CMAC_VALID;
}
