/* © 2014 Silicon Laboratories Inc.
 */
/**
 * @file ZW_s2_key_scrambler.c
 * @author Christian Salmony Olsen
 * @date 31/08/2015
 * @brief This module scrambles and de-scrambles the S2 network key.
 */
#define DEBUG
#include <platform.h>
#ifdef LINUX_TEST
#include <string.h>
#include <aes.h>
#else
#include <ZW_stdint.h>
#include <ZW_typedefs.h>
#include <ZW_aes_api.h>
#include <ZW_mem_api.h>
#endif
#include <ZW_s2_key_scrambler.h>

#define BLOCK_LENGTH (16)

#ifdef LINUX_TEST
int *p;
#define SCRAMBLING_KEY_ADDRESS p
static uint8_t random_aes_input[BLOCK_LENGTH] = {
#else
static const uint8_t random_aes_input[BLOCK_LENGTH] = {
#endif
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
};

#ifdef LINUX_TEST
static uint8_t aes_key[BLOCK_LENGTH];
#endif
static uint8_t aes_output[BLOCK_LENGTH];

void ZW_s2_key_scrambler_descramble(uint8_t * p_scramble_key, uint8_t * p_key)
{

#ifdef LINUX_TEST
  memset(aes_key, 0x12, BLOCK_LENGTH);
  AES128_ECB_encrypt(random_aes_input, aes_key, aes_output);
#else

  // Step 1: Calculate AES output with random data from code flash.
  ZW_AES_ecb(p_scramble_key, random_aes_input, aes_output);
#endif

  // Step 2: XOR AES output with scrambled data and get non-scrambled data.
  {
    register uint8_t count_scram;
    for (count_scram = 0; count_scram < 16; count_scram++)
    {
      p_key[count_scram] = aes_output[count_scram] ^ p_key[count_scram];
    }
  }
}
