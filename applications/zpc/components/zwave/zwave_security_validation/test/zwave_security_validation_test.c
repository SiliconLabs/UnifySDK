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
#include "zwave_security_validation.h"
#include "unity.h"
#include "ZW_classcmd.h"

// Mock includes
#include "zwave_controller_utils_mock.h"
#include "zwave_controller_keyset_mock.h"
#include "zwave_controller_storage_mock.h"
#include "zwave_tx_scheme_selector_mock.h"

// Static variables
static zwave_controller_connection_info_t connection = {};
static zwave_keyset_t dummy_keyset                   = 0xbc;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}
/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_zwave_security_validation_is_security_valid_for_support()
{
  // NULL pointer
  TEST_ASSERT_FALSE(zwave_security_validation_is_security_valid_for_support(
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS,
    NULL));

  // Security Class not good enough
  connection.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
  zwave_controller_encapsulation_scheme_t minimal_scheme
    = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED;
  zwave_controller_encapsulation_scheme_t zpc_scheme
    = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED;

  zpc_highest_security_class_ExpectAndReturn(zpc_scheme);
  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    zpc_scheme,
    minimal_scheme,
    false);
  TEST_ASSERT_FALSE(
    zwave_security_validation_is_security_valid_for_support(minimal_scheme,
                                                            &connection));

  // Not highest key: (zpc_scheme != connection.encapsulation) and minimal scheme != NONE
  zpc_highest_security_class_ExpectAndReturn(zpc_scheme);
  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    zpc_scheme,
    minimal_scheme,
    true);
  TEST_ASSERT_FALSE(
    zwave_security_validation_is_security_valid_for_support(minimal_scheme,
                                                            &connection));

  // Highest key: (zpc_scheme == connection.encapsulation)
  minimal_scheme           = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS;
  zpc_scheme               = minimal_scheme;
  connection.encapsulation = minimal_scheme;
  zpc_highest_security_class_ExpectAndReturn(zpc_scheme);
  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    zpc_scheme,
    minimal_scheme,
    true);
  TEST_ASSERT_TRUE(
    zwave_security_validation_is_security_valid_for_support(minimal_scheme,
                                                            &connection));

  // Not highest key: (zpc_scheme != connection.encapsulation)
  // and minimal scheme == NONE, node request on its own highest key
  minimal_scheme = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  zpc_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS;
  connection.encapsulation
    = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED;
  connection.remote.node_id = 25;
  zpc_highest_security_class_ExpectAndReturn(zpc_scheme);
  zwave_controller_storage_get_node_granted_keys_ExpectAndReturn(
    connection.remote.node_id,
    NULL,
    SL_STATUS_OK);
  zwave_controller_storage_get_node_granted_keys_IgnoreArg_keys();
  zwave_controller_storage_get_node_granted_keys_ReturnThruPtr_keys(
    &dummy_keyset);
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    dummy_keyset,
    connection.encapsulation);
  zwave_controller_storage_get_node_granted_keys_IgnoreArg_keys();
  TEST_ASSERT_TRUE(
    zwave_security_validation_is_security_valid_for_support(minimal_scheme,
                                                            &connection));

  // Not highest key: (zpc_scheme != connection.encapsulation)
  // and minimal scheme == NONE, node request not on its own highest key
  minimal_scheme = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  zpc_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS;
  connection.encapsulation
    = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED;
  connection.remote.node_id = 25;
  zpc_highest_security_class_ExpectAndReturn(zpc_scheme);
  zwave_controller_storage_get_node_granted_keys_ExpectAndReturn(
    connection.remote.node_id,
    NULL,
    SL_STATUS_OK);
  zwave_controller_storage_get_node_granted_keys_IgnoreArg_keys();
  zwave_controller_storage_get_node_granted_keys_ReturnThruPtr_keys(
    &dummy_keyset);
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    dummy_keyset,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS);  // Here we pretend the node suports ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS
  TEST_ASSERT_FALSE(
    zwave_security_validation_is_security_valid_for_support(minimal_scheme,
                                                            &connection));
  // Not highest key: (zpc_scheme != connection.encapsulation)
  // and minimal scheme == NONE, but cannot read remote node granted keys
  minimal_scheme = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  zpc_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS;
  connection.encapsulation
    = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED;
  connection.remote.node_id = 25;
  zpc_highest_security_class_ExpectAndReturn(zpc_scheme);
  zwave_controller_storage_get_node_granted_keys_ExpectAndReturn(
    connection.remote.node_id,
    NULL,
    SL_STATUS_FAIL);
  zwave_controller_storage_get_node_granted_keys_IgnoreArg_keys();
  TEST_ASSERT_FALSE(
    zwave_security_validation_is_security_valid_for_support(minimal_scheme,
                                                            &connection));

}

void test_zwave_security_validation_is_security_valid_for_control()
{
  // NULL pointer
  TEST_ASSERT_FALSE(
    zwave_security_validation_is_security_valid_for_control(NULL));

  // No S2:
  zwave_controller_storage_is_node_s2_capable_ExpectAndReturn(
    connection.remote.node_id,
    false);
  TEST_ASSERT_TRUE(
    zwave_security_validation_is_security_valid_for_control(&connection));

  // S2, using its highest key:
  connection.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS;
  zwave_controller_storage_is_node_s2_capable_ExpectAndReturn(
    connection.remote.node_id,
    true);
  zwave_controller_storage_get_node_granted_keys_ExpectAndReturn(
    connection.remote.node_id,
    NULL,
    SL_STATUS_OK);
  zwave_controller_storage_get_node_granted_keys_IgnoreArg_keys();
  zwave_controller_storage_get_node_granted_keys_ReturnThruPtr_keys(
    &dummy_keyset);
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    dummy_keyset,
    connection.encapsulation);

  TEST_ASSERT_TRUE(
    zwave_security_validation_is_security_valid_for_control(&connection));

  // S2, not using its highest key:
  connection.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS;
  zwave_controller_storage_is_node_s2_capable_ExpectAndReturn(
    connection.remote.node_id,
    true);
  zwave_controller_storage_get_node_granted_keys_ExpectAndReturn(
    connection.remote.node_id,
    NULL,
    SL_STATUS_OK);
  zwave_controller_storage_get_node_granted_keys_IgnoreArg_keys();
  zwave_controller_storage_get_node_granted_keys_ReturnThruPtr_keys(
    &dummy_keyset);
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    dummy_keyset,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED);
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    dummy_keyset,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED);

  TEST_ASSERT_FALSE(
    zwave_security_validation_is_security_valid_for_control(&connection));

  // S2, not using its highest key:
  connection.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS;
  zwave_controller_storage_is_node_s2_capable_ExpectAndReturn(
    connection.remote.node_id,
    true);
  zwave_controller_storage_get_node_granted_keys_ExpectAndReturn(
    connection.remote.node_id,
    NULL,
    SL_STATUS_FAIL);
  zwave_controller_storage_get_node_granted_keys_IgnoreArg_keys();
  zwave_controller_storage_get_node_granted_keys_ReturnThruPtr_keys(
    &dummy_keyset);
  TEST_ASSERT_TRUE(
    zwave_security_validation_is_security_valid_for_control(&connection));

}

void test_zwave_security_validation_is_s2_nif_downgrade_attack_detected()
{
  const uint8_t *nif = NULL;
  uint8_t nif_length = 34;

  zwave_node_id_t test_node_id = 45;

  // Case for true:
  zwave_controller_storage_is_node_s2_capable_ExpectAndReturn(test_node_id,
                                                              true);
  is_command_class_in_supported_list_ExpectAndReturn(COMMAND_CLASS_SECURITY_2,
                                                     nif,
                                                     nif_length,
                                                     false);
  TEST_ASSERT_TRUE(
    zwave_security_validation_is_s2_nif_downgrade_attack_detected(test_node_id,
                                                                  nif,
                                                                  nif_length));

  // S2 is still in the NIF
  zwave_controller_storage_is_node_s2_capable_ExpectAndReturn(test_node_id,
                                                              true);
  is_command_class_in_supported_list_ExpectAndReturn(COMMAND_CLASS_SECURITY_2,
                                                     nif,
                                                     nif_length,
                                                     true);
  TEST_ASSERT_FALSE(
    zwave_security_validation_is_s2_nif_downgrade_attack_detected(test_node_id,
                                                                  nif,
                                                                  nif_length));

  // Not S2 capable
  zwave_controller_storage_is_node_s2_capable_ExpectAndReturn(test_node_id,
                                                              false);
  TEST_ASSERT_FALSE(
    zwave_security_validation_is_s2_nif_downgrade_attack_detected(test_node_id,
                                                                  nif,
                                                                  nif_length));
}
