/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "unity.h"
#include "sl_log.h"

// Generic includes
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>

// Includes from this component
#include "network_monitor.h"

// Interfaces
#include "ucl_definitions.h"

// Includes from other components
#include "contiki_test_helper.h"
#include "zwapi_protocol_mem_mock.h"
#include "zwave_controller_callbacks_mock.h"
#include "zwave_controller_utils_mock.h"
#include "zwave_controller_keyset_mock.h"
#include "zwave_utils_mock.h"
#include "zwave_network_management_mock.h"
#include "zwave_network_management_state.h"
#include "zwapi_init_mock.h"
#include "uic_mqtt_mock.h"
#include "zwave_tx_scheme_selector_mock.h"
#include "zwave_command_classes_fixt_mock.h"
#include "zpc_config_mock.h"

#include "attribute_store_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_resolver_mock.h"
#include "attribute_store_helper_mock.h"
#include "zwapi_protocol_basis_mock.h"

#define LOG_TAG "network_monitor_test"

const zwave_controller_callbacks_t *nm_callbacks;
char zpc_write_state_topic[MQTT_TOPIC_MAX_LENGTH];

static zwave_home_id_t home_id     = 0xdeadbeef;
static zwave_node_id_t node_id     = 0x1;
static zwave_node_id_t al_node_id  = 32;
static zwave_node_id_t fl_node_id  = 100;
static zwave_node_id_t nl_node_id  = 56;
static zwave_keyset_t granted_keys = 0x83;
static zwave_nodemask_t node_list  = {0x71};  // 0b01110001;

static attribute_store_node_update_callback_t on_nif_update_callback;

static attribute_store_node_t root_node_mock    = 0x39;
static attribute_store_node_t home_id_node_mock = 0x89bf7;
static attribute_store_node_t node_id_node_mock = 0x1111;

// Attribute Resolver Stubs variables
void (*attribute_resolver_set_resolution_listener_callback)(
  attribute_store_node_t);
attribute_store_node_t attribute_resolver_set_resolution_listener_node;

static attribute_store_node_update_callback_t
  my_attribute_store_wakeup_interval_update_cb;
static sl_status_t attribute_store_register_callback_by_type_and_state_save(
  attribute_store_node_update_callback_t cb,
  attribute_store_type_t test_type,
  attribute_store_node_value_state_t value_state,
  int n);

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  nm_callbacks                                 = NULL;
  on_nif_update_callback                       = NULL;
  my_attribute_store_wakeup_interval_update_cb = NULL;
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp()
{
  contiki_test_helper_init();
  zwave_network_management_get_state_IgnoreAndReturn(NM_IDLE);
}

static sl_status_t my_nm_callback_save(const zwave_controller_callbacks_t *cb,
                                       int n)
{
  nm_callbacks = cb;
  return SL_STATUS_OK;
}

static sl_status_t save_on_nif_update_callback(
  attribute_store_node_update_callback_t callback_function,
  attribute_store_type_t type,
  int num_calls)
{
  on_nif_update_callback = callback_function;
  TEST_ASSERT_EQUAL(ATTRIBUTE_ZWAVE_NIF, type);
  return SL_STATUS_OK;
}

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

static void attribute_resolver_set_resolution_listener_stub(
  attribute_store_node_t node, void (*callback)(attribute_store_node_t))
{
  attribute_resolver_set_resolution_listener_callback = callback;
  attribute_resolver_set_resolution_listener_node     = node;
}

static void network_monitor_pause_nl_nodes_resolution_verification(
  attribute_store_node_t current_network_node)
{
  static attribute_store_node_t node_id_node = 3;
  attribute_store_get_node_child_by_type_ExpectAndReturn(current_network_node,
                                                         ATTRIBUTE_NODE_ID,
                                                         0,
                                                         node_id_node);

  // Let's do 4 nodes, one for each operating mode.
  al_node_id = 32;
  attribute_store_read_value_ExpectAndReturn(node_id_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_node_id_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(&al_node_id,
                                                         sizeof(al_node_id));
  get_operating_mode_ExpectAndReturn(al_node_id, OPERATING_MODE_AL);

  // NL node
  node_id_node++;
  attribute_store_get_node_child_by_type_ExpectAndReturn(current_network_node,
                                                         ATTRIBUTE_NODE_ID,
                                                         1,
                                                         node_id_node);

  nl_node_id = 56;
  attribute_store_read_value_ExpectAndReturn(node_id_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_node_id_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(&nl_node_id,
                                                         sizeof(nl_node_id));
  get_operating_mode_ExpectAndReturn(nl_node_id, OPERATING_MODE_NL);
  attribute_resolver_pause_node_resolution_Expect(node_id_node);

  // FL node
  node_id_node++;
  attribute_store_get_node_child_by_type_ExpectAndReturn(current_network_node,
                                                         ATTRIBUTE_NODE_ID,
                                                         2,
                                                         node_id_node);

  fl_node_id = 100;
  attribute_store_read_value_ExpectAndReturn(node_id_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_node_id_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(&fl_node_id,
                                                         sizeof(fl_node_id));
  get_operating_mode_ExpectAndReturn(fl_node_id, OPERATING_MODE_FL);

  // Unkown node
  node_id_node++;
  attribute_store_get_node_child_by_type_ExpectAndReturn(current_network_node,
                                                         ATTRIBUTE_NODE_ID,
                                                         3,
                                                         node_id_node);

  node_id = 1;
  attribute_store_read_value_ExpectAndReturn(node_id_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_node_id_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(&node_id,
                                                         sizeof(node_id));
  get_operating_mode_ExpectAndReturn(node_id, OPERATING_MODE_UNKNOWN);

  // No more nodes, exit the while loop
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    current_network_node,
    ATTRIBUTE_NODE_ID,
    4,
    ATTRIBUTE_STORE_INVALID_NODE);
}

static sl_status_t attribute_store_register_callback_by_type_and_state_save(
  attribute_store_node_update_callback_t cb,
  attribute_store_type_t test_type,
  attribute_store_node_value_state_t value_state,
  int n)
{
  my_attribute_store_wakeup_interval_update_cb = cb;
  return SL_STATUS_OK;
}

/* Test of the network_monitor startup and shutdown
 */
void test_network_monitor_init()
{
  attribute_store_register_callback_by_type_and_state_AddCallback(
    attribute_store_register_callback_by_type_and_state_save);
  attribute_store_register_callback_by_type_and_state_ExpectAndReturn(
    NULL,
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL,
    REPORTED_ATTRIBUTE,
    SL_STATUS_OK);
  attribute_store_register_callback_by_type_and_state_IgnoreArg_callback_function();

  zwave_controller_register_callbacks_AddCallback(my_nm_callback_save);
  zwave_controller_register_callbacks_ExpectAndReturn(NULL, SL_STATUS_OK);
  zwave_controller_register_callbacks_IgnoreArg_callbacks();

  attribute_store_register_callback_by_type_AddCallback(
    save_on_nif_update_callback);
  attribute_store_register_callback_by_type_ExpectAndReturn(NULL,
                                                            ATTRIBUTE_ZWAVE_NIF,
                                                            SL_STATUS_OK);
  attribute_store_register_callback_by_type_IgnoreArg_callback_function();

  // There will be many calls to this function, just ignore them
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(node_id);

  zwave_network_management_get_granted_keys_IgnoreAndReturn(granted_keys);

  // At startup we expect the network monitor to:
  // 1. populate the Attribute Store, asking about the node list
  zwapi_get_full_node_list_ExpectAndReturn(0, SL_STATUS_OK);
  zwapi_get_full_node_list_IgnoreArg_node_list();
  zwapi_get_full_node_list_ReturnArrayThruPtr_node_list(
    node_list,
    sizeof(zwave_nodemask_t));

  /// FIXME: Try to verify that the input parameter are used correctly
  zwave_tx_scheme_get_node_connection_info_Ignore();

  // The number of read/write to the attribute store depends on each node:
  // ZPC Node
  static const unid_t expected_unids[4] = {"zw-DEADBEEF-0001",
                                           "zw-DEADBEEF-0005",
                                           "zw-DEADBEEF-0006",
                                           "zw-DEADBEEF-0007"};
  //const unid_t expected_unid = "zw-DEADBEEF-0001";
  static const attribute_store_node_t attr_id_node                  = 42;
  static const attribute_store_node_t attr_id_endpoint              = 90;
  static const attribute_store_node_t attr_id_granted_security_keys = 43;
  static const attribute_store_node_t attr_id_network_status        = 44;
  static const attribute_store_node_t attr_id_nif                   = 91;
  static const node_state_topic_state_t status_node_state_topic_included
    = NODE_STATE_TOPIC_STATE_INCLUDED;
  static const node_state_topic_state_t status_node_state_node_id_assigned
    = NODE_STATE_TOPIC_STATE_NODEID_ASSIGNED;
  static const zwave_endpoint_id_t endpoint_id = 0;

  for (unsigned int i = 0; i < sizeof(expected_unids) / sizeof(*expected_unids);
       i++) {
    // Mock of calls from network_monitor_add_attribute_store_node
    const unid_t *expected_unid = &expected_unids[i];
    attribute_store_network_helper_create_node_id_node_ExpectAndReturn(
      *expected_unid,
      attr_id_node);

    // Endpoint 0 added
    attribute_store_get_node_child_by_type_ExpectAndReturn(
      attr_id_node,
      ATTRIBUTE_ENDPOINT_ID,
      0,
      ATTRIBUTE_STORE_INVALID_NODE);
    attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_ENDPOINT_ID,
                                             attr_id_node,
                                             attr_id_endpoint);
    attribute_store_set_node_attribute_value_ExpectAndReturn(
      attr_id_endpoint,
      REPORTED_ATTRIBUTE,
      &endpoint_id,
      sizeof(endpoint_id),
      SL_STATUS_OK);

    // Network Status
    attribute_store_get_node_child_by_type_ExpectAndReturn(
      attr_id_node,
      ATTRIBUTE_NETWORK_STATUS,
      0,
      ATTRIBUTE_STORE_INVALID_NODE);
    attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_NETWORK_STATUS,
                                             attr_id_node,
                                             attr_id_network_status);
    // for the ZPC node expect included, for the rest expect assigned
    if (i == 0) {
      attribute_store_set_node_attribute_value_ExpectAndReturn(
        attr_id_network_status,
        REPORTED_ATTRIBUTE,
        &status_node_state_topic_included,
        sizeof(node_state_topic_state_t),
        SL_STATUS_OK);

      // Granted keys (only for zpc node)
      attribute_store_get_node_child_by_type_ExpectAndReturn(
        attr_id_node,
        ATTRIBUTE_GRANTED_SECURITY_KEYS,
        0,
        ATTRIBUTE_STORE_INVALID_NODE);
      attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                               attr_id_node,
                                               attr_id_granted_security_keys);
      static const zwave_keyset_t keyset
        = ZWAVE_CONTROLLER_S0_KEY | ZWAVE_CONTROLLER_S2_UNAUTHENTICATED_KEY
          | ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY;
      attribute_store_set_node_attribute_value_ExpectAndReturn(
        attr_id_granted_security_keys,
        REPORTED_ATTRIBUTE,
        &keyset,
        sizeof(keyset),
        SL_STATUS_OK);
    } else {
      attribute_store_set_node_attribute_value_ExpectAndReturn(
        attr_id_network_status,
        REPORTED_ATTRIBUTE,
        &status_node_state_node_id_assigned,
        sizeof(node_state_topic_state_t),
        SL_STATUS_OK);

      // Non zpc nodes add NIF
      attribute_store_get_node_child_by_type_ExpectAndReturn(
        attr_id_node,
        ATTRIBUTE_ENDPOINT_ID,
        0,
        attr_id_endpoint);
      attribute_store_get_node_child_by_type_ExpectAndReturn(
        attr_id_endpoint,
        ATTRIBUTE_ZWAVE_NIF,
        0,
        ATTRIBUTE_STORE_INVALID_NODE);
      attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_ZWAVE_NIF,
                                               attr_id_endpoint,
                                               attr_id_nif);
    }
  }

  attribute_resolver_set_resolution_listener_Stub(
    (CMOCK_attribute_resolver_set_resolution_listener_CALLBACK)
      attribute_resolver_set_resolution_listener_stub);
  // During init, the network monitor will try to activate node resolution
  // only for our HomeID (start of network_monitor_activate_network_resolution())
  attribute_store_get_root_ExpectAndReturn(root_node_mock);
  attribute_store_get_node_child_by_value_ExpectAndReturn(root_node_mock,
                                                          ATTRIBUTE_HOME_ID,
                                                          REPORTED_ATTRIBUTE,
                                                          NULL,
                                                          0,
                                                          0,
                                                          home_id_node_mock);
  attribute_store_get_node_child_by_value_IgnoreArg_value();
  attribute_store_get_node_child_by_value_IgnoreArg_value_size();
  attribute_store_get_node_child_by_type_ExpectAndReturn(root_node_mock,
                                                         ATTRIBUTE_HOME_ID,
                                                         0,
                                                         home_id_node_mock);
  attribute_store_get_node_child_by_type_ExpectAndReturn(root_node_mock,
                                                         ATTRIBUTE_HOME_ID,
                                                         1,
                                                         node_id_node_mock);
  attribute_resolver_pause_node_resolution_Expect(node_id_node_mock);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    root_node_mock,
    ATTRIBUTE_HOME_ID,
    2,
    ATTRIBUTE_STORE_INVALID_NODE);

  network_monitor_pause_nl_nodes_resolution_verification(home_id_node_mock);

  attribute_resolver_resume_node_resolution_Expect(home_id_node_mock);
  // end of network_monitor_activate_network_resolution()

  // Start the process and let the init call everything
  network_state_monitor_init();
  process_start(&network_monitor_process, 0);
  contiki_test_helper_run(0);
}

void test_network_monitor_exit()
{
  test_network_monitor_init();

  process_exit(&network_monitor_process);
  contiki_test_helper_run(0);
}

void test_network_monitor_test_node_id_assigned()
{
  test_network_monitor_init();
  zwave_node_id_t zwave_node_id = 0x24;
  nm_callbacks->on_node_id_assigned(zwave_node_id, true, PROTOCOL_ZWAVE);

  // On node added triggers all kind of calls to attribute_store_get_node_child_by_type()
  //attribute_store_get_node_child_by_type_IgnoreAndReturn(new_node_mock);

  const unid_t expected_unid1 = "zw-DEADBEEF-0024";
  attribute_store_network_helper_create_node_id_node_ExpectAndReturn(
    expected_unid1,
    101);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    101,
    ATTRIBUTE_ENDPOINT_ID,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_ENDPOINT_ID, 101, 102);
  const zwave_endpoint_id_t ep0 = 0;
  attribute_store_set_node_attribute_value_ExpectAndReturn(102,
                                                           REPORTED_ATTRIBUTE,
                                                           &ep0,
                                                           sizeof(ep0),
                                                           SL_STATUS_OK);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    101,
    ATTRIBUTE_NETWORK_STATUS,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_NETWORK_STATUS, 101, 103);
  const node_state_topic_state_t network_status
    = NODE_STATE_TOPIC_STATE_NODEID_ASSIGNED;
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    103,
    REPORTED_ATTRIBUTE,
    &network_status,
    sizeof(network_status),
    SL_STATUS_OK);
  store_protocol_ExpectAndReturn(zwave_node_id, PROTOCOL_ZWAVE, SL_STATUS_OK);
  contiki_test_helper_run(0);
}

void test_network_monitor_on_node_deleted()
{
  zwave_node_id_t zwave_node_id = 0x24;
  // When the node gets removed, it will get removed from the Attribute Store too
  // Our stub always return node_id_node_mock when searching for a node_id node in the Attribute Store
  attribute_store_network_helper_get_node_id_node_ExpectAndReturn(
    NULL,
    node_id_node_mock);
  attribute_store_network_helper_get_node_id_node_IgnoreArg_node_unid();
  attribute_store_delete_node_ExpectAndReturn(node_id_node_mock, SL_STATUS_OK);
  attribute_resolver_clear_resolution_listener_Expect(node_id_node_mock, NULL);
  attribute_resolver_clear_resolution_listener_IgnoreArg_callback();
  nm_callbacks->on_node_deleted(zwave_node_id);
  contiki_test_helper_run(0);

  // Deleting NodeID 0 should not do anything
  nm_callbacks->on_node_deleted(0);
  contiki_test_helper_run(0);
}

void test_network_monitor_on_node_added()
{
  static zwave_dsk_t dsk                     = {0};
  static const zwave_keyset_t granted_keys   = 0x87;
  static const zwave_node_info_t nif         = {0};
  static const zwave_node_id_t zwave_node_id = 0x42;
  static const uint8_t kex_fail     = ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE;
  static const unid_t expected_unid = "zw-DEADBEEF-0042";

  attribute_store_network_helper_create_node_id_node_ExpectAndReturn(
    expected_unid,
    102);

  // ATTRIBUTE_GRANTED_SECURITY_KEYS
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    102,
    ATTRIBUTE_GRANTED_SECURITY_KEYS,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                           102,
                                           103);
  attribute_store_set_node_attribute_value_ExpectAndReturn(103,
                                                           REPORTED_ATTRIBUTE,
                                                           &granted_keys,
                                                           sizeof(granted_keys),
                                                           SL_STATUS_OK);

  // ATTRIBUTE_KEX_FAIL_TYPE
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    102,
    ATTRIBUTE_KEX_FAIL_TYPE,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_KEX_FAIL_TYPE, 102, 104);
  attribute_store_set_node_attribute_value_ExpectAndReturn(104,
                                                           REPORTED_ATTRIBUTE,
                                                           &kex_fail,
                                                           sizeof(kex_fail),
                                                           SL_STATUS_OK);

  // ATTRIBUTE_S2_DSK
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    102,
    ATTRIBUTE_S2_DSK,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_S2_DSK, 102, 105);
  attribute_store_set_node_attribute_value_ExpectAndReturn(105,
                                                           REPORTED_ATTRIBUTE,
                                                           dsk,
                                                           sizeof(dsk),
                                                           SL_STATUS_OK);

  // ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    102,
    ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                           102,
                                           106);
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    106,
    REPORTED_ATTRIBUTE,
    &nif.listening_protocol,
    sizeof(nif.listening_protocol),
    SL_STATUS_OK);

  // ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    102,
    ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
                                           102,
                                           107);
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    107,
    REPORTED_ATTRIBUTE,
    &nif.optional_protocol,
    sizeof(nif.optional_protocol),
    SL_STATUS_OK);

  // ATTRIBUTE_ZWAVE_NIF
  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    expected_unid,
    0,
    110);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    110,
    ATTRIBUTE_ZWAVE_NIF,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_ZWAVE_NIF, 110, 111);

  // Update zwapi node list cache
  zwapi_get_full_node_list_ExpectAndReturn(0, SL_STATUS_OK);
  zwapi_get_full_node_list_IgnoreArg_node_list();
  zwapi_get_full_node_list_ReturnArrayThruPtr_node_list(
    node_list,
    sizeof(zwave_nodemask_t));

  nm_callbacks->on_node_added(SL_STATUS_OK,
                              &nif,
                              zwave_node_id,
                              dsk,
                              granted_keys,
                              kex_fail,
                              PROTOCOL_ZWAVE);
  store_protocol_ExpectAndReturn(zwave_node_id, PROTOCOL_ZWAVE, SL_STATUS_OK);
  contiki_test_helper_run(0);
}

void test_network_monitor_on_nif_updated()
{
  TEST_ASSERT_NOT_NULL(on_nif_update_callback);

  attribute_store_node_t test_node = 3;

  attribute_store_is_value_defined_ExpectAndReturn(test_node,
                                                   REPORTED_ATTRIBUTE,
                                                   false);

  attribute_store_node_t test_node_id_node = 23;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(test_node,
                                                             ATTRIBUTE_NODE_ID,
                                                             test_node_id_node);

  // Now we have posted the Interview initiated event.
  attribute_store_node_t test_network_status_node = 345;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_NETWORK_STATUS,
    0,
    test_network_status_node);

  const node_state_topic_state_t network_status = NODE_STATE_TOPIC_INTERVIEWING;
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    test_network_status_node,
    REPORTED_ATTRIBUTE,
    &network_status,
    sizeof(node_state_topic_state_t),
    SL_STATUS_OK);

  on_nif_update_callback(test_node, ATTRIBUTE_CREATED);
  contiki_test_helper_run(0);

  // Now trigger the callback, for interview completed
  attribute_resolver_set_resolution_listener_callback(test_node_id_node);
  attribute_resolver_clear_resolution_listener_Expect(
    test_node_id_node,
    attribute_resolver_set_resolution_listener_callback);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_NETWORK_STATUS,
    0,
    test_network_status_node);

  const node_state_topic_state_t network_status_included
    = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    test_network_status_node,
    REPORTED_ATTRIBUTE,
    &network_status_included,
    sizeof(node_state_topic_state_t),
    SL_STATUS_OK);

  contiki_test_helper_run(0);
}

void test_non_sleeping_node_failing_status_monitoring_procedure()
{
  static zwave_node_id_t my_node_id                  = 0x55;
  static attribute_store_node_t my_test_node_node_id = 0x068;
  static attribute_store_node_t my_node_offline_node = 0x077;
  zpc_config_t my_config_mock                        = {0};
  my_config_mock.accepted_transmit_failure           = 2;
  // mock first failure
  nm_callbacks->on_frame_transmission_failed(my_node_id);
  contiki_test_helper_run(0);
  // mock for failing transmission
  zpc_get_config_IgnoreAndReturn(&my_config_mock);
  attribute_store_network_helper_get_node_id_node_ExpectAndReturn(
    NULL,
    my_test_node_node_id);
  attribute_store_network_helper_get_node_id_node_IgnoreArg_node_unid();
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    my_test_node_node_id,
    ATTRIBUTE_NETWORK_STATUS,
    0,
    my_node_offline_node);
  attribute_store_get_node_attribute_value_ExpectAndReturn(my_node_offline_node,
                                                           REPORTED_ATTRIBUTE,
                                                           NULL,
                                                           NULL,
                                                           SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  static node_state_topic_state_t my_test_previous_node_status
    = NODE_STATE_TOPIC_LAST;
  static uint8_t my_test_previous_node_status_size
    = sizeof(node_state_topic_state_t);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value(
    &my_test_previous_node_status);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &my_test_previous_node_status_size);

  const node_state_topic_state_t network_status
    = NODE_STATE_TOPIC_STATE_OFFLINE;
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    my_node_offline_node,
    REPORTED_ATTRIBUTE,
    &network_status,
    sizeof(network_status),
    SL_STATUS_OK);
  attribute_resolver_pause_node_resolution_Expect(my_test_node_node_id);
  // mock the second failure
  nm_callbacks->on_frame_transmission_failed(my_node_id);
  contiki_test_helper_run(0);
  // mock for successful transmission
  attribute_store_set_child_reported_ExpectAndReturn(
    my_test_node_node_id,
    ATTRIBUTE_LAST_RECEIVED_FRAME_TIMESTAMP,
    NULL,
    sizeof(unsigned long),
    SL_STATUS_OK);
  attribute_store_set_child_reported_IgnoreArg_value();
  attribute_store_network_helper_get_node_id_node_ExpectAndReturn(
    NULL,
    my_test_node_node_id);
  attribute_store_network_helper_get_node_id_node_IgnoreArg_node_unid();
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    my_test_node_node_id,
    ATTRIBUTE_NETWORK_STATUS,
    0,
    my_node_offline_node);

  attribute_store_get_node_attribute_value_ExpectAndReturn(my_node_offline_node,
                                                           REPORTED_ATTRIBUTE,
                                                           NULL,
                                                           NULL,
                                                           SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnThruPtr_value(
    &my_test_previous_node_status);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &my_test_previous_node_status_size);

  const node_state_topic_state_t network_status_2
    = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    my_node_offline_node,
    REPORTED_ATTRIBUTE,
    &network_status_2,
    sizeof(network_status_2),
    SL_STATUS_OK);
  attribute_resolver_resume_node_resolution_Expect(my_test_node_node_id);
  // mock the node which is now responding
  nm_callbacks->on_frame_transmission_success(my_node_id);
  contiki_test_helper_run(0);
}

void test_sleeping_node_failing_status_monitoring_procedure()
{
  static attribute_store_node_t
    my_test_wakeup_interval_in_attribute_store_update
    = 0x0065;
  static attribute_store_node_t my_test_node_id_node          = 0xAA;
  static attribute_store_node_t my_last_awaked_timestamp_node = 0x88;
  static attribute_store_node_t my_newtork_status_node        = 0x77;
  static zwave_node_id_t my_test_node_id                      = 0x01;
  static uint8_t my_test_node_id_size            = sizeof(zwave_node_id_t);
  zpc_config_t my_nm_missing_wakeup_notification = {0};
  my_nm_missing_wakeup_notification.missing_wake_up_notification = 2;
  static uint8_t my_test_wakeup_interval[]
    = {0x01, 0x00, 0x00, 0x00};  //1 seconds
  static uint8_t my_test_wakeup_interval_value_size
    = sizeof(my_test_wakeup_interval);

  // Mock wakeup interval attribute update
  attribute_store_get_node_attribute_value_ExpectAndReturn(
    my_test_wakeup_interval_in_attribute_store_update,
    REPORTED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnArrayThruPtr_value(
    my_test_wakeup_interval,
    my_test_wakeup_interval_value_size);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &my_test_wakeup_interval_value_size);
  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    my_test_wakeup_interval_in_attribute_store_update,
    ATTRIBUTE_NODE_ID,
    my_test_node_id_node);
  attribute_store_get_node_attribute_value_ExpectAndReturn(my_test_node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           NULL,
                                                           NULL,
                                                           SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnMemThruPtr_value(
    (uint8_t *)&my_test_node_id,
    (uint8_t)my_test_node_id_size);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    (uint8_t *)&my_test_node_id_size);
  zpc_get_config_IgnoreAndReturn(&my_nm_missing_wakeup_notification);

  contiki_test_helper_run(0);
  my_attribute_store_wakeup_interval_update_cb(
    my_test_wakeup_interval_in_attribute_store_update,
    ATTRIBUTE_UPDATED);

  // mocking for ctimer callbacks handler
  attribute_store_network_helper_get_node_id_node_ExpectAndReturn(
    NULL,
    my_test_node_id_node);
  attribute_store_network_helper_get_node_id_node_IgnoreArg_node_unid();
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    my_test_node_id_node,
    ATTRIBUTE_LAST_RECEIVED_FRAME_TIMESTAMP,
    0,
    my_last_awaked_timestamp_node);
  static unsigned long my_test_lastwake_timestamp = 0;
  attribute_store_read_value_ExpectAndReturn(my_last_awaked_timestamp_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(unsigned long),
                                             1);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &my_test_lastwake_timestamp,
    sizeof(my_test_lastwake_timestamp));

  // mocking the network update status failing
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    my_test_node_id_node,
    ATTRIBUTE_NETWORK_STATUS,
    0,
    my_newtork_status_node);
  attribute_store_get_node_attribute_value_ExpectAndReturn(
    my_newtork_status_node,
    REPORTED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  static node_state_topic_state_t test_previous_node_status
    = NODE_STATE_TOPIC_LAST;
  static uint8_t test_previous_node_status_size
    = sizeof(node_state_topic_state_t);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value(
    &test_previous_node_status);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_previous_node_status_size);

  const node_state_topic_state_t network_status
    = NODE_STATE_TOPIC_STATE_OFFLINE;
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    my_newtork_status_node,
    REPORTED_ATTRIBUTE,
    &network_status,
    sizeof(network_status),
    SL_STATUS_OK);

  contiki_test_helper_run(2000);

  // mock to hear successfull tranmission from the failing node
  attribute_store_set_child_reported_ExpectAndReturn(
    my_test_node_id_node,
    ATTRIBUTE_LAST_RECEIVED_FRAME_TIMESTAMP,
    NULL,
    sizeof(unsigned long),
    SL_STATUS_OK);
  attribute_store_set_child_reported_IgnoreArg_value();
  attribute_store_network_helper_get_node_id_node_ExpectAndReturn(
    NULL,
    my_test_node_id_node);
  attribute_store_network_helper_get_node_id_node_IgnoreArg_node_unid();
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    my_test_node_id_node,
    ATTRIBUTE_NETWORK_STATUS,
    0,
    my_newtork_status_node);
  attribute_store_get_node_attribute_value_ExpectAndReturn(
    my_newtork_status_node,
    REPORTED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnThruPtr_value(
    &test_previous_node_status);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_previous_node_status_size);

  const node_state_topic_state_t network_status_2
    = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    my_newtork_status_node,
    REPORTED_ATTRIBUTE,
    &network_status_2,
    sizeof(network_status_2),
    SL_STATUS_OK);
  // mock the node which is now responding
  nm_callbacks->on_frame_transmission_success(my_test_node_id);
  contiki_test_helper_run(0);
}

void test_fail_interview_procedure()
{
  attribute_store_node_t test_nif_node     = 1;
  attribute_store_node_t test_node_id_node = 11;
  attribute_store_is_value_defined_ExpectAndReturn(test_nif_node,
                                                   REPORTED_ATTRIBUTE,
                                                   false);
  attribute_store_get_first_parent_with_type_ExpectAndReturn(test_nif_node,
                                                             ATTRIBUTE_NODE_ID,
                                                             test_node_id_node);
  // mocking network_monitor_handle_event_node_interview_initiated
  attribute_store_node_t test_network_status_node = 2;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_NETWORK_STATUS,
    0,
    test_network_status_node);
  static node_state_topic_state_t network_status
    = NODE_STATE_TOPIC_INTERVIEWING;
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    test_network_status_node,
    REPORTED_ATTRIBUTE,
    &network_status,
    sizeof(node_state_topic_state_t),
    SL_STATUS_OK);

  on_nif_update_callback(test_nif_node, ATTRIBUTE_CREATED);
  contiki_test_helper_run(0);

  // Mocking failling frame tranmission
  static zwave_node_id_t test_node_id = 3;
  nm_callbacks->on_frame_transmission_failed(test_node_id);
  contiki_test_helper_run(0);

  zpc_config_t my_config_mock              = {0};
  my_config_mock.accepted_transmit_failure = 2;
  zpc_get_config_IgnoreAndReturn(&my_config_mock);
  attribute_store_network_helper_get_node_id_node_ExpectAndReturn(
    NULL,
    test_node_id_node);
  attribute_store_network_helper_get_node_id_node_IgnoreArg_node_unid();
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_NETWORK_STATUS,
    0,
    test_network_status_node);
  attribute_store_get_node_attribute_value_ExpectAndReturn(
    test_network_status_node,
    REPORTED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  static uint8_t my_test_previous_node_status_size
    = sizeof(node_state_topic_state_t);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value(&network_status);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &my_test_previous_node_status_size);

  const node_state_topic_state_t network_status_failed_interview
    = NODE_STATE_TOPIC_STATE_INTERVIEW_FAIL;
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    test_network_status_node,
    REPORTED_ATTRIBUTE,
    &network_status_failed_interview,
    sizeof(network_status_failed_interview),
    SL_STATUS_OK);
  attribute_resolver_pause_node_resolution_Expect(test_node_id_node);
  // mock the second failure
  nm_callbacks->on_frame_transmission_failed(test_node_id);
  contiki_test_helper_run(0);
}
