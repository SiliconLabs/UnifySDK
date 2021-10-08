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
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute.hpp"
#include "zpc_attribute_store_test_helper.h"

#include "zwave_command_class_agi.h"
#include "zwave_unid.h"
#include "ZW_classcmd.h"
#include "workaround_for_test.hpp"
using namespace attribute_store;

extern "C" {
#include "zwave_network_management_mock.h"
#include "zwave_command_handler_mock.h"
#include "attribute_resolver_mock.h"
#include "zwave_tx_mock.h"

#include "unity.h"

// Note, Multi Channel Association shares attributes with Association
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_##type

// Helper macro to validate array attributes stored under GROUP_ID attributes
#define validate_group_attribute_array(attribute_id, expected)                 \
  {                                                                            \
    try {                                                                      \
      attribute attr_actual = attribute(endpoint_id_node)                      \
                                .child_by_type(ATTRIBUTE(GROUP_ID))            \
                                .child_by_type(attribute_id);                  \
      TEST_ASSERT_TRUE(attr_actual.is_valid());                                \
      auto actual = attr_actual.get<std::vector<uint8_t>>(REPORTED_ATTRIBUTE); \
      TEST_ASSERT_EQUAL(sizeof(expected), actual.size());                      \
      TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, actual.data(), actual.size());    \
    } catch (std::exception & ex) {                                            \
      TEST_ASSERT_TRUE_MESSAGE(false, "Exception thrown");                     \
    }                                                                          \
  }

static zwave_command_handler_t agi_handler;
static const zwave_controller_connection_info_t connection
  = {.remote = {.node_id = node_id, .endpoint_id = endpoint_id}};

std::map<attribute_store_type_t, attribute_resolver_function_t>
  resolver_get_callbacks;
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  resolver_get_callbacks.emplace(node_type, get_func);
  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zwave_unid_set_home_id(home_id);
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);
  attribute_resolver_register_rule_IgnoreAndReturn(SL_STATUS_OK);
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
                    new_command_class_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_ASSOCIATION_GRP_INFO,
                    new_command_class_handler.command_class);
  TEST_ASSERT_EQUAL(ASSOCIATION_GRP_INFO_VERSION_V3,
                    new_command_class_handler.version);
  TEST_ASSERT_NOT_NULL(new_command_class_handler.control_handler);
  TEST_ASSERT_NOT_NULL(new_command_class_handler.support_handler);
  agi_handler = new_command_class_handler;
  return SL_STATUS_OK;
}

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);
  zwave_command_class_agi_init();

  // Test attributes created at inits
  attribute attr_ep(zpc_endpoint_id_node);
  TEST_ASSERT_TRUE(attr_ep.is_valid());
  attribute attr_gid(attr_ep.child_by_type(ATTRIBUTE(GROUP_ID)));
  TEST_ASSERT_TRUE(attr_gid.is_valid());
  attribute attr_name = attr_gid.child_by_type(ATTRIBUTE(GROUP_NAME));
  TEST_ASSERT_TRUE(attr_name.is_valid());
  attribute attr_profile = attr_gid.child_by_type(ATTRIBUTE(GROUP_PROFILE));
  TEST_ASSERT_TRUE(attr_profile.is_valid());
  attribute attr_cmdlst = attr_gid.child_by_type(ATTRIBUTE(GROUP_COMMAND_LIST));
  TEST_ASSERT_TRUE(attr_cmdlst.is_valid());
  attribute attr_grp_content = attr_gid.child_by_type(ATTRIBUTE(GROUP_CONTENT));
  TEST_ASSERT_TRUE(attr_grp_content.is_valid());
  attribute attr_max = attr_gid.child_by_type(ATTRIBUTE(MAX_NODES_SUPPORTED));
  TEST_ASSERT_TRUE(attr_max.is_valid());
  TEST_ASSERT_EQUAL(
    attr_max.get<association_group_capacity_t>(REPORTED_ATTRIBUTE),
    MAX_SUPPORTED_NODES_PER_GROUP);

  // Create group on the endpoint_node
  attribute attr_ep_node(endpoint_id_node);
  attr_ep_node.add_node(ATTRIBUTE_ZWAVE_NIF)
    .set_reported<uint8_t>(COMMAND_CLASS_ASSOCIATION_GRP_INFO);
  attr_ep_node.add_node(ATTRIBUTE(GROUP_ID))
    .set_reported<association_group_id_t>(1);
  TEST_ASSERT_TRUE(attr_ep_node.child_by_type(ATTRIBUTE(GROUP_ID)).is_valid());
}

// Get functions
void test_zwave_command_class_agi_group_info_get()
{
  const ZW_ASSOCIATION_GROUP_INFO_GET_FRAME expected
    = {.cmdClass           = COMMAND_CLASS_ASSOCIATION_GRP_INFO,
       .cmd                = ASSOCIATION_GROUP_INFO_GET,
       .properties1        = RESERVED_BYTE,
       .groupingIdentifier = 1};
  attribute_store_log();
  attribute group_profile = attribute(endpoint_id_node)
                              .child_by_type(ATTRIBUTE(GROUP_ID))
                              .child_by_type(ATTRIBUTE(GROUP_PROFILE));
  TEST_ASSERT_TRUE(group_profile.is_valid());
  uint8_t frame[100];
  uint16_t frame_length = 0;
  resolver_get_callbacks.find(ATTRIBUTE(GROUP_PROFILE))
    ->second(group_profile, frame, &frame_length);

  TEST_ASSERT_EQUAL(sizeof(expected), frame_length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(reinterpret_cast<const uint8_t *>(&expected),
                                frame,
                                frame_length);
}

void test_zwave_command_class_agi_command_list_get()
{
  const ZW_ASSOCIATION_GROUP_COMMAND_LIST_GET_FRAME expected
    = {.cmdClass           = COMMAND_CLASS_ASSOCIATION_GRP_INFO,
       .cmd                = ASSOCIATION_GROUP_COMMAND_LIST_GET,
       .properties1        = RESERVED_BYTE,
       .groupingIdentifier = 1};
  attribute_store_log();
  attribute group_profile = attribute(endpoint_id_node)
                              .child_by_type(ATTRIBUTE(GROUP_ID))
                              .child_by_type(ATTRIBUTE(GROUP_COMMAND_LIST));
  TEST_ASSERT_TRUE(group_profile.is_valid());
  uint8_t frame[100];
  uint16_t frame_length = 0;
  resolver_get_callbacks.find(ATTRIBUTE(GROUP_COMMAND_LIST))
    ->second(group_profile, frame, &frame_length);

  TEST_ASSERT_EQUAL(sizeof(expected), frame_length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(reinterpret_cast<const uint8_t *>(&expected),
                                frame,
                                frame_length);
}

void test_zwave_command_class_agi_name_get()
{
  const ZW_ASSOCIATION_GROUP_NAME_GET_FRAME expected
    = {.cmdClass           = COMMAND_CLASS_ASSOCIATION_GRP_INFO,
       .cmd                = ASSOCIATION_GROUP_NAME_GET,
       .groupingIdentifier = 1};
  attribute_store_log();
  attribute group_profile = attribute(endpoint_id_node)
                              .child_by_type(ATTRIBUTE(GROUP_ID))
                              .child_by_type(ATTRIBUTE(GROUP_NAME));
  TEST_ASSERT_TRUE(group_profile.is_valid());
  uint8_t frame[100];
  uint16_t frame_length = 0;
  resolver_get_callbacks.find(ATTRIBUTE(GROUP_NAME))
    ->second(group_profile, frame, &frame_length);

  TEST_ASSERT_EQUAL(sizeof(expected), frame_length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(reinterpret_cast<const uint8_t *>(&expected),
                                frame,
                                frame_length);
}

// Report functions
void test_zwave_command_class_agi_group_info_report()
{
  const ZW_ASSOCIATION_GROUP_INFO_REPORT_1BYTE_FRAME frame
    = {.cmdClass      = COMMAND_CLASS_ASSOCIATION_GRP_INFO,
       .cmd           = ASSOCIATION_GROUP_INFO_REPORT,
       .properties1   = RESERVED_BYTE,
       .variantgroup1 = {}};
  agi_handler.control_handler(&connection,
                              reinterpret_cast<const uint8_t *>(&frame),
                              sizeof(frame));
  // TODO: Implement test here
}

// test ASSOCIATION_GROUP_COMMAND_LIST_REPORT
void test_zwave_command_class_agi_command_list_report_sunny()
{
  const ZW_ASSOCIATION_GROUP_COMMAND_LIST_REPORT_4BYTE_FRAME frame
    = {.cmdClass           = COMMAND_CLASS_ASSOCIATION_GRP_INFO,
       .cmd                = ASSOCIATION_GROUP_COMMAND_LIST_REPORT,
       .groupingIdentifier = 1,
       .listLength         = 4,
       .command1           = COMMAND_CLASS_BASIC,
       .command2           = BASIC_SET,
       .command3           = COMMAND_CLASS_WINDOW_COVERING,
       .command4           = WINDOW_COVERING_SET_IN_BOTTOM_2};
  agi_handler.control_handler(&connection,
                              reinterpret_cast<const uint8_t *>(&frame),
                              sizeof(frame));
  //attribute_store_log();
  try {
    attribute attr_command_list
      = attribute(endpoint_id_node)
          .child_by_type(ATTRIBUTE(GROUP_ID))
          .child_by_type(ATTRIBUTE(GROUP_COMMAND_LIST));
    TEST_ASSERT_TRUE(attr_command_list.is_valid());
    const uint8_t expected[] = {COMMAND_CLASS_BASIC,
                                BASIC_SET,
                                COMMAND_CLASS_WINDOW_COVERING,
                                WINDOW_COVERING_SET_IN_BOTTOM_2};
    validate_group_attribute_array(ATTRIBUTE(GROUP_COMMAND_LIST), expected);
  } catch (std::exception &ex) {
    TEST_ASSERT_TRUE_MESSAGE(false, "Exception thrown");
  }
}

void test_zwave_command_class_agi_command_list_report_error_listLength()
{
  const ZW_ASSOCIATION_GROUP_COMMAND_LIST_REPORT_4BYTE_FRAME frame
    = {.cmdClass           = COMMAND_CLASS_ASSOCIATION_GRP_INFO,
       .cmd                = ASSOCIATION_GROUP_COMMAND_LIST_REPORT,
       .groupingIdentifier = 1,
       .listLength         = 5,
       .command1           = COMMAND_CLASS_BASIC,
       .command2           = BASIC_SET,
       .command3           = COMMAND_CLASS_WINDOW_COVERING,
       .command4           = WINDOW_COVERING_SET_IN_BOTTOM_2};
  agi_handler.control_handler(&connection,
                              reinterpret_cast<const uint8_t *>(&frame),
                              sizeof(frame));
  //attribute_store_log();
  try {
    attribute attr_command_list
      = attribute(endpoint_id_node)
          .child_by_type(ATTRIBUTE(GROUP_ID))
          .child_by_type(ATTRIBUTE(GROUP_COMMAND_LIST));
    TEST_ASSERT_TRUE(attr_command_list.is_valid());
    TEST_ASSERT_FALSE(attr_command_list.reported_exists());
  } catch (std::exception &ex) {
    TEST_ASSERT_TRUE_MESSAGE(false, "Exception thrown");
  }
}

// test ASSOCIATION_GROUP_NAME_REPORT
void test_zwave_command_class_agi_name_report_sunny()
{
  const ZW_ASSOCIATION_GROUP_NAME_REPORT_3BYTE_FRAME frame
    = {.cmdClass           = COMMAND_CLASS_ASSOCIATION_GRP_INFO,
       .cmd                = ASSOCIATION_GROUP_NAME_REPORT,
       .groupingIdentifier = 1,
       .lengthOfName       = 3,
       .name1              = 'U',
       .name2              = 'I',
       .name3              = 'C'};
  agi_handler.control_handler(&connection,
                              reinterpret_cast<const uint8_t *>(&frame),
                              sizeof(frame));
  //attribute_store_log();
  const uint8_t expected[] = {'U', 'I', 'C', '\0'};
  validate_group_attribute_array(ATTRIBUTE(GROUP_NAME), expected);
}

void test_zwave_command_class_agi_name_report_error_lengthOfFrame_too_large()
{
  const ZW_ASSOCIATION_GROUP_NAME_REPORT_4BYTE_FRAME frame
    = {.cmdClass           = COMMAND_CLASS_ASSOCIATION_GRP_INFO,
       .cmd                = ASSOCIATION_GROUP_NAME_REPORT,
       .groupingIdentifier = 1,
       .lengthOfName       = 5,
       .name1              = 'U',
       .name2              = 'I',
       .name3              = 'C',
       .name4              = '\0'};
  agi_handler.control_handler(&connection,
                              reinterpret_cast<const uint8_t *>(&frame),
                              sizeof(frame));
  //attribute_store_log();
  try {
    attribute attr_group_name = attribute(endpoint_id_node)
                                  .child_by_type(ATTRIBUTE(GROUP_ID))
                                  .child_by_type(ATTRIBUTE(GROUP_NAME));
    TEST_ASSERT_TRUE(attr_group_name.is_valid());
    TEST_ASSERT_FALSE(attr_group_name.reported_exists());
  } catch (std::exception &ex) {
    TEST_ASSERT_TRUE_MESSAGE(false, "Exception thrown");
  }
}

void test_zwave_command_class_agi_name_report_error_frame_too_small()
{
  const ZW_ASSOCIATION_GROUP_NAME_REPORT_1BYTE_FRAME frame
    = {.cmdClass           = COMMAND_CLASS_ASSOCIATION_GRP_INFO,
       .cmd                = ASSOCIATION_GROUP_NAME_REPORT,
       .groupingIdentifier = 1,
       .lengthOfName       = 0,
       .name1              = 0};
  agi_handler.control_handler(&connection,
                              reinterpret_cast<const uint8_t *>(&frame),
                              sizeof(frame) - 1);
  //attribute_store_log();
  try {
    attribute attr_group_name = attribute(endpoint_id_node)
                                  .child_by_type(ATTRIBUTE(GROUP_ID))
                                  .child_by_type(ATTRIBUTE(GROUP_NAME));
    TEST_ASSERT_TRUE(attr_group_name.is_valid());
    TEST_ASSERT_FALSE(attr_group_name.reported_exists());
  } catch (std::exception &ex) {
    TEST_ASSERT_TRUE_MESSAGE(false, "Exception thrown");
  }
}

void test_zwave_command_class_agi_group_info_report_happy_case()
{
  const uint8_t frame[] = {COMMAND_CLASS_ASSOCIATION_GRP_INFO,
                           ASSOCIATION_GROUP_INFO_REPORT,
                           1,   // Group Count
                           1,   // Group ID
                           0,   // Mode - reserved
                           01,  // Profile MSB
                           02,  // Profile LSB
                           0,   // Reserved
                           0,   // Event Code MSB - Reserved
                           0};  // Event Code LSB - Reserved
  agi_handler.control_handler(&connection, frame, sizeof(frame));
  try {
    attribute attr_group_profile = attribute(endpoint_id_node)
                                     .child_by_type(ATTRIBUTE(GROUP_ID))
                                     .child_by_type(ATTRIBUTE(GROUP_PROFILE));
    TEST_ASSERT_TRUE(attr_group_profile.is_valid());
    agi_profile_t profile = attr_group_profile.reported<agi_profile_t>();
    TEST_ASSERT_EQUAL(0x0102, profile);
  } catch (std::exception &ex) {
    TEST_ASSERT_TRUE_MESSAGE(false, "Exception thrown");
  }
}

void test_zwave_command_class_agi_group_info_report_group_block_too_small()
{
  const uint8_t frame[] = {
    COMMAND_CLASS_ASSOCIATION_GRP_INFO,
    ASSOCIATION_GROUP_INFO_REPORT,
    1,   // Group Count
    1,   // Group ID
    0,   // Mode - reserved
    02,  // Profile MSB
    01   // Profile LSB
  };
  agi_handler.control_handler(&connection, frame, sizeof(frame));
  try {
    attribute attr_group_profile = attribute(endpoint_id_node)
                                     .child_by_type(ATTRIBUTE(GROUP_ID))
                                     .child_by_type(ATTRIBUTE(GROUP_PROFILE));
    TEST_ASSERT_TRUE(attr_group_profile.is_valid());
    agi_profile_t profile = attr_group_profile.reported<agi_profile_t>();
    TEST_ASSERT_EQUAL(0x0201, profile);
  } catch (std::exception &ex) {
    TEST_ASSERT_TRUE_MESSAGE(false, "Exception thrown");
  }
}

void test_zwave_command_class_agi_group_info_report_multiple_blocks()
{
  const uint8_t frame[] = {
    COMMAND_CLASS_ASSOCIATION_GRP_INFO,
    ASSOCIATION_GROUP_INFO_REPORT,
    2,   // Group Count
    2,   // Group ID
    0,   // Mode - reserved
    01,  // Profile MSB
    02,  // Profile LSB
    0,   // Reserved
    0,   // Event Code MSB - Reserved
    0,   // Event Code LSB - Reserved
    1,   // Group ID
    0,   // Mode - reserved
    05,  // Profile MSB
    05   // Profile LSB
  };
  agi_handler.control_handler(&connection, frame, sizeof(frame));
  try {
    attribute attr_group_profile = attribute(endpoint_id_node)
                                     .child_by_type(ATTRIBUTE(GROUP_ID))
                                     .child_by_type(ATTRIBUTE(GROUP_PROFILE));
    TEST_ASSERT_TRUE(attr_group_profile.is_valid());
    agi_profile_t profile = attr_group_profile.reported<agi_profile_t>();
    TEST_ASSERT_EQUAL(0x0505, profile);

    //Verify that group 2 does not exist
    const agi_profile_t group_profile_2 = 2;
    attribute attr_group_profile_2
      = attribute(endpoint_id_node)
          .child_by_type(ATTRIBUTE(GROUP_ID))
          .child_by_type_and_value(ATTRIBUTE(GROUP_PROFILE), group_profile_2);
    TEST_ASSERT_FALSE(attr_group_profile_2.is_valid());

  } catch (std::exception &ex) {
    TEST_ASSERT_TRUE_MESSAGE(false, ex.what());
  }
}

void test_zwave_command_class_agi_group_info_report_too_short_first_block()
{
  const uint8_t frame[] = {
    COMMAND_CLASS_ASSOCIATION_GRP_INFO,
    ASSOCIATION_GROUP_INFO_REPORT,
    2,   // Group Count
    1,   // Group ID
    0,   // Mode - reserved
    01,  // Profile MSB
  };
  agi_handler.control_handler(&connection, frame, sizeof(frame));
  try {
    attribute attr_group_profile = attribute(endpoint_id_node)
                                     .child_by_type(ATTRIBUTE(GROUP_ID))
                                     .child_by_type(ATTRIBUTE(GROUP_PROFILE));
    TEST_ASSERT_TRUE(attr_group_profile.is_valid());
    TEST_ASSERT_FALSE(attr_group_profile.reported_exists());

  } catch (std::exception &ex) {
    TEST_ASSERT_TRUE_MESSAGE(false, "Exception thrown");
  }
}

void test_zwave_command_class_agi_group_info_report_too_short_second_block()
{
  const uint8_t frame[] = {
    COMMAND_CLASS_ASSOCIATION_GRP_INFO,
    ASSOCIATION_GROUP_INFO_REPORT,
    2,   // Group Count
    2,   // Group ID
    0,   // Mode - reserved
    01,  // Profile MSB
    02,  // Profile LSB
    0,   // Reserved
    0,   // Event Code MSB - Reserved
    0,   // Event Code LSB - Reserved
    1,   // Group ID
    0,   // Mode - reserved
    05   // Profile MSB
  };
  agi_handler.control_handler(&connection, frame, sizeof(frame));
  try {
    attribute attr_group_profile = attribute(endpoint_id_node)
                                     .child_by_type(ATTRIBUTE(GROUP_ID))
                                     .child_by_type(ATTRIBUTE(GROUP_PROFILE));
    TEST_ASSERT_TRUE(attr_group_profile.is_valid());
    TEST_ASSERT_FALSE(attr_group_profile.reported_exists());

  } catch (std::exception &ex) {
    TEST_ASSERT_TRUE_MESSAGE(false, "Exception thrown");
  }
}

// Test Group Name Get command handler
void test_zwave_command_class_agi_handle_group_name_get()
{
  uint8_t expected_name[]
    = {0x4c, 0x69, 0x66, 0x65, 0x6c, 0x69, 0x6e, 0x65, 0x00};
  const uint8_t get_frame[] = {COMMAND_CLASS_ASSOCIATION_GRP_INFO,
                               ASSOCIATION_GROUP_NAME_GET_V3,
                               LIFELINE_GROUP_ID};
  zwave_tx_send_data_IgnoreAndReturn(SL_STATUS_OK);
  agi_handler.support_handler(&connection, get_frame, sizeof(get_frame));
  try {
    attribute attr_group_name_node = attribute(zpc_endpoint_id_node)
                                       .child_by_type(ATTRIBUTE(GROUP_ID))
                                       .child_by_type(ATTRIBUTE(GROUP_NAME));
    TEST_ASSERT_TRUE(attr_group_name_node.is_valid());
    TEST_ASSERT_TRUE(attr_group_name_node.reported_exists());
    uint8_t group_name[255] = {};
    uint8_t name_size;

    attribute_store_node_t test_attr_group_name
      = attribute_store_get_node_child_by_type(
        attribute_store_get_node_child_by_type(zpc_endpoint_id_node,
                                               ATTRIBUTE(GROUP_ID),
                                               0),
        ATTRIBUTE(GROUP_NAME),
        0);

    attribute_store_get_node_attribute_value(test_attr_group_name,
                                             REPORTED_ATTRIBUTE,
                                             group_name,
                                             &name_size);
    TEST_ASSERT_EQUAL(name_size, sizeof(expected_name));
  } catch (std::exception &ex) {
    TEST_ASSERT_TRUE_MESSAGE(false, "Exception thrown");
  }
}

// Test Group Info Get Command handler
void test_zwave_command_class_agi_handle_group_info_get()
{
  // Test a scenario when List Mode is set to 0
  const uint8_t get_frame[]       = {COMMAND_CLASS_ASSOCIATION_GRP_INFO,
                               ASSOCIATION_GROUP_INFO_GET_V3,
                               0x00,
                               LIFELINE_GROUP_ID};
  uint8_t expected_report_frame[] = {COMMAND_CLASS_ASSOCIATION_GRP_INFO,
                                     ASSOCIATION_GROUP_INFO_REPORT,
                                     0x01,
                                     LIFELINE_GROUP_ID,
                                     0x00,
                                     0x00,
                                     0x01,
                                     RESERVED_BYTE,
                                     0x00,
                                     0x00};
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection,
                                              sizeof(connection),
                                              sizeof(expected_report_frame),
                                              expected_report_frame,
                                              sizeof(expected_report_frame),
                                              NULL,
                                              0,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_connection();

  zwave_controller_connection_info_t connection_info_1
    = {.local = {.node_id = 1, .endpoint_id = 0, .is_multicast = false}};

  agi_handler.support_handler(&connection_info_1, get_frame, sizeof(get_frame));

  // Test a scenario when List Mode is set to 1
  const uint8_t get_frame_2[]
    = {COMMAND_CLASS_ASSOCIATION_GRP_INFO, ASSOCIATION_GROUP_INFO_GET_V3, 0x40};
  // built the second Group profile
  agi_profile_t profile_2           = 0x0003;
  association_group_id_t group_id_2 = 0x02;
  attribute attr_ep_node_zpc(zpc_endpoint_id_node);
  attr_ep_node_zpc.add_node(ATTRIBUTE(GROUP_ID))
    .set_reported<association_group_id_t>(group_id_2);
  attr_ep_node_zpc.child_by_type_and_value(ATTRIBUTE(GROUP_ID), group_id_2)
    .add_node(ATTRIBUTE(GROUP_PROFILE))
    .set_reported<agi_profile_t>(profile_2);

  uint8_t expected_report_frame_2[] = {COMMAND_CLASS_ASSOCIATION_GRP_INFO,
                                       ASSOCIATION_GROUP_INFO_REPORT,
                                       0x82,
                                       LIFELINE_GROUP_ID,
                                       0x00,
                                       0x00,
                                       0x01,
                                       RESERVED_BYTE,
                                       0x00,
                                       0x00,
                                       group_id_2,
                                       0x00,
                                       0x00,
                                       0x03,
                                       RESERVED_BYTE,
                                       0x00,
                                       0x00};
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection,
                                              sizeof(connection),
                                              sizeof(expected_report_frame_2),
                                              expected_report_frame_2,
                                              sizeof(expected_report_frame_2),
                                              NULL,
                                              0,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_connection();
  agi_handler.support_handler(&connection_info_1,
                              get_frame_2,
                              sizeof(get_frame_2));
}

// test Group Command List Get Command handler
void test_zwave_command_class_agi_handle_group_command_list_get()
{
  const uint8_t get_frame[] = {COMMAND_CLASS_ASSOCIATION_GRP_INFO,
                               ASSOCIATION_GROUP_COMMAND_LIST_GET,
                               0x00,
                               LIFELINE_GROUP_ID};
  // add GROUP_COMMAND_LIST attribute value
  zwave_command_class_agi_add_group_commands(LIFELINE_GROUP_ID, 0x5a, 0x01);
  uint8_t expected_report_frame[] = {COMMAND_CLASS_ASSOCIATION_GRP_INFO,
                                     ASSOCIATION_GROUP_COMMAND_LIST_REPORT,
                                     LIFELINE_GROUP_ID,
                                     0x02,
                                     0x5a,
                                     0x01};
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection,
                                              sizeof(connection),
                                              sizeof(expected_report_frame),
                                              expected_report_frame,
                                              sizeof(expected_report_frame),
                                              NULL,
                                              0,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_connection();
  agi_handler.support_handler(&connection, get_frame, sizeof(get_frame));
}
}
