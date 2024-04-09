/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include <stdlib.h>
#include "workaround_for_test.hpp"

extern "C" {

#include "zwave_command_class_user_credential.h"
#include "zwave_command_class_user_credential_types.h"
#include "zwave_command_classes_utils.h"
#include "unity.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "attribute_store_type_registration.h"
#include "zpc_attribute_store_type_registration.h"

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
#include "dotdot_mqtt_mock.h"
#include "dotdot_mqtt_generated_commands_mock.h"
// Used for delayed interview
#include "zwave_network_management_mock.h"

#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_##type

static zwave_command_handler_t handler = {};

// This assumes that XXX_GET and XXX_SET id are > 0
struct bound_functions {
  uint8_t get_func_id;
  uint8_t set_func_id;
};
static std::map<attribute_store_type_t, bound_functions> attributes_binding
  = {{ATTRIBUTE(NUMBER_OF_USERS), {USER_CAPABILITIES_GET, 0}},
     {ATTRIBUTE(SUPPORT_CREDENTIAL_CHECKSUM), {CREDENTIAL_CAPABILITIES_GET, 0}},
     {ATTRIBUTE(ALL_USERS_CHECKSUM), {ALL_USERS_CHECKSUM_GET, 0}},
     {ATTRIBUTE(USER_UNIQUE_ID), {USER_GET, 0}},
     {ATTRIBUTE(USER_OPERATION_TYPE), {0, USER_SET}},
     {ATTRIBUTE(CREDENTIAL_SLOT), {CREDENTIAL_GET, 0}},
     {ATTRIBUTE(CREDENTIAL_OPERATION_TYPE), {0, CREDENTIAL_SET}}};

// Filed with resolver function for given ID in attribute_resolver_register_rule_stub based on
// attributes_binding map.
//
// e.g : resolver_functions[USER_CAPABILITIES_GET] = user_capabilities_get_func()
static std::map<uint8_t, attribute_resolver_function_t> resolver_functions;

// Buffer for frame
static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

const uint8_t MAX_CHAR_SIZE = 64;
/////////////////////////////////////////////////////
// C++ HELPERS
/////////////////////////////////////////////////////
}  // Extern C

#ifdef __cplusplus
template<class T> void helper_test_attribute_store_values(
  std::map<attribute_store_type_t, T> attribute_map_values,
  attribute_store_node_t parent = ATTRIBUTE_STORE_INVALID_NODE)
{
  if (parent == ATTRIBUTE_STORE_INVALID_NODE) {
    parent = endpoint_id_node;
  }

  for (auto &attr: attribute_map_values) {
    T value;
    sl_status_t status = attribute_store_get_child_reported(parent,
                                                            attr.first,
                                                            &value,
                                                            sizeof(value));

    std::string message
      = "Can't get reported value of "
        + std::string(attribute_store_get_type_name(attr.first));

    TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK, status, message.c_str());

    message = "Mismatch value for attribute : "
              + std::string(attribute_store_get_type_name(attr.first));
    TEST_ASSERT_EQUAL_MESSAGE(attr.second, value, message.c_str());
  }
}
#endif

extern "C" {
/////////////////////////////////////////////////////
// HELPERS
/////////////////////////////////////////////////////
struct uint16_exploded {
  uint8_t msb;  // Most Significant Bit
  uint8_t lsb;  // Less Significant Bit
};
uint16_exploded explode_uint16(uint16_t value)
{
  uint8_t msb = (value & 0xFF00) >> 8;
  uint8_t lsb = (value & 0x00FF);
  return uint16_exploded {msb, lsb};
}

struct credential_structure_nodes {
  attribute_store_node_t user_id_node;
  attribute_store_node_t credential_type_node;
  attribute_store_node_t credential_slot_node;
};

/**
 * @brief Create credential structure and return associated nodes
 * 
 * @param user_id 
 * @param credential_type 
 * @param credential_slot 
 * @param value_state If DESIRED_ATTRIBUTE : credential_type/credential_slot will have their desired value setup, reported otherwise
 * @return credential_structure_nodes 
 */
credential_structure_nodes helper_create_credential_structure(
  user_credential_user_unique_id_t user_id,
  user_credential_type_t credential_type,
  user_credential_slot_t credential_slot,
  attribute_store_node_value_state_t value_state = DESIRED_ATTRIBUTE)
{
  credential_structure_nodes nodes;
  nodes.user_id_node = attribute_store_emplace(endpoint_id_node,
                                               ATTRIBUTE(USER_UNIQUE_ID),
                                               &user_id,
                                               sizeof(user_id));
  // It should be always reported
  nodes.credential_type_node
    = attribute_store_emplace(nodes.user_id_node,
                              ATTRIBUTE(CREDENTIAL_TYPE),
                              &credential_type,
                              sizeof(credential_type));

  if (value_state == DESIRED_ATTRIBUTE) {
    nodes.credential_slot_node
      = attribute_store_emplace_desired(nodes.credential_type_node,
                                        ATTRIBUTE(CREDENTIAL_SLOT),
                                        &credential_slot,
                                        sizeof(credential_slot));
  } else {
    nodes.credential_slot_node
      = attribute_store_emplace(nodes.credential_type_node,
                                ATTRIBUTE(CREDENTIAL_SLOT),
                                &credential_slot,
                                sizeof(credential_slot));
  }

  return nodes;
}

void helper_test_get_no_args_happy_case(uint8_t get_command_id)
{
  auto &get_func = resolver_functions[get_command_id];

  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL_MESSAGE(
    get_func,
    "Couldn't find get function in resolver_functions.");
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            get_func(0, received_frame, &received_frame_size),
                            "Get function should have returned OK");

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_USER_CREDENTIAL, get_command_id};
  TEST_ASSERT_EQUAL_MESSAGE(sizeof(expected_frame),
                            received_frame_size,
                            "Get frame size is incorrect");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(expected_frame,
                                        received_frame,
                                        received_frame_size,
                                        "Get frame contents mismatch");
}

/**
 * @brief Test given SET or GET function with args
 * 
 * > Can also test if SET or GET command return SL_STATUS_NOT_SUPPORTED if node == INVALID_STORAGE_TYPE
 * 
 * @param command_id  Command that should be in resolver_functions
 * @param base_node   Base node on which the GET function will be called. If INVALID_STORAGE_TYPE expect the SET or GET function to return SL_STATUS_NOT_SUPPORTED and return.
 * @param nodes       Node data that will be used to check and construct the SET or GET frame (order matter since the first one will be the first argument, etc..)
 * @param extra_bytes Extra bytes that will be added to the frame
 */
void helper_test_set_get_with_args(
  uint8_t command_id,
  attribute_store_node_t base_node,
  std::vector<
    std::pair<attribute_store_node_t, attribute_store_node_value_state_t>> nodes
  = {},
  std::vector<uint8_t> extra_bytes = {})
{
  auto &command_func = resolver_functions[command_id];

  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL_MESSAGE(
    command_func,
    "Couldn't find command function in resolver_functions.");

  if (base_node == ATTRIBUTE_STORE_INVALID_NODE) {
    TEST_ASSERT_EQUAL_MESSAGE(
      SL_STATUS_NOT_SUPPORTED,
      command_func(base_node, received_frame, &received_frame_size),
      "Command function should have returned SL_STATUS_NOT_SUPPORTED since no "
      "node "
      "was provided");
    return;
  }

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    command_func(base_node, received_frame, &received_frame_size),
    "Command function should have returned SL_STATUS_OK");

  std::vector<uint8_t> expected_frame
    = {COMMAND_CLASS_USER_CREDENTIAL, command_id};

  for (auto &node_data: nodes) {
    auto node             = node_data.first;
    auto node_value_state = node_data.second;

    auto node_type         = attribute_store_get_node_type(node);
    auto node_storage_type = attribute_store_get_storage_type(node_type);

    sl_status_t status;
    switch (node_storage_type) {
      case U8_STORAGE_TYPE: {
        uint8_t uint8_value;
        status = attribute_store_read_value(node,
                                            node_value_state,
                                            &uint8_value,
                                            sizeof(uint8_value));
        expected_frame.push_back(uint8_value);
      } break;
      case U16_STORAGE_TYPE: {
        uint16_t uint16_value;
        status               = attribute_store_read_value(node,
                                            node_value_state,
                                            &uint16_value,
                                            sizeof(uint16_value));
        auto exploded_uint16 = explode_uint16(uint16_value);
        expected_frame.push_back(exploded_uint16.msb);
        expected_frame.push_back(exploded_uint16.lsb);
      } break;
      // Variable length field
      case BYTE_ARRAY_STORAGE_TYPE: {
        // Length node should be a parent of the data node
        auto data_length_node = attribute_store_get_node_parent(node);

        uint8_t data_length = 0;
        attribute_store_read_value(data_length_node,
                                   node_value_state,
                                   &data_length,
                                   sizeof(data_length));
        if (data_length == 0) {
          TEST_FAIL_MESSAGE(
            "Can't get data_length for BYTE_ARRAY_STORAGE_TYPE type");
        }

        std::vector<uint8_t> data;
        data.resize(data_length);
        status = attribute_store_read_value(node,
                                            node_value_state,
                                            data.data(),
                                            data_length);

        expected_frame.push_back(data_length);
        for (uint8_t i = 0; i < data_length; i++) {
          expected_frame.push_back(data[i]);
        }

      } break;
      case C_STRING_STORAGE_TYPE: {
        char reported_c_str[MAX_CHAR_SIZE];

        status                       = attribute_store_get_reported_string(node,
                                                     reported_c_str,
                                                     MAX_CHAR_SIZE);
        std::string reported_cpp_str = reported_c_str;
        if (reported_cpp_str.empty()) {
          TEST_FAIL_MESSAGE("Can't get string value");
        }

        expected_frame.push_back(reported_cpp_str.length());
        for (const char &c: reported_cpp_str) {
          expected_frame.push_back(c);
        }

      } break;
      default:
        TEST_FAIL_MESSAGE(
          "Unkown storage type in helper_test_set_get_with_args_happy_case");
    }

    std::string message
      = "Should be able to get value of "
        + std::string(attribute_store_type_get_node_type_name(node));
    TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK, status, message.c_str());
  }

  for(auto &byte : extra_bytes) {
    expected_frame.push_back(byte);
  }

  TEST_ASSERT_EQUAL_MESSAGE(expected_frame.size(),
                            received_frame_size,
                            "Command frame size is incorrect");
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(expected_frame.data(),
                                        received_frame,
                                        received_frame_size,
                                        "Command frame contents mismatch");
}

void helper_test_string_value(
  std::map<attribute_store_type_t, std::string> attribute_map_values,
  attribute_store_node_t parent = ATTRIBUTE_STORE_INVALID_NODE)
{
  if (parent == ATTRIBUTE_STORE_INVALID_NODE) {
    parent = endpoint_id_node;
  }

  for (auto &attr: attribute_map_values) {
    char reported_c_str[MAX_CHAR_SIZE];

    auto str_node = attribute_store_get_first_child_by_type(parent, attr.first);
    sl_status_t status = attribute_store_get_reported_string(str_node,
                                                             reported_c_str,
                                                             MAX_CHAR_SIZE);

    std::string message
      = "Can't get string reported value of "
        + std::string(attribute_store_get_type_name(attr.first));

    TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK, status, message.c_str());

    message = "Mismatch value for attribute : "
              + std::string(attribute_store_get_type_name(attr.first));
    TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(attr.second.c_str(),
                                         reported_c_str,
                                         attr.second.size() + 1,
                                         message.c_str());
  }
}

/////////////////////////////////////////////////////
// Test case
/////////////////////////////////////////////////////
// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  std::string message;
  std::string node_name = attribute_store_get_type_name(node_type);

  if (attributes_binding.find(node_type) != attributes_binding.end()) {
    const auto func = attributes_binding[node_type];

    if (func.get_func_id == 0) {
      message
        = "GET function should not be defined for " + node_name
          + ". Did you forget to add it to the attributes_binding variable ?";
      TEST_ASSERT_NULL_MESSAGE(get_func, message.c_str());
    } else {
      message = "SET function should be defined for " + node_name
                + ". Did you forget to remove it to the attributes_binding "
                  "variable ?";
      TEST_ASSERT_NOT_NULL_MESSAGE(get_func, message.c_str());
      resolver_functions[func.get_func_id] = get_func;
    }

    if (func.set_func_id == 0) {
      message
        = "SET function should not be defined for " + node_name
          + ". Did you forget to add it to the attributes_binding variable ?";
      TEST_ASSERT_NULL_MESSAGE(set_func, message.c_str());
    } else {
      message = "SET function should be defined for " + node_name
                + ". Did you forget to remove it to the attributes_binding "
                  "variable ?";
      TEST_ASSERT_NOT_NULL_MESSAGE(set_func, message.c_str());
      resolver_functions[func.set_func_id] = set_func;
    }
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS,
                    handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_USER_CREDENTIAL, handler.command_class);
  TEST_ASSERT_EQUAL(1, handler.version);
  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_NULL(handler.support_handler);
  TEST_ASSERT_FALSE(handler.manual_security_validation);

  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_register_known_attribute_types();
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
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
  zpc_attribute_store_test_helper_create_network();

  // Unset previous definition get/set functions
  for (auto &r: resolver_functions) {
    r.second = NULL;
  }

  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size = 0;
  // Unset previous definition of handler
  memset(&handler, 0, sizeof(zwave_command_handler_t));

  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);
  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_user_credential_init());
}

/// Called after each and every test
void tearDown() {}

////////////////////////////////////////////////////////////////////////////
// User Capabilities Get/Report
////////////////////////////////////////////////////////////////////////////
void test_user_credential_user_capabilities_get_happy_case()
{
  helper_test_get_no_args_happy_case(USER_CAPABILITIES_GET);
}

void test_user_credential_user_capabilities_report_happy_case()
{
  uint16_t number_of_users;
  user_credential_supported_credential_rules_t cred_rule_bitmask;
  uint8_t username_max_length;
  uint8_t support_user_schedule;
  uint8_t support_all_users_checksum;
  uint8_t support_user_checksum;
  uint8_t supported_user_types_bitmask_length;
  user_credential_supported_user_type_bitmask_t supported_user_types_bitmask;

  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  // Test reported values based on the variables in this function
  auto test_reported_values = [&]() {
    // Create frame object
    std::vector<uint8_t> report_frame = {
      COMMAND_CLASS_USER_CREDENTIAL,
      USER_CAPABILITIES_REPORT,
      (uint8_t)((number_of_users & 0xFF00) >> 8),  // MSB
      (uint8_t)(number_of_users & 0x00FF),         // LSB
      cred_rule_bitmask,
      username_max_length,
      (uint8_t)((support_user_schedule << 7) | (support_all_users_checksum << 6)
                | (support_user_checksum << 5)),
      supported_user_types_bitmask_length};

    // Add bitmask value
    for (uint8_t i = 0; i < supported_user_types_bitmask_length; i++) {
      uint8_t offset   = i;
      uint8_t shift    = 8 * offset;
      uint32_t bitmask = 0xFF << shift;

      uint8_t value_8bit = (supported_user_types_bitmask & bitmask) >> shift;
      report_frame.push_back(value_8bit);
    }

    // Do the report
    TEST_ASSERT_EQUAL(
      SL_STATUS_OK,
      handler.control_handler(&info, report_frame.data(), report_frame.size()));

    // Define data structures
    std::map<attribute_store_type_t, uint8_t> uint8_attribute_map = {
      {ATTRIBUTE(SUPPORTED_CREDENTIAL_RULES), cred_rule_bitmask},
      {ATTRIBUTE(MAX_USERNAME_LENGTH), username_max_length},
      {ATTRIBUTE(SUPPORT_USER_SCHEDULE), support_user_schedule},
      {ATTRIBUTE(SUPPORT_ALL_USERS_CHECKSUM), support_all_users_checksum},
      {ATTRIBUTE(SUPPORT_USER_CHECKSUM), support_user_checksum},
    };

    std::map<attribute_store_type_t, uint16_t> uint16_attribute_map = {
      {ATTRIBUTE(NUMBER_OF_USERS), number_of_users},
    };
    std::map<attribute_store_type_t, uint32_t> uint32_attribute_map = {
      {ATTRIBUTE(SUPPORTED_USER_TYPES), supported_user_types_bitmask},
    };

    // Test values
    helper_test_attribute_store_values(uint8_attribute_map);
    helper_test_attribute_store_values(uint16_attribute_map);
    helper_test_attribute_store_values(uint32_attribute_map);

    attribute_store_node_t all_user_checksum_node
      = attribute_store_get_node_child_by_type(endpoint_id_node,
                                               ATTRIBUTE(ALL_USERS_CHECKSUM),
                                               0);
    if (support_all_users_checksum) {
      TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                    all_user_checksum_node,
                                    "ALL_USERS_CHECKSUM node should exists "
                                    "since all_user_checksum_node == true");
    } else {
      TEST_ASSERT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                all_user_checksum_node,
                                "ALL_USERS_CHECKSUM node should NOT exists "
                                "since all_user_checksum_node == false");
    }

    // Remove node for next tests cases
    attribute_store_delete_node(all_user_checksum_node);
  };

  printf("Test with first set of data\n");
  number_of_users                     = 1212;
  cred_rule_bitmask                   = 0b00000111;
  username_max_length                 = 12;
  support_user_schedule               = 1;
  support_all_users_checksum          = 1;
  support_user_checksum               = 1;
  supported_user_types_bitmask_length = 2;
  supported_user_types_bitmask        = 0b111111111;
  // Do the testing
  test_reported_values();

  printf("Test with second set of data\n");
  number_of_users                     = 1313;
  cred_rule_bitmask                   = 0b00011111;
  username_max_length                 = 21;
  support_user_schedule               = 0;
  support_all_users_checksum          = 0;
  support_user_checksum               = 0;
  supported_user_types_bitmask_length = 4;
  supported_user_types_bitmask        = 0xFFFFFFFF;
  // Do the testing
  test_reported_values();
}

////////////////////////////////////////////////////////////////////////////
// Credential Capabilities Get/Report
////////////////////////////////////////////////////////////////////////////
void test_user_credential_credential_capabilities_get_happy_case()
{
  helper_test_get_no_args_happy_case(CREDENTIAL_CAPABILITIES_GET);
}

void test_user_credential_credential_capabilities_report_happy_case()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  auto create_report_frame
    = [&](uint8_t credential_checksum_support,
          std::vector<user_credential_type_t> credential_type,
          std::vector<uint8_t> cl_support,
          std::vector<uint16_t> supported_credential_slots,
          std::vector<uint16_t> min_length,
          std::vector<uint16_t> max_length) {
        std::vector<uint8_t> report_frame
          = {COMMAND_CLASS_USER_CREDENTIAL,
             CREDENTIAL_CAPABILITIES_REPORT,
             (uint8_t)(credential_checksum_support << 7),
             (uint8_t)credential_type.size()};

        auto push_uint16 = [&](std::vector<uint16_t> vector) {
          for (auto &v: vector) {
            report_frame.push_back((uint8_t)((v & 0xFF00) >> 8));
            report_frame.push_back((uint8_t)(v & 0x00FF));
          }
        };

        for (auto &c: credential_type) {
          report_frame.push_back(c);
        }

        for (auto &cl: cl_support) {
          report_frame.push_back((uint8_t)(cl << 7));
        }

        push_uint16(supported_credential_slots);
        push_uint16(min_length);
        push_uint16(max_length);

        return report_frame;
      };

  uint8_t credential_checksum_support                 = 1;
  std::vector<user_credential_type_t> credential_type = {1, 3, 4, 5};
  std::vector<uint8_t> cl_support                     = {1, 0, 0, 1};
  std::vector<uint16_t> supported_credential_slots = {1233, 11233, 21233, 33};
  std::vector<uint16_t> min_length                 = {2, 2362, 255, 1255};
  std::vector<uint16_t> max_length = {5632, 15632, 25632, 32568};

  auto test_report_values = [&]() {
    uint8_t reported_credential_checksum_support;
    attribute_store_node_t support_node
      = attribute_store_get_first_child_by_type(
        endpoint_id_node,
        ATTRIBUTE(SUPPORT_CREDENTIAL_CHECKSUM));

    TEST_ASSERT_NOT_EQUAL_MESSAGE(
      ATTRIBUTE_STORE_INVALID_NODE,
      support_node,
      "credential checksum support node should be defined");

    attribute_store_get_reported(support_node,
                                 &reported_credential_checksum_support,
                                 sizeof(reported_credential_checksum_support));

    TEST_ASSERT_EQUAL_MESSAGE(credential_checksum_support,
                              reported_credential_checksum_support,
                              "Incorrect reported credential checksum support");

    for (uint8_t i = 0; i < credential_type.size(); i++) {
      printf("Testing credential batch %d\n", i);

      attribute_store_node_t type_node = attribute_store_get_node_child_by_type(
        endpoint_id_node,
        ATTRIBUTE(SUPPORTED_CREDENTIAL_TYPE),
        i);
      TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                    type_node,
                                    "Credential type node should be defined");
      user_credential_type_t reported_credential;

      attribute_store_get_reported(type_node,
                                   &reported_credential,
                                   sizeof(reported_credential));

      TEST_ASSERT_EQUAL_MESSAGE(credential_type[i],
                                reported_credential,
                                "Incorrect credential type");

      std::map<attribute_store_type_t, uint8_t> uint8_attribute_map = {
        {ATTRIBUTE(CREDENTIAL_LEARN_READ_BACK_SUPPORT), cl_support[i]},
      };

      std::map<attribute_store_type_t, uint16_t> uint16_attribute_map = {
        {ATTRIBUTE(CREDENTIAL_SUPPORTED_SLOT_COUNT),
         supported_credential_slots[i]},
        {ATTRIBUTE(CREDENTIAL_MIN_LENGTH), min_length[i]},
        {ATTRIBUTE(CREDENTIAL_MAX_LENGTH), max_length[i]},
      };
      helper_test_attribute_store_values(uint8_attribute_map, type_node);
      helper_test_attribute_store_values(uint16_attribute_map, type_node);
    }
  };

  printf("Test with first set of data\n");
  auto report_frame = create_report_frame(credential_checksum_support,
                                          credential_type,
                                          cl_support,
                                          supported_credential_slots,
                                          min_length,
                                          max_length);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    handler.control_handler(&info, report_frame.data(), report_frame.size()));

  test_report_values();

  printf("Test with second set of data\n");
  credential_checksum_support = 0;
  credential_type             = {6, 7, 8};
  cl_support                  = {0, 1, 1};
  supported_credential_slots  = {15, 1565, 153};
  min_length                  = {155, 15, 5};
  max_length                  = {1111, 111, 11};
  report_frame                = create_report_frame(credential_checksum_support,
                                     credential_type,
                                     cl_support,
                                     supported_credential_slots,
                                     min_length,
                                     max_length);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    handler.control_handler(&info, report_frame.data(), report_frame.size()));

  test_report_values();
}

////////////////////////////////////////////////////////////////////////////
// All User Checksum Get/Report
////////////////////////////////////////////////////////////////////////////
void test_user_credential_all_users_checksum_get_happy_case()
{
  helper_test_get_no_args_happy_case(ALL_USERS_CHECKSUM_GET);
}

void test_user_credential_all_users_checksum_report_happy_case()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  auto create_report_frame
    = [&](user_credential_all_users_checksum_t checksum) {
        std::vector<uint8_t> report_frame
          = {COMMAND_CLASS_USER_CREDENTIAL, ALL_USERS_CHECKSUM_REPORT};

        auto exploded_checksum = explode_uint16(checksum);

        report_frame.push_back(exploded_checksum.msb);
        report_frame.push_back(exploded_checksum.lsb);

        return report_frame;
      };

  user_credential_all_users_checksum_t expected_checksum = 0xABCD;

  auto report_frame = create_report_frame(expected_checksum);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    handler.control_handler(&info, report_frame.data(), report_frame.size()));

  std::map<attribute_store_type_t, uint16_t> uint16_attribute_map
    = {{ATTRIBUTE(ALL_USERS_CHECKSUM), expected_checksum}};
  helper_test_attribute_store_values(uint16_attribute_map);
}

////////////////////////////////////////////////////////////////////////////
// User Set/Get/Report
////////////////////////////////////////////////////////////////////////////
void test_user_credential_user_set_add_or_modify_happy_case()
{
  user_credential_user_unique_id_t user_id                    = 12121;
  user_credential_operation_type_t operation_type             = USER_SET_OPERATION_TYPE_ADD;
  user_credential_type_t user_type                            = 5;
  user_credential_rule_t credential_rule                      = 2;
  user_credential_user_active_state_t user_active_state       = 1;
  user_credential_expiring_timeout_minutes_t expiring_timeout = 55;
  user_credential_user_name_encoding_t user_name_encoding     = 0;
  std::string user_name                                       = "DoUzE";

  auto user_id_node = attribute_store_emplace(endpoint_id_node,
                                              ATTRIBUTE(USER_UNIQUE_ID),
                                              &user_id,
                                              sizeof(user_id));

  auto operation_type_node
    = attribute_store_emplace_desired(user_id_node,
                                      ATTRIBUTE(USER_OPERATION_TYPE),
                                      &operation_type,
                                      sizeof(operation_type));
  auto user_type_node = attribute_store_emplace_desired(user_id_node,
                                                        ATTRIBUTE(USER_TYPE),
                                                        &user_type,
                                                        sizeof(user_type));
  auto user_active_state_node
    = attribute_store_emplace(user_id_node,
                              ATTRIBUTE(USER_ACTIVE_STATE),
                              &user_active_state,
                              sizeof(user_active_state));
  auto credential_rule_node
    = attribute_store_emplace_desired(user_id_node,
                                      ATTRIBUTE(CREDENTIAL_RULE),
                                      &credential_rule,
                                      sizeof(credential_rule));
  auto expiring_timeout_node
    = attribute_store_emplace(user_id_node,
                              ATTRIBUTE(USER_EXPIRING_TIMEOUT_MINUTES),
                              &expiring_timeout,
                              sizeof(expiring_timeout));
  auto user_name_encoding_node
    = attribute_store_emplace_desired(user_id_node,
                                      ATTRIBUTE(USER_NAME_ENCODING),
                                      &user_name_encoding,
                                      sizeof(user_name_encoding));
  auto user_name_node
    = attribute_store_add_node(ATTRIBUTE(USER_NAME), user_id_node);

  sl_status_t status
    = attribute_store_set_reported_string(user_name_node, user_name.c_str());
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK, status, "Can't set username");

  printf("Send with USER_SET_OPERATION_TYPE_MODIFY\n");
  helper_test_set_get_with_args(
    USER_SET,
    operation_type_node,
    {{operation_type_node, DESIRED_ATTRIBUTE},
     {user_id_node, REPORTED_ATTRIBUTE},
     {user_type_node, DESIRED_ATTRIBUTE},
     {user_active_state_node, REPORTED_ATTRIBUTE},
     {credential_rule_node, DESIRED_ATTRIBUTE},
     {expiring_timeout_node, REPORTED_ATTRIBUTE},
     {user_name_encoding_node, DESIRED_ATTRIBUTE},
     {user_name_node, REPORTED_ATTRIBUTE}});


  printf("Send with USER_SET_OPERATION_TYPE_MODIFY\n");

  operation_type = USER_SET_OPERATION_TYPE_ADD;
  attribute_store_set_desired(operation_type_node,
                              &operation_type,
                              sizeof(operation_type));
  helper_test_set_get_with_args(
    USER_SET,
    operation_type_node,
    {{operation_type_node, DESIRED_ATTRIBUTE},
     {user_id_node, REPORTED_ATTRIBUTE},
     {user_type_node, DESIRED_ATTRIBUTE},
     {user_active_state_node, REPORTED_ATTRIBUTE},
     {credential_rule_node, DESIRED_ATTRIBUTE},
     {expiring_timeout_node, REPORTED_ATTRIBUTE},
     {user_name_encoding_node, DESIRED_ATTRIBUTE},
     {user_name_node, REPORTED_ATTRIBUTE}});            
}

void test_user_credential_user_delete_remove_happy_case()
{
  user_credential_user_unique_id_t user_id                    = 12121;
  user_credential_operation_type_t operation_type             = USER_SET_OPERATION_TYPE_DELETE;

  auto user_id_node = attribute_store_emplace(endpoint_id_node,
                                              ATTRIBUTE(USER_UNIQUE_ID),
                                              &user_id,
                                              sizeof(user_id));

  auto operation_type_node
    = attribute_store_emplace_desired(user_id_node,
                                      ATTRIBUTE(USER_OPERATION_TYPE),
                                      &operation_type,
                                      sizeof(operation_type));

  helper_test_set_get_with_args(USER_SET,
                                operation_type_node,
                                {{operation_type_node, DESIRED_ATTRIBUTE},
                                 {user_id_node, REPORTED_ATTRIBUTE}});
}

void test_user_credential_user_set_invalid_node()
{
  helper_test_set_get_with_args(USER_SET, ATTRIBUTE_STORE_INVALID_NODE);
}


void test_user_credential_user_get_happy_case()
{
  user_credential_user_unique_id_t user_id = 12;
  auto user_node = attribute_store_emplace_desired(endpoint_id_node,
                                                   ATTRIBUTE(USER_UNIQUE_ID),
                                                   &user_id,
                                                   sizeof(user_id));

  helper_test_set_get_with_args(USER_GET,
                                user_node,
                                {{user_node, DESIRED_ATTRIBUTE}});
}

void test_user_credential_user_get_not_found()
{
  helper_test_set_get_with_args(USER_GET, ATTRIBUTE_STORE_INVALID_NODE);
}

std::vector<uint8_t> helper_create_user_report_frame(
  user_credential_user_unique_id_t next_user_id,
  user_credential_user_modifier_type_t user_modifier_type,
  user_credential_user_modifier_node_id_t user_modifier_node_id,
  user_credential_user_unique_id_t user_id,
  user_credential_user_type_t user_type,
  user_credential_user_active_state_t user_active_state,
  user_credential_supported_credential_rules_t credential_rule,
  user_credential_expiring_timeout_minutes_t expiring_timeout_minutes,
  user_credential_user_name_encoding_t user_name_encoding,
  std::string user_name)
{
  std::vector<uint8_t> report_frame
    = {COMMAND_CLASS_USER_CREDENTIAL, USER_REPORT};

  auto exploded_next_user_id = explode_uint16(next_user_id);
  report_frame.push_back(exploded_next_user_id.msb);
  report_frame.push_back(exploded_next_user_id.lsb);

  report_frame.push_back(user_modifier_type);

  auto exploded_user_modifier_node_id = explode_uint16(user_modifier_node_id);
  report_frame.push_back(exploded_user_modifier_node_id.msb);
  report_frame.push_back(exploded_user_modifier_node_id.lsb);

  auto exploded_user_id = explode_uint16(user_id);
  report_frame.push_back(exploded_user_id.msb);
  report_frame.push_back(exploded_user_id.lsb);

  report_frame.push_back(user_type);
  report_frame.push_back(user_active_state);
  report_frame.push_back(credential_rule);

  auto exploded_time = explode_uint16(expiring_timeout_minutes);
  report_frame.push_back(exploded_time.msb);
  report_frame.push_back(exploded_time.lsb);

  report_frame.push_back(user_name_encoding);
  report_frame.push_back(user_name.size());
  for (char c: user_name) {
    report_frame.push_back(c);
  }
  return report_frame;
};

void test_user_credential_user_report_happy_case()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  auto user_get = resolver_functions[USER_GET];
  TEST_ASSERT_NOT_NULL_MESSAGE(user_get, "User get function should be defined");

  constexpr user_credential_user_unique_id_t EXPECTED_FIRST_USER_ID  = 12;
  constexpr user_credential_user_unique_id_t EXPECTED_SECOND_USER_ID = 1212;

  user_credential_user_unique_id_t next_user_id = EXPECTED_SECOND_USER_ID;
  user_credential_user_modifier_type_t user_modifier_type       = 2;
  user_credential_user_modifier_node_id_t user_modifier_node_id = 1313;
  user_credential_user_unique_id_t user_id = EXPECTED_FIRST_USER_ID;
  user_credential_user_type_t user_type    = 3;
  user_credential_user_active_state_t user_active_state               = 1;
  user_credential_supported_credential_rules_t credential_rule        = 2;
  user_credential_expiring_timeout_minutes_t expiring_timeout_minutes = 1515;
  user_credential_user_name_encoding_t user_name_encoding             = 0;
  std::string user_name                                               = "DoUzE";

  auto test_user_values = [&](attribute_store_node_t user_node) {
    // Check main node value
    user_credential_user_unique_id_t reported_id;
    attribute_store_get_reported(user_node, &reported_id, sizeof(reported_id));
    TEST_ASSERT_EQUAL_MESSAGE(user_id,
                              reported_id,
                              "User Unique ID is incorrect");
    // Then test values underneath
    std::map<attribute_store_type_t, uint8_t> uint8_attribute_map
      = {{ATTRIBUTE(USER_MODIFIER_TYPE), user_modifier_type},
         {ATTRIBUTE(USER_TYPE), user_type},
         {ATTRIBUTE(USER_ACTIVE_STATE), user_active_state},
         {ATTRIBUTE(CREDENTIAL_RULE), credential_rule}};
    helper_test_attribute_store_values(uint8_attribute_map, user_node);

    std::map<attribute_store_type_t, uint16_t> uint16_attribute_map
      = {{ATTRIBUTE(USER_MODIFIER_NODE_ID), user_modifier_node_id},
         {ATTRIBUTE(USER_EXPIRING_TIMEOUT_MINUTES), expiring_timeout_minutes}};
    helper_test_attribute_store_values(uint16_attribute_map, user_node);

    helper_test_string_value({{ATTRIBUTE(USER_NAME), user_name}}, user_node);

    // Check user credential desired values
    user_credential_type_t credential_type = 1;
    user_credential_slot_t credential_slot = 12;
    auto credential_type_node
      = attribute_store_get_node_child_by_type(user_node,
                                               ATTRIBUTE(CREDENTIAL_TYPE),
                                               0);
    attribute_store_get_reported(credential_type_node,
                                 &credential_type,
                                 sizeof(credential_type));
    TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                  credential_type_node,
                                  "credential_type_node should exist");

    auto credential_slot_node
      = attribute_store_get_node_child_by_type(credential_type_node,
                                               ATTRIBUTE(CREDENTIAL_SLOT),
                                               0);
    attribute_store_get_desired(credential_slot_node,
                                &credential_slot,
                                sizeof(credential_slot));

    TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                  credential_slot_node,
                                  "credential_slot_node should exist");

    TEST_ASSERT_EQUAL_MESSAGE(
      0,
      credential_type,
      "Credential Type should be at 0 and should exists");
    TEST_ASSERT_EQUAL_MESSAGE(
      0,
      credential_slot,
      "Credential Slot should be at 0 and should exists");
  };  // end test_user_values lambda

  auto report_frame = helper_create_user_report_frame(next_user_id,
                                                      user_modifier_type,
                                                      user_modifier_node_id,
                                                      user_id,
                                                      user_type,
                                                      user_active_state,
                                                      credential_rule,
                                                      expiring_timeout_minutes,
                                                      user_name_encoding,
                                                      user_name);

  printf("First user creation\n");

  // Create first user
  auto first_user_id_node
    = attribute_store_add_node(ATTRIBUTE(USER_UNIQUE_ID), endpoint_id_node);
  // First user should have ID 0 and then set in the report
  user_credential_user_unique_id_t first_id = 0;
  attribute_store_set_desired(first_user_id_node, &first_id, sizeof(first_id));

  // Simulate get on that user
  sl_status_t user_get_status
    = user_get(first_user_id_node, received_frame, &received_frame_size);

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            user_get_status,
                            "User get should have returned SL_STATUS_OK");

  // Call report
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    handler.control_handler(&info, report_frame.data(), report_frame.size()),
    "User Report should have returned SL_STATUS_OK");
  // Test values
  test_user_values(first_user_id_node);

  // Test structure
  auto user_id_count
    = attribute_store_get_node_child_count_by_type(endpoint_id_node,
                                                   ATTRIBUTE(USER_UNIQUE_ID));
  TEST_ASSERT_EQUAL_MESSAGE(2,
                            user_id_count,
                            "User node count mismatch. Should be 2 : 1 for "
                            "the reported one and 1 for the next one");
  auto second_user_id_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(USER_UNIQUE_ID),
                                             1);

  printf("Second and last user creation\n");

  // Simulate a node with desired value of 0 to see if it is properly removed
  // to prevent infinite loop
  user_credential_user_unique_id_t id_zero = 0;
  attribute_store_emplace_desired(endpoint_id_node,
                                  ATTRIBUTE(USER_UNIQUE_ID),
                                  &id_zero,
                                  sizeof(id_zero));

  // Check second & not defined user id
  user_credential_user_unique_id_t second_user_id;
  attribute_store_get_desired(second_user_id_node,
                              &second_user_id,
                              sizeof(second_user_id));
  TEST_ASSERT_EQUAL_MESSAGE(
    next_user_id,
    second_user_id,
    "Second user id should have it's desired value defined");

  // Simulate a GET on that user
  // Removes the desired state and sets to reported instead
  user_get_status
    = user_get(second_user_id_node, received_frame, &received_frame_size);

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            user_get_status,
                            "User get should have returned SL_STATUS_OK");

  // Second and last user
  next_user_id          = 0;
  user_modifier_type    = 3;
  user_modifier_node_id = 1414;
  user_id               = EXPECTED_SECOND_USER_ID;
  user_type             = 5;
  user_active_state     = 0;
  credential_rule       = 1;
  user_name_encoding    = 1;
  user_name             = "NoDoUzE4YoU";

  report_frame = helper_create_user_report_frame(next_user_id,
                                                 user_modifier_type,
                                                 user_modifier_node_id,
                                                 user_id,
                                                 user_type,
                                                 user_active_state,
                                                 credential_rule,
                                                 expiring_timeout_minutes,
                                                 user_name_encoding,
                                                 user_name);
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    handler.control_handler(&info, report_frame.data(), report_frame.size()),
    "Second User Report should have returned SL_STATUS_OK");
  // Test values
  test_user_values(second_user_id_node);

  // Test structure

  // The report function should not have created an other user unique id node (next_user_id = 0)
  user_id_count
    = attribute_store_get_node_child_count_by_type(endpoint_id_node,
                                                   ATTRIBUTE(USER_UNIQUE_ID));
  TEST_ASSERT_EQUAL_MESSAGE(2,
                            user_id_count,
                            "User node count mismatch. Should be 2 : 1 for "
                            "the reported one and 1 for the next one");

  user_credential_user_unique_id_t reported_id;
  attribute_store_get_reported(first_user_id_node,
                               &reported_id,
                               sizeof(reported_id));
  TEST_ASSERT_EQUAL_MESSAGE(EXPECTED_FIRST_USER_ID,
                            reported_id,
                            "First user id mismatch");
  attribute_store_get_reported(second_user_id_node,
                               &reported_id,
                               sizeof(reported_id));
  TEST_ASSERT_EQUAL_MESSAGE(EXPECTED_SECOND_USER_ID,
                            reported_id,
                            "Second user id mismatch");
}

void test_user_credential_user_report_user_not_found()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;
  auto report_frame
    = helper_create_user_report_frame(0, 0, 0, 12, 0, 0, 0, 0, 0, "^^");

  // Not found since we haven't created any user_unique_id
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_NOT_SUPPORTED,
    handler.control_handler(&info, report_frame.data(), report_frame.size()),
    "User Report should have returned SL_STATUS_NOT_SUPPORTED");

  // Check that no user_unique_id node has been created
  auto user_id_count
    = attribute_store_get_node_child_count_by_type(endpoint_id_node,
                                                   ATTRIBUTE(USER_UNIQUE_ID));
  TEST_ASSERT_EQUAL_MESSAGE(0,
                            user_id_count,
                            "No user_unique_id node should be created");
}

void test_user_credential_user_report_user_with_id0()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;
  auto report_frame
    = helper_create_user_report_frame(0, 0, 0, 0, 0, 0, 0, 0, 0, "DOUZE");

  // Create user id with reported ID of 0 to simulate user get of id 0
  user_credential_user_unique_id_t user_id = 0;
  attribute_store_emplace_desired(endpoint_id_node,
                                  ATTRIBUTE(USER_UNIQUE_ID),
                                  &user_id,
                                  sizeof(user_id));

  // Not found since we haven't created any user_unique_id
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    handler.control_handler(&info, report_frame.data(), report_frame.size()),
    "User Report should have returned SL_STATUS_OK and ignore the frame");

  // Check that no user_unique_id node has been created (and the invalid node was deleted)
  auto user_id_count
    = attribute_store_get_node_child_count_by_type(endpoint_id_node,
                                                   ATTRIBUTE(USER_UNIQUE_ID));
  TEST_ASSERT_EQUAL_MESSAGE(0,
                            user_id_count,
                            "No user_unique_id node should be created");
}

void test_user_credential_user_report_user_deleted()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  // Create user id with reported ID of 0 to simulate user get of id 0
  user_credential_user_unique_id_t user_id = 12;
  auto user_id_node = attribute_store_emplace_desired(endpoint_id_node,
                                                      ATTRIBUTE(USER_UNIQUE_ID),
                                                      &user_id,
                                                      sizeof(user_id));

  TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                user_id_node,
                                "User ID node should be created");

  auto user_id_count
    = attribute_store_get_node_child_count_by_type(endpoint_id_node,
                                                   ATTRIBUTE(USER_UNIQUE_ID));
  TEST_ASSERT_EQUAL_MESSAGE(1, user_id_count, "Should have one user by now");

  auto report_frame = helper_create_user_report_frame(user_id,
                                                      USER_REPORT_DNE,
                                                      0,
                                                      0,
                                                      0,
                                                      0,
                                                      0,
                                                      0,
                                                      0,
                                                      "????");

  // Should delete this user
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    handler.control_handler(&info, report_frame.data(), report_frame.size()),
    "User Report should have returned SL_STATUS_NOT_SUPPORTED");

  user_id_count
    = attribute_store_get_node_child_count_by_type(endpoint_id_node,
                                                   ATTRIBUTE(USER_UNIQUE_ID));
  TEST_ASSERT_EQUAL_MESSAGE(1,
                            user_id_count,
                            "Should not have any user by now");
}

////////////////////////////////////////////////////////////////////////////
// Credential Set/Get/Report
////////////////////////////////////////////////////////////////////////////

//>> Set
void test_user_credential_credential_set_1byte_happy_case()
{
  user_credential_type_t credential_type          = 2;
  user_credential_slot_t credential_slot          = 1212;
  user_credential_operation_type_t operation_type = USER_CREDENTIAL_OPERATION_TYPE_ADD;
  std::vector<uint8_t> credential_data            = {12};

  // Create the node with reported attribute
  auto nodes                = helper_create_credential_structure(12,
                                                  credential_type,
                                                  credential_slot,
                                                  REPORTED_ATTRIBUTE);
  auto user_node            = nodes.user_id_node;
  auto credential_type_node = nodes.credential_type_node;
  auto credential_slot_node = nodes.credential_slot_node;

  // Operation type
  auto operation_type_node
    = attribute_store_emplace_desired(credential_slot_node,
                              ATTRIBUTE(CREDENTIAL_OPERATION_TYPE),
                              &operation_type,
                              sizeof(operation_type));
  // CREDENTIAL_DATA
  uint8_t credential_data_length = credential_data.size();
  auto credential_data_length_node
    = attribute_store_emplace_desired(credential_slot_node,
                                      ATTRIBUTE(CREDENTIAL_DATA_LENGTH),
                                      &credential_data_length,
                                      sizeof(uint8_t));

  auto credential_data_node
    = attribute_store_emplace_desired(credential_data_length_node,
                                      ATTRIBUTE(CREDENTIAL_DATA),
                                      credential_data.data(),
                                      credential_data.size());

  helper_test_set_get_with_args(CREDENTIAL_SET,
                                operation_type_node,
                                {
                                  {user_node, REPORTED_ATTRIBUTE},
                                  {credential_type_node, REPORTED_ATTRIBUTE},
                                  {credential_slot_node, REPORTED_ATTRIBUTE},
                                  {operation_type_node, DESIRED_ATTRIBUTE},
                                  {credential_data_node, DESIRED_ATTRIBUTE},
                                });
}

void test_user_credential_credential_set_12byte_happy_case()
{
  user_credential_type_t credential_type          = 2;
  user_credential_slot_t credential_slot          = 1212;
  user_credential_operation_type_t operation_type = USER_CREDENTIAL_OPERATION_TYPE_MODIFY;
  std::vector<uint8_t> credential_data
    = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

  // Create the node with reported attribute
  auto nodes                = helper_create_credential_structure(12,
                                                  credential_type,
                                                  credential_slot,
                                                  REPORTED_ATTRIBUTE);
  auto user_node            = nodes.user_id_node;
  auto credential_type_node = nodes.credential_type_node;
  auto credential_slot_node = nodes.credential_slot_node;

  // Operation type
  auto operation_type_node
    = attribute_store_emplace_desired(credential_slot_node,
                              ATTRIBUTE(CREDENTIAL_OPERATION_TYPE),
                              &operation_type,
                              sizeof(operation_type));
  // CREDENTIAL_DATA
  uint8_t credential_data_length = credential_data.size();
  auto credential_data_length_node
    = attribute_store_emplace(credential_slot_node,
                              ATTRIBUTE(CREDENTIAL_DATA_LENGTH),
                              &credential_data_length,
                              sizeof(uint8_t));

  auto credential_data_node
    = attribute_store_emplace(credential_data_length_node,
                              ATTRIBUTE(CREDENTIAL_DATA),
                              credential_data.data(),
                              credential_data.size());

  helper_test_set_get_with_args(CREDENTIAL_SET,
                                operation_type_node,
                                {
                                  {user_node, REPORTED_ATTRIBUTE},
                                  {credential_type_node, REPORTED_ATTRIBUTE},
                                  {credential_slot_node, REPORTED_ATTRIBUTE},
                                  {operation_type_node, DESIRED_ATTRIBUTE},
                                  {credential_data_node, REPORTED_ATTRIBUTE},
                                });
}

void test_user_credential_credential_set_invalid_node()
{
  helper_test_set_get_with_args(CREDENTIAL_SET, ATTRIBUTE_STORE_INVALID_NODE);
}

void test_user_credential_credential_set_delete_happy_case()
{
  user_credential_type_t credential_type          = 2;
  user_credential_slot_t credential_slot          = 1212;
  user_credential_operation_type_t operation_type = USER_CREDENTIAL_OPERATION_TYPE_DELETE;

  // Create the node with reported attribute
  auto nodes                = helper_create_credential_structure(12,
                                                  credential_type,
                                                  credential_slot,
                                                  REPORTED_ATTRIBUTE);
  auto user_node            = nodes.user_id_node;
  auto credential_type_node = nodes.credential_type_node;
  auto credential_slot_node = nodes.credential_slot_node;

  // Operation type
  auto operation_type_node
    = attribute_store_emplace_desired(credential_slot_node,
                              ATTRIBUTE(CREDENTIAL_OPERATION_TYPE),
                              &operation_type,
                              sizeof(operation_type));

  helper_test_set_get_with_args(CREDENTIAL_SET,
                                operation_type_node,
                                {
                                  {user_node, REPORTED_ATTRIBUTE},
                                  {credential_type_node, REPORTED_ATTRIBUTE},
                                  {credential_slot_node, REPORTED_ATTRIBUTE},
                                  {operation_type_node, DESIRED_ATTRIBUTE},
                                },
                                {0x00} // Credential data length
                                );
}


//>> Get

void test_user_credential_credential_get_happy_case()
{
  user_credential_type_t credential_type = 2;
  user_credential_slot_t credential_slot = 1212;

  auto nodes
    = helper_create_credential_structure(12, credential_type, credential_slot);
  auto user_node            = nodes.user_id_node;
  auto credential_type_node = nodes.credential_type_node;
  auto credential_slot_node = nodes.credential_slot_node;

  // Get should make both credential_type_node and credential_slot_node reported value as the desired values
  helper_test_set_get_with_args(CREDENTIAL_GET,
                                credential_slot_node,
                                {
                                  {user_node, REPORTED_ATTRIBUTE},
                                  {credential_type_node, REPORTED_ATTRIBUTE},
                                  {credential_slot_node, DESIRED_ATTRIBUTE},
                                });
}

void test_user_credential_credential_get_no_credential_type()
{
  helper_test_set_get_with_args(CREDENTIAL_GET, ATTRIBUTE_STORE_INVALID_NODE);
}

std::vector<uint8_t> helper_create_credential_report_frame(
  user_credential_user_unique_id_t user_id,
  user_credential_type_t credential_type,
  user_credential_slot_t credential_slot,
  uint8_t crb,
  std::vector<uint8_t> credential_data,
  user_credential_user_modifier_type_t credential_modifier_type,
  user_credential_user_modifier_node_id_t credential_modifier_node_id,
  user_credential_type_t next_credential_type,
  user_credential_slot_t next_credential_slot)
{
  std::vector<uint8_t> report_frame
    = {COMMAND_CLASS_USER_CREDENTIAL, CREDENTIAL_REPORT};

  auto exploded_user_id = explode_uint16(user_id);
  report_frame.push_back(exploded_user_id.msb);
  report_frame.push_back(exploded_user_id.lsb);

  report_frame.push_back(credential_type);

  auto exploded_credential_slot = explode_uint16(credential_slot);
  report_frame.push_back(exploded_credential_slot.msb);
  report_frame.push_back(exploded_credential_slot.lsb);

  report_frame.push_back(crb << 7);
  report_frame.push_back(credential_data.size());

  for (uint8_t credential_value: credential_data) {
    report_frame.push_back(credential_value);
  }

  report_frame.push_back(credential_modifier_type);
  auto exploded_credential_modifier_node_id
    = explode_uint16(credential_modifier_node_id);
  report_frame.push_back(exploded_credential_modifier_node_id.msb);
  report_frame.push_back(exploded_credential_modifier_node_id.lsb);

  report_frame.push_back(next_credential_type);
  auto exploded_next_credential_slot = explode_uint16(next_credential_slot);
  report_frame.push_back(exploded_next_credential_slot.msb);
  report_frame.push_back(exploded_next_credential_slot.lsb);

  return report_frame;
};

void test_user_credential_credential_report_no_credential()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  user_credential_user_unique_id_t user_id = 12;
  auto user_id_node = attribute_store_emplace(endpoint_id_node,
                                              ATTRIBUTE(USER_UNIQUE_ID),
                                              &user_id,
                                              sizeof(user_id));
  TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                user_id_node,
                                "User ID node should be defined");

  auto report_frame
    = helper_create_credential_report_frame(user_id, 0, 0, 0, {}, 0, 0, 0, 0);

  // SHould return ok since credential_type and credential node is 0
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    handler.control_handler(&info, report_frame.data(), report_frame.size()),
    "Credential Report should have returned SL_STATUS_NOT_SUPPORTED");
}

void test_user_credential_credential_report_missing_user()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  auto report_frame
    = helper_create_credential_report_frame(12, 1, 1, 0, {}, 0, 0, 0, 0);

  // Not found since we haven't created any user_unique_id
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_NOT_SUPPORTED,
    handler.control_handler(&info, report_frame.data(), report_frame.size()),
    "Credential Report should have returned SL_STATUS_NOT_SUPPORTED");
}

void test_user_credential_credential_report_missing_credential_type()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  user_credential_user_unique_id_t user_id = 12;
  auto user_id_node = attribute_store_emplace(endpoint_id_node,
                                              ATTRIBUTE(USER_UNIQUE_ID),
                                              &user_id,
                                              sizeof(user_id));

  TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                user_id_node,
                                "User ID node should be defined");

  auto report_frame
    = helper_create_credential_report_frame(user_id, 1, 5, 0, {}, 0, 0, 0, 0);

  // Not found since we haven't created any credential_type
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_NOT_SUPPORTED,
    handler.control_handler(&info, report_frame.data(), report_frame.size()),
    "Credential Report should have returned SL_STATUS_NOT_SUPPORTED");
}

void test_user_credential_credential_report_missing_credential_slot()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  user_credential_user_unique_id_t user_id = 12;
  auto user_id_node = attribute_store_emplace(endpoint_id_node,
                                              ATTRIBUTE(USER_UNIQUE_ID),
                                              &user_id,
                                              sizeof(user_id));
  TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                user_id_node,
                                "User ID node should be defined");
  user_credential_type_t credential_type = 121;
  auto credential_type_node
    = attribute_store_emplace(user_id_node,
                              ATTRIBUTE(CREDENTIAL_TYPE),
                              &credential_type,
                              sizeof(credential_type));
  TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                credential_type_node,
                                "Credential type node should be defined");

  auto report_frame = helper_create_credential_report_frame(user_id,
                                                            credential_type,
                                                            1212,
                                                            0,
                                                            {},
                                                            0,
                                                            0,
                                                            0,
                                                            0);

  // Not found since we haven't created any credential_slot
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_NOT_SUPPORTED,
    handler.control_handler(&info, report_frame.data(), report_frame.size()),
    "Credential Report should have returned SL_STATUS_NOT_SUPPORTED");
}

void test_user_credential_credential_report_happy_case()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  auto credential_get = resolver_functions[CREDENTIAL_GET];
  TEST_ASSERT_NOT_NULL_MESSAGE(credential_get,
                               "Credential get function should be defined");

  user_credential_user_unique_id_t user_id = 122;
  user_credential_type_t credential_type   = 12;
  user_credential_slot_t credential_slot   = 1212;
  uint8_t crb                              = 1;
  std::vector<uint8_t> credential_data     = {12, 13, 14, 15, 16};
  user_credential_user_modifier_type_t credential_modifier_type       = 13;
  user_credential_user_modifier_node_id_t credential_modifier_node_id = 1312;
  user_credential_type_t next_credential_type                         = 12;
  user_credential_slot_t next_credential_slot                         = 1;

  auto test_user_values = [&](attribute_store_node_t credential_type_node,
                              attribute_store_node_t credential_slot_node) {
    // Check main node value
    user_credential_type_t reported_cred_type;
    attribute_store_get_reported(credential_type_node,
                                 &reported_cred_type,
                                 sizeof(reported_cred_type));
    TEST_ASSERT_EQUAL_MESSAGE(credential_type,
                              reported_cred_type,
                              "Credential type value mismatch");

    user_credential_slot_t reported_cred_slot;
    attribute_store_get_reported(credential_slot_node,
                                 &reported_cred_slot,
                                 sizeof(reported_cred_slot));
    TEST_ASSERT_EQUAL_MESSAGE(credential_slot,
                              reported_cred_slot,
                              "Credential slot value mismatch");

    // Then test values underneath
    std::map<attribute_store_type_t, uint8_t> uint8_attribute_map
      = {{ATTRIBUTE(CREDENTIAL_MODIFIER_TYPE), credential_modifier_type},
         {ATTRIBUTE(CREDENTIAL_READ_BACK), crb}};
    helper_test_attribute_store_values(uint8_attribute_map,
                                       credential_slot_node);

    std::map<attribute_store_type_t, uint16_t> uint16_attribute_map
      = {{ATTRIBUTE(CREDENTIAL_MODIFIER_NODE_ID), credential_modifier_node_id}};
    helper_test_attribute_store_values(uint16_attribute_map,
                                       credential_slot_node);

    auto credential_length_node = attribute_store_get_first_child_by_type(
      credential_slot_node,
      ATTRIBUTE(CREDENTIAL_DATA_LENGTH));
    uint8_t reported_credential_length;
    attribute_store_get_reported(credential_length_node,
                                 &reported_credential_length,
                                 sizeof(reported_credential_length));
    TEST_ASSERT_EQUAL_MESSAGE(credential_data.size(),
                              reported_credential_length,
                              "Credential data length value mismatch");

    std::vector<uint8_t> reported_credential_data;
    reported_credential_data.resize(reported_credential_length);

    attribute_store_get_child_reported(credential_length_node,
                                       ATTRIBUTE(CREDENTIAL_DATA),
                                       reported_credential_data.data(),
                                       reported_credential_length);

    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(credential_data.data(),
                                          reported_credential_data.data(),
                                          credential_data.size(),
                                          "Credential data mismatch");
  };  // end test_user_values lambda

  auto check_credentials_node_count =
    [](attribute_store_node_t user_id_node,
       size_t expected_credential_type_count,
       size_t expected_credential_slot_count) {
      auto count_credential_type = attribute_store_get_node_child_count_by_type(
        user_id_node,
        ATTRIBUTE(CREDENTIAL_TYPE));
      TEST_ASSERT_EQUAL_MESSAGE(expected_credential_type_count,
                                count_credential_type,
                                "Incorrect credential type count");

      size_t count_credential_slot = 0;
      for (size_t i = 0; i < count_credential_type; i++) {
        auto credential_type_node
          = attribute_store_get_node_child_by_type(user_id_node,
                                                   ATTRIBUTE(CREDENTIAL_TYPE),
                                                   i);
        count_credential_slot += attribute_store_get_node_child_count_by_type(
          credential_type_node,
          ATTRIBUTE(CREDENTIAL_SLOT));
      }

      TEST_ASSERT_EQUAL_MESSAGE(expected_credential_slot_count,
                                count_credential_slot,
                                "Incorrect credential slot count");
    };

  auto check_next_credential_node_value
    = [&](attribute_store_node_t next_credential_type_node,
          attribute_store_node_t next_credential_slot_node) {
        // First check node existence
        TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                      next_credential_type_node,
                                      "next_credential_type_node should exist");
        TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                      next_credential_slot_node,
                                      "next_credential_slot_node should exist");

        user_credential_type_t reported_next_credential_type = 0;
        user_credential_slot_t reported_next_credential_slot = 0;

        attribute_store_read_value(next_credential_type_node,
                                   REPORTED_ATTRIBUTE,
                                   &reported_next_credential_type,
                                   sizeof(reported_next_credential_type));

        attribute_store_read_value(next_credential_slot_node,
                                   DESIRED_ATTRIBUTE,
                                   &reported_next_credential_slot,
                                   sizeof(reported_next_credential_slot));

        TEST_ASSERT_EQUAL_MESSAGE(next_credential_type,
                                  reported_next_credential_type,
                                  "Next Credential Type value mismatch");
        TEST_ASSERT_EQUAL_MESSAGE(next_credential_slot,
                                  reported_next_credential_slot,
                                  "Next Credential Slot value mismatch");
      };

  //>>>> First credential
  printf("First credential creation\n");

  auto report_frame
    = helper_create_credential_report_frame(user_id,
                                            credential_type,
                                            credential_slot,
                                            crb,
                                            credential_data,
                                            credential_modifier_type,
                                            credential_modifier_node_id,
                                            next_credential_type,
                                            next_credential_slot);

  // Create first credential
  auto nodes
    = helper_create_credential_structure(user_id, 0, 0, DESIRED_ATTRIBUTE);

  auto user_id_node               = nodes.user_id_node;
  auto first_credential_type_node = nodes.credential_type_node;
  auto first_credential_slot_node = nodes.credential_slot_node;

  attribute_store_log();

  // Simulate get on that credential
  sl_status_t credential_get_status = credential_get(first_credential_slot_node,
                                                     received_frame,
                                                     &received_frame_size);

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    credential_get_status,
    "First Credential get should have returned SL_STATUS_OK");

  // Call report
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    handler.control_handler(&info, report_frame.data(), report_frame.size()),
    "First Credential Report should have returned SL_STATUS_OK");
  // Test values
  test_user_values(first_credential_type_node, first_credential_slot_node);
  // We should have 1 credential type and 2 credential slot
  check_credentials_node_count(user_id_node, 1, 2);

  // Test if next credentials are well defined
  auto second_credential_slot_node
    = attribute_store_get_node_child_by_type(first_credential_type_node,
                                             ATTRIBUTE(CREDENTIAL_SLOT),
                                             1);
  check_next_credential_node_value(first_credential_type_node,
                                   second_credential_slot_node);

  //>>>> Second credential
  printf("Second credential creation\n");

  credential_type             = next_credential_type;
  credential_slot             = next_credential_slot;
  crb                         = 0;
  credential_data             = {1, 2};
  credential_modifier_type    = 3;
  credential_modifier_node_id = 13121;
  next_credential_type        = 121;
  next_credential_slot        = 3;

  report_frame
    = helper_create_credential_report_frame(user_id,
                                            credential_type,
                                            credential_slot,
                                            crb,
                                            credential_data,
                                            credential_modifier_type,
                                            credential_modifier_node_id,
                                            next_credential_type,
                                            next_credential_slot);

  // Simulate get on that credential
  credential_get_status = credential_get(second_credential_slot_node,
                                         received_frame,
                                         &received_frame_size);

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            credential_get_status,
                            "Credential get should have returned SL_STATUS_OK");

  // Call report
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    handler.control_handler(&info, report_frame.data(), report_frame.size()),
    "Second Credential Report should have returned SL_STATUS_OK");
  // Test values
  test_user_values(first_credential_type_node, second_credential_slot_node);
  // We should have 2 credential type and 2 credential slot
  check_credentials_node_count(user_id_node, 2, 3);

  auto second_credential_type_node
    = attribute_store_get_node_child_by_type(user_id_node,
                                             ATTRIBUTE(CREDENTIAL_TYPE),
                                             1);
  auto third_credential_slot_node
    = attribute_store_get_node_child_by_type(second_credential_type_node,
                                             ATTRIBUTE(CREDENTIAL_SLOT),
                                             0);

  // New nodes so we check their desired values
  check_next_credential_node_value(second_credential_type_node,
                                   third_credential_slot_node);

  //>>>> Third credential
  printf("Third and last credential creation\n");
  credential_type             = next_credential_type;
  credential_slot             = next_credential_slot;
  crb                         = 1;
  credential_data             = {15};
  credential_modifier_type    = 2;
  credential_modifier_node_id = 12;
  next_credential_type        = 0;
  next_credential_slot        = 0;

  report_frame
    = helper_create_credential_report_frame(user_id,
                                            credential_type,
                                            credential_slot,
                                            crb,
                                            credential_data,
                                            credential_modifier_type,
                                            credential_modifier_node_id,
                                            next_credential_type,
                                            next_credential_slot);

  // Simulate get on that credential
  credential_get_status = credential_get(third_credential_slot_node,
                                         received_frame,
                                         &received_frame_size);

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            credential_get_status,
                            "Credential get should have returned SL_STATUS_OK");

  // Call report
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    handler.control_handler(&info, report_frame.data(), report_frame.size()),
    "Second Credential Report should have returned SL_STATUS_OK");
  // Test values
  test_user_values(second_credential_type_node, third_credential_slot_node);
  // We should still have 2 credential type and 3 credential slots since it was the last credential
  check_credentials_node_count(user_id_node, 2, 3);
}

////////////////////////////////////////////////////////////////////////////
// Attribute creation
////////////////////////////////////////////////////////////////////////////
void test_attribute_creation_happy_case()
{
  zwave_cc_version_t version = 1;
  attribute_store_emplace(endpoint_id_node,
                          ATTRIBUTE(VERSION),
                          &version,
                          sizeof(version));

  std::vector<attribute_store_type_t> created_types = {
    ATTRIBUTE(NUMBER_OF_USERS),
    ATTRIBUTE(SUPPORT_CREDENTIAL_CHECKSUM),
  };

  for (auto &type: created_types) {
    const std::string error_msg
      = "Attribute " + std::string(attribute_store_get_type_name(type))
        + " should exist";

    TEST_ASSERT_NOT_EQUAL_MESSAGE(
      ATTRIBUTE_STORE_INVALID_NODE,
      attribute_store_get_node_child_by_type(endpoint_id_node, type, 0),
      error_msg.c_str());
  }
}

void test_attribute_creation_no_version()
{
  zwave_cc_version_t version = 0;
  attribute_store_emplace(endpoint_id_node,
                          ATTRIBUTE(VERSION),
                          &version,
                          sizeof(version));

  std::vector<attribute_store_type_t> created_types = {
    ATTRIBUTE(NUMBER_OF_USERS),
    ATTRIBUTE(SUPPORT_CREDENTIAL_CHECKSUM),
  };

  for (auto &type: created_types) {
    const std::string error_msg
      = "Attribute " + std::string(attribute_store_get_type_name(type))
        + " should NOT exist";

    TEST_ASSERT_EQUAL_MESSAGE(
      ATTRIBUTE_STORE_INVALID_NODE,
      attribute_store_get_node_child_by_type(endpoint_id_node, type, 0),
      error_msg.c_str());
  }
}

void test_post_interview_discovery()
{
  // Lambdas
  auto count_user_node = [](size_t expected_value) {
    auto user_id_node_count
      = attribute_store_get_node_child_count_by_type(endpoint_id_node,
                                                     ATTRIBUTE(USER_UNIQUE_ID));
    TEST_ASSERT_EQUAL_MESSAGE(expected_value,
                              user_id_node_count,
                              "User ID node count mismatch");
  };

  // Test logic

  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE;
  auto network_status_node
    = attribute_store_emplace(node_id_node,
                              DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                              &network_status,
                              sizeof(network_status));

  count_user_node(0);

  // Interview state
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  count_user_node(0);

  // Simulate one more endpoint and only add nif to one of them

  // Add NIF to our endpoint to tell that we support User Credential
  std::vector<uint8_t> nif_value = {COMMAND_CLASS_USER_CREDENTIAL};
  attribute_store_emplace(endpoint_id_node,
                          ATTRIBUTE_ZWAVE_SECURE_NIF,
                          nif_value.data(),
                          nif_value.size());

  // Add another endpoint without User Credential support
  zwave_endpoint_id_t endpoint_id_2 = endpoint_id + 12;
  attribute_store_emplace(node_id_node,
                          ATTRIBUTE_ENDPOINT_ID,
                          &endpoint_id_2,
                          sizeof(endpoint_id_2));

  // Display current store log for debug purpose
  attribute_store_log();

  // Set two random user id to see if discovery remove it
  user_credential_user_unique_id_t user_id = 12;
  attribute_store_emplace(endpoint_id_node,
                          ATTRIBUTE(USER_UNIQUE_ID),
                          &user_id,
                          sizeof(user_id));
  user_id = 1222;
  attribute_store_emplace(endpoint_id_node,
                          ATTRIBUTE(USER_UNIQUE_ID),
                          &user_id,
                          sizeof(user_id));

  // Set the network status to online
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // Now we check attribute store
  auto user_id_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(USER_UNIQUE_ID));

  attribute_store_get_desired(user_id_node, &user_id, sizeof(user_id));
  TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                user_id_node,
                                "User id node should exists");
  TEST_ASSERT_EQUAL_MESSAGE(0, user_id, "User ID desired value should be 0");
  count_user_node(1);
}

}  // extern "C"