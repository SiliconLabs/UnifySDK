/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "user_code_cluster_server.h"
#include "unity.h"

// Definitions
#include "dotdot_cluster_id_definitions.h"
#include "attribute_store_defined_attribute_types.h"
#include "zwave_command_class_association_types.h"
#include "zap-types.h"

// ZPC Components
#include "zpc_attribute_store_type_registration.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Unify components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "unify_dotdot_attribute_store_node_state.h"

// Mocks
#include "dotdot_mqtt_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_USER_CODE_##type

// Callback functions
// clang-format off
static uic_mqtt_dotdot_door_lock_setpin_code_callback_t         set_pin_code_command = NULL;
static uic_mqtt_dotdot_door_lock_getpin_code_callback_t         get_pin_code_command = NULL;
static uic_mqtt_dotdot_door_lock_clearpin_code_callback_t       clear_pin_code_command = NULL;
static uic_mqtt_dotdot_door_lock_clear_allpin_codes_callback_t  clear_all_pin_codes_command = NULL;
static uic_mqtt_dotdot_door_lock_set_user_status_callback_t     set_user_status_command = NULL;
static uic_mqtt_dotdot_door_lock_get_user_status_callback_t     get_user_status_command = NULL;
// clang-format on

// Stub functions for intercepting callback registration.
void uic_mqtt_dotdot_door_lock_setpin_code_callback_set_stub(
  const uic_mqtt_dotdot_door_lock_setpin_code_callback_t callback,
  int cmock_num_calls)
{
  set_pin_code_command = callback;
}

void uic_mqtt_dotdot_door_lock_getpin_code_callback_set_stub(
  const uic_mqtt_dotdot_door_lock_getpin_code_callback_t callback,
  int cmock_num_calls)
{
  get_pin_code_command = callback;
}

void uic_mqtt_dotdot_door_lock_clearpin_code_callback_set_stub(
  const uic_mqtt_dotdot_door_lock_clearpin_code_callback_t callback,
  int cmock_num_calls)
{
  clear_pin_code_command = callback;
}

void uic_mqtt_dotdot_door_lock_clear_allpin_codes_callback_set_stub(
  const uic_mqtt_dotdot_door_lock_clear_allpin_codes_callback_t callback,
  int cmock_num_calls)
{
  clear_all_pin_codes_command = callback;
}

void uic_mqtt_dotdot_door_lock_set_user_status_callback_set_stub(
  const uic_mqtt_dotdot_door_lock_set_user_status_callback_t callback,
  int cmock_num_calls)
{
  set_user_status_command = callback;
}

void uic_mqtt_dotdot_door_lock_get_user_status_callback_set_stub(
  const uic_mqtt_dotdot_door_lock_get_user_status_callback_t callback,
  int cmock_num_calls)
{
  get_user_status_command = callback;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_register_known_attribute_types();
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
  zpc_attribute_store_test_helper_create_network();

  // Intercept the dotdot MQTT callbacks
  set_pin_code_command        = NULL;
  get_pin_code_command        = NULL;
  clear_pin_code_command      = NULL;
  clear_all_pin_codes_command = NULL;
  set_user_status_command     = NULL;
  get_user_status_command     = NULL;

  // clang-format off
  uic_mqtt_dotdot_door_lock_setpin_code_callback_set_Stub(&uic_mqtt_dotdot_door_lock_setpin_code_callback_set_stub);
  uic_mqtt_dotdot_door_lock_getpin_code_callback_set_Stub(&uic_mqtt_dotdot_door_lock_getpin_code_callback_set_stub);
  uic_mqtt_dotdot_door_lock_clearpin_code_callback_set_Stub(&uic_mqtt_dotdot_door_lock_clearpin_code_callback_set_stub);
  uic_mqtt_dotdot_door_lock_clear_allpin_codes_callback_set_Stub(&uic_mqtt_dotdot_door_lock_clear_allpin_codes_callback_set_stub);
  uic_mqtt_dotdot_door_lock_set_user_status_callback_set_Stub(uic_mqtt_dotdot_door_lock_set_user_status_callback_set_stub);
  uic_mqtt_dotdot_door_lock_get_user_status_callback_set_Stub(uic_mqtt_dotdot_door_lock_get_user_status_callback_set_stub);
  // clang-format on

  // Run the component init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, user_code_cluster_server_init());
}

void test_user_code_cluster_server_set_pin_code()
{
  TEST_ASSERT_NOT_NULL(set_pin_code_command);
  const uint16_t user_id = 3;

  // Support check
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    set_pin_code_command(supporting_node_unid,
                         endpoint_id,
                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                         user_id,
                         ZCL_DRLK_USER_STATUS_OCCUPIED_ENABLED,
                         ZCL_DRLK_USER_TYPE_UNRESTRICTED_USER,
                         "1234"));

  attribute_store_node_t data_node
    = attribute_store_create_child_if_missing(endpoint_id_node,
                                              ATTRIBUTE(DATA));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    set_pin_code_command(supporting_node_unid,
                         endpoint_id,
                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                         user_id,
                         ZCL_DRLK_USER_STATUS_OCCUPIED_ENABLED,
                         ZCL_DRLK_USER_TYPE_UNRESTRICTED_USER,
                         "1234"));

  // Try to set a PIN code for non-existing User ID:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    set_pin_code_command(supporting_node_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         user_id,
                                         ZCL_DRLK_USER_STATUS_OCCUPIED_ENABLED,
                                         ZCL_DRLK_USER_TYPE_UNRESTRICTED_USER,
                                         "1234"));

  attribute_store_node_t user_id_node
    = attribute_store_emplace(data_node,
                              ATTRIBUTE(USER_ID),
                              &user_id,
                              sizeof(user_id));

  attribute_store_create_child_if_missing(user_id_node,
                                          ATTRIBUTE(USER_ID_STATUS));
  attribute_store_node_t code_node
    = attribute_store_create_child_if_missing(user_id_node, ATTRIBUTE(CODE));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    set_pin_code_command(supporting_node_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         user_id,
                                         ZCL_DRLK_USER_STATUS_OCCUPIED_ENABLED,
                                         ZCL_DRLK_USER_TYPE_UNRESTRICTED_USER,
                                         "1234"));

  char saved_code[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  attribute_store_get_desired_string(code_node,
                                     saved_code,
                                     ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("1234", saved_code);
}

void test_user_code_cluster_server_get_pin_code()
{
  TEST_ASSERT_NOT_NULL(get_pin_code_command);
  const uint16_t user_id = 3;

  // Support check
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    get_pin_code_command(supporting_node_unid,
                         endpoint_id,
                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                         user_id));

  attribute_store_node_t data_node
    = attribute_store_create_child_if_missing(endpoint_id_node,
                                              ATTRIBUTE(DATA));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    get_pin_code_command(supporting_node_unid,
                         endpoint_id,
                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                         user_id));

  // Try to get a PIN code for non-existing User ID:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    get_pin_code_command(supporting_node_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         user_id));

  attribute_store_node_t user_id_node
    = attribute_store_emplace(data_node,
                              ATTRIBUTE(USER_ID),
                              &user_id,
                              sizeof(user_id));

  attribute_store_create_child_if_missing(user_id_node,
                                          ATTRIBUTE(USER_ID_STATUS));
  attribute_store_node_t code_node
    = attribute_store_create_child_if_missing(user_id_node, ATTRIBUTE(CODE));
  uint8_t value = 0;
  attribute_store_set_desired(code_node, &value, sizeof(value));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    get_pin_code_command(supporting_node_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         user_id));

  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(code_node));
}

void test_user_code_cluster_server_set_user_status()
{
  TEST_ASSERT_NOT_NULL(set_user_status_command);
  const uint16_t user_id = 3;

  // Support check
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    set_user_status_command(supporting_node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                            user_id,
                            ZCL_DRLK_USER_STATUS_OCCUPIED_ENABLED));

  attribute_store_node_t data_node
    = attribute_store_create_child_if_missing(endpoint_id_node,
                                              ATTRIBUTE(DATA));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    set_user_status_command(supporting_node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                            user_id,
                            ZCL_DRLK_USER_STATUS_OCCUPIED_ENABLED));

  // Try to set a status for non-existing User ID:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    set_user_status_command(supporting_node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                            user_id,
                            ZCL_DRLK_USER_STATUS_OCCUPIED_ENABLED));

  attribute_store_node_t user_id_node
    = attribute_store_emplace(data_node,
                              ATTRIBUTE(USER_ID),
                              &user_id,
                              sizeof(user_id));

  attribute_store_node_t status_node
    = attribute_store_create_child_if_missing(user_id_node,
                                              ATTRIBUTE(USER_ID_STATUS));
  attribute_store_set_desired_number(status_node, 3);
  attribute_store_create_child_if_missing(user_id_node, ATTRIBUTE(CODE));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    set_user_status_command(supporting_node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                            user_id,
                            ZCL_DRLK_USER_STATUS_OCCUPIED_ENABLED));

  TEST_ASSERT_EQUAL(1, attribute_store_get_desired_number(status_node));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    set_user_status_command(supporting_node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                            user_id,
                            ZCL_DRLK_USER_STATUS_OCCUPIED_DISABLED));

  TEST_ASSERT_EQUAL(2, attribute_store_get_desired_number(status_node));
}

void test_user_code_cluster_server_get_user_status()
{
  TEST_ASSERT_NOT_NULL(get_user_status_command);
  const uint16_t user_id = 3;

  // Support check
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    get_user_status_command(supporting_node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                            user_id));

  attribute_store_node_t data_node
    = attribute_store_create_child_if_missing(endpoint_id_node,
                                              ATTRIBUTE(DATA));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    get_user_status_command(supporting_node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                            user_id));

  // Try to get a PIN code for non-existing User ID:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    get_user_status_command(supporting_node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                            user_id));

  attribute_store_node_t user_id_node
    = attribute_store_emplace(data_node,
                              ATTRIBUTE(USER_ID),
                              &user_id,
                              sizeof(user_id));

  attribute_store_node_t user_status_node
    = attribute_store_create_child_if_missing(user_id_node,
                                              ATTRIBUTE(USER_ID_STATUS));
  attribute_store_create_child_if_missing(user_id_node, ATTRIBUTE(CODE));
  attribute_store_set_reported_number(user_status_node, 3);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    get_user_status_command(supporting_node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                            user_id));

  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(user_status_node));
}

void test_user_code_cluster_server_clear_pin_code()
{
  TEST_ASSERT_NOT_NULL(clear_pin_code_command);
  const uint16_t user_id = 3;

  // Support check
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    clear_pin_code_command(supporting_node_unid,
                           endpoint_id,
                           UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                           user_id));

  attribute_store_node_t data_node
    = attribute_store_create_child_if_missing(endpoint_id_node,
                                              ATTRIBUTE(DATA));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    clear_pin_code_command(supporting_node_unid,
                           endpoint_id,
                           UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                           user_id));

  // Try to get a PIN code for non-existing User ID:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    clear_pin_code_command(supporting_node_unid,
                                           endpoint_id,
                                           UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                           user_id));

  attribute_store_node_t user_id_node
    = attribute_store_emplace(data_node,
                              ATTRIBUTE(USER_ID),
                              &user_id,
                              sizeof(user_id));

  attribute_store_node_t user_status_node
    = attribute_store_create_child_if_missing(user_id_node,
                                              ATTRIBUTE(USER_ID_STATUS));
  attribute_store_create_child_if_missing(user_id_node, ATTRIBUTE(CODE));
  attribute_store_set_reported_number(user_status_node, 3);
  attribute_store_set_desired_as_reported(user_status_node);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    clear_pin_code_command(supporting_node_unid,
                                           endpoint_id,
                                           UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                           user_id));

  TEST_ASSERT_EQUAL(0, attribute_store_get_desired_number(user_status_node));
}

void test_user_code_cluster_server_clear_all_pin_codes()
{
  TEST_ASSERT_NOT_NULL(clear_all_pin_codes_command);

  // Support check
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    clear_all_pin_codes_command(supporting_node_unid,
                                endpoint_id,
                                UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  attribute_store_node_t data_node
    = attribute_store_create_child_if_missing(endpoint_id_node,
                                              ATTRIBUTE(DATA));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    clear_all_pin_codes_command(supporting_node_unid,
                                endpoint_id,
                                UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Try to clear when it's not possible
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    clear_all_pin_codes_command(supporting_node_unid,
                                endpoint_id,
                                UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));

  attribute_store_node_t clear_all_node
    = attribute_store_create_child_if_missing(data_node,
                                              ATTRIBUTE(DELETE_ALL_REQUESTED));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    clear_all_pin_codes_command(supporting_node_unid,
                                endpoint_id,
                                UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));

  TEST_ASSERT_EQUAL(1, attribute_store_get_desired_number(clear_all_node));
}