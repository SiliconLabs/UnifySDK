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
#include "unity.h"
#include "unity_helpers.h"
#include <string.h>
#include <sys/param.h>
#include "ucl_network_management_test.hpp"

#include "zpc_converters.h"
#include "ucl_network_management.h"
#include "contiki_test_helper.h"
#include "mqtt_test_helper.h"
#include "zwave_unid.h"
#include "process.h"

PROCESS_NAME(ucl_network_management_process);

extern "C" {
// Mocks (must be included under extern "C")
#include "zwave_controller_callbacks_mock.h"
#include "zwave_controller_mock.h"
#include "zwave_network_management_mock.h"
#include "uic_mqtt_mock.h"
#include "ucl_nm_neighbor_discovery_mock.h"

#define LOG_TAG "ucl_network_management_test"

#define KEYS_ALL                                                      \
  ((ZWAVE_CONTROLLER_S0_KEY | ZWAVE_CONTROLLER_S2_UNAUTHENTICATED_KEY \
    | ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY | ZWAVE_CONTROLLER_S2_ACCESS_KEY))
const zwave_controller_callbacks_t *nm_callbacks;

const char ucl_nm_topic[]
  = "ucl/by-unid/zw-DEADBEEF-0001/ProtocolController/NetworkManagement";
const char ucl_nm_state_write_topic[]
  = "ucl/by-unid/zw-DEADBEEF-0001/ProtocolController/NetworkManagement/Write";
static char buf[500];  // Buffer used for mqtt message pops
static void init_ucl_network_management();

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  contiki_test_helper_init();
  mqtt_test_helper_init();
  init_ucl_network_management();
}

void tearDown()
{
  TEST_ASSERT_NULL_MESSAGE(mqtt_test_helper_pop_publish(ucl_nm_topic, buf),
                           "Expect no more messages");
}

static sl_status_t my_nm_callback_save(const zwave_controller_callbacks_t *cb,
                                       int n)
{
  nm_callbacks = cb;
  return SL_STATUS_OK;
}

static void init_ucl_network_management()
{
  uint32_t home_id        = 0xdeadbeef;
  zwave_node_id_t node_id = 0x1;
  zwave_controller_register_callbacks_AddCallback(my_nm_callback_save);
  zwave_controller_register_callbacks_ExpectAndReturn(NULL, SL_STATUS_OK);
  zwave_controller_register_callbacks_IgnoreArg_callbacks();
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(node_id);
  zwave_network_management_get_state_IgnoreAndReturn(NM_IDLE);
  ucl_nm_neighbor_discovery_init_Ignore();

  process_start(&ucl_network_management_process, NULL);
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL_MESSAGE(
    1,
    mqtt_test_helper_get_num_subscribers(ucl_nm_state_write_topic),
    "ucl_network_management shall subscribe to this topic on init");
  TEST_ASSERT_EQUAL_MESSAGE(
    1,
    mqtt_test_helper_get_num_publish(ucl_nm_topic),
    "Expect 1 publish to ucl network management state topic");
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"idle", "SupportedStateList": ["add node", "remove node", "reset"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
}

void test_ucl_network_management_init()
{
  TEST_ASSERT_TRUE_MESSAGE(true,
                           "This only tests setUp(), which does the init");
}

void test_ucl_network_management_exit()
{
  uic_mqtt_unretain_Expect(ucl_nm_topic);
  process_exit(&ucl_network_management_process);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_MESSAGE(
    0,
    mqtt_test_helper_get_num_subscribers(ucl_nm_state_write_topic),
    "ucl_network_management shall unsubscribe to this topic on exit");
  TEST_ASSERT_EQUAL_JSON(R"({"State":"idle", "SupportedStateList": []})",
                         mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
  TEST_ASSERT_EQUAL_MESSAGE(2,
                            mqtt_test_helper_get_num_publish(ucl_nm_topic),
                            "Dont expect any additional publish");
}

void test_malformed_json()
{
  const char message[] = R"({"State": "add node",})";
  mqtt_test_helper_publish(ucl_nm_state_write_topic, message, sizeof(message));
  contiki_test_helper_run(0);
}

void test_invalid_state()
{
  const char message[] = R"({"State": "something invalid"})";
  mqtt_test_helper_publish(ucl_nm_state_write_topic, message, sizeof(message));
  contiki_test_helper_run(0);
}

void test_missing_state()
{
  const char message[] = "{}";
  mqtt_test_helper_publish(ucl_nm_state_write_topic, message, sizeof(message));
  contiki_test_helper_run(0);
}

////////////////////////////////////////////////////////////////////////////////
// Test "State": "add node"
////////////////////////////////////////////////////////////////////////////////
static void add_node_over_mqtt(zwave_dsk_t dsk, unsigned int on_dsk_report_len)
{
  char message[] = R"({"State": "add node"})";
  mqtt_test_helper_publish(ucl_nm_state_write_topic, message, strlen(message));
  zwave_network_management_add_node_ExpectAndReturn(SL_STATUS_OK);
  contiki_test_helper_run(0);
  nm_callbacks->on_state_updated(NM_WAITING_FOR_ADD);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"add node", "SupportedStateList": ["idle"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));

  zwave_network_management_keys_set_ExpectAndReturn(true,
                                                    true,
                                                    KEYS_ALL,
                                                    SL_STATUS_OK);
  nm_callbacks->on_keys_report(true, KEYS_ALL);
  contiki_test_helper_run(0);

  nm_callbacks->on_dsk_report(on_dsk_report_len, dsk, KEYS_ALL);
  contiki_test_helper_run(0);
}

static void add_node_over_mqtt_validate_result(zwave_dsk_t dsk,
                                               unsigned int on_dsk_report_len)
{
  add_node_over_mqtt(dsk, on_dsk_report_len);
  char dsk_str[100];
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zpc_converters_dsk_to_str(dsk, dsk_str, sizeof(dsk_str)));
  // Blank out 'input_length' bytes with 'x'
  for (unsigned int i = 0; i < on_dsk_report_len / 2; i++) {
    // Get the position of 'I' in the following sequence: Ixxxx-Ixxxx-Ixxxx-...
    unsigned int index = i * 5 + i;
    for (unsigned int j = 0; j < 5; j++) {
      dsk_str[index + j] = 'x';
    }
  }
  std::string expected(R"({
                      "State": "add node",
                      "StateParameters": {
                        "ProvisioningMode": "ZWaveDSK",
                        "SecurityCode" : ")"
                       + std::string(dsk_str) + R"("
                      },
                      "RequestedStateParameters": [
                        "SecurityCode",
                        "UserAccept",
                        "AllowMultipleInclusions"
                      ],
                      "SupportedStateList": ["idle"]
                    })");
  TEST_ASSERT_EQUAL_JSON(expected.c_str(),
                         mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
  TEST_ASSERT_NULL_MESSAGE(mqtt_test_helper_pop_publish(ucl_nm_topic, buf),
                           "Expect no more messages");
}

void test_add_node_dsk_len()
{
  // clang-format off
  zwave_dsk_t dsk = {0x30, 0x39, 0xad, 0xde, 0xef, 0xbe, 0x10, 0x01,
                     0xad, 0xde, 0xef, 0xbe, 0xee, 0xff, 0x01, 0x00};
  // clang-format on

  add_node_over_mqtt_validate_result(dsk, 0);
  add_node_over_mqtt_validate_result(dsk, 2);
  add_node_over_mqtt_validate_result(dsk, 4);
  add_node_over_mqtt_validate_result(dsk, 6);
  add_node_over_mqtt_validate_result(dsk, 8);
  add_node_over_mqtt_validate_result(dsk, 10);
  add_node_over_mqtt_validate_result(dsk, 12);
  add_node_over_mqtt_validate_result(dsk, 14);
  add_node_over_mqtt_validate_result(dsk, 16);
  // Test on_dsk_report_len of uneven is ignored
  add_node_over_mqtt(dsk, 1);
  TEST_ASSERT_NULL_MESSAGE(mqtt_test_helper_pop_publish(ucl_nm_topic, buf),
                           "Expect no more messages");
  // Test on_dsk_report_len greater than MAX dsk len is ignored
  add_node_over_mqtt(dsk, 18);
  TEST_ASSERT_NULL_MESSAGE(mqtt_test_helper_pop_publish(ucl_nm_topic, buf),
                           "Expect no more messages");
}

void test_add_node_sunny_day()
{
  // clang-format off
  zwave_dsk_t dsk = {0x00, 0x00, 0xad, 0xde, 0xef, 0xbe, 0x10, 0x01,
                     0xad, 0xde, 0xef, 0xbe, 0xee, 0xff, 0x01, 0x00};
  // clang-format on
  add_node_over_mqtt_validate_result(dsk, 2);
  const char message[] = R"({
                            "State": "add node",
                            "StateParameters": {
                              "UserAccept": true,
                              "SecurityCode" : "12345-44510-61374-04097-44510-61374-61183-00256"
                            }
                          })";
  mqtt_test_helper_publish(ucl_nm_state_write_topic, message, sizeof(message));
  dsk[0] = 0x30;
  dsk[1] = 0x39;
  zwave_network_management_dsk_set_ExpectAndReturn(dsk, SL_STATUS_OK);
  contiki_test_helper_run(0);
}

void test_add_node_user_accept_false()
{
  // clang-format off
  zwave_dsk_t dsk = {0x00, 0x00, 0xad, 0xde, 0xef, 0xbe, 0x10, 0x01,
                     0xad, 0xde, 0xef, 0xbe, 0xee, 0xff, 0x01, 0x00};
  // clang-format on
  add_node_over_mqtt_validate_result(dsk, 2);
  const char message[] = R"({
                            "State": "add node",
                            "StateParameters": {
                              "UserAccept": false,
                              "SecurityCode" : "12345-44510-61374-04097-44510-61374-61183-00256"
                            }
                          })";
  mqtt_test_helper_publish(ucl_nm_state_write_topic, message, sizeof(message));
  zwave_network_management_abort_ExpectAndReturn(SL_STATUS_OK);
  contiki_test_helper_run(0);
}

void test_add_node_user_accept_missing()
{
  // clang-format off
  zwave_dsk_t dsk = {0x00, 0x00, 0xad, 0xde, 0xef, 0xbe, 0x10, 0x01,
                     0xad, 0xde, 0xef, 0xbe, 0xee, 0xff, 0x01, 0x00};
  // clang-format on
  add_node_over_mqtt_validate_result(dsk, 2);
  const char message[] = R"({
                              "State": "add node",
                              "StateParameters": {
                                "SecurityCode" : "12345-44510-61374-04097-44510-61374-61183-00256"
                              }
                            })";
  zwave_network_management_add_node_ExpectAndReturn(SL_STATUS_OK);
  mqtt_test_helper_publish(ucl_nm_state_write_topic, message, sizeof(message));
  contiki_test_helper_run(0);
}

void test_add_node_security_code_is_missing()
{
  // clang-format off
  zwave_dsk_t dsk = {0x00, 0x00, 0xad, 0xde, 0xef, 0xbe, 0x10, 0x01,
                     0xad, 0xde, 0xef, 0xbe, 0xee, 0xff, 0x01, 0x00};
  // clang-format on
  add_node_over_mqtt_validate_result(dsk, 2);
  const char message[] = R"({"
                              "State": "add node",
                              "StateParameters": {
                                "UserAccept": true
                              }
                            })";
  mqtt_test_helper_publish(ucl_nm_state_write_topic, message, sizeof(message));
  contiki_test_helper_run(0);
}

void test_add_node_user_accept_no_dsk()
{
  // clang-format off
  zwave_dsk_t dsk = {0x00, 0x00, 0xad, 0xde, 0xef, 0xbe, 0x10, 0x01,
                     0xad, 0xde, 0xef, 0xbe, 0xee, 0xff, 0x01, 0x00};
  // clang-format on
  add_node_over_mqtt_validate_result(dsk, 2);
  const char message[] = R"({
                            "State": "add node",
                            "StateParameters": {
                              "UserAccept": true
                            }
                          })";
  mqtt_test_helper_publish(ucl_nm_state_write_topic, message, sizeof(message));
  dsk[0] = 0x30;
  dsk[1] = 0x39;
  contiki_test_helper_run(0);
}

void test_add_node_malformed_dsk()
{
  // clang-format off
  zwave_dsk_t dsk = {0x00, 0x00, 0xad, 0xde, 0xef, 0xbe, 0x10, 0x01,
                     0xad, 0xde, 0xef, 0xbe, 0xee, 0xff, 0x01, 0x00};
  // clang-format on
  add_node_over_mqtt_validate_result(dsk, 2);
  {  // Test with too large number in dsk
    const char message[] = R"({
                                "State": "add node",
                                "StateParameters": {
                                  "UserAccept": true,
                                  "SecurityCode" : "12345-44510-61374-04097-44510-61374-90000-00256"
                                }
                            })";
    mqtt_test_helper_publish(ucl_nm_state_write_topic,
                             message,
                             sizeof(message));
    contiki_test_helper_run(0);
  }

  {  // Test with too many entries in dsk
    const char message[] = R"({
                                "State": "add node",
                                "StateParameters": {
                                  "UserAccept": true,
                                  "SecurityCode" : "1-2-3-4-5-6-7-8-9"
                                }
                              })";
    mqtt_test_helper_publish(ucl_nm_state_write_topic,
                             message,
                             sizeof(message));
    contiki_test_helper_run(0);
  }

  {  // Test with invalid character in dsk
    const char message[] = R"({
                                "State": "add node",
                                "StateParameters": {
                                  "UserAccept": true,
                                  "SecurityCode" : "1-J-3-4-5-6-7-8"
                                }
                              })";
    mqtt_test_helper_publish(ucl_nm_state_write_topic,
                             message,
                             sizeof(message));
    contiki_test_helper_run(0);
  }
}

void test_add_node_no_state_parameters()
{
  // clang-format off
  zwave_dsk_t dsk = {0x00, 0x00, 0xad, 0xde, 0xef, 0xbe, 0x10, 0x01,
                     0xad, 0xde, 0xef, 0xbe, 0xee, 0xff, 0x01, 0x00};
  // clang-format on

  add_node_over_mqtt_validate_result(dsk, 2);
  const char message[] = R"({"State": "add node"})";
  zwave_network_management_add_node_ExpectAndReturn(SL_STATUS_OK);
  mqtt_test_helper_publish(ucl_nm_state_write_topic, message, sizeof(message));
  contiki_test_helper_run(0);
}

void test_add_allow_multiple_inclusions()
{
  TEST_ASSERT_NOT_NULL(nm_callbacks->on_state_updated);
  // Set AllowMultipleInclusions to true
  {
    const char message[] = R"({
                                "State": "add node",
                                "StateParameters": {"AllowMultipleInclusions": true}
                              })";
    mqtt_test_helper_publish(ucl_nm_state_write_topic,
                             message,
                             sizeof(message));
  }
  zwave_network_management_add_node_ExpectAndReturn(SL_STATUS_OK);
  nm_callbacks->on_state_updated(NM_WAITING_FOR_ADD);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"add node", "SupportedStateList":["idle"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));

  // When Network Management goes to NM_IDLE state,
  // expect zwave_network_management_add_node to be called
  zwave_network_management_add_node_ExpectAndReturn(SL_STATUS_OK);
  nm_callbacks->on_state_updated(NM_IDLE);

  contiki_test_helper_run(0);
  TEST_ASSERT_NULL_MESSAGE(mqtt_test_helper_pop_publish(ucl_nm_topic, buf),
                           "Expect no mqtt messages are published");
  // Enter idle from MQTT side,
  // should go not trigger zwave_network_management_add_node
  {
    const char message[] = R"({"State": "idle"})";
    mqtt_test_helper_publish(ucl_nm_state_write_topic,
                             message,
                             sizeof(message));
  }
  zwave_network_management_abort_ExpectAndReturn(SL_STATUS_OK);
  contiki_test_helper_run(0);

  nm_callbacks->on_state_updated(NM_IDLE);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State": "idle", "SupportedStateList":["add node", "remove node", "reset"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
  // Test add node doesn't get re-triggered when NM_IDLE is reported
  {
    const char message[] = R"({"State": "add node"})";
    mqtt_test_helper_publish(ucl_nm_state_write_topic,
                             message,
                             sizeof(message));
  }
  zwave_network_management_add_node_ExpectAndReturn(SL_STATUS_OK);
  nm_callbacks->on_state_updated(NM_WAITING_FOR_ADD);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"add node", "SupportedStateList":["idle"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));

  nm_callbacks->on_state_updated(NM_IDLE);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"idle", "SupportedStateList":["add node", "remove node", "reset"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));

  // Test add node with "AllowMultipleInclusions": false
  // doesn't get re-triggered when NM_IDLE is reported
  {
    const char message[] = R"({
                                "State": "add node",
                                "StateParameters": {"AllowMultipleInclusions": false}
                              })";
    mqtt_test_helper_publish(ucl_nm_state_write_topic,
                             message,
                             sizeof(message));
  }
  zwave_network_management_add_node_ExpectAndReturn(SL_STATUS_OK);
  contiki_test_helper_run(0);
  nm_callbacks->on_state_updated(NM_IDLE);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"idle", "SupportedStateList":["add node", "remove node", "reset"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
}

void test_case_insensitive_json()
{
  // clang-format off
  zwave_dsk_t dsk = {0x00, 0x00, 0xad, 0xde, 0xef, 0xbe, 0x10, 0x01,
                     0xad, 0xde, 0xef, 0xbe, 0xee, 0xff, 0x01, 0x00};
  // clang-format on
  {
    const char message[] = R"({
                                "StaTe": "AdD nOdE",
                                "StAtepaRamEtErS": {"ALloWMuLtiPleInclUsiOns": false}
                              })";
    mqtt_test_helper_publish(ucl_nm_state_write_topic,
                             message,
                             sizeof(message));
  }
  zwave_network_management_add_node_ExpectAndReturn(SL_STATUS_OK);
  contiki_test_helper_run(0);

  zwave_network_management_keys_set_ExpectAndReturn(true,
                                                    true,
                                                    KEYS_ALL,
                                                    SL_STATUS_OK);
  TEST_ASSERT_NOT_NULL(nm_callbacks->on_keys_report);
  nm_callbacks->on_keys_report(true, KEYS_ALL);
  contiki_test_helper_run(0);

  nm_callbacks->on_dsk_report(2, dsk, KEYS_ALL);
  contiki_test_helper_run(0);

  const char *expected = R"({
                              "State": "add node",
                              "StateParameters": {
                                "ProvisioningMode": "ZWaveDSK",
                                "SecurityCode" : "xxxxx-44510-61374-04097-44510-61374-61183-00256"
                              },
                              "RequestedStateParameters": [
                                "SecurityCode",
                                "UserAccept",
                                "AllowMultipleInclusions"
                              ],
                              "SupportedStateList":["idle"]
                            })";
  TEST_ASSERT_EQUAL_JSON(expected,
                         mqtt_test_helper_pop_publish(ucl_nm_topic, buf));

  {
    const char message[] = R"({
                                "StAtE": "aDd nOdE",
                                "StaTeParamEtErs": {
                                  "UsErAcCepT": true,
                                  "SecuRItyCOde" : "12345-44510-61374-04097-44510-61374-61183-00256"
                                }
                              })";
    mqtt_test_helper_publish(ucl_nm_state_write_topic,
                             message,
                             sizeof(message));
  }
  dsk[0] = 0x30;
  dsk[1] = 0x39;
  zwave_network_management_dsk_set_ExpectAndReturn(dsk, SL_STATUS_OK);
  contiki_test_helper_run(0);
}

////////////////////////////////////////////////////////////////////////////////
// Test "State": "remove node"
////////////////////////////////////////////////////////////////////////////////
void test_remove_node()
{
  const char message[] = R"({"State": "remove node"})";
  mqtt_test_helper_publish(ucl_nm_state_write_topic, message, sizeof(message));
  zwave_network_management_remove_node_ExpectAndReturn(SL_STATUS_OK);
  contiki_test_helper_run(0);
}

////////////////////////////////////////////////////////////////////////////////
// Test "State": "remove failed node"
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Test "State": "reset"
////////////////////////////////////////////////////////////////////////////////
void test_reset_success()
{
  const char message[] = R"({"State":"reset"})";
  mqtt_test_helper_publish(ucl_nm_state_write_topic, message, sizeof(message));
  zwave_controller_reset_Expect();
  zwave_controller_is_reset_ongoing_ExpectAndReturn(true);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(R"({"State":"reset", "SupportedStateList":[]})",
                         mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
}

void test_reset_failed()
{
  const char message[] = R"({"State":"reset"})";
  mqtt_test_helper_publish(ucl_nm_state_write_topic, message, sizeof(message));
  zwave_controller_reset_Expect();
  zwave_controller_is_reset_ongoing_ExpectAndReturn(false);
  contiki_test_helper_run(0);
  TEST_ASSERT_NULL_MESSAGE(mqtt_test_helper_pop_publish(ucl_nm_topic, buf),
                           "No publication should have been made");
}

////////////////////////////////////////////////////////////////////////////////
// Test "State": "idle"
////////////////////////////////////////////////////////////////////////////////
void test_idle()
{
  const char message[] = R"({"State":"idle"})";
  mqtt_test_helper_publish(ucl_nm_state_write_topic, message, sizeof(message));
  zwave_network_management_abort_ExpectAndReturn(SL_STATUS_OK);
  contiki_test_helper_run(0);
}

////////////////////////////////////////////////////////////////////////////////
// Test invalid states
////////////////////////////////////////////////////////////////////////////////
void test_invalid_states()
{
  std::vector<std::string> invalid_states = {"join network",
                                             "leave network",
                                             "node interview",
                                             "network repair",
                                             "network maintenance",
                                             "temporarily offline",
                                             "scan mode",
                                             "gibberish state",
                                             "æøå¨¨'!@?\"\0 "};
  for (std::string &state: invalid_states) {
    std::string message = R"({"State":")" + state + R"("})";
    mqtt_test_helper_publish(ucl_nm_state_write_topic,
                             message.c_str(),
                             message.length());
    contiki_test_helper_run(0);
    // Don't expect anything to happen when receiving these states
  }
}

////////////////////////////////////////////////////////////////////////////////
// Test NetworkManagement Callbacks
////////////////////////////////////////////////////////////////////////////////
void test_nm_callback_on_state_updated()
{
  TEST_ASSERT_NOT_NULL(nm_callbacks->on_state_updated);
  nm_callbacks->on_state_updated(NM_IDLE);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"idle", "SupportedStateList":["add node", "remove node", "reset"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
  nm_callbacks->on_state_updated(NM_WAITING_FOR_ADD);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"add node", "SupportedStateList":["idle"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));

  nm_callbacks->on_state_updated(NM_NODE_FOUND);
  contiki_test_helper_run(0);
  TEST_ASSERT_NULL(mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
  nm_callbacks->on_state_updated(NM_WAIT_FOR_PROTOCOL);
  contiki_test_helper_run(0);
  TEST_ASSERT_NULL(mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
  nm_callbacks->on_state_updated(NM_WAIT_FOR_SECURE_ADD);
  contiki_test_helper_run(0);
  TEST_ASSERT_NULL(mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
  nm_callbacks->on_state_updated(NM_PREPARE_SUC_INCLISION);
  contiki_test_helper_run(0);
  TEST_ASSERT_NULL(mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
  nm_callbacks->on_state_updated(NM_WAIT_FOR_SUC_INCLUSION);
  contiki_test_helper_run(0);
  TEST_ASSERT_NULL(mqtt_test_helper_pop_publish(ucl_nm_topic, buf));

  nm_callbacks->on_state_updated(NM_SET_DEFAULT);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(R"({"State":"reset", "SupportedStateList":[]})",
                         mqtt_test_helper_pop_publish(ucl_nm_topic, buf));

  nm_callbacks->on_state_updated(NM_LEARN_MODE);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"join network", "SupportedStateList":["idle"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
  nm_callbacks->on_state_updated(NM_LEARN_MODE_STARTED);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"join network", "SupportedStateList":["idle"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
  nm_callbacks->on_state_updated(NM_WAIT_FOR_SECURE_LEARN);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"join network", "SupportedStateList":["idle"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
  nm_callbacks->on_state_updated(NM_WAITING_FOR_NODE_REMOVAL);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"remove node", "SupportedStateList":["idle"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));

  nm_callbacks->on_state_updated(NM_WAITING_FOR_FAILED_NODE_REMOVAL);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"remove node", "SupportedStateList":["idle"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
  nm_callbacks->on_state_updated(NM_REPLACE_FAILED_REQ);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"remove node", "SupportedStateList":["idle"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
  nm_callbacks->on_state_updated(NM_SEND_NOP);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"remove node", "SupportedStateList":["idle"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
  nm_callbacks->on_state_updated(NM_WAIT_FOR_TX_TO_SELF_DESTRUCT);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"remove node", "SupportedStateList":["idle"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
  nm_callbacks->on_state_updated(NM_WAIT_FOR_SELF_DESTRUCT_REMOVAL);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"remove node", "SupportedStateList":["idle"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
  nm_callbacks->on_state_updated(NM_FAILED_NODE_REMOVE);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"remove node", "SupportedStateList":["idle"]})",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
}

void test_nm_callback_on_node_added()
{
  zwave_node_info_t nif   = {0};
  zwave_node_id_t node_id = 3;
  // clang-format off
  zwave_dsk_t dsk = {0x30, 0x39, 0xad, 0xde, 0xef, 0xbe, 0x10, 0x01,
                     0xad, 0xde, 0xef, 0xbe, 0xee, 0xff, 0x01, 0x00};
  // clang-format on
  nm_callbacks->on_node_added(SL_STATUS_OK,
                              &nif,
                              node_id,
                              dsk,
                              KEYS_ALL,
                              ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE,
                              PROTOCOL_ZWAVE);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_JSON(
    R"({"State":"idle", "SupportedStateList":["add node", "remove node", "reset"] })",
    mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
}

void test_nm_callback_on_new_network_entered()
{
  zwave_home_id_t home_id = 0x12345678;
  zwave_node_id_t node_id = 3;
  uic_mqtt_unretain_Expect(ucl_nm_topic);
  TEST_ASSERT_NOT_NULL(nm_callbacks->on_network_address_update);
  nm_callbacks->on_network_address_update(home_id, node_id);
  TEST_ASSERT_EQUAL(
    0,
    mqtt_test_helper_get_num_subscribers(ucl_nm_state_write_topic));
  TEST_ASSERT_EQUAL(1,
                    mqtt_test_helper_get_num_subscribers(
                      "ucl/by-unid/zw-12345678-0003/ProtocolController/"
                      "NetworkManagement/Write"));
}

void test_nm_callback_on_keys_report()
{
  zwave_network_management_keys_set_ExpectAndReturn(true,
                                                    true,
                                                    KEYS_ALL,
                                                    SL_STATUS_OK);
  nm_callbacks->on_keys_report(true, KEYS_ALL);
  contiki_test_helper_run(0);
}

void test_nm_callback_on_dsk_report()
{
  TEST_ASSERT_NOT_NULL(nm_callbacks->on_dsk_report);
  // clang-format off
  zwave_dsk_t dsk = {0x30, 0x39, 0xad, 0xde, 0xef, 0xbe, 0x10, 0x01,
                     0xad, 0xde, 0xef, 0xbe, 0xee, 0xff, 0x01, 0x00};
  // clang-format on
  std::vector<zwave_keyset_t> keysets
    = {ZWAVE_CONTROLLER_S0_KEY,
       ZWAVE_CONTROLLER_S2_UNAUTHENTICATED_KEY,
       ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY,
       ZWAVE_CONTROLLER_S2_ACCESS_KEY};

  for (zwave_keyset_t keys: keysets) {
    // if not access or authenticated, expect dsk_set immediately
    if (!((keys & ZWAVE_CONTROLLER_S2_ACCESS_KEY)
          || (keys & ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY))) {
      zwave_network_management_dsk_set_ExpectAndReturn(dsk, SL_STATUS_OK);
    }
    nm_callbacks->on_dsk_report(0, dsk, keys);
    contiki_test_helper_run(0);
    // if access or authenticated expect request for dsk over mqtt
    if (((keys & ZWAVE_CONTROLLER_S2_ACCESS_KEY)
         || (keys & ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY))) {
      char dsk_str[100];
      TEST_ASSERT_EQUAL(
        SL_STATUS_OK,
        zpc_converters_dsk_to_str(dsk, dsk_str, sizeof(dsk_str)));
      std::string expected(R"({
                      "State": "add node",
                      "StateParameters": {
                        "ProvisioningMode": "ZWaveDSK",
                        "SecurityCode" : ")"
                           + std::string(dsk_str) + R"("
                      },
                      "RequestedStateParameters": [
                        "SecurityCode",
                        "UserAccept",
                        "AllowMultipleInclusions"
                      ],
                      "SupportedStateList":["idle"]
                    })");
      TEST_ASSERT_EQUAL_JSON(expected.c_str(),
                             mqtt_test_helper_pop_publish(ucl_nm_topic, buf));
    }
  }
}

void test_ucl_network_management_remove_offline_node()
{
  zwave_node_id_t test_node_id_failed = 0x78;
  zwave_network_management_remove_failed_ExpectAndReturn(test_node_id_failed,
                                                         SL_STATUS_OK);
  ucl_network_management_remove_offline_node(test_node_id_failed);
  contiki_test_helper_run(0);
}

void test_ucl_network_management_remove_multiple_offline_nodes()
{
  zwave_node_id_t test_node_id_failed_1 = 0x81;
  zwave_node_id_t test_node_id_failed_2 = 0x82;
  zwave_network_management_get_state_StopIgnore();
  zwave_network_management_get_state_ExpectAndReturn(NM_SET_DEFAULT);
  contiki_test_helper_run(10);
  ucl_network_management_remove_offline_node(test_node_id_failed_1);
  zwave_network_management_get_state_ExpectAndReturn(NM_SET_DEFAULT);
  contiki_test_helper_run(10);
  ucl_network_management_remove_offline_node(test_node_id_failed_2);

  zwave_network_management_get_state_ExpectAndReturn(NM_IDLE);
  zwave_network_management_remove_failed_ExpectAndReturn(test_node_id_failed_1,
                                                         SL_STATUS_OK);
  contiki_test_helper_run(1000);

  zwave_network_management_get_state_ExpectAndReturn(NM_IDLE);
  zwave_network_management_remove_failed_ExpectAndReturn(test_node_id_failed_2,
                                                         SL_STATUS_OK);
  contiki_test_helper_run(1000);
}

void test_ucl_network_management_request_node_neighbor_update()
{
  zwave_node_id_t test_node_id_neighbor_update = 0x98;
  ucl_nm_trigger_node_neighbor_update_Expect(test_node_id_neighbor_update);
  ucl_network_management_request_node_neighbor_update(
    test_node_id_neighbor_update);
  contiki_test_helper_run(0);
}
}
