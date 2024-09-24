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
#include "attribute.hpp"

// Test helpers
#include "zwave_command_class_test_helper.hpp"

using namespace zwave_command_class_test_helper;

extern "C" {

#include "zwave_command_class_user_credential.h"
#include "zwave_command_class_user_credential_api.h"
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

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"
#include "dotdot_mqtt_mock.h"
#include "dotdot_mqtt_generated_commands_mock.h"
#include "zwave_command_class_notification_mock.h"

// Used for delayed interview
#include "zwave_network_management_mock.h"

#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_##type

const resolver_function_map attributes_binding
  = {{ATTRIBUTE(NUMBER_OF_USERS), {USER_CAPABILITIES_GET, 0}},
     {ATTRIBUTE(SUPPORT_CREDENTIAL_CHECKSUM), {CREDENTIAL_CAPABILITIES_GET, 0}},
     {ATTRIBUTE(ALL_USERS_CHECKSUM), {ALL_USERS_CHECKSUM_GET, 0}},
     {ATTRIBUTE(USER_UNIQUE_ID), {USER_GET, 0}},
     {ATTRIBUTE(USER_OPERATION_TYPE), {0, USER_SET}},
     {ATTRIBUTE(CREDENTIAL_SLOT), {CREDENTIAL_GET, 0}},
     {ATTRIBUTE(CREDENTIAL_OPERATION_TYPE), {0, CREDENTIAL_SET}},
     {ATTRIBUTE(CREDENTIAL_LEARN_OPERATION_TYPE), {0, CREDENTIAL_LEARN_START}},
     {ATTRIBUTE(CREDENTIAL_LEARN_STOP), {0, CREDENTIAL_LEARN_CANCEL}},
     {ATTRIBUTE(ASSOCIATION_DESTINATION_CREDENTIAL_SLOT),
      {0, USER_CREDENTIAL_ASSOCIATION_SET}},
     {ATTRIBUTE(USER_CHECKSUM), {USER_CHECKSUM_GET, 0}},
     {ATTRIBUTE(CREDENTIAL_CHECKSUM), {CREDENTIAL_CHECKSUM_GET, 0}},
     {ATTRIBUTE(ADMIN_PIN_CODE), {ADMIN_PIN_CODE_GET, ADMIN_PIN_CODE_SET}}};

// Tested command class handler
const zwave_struct_handler_args command_class_handler
  = {.command_class_id  = COMMAND_CLASS_USER_CREDENTIAL,
     .supported_version = USER_CREDENTIAL_VERSION,
     .scheme            = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS};

/////////////////////////////////////////////////////
// HELPERS
/////////////////////////////////////////////////////
struct credential_structure_nodes {
  attribute_store::attribute user_id_node;
  attribute_store::attribute credential_type_node;
  attribute_store::attribute credential_slot_node;

  void print()
  {
    printf("%s / %s / %s\n",
           user_id_node.name_and_id().c_str(),
           credential_type_node.name_and_id().c_str(),
           credential_slot_node.name_and_id().c_str());
  };
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

  nodes.user_id_node
    = cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  nodes.credential_type_node
    = nodes.user_id_node.emplace_node(ATTRIBUTE(CREDENTIAL_TYPE),
                                      credential_type);

  nodes.credential_slot_node
    = nodes.credential_type_node.emplace_node(ATTRIBUTE(CREDENTIAL_SLOT),
                                              credential_slot,
                                              value_state);

  return nodes;
}

////////////////////////////////////////////////////
// Report helpers
////////////////////////////////////////////////////

void helper_simulate_user_capabilites_report(
  uint16_t number_of_users,
  user_credential_supported_credential_rules_t cred_rule_bitmask,
  uint8_t username_max_length,
  uint8_t support_user_schedule,
  uint8_t support_all_users_checksum,
  uint8_t support_user_checksum,
  uint8_t supported_user_types_bitmask_length,
  user_credential_supported_user_type_bitmask_t supported_user_types_bitmask)
{
  zwave_frame report_frame;

  report_frame.add(number_of_users);
  report_frame.add(cred_rule_bitmask);
  report_frame.add(username_max_length);
  uint8_t support_byte = (support_user_schedule << 7)
                         | (support_all_users_checksum << 6)
                         | (support_user_checksum << 5);
  report_frame.add(support_byte);
  report_frame.add_bitmask(supported_user_types_bitmask_length,
                           supported_user_types_bitmask);

  // Add bitmask value
  helper_test_report_frame(USER_CAPABILITIES_REPORT, report_frame);
}

void helper_simulate_credential_capabilites_report(
  uint8_t credential_checksum_support,
  uint8_t admin_code_support,
  uint8_t admin_code_deactivation_support,
  std::vector<user_credential_type_t> credential_type,
  std::vector<uint8_t> cl_support,
  std::vector<uint16_t> supported_credential_slots,
  std::vector<uint8_t> min_length,
  std::vector<uint8_t> max_length,
  std::vector<uint8_t> cl_timeout,
  std::vector<uint8_t> cl_steps)
{
  if (credential_type.size() != cl_support.size()
      || credential_type.size() != supported_credential_slots.size()
      || credential_type.size() != min_length.size()
      || credential_type.size() != max_length.size()
      || credential_type.size() != cl_timeout.size()
      || credential_type.size() != cl_steps.size()) {
    TEST_FAIL_MESSAGE("Size of credential_type, cl_support, "
                      "supported_credential_slots, min_length, max_length, "
                      "cl_timeout and cl_steps"
                      "should be the same.");
  }

  zwave_frame report_frame;

  report_frame.add(static_cast<uint8_t>(
    credential_checksum_support << 7 | admin_code_support << 6
    | admin_code_deactivation_support << 5));
  report_frame.add(static_cast<uint8_t>(credential_type.size()));

  for (auto &c: credential_type) {
    report_frame.add(c);
  }
  for (auto &cl: cl_support) {
    report_frame.add(static_cast<uint8_t>(cl << 7));
  }
  for (auto &c: supported_credential_slots) {
    report_frame.add(c);
  }
  for (auto &c: min_length) {
    report_frame.add(c);
  }
  for (auto &c: max_length) {
    report_frame.add(c);
  }
  for (auto &c: cl_timeout) {
    report_frame.add(c);
  }
  for (auto &c: cl_steps) {
    report_frame.add(c);
  }

  helper_test_report_frame(CREDENTIAL_CAPABILITIES_REPORT, report_frame);
};

void helper_simulate_user_checksum_report(
  user_credential_user_unique_id_t user_id,
  user_credential_checksum_t expected_checksum,
  sl_status_t expected_status = SL_STATUS_OK)
{
  zwave_frame report_frame;
  report_frame.add(user_id);
  report_frame.add(expected_checksum);

  helper_test_report_frame(USER_CHECKSUM_REPORT, report_frame, expected_status);
}

void helper_simulate_credential_checksum_report(
  user_credential_type_t credential_type,
  user_credential_checksum_t expected_checksum,
  sl_status_t expected_status = SL_STATUS_OK)
{
  zwave_frame report_frame;
  report_frame.add(credential_type);
  report_frame.add(expected_checksum);

  helper_test_report_frame(CREDENTIAL_CHECKSUM_REPORT,
                           report_frame,
                           expected_status);
}

void helper_simulate_learn_status_report_frame(
  uint8_t credential_learn_status,
  user_credential_user_unique_id_t user_id,
  user_credential_type_t credential_type,
  user_credential_slot_t credential_slot,
  uint8_t learn_steps_remaining)
{
  zwave_frame report_frame;

  report_frame.add(credential_learn_status);
  report_frame.add(user_id);
  report_frame.add(credential_type);
  report_frame.add(credential_slot);
  report_frame.add(learn_steps_remaining);

  helper_test_report_frame(CREDENTIAL_LEARN_REPORT, report_frame);
}

void helper_simulate_association_report_frame(
  user_credential_user_unique_id_t source_user_id,
  user_credential_type_t source_credential_type,
  user_credential_slot_t source_credential_slot,
  user_credential_user_unique_id_t destination_user_id,
  user_credential_slot_t destination_credential_slot,
  uint8_t credential_association_status,
  sl_status_t expected_status = SL_STATUS_OK)
{
  zwave_frame report_frame;

  report_frame.add(source_user_id);
  report_frame.add(source_credential_type);
  report_frame.add(source_credential_slot);
  report_frame.add(destination_user_id);
  report_frame.add(destination_credential_slot);
  report_frame.add(credential_association_status);

  helper_test_report_frame(USER_CREDENTIAL_ASSOCIATION_REPORT,
                           report_frame,
                           expected_status);
}

void helper_test_credential_learn_structure(
  attribute_store::attribute user_id_node,
  attribute_store::attribute credential_type_node,
  attribute_store::attribute credential_slot_node,
  user_credential_learn_timeout_t expected_cl_timeout,
  user_credential_operation_type_t expected_operation_type)
{
  TEST_ASSERT_TRUE_MESSAGE(user_id_node.is_valid(),
                           "Credential type node should exist");
  TEST_ASSERT_TRUE_MESSAGE(credential_type_node.is_valid(),
                           "Credential type node should exist");
  TEST_ASSERT_TRUE_MESSAGE(credential_slot_node.is_valid(),
                           "Credential type slot should exist");

  helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_LEARN_TIMEOUT),
                              expected_cl_timeout,
                              credential_slot_node);

  auto operation_type_node
    = helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_LEARN_OPERATION_TYPE),
                                  expected_operation_type,
                                  credential_slot_node,
                                  DESIRED_ATTRIBUTE);

  user_credential_user_unique_id_t user_id
    = user_id_node.reported<user_credential_user_unique_id_t>();
  user_credential_type_t credential_type
    = credential_type_node.reported<user_credential_type_t>();
  user_credential_slot_t credential_slot
    = credential_slot_node.reported<user_credential_slot_t>();

  zwave_frame learn_start_frame;
  learn_start_frame.add(user_id);
  learn_start_frame.add(credential_type);
  learn_start_frame.add(credential_slot);
  learn_start_frame.add(expected_operation_type);
  learn_start_frame.add(expected_cl_timeout);

  helper_test_get_set_frame_happy_case(CREDENTIAL_LEARN_START,
                                       operation_type_node,
                                       learn_start_frame);

  uint8_t step_remaining                      = 2;
  user_credential_learn_status_t learn_status = CREDENTIAL_LEARN_REPORT_SUCCESS;
  helper_simulate_learn_status_report_frame(learn_status,
                                            user_id,
                                            credential_type,
                                            credential_slot,
                                            step_remaining);

  helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_LEARN_STATUS),
                              learn_status,
                              credential_slot_node);
  helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_LEARN_STEPS_REMAINING),
                              step_remaining,
                              credential_slot_node);
};

struct association_nodes_t {
  attribute_store::attribute association_user_id_node;
  attribute_store::attribute association_credential_slot_node;
};
association_nodes_t
  helper_setup_association(attribute_store::attribute credential_slot_node,
                           user_credential_user_unique_id_t destination_user_id,
                           user_credential_slot_t destination_credential_slot)
{
  association_nodes_t association_nodes;

  association_nodes.association_user_id_node
    = credential_slot_node.emplace_node(
      ATTRIBUTE(ASSOCIATION_DESTINATION_USER_ID),
      destination_user_id,
      DESIRED_ATTRIBUTE);

  association_nodes.association_credential_slot_node
    = credential_slot_node.emplace_node(
      ATTRIBUTE(ASSOCIATION_DESTINATION_CREDENTIAL_SLOT),
      destination_credential_slot,
      DESIRED_ATTRIBUTE);

  return association_nodes;
}

void helper_fill_credential_data(
  attribute_store::attribute credential_slot_node,
  std::string credential_data,
  user_credential_modifier_type_t modifier_type)
{
  credential_slot_node.emplace_node(ATTRIBUTE(CREDENTIAL_DATA),
                                    credential_data);
  credential_slot_node.emplace_node(ATTRIBUTE(CREDENTIAL_MODIFIER_TYPE),
                                    modifier_type);
}

void helper_test_credential_data(
  attribute_store::attribute credential_slot_node,
  std::string credential_data,
  user_credential_modifier_type_t modifier_type)
{
  helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_DATA),
                              credential_data,
                              credential_slot_node);

  helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_MODIFIER_TYPE),
                              modifier_type,
                              credential_slot_node);
}

void helper_simulate_credential_report_frame(
  uint8_t credential_report_type,
  user_credential_user_unique_id_t user_id,
  user_credential_type_t credential_type,
  user_credential_slot_t credential_slot,
  uint8_t crb,
  std::vector<uint8_t> credential_data,
  user_credential_modifier_type_t credential_modifier_type,
  user_credential_modifier_node_id_t credential_modifier_node_id,
  user_credential_type_t next_credential_type,
  user_credential_slot_t next_credential_slot,
  sl_status_t expected_status = SL_STATUS_OK)
{
  zwave_frame report_frame;
  report_frame.add(credential_report_type);
  report_frame.add(user_id);
  report_frame.add(credential_type);
  report_frame.add(credential_slot);
  report_frame.add(static_cast<uint8_t>(crb << 7));
  report_frame.add(credential_data);
  report_frame.add(credential_modifier_type);
  report_frame.add(credential_modifier_node_id);
  report_frame.add(next_credential_type);
  report_frame.add(next_credential_slot);

  helper_test_report_frame(CREDENTIAL_REPORT, report_frame, expected_status);
};

std::vector<uint8_t> string_to_uint8_vector(const std::string &str)
{
  return std::vector<uint8_t>(str.begin(), str.end());
}

void helper_create_credential_checksum_structure()
{
  // Since credential are attached to an user we simulate somes users
  std::vector<user_credential_user_unique_id_t> user_ids = {12, 15, 18};
  std::map<user_credential_user_unique_id_t, attribute_store::attribute>
    user_id_nodes = {};

  for (auto user_id: user_ids) {
    user_id_nodes.insert(
      {user_id,
       cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id)});
  }

  // Create simulated envrionment for credentials
  helper_simulate_credential_capabilites_report(1,
                                                1,
                                                1,
                                                {ZCL_CRED_TYPE_PIN_CODE,
                                                 ZCL_CRED_TYPE_PASSWORD,
                                                 ZCL_CRED_TYPE_EYE_BIOMETRIC},
                                                {1, 1, 1},
                                                {8, 8, 8},
                                                {2, 34, 5},
                                                {6, 6, 10},
                                                {20, 20, 2},
                                                {1, 1, 95});

  std::vector<user_credential_user_unique_id_t> credential_user_ids
    = {user_ids[0], user_ids[1], user_ids[0], user_ids[2]};

  std::vector<user_credential_type_t> credential_types
    = {ZCL_CRED_TYPE_PIN_CODE,
       ZCL_CRED_TYPE_PIN_CODE,
       ZCL_CRED_TYPE_PASSWORD,
       ZCL_CRED_TYPE_EYE_BIOMETRIC};
  std::vector<user_credential_slot_t> credential_slots = {2, 4, 1, 8};

  std::
    vector<std::vector<uint8_t>>
      credential_data = {
        {0x39, 0x32, 0x37, 0x37},              // "9277" in ASCII
        {0x39, 0x35, 0x34, 0x39, 0x38, 0x38},  // "954988" in ASCII
        {
          0x00, 0x7A, 0x00, 0x77, 0x00, 0x61, 0x00, 0x76, 0x00,
          0x65, 0x00, 0x6E, 0x00, 0x6F, 0x00, 0x64, 0x00, 0x65,
          0x00, 0x70, 0x00, 0x61, 0x00, 0x73, 0x00, 0x73, 0x00,
          0x77, 0x00, 0x6F, 0x00, 0x72, 0x00, 0x64},  // zwavenodepassword in Unicode UTF-16 format, in big endian order,
        {0x24, 0x01}  // Raw data
      };

  if (credential_types.size() != credential_slots.size()
      || credential_slots.size() != credential_data.size()
      || credential_slots.size() != credential_user_ids.size()) {
    TEST_FAIL_MESSAGE("All vectors should be the same size");
  }

  for (size_t i = 0; i < credential_types.size(); i++) {
    helper_simulate_credential_report_frame(0x00,  // Credential Add
                                            credential_user_ids[i],
                                            credential_types[i],
                                            credential_slots[i],
                                            0,
                                            credential_data[i],
                                            0x02,  // Anything but 0
                                            0,
                                            0,
                                            0);
  }
}

/////////////////////////////////////////////////////
// Test case
/////////////////////////////////////////////////////

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
  zwave_setUp(command_class_handler,
              &zwave_command_class_user_credential_init,
              attributes_binding);
}

/// Called after each and every test
void tearDown() {}

////////////////////////////////////////////////////////////////////////////
// User Capabilities Get/Report
////////////////////////////////////////////////////////////////////////////
void test_user_credential_user_capabilities_get_happy_case()
{
  helper_test_get_set_frame_happy_case(USER_CAPABILITIES_GET);
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

  // Test reported values based on the variables in this function
  auto test_reported_values = [&]() {
    // Create frame object
    helper_simulate_user_capabilites_report(number_of_users,
                                            cred_rule_bitmask,
                                            username_max_length,
                                            support_user_schedule,
                                            support_all_users_checksum,
                                            support_user_checksum,
                                            supported_user_types_bitmask_length,
                                            supported_user_types_bitmask);

    helper_test_attribute_value(ATTRIBUTE(NUMBER_OF_USERS), number_of_users);
    helper_test_attribute_value(ATTRIBUTE(SUPPORTED_CREDENTIAL_RULES),
                                cred_rule_bitmask);
    helper_test_attribute_value(ATTRIBUTE(MAX_USERNAME_LENGTH),
                                username_max_length);
    helper_test_attribute_value(ATTRIBUTE(SUPPORT_USER_SCHEDULE),
                                support_user_schedule);
    helper_test_attribute_value(ATTRIBUTE(SUPPORT_ALL_USERS_CHECKSUM),
                                support_all_users_checksum);
    helper_test_attribute_value(ATTRIBUTE(SUPPORT_USER_CHECKSUM),
                                support_user_checksum);
    helper_test_attribute_value(ATTRIBUTE(SUPPORTED_USER_TYPES),
                                supported_user_types_bitmask);

    attribute_store_node_t all_user_checksum_node
      = cpp_endpoint_id_node.child_by_type(ATTRIBUTE(ALL_USERS_CHECKSUM));
    TEST_ASSERT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                              all_user_checksum_node,
                              "ALL_USERS_CHECKSUM node should NOT exists yet "
                              "even if support_all_users_checksum == true");
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
  helper_test_get_set_frame_happy_case(CREDENTIAL_CAPABILITIES_GET);
}

void test_user_credential_credential_capabilities_report_happy_case()
{
  uint8_t credential_checksum_support                 = 1;
  uint8_t admin_code_support                          = 1;
  uint8_t admin_code_deactivation_support             = 1;
  std::vector<user_credential_type_t> credential_type = {1, 3, 4, 5};
  std::vector<uint8_t> cl_support                     = {1, 0, 0, 1};
  std::vector<uint16_t> supported_credential_slots = {1233, 11233, 21233, 33};
  std::vector<uint8_t> min_length                  = {2, 23, 255, 12};
  std::vector<uint8_t> max_length                  = {56, 156, 255, 32};
  std::vector<uint8_t> cl_timeout                  = {100, 0, 0, 128};
  std::vector<uint8_t> cl_steps                    = {2, 0, 0, 12};
  uint16_t expected_credential_type_mask           = 0b11101;

  auto test_report_values = [&]() {
    helper_test_attribute_value(ATTRIBUTE(SUPPORT_CREDENTIAL_CHECKSUM),
                                credential_checksum_support);
    helper_test_attribute_value(ATTRIBUTE(SUPPORT_ADMIN_PIN_CODE),
                                admin_code_support);
    helper_test_attribute_value(ATTRIBUTE(SUPPORT_ADMIN_PIN_CODE_DEACTIVATION),
                                admin_code_deactivation_support);
    auto admin_code_node
      = cpp_endpoint_id_node.child_by_type(ATTRIBUTE(ADMIN_PIN_CODE));
    if (admin_code_support == 1) {
      TEST_ASSERT_TRUE_MESSAGE(admin_code_node.is_valid(),
                               "Admin code node should exists");
      // For next tests
      admin_code_node.delete_node();
    } else {
      TEST_ASSERT_FALSE_MESSAGE(admin_code_node.is_valid(),
                                "Admin code node should NOT exists");
    }

    helper_test_attribute_value(
      DOTDOT_ATTRIBUTE_ID_USER_CREDENTIAL_SUPPORTED_CREDENTIAL_TYPES,
      expected_credential_type_mask);

    for (uint8_t i = 0; i < credential_type.size(); i++) {
      printf("Testing credential batch %d\n", i);

      auto type_node = cpp_endpoint_id_node.child_by_type(
        ATTRIBUTE(SUPPORTED_CREDENTIAL_TYPE),
        i);
      TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                    type_node,
                                    "Credential type node should be defined");
      TEST_ASSERT_EQUAL_MESSAGE(credential_type[i],
                                type_node.reported<user_credential_type_t>(),
                                "Incorrect credential type");

      helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_MIN_LENGTH),
                                  min_length[i],
                                  type_node);
      helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_MAX_LENGTH),
                                  max_length[i],
                                  type_node);
      helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_LEARN_SUPPORT),
                                  cl_support[i],
                                  type_node);
      helper_test_attribute_value(
        ATTRIBUTE(CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT),
        cl_timeout[i],
        type_node);
      helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_LEARN_NUMBER_OF_STEPS),
                                  cl_steps[i],
                                  type_node);
      helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_SUPPORTED_SLOT_COUNT),
                                  supported_credential_slots[i],
                                  type_node);
    }
  };

  printf("Test with first set of data\n");
  helper_simulate_credential_capabilites_report(credential_checksum_support,
                                                admin_code_support,
                                                admin_code_deactivation_support,
                                                credential_type,
                                                cl_support,
                                                supported_credential_slots,
                                                min_length,
                                                max_length,
                                                cl_timeout,
                                                cl_steps);

  test_report_values();

  printf("Test with second set of data\n");
  credential_checksum_support     = 0;
  admin_code_support              = 0;
  admin_code_deactivation_support = 0;
  credential_type                 = {6, 7, 8};
  cl_support                      = {0, 1, 1};
  supported_credential_slots      = {15, 1565, 153};
  min_length                      = {155, 15, 5};
  max_length                      = {180, 111, 11};
  cl_timeout                      = {0, 10, 12};
  cl_steps                        = {0, 1, 2};
  expected_credential_type_mask   = 0b11100000;

  helper_simulate_credential_capabilites_report(credential_checksum_support,
                                                admin_code_support,
                                                admin_code_deactivation_support,
                                                credential_type,
                                                cl_support,
                                                supported_credential_slots,
                                                min_length,
                                                max_length,
                                                cl_timeout,
                                                cl_steps);

  test_report_values();
}

////////////////////////////////////////////////////////////////////////////
// User Set/Get/Report
////////////////////////////////////////////////////////////////////////////
void test_user_credential_user_set_add_or_modify_happy_case()
{
  user_credential_user_unique_id_t user_id        = 12121;
  user_credential_operation_type_t operation_type = USER_SET_OPERATION_TYPE_ADD;
  user_credential_type_t user_type                = 5;
  user_credential_rule_t credential_rule          = 2;
  user_credential_user_active_state_t user_active_state       = 1;
  user_credential_expiring_timeout_minutes_t expiring_timeout = 55;
  user_credential_user_name_encoding_t user_name_encoding     = 0;
  std::string user_name                                       = "DoUzE";

  // Create structure
  auto user_id_node
    = cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  auto operation_type_node
    = user_id_node.emplace_node(ATTRIBUTE(USER_OPERATION_TYPE),
                                operation_type,
                                DESIRED_ATTRIBUTE);
  user_id_node.emplace_node(ATTRIBUTE(USER_TYPE), user_type, DESIRED_ATTRIBUTE);
  user_id_node.emplace_node(ATTRIBUTE(USER_ACTIVE_STATE), user_active_state);
  user_id_node.emplace_node(ATTRIBUTE(CREDENTIAL_RULE),
                            credential_rule,
                            DESIRED_ATTRIBUTE);
  user_id_node.emplace_node(ATTRIBUTE(USER_EXPIRING_TIMEOUT_MINUTES),
                            expiring_timeout);
  user_id_node.emplace_node(ATTRIBUTE(USER_NAME_ENCODING),
                            user_name_encoding,
                            DESIRED_ATTRIBUTE);
  user_id_node.emplace_node(ATTRIBUTE(USER_NAME), user_name);

  auto create_and_send_set_frame = [&]() {
    zwave_frame set_frame;
    set_frame.add(operation_type);
    set_frame.add(user_id);
    set_frame.add(user_type);
    set_frame.add(user_active_state);
    set_frame.add(credential_rule);
    set_frame.add(expiring_timeout);
    set_frame.add(user_name_encoding);
    set_frame.add(user_name);

    helper_test_get_set_frame_happy_case(USER_SET,
                                         operation_type_node,
                                         set_frame);
  };

  // Test user set
  printf("Send with USER_SET_OPERATION_TYPE_ADD\n");
  create_and_send_set_frame();

  printf("Send with USER_SET_OPERATION_TYPE_MODIFY\n");
  operation_type = USER_SET_OPERATION_TYPE_MODIFY;
  operation_type_node.set_desired(operation_type);
  create_and_send_set_frame();
}

void test_user_credential_user_delete_happy_case()
{
  user_credential_user_unique_id_t user_id = 12121;
  user_credential_operation_type_t operation_type
    = USER_SET_OPERATION_TYPE_DELETE;

  auto user_id_node
    = cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  auto operation_type_node
    = user_id_node.emplace_node(ATTRIBUTE(USER_OPERATION_TYPE),
                                operation_type,
                                DESIRED_ATTRIBUTE);

  zwave_frame set_frame;
  set_frame.add(operation_type);
  set_frame.add(user_id);

  helper_test_get_set_frame_happy_case(USER_SET,
                                       operation_type_node,
                                       set_frame);
}

void test_user_credential_user_set_invalid_node()
{
  helper_test_get_set_fail_case(USER_GET, SL_STATUS_NOT_SUPPORTED);
}

void test_user_credential_user_get_happy_case()
{
  user_credential_user_unique_id_t user_id = 12;
  auto user_id_node
    = cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID),
                                        user_id,
                                        DESIRED_ATTRIBUTE);

  zwave_frame get_frame;
  get_frame.add(user_id);

  helper_test_get_set_frame_happy_case(USER_GET, user_id_node, get_frame);
}

void test_user_credential_user_get_not_found()
{
  helper_test_get_set_fail_case(USER_GET, SL_STATUS_NOT_SUPPORTED);
}

void helper_simulate_user_report_frame(
  uint8_t user_report_type,
  user_credential_user_unique_id_t next_user_id,
  user_credential_modifier_type_t user_modifier_type,
  user_credential_modifier_node_id_t user_modifier_node_id,
  user_credential_user_unique_id_t user_id,
  user_credential_user_type_t user_type,
  user_credential_user_active_state_t user_active_state,
  user_credential_supported_credential_rules_t credential_rule,
  user_credential_expiring_timeout_minutes_t expiring_timeout_minutes,
  user_credential_user_name_encoding_t user_name_encoding,
  const std::string &user_name,
  sl_status_t expected_status = SL_STATUS_OK)
{
  zwave_frame report_frame;
  report_frame.add(user_report_type);
  report_frame.add(next_user_id);
  report_frame.add(user_modifier_type);
  report_frame.add(user_modifier_node_id);
  report_frame.add(user_id);
  report_frame.add(user_type);
  report_frame.add(user_active_state);
  report_frame.add(credential_rule);
  report_frame.add(expiring_timeout_minutes);
  report_frame.add(user_name_encoding);
  report_frame.add(user_name);

  // Use helper function to get the report frame
  helper_test_report_frame(USER_REPORT, report_frame, expected_status);
}

// FIXME: Will be changed
// https://github.com/Z-Wave-Alliance/AWG/issues/168
std::vector<uint8_t> helper_create_credential_notification_report(
  user_credential_user_unique_id_t user_id,
  user_credential_type_t credential_type,
  user_credential_slot_t credential_slot,
  bool crb,
  const std::string &credential_data,
  user_credential_modifier_type_t modifier_type,
  user_credential_modifier_node_id_t modifier_node_id)
{
  zwave_frame report_frame;

  report_frame.add(user_id);
  report_frame.add(credential_type);
  report_frame.add(credential_slot);
  report_frame.add(static_cast<uint8_t>(crb ? 1 : 0));
  report_frame.add(credential_data);
  report_frame.add(modifier_type);
  report_frame.add(modifier_node_id);

  return report_frame;
};

void test_user_credential_user_report_happy_case()
{
  constexpr user_credential_user_unique_id_t EXPECTED_FIRST_USER_ID  = 12;
  constexpr user_credential_user_unique_id_t EXPECTED_SECOND_USER_ID = 1212;

  user_credential_user_unique_id_t next_user_id      = EXPECTED_SECOND_USER_ID;
  user_credential_modifier_type_t user_modifier_type = 2;
  user_credential_modifier_node_id_t user_modifier_node_id = 1313;
  user_credential_user_unique_id_t user_id = EXPECTED_FIRST_USER_ID;
  user_credential_user_type_t user_type    = 3;
  user_credential_user_active_state_t user_active_state               = 1;
  user_credential_supported_credential_rules_t credential_rule        = 2;
  user_credential_expiring_timeout_minutes_t expiring_timeout_minutes = 1515;
  user_credential_user_name_encoding_t user_name_encoding             = 0;
  std::string user_name                                               = "DoUzE";

  // Those functions are exposed and checks user values, so we need to setup the capabilities
  uint16_t number_of_users                                       = 1312;
  user_credential_supported_credential_rules_t cred_rule_bitmask = 0x0F;
  uint8_t username_max_length                                    = 112;
  uint8_t support_user_schedule                                  = 0;
  uint8_t support_all_users_checksum
    = 1;  // Used to check if the user checksum will be computed at the end
  uint8_t support_user_checksum               = 0;
  uint8_t supported_user_types_bitmask_length = 2;
  user_credential_supported_user_type_bitmask_t supported_user_types_bitmask
    = 0xFF;
  helper_simulate_user_capabilites_report(number_of_users,
                                          cred_rule_bitmask,
                                          username_max_length,
                                          support_user_schedule,
                                          support_all_users_checksum,
                                          support_user_checksum,
                                          supported_user_types_bitmask_length,
                                          supported_user_types_bitmask);

  auto test_user_values = [&](const attribute_store::attribute &user_node) {
    TEST_ASSERT_EQUAL_MESSAGE(
      user_id,
      user_node.reported<user_credential_user_unique_id_t>(),
      "User Unique ID is incorrect");

    helper_test_attribute_value(ATTRIBUTE(USER_MODIFIER_TYPE),
                                user_modifier_type,
                                user_node);
    helper_test_attribute_value(ATTRIBUTE(USER_TYPE), user_type, user_node);
    helper_test_attribute_value(ATTRIBUTE(USER_ACTIVE_STATE),
                                user_active_state,
                                user_node);
    helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_RULE),
                                credential_rule,
                                user_node);
    helper_test_attribute_value(ATTRIBUTE(USER_EXPIRING_TIMEOUT_MINUTES),
                                expiring_timeout_minutes,
                                user_node);
    helper_test_attribute_value(ATTRIBUTE(USER_NAME_ENCODING),
                                user_name_encoding,
                                user_node);
    helper_test_attribute_value(ATTRIBUTE(USER_NAME), user_name, user_node);
    helper_test_attribute_value(ATTRIBUTE(USER_MODIFIER_NODE_ID),
                                user_modifier_node_id,
                                user_node);

    // Check user credential desired values
    auto credential_type_node
      = helper_test_and_get_node(ATTRIBUTE(CREDENTIAL_TYPE), user_node);

    auto credential_type
      = credential_type_node.reported<user_credential_type_t>();
    auto credential_slot = helper_test_and_get_node(ATTRIBUTE(CREDENTIAL_SLOT),
                                                    credential_type_node)
                             .desired<user_credential_slot_t>();

    TEST_ASSERT_EQUAL_MESSAGE(
      0,
      credential_type,
      "Credential Type should be at 0 and should exists");
    TEST_ASSERT_EQUAL_MESSAGE(
      0,
      credential_slot,
      "Credential Slot should be at 0 and should exists");
  };  // end test_user_values lambda

  printf("First user creation\n");

  // Create first user
  auto first_user_id_node
    = cpp_endpoint_id_node.add_node(ATTRIBUTE(USER_UNIQUE_ID));
  first_user_id_node.set_desired<user_credential_user_unique_id_t>(0);

  zwave_frame get_frame;
  get_frame.add(static_cast<user_credential_user_unique_id_t>(0));
  helper_test_get_set_frame_happy_case(USER_GET, first_user_id_node, get_frame);

  // Call report
  helper_simulate_user_report_frame(0x04,
                                    next_user_id,
                                    user_modifier_type,
                                    user_modifier_node_id,
                                    user_id,
                                    user_type,
                                    user_active_state,
                                    credential_rule,
                                    expiring_timeout_minutes,
                                    user_name_encoding,
                                    user_name);

  // Test values
  // For the first
  first_user_id_node
    = cpp_endpoint_id_node.child_by_type(ATTRIBUTE(USER_UNIQUE_ID), 1);
  test_user_values(first_user_id_node);

  // Test structure
  auto user_id_count
    = cpp_endpoint_id_node.children(ATTRIBUTE(USER_UNIQUE_ID)).size();
  TEST_ASSERT_EQUAL_MESSAGE(2,
                            user_id_count,
                            "User node count mismatch. Should be 2 by now. One "
                            "with special id 0 and the "
                            "reported one");

  // Simulate no credential for this User
  helper_simulate_credential_report_frame(0x00,
                                          user_id,
                                          0,
                                          0,
                                          0,
                                          std::vector<uint8_t>(),
                                          0,
                                          0,
                                          0,
                                          0);
  auto second_user_id_node
    = cpp_endpoint_id_node.child_by_type(ATTRIBUTE(USER_UNIQUE_ID), 2);
  
  TEST_ASSERT_TRUE_MESSAGE(
    second_user_id_node.is_valid(),
    "Second user node should be created & valid");

  TEST_ASSERT_FALSE_MESSAGE(
    cpp_endpoint_id_node.child_by_type(ATTRIBUTE(ALL_USERS_CHECKSUM))
      .is_valid(),
    "ALL_USERS_CHECKSUM node should not be created yet");

  user_id_count
    = cpp_endpoint_id_node.children(ATTRIBUTE(USER_UNIQUE_ID)).size();
  TEST_ASSERT_EQUAL_MESSAGE(
    3,
    user_id_count,
    "User node count mismatch. Should be 3 by now. One "
    "with special id 0, the reported one & the next in queue");

  printf("Second and last user creation\n");

  TEST_ASSERT_EQUAL_MESSAGE(
    next_user_id,
    second_user_id_node.desired<user_credential_user_unique_id_t>(),
    "Second user id should have it's desired value defined");

  // Simulate a GET on that user
  // Removes the desired state and sets to reported instead
  get_frame.clear();
  get_frame.add(next_user_id);
  helper_test_get_set_frame_happy_case(USER_GET,
                                       second_user_id_node,
                                       get_frame);

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

  helper_simulate_user_report_frame(0x04,
                                    next_user_id,
                                    user_modifier_type,
                                    user_modifier_node_id,
                                    user_id,
                                    user_type,
                                    user_active_state,
                                    credential_rule,
                                    expiring_timeout_minutes,
                                    user_name_encoding,
                                    user_name);
  // Test values
  test_user_values(second_user_id_node);

  // Test structure
  // The report function should not have created an other user unique id node (next_user_id = 0)
  user_id_count
    = cpp_endpoint_id_node.children(ATTRIBUTE(USER_UNIQUE_ID)).size();

  TEST_ASSERT_EQUAL_MESSAGE(
    3,
    user_id_count,
    "User node count mismatch. Should only 2 users + 1 with id 0 created.");

  TEST_ASSERT_EQUAL_MESSAGE(
    EXPECTED_FIRST_USER_ID,
    first_user_id_node.reported<user_credential_user_unique_id_t>(),
    "First user id mismatch");
  TEST_ASSERT_EQUAL_MESSAGE(
    EXPECTED_SECOND_USER_ID,
    second_user_id_node.reported<user_credential_user_unique_id_t>(),
    "Second user id mismatch");

  // Still no credential, so we can create the all user checksum
  helper_simulate_credential_report_frame(0x00,
                                          user_id,
                                          0,
                                          0,
                                          0,
                                          std::vector<uint8_t>(),
                                          0,
                                          0,
                                          0,
                                          0);
  TEST_ASSERT_TRUE_MESSAGE(
    cpp_endpoint_id_node.child_by_type(ATTRIBUTE(ALL_USERS_CHECKSUM))
      .is_valid(),
    "ALL_USERS_CHECKSUM node should be created");
}

void test_user_credential_no_all_users_checksum()
{
  user_credential_user_unique_id_t next_user_id                       = 0;
  user_credential_modifier_type_t user_modifier_type                  = 2;
  user_credential_modifier_node_id_t user_modifier_node_id            = 1313;
  user_credential_user_unique_id_t user_id                            = 12;
  user_credential_user_type_t user_type                               = 3;
  user_credential_user_active_state_t user_active_state               = 1;
  user_credential_supported_credential_rules_t credential_rule        = 2;
  user_credential_expiring_timeout_minutes_t expiring_timeout_minutes = 1515;
  user_credential_user_name_encoding_t user_name_encoding             = 0;
  std::string user_name                                               = "DoUzE";

  // Those functions are exposed and checks user values, so we need to setup the capabilities
  uint16_t number_of_users                                       = 1312;
  user_credential_supported_credential_rules_t cred_rule_bitmask = 0x0F;
  uint8_t username_max_length                                    = 112;
  uint8_t support_user_schedule                                  = 0;
  uint8_t support_all_users_checksum
    = 0;  // Used to check if the user checksum will be computed at the end
  uint8_t support_user_checksum               = 0;
  uint8_t supported_user_types_bitmask_length = 2;
  user_credential_supported_user_type_bitmask_t supported_user_types_bitmask
    = 0xFF;
  helper_simulate_user_capabilites_report(number_of_users,
                                          cred_rule_bitmask,
                                          username_max_length,
                                          support_user_schedule,
                                          support_all_users_checksum,
                                          support_user_checksum,
                                          supported_user_types_bitmask_length,
                                          supported_user_types_bitmask);

  // Create first user
  auto first_user_id_node
    = cpp_endpoint_id_node.add_node(ATTRIBUTE(USER_UNIQUE_ID));
  first_user_id_node.set_desired<user_credential_user_unique_id_t>(0);

  zwave_frame get_frame;
  get_frame.add(static_cast<user_credential_user_unique_id_t>(0));
  helper_test_get_set_frame_happy_case(USER_GET, first_user_id_node, get_frame);

  // Call report
  helper_simulate_user_report_frame(0x04,
                                    next_user_id,
                                    user_modifier_type,
                                    user_modifier_node_id,
                                    user_id,
                                    user_type,
                                    user_active_state,
                                    credential_rule,
                                    expiring_timeout_minutes,
                                    user_name_encoding,
                                    user_name);

  TEST_ASSERT_FALSE_MESSAGE(
    cpp_endpoint_id_node.child_by_type(ATTRIBUTE(ALL_USERS_CHECKSUM))
      .is_valid(),
    "ALL_USERS_CHECKSUM node should not be created yet");
}
void test_user_credential_user_report_user_with_id0()
{
  // User Report should have returned SL_STATUS_OK and ignore the frame
  helper_simulate_user_report_frame(0x04, 0, 0, 0, 0, 0, 0, 0, 0, 0, "DOUZE");

  // Check that no user_unique_id node has been created (and the invalid node was deleted)
  auto user_id_count
    = cpp_endpoint_id_node.children(ATTRIBUTE(USER_UNIQUE_ID)).size();

  TEST_ASSERT_EQUAL_MESSAGE(0,
                            user_id_count,
                            "No user_unique_id node should be created");
}

void test_user_credential_user_report_user_deleted()
{
  // Create user id with reported ID of 0 to simulate user get of id 0
  user_credential_user_unique_id_t user_id = 12;
  cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID),
                                    user_id,
                                    DESIRED_ATTRIBUTE);

  auto user_id_count
    = cpp_endpoint_id_node.children(ATTRIBUTE(USER_UNIQUE_ID)).size();
  TEST_ASSERT_EQUAL_MESSAGE(1, user_id_count, "Should have one user by now");

  // Should delete this user
  helper_simulate_user_report_frame(0x04,
                                    user_id,
                                    USER_REPORT_DNE,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    "????");

  user_id_count
    = cpp_endpoint_id_node.children(ATTRIBUTE(USER_UNIQUE_ID)).size();
  TEST_ASSERT_EQUAL_MESSAGE(1,
                            user_id_count,
                            "Should not have any user by now");
}

////////////////////////////////////////////////////////////////////////////
// Credential Set/Get/Report
////////////////////////////////////////////////////////////////////////////

void helper_test_credential_set(
  user_credential_operation_type_t operation_type,
  std::vector<uint8_t> credential_data,
  attribute_store_node_value_state_t credential_data_state)
{
  user_credential_user_unique_id_t user_id = 12;
  user_credential_type_t credential_type   = 2;
  user_credential_slot_t credential_slot   = 1212;

  // Create the node with reported attribute
  auto nodes                = helper_create_credential_structure(user_id,
                                                  credential_type,
                                                  credential_slot,
                                                  REPORTED_ATTRIBUTE);
  auto credential_slot_node = nodes.credential_slot_node;

  // Operation type
  auto operation_type_node
    = credential_slot_node.emplace_node(ATTRIBUTE(CREDENTIAL_OPERATION_TYPE),
                                        operation_type,
                                        DESIRED_ATTRIBUTE);
  // CREDENTIAL_DATA
  credential_slot_node.emplace_node(ATTRIBUTE(CREDENTIAL_DATA),
                                    credential_data,
                                    credential_data_state);

  zwave_frame set_frame;
  set_frame.add(user_id);
  set_frame.add(credential_type);
  set_frame.add(credential_slot);
  set_frame.add(operation_type);
  if (credential_data.size() > 0) {
    set_frame.add(credential_data);
  } else {
    set_frame.add(static_cast<uint8_t>(0x00));
  }

  helper_test_get_set_frame_happy_case(CREDENTIAL_SET,
                                       operation_type_node,
                                       set_frame);
}

//>> Set
void test_user_credential_credential_set_1byte_happy_case()
{
  helper_test_credential_set(USER_CREDENTIAL_OPERATION_TYPE_ADD,
                             {12},
                             REPORTED_ATTRIBUTE);
}

void test_user_credential_credential_set_12byte_happy_case()
{
  helper_test_credential_set(USER_CREDENTIAL_OPERATION_TYPE_ADD,
                             {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
                             DESIRED_ATTRIBUTE);
}

void test_user_credential_credential_set_invalid_node()
{
  helper_test_get_set_fail_case(CREDENTIAL_SET);
}

void test_user_credential_credential_set_delete_happy_case()
{
  helper_test_credential_set(USER_CREDENTIAL_OPERATION_TYPE_DELETE,
                             {},                // Empty credential data
                             DESIRED_ATTRIBUTE  // doesn't matter here
  );
}

//>> Get
void test_user_credential_credential_get_happy_case()
{
  user_credential_user_unique_id_t user_id = 12;
  user_credential_type_t credential_type   = 2;
  user_credential_slot_t credential_slot   = 1212;

  auto nodes = helper_create_credential_structure(user_id,
                                                  credential_type,
                                                  credential_slot);

  zwave_frame get_frame;
  get_frame.add(user_id);
  get_frame.add(credential_type);
  get_frame.add(credential_slot);

  helper_test_get_set_frame_happy_case(CREDENTIAL_GET,
                                       nodes.credential_slot_node,
                                       get_frame);
}

void test_user_credential_credential_get_no_credential_type()
{
  helper_test_get_set_fail_case(CREDENTIAL_GET, SL_STATUS_NOT_SUPPORTED);
}

void test_user_credential_credential_report_no_credential()
{
  user_credential_user_unique_id_t user_id = 12;
  cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  // Should return ok since credential_type and credential node is 0
  helper_simulate_credential_report_frame(0x04,  // Credential get,
                                          user_id,
                                          0,
                                          0,
                                          0,
                                          {},
                                          0,
                                          0,
                                          0,
                                          0);
}

void test_user_credential_credential_report_missing_user()
{
  // Not found since we haven't created any user_unique_id
  helper_simulate_credential_report_frame(0x04,
                                          12,
                                          1,
                                          1,
                                          0,
                                          {},
                                          0,
                                          0,
                                          0,
                                          0,
                                          SL_STATUS_FAIL);
}

void test_user_credential_credential_report_happy_case()
{
  user_credential_user_unique_id_t user_id = 122;
  user_credential_type_t credential_type   = 12;
  user_credential_slot_t credential_slot   = 1212;
  uint8_t crb                              = 1;
  std::vector<uint8_t> credential_data     = {12, 13, 14, 15, 16};
  user_credential_modifier_type_t credential_modifier_type       = 13;
  user_credential_modifier_node_id_t credential_modifier_node_id = 1312;
  user_credential_type_t next_credential_type                    = 12;
  user_credential_slot_t next_credential_slot                    = 1;

  auto test_credential_values
    = [&](const attribute_store::attribute &credential_type_node,
          const attribute_store::attribute &credential_slot_node) {
        // Check main node value
        TEST_ASSERT_EQUAL_MESSAGE(
          credential_type,
          credential_type_node.reported<user_credential_type_t>(),
          "Credential type value mismatch");
        TEST_ASSERT_EQUAL_MESSAGE(
          credential_slot,
          credential_slot_node.reported<user_credential_slot_t>(),
          "Credential slot value mismatch");

        helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_READ_BACK),
                                    crb,
                                    credential_slot_node);
        helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_MODIFIER_TYPE),
                                    credential_modifier_type,
                                    credential_slot_node);
        helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_MODIFIER_NODE_ID),
                                    credential_modifier_node_id,
                                    credential_slot_node);
        helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_DATA),
                                    credential_data,
                                    credential_slot_node);
      };  // end test_credential_values lambda

  auto check_credentials_node_count
    = [](const attribute_store::attribute &user_id_node,
         size_t expected_credential_type_count,
         size_t expected_credential_slot_count) {
        auto count_credential_type
          = user_id_node.children(ATTRIBUTE(CREDENTIAL_TYPE)).size();
        TEST_ASSERT_EQUAL_MESSAGE(expected_credential_type_count,
                                  count_credential_type,
                                  "Incorrect credential type count");

        size_t count_credential_slot = 0;
        for (size_t i = 0; i < count_credential_type; i++) {
          auto credential_type_node
            = user_id_node.child_by_type(ATTRIBUTE(CREDENTIAL_TYPE), i);
          count_credential_slot
            += credential_type_node.children(ATTRIBUTE(CREDENTIAL_SLOT)).size();
        }

        TEST_ASSERT_EQUAL_MESSAGE(expected_credential_slot_count,
                                  count_credential_slot,
                                  "Incorrect credential slot count");
      };

  auto check_next_credential_node_value
    = [&](const attribute_store::attribute &next_credential_type_node,
          const attribute_store::attribute &next_credential_slot_node) {
        // First check node existence
        TEST_ASSERT_TRUE_MESSAGE(next_credential_type_node.is_valid(),
                                 "next_credential_type_node should exist");
        TEST_ASSERT_TRUE_MESSAGE(next_credential_slot_node.is_valid(),
                                 "next_credential_slot_node should exist");

        TEST_ASSERT_EQUAL_MESSAGE(
          next_credential_type,
          next_credential_type_node.reported<user_credential_type_t>(),
          "Next Credential Type value mismatch");
        TEST_ASSERT_EQUAL_MESSAGE(
          next_credential_slot,
          next_credential_slot_node.desired<user_credential_slot_t>(),
          "Next Credential Slot value mismatch");
      };

  //>>>> First credential
  printf("First credential creation\n");

  // Create first credential
  auto nodes
    = helper_create_credential_structure(user_id, 0, 0, DESIRED_ATTRIBUTE);

  auto user_id_node               = nodes.user_id_node;
  auto first_credential_type_node = nodes.credential_type_node;
  auto first_credential_slot_node = nodes.credential_slot_node;

  attribute_store_log();

  zwave_frame get_frame;
  get_frame.add(user_id);
  get_frame.add(static_cast<user_credential_type_t>(0));
  get_frame.add(static_cast<user_credential_slot_t>(0));

  // Simulate get on that credential
  helper_test_get_set_frame_happy_case(CREDENTIAL_GET,
                                       first_credential_slot_node,
                                       get_frame);

  // Call report
  helper_simulate_credential_report_frame(0x04,  // Response to get
                                          user_id,
                                          credential_type,
                                          credential_slot,
                                          crb,
                                          credential_data,
                                          credential_modifier_type,
                                          credential_modifier_node_id,
                                          next_credential_type,
                                          next_credential_slot);

  // Test values
  test_credential_values(first_credential_type_node,
                         first_credential_slot_node);
  // We should have 1 credential type and 2 credential slot
  check_credentials_node_count(user_id_node, 1, 2);

  // Test if next credentials are well defined
  auto second_credential_slot_node
    = first_credential_type_node.child_by_type(ATTRIBUTE(CREDENTIAL_SLOT), 1);
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

  // Simulate get on that credential
  get_frame.clear();
  get_frame.add(user_id);
  get_frame.add(credential_type);
  get_frame.add(credential_slot);
  helper_test_get_set_frame_happy_case(CREDENTIAL_GET,
                                       second_credential_slot_node,
                                       get_frame);

  // Call report
  helper_simulate_credential_report_frame(0x04,  // Response to get
                                          user_id,
                                          credential_type,
                                          credential_slot,
                                          crb,
                                          credential_data,
                                          credential_modifier_type,
                                          credential_modifier_node_id,
                                          next_credential_type,
                                          next_credential_slot);

  // Test values
  test_credential_values(first_credential_type_node,
                         second_credential_slot_node);
  // We should have 2 credential type and 2 credential slot
  check_credentials_node_count(user_id_node, 2, 3);

  auto second_credential_type_node
    = user_id_node.child_by_type(ATTRIBUTE(CREDENTIAL_TYPE), 1);
  auto third_credential_slot_node
    = second_credential_type_node.child_by_type(ATTRIBUTE(CREDENTIAL_SLOT), 0);

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

  // Simulate get on that credential
  get_frame.clear();
  get_frame.add(user_id);
  get_frame.add(credential_type);
  get_frame.add(credential_slot);
  helper_test_get_set_frame_happy_case(CREDENTIAL_GET,
                                       third_credential_slot_node,
                                       get_frame);

  // Call report
  helper_simulate_credential_report_frame(0x04,  // Response to get
                                          user_id,
                                          credential_type,
                                          credential_slot,
                                          crb,
                                          credential_data,
                                          credential_modifier_type,
                                          credential_modifier_node_id,
                                          next_credential_type,
                                          next_credential_slot);

  // Test values
  test_credential_values(second_credential_type_node,
                         third_credential_slot_node);
  // We should still have 2 credential type and 3 credential slots since it was the last credential
  check_credentials_node_count(user_id_node, 2, 3);
}

////////////////////////////////////////////////////////////////////////////
// Attribute creation
////////////////////////////////////////////////////////////////////////////
void test_attribute_creation_happy_case()
{
  helper_set_version(1);

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
  helper_set_version(0);

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
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);

  // Lambdas
  auto count_user_node = [](size_t expected_value) {
    auto user_id_node_count
      = cpp_endpoint_id_node.children(ATTRIBUTE(USER_UNIQUE_ID)).size();
    TEST_ASSERT_EQUAL_MESSAGE(expected_value,
                              user_id_node_count,
                              "User ID node count mismatch");
  };

  // Test logic
  attribute_store::attribute cpp_node_id_node(node_id_node);

  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE;
  auto network_status_node
    = cpp_node_id_node.emplace_node(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                    network_status);

  count_user_node(0);

  // Interview state
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING;
  network_status_node.set_reported(network_status);
  count_user_node(0);

  // Simulate one more endpoint and only add nif to one of them

  // Add NIF to our endpoint to tell that we support User Credential
  std::vector<uint8_t> nif_value = {COMMAND_CLASS_USER_CREDENTIAL};
  cpp_endpoint_id_node.emplace_node(ATTRIBUTE_ZWAVE_SECURE_NIF, nif_value);

  // Add another endpoint without User Credential support
  zwave_endpoint_id_t endpoint_id_2 = endpoint_id + 12;
  cpp_node_id_node.emplace_node(ATTRIBUTE_ENDPOINT_ID, endpoint_id_2);

  // Display current store log for debug purpose
  attribute_store_log();

  // Set the network status to online
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  network_status_node.set_reported(network_status);

  // Now we check attribute store
  auto user_id_node = helper_test_attribute_value(
    ATTRIBUTE(USER_UNIQUE_ID),
    static_cast<user_credential_user_unique_id_t>(0),
    cpp_endpoint_id_node,
    DESIRED_ATTRIBUTE);
  count_user_node(1);

  // Simulate user found
  user_credential_user_unique_id_t user_id = 12;
  user_id_node.set_reported<user_credential_user_unique_id_t>(user_id);

  // Go back to Interview state
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING;
  network_status_node.set_reported(network_status);

  // Then back in Online Functional
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  network_status_node.set_reported(network_status);

  // User should still be there
  count_user_node(1);
  // With right value
  helper_test_attribute_value(ATTRIBUTE(USER_UNIQUE_ID), user_id);
}

void test_user_credential_add_credential_already_defined_cred_type_and_slot()
{
  // Initialize the notification callback
  helper_set_version(1);

  user_credential_user_unique_id_t user_id = 12;
  user_credential_type_t credential_type   = ZCL_CRED_TYPE_PIN_CODE;
  user_credential_slot_t credential_slot   = 1;
  std::string credential_data              = "12";
  bool crb                                 = true;
  user_credential_modifier_type_t credential_modifier_type       = 2;
  user_credential_modifier_node_id_t credential_modifier_node_id = 1212;

  // Simulate user
  cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  // Capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_PIN_CODE};
  std::vector<uint8_t> supported_cl                = {1};
  std::vector<uint16_t> supported_credential_slots = {1};
  std::vector<uint8_t> supported_cred_min_length   = {2};
  std::vector<uint8_t> supported_cred_max_length   = {6};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                {1},
                                                {1});

  // Add credential
  sl_status_t status = zwave_command_class_user_credential_add_new_credential(
    endpoint_id_node,
    user_id,
    credential_type,
    credential_slot,
    credential_data.c_str());

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            status,
                            "Credential add should have returned SL_STATUS_OK");

  helper_simulate_credential_report_frame(
    0x00,  // Credential added
    user_id,
    credential_type,
    credential_slot,
    crb,
    string_to_uint8_vector(credential_data),
    credential_modifier_type,
    credential_modifier_node_id,
    0,
    0);

  // Try to add same credential type and slot on same user but with different credential data
  credential_data = "1234";
  status          = zwave_command_class_user_credential_add_new_credential(
    endpoint_id_node,
    user_id,
    credential_type,
    credential_slot,
    credential_data.c_str());

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    status,
    "Credential add should have returned SL_STATUS_FAIL (trying to add same "
    "credential type/slot to same user).");

  // Try to add same credential type/slot on other user
  user_id = 15;
  cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  status = zwave_command_class_user_credential_add_new_credential(
    endpoint_id_node,
    user_id,
    credential_type,
    credential_slot,
    credential_data.c_str());

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    status,
    "Credential add should have returned SL_STATUS_FAIL (trying to add same "
    "credential type/slot to different user).");
}

void test_user_credential_add_credential_invalid_slot()
{
  user_credential_user_unique_id_t user_id = 12;
  user_credential_type_t credential_type   = ZCL_CRED_TYPE_PIN_CODE;
  user_credential_slot_t credential_slot   = 58;
  std::string credential_data              = "12";

  // Simulate user
  cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  // Se capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_PIN_CODE};
  std::vector<uint8_t> supported_cl                = {1};
  std::vector<uint16_t> supported_credential_slots = {1};
  std::vector<uint8_t> supported_cred_min_length   = {2};
  std::vector<uint8_t> supported_cred_max_length   = {6};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                {1},
                                                {1});

  // Add credential
  sl_status_t status = zwave_command_class_user_credential_add_new_credential(
    endpoint_id_node,
    user_id,
    credential_type,
    credential_slot,
    credential_data.c_str());

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    status,
    "Credential add should have returned SL_STATUS_FAIL (trying to add a non "
    "supported slot)");
}

void test_user_credential_user_add_modify_delete_happy_case()
{
  // Those functions are exposed and checks user values, so we need to setup the capabilities
  uint16_t number_of_users                                       = 12;
  user_credential_supported_credential_rules_t cred_rule_bitmask = 0x0F;
  uint8_t username_max_length                                    = 112;
  uint8_t support_user_schedule                                  = 0;
  uint8_t support_all_users_checksum                             = 0;
  uint8_t support_user_checksum                                  = 0;
  uint8_t supported_user_types_bitmask_length                    = 2;
  user_credential_supported_user_type_bitmask_t supported_user_types_bitmask
    = 0xFF;
  helper_simulate_user_capabilites_report(number_of_users,
                                          cred_rule_bitmask,
                                          username_max_length,
                                          support_user_schedule,
                                          support_all_users_checksum,
                                          support_user_checksum,
                                          supported_user_types_bitmask_length,
                                          supported_user_types_bitmask);

  // Setup user
  user_credential_user_unique_id_t user_id                    = 12;
  user_credential_type_t user_type                            = 2;
  user_credential_rule_t credential_rule                      = 1;
  user_credential_user_active_state_t user_active_state       = 1;
  user_credential_expiring_timeout_minutes_t expiring_timeout = 0;
  user_credential_user_name_encoding_t user_name_encoding     = 0;
  std::string user_name                                       = "MICHEL VNR";

  // Add user
  sl_status_t status
    = zwave_command_class_user_credential_add_new_user(endpoint_id_node,
                                                       user_id,
                                                       user_type,
                                                       credential_rule,
                                                       user_active_state,
                                                       expiring_timeout,
                                                       user_name_encoding,
                                                       user_name.c_str());

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            status,
                            "zwave_command_class_user_credential_add_new_user "
                            "should have returned SL_STATUS_OK");
  // Get user node
  auto user_node = helper_test_and_get_node(ATTRIBUTE(USER_UNIQUE_ID));
  auto operation_type_node
    = helper_test_and_get_node(ATTRIBUTE(USER_OPERATION_TYPE), user_node);
  TEST_ASSERT_EQUAL_MESSAGE(
    USER_SET_OPERATION_TYPE_ADD,
    operation_type_node.desired<user_credential_operation_type_t>(),
    "Operation type mismatch for user add");

  zwave_frame set_frame;
  set_frame.add(
    static_cast<user_credential_operation_type_t>(USER_SET_OPERATION_TYPE_ADD));
  set_frame.add(user_id);
  set_frame.add(user_type);
  set_frame.add(user_active_state);
  set_frame.add(credential_rule);
  set_frame.add(expiring_timeout);
  set_frame.add(user_name_encoding);
  set_frame.add(user_name);
  helper_test_get_set_frame_happy_case(USER_SET,
                                       operation_type_node,
                                       set_frame);

  // Create notification report frame
  user_credential_modifier_type_t user_modifier_type       = 2;
  user_credential_modifier_node_id_t user_modifier_node_id = 1212;

  helper_simulate_user_report_frame(USER_SET_OPERATION_TYPE_ADD,
                                    0,  // Next user id
                                    user_modifier_type,
                                    user_modifier_node_id,
                                    user_id,
                                    user_type,
                                    user_active_state,
                                    credential_rule,
                                    expiring_timeout,
                                    user_name_encoding,
                                    user_name);

  // Check values
  auto test_attribute_store_values = [&]() {
    helper_test_attribute_value(ATTRIBUTE(USER_MODIFIER_TYPE),
                                user_modifier_type,
                                user_node);
    helper_test_attribute_value(ATTRIBUTE(USER_TYPE), user_type, user_node);
    helper_test_attribute_value(ATTRIBUTE(USER_ACTIVE_STATE),
                                user_active_state,
                                user_node);
    helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_RULE),
                                credential_rule,
                                user_node);
    helper_test_attribute_value(ATTRIBUTE(USER_MODIFIER_NODE_ID),
                                user_modifier_node_id,
                                user_node);
    helper_test_attribute_value(ATTRIBUTE(USER_EXPIRING_TIMEOUT_MINUTES),
                                expiring_timeout,
                                user_node);

    helper_test_attribute_value(ATTRIBUTE(USER_NAME), user_name, user_node);
  };
  test_attribute_store_values();

  // Now let's modify this user
  user_type          = 7;
  credential_rule    = 2;
  user_active_state  = 0;
  expiring_timeout   = 10;
  user_name_encoding = 2;
  user_name          = "JACKIE CAMION TURBO PLUS";

  status = zwave_command_class_user_credential_modify_user(endpoint_id_node,
                                                           user_id,
                                                           user_type,
                                                           credential_rule,
                                                           user_active_state,
                                                           expiring_timeout,
                                                           user_name_encoding,
                                                           user_name.c_str());
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            status,
                            "zwave_command_class_user_credential_modify_user "
                            "should have returned SL_STATUS_OK");
  TEST_ASSERT_EQUAL_MESSAGE(
    USER_SET_OPERATION_TYPE_MODIFY,
    operation_type_node.desired<user_credential_operation_type_t>(),
    "Operation type mismatch for user modify");

  // Set frame
  set_frame.clear();
  set_frame.add(static_cast<user_credential_operation_type_t>(
    USER_SET_OPERATION_TYPE_MODIFY));
  set_frame.add(user_id);
  set_frame.add(user_type);
  set_frame.add(user_active_state);
  set_frame.add(credential_rule);
  set_frame.add(expiring_timeout);
  set_frame.add(user_name_encoding);
  set_frame.add(user_name);
  helper_test_get_set_frame_happy_case(USER_SET,
                                       operation_type_node,
                                       set_frame);

  // Report back
  helper_simulate_user_report_frame(USER_SET_OPERATION_TYPE_MODIFY,
                                    0,  // Next user id
                                    user_modifier_type,
                                    user_modifier_node_id,
                                    user_id,
                                    user_type,
                                    user_active_state,
                                    credential_rule,
                                    expiring_timeout,
                                    user_name_encoding,
                                    user_name);

  // Create notification report frame
  user_modifier_type    = 5;
  user_modifier_node_id = 12122;

  helper_simulate_user_report_frame(USER_SET_OPERATION_TYPE_MODIFY,
                                    0,  // Next user id
                                    user_modifier_type,
                                    user_modifier_node_id,
                                    user_id,
                                    user_type,
                                    user_active_state,
                                    credential_rule,
                                    expiring_timeout,
                                    user_name_encoding,
                                    user_name);

  // Check values
  test_attribute_store_values();

  // Now let's delete this user
  status = zwave_command_class_user_credential_delete_user(endpoint_id_node,
                                                           user_id);
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            status,
                            "User delete should have returned SL_STATUS_OK");

  TEST_ASSERT_EQUAL_MESSAGE(
    USER_SET_OPERATION_TYPE_DELETE,
    operation_type_node.desired<user_credential_operation_type_t>(),
    "Operation type mismatch for user delete");

  // Set frame
  set_frame.clear();
  set_frame.add(static_cast<user_credential_operation_type_t>(
    USER_SET_OPERATION_TYPE_DELETE));
  set_frame.add(user_id);
  helper_test_get_set_frame_happy_case(USER_SET,
                                       operation_type_node,
                                       set_frame);

  helper_simulate_user_report_frame(USER_SET_OPERATION_TYPE_DELETE,
                                    0,  // Next user id
                                    user_modifier_type,
                                    user_modifier_node_id,
                                    user_id,
                                    user_type,
                                    user_active_state,
                                    credential_rule,
                                    expiring_timeout,
                                    user_name_encoding,
                                    user_name);
}

void test_user_credential_credential_add_modify_delete_happy_case()
{
  helper_set_version(1);

  user_credential_user_unique_id_t user_id = 12;
  user_credential_type_t credential_type   = ZCL_CRED_TYPE_PIN_CODE;
  user_credential_slot_t credential_slot   = 1;
  bool crb                                 = true;
  std::string credential_data              = "12";
  user_credential_modifier_type_t credential_modifier_type       = 2;
  user_credential_modifier_node_id_t credential_modifier_node_id = 1212;

  // Simulate user
  auto user_id_node
    = cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  // Se capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_PIN_CODE};
  std::vector<uint8_t> supported_cl                = {1};
  std::vector<uint16_t> supported_credential_slots = {1};
  std::vector<uint8_t> supported_cred_min_length   = {2};
  std::vector<uint8_t> supported_cred_max_length   = {6};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                {1},
                                                {1});

  // Add credential
  sl_status_t status = zwave_command_class_user_credential_add_new_credential(
    endpoint_id_node,
    user_id,
    credential_type,
    credential_slot,
    credential_data.c_str());

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            status,
                            "Credential add should have returned SL_STATUS_OK");

  // Get credential type
  auto credential_type_node
    = user_id_node.child_by_type(ATTRIBUTE(CREDENTIAL_TYPE));

  TEST_ASSERT_TRUE_MESSAGE(credential_type_node.is_valid(),
                           "Credential type node should exist");

  // Get credential slot
  auto credential_slot_node
    = credential_type_node.child_by_type(ATTRIBUTE(CREDENTIAL_SLOT));

  TEST_ASSERT_TRUE_MESSAGE(credential_slot_node.is_valid(),
                           "Credential slot node should exist");

  // Create notification report frame
  helper_simulate_credential_report_frame(
    0x00,  // Credential add
    user_id,
    credential_type,
    credential_slot,
    crb,
    string_to_uint8_vector(credential_data),
    credential_modifier_type,
    credential_modifier_node_id,
    0,
    0);

  // Check values
  auto test_attribute_store_values = [&]() {
    helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_TYPE),
                                credential_type,
                                user_id_node);
    helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_SLOT),
                                credential_slot,
                                credential_type_node);
    helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_MODIFIER_TYPE),
                                credential_modifier_type,
                                credential_slot_node);
    helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_MODIFIER_NODE_ID),
                                credential_modifier_node_id,
                                credential_slot_node);
  };
  test_attribute_store_values();

  crb                         = false;
  credential_data             = "121212";
  credential_modifier_type    = 3;
  credential_modifier_node_id = 15;

  // Modify credential
  helper_simulate_credential_report_frame(
    0x01,  // Credential modify
    user_id,
    credential_type,
    credential_slot,
    crb,
    string_to_uint8_vector(credential_data),
    credential_modifier_type,
    credential_modifier_node_id,
    0,
    0);

  test_attribute_store_values();

  helper_simulate_credential_report_frame(
    0x02,  // Credential remove
    user_id,
    credential_type,
    credential_slot,
    0,
    string_to_uint8_vector(credential_data),
    credential_modifier_type,
    credential_modifier_node_id,
    0,
    0);
  TEST_ASSERT_FALSE_MESSAGE(credential_slot_node.is_valid(),
                            "Credential slot node should be deleted");
  TEST_ASSERT_TRUE_MESSAGE(credential_type_node.is_valid(),
                           "Credential type node should still exist");
  TEST_ASSERT_TRUE_MESSAGE(user_id_node.is_valid(),
                           "User ID node should still exist");
}

void test_user_credential_user_add_capabilites_failure_cases()
{
  // Initialize the notification callback
  helper_set_version(1);

  // Those functions are exposed and checks user values, so we need to setup the capabilities
  uint16_t number_of_users                                       = 1;
  user_credential_supported_credential_rules_t cred_rule_bitmask = 2;
  uint8_t username_max_length                                    = 1;
  uint8_t support_user_schedule                                  = 0;
  uint8_t support_all_users_checksum                             = 0;
  uint8_t support_user_checksum                                  = 0;
  uint8_t supported_user_types_bitmask_length                    = 1;
  user_credential_supported_user_type_bitmask_t supported_user_types_bitmask
    = 1;
  helper_simulate_user_capabilites_report(number_of_users,
                                          cred_rule_bitmask,
                                          username_max_length,
                                          support_user_schedule,
                                          support_all_users_checksum,
                                          support_user_checksum,
                                          supported_user_types_bitmask_length,
                                          supported_user_types_bitmask);

  // User ID not valid
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    zwave_command_class_user_credential_add_new_user(endpoint_id_node,
                                                     12,  // User ID
                                                     0,   // User Type
                                                     1,   // Credential rule
                                                     1,
                                                     0,
                                                     0,
                                                     "G"),
    "User ID is not valid");

  // User Type not valid
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    zwave_command_class_user_credential_add_new_user(endpoint_id_node,
                                                     1,  // User ID
                                                     1,  // User Type
                                                     1,  // Credential rule
                                                     1,
                                                     0,
                                                     0,
                                                     "G"),
    "User Type is not valid");

  // Credential rule not valid
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    zwave_command_class_user_credential_add_new_user(endpoint_id_node,
                                                     1,  // User ID
                                                     0,  // User Type
                                                     2,  // Credential rule
                                                     1,
                                                     0,
                                                     0,
                                                     "G"),
    "Credential rule is not valid");

  // Username too long
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    zwave_command_class_user_credential_add_new_user(endpoint_id_node,
                                                     1,  // User ID
                                                     0,  // User Type
                                                     1,  // Credential rule
                                                     1,
                                                     0,
                                                     0,
                                                     "GERARD TURBO"),
    "User name is not valid");
}

void test_user_credential_user_add_capabilites_happy_cred_rule_cases()
{
  helper_set_version(1);

  uint16_t number_of_users                    = 1;
  uint8_t username_max_length                 = 1;
  uint8_t support_user_schedule               = 0;
  uint8_t support_all_users_checksum          = 0;
  uint8_t support_user_checksum               = 0;
  uint8_t supported_user_types_bitmask_length = 1;
  user_credential_supported_user_type_bitmask_t supported_user_types_bitmask
    = 1;

  std::map<user_credential_supported_credential_rules_t, uint8_t> cred_rule_map
    = {
      {0x02, 1},
      {0x04, 2},
      {0x08, 3},
      {0x06, 1},
      {0x06, 2},
      {0x0F, 3},
      {0x0F, 2},
      {0x0F, 1},
    };

  for (auto &cred_rule: cred_rule_map) {
    helper_simulate_user_capabilites_report(number_of_users,
                                            cred_rule.first,
                                            username_max_length,
                                            support_user_schedule,
                                            support_all_users_checksum,
                                            support_user_checksum,
                                            supported_user_types_bitmask_length,
                                            supported_user_types_bitmask);

    printf("Testing bitmask %d with value %d\n",
           cred_rule.first,
           cred_rule.second);
    TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                              zwave_command_class_user_credential_add_new_user(
                                endpoint_id_node,
                                1,                 // User ID
                                0,                 // User Type
                                cred_rule.second,  // Credential rule
                                1,
                                0,
                                0,
                                "G"),
                              "Should be able to add user");
  }
}

void test_user_credential_user_add_capabilites_happy_user_type_rule_cases()
{
  helper_set_version(1);

  uint16_t number_of_users                               = 1;
  uint8_t username_max_length                            = 1;
  uint8_t support_user_schedule                          = 0;
  uint8_t support_all_users_checksum                     = 0;
  uint8_t support_user_checksum                          = 0;
  uint8_t supported_user_types_bitmask_length            = 1;
  user_credential_supported_credential_rules_t cred_rule = 0x02;

  std::map<user_credential_supported_user_type_bitmask_t, uint8_t> type_rule_map
    = {
      {0x01, 0},
      {0x02, 1},
      {0x04, 2},
      {0x08, 3},
      {0x06, 1},
      {0x06, 2},
      {0x0F, 3},
      {0x0F, 2},
      {0x0F, 1},
    };

  for (auto &type_rule: type_rule_map) {
    helper_simulate_user_capabilites_report(number_of_users,
                                            cred_rule,
                                            username_max_length,
                                            support_user_schedule,
                                            support_all_users_checksum,
                                            support_user_checksum,
                                            supported_user_types_bitmask_length,
                                            type_rule.first);

    printf("Testing bitmask %d with value %d\n",
           type_rule.first,
           type_rule.second);
    TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                              zwave_command_class_user_credential_add_new_user(
                                endpoint_id_node,
                                1,                 // User ID
                                type_rule.second,  // User Type
                                1,                 // Credential rule
                                1,
                                0,
                                0,
                                "G"),
                              "Should be able to add user");
  }
}

void test_user_credential_user_modify_capabilites_failure_cases()
{
  // Initialize the notification callback
  helper_set_version(1);

  // Those functions are exposed and checks user values, so we need to setup the capabilities
  uint16_t number_of_users                                       = 1;
  user_credential_supported_credential_rules_t cred_rule_bitmask = 2;
  uint8_t username_max_length                                    = 1;
  uint8_t support_user_schedule                                  = 0;
  uint8_t support_all_users_checksum                             = 0;
  uint8_t support_user_checksum                                  = 0;
  uint8_t supported_user_types_bitmask_length                    = 1;
  user_credential_supported_user_type_bitmask_t supported_user_types_bitmask
    = 1;
  helper_simulate_user_capabilites_report(number_of_users,
                                          cred_rule_bitmask,
                                          username_max_length,
                                          support_user_schedule,
                                          support_all_users_checksum,
                                          support_user_checksum,
                                          supported_user_types_bitmask_length,
                                          supported_user_types_bitmask);

  // User ID not valid
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    zwave_command_class_user_credential_add_new_user(endpoint_id_node,
                                                     1,  // User ID
                                                     0,  // User Type
                                                     1,  // Credential rule
                                                     1,
                                                     0,
                                                     0,
                                                     "G"),
    "Should be able to add user");

  // User Type not valid
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    zwave_command_class_user_credential_modify_user(endpoint_id_node,
                                                    1,  // User ID
                                                    1,  // User Type
                                                    1,  // Credential rule
                                                    1,
                                                    0,
                                                    0,
                                                    "G"),
    "User Type is not valid");

  // Credential rule not valid
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    zwave_command_class_user_credential_modify_user(endpoint_id_node,
                                                    1,  // User ID
                                                    0,  // User Type
                                                    2,  // Credential rule
                                                    1,
                                                    0,
                                                    0,
                                                    "G"),
    "Credential rule is not valid");

  // Username too long
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    zwave_command_class_user_credential_modify_user(endpoint_id_node,
                                                    1,  // User ID
                                                    0,  // User Type
                                                    1,  // Credential rule
                                                    1,
                                                    0,
                                                    0,
                                                    "GERARD TURBO"),
    "User name is not valid");
}

void test_user_credential_credential_add_capabilites_failure_cases()
{
  // Initialize the notification callback
  helper_set_version(1);

  user_credential_user_unique_id_t user_id = 12;

  // Simulate user
  attribute_store_emplace(endpoint_id_node,
                          ATTRIBUTE(USER_UNIQUE_ID),
                          &user_id,
                          sizeof(user_id));

  // Se capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_HAND_BIOMETRIC};
  std::vector<uint8_t> supported_cl                = {1};
  std::vector<uint16_t> supported_credential_slots = {1};
  std::vector<uint8_t> supported_cred_min_length   = {2};
  std::vector<uint8_t> supported_cred_max_length   = {6};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                {1},
                                                {1});

  // Credential type not valid
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    zwave_command_class_user_credential_add_new_credential(
      endpoint_id_node,
      user_id,
      ZCL_CRED_TYPE_PIN_CODE,  // Credential type
      1,                       // Credential slot
      "12"),
    "Credential type shouldn't be valid");

  // Credential slot not valid
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    zwave_command_class_user_credential_add_new_credential(
      endpoint_id_node,
      user_id,
      ZCL_CRED_TYPE_HAND_BIOMETRIC,
      2,  // Credential slot
      "12"),
    "Credential slot shouldn't be valid");

  // Credential data too short
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    zwave_command_class_user_credential_add_new_credential(
      endpoint_id_node,
      user_id,
      ZCL_CRED_TYPE_HAND_BIOMETRIC,
      1,
      "1"),
    "Credential data should be too short");
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    zwave_command_class_user_credential_add_new_credential(
      endpoint_id_node,
      user_id,
      ZCL_CRED_TYPE_HAND_BIOMETRIC,
      1,
      "TURBO TROP LONG VROUM"),
    "Credential data should be too long");
}

void test_user_credential_credential_add_capabilites_happy_case()
{
  // Initialize the notification callback
  helper_set_version(1);

  user_credential_user_unique_id_t user_id = 12;

  // Simulate user
  cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  // Se capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_HAND_BIOMETRIC, ZCL_CRED_TYPE_PIN_CODE};
  std::vector<uint8_t> supported_cl                = {1, 0};
  std::vector<uint16_t> supported_credential_slots = {1, 5};
  std::vector<uint8_t> supported_cred_min_length   = {2, 4};
  std::vector<uint8_t> supported_cred_max_length   = {6, 8};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                {1, 1},
                                                {1, 1});

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    zwave_command_class_user_credential_add_new_credential(
      endpoint_id_node,
      user_id,
      ZCL_CRED_TYPE_HAND_BIOMETRIC,
      1,
      "TURBO"),
    "Credential #1 should be valid");

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    zwave_command_class_user_credential_add_new_credential(
      endpoint_id_node,
      user_id,
      ZCL_CRED_TYPE_PIN_CODE,
      4,
      "121212"),
    "Credential #2 should be valid");
}

void test_user_credential_credential_modify_capabilites_failure_cases()
{
  // Initialize the notification callback
  helper_set_version(1);

  user_credential_user_unique_id_t user_id = 12;

  // Simulate user
  cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  // Se capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_HAND_BIOMETRIC};
  std::vector<uint8_t> supported_cl                = {1};
  std::vector<uint16_t> supported_credential_slots = {1};
  std::vector<uint8_t> supported_cred_min_length   = {2};
  std::vector<uint8_t> supported_cred_max_length   = {7};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                {1},
                                                {1});
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    zwave_command_class_user_credential_add_new_credential(
      endpoint_id_node,
      user_id,
      ZCL_CRED_TYPE_HAND_BIOMETRIC,
      1,
      "VOITURE"),
    "Should be able to add credential");

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    zwave_command_class_user_credential_modify_credential(
      endpoint_id_node,
      user_id,
      ZCL_CRED_TYPE_HAND_BIOMETRIC,
      1,
      "V"),
    "Should not be able to modify credential data : it's too short");

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    zwave_command_class_user_credential_modify_credential(
      endpoint_id_node,
      user_id,
      ZCL_CRED_TYPE_HAND_BIOMETRIC,
      1,
      "MAX SPEEEEEEEEEEEEED TURBO"),
    "Should not be able to modify credential data : it's too long");
}

void helper_test_credential_rejected_case(uint8_t report_type)
{
  user_credential_user_unique_id_t user_id       = 12;
  user_credential_type_t credential_type         = 1;
  user_credential_slot_t credential_slot         = 1;
  user_credential_slot_t invalid_credential_slot = 2;
  auto credential_data = string_to_uint8_vector("1212");

  auto valid_user_node
    = cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);
  auto valid_cred_type_node
    = valid_user_node.emplace_node(ATTRIBUTE(CREDENTIAL_TYPE), credential_type);
  auto valid_cred_slot_node
    = valid_cred_type_node.emplace_node(ATTRIBUTE(CREDENTIAL_SLOT),
                                        credential_slot,
                                        REPORTED_ATTRIBUTE);
  auto invalid_cred_slot_node
    = valid_cred_type_node.emplace_node(ATTRIBUTE(CREDENTIAL_SLOT),
                                        invalid_credential_slot,
                                        DESIRED_ATTRIBUTE);
  invalid_cred_slot_node.emplace_node(ATTRIBUTE(CREDENTIAL_DATA),
                                      credential_data,
                                      DESIRED_ATTRIBUTE);

  helper_simulate_credential_report_frame(
    report_type,
    user_id,
    credential_type,
    // Duplicate credential send the original slot
    report_type == 0x07 ? credential_slot : invalid_credential_slot,
    1,
    credential_data,
    0,
    2,
    0,
    0);

  // Here the Credential Report command should remove this not this report
  TEST_ASSERT_FALSE_MESSAGE(invalid_cred_slot_node.is_valid(),
                            "Invalid Credential Slot node should NOT exist");
  TEST_ASSERT_TRUE_MESSAGE(valid_cred_slot_node.is_valid(),
                           "Valid Credential Slot node SHOULD exist");
  TEST_ASSERT_TRUE_MESSAGE(valid_cred_type_node.is_valid(),
                           "Valid User node should exist");
  TEST_ASSERT_TRUE_MESSAGE(valid_user_node.is_valid(),
                           "Valid Credential type node should exist");
}

void test_user_credential_credential_set_error_report_cred_add_happy_case()
{
  helper_test_credential_rejected_case(0x05);
}

void test_user_credential_credential_set_error_report_cred_modify_happy_case()
{
  // Common case
  helper_test_credential_rejected_case(0x06);

  // See if that works even without credential type
  cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), 12);
  helper_simulate_credential_report_frame(0x06,
                                          12,
                                          5,
                                          1,
                                          1,
                                          string_to_uint8_vector("1212"),
                                          0,
                                          0,
                                          0,
                                          0);
}

void test_user_credential_credential_set_error_report_cred_duplicate_happy_case()
{
  helper_test_credential_rejected_case(0x07);
}

void test_user_credential_credential_set_error_report_cred_security_rule_add_happy_case()
{
  helper_test_credential_rejected_case(0x08);
}

void test_user_credential_credential_set_error_report_cred_security_rule_modify_happy_case()
{
  user_credential_user_unique_id_t user_id = 12;
  user_credential_type_t credential_type   = 1;
  user_credential_slot_t credential_slot   = 1;
  uint8_t crb                              = 1;
  auto credential_data                     = string_to_uint8_vector("1212");

  auto valid_user_node
    = cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);
  auto valid_cred_type_node
    = valid_user_node.emplace_node(ATTRIBUTE(CREDENTIAL_TYPE), credential_type);
  auto valid_cred_slot_node
    = valid_cred_type_node.emplace_node(ATTRIBUTE(CREDENTIAL_SLOT),
                                        credential_slot,
                                        REPORTED_ATTRIBUTE);

  auto crb_node
    = valid_cred_slot_node.emplace_node(ATTRIBUTE(CREDENTIAL_READ_BACK),
                                        crb,
                                        DESIRED_ATTRIBUTE);
  auto credential_data_node
    = valid_cred_slot_node.emplace_node(ATTRIBUTE(CREDENTIAL_DATA),
                                        credential_data,
                                        DESIRED_ATTRIBUTE);

  helper_simulate_credential_report_frame(0x08,
                                          user_id,
                                          credential_type,
                                          credential_slot,
                                          crb,
                                          credential_data,
                                          0,
                                          2,
                                          0,
                                          0);

  // Check if the rejected report has cleared all desired values
  TEST_ASSERT_FALSE_MESSAGE(crb_node.desired_exists(),
                            "CRB desired value should NOT exist");

  TEST_ASSERT_FALSE_MESSAGE(credential_data_node.desired_exists(),
                            "Credential data desired value should NOT exist");
}

void test_user_credential_remove_all_users_happy_case()
{
  helper_set_version(1);

  std::vector<user_credential_user_unique_id_t> user_ids = {0, 12, 15, 19};
  for (auto user_id: user_ids) {
    cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);
  }

  auto status
    = zwave_command_class_user_credential_delete_all_users(endpoint_id_node);

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    status,
    "Remove all users should have returned SL_STATUS_OK");

  TEST_ASSERT_EQUAL_MESSAGE(
    user_ids.size(),
    cpp_endpoint_id_node.children(ATTRIBUTE(USER_UNIQUE_ID)).size(),
    "All users should have NOT have been removed yet");

  // This should remove all users
  helper_simulate_user_report_frame(0x02,  //  Delete user
                                    0,     // Next user id
                                    0,     // User Type
                                    0,     // User Node ID
                                    0,     // User ID
                                    0,     // User Type
                                    0,     // User Active State
                                    0,     // Credential Rule
                                    0,     // Expiring Timeout
                                    0,     // User Name Encoding
                                    "");   // User Name

  TEST_ASSERT_EQUAL_MESSAGE(
    1,  // User with ID 0 that might contains credentials
    cpp_endpoint_id_node.children(ATTRIBUTE(USER_UNIQUE_ID)).size(),
    "Only user 0 should be left");

  helper_test_attribute_value(ATTRIBUTE(USER_UNIQUE_ID),
                              static_cast<user_credential_user_unique_id_t>(0));
}

// TODO : to be refactored
void test_user_credential_remove_all_credentials_happy_case()
{
  helper_set_version(1);

  std::vector<credential_structure_nodes> expected_nodes;
  // WARNING : All those vectors should be the same size
  std::vector<user_credential_user_unique_id_t> user_ids = {12, 12, 12, 15, 19};
  std::vector<user_credential_type_t> credential_types   = {1, 1, 2, 5, 1};
  std::vector<user_credential_slot_t> credential_slots   = {1, 2, 2, 1, 3};
  // WARNING : Make sure that all the vector above have the same size
  const size_t expected_credential_count = user_ids.size();
  for (size_t i = 0; i < expected_credential_count; i++) {
    expected_nodes.push_back(
      helper_create_credential_structure(user_ids[i],
                                         credential_types[i],
                                         credential_slots[i],
                                         REPORTED_ATTRIBUTE));
  }

  auto status = zwave_command_class_user_credential_delete_all_credentials(
    endpoint_id_node);

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    status,
    "Remove all credentials should have returned SL_STATUS_OK");

  helper_simulate_credential_report_frame(0x02, 0, 0, 0, 0, {}, 0, 0, 0, 0);

  for (auto &node: expected_nodes) {
    TEST_ASSERT_FALSE_MESSAGE(node.credential_type_node.is_valid(),
                              "Credential type node should have been removed");
    TEST_ASSERT_FALSE_MESSAGE(node.credential_slot_node.is_valid(),
                              "Credential slot node should have been removed");
  }
}

/**
 * @brief Test the removal of credentials of a user
 * 
 * @param nodes Nodes to tests
 * @param deletion_condition If true will test if credential types and slots have been removed, otherwise will test if they still exist
 * 
 */
void helper_test_credential_structure_exists(credential_structure_nodes nodes,
                                             bool deletion_condition)
{
  if (deletion_condition) {
    TEST_ASSERT_FALSE_MESSAGE(nodes.credential_type_node.is_valid(),
                              "Credential type node should have been removed");
    TEST_ASSERT_FALSE_MESSAGE(nodes.credential_slot_node.is_valid(),
                              "Credential slot node should have been removed");
  } else {
    TEST_ASSERT_TRUE_MESSAGE(nodes.credential_type_node.is_valid(),
                             "Credential type node should still exist");
    TEST_ASSERT_TRUE_MESSAGE(nodes.credential_slot_node.is_valid(),
                             "Credential slot node should still exist");
  }
}

void test_user_credential_remove_all_credentials_of_user()
{
  helper_set_version(1);

  user_credential_user_unique_id_t user_to_delete = 12;

  std::vector<credential_structure_nodes> expected_nodes;
  // WARNING : All those vectors should be the same size
  std::vector<user_credential_user_unique_id_t> user_ids
    = {user_to_delete, user_to_delete, user_to_delete, 15, 19};
  std::vector<user_credential_type_t> credential_types = {1, 1, 2, 5, 1};
  std::vector<user_credential_slot_t> credential_slots = {1, 2, 2, 1, 3};
  // WARNING : Make sure that all the vector above have the same size
  const size_t expected_credential_count = user_ids.size();
  for (size_t i = 0; i < expected_credential_count; i++) {
    expected_nodes.push_back(
      helper_create_credential_structure(user_ids[i],
                                         credential_types[i],
                                         credential_slots[i],
                                         REPORTED_ATTRIBUTE));
  }

  auto status
    = zwave_command_class_user_credential_delete_all_credentials_for_user(
      endpoint_id_node,
      user_to_delete);

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    status,
    "Remove all credentials for user should have returned SL_STATUS_OK");

  helper_simulate_credential_report_frame(0x02,
                                          user_to_delete,
                                          0,
                                          0,
                                          0,
                                          {},
                                          0,
                                          0,
                                          0,
                                          0);

  for (auto &node: expected_nodes) {
    auto reported_user_id
      = node.user_id_node.reported<user_credential_user_unique_id_t>();
    node.print();

    helper_test_credential_structure_exists(node,
                                            reported_user_id == user_to_delete);
  }
}

struct expected_node_data {
  credential_structure_nodes nodes;
  bool deletion_condition;
};
void test_user_credential_remove_all_credentials_of_user_by_type()
{
  helper_set_version(1);

  user_credential_user_unique_id_t user_to_delete  = 12;
  user_credential_type_t credential_type_to_delete = 1;

  std::vector<expected_node_data> expected_nodes_data;
  // WARNING : All those vectors should be the same size
  std::vector<user_credential_user_unique_id_t> user_ids
    = {user_to_delete, user_to_delete, user_to_delete, 15, 19};
  std::vector<user_credential_type_t> credential_types
    = {credential_type_to_delete, credential_type_to_delete, 2, 5, 1};
  std::vector<user_credential_slot_t> credential_slots = {1, 2, 2, 1, 3};
  // WARNING : Make sure that all the vector above have the same size
  const size_t expected_credential_count = user_ids.size();
  for (size_t i = 0; i < expected_credential_count; i++) {
    expected_nodes_data.push_back(
      {helper_create_credential_structure(user_ids[i],
                                          credential_types[i],
                                          credential_slots[i],
                                          REPORTED_ATTRIBUTE),
       user_ids[i] == user_to_delete
         && credential_types[i] == credential_type_to_delete});
  }

  auto status
    = zwave_command_class_user_credential_delete_all_credentials_for_user_by_type(
      endpoint_id_node,
      user_to_delete,
      credential_type_to_delete);

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    status,
    "Remove all credentials for user should have returned SL_STATUS_OK");

  helper_simulate_credential_report_frame(0x02,
                                          user_to_delete,
                                          credential_type_to_delete,
                                          0,
                                          0,
                                          {},
                                          0,
                                          0,
                                          0,
                                          0);

  for (auto &node_data: expected_nodes_data) {
    auto nodes              = node_data.nodes;
    auto deletion_condition = node_data.deletion_condition;

    nodes.print();
    helper_test_credential_structure_exists(nodes, deletion_condition);
  }
}

void test_user_credential_remove_all_credentials_by_type()
{
  helper_set_version(1);

  user_credential_type_t credential_type_to_delete = 1;

  std::vector<expected_node_data> expected_nodes_data;
  // WARNING : All those vectors should be the same size
  std::vector<user_credential_user_unique_id_t> user_ids = {12, 12, 12, 15, 19};
  std::vector<user_credential_type_t> credential_types
    = {credential_type_to_delete,
       credential_type_to_delete,
       2,
       5,
       credential_type_to_delete};
  std::vector<user_credential_slot_t> credential_slots = {1, 2, 2, 1, 3};
  // WARNING : Make sure that all the vector above have the same size
  const size_t expected_credential_count = user_ids.size();
  for (size_t i = 0; i < expected_credential_count; i++) {
    expected_nodes_data.push_back(
      {helper_create_credential_structure(user_ids[i],
                                          credential_types[i],
                                          credential_slots[i],
                                          REPORTED_ATTRIBUTE),
       credential_types[i] == credential_type_to_delete});
  }

  auto status
    = zwave_command_class_user_credential_delete_all_credentials_by_type(
      endpoint_id_node,
      credential_type_to_delete);

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    status,
    "Remove all credentials for user should have returned SL_STATUS_OK");

  helper_simulate_credential_report_frame(0x02,
                                          0,
                                          credential_type_to_delete,
                                          0,
                                          0,
                                          {},
                                          0,
                                          0,
                                          0,
                                          0);

  for (auto &node_data: expected_nodes_data) {
    auto nodes              = node_data.nodes;
    auto deletion_condition = node_data.deletion_condition;

    nodes.print();
    helper_test_credential_structure_exists(nodes, deletion_condition);
  }
}

void test_user_credential_credential_learn_start_add_happy_case()
{
  user_credential_user_unique_id_t user_id   = 12;
  user_credential_type_t credential_type     = ZCL_CRED_TYPE_PIN_CODE;
  user_credential_slot_t credential_slot     = 1;
  user_credential_learn_timeout_t cl_timeout = 0;
  // Simulate user
  auto user_id_node
    = cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  // Set capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_PIN_CODE, ZCL_CRED_TYPE_HAND_BIOMETRIC};
  std::vector<uint8_t> supported_cl                = {1, 1};
  std::vector<uint16_t> supported_credential_slots = {1, 2};
  std::vector<uint8_t> supported_cred_min_length   = {2, 5};
  std::vector<uint8_t> supported_cred_max_length   = {6, 10};
  std::vector<uint8_t> supported_cl_timeout        = {20, 2};
  std::vector<uint8_t> supported_cl_steps          = {1, 95};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                supported_cl_timeout,
                                                supported_cl_steps);

  // Add credential
  // Test if we give 0 as timeout, it should be set to default value
  sl_status_t status
    = zwave_command_class_user_credential_credential_learn_start_add(
      endpoint_id_node,
      user_id,
      credential_type,
      credential_slot,
      cl_timeout);
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            status,
                            "Should be able to start learning");

  attribute_store::attribute credential_type_node;
  attribute_store::attribute credential_slot_node;
  auto update_credential_nodes = [&]() {
    credential_type_node
      = user_id_node.child_by_type_and_value(ATTRIBUTE(CREDENTIAL_TYPE),
                                             credential_type);

    credential_slot_node
      = credential_type_node.child_by_type_and_value(ATTRIBUTE(CREDENTIAL_SLOT),
                                                     credential_slot);
  };

  update_credential_nodes();
  helper_test_credential_learn_structure(user_id_node,
                                         credential_type_node,
                                         credential_slot_node,
                                         supported_cl_timeout[0],
                                         USER_CREDENTIAL_OPERATION_TYPE_ADD);

  // Test with specific timeout
  credential_type = ZCL_CRED_TYPE_HAND_BIOMETRIC;
  credential_slot = 1;
  cl_timeout      = 100;

  status = zwave_command_class_user_credential_credential_learn_start_add(
    endpoint_id_node,
    user_id,
    credential_type,
    credential_slot,
    cl_timeout);
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            status,
                            "Should be able to start learning");

  update_credential_nodes();
  helper_test_credential_learn_structure(user_id_node,
                                         credential_type_node,
                                         credential_slot_node,
                                         cl_timeout,
                                         USER_CREDENTIAL_OPERATION_TYPE_ADD);
}

void test_user_credential_credential_learn_start_add_cl_not_supported()
{
  user_credential_user_unique_id_t user_id   = 12;
  user_credential_type_t credential_type     = ZCL_CRED_TYPE_PIN_CODE;
  user_credential_slot_t credential_slot     = 1;
  user_credential_learn_timeout_t cl_timeout = 0;
  // Simulate user
  cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  // Set capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_PIN_CODE, ZCL_CRED_TYPE_HAND_BIOMETRIC};
  std::vector<uint8_t> supported_cl                = {0, 1};
  std::vector<uint16_t> supported_credential_slots = {1, 2};
  std::vector<uint8_t> supported_cred_min_length   = {2, 5};
  std::vector<uint8_t> supported_cred_max_length   = {6, 10};
  std::vector<uint8_t> supported_cl_timeout        = {0, 2};
  std::vector<uint8_t> supported_cl_steps          = {0, 95};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                supported_cl_timeout,
                                                supported_cl_steps);

  sl_status_t status
    = zwave_command_class_user_credential_credential_learn_start_add(
      endpoint_id_node,
      user_id,
      credential_type,
      credential_slot,
      cl_timeout);
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_FAIL,
                            status,
                            "Should not be able to start learning");
}

void test_user_credential_credential_learn_start_add_slot_not_supported()
{
  user_credential_user_unique_id_t user_id   = 12;
  user_credential_type_t credential_type     = ZCL_CRED_TYPE_PIN_CODE;
  user_credential_slot_t credential_slot     = 6;
  user_credential_learn_timeout_t cl_timeout = 0;
  // Simulate user
  cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  // Set capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_PIN_CODE, ZCL_CRED_TYPE_HAND_BIOMETRIC};
  std::vector<uint8_t> supported_cl                = {1, 1};
  std::vector<uint16_t> supported_credential_slots = {1, 2};
  std::vector<uint8_t> supported_cred_min_length   = {2, 5};
  std::vector<uint8_t> supported_cred_max_length   = {6, 10};
  std::vector<uint8_t> supported_cl_timeout        = {10, 2};
  std::vector<uint8_t> supported_cl_steps          = {1, 95};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                supported_cl_timeout,
                                                supported_cl_steps);

  sl_status_t status
    = zwave_command_class_user_credential_credential_learn_start_add(
      endpoint_id_node,
      user_id,
      credential_type,
      credential_slot,
      cl_timeout);
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_FAIL,
                            status,
                            "Should not be able to start learning");
}

void test_user_credential_credential_learn_start_add_type_not_supported()
{
  user_credential_user_unique_id_t user_id   = 12;
  user_credential_type_t credential_type     = ZCL_CRED_TYPE_BLE;
  user_credential_slot_t credential_slot     = 1;
  user_credential_learn_timeout_t cl_timeout = 0;
  // Simulate user
  cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  // Set capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_PIN_CODE, ZCL_CRED_TYPE_HAND_BIOMETRIC};
  std::vector<uint8_t> supported_cl                = {1, 1};
  std::vector<uint16_t> supported_credential_slots = {1, 2};
  std::vector<uint8_t> supported_cred_min_length   = {2, 5};
  std::vector<uint8_t> supported_cred_max_length   = {6, 10};
  std::vector<uint8_t> supported_cl_timeout        = {10, 2};
  std::vector<uint8_t> supported_cl_steps          = {1, 95};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                supported_cl_timeout,
                                                supported_cl_steps);

  sl_status_t status
    = zwave_command_class_user_credential_credential_learn_start_add(
      endpoint_id_node,
      user_id,
      credential_type,
      credential_slot,
      cl_timeout);
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_FAIL,
                            status,
                            "Should not be able to start learning");
}

void test_user_credential_credential_learn_start_add_credential_already_exists()
{
  user_credential_user_unique_id_t user_id   = 12;
  user_credential_type_t credential_type     = ZCL_CRED_TYPE_PIN_CODE;
  user_credential_slot_t credential_slot     = 1;
  user_credential_learn_timeout_t cl_timeout = 0;
  // Simulate user
  cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  // Simulate already existing credential
  user_id = 15;
  helper_create_credential_structure(user_id,
                                     credential_type,
                                     credential_slot,
                                     REPORTED_ATTRIBUTE);

  // Set capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_PIN_CODE, ZCL_CRED_TYPE_HAND_BIOMETRIC};
  std::vector<uint8_t> supported_cl                = {1, 1};
  std::vector<uint16_t> supported_credential_slots = {1, 2};
  std::vector<uint8_t> supported_cred_min_length   = {2, 5};
  std::vector<uint8_t> supported_cred_max_length   = {6, 10};
  std::vector<uint8_t> supported_cl_timeout        = {10, 2};
  std::vector<uint8_t> supported_cl_steps          = {1, 95};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                supported_cl_timeout,
                                                supported_cl_steps);

  sl_status_t status
    = zwave_command_class_user_credential_credential_learn_start_add(
      endpoint_id_node,
      user_id,
      credential_type,
      credential_slot,
      cl_timeout);
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_FAIL,
                            status,
                            "Should not be able to start learning");
}

void test_user_credential_credential_learn_start_modify_happy_case()
{
  user_credential_user_unique_id_t user_id   = 12;
  user_credential_type_t credential_type     = ZCL_CRED_TYPE_PIN_CODE;
  user_credential_slot_t credential_slot     = 1;
  user_credential_learn_timeout_t cl_timeout = 0;
  // Simulate user
  auto credential_nodes
    = helper_create_credential_structure(user_id,
                                         credential_type,
                                         credential_slot,
                                         REPORTED_ATTRIBUTE);

  // Set capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_PIN_CODE, ZCL_CRED_TYPE_HAND_BIOMETRIC};
  std::vector<uint8_t> supported_cl                = {1, 1};
  std::vector<uint16_t> supported_credential_slots = {1, 2};
  std::vector<uint8_t> supported_cred_min_length   = {2, 5};
  std::vector<uint8_t> supported_cred_max_length   = {6, 10};
  std::vector<uint8_t> supported_cl_timeout        = {20, 2};
  std::vector<uint8_t> supported_cl_steps          = {1, 95};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                supported_cl_timeout,
                                                supported_cl_steps);

  // Add credential
  // Test if we give 0 as timeout, it should be set to default value
  sl_status_t status
    = zwave_command_class_user_credential_credential_learn_start_modify(
      endpoint_id_node,
      user_id,
      credential_type,
      credential_slot,
      cl_timeout);
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            status,
                            "Should be able to start learning (modify)");

  helper_test_credential_learn_structure(credential_nodes.user_id_node,
                                         credential_nodes.credential_type_node,
                                         credential_nodes.credential_slot_node,
                                         supported_cl_timeout[0],
                                         USER_CREDENTIAL_OPERATION_TYPE_MODIFY);

  // Test with specific timeout
  credential_type  = ZCL_CRED_TYPE_HAND_BIOMETRIC;
  credential_slot  = 1;
  cl_timeout       = 100;
  credential_nodes = helper_create_credential_structure(user_id,
                                                        credential_type,
                                                        credential_slot,
                                                        REPORTED_ATTRIBUTE);

  status = zwave_command_class_user_credential_credential_learn_start_modify(
    endpoint_id_node,
    user_id,
    credential_type,
    credential_slot,
    cl_timeout);

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            status,
                            "Should be able to start learning (modify)");
  helper_test_credential_learn_structure(credential_nodes.user_id_node,
                                         credential_nodes.credential_type_node,
                                         credential_nodes.credential_slot_node,
                                         cl_timeout,
                                         USER_CREDENTIAL_OPERATION_TYPE_MODIFY);
}

void test_user_credential_credential_learn_start_modify_cl_not_supported()
{
  user_credential_user_unique_id_t user_id   = 12;
  user_credential_type_t credential_type     = ZCL_CRED_TYPE_PIN_CODE;
  user_credential_slot_t credential_slot     = 1;
  user_credential_learn_timeout_t cl_timeout = 0;
  // Simulate user
  helper_create_credential_structure(user_id,
                                     credential_type,
                                     credential_slot,
                                     REPORTED_ATTRIBUTE);

  // Set capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_PIN_CODE, ZCL_CRED_TYPE_HAND_BIOMETRIC};
  std::vector<uint8_t> supported_cl                = {0, 1};
  std::vector<uint16_t> supported_credential_slots = {1, 2};
  std::vector<uint8_t> supported_cred_min_length   = {2, 5};
  std::vector<uint8_t> supported_cred_max_length   = {6, 10};
  std::vector<uint8_t> supported_cl_timeout        = {0, 2};
  std::vector<uint8_t> supported_cl_steps          = {0, 95};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                supported_cl_timeout,
                                                supported_cl_steps);

  sl_status_t status
    = zwave_command_class_user_credential_credential_learn_start_modify(
      endpoint_id_node,
      user_id,
      credential_type,
      credential_slot,
      cl_timeout);
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_FAIL,
                            status,
                            "Should not be able to start learning (modify)");
}

void test_user_credential_credential_learn_start_modify_credential_not_existing()
{
  user_credential_user_unique_id_t user_id   = 12;
  user_credential_type_t credential_type     = ZCL_CRED_TYPE_PIN_CODE;
  user_credential_slot_t credential_slot     = 1;
  user_credential_learn_timeout_t cl_timeout = 0;

  // Simulate user
  cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  // Set capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_PIN_CODE, ZCL_CRED_TYPE_HAND_BIOMETRIC};
  std::vector<uint8_t> supported_cl                = {0, 1};
  std::vector<uint16_t> supported_credential_slots = {1, 2};
  std::vector<uint8_t> supported_cred_min_length   = {2, 5};
  std::vector<uint8_t> supported_cred_max_length   = {6, 10};
  std::vector<uint8_t> supported_cl_timeout        = {0, 2};
  std::vector<uint8_t> supported_cl_steps          = {0, 95};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                supported_cl_timeout,
                                                supported_cl_steps);

  sl_status_t status
    = zwave_command_class_user_credential_credential_learn_start_modify(
      endpoint_id_node,
      user_id,
      credential_type,
      credential_slot,
      cl_timeout);
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_FAIL,
                            status,
                            "Should not be able to start learning (modify)");
}

void test_user_credential_credential_learn_cancel_happy_case()
{
  uint8_t stop_flag = 0;
  auto stop_node
    = cpp_endpoint_id_node.emplace_node(ATTRIBUTE(CREDENTIAL_LEARN_STOP),
                                        stop_flag,
                                        DESIRED_ATTRIBUTE);

  helper_test_get_set_frame_happy_case(CREDENTIAL_LEARN_CANCEL, stop_node);

  TEST_ASSERT_EQUAL_MESSAGE(1,
                            stop_node.reported<uint8_t>(),
                            "Stop flag should have been set to 1");
}

void test_user_credential_uuic_association_same_slot_different_user()
{
  // Capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_PIN_CODE, ZCL_CRED_TYPE_HAND_BIOMETRIC};
  std::vector<uint8_t> supported_cl                = {1, 1};
  std::vector<uint16_t> supported_credential_slots = {5, 2};
  std::vector<uint8_t> supported_cred_min_length   = {2, 5};
  std::vector<uint8_t> supported_cred_max_length   = {6, 10};
  std::vector<uint8_t> supported_cl_timeout        = {20, 2};
  std::vector<uint8_t> supported_cl_steps          = {1, 95};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                supported_cl_timeout,
                                                supported_cl_steps);

  // Structure
  user_credential_user_unique_id_t source_user_id      = 12;
  user_credential_user_unique_id_t destination_user_id = 15;
  user_credential_type_t credential_type               = ZCL_CRED_TYPE_PIN_CODE;
  user_credential_slot_t source_credential_slot        = 1;
  user_credential_slot_t destination_credential_slot   = 1;
  // Credential data
  std::string credential_data                   = "123456";
  user_credential_modifier_type_t modifier_type = 5;
  uint8_t association_status = USER_CREDENTIAL_ASSOCIATION_REPORT_SUCCESS;

  auto source_nodes = helper_create_credential_structure(source_user_id,
                                                         credential_type,
                                                         source_credential_slot,
                                                         REPORTED_ATTRIBUTE);

  auto destination_user_id_node
    = cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID),
                                        destination_user_id);

  helper_fill_credential_data(source_nodes.credential_slot_node,
                              credential_data,
                              modifier_type);
  auto association_nodes
    = helper_setup_association(source_nodes.credential_slot_node,
                               destination_user_id,
                               destination_credential_slot);
  // Simulate Set
  zwave_frame set_frame;
  set_frame.add(source_user_id);
  set_frame.add(credential_type);
  set_frame.add(source_credential_slot);
  set_frame.add(destination_user_id);
  set_frame.add(destination_credential_slot);

  helper_test_get_set_frame_happy_case(
    USER_CREDENTIAL_ASSOCIATION_SET,
    association_nodes.association_credential_slot_node,
    set_frame);

  // Then simulate report
  helper_simulate_association_report_frame(source_user_id,
                                           credential_type,
                                           source_credential_slot,
                                           destination_user_id,
                                           destination_credential_slot,
                                           association_status);

  // Test data structure
  TEST_ASSERT_TRUE_MESSAGE(source_nodes.credential_type_node.is_valid(),
                           "Old credential type node should still exist");
  // Association nodes doesn't have any reported value so they shouldn't exists anymore
  TEST_ASSERT_FALSE_MESSAGE(
    association_nodes.association_user_id_node.is_valid(),
    "Association user id node should have been removed");
  TEST_ASSERT_FALSE_MESSAGE(
    association_nodes.association_credential_slot_node.is_valid(),
    "Association credential slot node should have been removed");

  TEST_ASSERT_EQUAL_MESSAGE(
    0,
    source_nodes.credential_type_node.child_count(),
    "Source Credential type node should have no children");

  // Check destination structure
  auto destination_credential_type_node
    = helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_TYPE),
                                  credential_type,
                                  destination_user_id_node);
  TEST_ASSERT_EQUAL_MESSAGE(source_nodes.credential_slot_node.parent(),
                            destination_credential_type_node,
                            "Old credential slot node should have new parent");

  auto destination_credential_slot_node
    = helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_SLOT),
                                  destination_credential_slot,
                                  destination_credential_type_node);

  helper_test_credential_data(destination_credential_slot_node,
                              credential_data,
                              modifier_type);

  helper_test_attribute_value(ATTRIBUTE(ASSOCIATION_STATUS),
                              association_status,
                              destination_credential_slot_node);
}

void test_user_credential_uuic_association_different_slot_different_user_with_existing_type()
{
  // Capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_PIN_CODE, ZCL_CRED_TYPE_HAND_BIOMETRIC};
  std::vector<uint8_t> supported_cl                = {1, 1};
  std::vector<uint16_t> supported_credential_slots = {5, 2};
  std::vector<uint8_t> supported_cred_min_length   = {2, 5};
  std::vector<uint8_t> supported_cred_max_length   = {6, 10};
  std::vector<uint8_t> supported_cl_timeout        = {20, 2};
  std::vector<uint8_t> supported_cl_steps          = {1, 95};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                supported_cl_timeout,
                                                supported_cl_steps);

  // Structure
  user_credential_user_unique_id_t source_user_id      = 12;
  user_credential_user_unique_id_t destination_user_id = 15;
  user_credential_type_t credential_type               = ZCL_CRED_TYPE_PIN_CODE;
  user_credential_slot_t source_credential_slot        = 1;
  user_credential_slot_t destination_credential_slot   = 2;
  // Credential data
  std::string credential_data                   = "123456";
  user_credential_modifier_type_t modifier_type = 5;
  uint8_t association_status = USER_CREDENTIAL_ASSOCIATION_REPORT_SUCCESS;

  auto source_nodes = helper_create_credential_structure(source_user_id,
                                                         credential_type,
                                                         source_credential_slot,
                                                         REPORTED_ATTRIBUTE);

  auto destination_user_id_node
    = cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID),
                                        destination_user_id);
  // Already present type
  destination_user_id_node.emplace_node(ATTRIBUTE(CREDENTIAL_TYPE),
                                        credential_type);

  helper_fill_credential_data(source_nodes.credential_slot_node,
                              credential_data,
                              modifier_type);
  auto association_nodes
    = helper_setup_association(source_nodes.credential_slot_node,
                               destination_user_id,
                               destination_credential_slot);

  // Simulate Set
  zwave_frame set_frame;
  set_frame.add(source_user_id);
  set_frame.add(credential_type);
  set_frame.add(source_credential_slot);
  set_frame.add(destination_user_id);
  set_frame.add(destination_credential_slot);

  helper_test_get_set_frame_happy_case(
    USER_CREDENTIAL_ASSOCIATION_SET,
    association_nodes.association_credential_slot_node,
    set_frame);

  // Then simulate report
  helper_simulate_association_report_frame(source_user_id,
                                           credential_type,
                                           source_credential_slot,
                                           destination_user_id,
                                           destination_credential_slot,
                                           association_status);

  // Test data structure
  TEST_ASSERT_TRUE_MESSAGE(source_nodes.credential_type_node.is_valid(),
                           "Old credential type node should still exist");
  // Association nodes doesn't have any reported value so they shouldn't exists anymore
  TEST_ASSERT_FALSE_MESSAGE(
    association_nodes.association_user_id_node.is_valid(),
    "Association user id node should have been removed");
  TEST_ASSERT_FALSE_MESSAGE(
    association_nodes.association_credential_slot_node.is_valid(),
    "Association credential slot node should have been removed");

  TEST_ASSERT_EQUAL_MESSAGE(
    0,
    source_nodes.credential_type_node.child_count(),
    "Source Credential type node should have no children");

  // Check destination structure
  auto destination_credential_type_node
    = helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_TYPE),
                                  credential_type,
                                  destination_user_id_node);

  TEST_ASSERT_EQUAL_MESSAGE(source_nodes.credential_slot_node.parent(),
                            destination_credential_type_node,
                            "Old credential slot node should have new parent");

  auto destination_credential_slot_node
    = helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_SLOT),
                                  destination_credential_slot,
                                  destination_credential_type_node);

  helper_test_credential_data(destination_credential_slot_node,
                              credential_data,
                              modifier_type);

  helper_test_attribute_value(ATTRIBUTE(ASSOCIATION_STATUS),
                              association_status,
                              destination_credential_slot_node);
}

void test_user_credential_uuic_association_different_slot_same_user()
{
  // Capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_PIN_CODE, ZCL_CRED_TYPE_HAND_BIOMETRIC};
  std::vector<uint8_t> supported_cl                = {1, 1};
  std::vector<uint16_t> supported_credential_slots = {5, 2};
  std::vector<uint8_t> supported_cred_min_length   = {2, 5};
  std::vector<uint8_t> supported_cred_max_length   = {6, 10};
  std::vector<uint8_t> supported_cl_timeout        = {20, 2};
  std::vector<uint8_t> supported_cl_steps          = {1, 95};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                supported_cl_timeout,
                                                supported_cl_steps);

  // Structure
  user_credential_user_unique_id_t source_user_id      = 12;
  user_credential_user_unique_id_t destination_user_id = 12;
  user_credential_type_t credential_type               = ZCL_CRED_TYPE_PIN_CODE;
  user_credential_slot_t source_credential_slot        = 1;
  user_credential_slot_t destination_credential_slot   = 2;
  // Credential data
  std::string credential_data                   = "123456";
  user_credential_modifier_type_t modifier_type = 5;
  uint8_t association_status = USER_CREDENTIAL_ASSOCIATION_REPORT_SUCCESS;

  auto source_nodes = helper_create_credential_structure(source_user_id,
                                                         credential_type,
                                                         source_credential_slot,
                                                         REPORTED_ATTRIBUTE);

  helper_fill_credential_data(source_nodes.credential_slot_node,
                              credential_data,
                              modifier_type);
  auto association_nodes
    = helper_setup_association(source_nodes.credential_slot_node,
                               destination_user_id,
                               destination_credential_slot);
  // Simulate Set
  zwave_frame set_frame;
  set_frame.add(source_user_id);
  set_frame.add(credential_type);
  set_frame.add(source_credential_slot);
  set_frame.add(destination_user_id);
  set_frame.add(destination_credential_slot);

  helper_test_get_set_frame_happy_case(
    USER_CREDENTIAL_ASSOCIATION_SET,
    association_nodes.association_credential_slot_node,
    set_frame);

  // Then simulate report
  helper_simulate_association_report_frame(source_user_id,
                                           credential_type,
                                           source_credential_slot,
                                           destination_user_id,
                                           destination_credential_slot,
                                           association_status);

  // Test data structure
  TEST_ASSERT_TRUE_MESSAGE(
    attribute_store_node_exists(source_nodes.credential_type_node),
    "Old credential type node should still exist");
  TEST_ASSERT_TRUE_MESSAGE(
    attribute_store_node_exists(source_nodes.credential_slot_node),
    "Old credential slot node should just been updated");
  // Association nodes doesn't have any reported value so they shouldn't exists anymore
  TEST_ASSERT_FALSE_MESSAGE(
    attribute_store_node_exists(association_nodes.association_user_id_node),
    "Association user id node should have been removed");
  TEST_ASSERT_FALSE_MESSAGE(
    attribute_store_node_exists(
      association_nodes.association_credential_slot_node),
    "Association credential slot node should have been removed");

  TEST_ASSERT_EQUAL_MESSAGE(
    1,
    attribute_store_get_node_child_count(source_nodes.credential_type_node),
    "Source Credential type node should have one children");

  // Check destination structure
  auto destination_credential_type_node
    = helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_TYPE),
                                  credential_type,
                                  source_nodes.user_id_node);

  auto destination_credential_slot_node
    = helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_SLOT),
                                  destination_credential_slot,
                                  destination_credential_type_node);

  helper_test_credential_data(destination_credential_slot_node,
                              credential_data,
                              modifier_type);

  helper_test_attribute_value(ATTRIBUTE(ASSOCIATION_STATUS),
                              association_status,
                              destination_credential_slot_node);
}

void test_user_credential_uuic_association_error_code()
{
  // Capabilities
  uint8_t supported_credential_checksum   = 1;
  uint8_t support_admin_code              = 1;
  uint8_t support_admin_code_deactivation = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_PIN_CODE, ZCL_CRED_TYPE_HAND_BIOMETRIC};
  std::vector<uint8_t> supported_cl                = {1, 1};
  std::vector<uint16_t> supported_credential_slots = {5, 2};
  std::vector<uint8_t> supported_cred_min_length   = {2, 5};
  std::vector<uint8_t> supported_cred_max_length   = {6, 10};
  std::vector<uint8_t> supported_cl_timeout        = {20, 2};
  std::vector<uint8_t> supported_cl_steps          = {1, 95};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                supported_cl_timeout,
                                                supported_cl_steps);

  // Structure
  user_credential_user_unique_id_t source_user_id      = 12;
  user_credential_user_unique_id_t destination_user_id = 15;
  user_credential_type_t credential_type               = ZCL_CRED_TYPE_PIN_CODE;
  user_credential_slot_t source_credential_slot        = 1;
  user_credential_slot_t destination_credential_slot   = 1;
  // Credential data
  std::string credential_data                   = "123456";
  user_credential_modifier_type_t modifier_type = 5;
  uint8_t association_status
    = USER_CREDENTIAL_ASSOCIATION_REPORT_SOURCE_CREDENTIAL_SLOT_EMPTY;

  auto source_nodes = helper_create_credential_structure(source_user_id,
                                                         credential_type,
                                                         source_credential_slot,
                                                         REPORTED_ATTRIBUTE);

  auto destination_user_id_node
    = cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID),
                                        destination_user_id);

  helper_fill_credential_data(source_nodes.credential_slot_node,
                              credential_data,
                              modifier_type);
  auto association_nodes
    = helper_setup_association(source_nodes.credential_slot_node,
                               destination_user_id,
                               destination_credential_slot);
  // Simulate Set
  zwave_frame set_frame;
  set_frame.add(source_user_id);
  set_frame.add(credential_type);
  set_frame.add(source_credential_slot);
  set_frame.add(destination_user_id);
  set_frame.add(destination_credential_slot);

  helper_test_get_set_frame_happy_case(
    USER_CREDENTIAL_ASSOCIATION_SET,
    association_nodes.association_credential_slot_node,
    set_frame);

  // Then simulate report
  helper_simulate_association_report_frame(source_user_id,
                                           credential_type,
                                           source_credential_slot,
                                           destination_user_id,
                                           destination_credential_slot,
                                           association_status);

  // Test data structure
  TEST_ASSERT_TRUE_MESSAGE(
    attribute_store_node_exists(source_nodes.credential_type_node),
    "Old credential type node should still exist");
  TEST_ASSERT_TRUE_MESSAGE(
    attribute_store_node_exists(source_nodes.credential_slot_node),
    "Old credential slot node should still exists");
  // Association nodes doesn't have any reported value so they shouldn't exists anymore
  TEST_ASSERT_FALSE_MESSAGE(
    attribute_store_node_exists(association_nodes.association_user_id_node),
    "Association user id node should have been removed");
  TEST_ASSERT_FALSE_MESSAGE(
    attribute_store_node_exists(
      association_nodes.association_credential_slot_node),
    "Association credential slot node should have been removed");

  TEST_ASSERT_EQUAL_MESSAGE(
    1,
    attribute_store_get_node_child_count(source_nodes.credential_type_node),
    "Source Credential type node should have one children");

  // No move should have been performed
  TEST_ASSERT_EQUAL_MESSAGE(
    0,
    attribute_store_get_node_child_count(destination_user_id_node),
    "Nothing should have changed for destination user ID node");

  // Test source credential slot node
  auto destination_credential_slot_node = source_nodes.credential_slot_node;
  helper_test_attribute_value(ATTRIBUTE(CREDENTIAL_SLOT),
                              destination_credential_slot,
                              source_nodes.credential_type_node);

  helper_test_credential_data(destination_credential_slot_node,
                              credential_data,
                              modifier_type);

  helper_test_attribute_value(ATTRIBUTE(ASSOCIATION_STATUS),
                              association_status,
                              destination_credential_slot_node);
}

void test_get_user_checksum_with_credentials_happy_case()
{
  user_credential_user_unique_id_t user_id                      = 12;
  user_credential_user_type_t user_type                         = 0x04;
  user_credential_user_active_state_t user_active_state         = 0x01;
  user_credential_supported_credential_rules_t credential_rules = 0x01;
  user_credential_user_name_encoding_t user_name_encoding       = 0x00;
  std::string user_name                                         = "Matt";

  // Creating user
  auto user_id_node
    = cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID),
                                        user_id,
                                        DESIRED_ATTRIBUTE);

  // Simulate get/report on user
  zwave_frame user_get_frame;
  user_get_frame.add(user_id);
  helper_test_get_set_frame_happy_case(USER_GET, user_id_node, user_get_frame);

  helper_simulate_user_report_frame(0x04,
                                    0,
                                    0x02,  // Z-Wave
                                    0,
                                    user_id,
                                    user_type,
                                    user_active_state,
                                    credential_rules,
                                    0,
                                    user_name_encoding,
                                    user_name);

  // Credentials
  std::vector<user_credential_type_t> credential_types
    = {ZCL_CRED_TYPE_PIN_CODE,
       ZCL_CRED_TYPE_PIN_CODE,
       ZCL_CRED_TYPE_PASSWORD,
       ZCL_CRED_TYPE_HAND_BIOMETRIC};
  std::vector<user_credential_slot_t> credential_slots = {2, 4, 1, 8};

  std::
    vector<std::vector<uint8_t>>
      credential_data = {
        {0x39, 0x32, 0x37, 0x37},              // "9277" in ASCII
        {0x39, 0x35, 0x34, 0x39, 0x38, 0x38},  // "954988" in ASCII
        {
          0x00, 0x7A, 0x00, 0x77, 0x00, 0x61, 0x00, 0x76, 0x00,
          0x65, 0x00, 0x6E, 0x00, 0x6F, 0x00, 0x64, 0x00, 0x65,
          0x00, 0x70, 0x00, 0x61, 0x00, 0x73, 0x00, 0x73, 0x00,
          0x77, 0x00, 0x6F, 0x00, 0x72, 0x00, 0x64},  // zwavenodepassword in Unicode UTF-16 format, in big endian order,
        {0x71, 0x99}  // Raw data
      };

  if (credential_types.size() != credential_slots.size()
      || credential_slots.size() != credential_data.size()) {
    TEST_FAIL_MESSAGE("All vectors should be the same size");
  }

  for (size_t i = 0; i < credential_types.size(); i++) {
    helper_simulate_credential_report_frame(
      0x00,
      user_id,
      credential_types[i],
      credential_slots[i],
      0,
      credential_data[i],
      0x02,  // Anything but 0
      0,
      (i + 1) == credential_types.size() ? 0 : credential_types[i + 1],
      (i + 1) == credential_types.size() ? 0 : credential_slots[i + 1]);
  }

  // Get checksum
  auto checksum_node = user_id_node.add_node(ATTRIBUTE(USER_CHECKSUM));

  zwave_frame checksum_get_frame;
  checksum_get_frame.add(user_id);

  helper_test_get_set_frame_happy_case(USER_CHECKSUM_GET,
                                       checksum_node,
                                       checksum_get_frame);

  user_credential_checksum_t expected_checksum = 0x9024;
  helper_simulate_user_checksum_report(user_id, expected_checksum);

  // Check if checksum is correct
  TEST_ASSERT_EQUAL_MESSAGE(
    expected_checksum,
    checksum_node.reported<user_credential_checksum_t>(),
    "Checksum mismatch");
}

void test_get_user_checksum_without_credentials_happy_case()
{
  user_credential_user_unique_id_t user_id                      = 12;
  user_credential_user_type_t user_type                         = 0x04;
  user_credential_user_active_state_t user_active_state         = 0x01;
  user_credential_supported_credential_rules_t credential_rules = 0x01;
  user_credential_user_name_encoding_t user_name_encoding       = 0x00;
  std::string user_name                                         = "Lillie";

  // Creating user
  auto user_id_node
    = cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID),
                                        user_id,
                                        DESIRED_ATTRIBUTE);

  // Simulate get/report on user
  zwave_frame user_get_frame;
  user_get_frame.add(user_id);
  helper_test_get_set_frame_happy_case(USER_GET, user_id_node, user_get_frame);

  helper_simulate_user_report_frame(0x04,
                                    0,
                                    0x02,  // Z-Wave
                                    0,
                                    user_id,
                                    user_type,
                                    user_active_state,
                                    credential_rules,
                                    0,
                                    user_name_encoding,
                                    user_name);

  // Get checksum
  auto checksum_node = user_id_node.add_node(ATTRIBUTE(USER_CHECKSUM));

  zwave_frame checksum_get_frame;
  checksum_get_frame.add(user_id);

  helper_test_get_set_frame_happy_case(USER_CHECKSUM_GET,
                                       checksum_node,
                                       checksum_get_frame);

  user_credential_checksum_t expected_checksum = 0xF900;
  helper_simulate_user_checksum_report(user_id, expected_checksum);

  // Check if checksum is correct
  TEST_ASSERT_EQUAL_MESSAGE(
    expected_checksum,
    checksum_node.reported<user_credential_checksum_t>(),
    "Checksum mismatch");
}

void test_get_user_checksum_without_credentials_mismatch_checksum()
{
  user_credential_user_unique_id_t user_id                      = 12;
  user_credential_user_type_t user_type                         = 0x04;
  user_credential_user_active_state_t user_active_state         = 0x01;
  user_credential_supported_credential_rules_t credential_rules = 0x01;
  user_credential_user_name_encoding_t user_name_encoding       = 0x00;
  std::string user_name                                         = "Lillie";

  // Creating user
  auto user_id_node
    = cpp_endpoint_id_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID),
                                        user_id,
                                        DESIRED_ATTRIBUTE);

  // Simulate get/report on user
  zwave_frame user_get_frame;
  user_get_frame.add(user_id);
  helper_test_get_set_frame_happy_case(USER_GET, user_id_node, user_get_frame);

  helper_simulate_user_report_frame(0x04,
                                    0,
                                    0x02,  // Z-Wave
                                    0,
                                    user_id,
                                    user_type,
                                    user_active_state,
                                    credential_rules,
                                    0,
                                    user_name_encoding,
                                    user_name);
  // Get checksum
  auto checksum_node = user_id_node.add_node(ATTRIBUTE(USER_CHECKSUM));

  zwave_frame checksum_get_frame;
  checksum_get_frame.add(user_id);

  helper_test_get_set_frame_happy_case(USER_CHECKSUM_GET,
                                       checksum_node,
                                       checksum_get_frame);

  // Simulate checksum mismatch
  user_credential_checksum_t expected_checksum       = 0xF900;
  user_credential_checksum_t error_expected_checksum = expected_checksum + 1;
  helper_simulate_user_checksum_report(user_id,
                                       error_expected_checksum,
                                       SL_STATUS_FAIL);

  // Check if checksum is correct
  TEST_ASSERT_EQUAL_MESSAGE(
    error_expected_checksum,
    checksum_node.reported<user_credential_checksum_t>(),
    "Calculated checksum mismatch");

  auto error_checksum_node
    = helper_test_attribute_value(ATTRIBUTE(USER_CHECKSUM_MISMATCH_ERROR),
                                  expected_checksum,
                                  user_id_node);

  // Now simulate checksum match
  helper_simulate_user_checksum_report(user_id, expected_checksum);

  TEST_ASSERT_EQUAL_MESSAGE(
    expected_checksum,
    checksum_node.reported<user_credential_checksum_t>(),
    "Calculated checksum mismatch");

  TEST_ASSERT_FALSE_MESSAGE(error_checksum_node.is_valid(),
                            "Error node should have been removed");
}

void test_get_credential_checksum_happy_case()
{
  helper_create_credential_checksum_structure();

  std::vector<user_credential_type_t> tested_credential_types
    = {ZCL_CRED_TYPE_PIN_CODE,
       ZCL_CRED_TYPE_PASSWORD,
       ZCL_CRED_TYPE_EYE_BIOMETRIC,
       ZCL_CRED_TYPE_RFID_CODE};

  std::vector<user_credential_checksum_t> expected_checksums
    = {0xD867, 0x6F76, 0xC06E, 0x0000};

  // Create empty RFID_CODE
  user_credential_type_t rfid_type = ZCL_CRED_TYPE_RFID_CODE;
  cpp_endpoint_id_node.emplace_node(ATTRIBUTE(SUPPORTED_CREDENTIAL_TYPE),
                                    rfid_type);

  if (tested_credential_types.size() != expected_checksums.size()) {
    TEST_FAIL_MESSAGE("All vectors should be the same size");
  }

  for (size_t i = 0; i < tested_credential_types.size(); i++) {
    auto current_credential_type = tested_credential_types[i];
    auto current_credential_type_node
      = cpp_endpoint_id_node.child_by_type_and_value(
        ATTRIBUTE(SUPPORTED_CREDENTIAL_TYPE),
        current_credential_type);

    TEST_ASSERT_TRUE_MESSAGE(current_credential_type_node.is_valid(),
                             "Credential type node should exist");

    // Get checksum
    auto checksum_node
      = current_credential_type_node.add_node(ATTRIBUTE(CREDENTIAL_CHECKSUM));

    zwave_frame checksum_get_frame;
    checksum_get_frame.add(current_credential_type);

    helper_test_get_set_frame_happy_case(CREDENTIAL_CHECKSUM_GET,
                                         checksum_node,
                                         checksum_get_frame);

    user_credential_checksum_t expected_checksum = expected_checksums[i];
    helper_simulate_credential_checksum_report(current_credential_type,
                                               expected_checksum);

    // Check if checksum is correct
    TEST_ASSERT_EQUAL_MESSAGE(
      expected_checksum,
      checksum_node.reported<user_credential_checksum_t>(),
      (std::string("Checksum mismatch for type ")
       + std::to_string(current_credential_type))
        .c_str());
  }
}

void test_get_credential_checksum_mismatch()
{
  helper_create_credential_checksum_structure();

  std::vector<user_credential_type_t> tested_credential_types
    = {ZCL_CRED_TYPE_PIN_CODE, ZCL_CRED_TYPE_PASSWORD};

  std::vector<user_credential_checksum_t> expected_checksums = {0xD867, 0x6F76};

  if (tested_credential_types.size() != expected_checksums.size()) {
    TEST_FAIL_MESSAGE("All vectors should be the same size");
  }

  for (size_t i = 0; i < tested_credential_types.size(); i++) {
    auto current_credential_type = tested_credential_types[i];
    auto current_credential_type_node
      = cpp_endpoint_id_node.child_by_type_and_value(
        ATTRIBUTE(SUPPORTED_CREDENTIAL_TYPE),
        current_credential_type);

    TEST_ASSERT_TRUE_MESSAGE(current_credential_type_node.is_valid(),
                             "Credential type node should exist");

    // Get checksum
    auto checksum_node
      = current_credential_type_node.add_node(ATTRIBUTE(CREDENTIAL_CHECKSUM));

    zwave_frame checksum_get_frame;
    checksum_get_frame.add(current_credential_type);

    helper_test_get_set_frame_happy_case(CREDENTIAL_CHECKSUM_GET,
                                         checksum_node,
                                         checksum_get_frame);

    user_credential_checksum_t error_checksum = expected_checksums[i] + 1;
    helper_simulate_credential_checksum_report(current_credential_type,
                                               error_checksum,
                                               SL_STATUS_FAIL);

    // Check error checksum
    TEST_ASSERT_EQUAL_MESSAGE(
      error_checksum,
      checksum_node.reported<user_credential_checksum_t>(),
      "Reported checksum mismatch");

    auto mismatch_checksum_node = helper_test_attribute_value(
      ATTRIBUTE(CREDENTIAL_CHECKSUM_MISMATCH_ERROR),
      expected_checksums[i],
      current_credential_type_node);

    // Try again with correct checksum

    user_credential_checksum_t expected_checksum = expected_checksums[i];
    helper_simulate_credential_checksum_report(current_credential_type,
                                               expected_checksum);

    // Check if checksum is correct
    TEST_ASSERT_EQUAL_MESSAGE(
      expected_checksum,
      checksum_node.reported<user_credential_checksum_t>(),
      "Checksum mismatch");

    TEST_ASSERT_FALSE_MESSAGE(
      mismatch_checksum_node.is_valid(),
      "Mismatch checksum node should have been removed");
  }
}

////////////////////////////////////////////////////////////////////////////
// All User Checksum Get/Report
////////////////////////////////////////////////////////////////////////////
void test_user_credential_all_users_checksum_get_happy_case()
{
  helper_test_get_set_frame_happy_case(ALL_USERS_CHECKSUM_GET);
}

void test_user_credential_all_users_checksum_report_no_users_happy_case()
{
  user_credential_all_users_checksum_t expected_checksum = 0x00;

  // Report frame
  zwave_frame report_frame;
  report_frame.add(expected_checksum);
  helper_test_report_frame(ALL_USERS_CHECKSUM_REPORT, report_frame);

  // Attribute store test
  helper_test_attribute_value(ATTRIBUTE(ALL_USERS_CHECKSUM), expected_checksum);
}

void test_user_credential_all_users_checksum_report_with_users_happy_case()
{
  user_credential_all_users_checksum_t expected_checksum = 0x57b9;

  // Add 2 users and one with credentials
  helper_simulate_user_report_frame(0x00,  // User Add
                                    0,
                                    0,
                                    0,
                                    12,
                                    1,
                                    1,
                                    15,
                                    0,
                                    0,
                                    "MICHEL");
  helper_simulate_user_report_frame(0x00,  // User Add
                                    0,
                                    0,
                                    0,
                                    1312,
                                    1,
                                    1,
                                    15,
                                    0,
                                    0,
                                    "TURBO");
  helper_simulate_credential_report_frame(0x00,  // Credential Add
                                          12,
                                          1,
                                          1,
                                          0,
                                          string_to_uint8_vector("1234"),
                                          0x02,
                                          0,
                                          0,
                                          0);
  // Report frame
  zwave_frame report_frame;
  report_frame.add(expected_checksum);
  helper_test_report_frame(ALL_USERS_CHECKSUM_REPORT, report_frame);

  // Attribute store test
  helper_test_attribute_value(ATTRIBUTE(ALL_USERS_CHECKSUM), expected_checksum);
}

////////////////////////////////////////////////////////////////////////////
// Admin PIN code Set/Get/Support
////////////////////////////////////////////////////////////////////////////
void test_user_credential_admin_pin_code_get_happy_case()
{
  helper_test_get_set_frame_happy_case(ADMIN_PIN_CODE_GET);
}

void helper_setup_credential_capabilities_admin_code(
  uint8_t support_admin_code = 1, uint8_t support_admin_code_deactivation = 1)
{
  uint8_t supported_credential_checksum   = 1;
  std::vector<user_credential_type_t> supported_credential_type
    = {ZCL_CRED_TYPE_PIN_CODE};
  std::vector<uint8_t> supported_cl                = {1};
  std::vector<uint16_t> supported_credential_slots = {1};
  std::vector<uint8_t> supported_cred_min_length   = {2};
  std::vector<uint8_t> supported_cred_max_length   = {6};
  helper_simulate_credential_capabilites_report(supported_credential_checksum,
                                                support_admin_code,
                                                support_admin_code_deactivation,
                                                supported_credential_type,
                                                supported_cl,
                                                supported_credential_slots,
                                                supported_cred_min_length,
                                                supported_cred_max_length,
                                                {1},
                                                {1});
}
void test_user_credential_admin_pin_code_set_happy_case()
{
  helper_setup_credential_capabilities_admin_code();

  std::string admin_pin_code = "1312";
  auto admin_pin_code_data   = string_to_uint8_vector(admin_pin_code);

  auto status = zwave_command_class_user_credential_set_admin_pin_code(
    endpoint_id_node,
    admin_pin_code.c_str());
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            status,
                            "Set Admin PIN API command should have worked");

  auto admin_pin_code_node
    = helper_test_and_get_node(ATTRIBUTE(ADMIN_PIN_CODE));

  zwave_frame set_frame;
  set_frame.add(admin_pin_code_data);

  helper_test_get_set_frame_happy_case(ADMIN_PIN_CODE_SET,
                                       admin_pin_code_node,
                                       set_frame);
}

void test_user_credential_admin_pin_code_set_deactivation_happy_case()
{
  helper_setup_credential_capabilities_admin_code();

  auto status
    = zwave_command_class_user_credential_set_admin_pin_code(endpoint_id_node,
                                                             "");
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            status,
                            "Set Admin PIN API command should have worked");

  auto admin_pin_code_node
    = helper_test_and_get_node(ATTRIBUTE(ADMIN_PIN_CODE));

  zwave_frame set_frame;
  set_frame.add(static_cast<uint8_t>(0));
  helper_test_get_set_frame_happy_case(ADMIN_PIN_CODE_SET,
                                       admin_pin_code_node,
                                       set_frame);
}

void test_user_credential_admin_pin_code_set_deactivation_not_supported()
{
  helper_setup_credential_capabilities_admin_code(1, 0);

  auto status
    = zwave_command_class_user_credential_set_admin_pin_code(endpoint_id_node,
                                                             "");
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_FAIL,
                            status,
                            "Set Admin PIN API command should NOT have worked");

  auto admin_pin_code_node
    = helper_test_and_get_node(ATTRIBUTE(ADMIN_PIN_CODE));

  helper_test_get_set_fail_case(ADMIN_PIN_CODE_SET,
                                SL_STATUS_NOT_SUPPORTED,
                                admin_pin_code_node);
}

void test_user_credential_admin_pin_code_report_happy_case()
{
  auto expected_admin_pin_code = string_to_uint8_vector("1312");
  uint8_t admin_pin_code_size
    = static_cast<uint8_t>(expected_admin_pin_code.size());
  uint8_t expected_operation_result = 0x04;

  // Report frame
  zwave_frame report_frame;
  uint8_t size_byte = (expected_operation_result << 4) | admin_pin_code_size;
  report_frame.add(size_byte);
  for (uint8_t c: expected_admin_pin_code) {
    report_frame.add(c);
  }

  helper_test_report_frame(ADMIN_PIN_CODE_REPORT, report_frame);

  // Attribute store test
  helper_test_attribute_value(ATTRIBUTE(ADMIN_PIN_CODE_OPERATION_RESULT),
                              expected_operation_result);
  helper_test_attribute_value(ATTRIBUTE(ADMIN_PIN_CODE),
                              expected_admin_pin_code);
}

void test_user_credential_admin_pin_code_report_deletion_happy_case()
{
  // Simulate report with 0 as pin code size
  uint8_t admin_pin_code_size       = 0;
  uint8_t expected_operation_result = 0x04;

  // Simulate already present code
  auto admin_pin_code_node
    = cpp_endpoint_id_node.emplace_node(ATTRIBUTE(ADMIN_PIN_CODE));
  admin_pin_code_node.set_reported(string_to_uint8_vector("1312"));

  // Report frame
  zwave_frame report_frame;
  uint8_t size_byte = (expected_operation_result << 4) | admin_pin_code_size;
  report_frame.add(size_byte);

  helper_test_report_frame(ADMIN_PIN_CODE_REPORT, report_frame);

  // Attribute store test
  helper_test_attribute_value(ATTRIBUTE(ADMIN_PIN_CODE_OPERATION_RESULT),
                              expected_operation_result);
  std::vector<uint8_t> empty_vector = {0};
  helper_test_attribute_value(ATTRIBUTE(ADMIN_PIN_CODE), empty_vector);
}

}  // extern "C"