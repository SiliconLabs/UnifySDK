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
  emAfOtaStorageAddImageFile_ExpectAndReturn(ota_path,
                                             EMBER_AF_OTA_STORAGE_SUCCESS);

  // ACT
  EmberStatus status = zigbeeHostAddOtaImage(ota_path);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_SUCCESS, status);
}

void test_ota_image_add_error(void)
{
  char *ota_path = "./image.ota";

  // ARRANGE
  emAfOtaStorageAddImageFile_ExpectAndReturn(ota_path,
                                             EMBER_AF_OTA_STORAGE_ERROR);

  // ACT
  EmberStatus status = zigbeeHostAddOtaImage(ota_path);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_ERR_FATAL, status);
}

void test_ota_image_add_invalid_input(void)
{
  // ARRANGE

  // ACT
  EmberStatus status = zigbeeHostAddOtaImage(NULL);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_BAD_ARGUMENT, status);
}

// prototype for update started callback used by ota-server
void emberAfPluginOtaServerUpdateStartedCallback(uint16_t manufacturerId,
                                                 uint16_t imageTypeId,
                                                 uint32_t firmwareVersion,
                                                 uint8_t maxDataSize,
                                                 uint32_t offset);
void test_ota_update_started_callback_sanity(void)
{
  EmberEUI64 eui64         = "\x83\x02\xFF\x92\x00\x56\xBC\xD2";
  uint16_t manufacturerId  = 0x2220;
  uint16_t imageTypeId     = 0x1000;
  uint32_t firmwareVersion = 0x00000020;
  uint8_t maxDataSize      = 0x80;
  uint32_t offset          = 0x00000020;

  // ARRANGE
  emberAfGetCurrentSenderEui64_ExpectAndReturn(NULL, EMBER_SUCCESS);
  emberAfGetCurrentSenderEui64_IgnoreArg_address();
  emberAfGetCurrentSenderEui64_ReturnThruPtr_address(eui64);

  callback_onOtaUpdateStarted_Expect(eui64,
                                     manufacturerId,
                                     imageTypeId,
                                     firmwareVersion);

  // ACT
  emberAfPluginOtaServerUpdateStartedCallback(manufacturerId,
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
  emberAfGetCurrentSenderEui64_ExpectAndReturn(NULL, EMBER_INVALID_CALL);
  emberAfGetCurrentSenderEui64_IgnoreArg_address();

  // ACT
  emberAfPluginOtaServerUpdateStartedCallback(manufacturerId,
                                              imageTypeId,
                                              firmwareVersion,
                                              maxDataSize,
                                              offset);

  // ASSERT (Handled by CMock)
}

// prototype for update completed callback used by ota-server
void emberAfPluginOtaServerUpdateCompleteCallback(uint16_t manufacturerId,
                                                  uint16_t imageTypeId,
                                                  uint32_t firmwareVersion,
                                                  EmberNodeId source,
                                                  uint8_t status);
void test_ota_update_completed_callback_sanity(void)
{
  EmberEUI64 eui64         = "\x83\x02\xFF\x92\x00\x56\xBC\xD2";
  EmberNodeId nodeId       = 0x995A;
  uint16_t manufacturerId  = 0x2220;
  uint16_t imageTypeId     = 0x1000;
  uint32_t firmwareVersion = 0x00000020;
  EmberAfStatus status     = EMBER_ZCL_STATUS_SUCCESS;

  // ARRANGE
  emberAfGetCurrentSenderEui64_ExpectAndReturn(NULL, EMBER_SUCCESS);
  emberAfGetCurrentSenderEui64_IgnoreArg_address();
  emberAfGetCurrentSenderEui64_ReturnThruPtr_address(eui64);

  callback_onOtaUpdateCompleted_Expect(eui64,
                                       manufacturerId,
                                       imageTypeId,
                                       firmwareVersion,
                                       status);

  // ACT
  emberAfPluginOtaServerUpdateCompleteCallback(manufacturerId,
                                               imageTypeId,
                                               firmwareVersion,
                                               nodeId,
                                               status);

  // ASSERT (Handled by CMock)
}

void test_ota_update_completed_callback_sender_not_found(void)
{
  EmberNodeId nodeId       = 0x97F2;
  uint16_t manufacturerId  = 0x2220;
  uint16_t imageTypeId     = 0x1000;
  uint32_t firmwareVersion = 0x00000020;

  // ARRANGE
  emberAfGetCurrentSenderEui64_ExpectAndReturn(NULL, EMBER_INVALID_CALL);
  emberAfGetCurrentSenderEui64_IgnoreArg_address();

  // ACT
  emberAfPluginOtaServerUpdateCompleteCallback(manufacturerId,
                                               imageTypeId,
                                               firmwareVersion,
                                               nodeId,
                                               EMBER_ZCL_STATUS_SUCCESS);

  // ASSERT (Handled by CMock)
}
