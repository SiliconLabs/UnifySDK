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

// Mock includes
#include "zwave_controller_utils_mock.h"
#include "zwave_controller_keyset_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "attribute_store_defined_attribute_types.h"

// UIC includes
#include "attribute_store_helper_mock.h"
#include "attribute_store_mock.h"

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

static void get_node_granted_keys_verification(zwave_node_id_t node_id)
{
  attribute_store_node_t test_node_id_node = 234;
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    node_id,
    test_node_id_node);
  attribute_store_node_t test_granted_keys_node = 434;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_GRANTED_SECURITY_KEYS,
    0,
    test_granted_keys_node);

  attribute_store_get_reported_ExpectAndReturn(test_granted_keys_node,
                                               NULL,
                                               sizeof(zwave_keyset_t),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnMemThruPtr_value(&dummy_keyset,
                                                      sizeof(zwave_keyset_t));
}

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
  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    zpc_scheme,
    minimal_scheme,
    true);
  get_node_granted_keys_verification(connection.remote.node_id);
  /// TODO: UIC-1102 Wait for the 2021D spec release to be in force for cert to re-activate this.
  //zwave_controller_get_highest_encapsulation_ExpectAndReturn(
  //  dummy_keyset,
  //  connection.encapsulation);
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
  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    zpc_scheme,
    minimal_scheme,
    true);
  get_node_granted_keys_verification(connection.remote.node_id);
  /// TODO: UIC-1102 Wait for the 2021D spec release to be in force for cert to re-activate this.
  //zwave_controller_get_highest_encapsulation_ExpectAndReturn(
  //  dummy_keyset,
  //  ZWAVE_CONTROLLER_ENCAPSULATION_NONE);
  TEST_ASSERT_TRUE(
    zwave_security_validation_is_security_valid_for_support(minimal_scheme,
                                                            &connection));
}

void test_zwave_security_validation_is_security_valid_for_control()
{
  // NULL pointer
  TEST_ASSERT_FALSE(
    zwave_security_validation_is_security_valid_for_control(NULL));

  // No S2:
  attribute_store_node_t test_node_id_node = 234;
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    connection.remote.node_id,
    test_node_id_node);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_NODE_IS_S2_CAPABLE,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  TEST_ASSERT_TRUE(
    zwave_security_validation_is_security_valid_for_control(&connection));

  // S2, using its highest key:
  attribute_store_node_t test_s2_capable_node = 321;
  connection.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS;
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    connection.remote.node_id,
    test_node_id_node);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_NODE_IS_S2_CAPABLE,
    0,
    test_s2_capable_node);
  get_node_granted_keys_verification(connection.remote.node_id);
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    dummy_keyset,
    connection.encapsulation);
  TEST_ASSERT_TRUE(
    zwave_security_validation_is_security_valid_for_control(&connection));

  // S2, not using its highest key:
  connection.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS;
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    connection.remote.node_id,
    test_node_id_node);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_NODE_IS_S2_CAPABLE,
    0,
    test_s2_capable_node);
  get_node_granted_keys_verification(connection.remote.node_id);
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    dummy_keyset,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED);
  TEST_ASSERT_FALSE(
    zwave_security_validation_is_security_valid_for_control(&connection));
}

void test_zwave_security_validation_set_node_as_s2_capable()
{
  // Test first where it exist and no creation is needed:
  zwave_node_id_t test_node_id             = 45;
  attribute_store_node_t test_node_id_node = 234;
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);

  attribute_store_node_t test_s2_capable_node = 321;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_NODE_IS_S2_CAPABLE,
    0,
    test_s2_capable_node);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_security_validation_set_node_as_s2_capable(test_node_id));

  // If it does not exist:
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_NODE_IS_S2_CAPABLE,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_NODE_IS_S2_CAPABLE,
                                           test_node_id_node,
                                           SL_STATUS_OWNERSHIP);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OWNERSHIP,
    zwave_security_validation_set_node_as_s2_capable(test_node_id));
}

void test_zwave_security_validation_is_node_s2_capable()
{
  // True test:
  zwave_node_id_t test_node_id             = 45;
  attribute_store_node_t test_node_id_node = 234;
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);
  attribute_store_node_t test_s2_capable_node = 321;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_NODE_IS_S2_CAPABLE,
    0,
    test_s2_capable_node);
  TEST_ASSERT_TRUE(zwave_security_validation_is_node_s2_capable(test_node_id));

  // False test:
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_NODE_IS_S2_CAPABLE,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  TEST_ASSERT_FALSE(zwave_security_validation_is_node_s2_capable(test_node_id));
}

void test_zwave_security_validation_is_s2_nif_downgrade_attack_detected()
{
  const uint8_t *nif = NULL;
  uint8_t nif_length = 34;

  zwave_node_id_t test_node_id                = 45;
  attribute_store_node_t test_node_id_node    = 234;
  attribute_store_node_t test_s2_capable_node = 321;

  // Case for true:
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_NODE_IS_S2_CAPABLE,
    0,
    test_s2_capable_node);
  is_command_class_in_supported_list_ExpectAndReturn(COMMAND_CLASS_SECURITY_2,
                                                     nif,
                                                     nif_length,
                                                     false);
  TEST_ASSERT_TRUE(
    zwave_security_validation_is_s2_nif_downgrade_attack_detected(test_node_id,
                                                                  nif,
                                                                  nif_length));

  // S2 is still in the NIF
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_NODE_IS_S2_CAPABLE,
    0,
    test_s2_capable_node);
  is_command_class_in_supported_list_ExpectAndReturn(COMMAND_CLASS_SECURITY_2,
                                                     nif,
                                                     nif_length,
                                                     true);
  TEST_ASSERT_FALSE(
    zwave_security_validation_is_s2_nif_downgrade_attack_detected(test_node_id,
                                                                  nif,
                                                                  nif_length));

  // Not S2 capable
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_NODE_IS_S2_CAPABLE,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  TEST_ASSERT_FALSE(
    zwave_security_validation_is_s2_nif_downgrade_attack_detected(test_node_id,
                                                                  nif,
                                                                  nif_length));
}