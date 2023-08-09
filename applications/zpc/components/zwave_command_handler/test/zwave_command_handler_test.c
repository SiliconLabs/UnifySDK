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
#include "zwave_command_handler.h"
#include "zwave_command_handler_test.h"

// Includes from other components
#include "zwave_controller_callbacks_mock.h"
#include "zwave_controller_keyset_mock.h"
#include "zwave_controller_mock.h"
#include "zwave_controller_storage_mock.h"
#include "zwave_controller_utils_mock.h"
#include "zwave_security_validation_mock.h"
#include "zwave_network_management_mock.h"
#include "sl_log.h"

#define LOG_TAG "zwave_command_handler_test"

// Test includes
#include "unity.h"

// Our callbacks
static const zwave_controller_callbacks_t *zwave_controller_callbacks;
uint8_t cc_zwave_plus_init_counter        = 0;
uint8_t cc_zwave_plus_handle_counter      = 0;
uint8_t cc_time_init_counter              = 0;
uint8_t cc_time_handle_counter            = 0;
uint8_t cc_time2_handle_counter           = 0;
uint8_t cc_version_init_counter           = 0;
uint8_t cc_version_control_handle_counter = 0;
uint8_t cc_version_support_handle_counter = 0;
static zwave_keyset_t our_granted_keys    = 0x87;

static sl_status_t
  zwave_controller_callback_save(const zwave_controller_callbacks_t *cb, int n)
{
  zwave_controller_callbacks = cb;
  return SL_STATUS_OK;
}

static void
  zwave_controller_set_application_nif_stub(const uint8_t *command_classes,
                                            uint8_t command_classes_length)
{
  TEST_ASSERT_EQUAL(3, command_classes_length);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_ZWAVEPLUS_INFO, command_classes[0]);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY, command_classes[2]);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_TIME_V2, command_classes[1]);
}

static void zwave_controller_set_secure_application_nif_stub(
  const uint8_t *command_classes, uint8_t command_classes_length)
{
  TEST_ASSERT_EQUAL(1, command_classes_length);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_VERSION_V2, command_classes[0]);
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  //sl_log_setup(__FILE__, ".log");
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void zwave_command_handler_init_test_helper()
{
  // Set our initialization function counters to 0.
  cc_zwave_plus_init_counter        = 0;
  cc_time_init_counter              = 0;
  cc_version_init_counter           = 0;
  cc_zwave_plus_handle_counter      = 0;
  cc_time_handle_counter            = 0;
  cc_version_support_handle_counter = 0;
  cc_version_control_handle_counter = 0;

  zwave_controller_deregister_callbacks_IgnoreAndReturn(SL_STATUS_OK);
  //Clear the old list
  zwave_command_handler_teardown();
  zwave_network_scheme_str_IgnoreAndReturn("Dummy Message");

  // Register our 3 Command Class handlers:
  cc_version_init();
  cc_time_init();
  cc_zwave_plus_init();

  zwave_node_id_t our_node_id = 0x30;
  zwave_network_management_get_node_id_ExpectAndReturn(our_node_id);
  zwave_controller_storage_get_node_granted_keys_ExpectAndReturn(our_node_id,
                                                                 NULL,
                                                                 SL_STATUS_OK);
  zwave_controller_storage_get_node_granted_keys_IgnoreArg_keys();
  zwave_controller_storage_get_node_granted_keys_ReturnThruPtr_keys(
    &our_granted_keys);

  // Initialize the Command Class handler component.
  zwave_controller_register_callbacks_AddCallback(
    zwave_controller_callback_save);
  zwave_controller_register_callbacks_ExpectAndReturn(NULL, SL_STATUS_OK);
  zwave_controller_register_callbacks_IgnoreArg_callbacks();

  zwave_network_management_get_home_id_ExpectAndReturn(0x20);
  zwave_network_management_get_node_id_ExpectAndReturn(our_node_id);

  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    our_granted_keys,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS);
  zwave_controller_get_highest_encapsulation_IgnoreArg_keyset();

  // We have 3 Command Classes registered, each of them will be checked
  zwave_controller_encapsulation_scheme_greater_equal_IgnoreAndReturn(true);

  zwave_controller_set_application_nif_Stub(
    (CMOCK_zwave_controller_set_application_nif_CALLBACK)
      zwave_controller_set_application_nif_stub);

  zwave_controller_set_secure_application_nif_Stub(
    (CMOCK_zwave_controller_set_secure_application_nif_CALLBACK)
      zwave_controller_set_secure_application_nif_stub);

  // Call the init function.
  zwave_command_handler_init();

  // Verify that every Command Class handler has been initialized once
  TEST_ASSERT_EQUAL_UINT8(1, cc_zwave_plus_init_counter);
  TEST_ASSERT_EQUAL_UINT8(1, cc_time_init_counter);
  TEST_ASSERT_EQUAL_UINT8(1, cc_version_init_counter);
}

void test_on_frame_received_supported_cc()
{
  zwave_command_handler_init_test_helper();
  zwave_security_validation_is_security_valid_for_support_ExpectAndReturn(
    ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
    &connection_info_frame_non_secure,
    true);
  // Receive a frame
  zwave_controller_callbacks->on_application_frame_received(
    &connection_info_frame_non_secure,
    NULL,
    frame_data_1,
    sizeof(frame_data_1));

  TEST_ASSERT_EQUAL_UINT8(1, cc_zwave_plus_handle_counter);
}

void test_security_levels()
{
  zwave_command_handler_init_test_helper();

  zwave_security_validation_is_security_valid_for_support_ExpectAndReturn(
    ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
    &connection_info_frame_security_2_access,
    true);
  zwave_controller_callbacks->on_application_frame_received(
    &connection_info_frame_security_2_access,
    NULL,
    frame_data_1,
    sizeof(frame_data_1));

  // Last frame should have gone through.
  TEST_ASSERT_EQUAL_UINT8(1, cc_zwave_plus_handle_counter);

  // Z-Wave Plus info should be rejected if security validation does not like it
  zwave_security_validation_is_security_valid_for_support_ExpectAndReturn(
    ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
    &connection_info_frame_security_2_access,
    false);
  zwave_controller_callbacks->on_application_frame_received(
    &connection_info_frame_security_2_authenticated,
    NULL,
    frame_data_1,
    sizeof(frame_data_1));

  TEST_ASSERT_EQUAL_UINT8(1, cc_zwave_plus_handle_counter);
}

void test_command_class_not_supported()
{
  zwave_command_handler_init_test_helper();

  zwave_controller_callbacks->on_application_frame_received(
    &connection_info_frame_security_2_authenticated,
    NULL,
    frame_data_3,
    sizeof(frame_data_3));

  // Nobody was invoked after this:
  TEST_ASSERT_EQUAL_UINT8(0, cc_zwave_plus_handle_counter);
  TEST_ASSERT_EQUAL_UINT8(0, cc_time_handle_counter);
  TEST_ASSERT_EQUAL_UINT8(0, cc_time2_handle_counter);
  TEST_ASSERT_EQUAL_UINT8(0, cc_version_support_handle_counter);
  TEST_ASSERT_EQUAL_UINT8(0, cc_version_control_handle_counter);
}

void test_entering_a_new_network()
{
  zwave_command_handler_init_test_helper();

  zwave_home_id_t home_id             = 0x12345678;
  zwave_node_id_t node_id             = 0x12;
  zwave_kex_fail_type_t kex_fail_type = 0x00;

  // When entering a new network, we expect the NIF to get set.
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    our_granted_keys,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS);
  zwave_controller_get_highest_encapsulation_IgnoreArg_keyset();

  // We have 3 Command Classes registered, each of them will be checked
  zwave_controller_encapsulation_scheme_greater_equal_IgnoreAndReturn(true);

  zwave_controller_set_application_nif_Stub(
    (CMOCK_zwave_controller_set_application_nif_CALLBACK)
      zwave_controller_set_application_nif_stub);

  zwave_controller_set_secure_application_nif_Stub(
    (CMOCK_zwave_controller_set_secure_application_nif_CALLBACK)
      zwave_controller_set_secure_application_nif_stub);

  zwave_controller_callbacks->on_new_network_entered(home_id,
                                                     node_id,
                                                     our_granted_keys,
                                                     kex_fail_type);

  // No frame should have been received here.
  TEST_ASSERT_EQUAL_UINT8(0, cc_zwave_plus_handle_counter);
  TEST_ASSERT_EQUAL_UINT8(0, cc_time_handle_counter);
  TEST_ASSERT_EQUAL_UINT8(0, cc_version_support_handle_counter);
  TEST_ASSERT_EQUAL_UINT8(0, cc_version_control_handle_counter);
}

void test_disabling_controls_check()
{
  zwave_command_handler_init_test_helper();

  TEST_ASSERT_TRUE(zwave_command_handler_controls(COMMAND_CLASS_VERSION));
  TEST_ASSERT_FALSE(zwave_command_handler_controls(COMMAND_CLASS_TIME));
}
