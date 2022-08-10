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
// Includes from this component
#include "zwave_command_class_association_send.h"
#include "zwave_command_class_association_helper.h"
#include "zwave_command_class_agi.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "unity.h"
#include "sl_log.h"
#include "ZW_classcmd.h"

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "zwave_unid.h"
#include "zwave_controller_utils.h"
#include "attribute_resolver.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "zwave_tx_scheme_selector_mock.h"
//#include "zwave_command_handler_mock.h"
#include "zwave_network_management_mock.h"
#include "zwave_tx_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_##type

// Test frame to send
static const uint8_t test_frame[]       = {0x02, 0x04, 0x00, 0xFF, 0x02, 0x05};
static const intptr_t test_user_pointer = 3943852;

static zwave_controller_connection_info_t connection = {};
static void *received_user_pointers[10]              = {};

static on_zwave_tx_send_data_complete_t on_association_tx_complete = NULL;

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

static uint8_t tx_call_count            = 0;
static uint8_t send_complete_call_count = 0;
static zwave_tx_options_t tx_options    = {};

// Stub functions
static sl_status_t zwave_tx_send_data_stub(
  const zwave_controller_connection_info_t *connection,
  uint16_t frame_length,
  const uint8_t *frame_data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user,
  zwave_tx_session_id_t *session,
  int cmock_num_calls)
{
  on_association_tx_complete = on_send_complete;
  memcpy(received_frame, frame_data, frame_length);
  received_frame_size                   = frame_length;
  received_user_pointers[tx_call_count] = user;
  tx_call_count += 1;
  return SL_STATUS_OK;
}

static void test_send_complete_callback(sl_status_t status, void *user)
{
  send_complete_call_count += 1;
  TEST_ASSERT_EQUAL_PTR(test_user_pointer, user);
}

void stub_zwave_tx_scheme_get_zpc_granted_keys(zwave_keyset_t *keys,
                                               int cmock_num_calls)
{
  *keys = 0x87;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zwave_unid_set_home_id(home_id);
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);
  zwave_tx_scheme_get_zpc_granted_keys_Stub(
    &stub_zwave_tx_scheme_get_zpc_granted_keys);
  zpc_attribute_store_test_helper_create_network();

  // Init our association groups
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_agi_init());

  // TX interceptions
  zwave_tx_send_data_AddCallback(&zwave_tx_send_data_stub);

  memset(&connection, 0, sizeof(connection));
  memset(received_frame, 0, sizeof(received_frame));
  memset(&tx_options, 0, sizeof(tx_options));
  tx_call_count              = 0;
  send_complete_call_count   = 0;
  on_association_tx_complete = NULL;

  // Add some associations in our lifeline:
  association_t association = {};
  association.node_id       = zpc_node_id;
  association.endpoint_id   = 3;
  association.type          = ENDPOINT;
  add_reported_association(zpc_node_id, 0, LIFELINE_GROUP_ID, association);

  association.node_id     = zpc_node_id + 1;
  association.endpoint_id = 3;
  association.type        = NODE_ID;
  add_reported_association(zpc_node_id, 0, LIFELINE_GROUP_ID, association);

  association.node_id     = zpc_node_id + 2;
  association.endpoint_id = 9;
  association.type        = ENDPOINT;
  add_reported_association(zpc_node_id, 0, LIFELINE_GROUP_ID, association);

  association.node_id     = 34;
  association.endpoint_id = 0;
  association.type        = ENDPOINT;
  add_reported_association(zpc_node_id, 0, LIFELINE_GROUP_ID, association);
}

void test_zwave_command_class_association_send()
{
  // Try to trigger a transmission. It should not be allowed until the
  // commands are registered.

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_association_send_to_group(LIFELINE_GROUP_ID,
                                                    sizeof(test_frame),
                                                    test_frame,
                                                    true,
                                                    NULL,
                                                    34,
                                                    NULL,
                                                    NULL));

  // Register the commands, then it will be accepted.
  zwave_command_class_agi_add_group_commands(LIFELINE_GROUP_ID, 0x02, 0x04);

  //zwave_tx_scheme_get_node_connection_info_Expect(2, 0, NULL);
  //zwave_tx_scheme_get_node_connection_info_IgnoreArg_connection_info();
  zwave_tx_scheme_get_node_connection_info_Ignore();

  zwave_tx_send_data_ExpectAndReturn(NULL,
                                     sizeof(test_frame),
                                     test_frame,
                                     &tx_options,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_ExpectAndReturn(NULL,
                                     sizeof(test_frame),
                                     test_frame,
                                     &tx_options,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_association_send_to_group(LIFELINE_GROUP_ID,
                                    sizeof(test_frame),
                                    test_frame,
                                    true,
                                    &tx_options,
                                    34,
                                    &test_send_complete_callback,
                                    (void *)test_user_pointer));
  TEST_ASSERT_EQUAL(2, tx_call_count);

  // Pretend that Tx sends a callback:
  TEST_ASSERT_NOT_NULL(on_association_tx_complete);
  for (uint8_t i = 0; i < tx_call_count; i++) {
    TEST_ASSERT_EQUAL(0, send_complete_call_count);
    on_association_tx_complete(TRANSMIT_COMPLETE_OK,
                               NULL,
                               received_user_pointers[i]);
  }
  TEST_ASSERT_EQUAL(1, send_complete_call_count);
}
