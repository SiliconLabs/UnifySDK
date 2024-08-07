/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include <unity.h>

// Zigbee stack includes
#define SIGNED_ENUM
#include "af_mock.h"
#include "address-table_mock.h"
#include "ota-storage_mock.h"

#include "zigbee_host.h"
#include "zigbee_host_common.h"

#include "test_zigbee_host_callbacks_mock.h"

extern struct zigbeeHostState z3gwState;

/**
 * @brief  Setup the test suite
 * (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void)
{
  static struct zigbeeHostCallbacks callbacks = {
    .onOtaUpdateStarted   = callback_onOtaUpdateStarted,
    .onOtaUpdateCompleted = callback_onOtaUpdateCompleted,
  };
  z3gwState.callbacks = &callbacks;
}

/**
 * @brief  Teardown the test suite
 * (called once after all test_xxx functions are called)
 *
 * @param num_failures
 * @return int
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * @brief Setup before each test.
 *
 */
void setUp(void) {}

/**
 * @brief Teardown after each test.
 *
 */
void tearDown(void) {}

void test_ota_image_add_sanity(void)
{
  char *ota_path = "./image.ota";

  // ARRANGE
  sli_zigbee_af_ota_storage_add_image_file_ExpectAndReturn(ota_path,
                                             SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS);

  // ACT
  sl_status_t status = zigbeeHostAddOtaImage(ota_path);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK , status);
}

void test_ota_image_add_error(void)
{
  char *ota_path = "./image.ota";

  // ARRANGE
  sli_zigbee_af_ota_storage_add_image_file_ExpectAndReturn(ota_path,
                                             SL_ZIGBEE_AF_OTA_STORAGE_ERROR);

  // ACT
  sl_status_t status = zigbeeHostAddOtaImage(ota_path);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, status);
}

void test_ota_image_add_invalid_input(void)
{
  // ARRANGE

  // ACT
  sl_status_t status = zigbeeHostAddOtaImage(NULL);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, status);
}

// prototype for update started callback used by ota-server
void sl_zigbee_af_ota_server_update_started_cb(uint16_t manufacturerId,
                                                 uint16_t imageTypeId,
                                                 uint32_t firmwareVersion,
                                                 uint8_t maxDataSize,
                                                 uint32_t offset);
void test_ota_update_started_callback_sanity(void)
{
  sl_802154_long_addr_t eui64         = "\x83\x02\xFF\x92\x00\x56\xBC\xD2";
  uint16_t manufacturerId  = 0x2220;
  uint16_t imageTypeId     = 0x1000;
  uint32_t firmwareVersion = 0x00000020;
  uint8_t maxDataSize      = 0x80;
  uint32_t offset          = 0x00000020;

  // ARRANGE
  sl_zigbee_af_get_current_sender_eui64_ExpectAndReturn(NULL, SL_STATUS_OK );
  sl_zigbee_af_get_current_sender_eui64_IgnoreArg_address();
  sl_zigbee_af_get_current_sender_eui64_ReturnThruPtr_address(eui64);

  callback_onOtaUpdateStarted_Expect(eui64,
                                     manufacturerId,
                                     imageTypeId,
                                     firmwareVersion);

  // ACT
  sl_zigbee_af_ota_server_update_started_cb(manufacturerId,
                                              imageTypeId,
                                              firmwareVersion,
                                              maxDataSize,
                                              offset);

  // ASSERT (Handled by CMock)
}

void test_ota_update_started_callback_sender_not_found(void)
{
  uint16_t manufacturerId  = 0x2220;
  uint16_t imageTypeId     = 0x1000;
  uint32_t firmwareVersion = 0x00000020;
  uint8_t maxDataSize      = 0x80;
  uint32_t offset          = 0x00000020;

  // ARRANGE
  sl_zigbee_af_get_current_sender_eui64_ExpectAndReturn(NULL, SL_STATUS_INVALID_STATE );
  sl_zigbee_af_get_current_sender_eui64_IgnoreArg_address();

  // ACT
  sl_zigbee_af_ota_server_update_started_cb(manufacturerId,
                                              imageTypeId,
                                              firmwareVersion,
                                              maxDataSize,
                                              offset);

  // ASSERT (Handled by CMock)
}

// prototype for update completed callback used by ota-server
void sl_zigbee_af_ota_server_update_complete_cb(uint16_t manufacturerId,
                                                  uint16_t imageTypeId,
                                                  uint32_t firmwareVersion,
                                                  sl_802154_short_addr_t source,
                                                  uint8_t status);
void test_ota_update_completed_callback_sanity(void)
{
  sl_802154_long_addr_t eui64         = "\x83\x02\xFF\x92\x00\x56\xBC\xD2";
  sl_802154_short_addr_t nodeId       = 0x995A;
  uint16_t manufacturerId  = 0x2220;
  uint16_t imageTypeId     = 0x1000;
  uint32_t firmwareVersion = 0x00000020;
  sl_zigbee_af_status_t status     = SL_ZIGBEE_ZCL_STATUS_SUCCESS;

  // ARRANGE
  sl_zigbee_af_get_current_sender_eui64_ExpectAndReturn(NULL, SL_STATUS_OK );
  sl_zigbee_af_get_current_sender_eui64_IgnoreArg_address();
  sl_zigbee_af_get_current_sender_eui64_ReturnThruPtr_address(eui64);

  callback_onOtaUpdateCompleted_Expect(eui64,
                                       manufacturerId,
                                       imageTypeId,
                                       firmwareVersion,
                                       status);

  // ACT
  sl_zigbee_af_ota_server_update_complete_cb(manufacturerId,
                                               imageTypeId,
                                               firmwareVersion,
                                               nodeId,
                                               status);

  // ASSERT (Handled by CMock)
}

void test_ota_update_completed_callback_sender_not_found(void)
{
  sl_802154_short_addr_t nodeId       = 0x97F2;
  uint16_t manufacturerId  = 0x2220;
  uint16_t imageTypeId     = 0x1000;
  uint32_t firmwareVersion = 0x00000020;

  // ARRANGE
  sl_zigbee_af_get_current_sender_eui64_ExpectAndReturn(NULL, SL_STATUS_INVALID_STATE );
  sl_zigbee_af_get_current_sender_eui64_IgnoreArg_address();

  // ACT
  sl_zigbee_af_ota_server_update_complete_cb(manufacturerId,
                                               imageTypeId,
                                               firmwareVersion,
                                               nodeId,
                                               SL_ZIGBEE_ZCL_STATUS_SUCCESS);

  // ASSERT (Handled by CMock)
}
