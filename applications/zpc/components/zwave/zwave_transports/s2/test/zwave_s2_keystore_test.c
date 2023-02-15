/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "zwave_s2_keystore.h"
#include "s2_keystore.h"
#include "zwave_s2_keystore_int.h"

#include "zwapi_protocol_mem.h"
#include "zwapi_protocol_basis.h"
#include "zwapi_init_mock.h"
#include "sl_log.h"
#include "sl_status.h"

#include "unity.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

const uint8_t my_priv_key[] = "MyPrivateKey.123MyPrivateKey.123";
static uint8_t chip_type    = 0;
static uint8_t the_nvr_version;

extern uint8_t dynamic_ecdh_private_key[32];

/// Override mock implementation of zwapi_nvr_get_value
sl_status_t
  zwapi_nvr_get_value(uint8_t bOffset, uint8_t bLength, uint8_t *bRetBuffer)
{
  if (bOffset == offsetof(NVR_FLASH_STRUCT, bRevision)) {
    *bRetBuffer = the_nvr_version;
  } else if (bOffset == offsetof(NVR_FLASH_STRUCT, aSecurityPrivateKey)) {
    memcpy(bRetBuffer, my_priv_key, bLength);
  } else {
    TEST_ASSERT_MESSAGE(0, "Wrong offset");
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

void test_read_private_ecdh_key()
{
  uint8_t key[32];
  const uint8_t my_ecdh_priv_key[] = ".This is the App NVM dynamic key";
  chip_type                        = ZW_GECKO_CHIP_700;

  nvm_config_set(ecdh_priv_key, my_ecdh_priv_key);
  memcpy(dynamic_ecdh_private_key, ".....This is the dynamic key....", 32);

  uint8_t version = 0;
  zwave_s2_keystore_set_ecdh_key_mode(ZWAVE_S2_KEYSTORE_DYNAMIC_ECDH_KEY);
  // We do not call zwapi_get_chip_type_version when the key is type of
  // ZWAVE_S2_KEYSTORE_DYNAMIC_ECDH_KEY so no need of setting mocks for
  // zwapi_get_chip_type_version()
  keystore_private_key_read(key);
  TEST_ASSERT_EQUAL_STRING_LEN(dynamic_ecdh_private_key, key, 32);

  zwapi_get_chip_type_version_Expect(&chip_type, &version);
  zwapi_get_chip_type_version_IgnoreArg_type();
  zwapi_get_chip_type_version_ReturnThruPtr_type(&chip_type);
  zwave_s2_keystore_set_ecdh_key_mode(ZWAVE_S2_KEYSTORE_STATIC_ECDH_KEY);
  keystore_private_key_read(key);
  TEST_ASSERT_EQUAL_STRING_LEN(my_priv_key, key, 32);

  zwapi_get_chip_type_version_Expect(&chip_type, &version);
  zwapi_get_chip_type_version_IgnoreArg_type();
  zwapi_get_chip_type_version_ReturnThruPtr_type(&chip_type);
  chip_type       = ZW_500_SERIES_CHIP_TYPE;
  the_nvr_version = 2;
  keystore_private_key_read(key);
  TEST_ASSERT_EQUAL_STRING_LEN(my_priv_key, key, 32);

  zwapi_get_chip_type_version_Expect(&chip_type, &version);
  zwapi_get_chip_type_version_IgnoreArg_type();
  zwapi_get_chip_type_version_ReturnThruPtr_type(&chip_type);
  the_nvr_version = 1;
  keystore_private_key_read(key);
  TEST_ASSERT_EQUAL_STRING_LEN(my_ecdh_priv_key, key, 32);
}

void test_sl_log_keys()
{
  // Nothing to test here really.
  zwave_s2_log_security_keys(SL_LOG_INFO);
  uint8_t assigned_keys = 0xFF;
  nvm_config_set(assigned_keys, &assigned_keys);
  zwave_s2_save_security_keys("/tmp/test.txt");
}

void test_zwave_s2_keystore_reset_and_get_assigned_keys()
{
  uint8_t assigned_keys = 0xFF;
  nvm_config_set(assigned_keys, &assigned_keys);
  zwave_s2_keystore_reset_assigned_keys();

  TEST_ASSERT_EQUAL(0, zwave_s2_keystore_get_assigned_keys());
}
