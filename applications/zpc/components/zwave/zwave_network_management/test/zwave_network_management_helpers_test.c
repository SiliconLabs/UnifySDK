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
#include "zwave_network_management.h"
#include "zwave_network_management_callbacks.h"
#include "zwave_network_management_helpers.h"
#include "nm_state_machine.h"

// Test framework
#include "unity.h"

// Interfaces
#include "zwave_generic_types.h"

// Generic includes
#include <string.h>

// Mocks
#include "zwapi_protocol_basis_mock.h"
#include "zwapi_protocol_mem_mock.h"
#include "zwapi_protocol_controller_mock.h"
#include "zwave_controller_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_reset_nms_last_operation_data()
{
  // Do something with the NMS:
  nms.node_id_being_handled    = 12;
  nms.is_controller            = 23;
  nms.flags                    = 34;
  nms.inclusion_protocol       = PROTOCOL_ZWAVE_LONG_RANGE;
  nms.requested_csa            = 1;
  nms.requested_keys           = 2;
  nms.kex_fail_type            = 3;
  nms.learn_mode_intent        = ZWAVE_NETWORK_MANAGEMENT_LEARN_NWI;
  nms.s2_bootstrapping_started = true;
  memset(nms.reported_dsk, 1, sizeof(zwave_dsk_t));
  memset(nms.expected_dsk, 1, sizeof(zwave_dsk_t));

  // Ask for reset:
  reset_nms_last_operation_data();

  TEST_ASSERT_EQUAL(0, nms.node_id_being_handled);
  TEST_ASSERT_EQUAL(0, nms.is_controller);
  TEST_ASSERT_EQUAL(0, nms.flags);
  TEST_ASSERT_EQUAL(PROTOCOL_ZWAVE, nms.inclusion_protocol);
  TEST_ASSERT_EQUAL(0, nms.requested_csa);
  TEST_ASSERT_EQUAL(0, nms.requested_keys);
  TEST_ASSERT_EQUAL(0, nms.kex_fail_type);
  TEST_ASSERT_EQUAL(0, nms.learn_mode_intent);
  TEST_ASSERT_FALSE(nms.s2_bootstrapping_started);
  const zwave_dsk_t expected_dsk = {};
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected_dsk,
                               nms.reported_dsk,
                               sizeof(zwave_dsk_t));
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected_dsk,
                               nms.expected_dsk,
                               sizeof(zwave_dsk_t));
}

void test_we_are_alone_in_our_network()
{
  TEST_ASSERT_TRUE(we_are_alone_in_our_network());

  // Add some nodes (1 and 9)
  ZW_ADD_NODE_TO_MASK(1, nms.cached_node_list);
  ZW_ADD_NODE_TO_MASK(9, nms.cached_node_list);

  TEST_ASSERT_FALSE(we_are_alone_in_our_network());

  // Pretend we are NodeID 9
  nms.cached_local_node_id = 9;
  TEST_ASSERT_FALSE(we_are_alone_in_our_network());

  // Remove NodeID 1 from the mask
  ZW_REMOVE_NODE_FROM_MASK(1, nms.cached_node_list);
  TEST_ASSERT_TRUE(we_are_alone_in_our_network());

  // Add a node with high NodeID:
  ZW_ADD_NODE_TO_MASK(1304, nms.cached_node_list);
  TEST_ASSERT_FALSE(we_are_alone_in_our_network());

  // Remove our own NodeID from the node mask.
  ZW_REMOVE_NODE_FROM_MASK(nms.cached_local_node_id, nms.cached_node_list);
  TEST_ASSERT_FALSE(we_are_alone_in_our_network());

  // No more nodes in the nodemask:
  memset(nms.cached_node_list, 0, sizeof(zwave_nodemask_t));
  TEST_ASSERT_TRUE(we_are_alone_in_our_network());
};

void test_network_management_refresh_cached_ids()
{
  zwapi_memory_get_ids_ExpectAndReturn(&nms.cached_home_id,
                                       &nms.cached_local_node_id,
                                       SL_STATUS_OK);
  network_management_refresh_cached_ids();
}

void test_network_management_refresh_cached_node_list()
{
  zwapi_get_full_node_list_ExpectAndReturn(nms.cached_node_list, SL_STATUS_OK);
  network_management_refresh_cached_node_list();
}

void test_network_management_refresh_controller_capabilities_bitmask()
{
  uint8_t return_value = 12;
  zwapi_get_controller_capabilities_ExpectAndReturn(return_value);
  network_management_refresh_controller_capabilities_bitmask();

  TEST_ASSERT_EQUAL(return_value, nms.controller_capabilities_bitmask);
}

void test_network_management_refresh_network_information()
{
  zwapi_memory_get_ids_ExpectAndReturn(&nms.cached_home_id,
                                       &nms.cached_local_node_id,
                                       SL_STATUS_OK);
  zwapi_get_full_node_list_ExpectAndReturn(nms.cached_node_list, SL_STATUS_OK);
  zwapi_get_controller_capabilities_ExpectAndReturn(0);
  network_management_refresh_network_information();
}

void test_network_management_stop_ongoing_operations()
{
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);

  network_management_stop_ongoing_operations();
}

void test_is_command_class_in_nif()
{
  const zwave_command_class_t test_nif_1[] = {0xFFFF, 0x9F, 0x20, 0x25};

  TEST_ASSERT_TRUE(
    is_command_class_in_nif(test_nif_1,
                            sizeof(test_nif_1) / sizeof(zwave_command_class_t),
                            0xFFFF));

  TEST_ASSERT_TRUE(
    is_command_class_in_nif(test_nif_1,
                            sizeof(test_nif_1) / sizeof(zwave_command_class_t),
                            0x9F));

  TEST_ASSERT_TRUE(
    is_command_class_in_nif(test_nif_1,
                            sizeof(test_nif_1) / sizeof(zwave_command_class_t),
                            0x20));

  TEST_ASSERT_TRUE(
    is_command_class_in_nif(test_nif_1,
                            sizeof(test_nif_1) / sizeof(zwave_command_class_t),
                            0x25));

  TEST_ASSERT_FALSE(
    is_command_class_in_nif(test_nif_1,
                            sizeof(test_nif_1) / sizeof(zwave_command_class_t),
                            0x98));

  const zwave_command_class_t
    test_nif_2[ZWAVE_CONTROLLER_MAXIMUM_COMMAND_CLASS_LIST_LENGTH]
    = {0xFFFF, 0x9F, 0x20, 0x25};

  TEST_ASSERT_TRUE(is_command_class_in_nif(
    test_nif_2,
    ZWAVE_CONTROLLER_MAXIMUM_COMMAND_CLASS_LIST_LENGTH + 3,
    0x9F));

  TEST_ASSERT_FALSE(is_command_class_in_nif(
    test_nif_2,
    ZWAVE_CONTROLLER_MAXIMUM_COMMAND_CLASS_LIST_LENGTH + 3,
    0x98));
}

// Not in the helper functions, but it is tested here:
void test_zwave_network_management_is_zpc_sis()
{
  nms.controller_capabilities_bitmask = 0;
  TEST_ASSERT_FALSE(zwave_network_management_is_zpc_sis());

  nms.controller_capabilities_bitmask
    = CONTROLLER_NODEID_SERVER_PRESENT | CONTROLLER_IS_SUC;
  TEST_ASSERT_TRUE(zwave_network_management_is_zpc_sis());

  nms.controller_capabilities_bitmask = CONTROLLER_IS_SECONDARY
                                        | CONTROLLER_NODEID_SERVER_PRESENT
                                        | CONTROLLER_IS_SUC;
  TEST_ASSERT_FALSE(zwave_network_management_is_zpc_sis());

  nms.controller_capabilities_bitmask = CONTROLLER_IS_SUC;
  TEST_ASSERT_FALSE(zwave_network_management_is_zpc_sis());

  nms.controller_capabilities_bitmask = CONTROLLER_NODEID_SERVER_PRESENT;
  TEST_ASSERT_FALSE(zwave_network_management_is_zpc_sis());
}

void test_we_should_self_destruct()
{
  // Happy case Z-Wave no granted keys.
  nms.learn_mode_intent        = ZWAVE_NETWORK_MANAGEMENT_LEARN_SMART_START;
  nms.kex_fail_type            = 0;
  nms.s2_bootstrapping_started = true;
  nms.cached_local_node_id     = 2;
  nms.granted_keys             = 0;

  TEST_ASSERT_FALSE(we_should_self_destruct());

  // Unknown NodeIDs, we bail out (can't figure if we should aplly Z-Wave or Z-Wave LR rules)
  nms.cached_local_node_id = 0;
  TEST_ASSERT_TRUE(we_should_self_destruct());
  nms.cached_local_node_id = 238;
  TEST_ASSERT_TRUE(we_should_self_destruct());
  nms.cached_local_node_id = 5000;
  TEST_ASSERT_TRUE(we_should_self_destruct());

  // No bootstrapping, we get out of there
  nms.cached_local_node_id     = 256;
  nms.s2_bootstrapping_started = false;
  TEST_ASSERT_TRUE(we_should_self_destruct());

  // KEX fail not 0, we get out of there
  nms.cached_local_node_id     = 256;
  nms.s2_bootstrapping_started = true;
  nms.kex_fail_type            = 1;
  TEST_ASSERT_TRUE(we_should_self_destruct());

  // Bootstrapping went horribly wrong, but we did not smartstart, stay in the network:
  nms.learn_mode_intent        = ZWAVE_NETWORK_MANAGEMENT_LEARN_NWI;
  nms.cached_local_node_id     = 256;
  nms.s2_bootstrapping_started = true;
  nms.kex_fail_type            = 80;
  TEST_ASSERT_FALSE(we_should_self_destruct());

  // No authenticated keys granted with LR, (grant everything else than Auth / Access Control):
  nms.learn_mode_intent        = ZWAVE_NETWORK_MANAGEMENT_LEARN_SMART_START;
  nms.cached_local_node_id     = 256;
  nms.s2_bootstrapping_started = true;
  nms.kex_fail_type            = 0;
  nms.granted_keys             = ((~ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY)
                      & (~ZWAVE_CONTROLLER_S2_ACCESS_KEY));
  TEST_ASSERT_TRUE(we_should_self_destruct());

  // Happy case for LR:
  nms.learn_mode_intent        = ZWAVE_NETWORK_MANAGEMENT_LEARN_SMART_START;
  nms.cached_local_node_id     = 1000;
  nms.s2_bootstrapping_started = true;
  nms.kex_fail_type            = 0;
  nms.granted_keys             = ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY;
  TEST_ASSERT_FALSE(we_should_self_destruct());

  // Happy case 2 for LR:
  nms.learn_mode_intent        = ZWAVE_NETWORK_MANAGEMENT_LEARN_SMART_START;
  nms.cached_local_node_id     = 1000;
  nms.s2_bootstrapping_started = true;
  nms.kex_fail_type            = 0;
  nms.granted_keys             = ZWAVE_CONTROLLER_S2_ACCESS_KEY;
  TEST_ASSERT_FALSE(we_should_self_destruct());
}

void test_take_sis_role_if_no_suc_in_network()
{
  // Nothing should happen if SIS is present.
  nms.controller_capabilities_bitmask = CONTROLLER_NODEID_SERVER_PRESENT;
  network_management_take_sis_role_if_no_suc_in_network();

  // Nothing should happen if we are secondary:
  nms.controller_capabilities_bitmask = CONTROLLER_IS_SECONDARY;
  network_management_take_sis_role_if_no_suc_in_network();

  // In any other case, we try to take the SIS role;
  nms.controller_capabilities_bitmask
    = ~(CONTROLLER_IS_SECONDARY | CONTROLLER_NODEID_SERVER_PRESENT);
  zwapi_set_suc_node_id_ExpectAndReturn(nms.cached_local_node_id,
                                        true,
                                        false,
                                        ZW_SUC_FUNC_NODEID_SERVER,
                                        &on_set_suc_node_id_completed,
                                        SL_STATUS_OK);
  // network_management_refresh_controller_capabilities_bitmask()
  zwapi_get_controller_capabilities_ExpectAndReturn(23);

  network_management_take_sis_role_if_no_suc_in_network();
}

void test_network_management_is_ready_for_a_new_operation()
{
  zwave_controller_is_reset_ongoing_ExpectAndReturn(false);
  TEST_ASSERT_TRUE(network_management_is_ready_for_a_new_operation());

  zwave_controller_is_reset_ongoing_ExpectAndReturn(true);
  TEST_ASSERT_FALSE(network_management_is_ready_for_a_new_operation());

  nms.state = NM_IDLE + 1;
  TEST_ASSERT_FALSE(network_management_is_ready_for_a_new_operation());
}