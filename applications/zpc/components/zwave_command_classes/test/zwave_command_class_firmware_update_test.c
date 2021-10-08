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
#include "zwave_command_class_firmware_update.h"
#include "zwave_command_class_firmware_update_internals.h"
#include "zwave_command_classes_utils.h"
#include "unity.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "zpc_attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "zwave_tx_scheme_selector.h"
#include "ota_time.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_network_management_mock.h"
#include "zwapi_protocol_basis_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_FWU_MD_##type

// Static variables
static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

static uint8_t u8_value        = 0;
static uint32_t u32_value      = 0;
static clock_time_t time_value = 0;

static void create_firmware_file(const char *file_name, size_t file_size)
{
  FILE *fp     = fopen(file_name, "wb");
  uint8_t data = 0;
  for (size_t i = 0; i < file_size; i++) {
    data = (uint8_t)i;
    fwrite(&data, sizeof(data), 1, fp);
  }
  fclose(fp);
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  // Create 2 test files in the current directory
  create_firmware_file("test_firmware.gbl", 200);
  create_firmware_file("test_firmware_empty.gbl", 0);
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  // Delete our fake firmware files
  remove("test_firmware.gbl");
  remove("test_firmware_empty.gbl");
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size = 0;
  u8_value            = 0;
  u32_value           = 0;

  zpc_attribute_store_test_helper_create_network();
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);
}

void test_zwave_command_class_firmware_update_configure_and_cancel()
{
  // Try to initiate a firmware transfer on a non-supporting node
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    command_class_firmware_update_initiate_firmware_update(
                      node_id,
                      endpoint_id,
                      0,
                      0,
                      "test_firmware.gbl"));

  // Create a supporting node data:
  attribute_store_node_t firmware_update_node
    = attribute_store_add_node(ATTRIBUTE(FWU), endpoint_id_node);
  attribute_store_node_t firmware_0_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW), firmware_update_node);
  u32_value = 0;  // Firmware ID 0
  attribute_store_set_reported(firmware_0_node, &u32_value, sizeof(u32_value));

  attribute_store_node_t firmware_0_id_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW_ID), firmware_0_node);
  u32_value = 340;  // ID for firmware 0 (can be anything)
  attribute_store_set_reported(firmware_0_id_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_0_upgradable_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW_UPGRADABLE), firmware_0_node);
  u32_value = 0xFF;
  attribute_store_set_reported(firmware_0_upgradable_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_activation_node
    = attribute_store_add_node(ATTRIBUTE(FWU_ACTIVATION), firmware_update_node);
  u32_value = (uint32_t) true;
  attribute_store_set_reported(firmware_activation_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_fragment_size_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FRAGMENT_SIZE),
                               firmware_update_node);

  u32_value = 100;
  attribute_store_set_reported(firmware_fragment_size_node,
                               &u32_value,
                               sizeof(u32_value));

  // Try to initiate a firmware transfer on a node, non-existing file
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    command_class_firmware_update_initiate_firmware_update(
                      node_id,
                      endpoint_id,
                      0,
                      0,
                      "test_firmware_wrong_file.gbl"));

  // Empty file
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    command_class_firmware_update_initiate_firmware_update(
                      node_id,
                      endpoint_id,
                      0,
                      0,
                      "test_firmware_empty.gbl"));

  // Wrong firmware target (non-existing)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    command_class_firmware_update_initiate_firmware_update(
                      node_id,
                      endpoint_id,
                      12,
                      24,
                      "test_firmware.gbl"));

  // Try to initiate a firmware transfer on a node, success case
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    command_class_firmware_update_initiate_firmware_update(
                      node_id,
                      endpoint_id,
                      0,
                      get_current_utc_current_time() - 1,
                      "test_firmware.gbl"));

  // Attribute Store verifications
  attribute_store_node_t firmware_transfer_node
    = attribute_store_get_node_child_by_type(firmware_update_node,
                                             ATTRIBUTE(FW_TRANSFER),
                                             0);
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(firmware_transfer_node,
                                                     REPORTED_ATTRIBUTE));
  attribute_store_get_desired(firmware_transfer_node,
                              &u32_value,
                              sizeof(u32_value));
  TEST_ASSERT_EQUAL(ONGOING, u32_value);

  attribute_store_node_t firmware_last_status_node
    = attribute_store_get_node_child_by_type(firmware_transfer_node,
                                             ATTRIBUTE(FW_TRANSFER_LAST_STATUS),
                                             0);
  attribute_store_get_reported(firmware_last_status_node,
                               &u32_value,
                               sizeof(u32_value));
  TEST_ASSERT_EQUAL(SUCCESS, u32_value);

  // Try to cancel the Firmware Update
  command_class_firmware_update_abort_ongoing_firmware_operation(node_id,
                                                                 endpoint_id);
  // Verify the attribute store, state IDLE and last status ABORTED
  attribute_store_get_desired(firmware_transfer_node,
                              &u32_value,
                              sizeof(u32_value));
  TEST_ASSERT_EQUAL(IDLE, u32_value);
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(firmware_transfer_node,
                                                     REPORTED_ATTRIBUTE));
  attribute_store_get_reported(firmware_last_status_node,
                               &u32_value,
                               sizeof(u32_value));
  TEST_ASSERT_EQUAL(ABORTED, u32_value);

  // Try to initiate a firmware transfer again, should work
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    command_class_firmware_update_initiate_firmware_update(
                      node_id,
                      endpoint_id,
                      0,
                      0,
                      "test_firmware.gbl"));
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(firmware_transfer_node,
                                                     REPORTED_ATTRIBUTE));
  attribute_store_get_desired(firmware_transfer_node,
                              &u32_value,
                              sizeof(u32_value));
  TEST_ASSERT_EQUAL(ONGOING, u32_value);
}

void test_zwave_command_class_firmware_abort_not_ongoing()
{
  // Create a supporting node data:
  attribute_store_node_t firmware_update_node
    = attribute_store_add_node(ATTRIBUTE(FWU), endpoint_id_node);

  // Try to abort when no transfer is ongoing:
  command_class_firmware_update_abort_ongoing_firmware_operation(node_id,
                                                                 endpoint_id);

  // Attribute Store verifications
  attribute_store_node_t firmware_transfer_node
    = attribute_store_get_node_child_by_type(firmware_update_node,
                                             ATTRIBUTE(FW_TRANSFER),
                                             0);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, firmware_transfer_node);
}

void test_zwave_command_class_firmware_abort_when_idle()
{
  // Try to initiate a firmware transfer on a non-supporting node
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    command_class_firmware_update_initiate_firmware_update(
                      node_id,
                      endpoint_id,
                      0,
                      0,
                      "test_firmware.gbl"));

  // Create a supporting node data:
  attribute_store_node_t firmware_update_node
    = attribute_store_add_node(ATTRIBUTE(FWU), endpoint_id_node);
  attribute_store_node_t firmware_0_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW), firmware_update_node);
  u32_value = 0;  // Firmware ID 0
  attribute_store_set_reported(firmware_0_node, &u32_value, sizeof(u32_value));

  attribute_store_node_t firmware_0_id_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW_ID), firmware_0_node);
  u32_value = 340;  // ID for firmware 0 (can be anything)
  attribute_store_set_reported(firmware_0_id_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_0_upgradable_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW_UPGRADABLE), firmware_0_node);
  u32_value = 0xFF;
  attribute_store_set_reported(firmware_0_upgradable_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_activation_node
    = attribute_store_add_node(ATTRIBUTE(FWU_ACTIVATION), firmware_update_node);
  u32_value = (uint32_t) true;
  attribute_store_set_reported(firmware_activation_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_fragment_size_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FRAGMENT_SIZE),
                               firmware_update_node);

  u32_value = 100;
  attribute_store_set_reported(firmware_fragment_size_node,
                               &u32_value,
                               sizeof(u32_value));

  // Try to initiate a firmware transfer on a node, success case
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    command_class_firmware_update_initiate_firmware_update(
                      node_id,
                      endpoint_id,
                      0,
                      get_current_utc_current_time() - 1,
                      "test_firmware.gbl"));

  // Attribute Store verifications
  attribute_store_node_t firmware_transfer_node
    = attribute_store_get_node_child_by_type(firmware_update_node,
                                             ATTRIBUTE(FW_TRANSFER),
                                             0);
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(firmware_transfer_node,
                                                     REPORTED_ATTRIBUTE));
  attribute_store_get_desired(firmware_transfer_node,
                              &u32_value,
                              sizeof(u32_value));
  TEST_ASSERT_EQUAL(ONGOING, u32_value);

  attribute_store_node_t firmware_last_status_node
    = attribute_store_get_node_child_by_type(firmware_transfer_node,
                                             ATTRIBUTE(FW_TRANSFER_LAST_STATUS),
                                             0);
  attribute_store_get_reported(firmware_last_status_node,
                               &u32_value,
                               sizeof(u32_value));
  TEST_ASSERT_EQUAL(SUCCESS, u32_value);

  // Try to cancel the Firmware Update
  command_class_firmware_update_abort_ongoing_firmware_operation(node_id,
                                                                 endpoint_id);
  // Verify the attribute store, state IDLE and last status ABORTED
  attribute_store_get_desired(firmware_transfer_node,
                              &u32_value,
                              sizeof(u32_value));
  TEST_ASSERT_EQUAL(IDLE, u32_value);
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(firmware_transfer_node,
                                                     REPORTED_ATTRIBUTE));
  attribute_store_get_reported(firmware_last_status_node,
                               &u32_value,
                               sizeof(u32_value));
  TEST_ASSERT_EQUAL(ABORTED, u32_value);

  // Set back the last status to SUCCESS:
  u32_value = SUCCESS;
  attribute_store_set_reported(firmware_last_status_node,
                               &u32_value,
                               sizeof(u32_value));

  // Try to cancel the Firmware Update, nothing should happen
  command_class_firmware_update_abort_ongoing_firmware_operation(node_id,
                                                                 endpoint_id);
  attribute_store_get_reported(firmware_last_status_node,
                               &u32_value,
                               sizeof(u32_value));
  TEST_ASSERT_EQUAL(SUCCESS, u32_value);
}

void test_zwave_command_class_firmware_update_no_activation()
{
  // Create a supporting node data:
  attribute_store_node_t firmware_update_node
    = attribute_store_add_node(ATTRIBUTE(FWU), endpoint_id_node);
  attribute_store_node_t firmware_0_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW), firmware_update_node);
  u32_value = 12;  // Firmware ID 12
  attribute_store_set_reported(firmware_0_node, &u32_value, sizeof(u32_value));

  attribute_store_node_t firmware_0_id_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW_ID), firmware_0_node);
  u32_value = 340;  // ID for firmware 0 (can be anything)
  attribute_store_set_reported(firmware_0_id_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_0_upgradable_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW_UPGRADABLE), firmware_0_node);
  u32_value = 0xFF;
  attribute_store_set_reported(firmware_0_upgradable_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_activation_node
    = attribute_store_add_node(ATTRIBUTE(FWU_ACTIVATION), firmware_update_node);
  u32_value = (uint32_t) false;
  attribute_store_set_reported(firmware_activation_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_fragment_size_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FRAGMENT_SIZE),
                               firmware_update_node);
  u32_value = 100;
  attribute_store_set_reported(firmware_fragment_size_node,
                               &u32_value,
                               sizeof(u32_value));

  // Try to initiate a firmware transfer on a node, success case
  // Year 2038
  long never_update = 2147483647;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    command_class_firmware_update_initiate_firmware_update(
                      node_id,
                      endpoint_id,
                      12,
                      never_update,
                      "test_firmware.gbl"));

  // Attribute Store verifications, here it should be IDLE/IDLE
  // waiting for the attribute store timeout
  attribute_store_node_t firmware_transfer_node
    = attribute_store_get_node_child_by_type(firmware_update_node,
                                             ATTRIBUTE(FW_TRANSFER),
                                             0);
  attribute_store_get_reported(firmware_transfer_node,
                               &u32_value,
                               sizeof(u32_value));
  TEST_ASSERT_EQUAL(IDLE, u32_value);
  attribute_store_get_desired(firmware_transfer_node,
                              &u32_value,
                              sizeof(u32_value));
  TEST_ASSERT_EQUAL(IDLE, u32_value);
}

void test_zwave_command_class_firmware_update_node_supports_firmware_activation()
{
  // Assume false by default.
  TEST_ASSERT_FALSE(node_supports_firmware_activation(endpoint_id_node));

  // Create a supporting node data:
  attribute_store_node_t firmware_update_node
    = attribute_store_add_node(ATTRIBUTE(FWU), endpoint_id_node);

  attribute_store_node_t firmware_activation_node
    = attribute_store_add_node(ATTRIBUTE(FWU_ACTIVATION), firmware_update_node);
  u32_value = (uint32_t) true;
  attribute_store_set_reported(firmware_activation_node,
                               &u32_value,
                               sizeof(u32_value));

  TEST_ASSERT_FALSE(node_supports_firmware_activation(endpoint_id_node));

  u32_value = 0xFF;
  attribute_store_set_reported(firmware_activation_node,
                               &u32_value,
                               sizeof(u32_value));
  TEST_ASSERT_FALSE(node_supports_firmware_activation(endpoint_id_node));

  zwave_cc_version_t version = 5;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));
  TEST_ASSERT_FALSE(node_supports_firmware_activation(endpoint_id_node));

  version = 7;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));
  TEST_ASSERT_TRUE(node_supports_firmware_activation(endpoint_id_node));

  u32_value = 0;
  attribute_store_set_reported(firmware_activation_node,
                               &u32_value,
                               sizeof(u32_value));
  TEST_ASSERT_FALSE(node_supports_firmware_activation(endpoint_id_node));
}

void test_zwave_command_class_firmware_update_node_supports_cc()
{
  // Assume false by default.
  TEST_ASSERT_FALSE(node_supports_cc(endpoint_id_node));

  // Create a supporting node data:
  attribute_store_node_t firmware_update_node
    = attribute_store_add_node(ATTRIBUTE(FWU), endpoint_id_node);

  attribute_store_node_t firmware_cc_node
    = attribute_store_add_node(ATTRIBUTE(FWU_CC), firmware_update_node);
  u32_value = (uint32_t) true;
  attribute_store_set_reported(firmware_cc_node, &u32_value, sizeof(u32_value));

  TEST_ASSERT_TRUE(node_supports_cc(endpoint_id_node));

  u32_value = 0x1;
  attribute_store_set_reported(firmware_cc_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_TRUE(node_supports_cc(endpoint_id_node));

  u32_value = 0;
  attribute_store_set_reported(firmware_cc_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_FALSE(node_supports_cc(endpoint_id_node));
}

void test_zwave_command_class_firmware_update_double_update_rejected()
{
  // Create a supporting node data:
  attribute_store_node_t firmware_update_node
    = attribute_store_add_node(ATTRIBUTE(FWU), endpoint_id_node);
  attribute_store_node_t firmware_0_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW), firmware_update_node);
  u32_value = 0;  // Firmware ID 0
  attribute_store_set_reported(firmware_0_node, &u32_value, sizeof(u32_value));

  attribute_store_node_t firmware_0_id_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW_ID), firmware_0_node);
  u32_value = 340;  // ID for firmware 0 (can be anything)
  attribute_store_set_reported(firmware_0_id_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_0_upgradable_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW_UPGRADABLE), firmware_0_node);
  u32_value = 0xFF;
  attribute_store_set_reported(firmware_0_upgradable_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_activation_node
    = attribute_store_add_node(ATTRIBUTE(FWU_ACTIVATION), firmware_update_node);
  u32_value = (uint32_t) true;
  attribute_store_set_reported(firmware_activation_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_fragment_size_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FRAGMENT_SIZE),
                               firmware_update_node);
  u32_value = 100;
  attribute_store_set_reported(firmware_fragment_size_node,
                               &u32_value,
                               sizeof(u32_value));

  // Try to initiate a firmware transfer on a node, success case
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    command_class_firmware_update_initiate_firmware_update(
                      node_id,
                      endpoint_id,
                      0,
                      24,
                      "test_firmware.gbl"));

  // Get the node to confirm that the firmware transfer is ongoing.
  attribute_store_node_t firmware_transfer_node
    = attribute_store_get_node_child_by_type(firmware_update_node,
                                             ATTRIBUTE(FW_TRANSFER),
                                             0);
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(firmware_transfer_node,
                                                     REPORTED_ATTRIBUTE));
  zwave_firmware_transfer_state_t state = ONGOING;
  attribute_store_set_reported(firmware_transfer_node, &state, sizeof(state));

  // Try to initiate a firmware transfer again ! it will be rejected
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    command_class_firmware_update_initiate_firmware_update(
                      node_id,
                      endpoint_id,
                      0,
                      0,
                      "test_firmware.gbl"));
}

void test_zwave_command_class_firmware_update_is_firmware_valid()
{
  // Create a supporting node data:
  attribute_store_node_t firmware_update_node
    = attribute_store_add_node(ATTRIBUTE(FWU), endpoint_id_node);
  attribute_store_node_t firmware_0_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW), firmware_update_node);
  u32_value = 0;  // Firmware ID 0
  attribute_store_set_reported(firmware_0_node, &u32_value, sizeof(u32_value));

  attribute_store_node_t firmware_0_id_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW_ID), firmware_0_node);
  u32_value = 340;  // ID for firmware 0 (can be anything)
  attribute_store_set_reported(firmware_0_id_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_0_upgradable_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW_UPGRADABLE), firmware_0_node);
  u32_value = 0xFF;
  attribute_store_set_reported(firmware_0_upgradable_node,
                               &u32_value,
                               sizeof(u32_value));
  attribute_store_node_t firmware_20_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW), firmware_update_node);
  u32_value = 20;  // Firmware ID 0
  attribute_store_set_reported(firmware_20_node, &u32_value, sizeof(u32_value));

  attribute_store_node_t firmware_20_id_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW_ID), firmware_0_node);
  u32_value = 340;  // ID for firmware 0 (can be anything)
  attribute_store_set_reported(firmware_20_id_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_20_upgradable_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW_UPGRADABLE), firmware_0_node);
  u32_value = 0xFF;
  attribute_store_set_reported(firmware_20_upgradable_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_activation_node
    = attribute_store_add_node(ATTRIBUTE(FWU_ACTIVATION), firmware_update_node);
  u32_value = (uint32_t) true;
  attribute_store_set_reported(firmware_activation_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_fragment_size_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FRAGMENT_SIZE),
                               firmware_update_node);
  u32_value = 100;
  attribute_store_set_reported(firmware_fragment_size_node,
                               &u32_value,
                               sizeof(u32_value));

  // Firmware ID 0 and 20 are valid
  TEST_ASSERT_TRUE(is_firmware_target_valid(endpoint_id_node, 0));
  TEST_ASSERT_TRUE(is_firmware_target_valid(endpoint_id_node, 20));
  TEST_ASSERT_FALSE(is_firmware_target_valid(endpoint_id_node, 1));
  TEST_ASSERT_FALSE(is_firmware_target_valid(endpoint_id_node, 19));
  TEST_ASSERT_FALSE(is_firmware_target_valid(endpoint_id_node, 21));
}

void test_zwave_command_class_firmware_update_is_firmware_update_ongoing()
{
  TEST_ASSERT_FALSE(is_firmware_update_ongoing(endpoint_id_node));

  // Create a supporting node data, and pretend it is ongoing firmware update:
  attribute_store_node_t firmware_update_node
    = attribute_store_add_node(ATTRIBUTE(FWU), endpoint_id_node);
  attribute_store_node_t firmware_transfer_node
    = attribute_store_add_node(ATTRIBUTE(FW_TRANSFER), firmware_update_node);

  u32_value = ONGOING;
  attribute_store_set_desired(firmware_transfer_node,
                              &u32_value,
                              sizeof(u32_value));

  // Still not ongoing.
  TEST_ASSERT_FALSE(is_firmware_update_ongoing(endpoint_id_node));
  attribute_store_set_reported(firmware_transfer_node,
                               &u32_value,
                               sizeof(u32_value));

  // Ongoing: No expiry and reported is ONGOING
  TEST_ASSERT_TRUE(is_firmware_update_ongoing(endpoint_id_node));

  attribute_store_set_reported(firmware_transfer_node,
                               &u32_value,
                               sizeof(u32_value));
  attribute_store_node_t expiry_time_node
    = attribute_store_add_node(ATTRIBUTE(FW_TRANSFER_EXPIRY_TIME),
                               firmware_transfer_node);
  time_value = clock_time() - 5;
  attribute_store_set_reported(expiry_time_node,
                               &time_value,
                               sizeof(time_value));

  // Still not ongoing (expired)
  TEST_ASSERT_FALSE(is_firmware_update_ongoing(endpoint_id_node));

  time_value = clock_time() + 15;
  attribute_store_set_reported(expiry_time_node,
                               &time_value,
                               sizeof(time_value));
  // Not expired:
  TEST_ASSERT_TRUE(is_firmware_update_ongoing(endpoint_id_node));
}

void test_zwave_command_class_firmware_update_postpone_expiry()
{
  // Create a supporting node data, and pretend it is ongoing firmware update:
  attribute_store_node_t firmware_update_node
    = attribute_store_add_node(ATTRIBUTE(FWU), endpoint_id_node);
  attribute_store_node_t firmware_transfer_node
    = attribute_store_add_node(ATTRIBUTE(FW_TRANSFER), firmware_update_node);

  u32_value = IDLE;
  attribute_store_set_desired(firmware_transfer_node,
                              &u32_value,
                              sizeof(u32_value));
  u32_value = ONGOING;
  attribute_store_set_reported(firmware_transfer_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t expiry_time_node
    = attribute_store_add_node(ATTRIBUTE(FW_TRANSFER_EXPIRY_TIME),
                               firmware_transfer_node);
  time_value = clock_time() - 5;
  attribute_store_set_reported(expiry_time_node,
                               &time_value,
                               sizeof(time_value));

  // Can't be postponed if expired:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    postpone_firmware_update_expiry(endpoint_id_node));

  clock_time_t current_time = clock_time();
  time_value                = clock_time() + 15;
  attribute_store_set_reported(expiry_time_node,
                               &time_value,
                               sizeof(time_value));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    postpone_firmware_update_expiry(endpoint_id_node));

  attribute_store_get_reported(expiry_time_node,
                               &time_value,
                               sizeof(time_value));

  TEST_ASSERT_TRUE(time_value >= (current_time + 60 * CLOCK_SECOND));
}

void test_zwave_command_class_firmware_no_fragment_size_data()
{
  // Create a supporting node data, missing the fragment size attribute
  attribute_store_node_t firmware_update_node
    = attribute_store_add_node(ATTRIBUTE(FWU), endpoint_id_node);
  attribute_store_node_t firmware_0_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW), firmware_update_node);
  u32_value = 0;  // Firmware ID 0
  attribute_store_set_reported(firmware_0_node, &u32_value, sizeof(u32_value));

  attribute_store_node_t firmware_0_id_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW_ID), firmware_0_node);
  u32_value = 340;  // ID for firmware 0 (can be anything)
  attribute_store_set_reported(firmware_0_id_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_0_upgradable_node
    = attribute_store_add_node(ATTRIBUTE(FWU_FW_UPGRADABLE), firmware_0_node);
  u32_value = 0xFF;
  attribute_store_set_reported(firmware_0_upgradable_node,
                               &u32_value,
                               sizeof(u32_value));

  attribute_store_node_t firmware_activation_node
    = attribute_store_add_node(ATTRIBUTE(FWU_ACTIVATION), firmware_update_node);
  u32_value = (uint32_t) true;
  attribute_store_set_reported(firmware_activation_node,
                               &u32_value,
                               sizeof(u32_value));

  // Try to initiate a firmware transfer on a node, it should be a failure
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    command_class_firmware_update_initiate_firmware_update(
                      node_id,
                      endpoint_id,
                      0,
                      24,
                      "test_firmware.gbl"));
}

void test_zwave_command_class_firmware_theoretical_fragment_size_lr_s2()
{
  attribute_store_node_t granted_keys_node
    = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);
  zwave_keyset_t keys = ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY;
  attribute_store_set_reported(granted_keys_node, &keys, sizeof(keys));

  attribute_store_node_t protocol_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                               node_id_node);
  zwave_protocol_t protocol = PROTOCOL_ZWAVE_LONG_RANGE;
  attribute_store_set_reported(protocol_node, &protocol, sizeof(protocol));

  zwave_endpoint_id_t endpoint_id = 0;
  attribute_store_set_reported(endpoint_id_node,
                               &endpoint_id,
                               sizeof(endpoint_id));

  TEST_ASSERT_EQUAL(
    152,
    firmware_transfer_get_theoretical_max_fragment_size(endpoint_id_node));
}

void test_zwave_command_class_firmware_theoretical_fragment_size_lr_s2_multi_channel()
{
  attribute_store_node_t granted_keys_node
    = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);
  zwave_keyset_t keys = ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY;
  attribute_store_set_reported(granted_keys_node, &keys, sizeof(keys));

  attribute_store_node_t protocol_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                               node_id_node);
  zwave_protocol_t protocol = PROTOCOL_ZWAVE_LONG_RANGE;
  attribute_store_set_reported(protocol_node, &protocol, sizeof(protocol));

  zwave_endpoint_id_t endpoint_id = 4;
  attribute_store_set_reported(endpoint_id_node,
                               &endpoint_id,
                               sizeof(endpoint_id));

  TEST_ASSERT_EQUAL(
    148,
    firmware_transfer_get_theoretical_max_fragment_size(endpoint_id_node));
}

void test_zwave_command_class_firmware_theoretical_fragment_size_unknown_protocol()
{
  attribute_store_node_t granted_keys_node
    = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);
  zwave_keyset_t keys = ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY;
  attribute_store_set_reported(granted_keys_node, &keys, sizeof(keys));

  attribute_store_node_t protocol_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                               node_id_node);
  zwave_protocol_t protocol = PROTOCOL_UNKNOWN;
  attribute_store_set_reported(protocol_node, &protocol, sizeof(protocol));

  zwave_endpoint_id_t endpoint_id = 4;
  attribute_store_set_reported(endpoint_id_node,
                               &endpoint_id,
                               sizeof(endpoint_id));

  TEST_ASSERT_EQUAL(
    ZWAVE_TX_SAFE_LOWEST_MAX_PAYLOAD - 6,
    firmware_transfer_get_theoretical_max_fragment_size(endpoint_id_node));
}

void test_zwave_command_class_firmware_theoretical_fragment_size_zwave_s0()
{
  attribute_store_node_t granted_keys_node
    = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);
  zwave_keyset_t keys = ZWAVE_CONTROLLER_S0_KEY;
  attribute_store_set_reported(granted_keys_node, &keys, sizeof(keys));

  attribute_store_node_t protocol_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                               node_id_node);
  zwave_protocol_t protocol = PROTOCOL_ZWAVE;
  attribute_store_set_reported(protocol_node, &protocol, sizeof(protocol));

  zwave_endpoint_id_t endpoint_id = 0;
  attribute_store_set_reported(endpoint_id_node,
                               &endpoint_id,
                               sizeof(endpoint_id));
  zwapi_get_rf_region_ExpectAndReturn(ZWAVE_RF_REGION_UNDEFINED);

  TEST_ASSERT_EQUAL(
    20,
    firmware_transfer_get_theoretical_max_fragment_size(endpoint_id_node));
}

void test_zwave_command_class_firmware_theoretical_fragment_size_zwave_jp_region()
{
  attribute_store_node_t granted_keys_node
    = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);
  zwave_keyset_t keys = ZWAVE_CONTROLLER_S2_UNAUTHENTICATED_KEY;
  attribute_store_set_reported(granted_keys_node, &keys, sizeof(keys));

  attribute_store_node_t protocol_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                               node_id_node);
  zwave_protocol_t protocol = PROTOCOL_ZWAVE;
  attribute_store_set_reported(protocol_node, &protocol, sizeof(protocol));

  zwave_endpoint_id_t endpoint_id = 0;
  attribute_store_set_reported(endpoint_id_node,
                               &endpoint_id,
                               sizeof(endpoint_id));
  zwapi_get_rf_region_ExpectAndReturn(ZWAVE_RF_REGION_JP);

  TEST_ASSERT_EQUAL(
    132,
    firmware_transfer_get_theoretical_max_fragment_size(endpoint_id_node));
}

void test_zwave_command_class_firmware_theoretical_fragment_size_zwave_kr_region()
{
  attribute_store_node_t granted_keys_node
    = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);
  zwave_keyset_t keys = ZWAVE_CONTROLLER_S2_ACCESS_KEY;
  attribute_store_set_reported(granted_keys_node, &keys, sizeof(keys));

  attribute_store_node_t protocol_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                               node_id_node);
  zwave_protocol_t protocol = PROTOCOL_ZWAVE;
  attribute_store_set_reported(protocol_node, &protocol, sizeof(protocol));

  zwave_endpoint_id_t endpoint_id = 0;
  attribute_store_set_reported(endpoint_id_node,
                               &endpoint_id,
                               sizeof(endpoint_id));
  zwapi_get_rf_region_ExpectAndReturn(ZWAVE_RF_REGION_KR);

  TEST_ASSERT_EQUAL(
    132,
    firmware_transfer_get_theoretical_max_fragment_size(endpoint_id_node));
}

void test_zwave_command_class_firmware_theoretical_fragment_size_zwave_non_secure()
{
  attribute_store_node_t granted_keys_node
    = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);
  zwave_keyset_t keys = 0;  // Non secure
  attribute_store_set_reported(granted_keys_node, &keys, sizeof(keys));

  attribute_store_node_t protocol_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                               node_id_node);
  zwave_protocol_t protocol = PROTOCOL_ZWAVE;
  attribute_store_set_reported(protocol_node, &protocol, sizeof(protocol));

  zwave_endpoint_id_t endpoint_id = 0;
  attribute_store_set_reported(endpoint_id_node,
                               &endpoint_id,
                               sizeof(endpoint_id));
  zwapi_get_rf_region_ExpectAndReturn(ZWAVE_RF_REGION_US_LR);

  TEST_ASSERT_EQUAL(
    36,
    firmware_transfer_get_theoretical_max_fragment_size(endpoint_id_node));
}
