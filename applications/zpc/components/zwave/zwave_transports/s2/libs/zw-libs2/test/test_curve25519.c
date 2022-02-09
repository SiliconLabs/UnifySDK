/* © 2015 Silicon Laboratories Inc.
 */
/*
 * aes_cmac_unit_test.c
 *
 *  Created on: 25/06/2015
 *      Author: COlsen
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "unity.h"
#include <curve25519.h>
#include <wc_util.h>
#include <bigint.h>

#ifdef __C51__
/* Local stub implementation of ZW_WatchDogKick() when compiling for C51. */
void ZW_WatchDogKick()
{
  // Do nothing. Stub implementation.
}
#endif // __C51__

#ifndef NULL
#define NULL   ((void *) 0)
#endif

#define KEY_SIZE (32) // In bytes

static const uint8_t alice_secret_key[KEY_SIZE] = {
        0x77,0x07,0x6d,0x0a,0x73,0x18,0xa5,0x7d
        ,0x3c,0x16,0xc1,0x72,0x51,0xb2,0x66,0x45
        ,0xdf,0x4c,0x2f,0x87,0xeb,0xc0,0x99,0x2a
        ,0xb1,0x77,0xfb,0xa5,0x1d,0xb9,0x2c,0x2a
};

/*static const uint8_t bob_secret_key[KEY_SIZE] = {
        0x5d,0xab,0x08,0x7e,0x62,0x4a,0x8a,0x4b
        ,0x79,0xe1,0x7f,0x8b,0x83,0x80,0x0e,0xe6
        ,0x6f,0x3b,0xb1,0x29,0x26,0x18,0xb6,0xfd
        ,0x1c,0x2f,0x8b,0x27,0xff,0x88,0xe0,0xeb
};*/

static uint8_t alice_public_key[KEY_SIZE];
#ifdef NOT_USED
static uint8_t bob_public_key[KEY_SIZE];
#endif

void test_alice_calculation_of_public_key(void)
{
  const uint8_t expected_alice_public_key[KEY_SIZE] = {
          0x85,0x20,0xf0,0x09,0x89,0x30,0xa7,0x54
          ,0x74,0x8b,0x7d,0xdc,0xb4,0x3e,0xf7,0x5a
          ,0x0d,0xbf,0x3a,0x0d,0x26,0x38,0x1a,0xf4
          ,0xeb,0xa4,0xa9,0x8e,0xaa,0x9b,0x4e,0x6a
  };

  crypto_scalarmult_curve25519_base(alice_public_key, alice_secret_key);

  UNITY_TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_alice_public_key, alice_public_key, KEY_SIZE, __LINE__, "");
}

void test_bigint_calc(void)
{
  uint32_t i = 0x12345678;

  const uint8_t some_data1[8] = {
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12
  };
  const uint8_t some_data2[8] = {
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12
  };

  uint8_t result[16];

  memset(result, 0x00, sizeof(result));

  bigint_mul(result, some_data1, some_data2, 8);

  print16(result);

  print_txt("------------------------------------");

  if ( *(char *)&i == 0x12 )
  {
    print_txt("Big endian\n");
//    ZW_DEBUG_MYPRODUCT_SEND_STR("BIG endian.");
  }
  else if ( *(char *)&i == 0x78 )
  {
    print_txt("Little endian\n");
//    ZW_DEBUG_MYPRODUCT_SEND_STR("LITTLE endian.");
  }
}

#ifdef NOT_USED

//void test_bob_calculation_of_public_key(void)
{
  const uint8_t expected_bob_public_key[KEY_SIZE] = {
          0xde,0x9e,0xdb,0x7d,0x7b,0x7d,0xc1,0xb4
          ,0xd3,0x5b,0x61,0xc2,0xec,0xe4,0x35,0x37
          ,0x3f,0x83,0x43,0xc8,0x5b,0x78,0x67,0x4d
          ,0xad,0xfc,0x7e,0x14,0x6f,0x88,0x2b,0x4f
  };

  crypto_scalarmult_curve25519_base(bob_public_key, bob_secret_key);

  UNITY_TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_bob_public_key, bob_public_key, KEY_SIZE, __LINE__, "");
}

//void test_alice_bob_shared_secret(void)
{
  const uint8_t expected_shared_secret[KEY_SIZE] = {
          0x4a,0x5d,0x9d,0x5b,0xa4,0xce,0x2d,0xe1
          ,0x72,0x8e,0x3b,0xf4,0x80,0x35,0x0f,0x25
          ,0xe0,0x7e,0x21,0xc9,0x47,0xd1,0x9e,0x33
          ,0x76,0xf0,0x9b,0x3c,0x1e,0x16,0x17,0x42
  };
  uint8_t k[KEY_SIZE];

  memset(k, 0x00, sizeof(k));

  crypto_scalarmult_curve25519(
          k,
          alice_secret_key,
          bob_public_key);

  UNITY_TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_shared_secret, k, KEY_SIZE, __LINE__, "");

  memset(k, 0x00, sizeof(k));

  crypto_scalarmult_curve25519(
          k,
          bob_secret_key,
          alice_public_key);

  UNITY_TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_shared_secret, k, KEY_SIZE, __LINE__, "");
}

//void test_10_shared_secrets(void)
{
  uint8_t person1_secret_key[KEY_SIZE];
  uint8_t person1_public_key[KEY_SIZE];
  uint8_t person1_shared_key_calc[KEY_SIZE];

  uint8_t person2_secret_key[KEY_SIZE];
  uint8_t person2_public_key[KEY_SIZE];
  uint8_t person2_shared_key_calc[KEY_SIZE];

  uint16_t count;
  uint8_t key_count;

  for (count = 0; count < 10; count++)
  {
    for (key_count = 0; key_count < KEY_SIZE; key_count++)
    {
      person1_secret_key[key_count] = (uint8_t)(rand() & 0xFF);
      person2_secret_key[key_count] = (uint8_t)(rand() & 0xFF);
    }
    crypto_scalarmult_curve25519_base(person1_public_key, person1_secret_key);
    crypto_scalarmult_curve25519_base(person2_public_key, person2_secret_key);

    crypto_scalarmult_curve25519(
            person1_shared_key_calc,
            person1_secret_key,
            person2_public_key);

    crypto_scalarmult_curve25519(
            person2_shared_key_calc,
            person2_secret_key,
            person1_public_key);

    UNITY_TEST_ASSERT_EQUAL_UINT8_ARRAY(person1_shared_key_calc, person2_shared_key_calc, KEY_SIZE, __LINE__, "");

    memset(person1_shared_key_calc, 0x00, KEY_SIZE);
    memset(person2_shared_key_calc, 0x01, KEY_SIZE);
  }
}

#endif
