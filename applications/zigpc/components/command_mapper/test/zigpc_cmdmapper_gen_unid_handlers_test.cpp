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

#include "zigpc_cmdmapper_test_cpp_includes.hpp"

#include <dotdot_mqtt.h>

/**
 * NOTE: The following commands have been chosen arbitrarily to verify common
 * cases in generated code.
 */

sl_status_t zigpc_command_mapper_on_off_toggle_handler(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t callback_type);

sl_status_t zigpc_command_mapper_door_lock_set_pin_code_handler(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t callback_type,
  DrlkPINUserID userid,
  DrlkSettableUserStatus user_status,
  DrlkUserType user_type,
  const char *pin);

sl_status_t zigpc_command_mapper_groups_get_group_membership_handler(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t callback_type,
  uint8_t group_list_count,
  const uint16_t *group_list);

sl_status_t zigpc_command_mapper_thermostat_set_weekly_schedule_handler(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t callback_type,
  uint8_t number_of_transitions,
  uint8_t day_of_week,
  uint8_t mode,
  uint8_t transitions_count,
  const TransitionType *transitions);

extern "C" {

// Unify includes
#include <unity.h>
#include <sl_status.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>
#include "zcl_util_mock.h"
#include "zigpc_gateway_mock.h"

// Component includes
#include "zigpc_command_mapper_int.h"

static struct {
  zigpc_zcl_frame_type_t frame_type;
  zcl_cluster_id_t cluster_id;
  zcl_command_id_t command_id;
  size_t command_arg_count;
  zigpc_zcl_frame_data_t *command_arg_list;
} stub_frame_builder_data;

sl_status_t
  stub_frame_builder(zcl_frame_t *const frame,
                     const zigpc_zcl_frame_type_t frame_type,
                     const zcl_cluster_id_t cluster_id,
                     const zcl_command_id_t command_id,
                     const size_t command_arg_count,
                     const zigpc_zcl_frame_data_t *const command_arg_list,
                     int cmock_num_calls)
{
  TEST_ASSERT_EQUAL_HEX(stub_frame_builder_data.frame_type, frame_type);
  TEST_ASSERT_EQUAL_HEX(stub_frame_builder_data.cluster_id, cluster_id);
  TEST_ASSERT_EQUAL_HEX(stub_frame_builder_data.command_id, command_id);
  TEST_ASSERT_EQUAL(stub_frame_builder_data.command_arg_count,
                    command_arg_count);

  for (size_t i = 0; i < command_arg_count; i++) {
    TEST_ASSERT_EQUAL_HEX8(stub_frame_builder_data.command_arg_list[i].type,
                           command_arg_list[i].type);
    switch (command_arg_list[i].type) {
      case ZIGPC_ZCL_DATA_TYPE_INT8:
      case ZIGPC_ZCL_DATA_TYPE_UINT8:
      case ZIGPC_ZCL_DATA_TYPE_ENUM8:
      case ZIGPC_ZCL_DATA_TYPE_MAP8:
        TEST_ASSERT_EQUAL_MEMORY(
          stub_frame_builder_data.command_arg_list[i].data,
          command_arg_list[i].data,
          1);
        break;
      case ZIGPC_ZCL_DATA_TYPE_INT16:
      case ZIGPC_ZCL_DATA_TYPE_UINT16:
      case ZIGPC_ZCL_DATA_TYPE_ENUM16:
      case ZIGPC_ZCL_DATA_TYPE_MAP16:
        TEST_ASSERT_EQUAL_MEMORY(
          stub_frame_builder_data.command_arg_list[i].data,
          command_arg_list[i].data,
          2);
        break;
      case ZIGPC_ZCL_DATA_TYPE_STRING:
      case ZIGPC_ZCL_DATA_TYPE_OCTSTR:
        TEST_ASSERT_EQUAL_STRING(
          stub_frame_builder_data.command_arg_list[i].data,
          command_arg_list[i].data);
        break;
      case ZIGPC_ZCL_DATA_TYPE_STRUCT_TRANSITION_TYPE:
        TEST_ASSERT_EQUAL_MEMORY(
          stub_frame_builder_data.command_arg_list[i].data,
          command_arg_list[i].data,
          sizeof(zigpc_zcl_transition_type_t));
        break;
      default:
        TEST_ASSERT_EQUAL_HEX8_MESSAGE(0,
                                       command_arg_list[i].type,
                                       "Unknown type to compare");
    }
  }

  return SL_STATUS_OK;
}

/**
 * @brief Setup the test suite (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void) {}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * @brief Setup before each test case
 *
 */
void setUp(void)
{
  stub_frame_builder_data.frame_type
    = ZIGPC_ZCL_FRAME_TYPE_GLOBAL_CMD_TO_SERVER;
  stub_frame_builder_data.cluster_id        = 0;
  stub_frame_builder_data.command_id        = 0;
  stub_frame_builder_data.command_arg_count = 0;
  stub_frame_builder_data.command_arg_list  = nullptr;
}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown(void) {}

void test_unid_command_without_arguments(void)
{
  zigbee_eui64_uint_t eui64 = 0x2303DF99103FCBDD;
  zigbee_endpoint_id_t ep   = 2;
  // ARRANGE

  zigpc_zcl_build_command_frame_ExpectAndReturn(
    nullptr,
    ZIGPC_ZCL_FRAME_TYPE_CMD_TO_SERVER,
    ZIGPC_ZCL_CLUSTER_ON_OFF,
    ZIGPC_ZCL_CLUSTER_ON_OFF_COMMAND_TOGGLE,
    0,
    nullptr,
    SL_STATUS_OK);
  zigpc_zcl_build_command_frame_IgnoreArg_frame();
  zigpc_zcl_build_command_frame_IgnoreArg_command_arg_list();

  zigpc_gateway_send_zcl_command_frame_IgnoreAndReturn(SL_STATUS_OK);

  // ACT
  sl_status_t status = zigpc_command_mapper_on_off_toggle_handler(
    zigpc_ucl::mqtt::build_unid(eui64).c_str(),
    ep,
    UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_unid_command_with_primitive_arguments(void)
{
  zigbee_eui64_uint_t eui64 = 0x1303DF99103FCBDD;
  zigbee_endpoint_id_t ep   = 3;
  // ucl command data
  std::string ucl_pin = "Hello 1234 World!?";
  uic_mqtt_dotdot_door_lock_command_set_pin_code_fields_t ucl_cmd_fields = {
    .userid      = 0x1234,
    .user_status = ZCL_DRLK_SETTABLE_USER_STATUS_OCCUPIED_ENABLED,
    .user_type   = ZCL_DRLK_USER_TYPE_MASTER_USER,
    .pin         = ucl_pin.c_str(),
  };
  // zigpc cmd arg list
  size_t cmd_arg_count = 4;
  zigpc_zcl_frame_data_t cmd_arg_list[cmd_arg_count]
    = {{.type = ZIGPC_ZCL_DATA_TYPE_UINT16, .data = &ucl_cmd_fields.userid},
       {.type = ZIGPC_ZCL_DATA_TYPE_UINT8, .data = &ucl_cmd_fields.user_status},
       {.type = ZIGPC_ZCL_DATA_TYPE_ENUM8, .data = &ucl_cmd_fields.user_type},
       {.type = ZIGPC_ZCL_DATA_TYPE_OCTSTR, .data = ucl_cmd_fields.pin}};

  // ARRANGE
  stub_frame_builder_data.frame_type = ZIGPC_ZCL_FRAME_TYPE_CMD_TO_SERVER;
  stub_frame_builder_data.cluster_id = ZIGPC_ZCL_CLUSTER_DOOR_LOCK;
  stub_frame_builder_data.command_id
    = ZIGPC_ZCL_CLUSTER_DOOR_LOCK_COMMAND_SET_PIN_CODE;
  stub_frame_builder_data.command_arg_count = cmd_arg_count;
  stub_frame_builder_data.command_arg_list  = cmd_arg_list;
  zigpc_zcl_build_command_frame_StubWithCallback(stub_frame_builder);

  zigpc_gateway_send_zcl_command_frame_IgnoreAndReturn(SL_STATUS_OK);

  // ACT
  sl_status_t status = zigpc_command_mapper_door_lock_set_pin_code_handler(
    zigpc_ucl::mqtt::build_unid(eui64).c_str(),
    ep,
    UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
    ucl_cmd_fields.userid,
    ucl_cmd_fields.user_status,
    ucl_cmd_fields.user_type,
    ucl_cmd_fields.pin);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_unid_command_with_array_arguments(void)
{
  zigbee_eui64_uint_t eui64 = 0x1303DF99103FCBDD;
  zigbee_endpoint_id_t ep   = 3;
  // ucl command data
  uint8_t ucl_group_count                  = 4;
  uint16_t ucl_group_list[ucl_group_count] = {0x12, 0x43, 0x63, 0x2592};
  uic_mqtt_dotdot_groups_command_get_group_membership_fields_t ucl_cmd_fields
    = {.group_list_count = ucl_group_count, .group_list = ucl_group_list};
  // zigpc cmd arg list
  size_t cmd_arg_count                               = 5;
  zigpc_zcl_frame_data_t cmd_arg_list[cmd_arg_count] = {
    {.type = ZIGPC_ZCL_DATA_TYPE_UINT8, .data = &ucl_group_count},
    {.type = ZIGPC_ZCL_DATA_TYPE_UINT16, .data = &ucl_group_list[0]},
    {.type = ZIGPC_ZCL_DATA_TYPE_UINT16, .data = &ucl_group_list[1]},
    {.type = ZIGPC_ZCL_DATA_TYPE_UINT16, .data = &ucl_group_list[2]},
    {.type = ZIGPC_ZCL_DATA_TYPE_UINT16, .data = &ucl_group_list[3]},
  };

  // ARRANGE
  stub_frame_builder_data.frame_type = ZIGPC_ZCL_FRAME_TYPE_CMD_TO_SERVER;
  stub_frame_builder_data.cluster_id = ZIGPC_ZCL_CLUSTER_GROUPS;
  stub_frame_builder_data.command_id
    = ZIGPC_ZCL_CLUSTER_GROUPS_COMMAND_GET_GROUP_MEMBERSHIP;
  stub_frame_builder_data.command_arg_count = cmd_arg_count;
  stub_frame_builder_data.command_arg_list  = cmd_arg_list;
  zigpc_zcl_build_command_frame_StubWithCallback(stub_frame_builder);

  zigpc_gateway_send_zcl_command_frame_IgnoreAndReturn(SL_STATUS_OK);

  // ACT
  sl_status_t status = zigpc_command_mapper_groups_get_group_membership_handler(
    zigpc_ucl::mqtt::build_unid(eui64).c_str(),
    ep,
    UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
    ucl_cmd_fields.group_list_count,
    ucl_cmd_fields.group_list);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_unid_command_with_struct_array_arguments(void)
{
  zigbee_eui64_uint_t eui64 = 0x1303AF99103FCBDD;
  zigbee_endpoint_id_t ep   = 4;
  // ucl command data
  uint8_t ucl_transition_count = 3;
  TransitionType ucl_transition_list[ucl_transition_count]
    = {{1, 1, 1}, {100, 200, 300}, {0xFF23, 0x3241, 0x1111}};
  uic_mqtt_dotdot_thermostat_command_set_weekly_schedule_fields_t ucl_cmd_fields
    = {
      .number_of_transitions = ucl_transition_count,
      .day_of_week           = 0x34,
      .mode                  = 0x80,
      .transitions_count     = ucl_transition_count,
      .transitions           = ucl_transition_list,

    };
  // zigpc cmd arg list
  size_t cmd_arg_count                               = 6;
  zigpc_zcl_frame_data_t cmd_arg_list[cmd_arg_count] = {
    {.type = ZIGPC_ZCL_DATA_TYPE_ENUM8,
     .data = &ucl_cmd_fields.number_of_transitions},
    {.type = ZIGPC_ZCL_DATA_TYPE_MAP8, .data = &ucl_cmd_fields.day_of_week},
    {.type = ZIGPC_ZCL_DATA_TYPE_MAP8, .data = &ucl_cmd_fields.mode},
    {.type = ZIGPC_ZCL_DATA_TYPE_STRUCT_TRANSITION_TYPE,
     .data = &ucl_transition_list[0]},
    {.type = ZIGPC_ZCL_DATA_TYPE_STRUCT_TRANSITION_TYPE,
     .data = &ucl_transition_list[1]},
    {.type = ZIGPC_ZCL_DATA_TYPE_STRUCT_TRANSITION_TYPE,
     .data = &ucl_transition_list[2]},
  };

  // ARRANGE
  stub_frame_builder_data.frame_type = ZIGPC_ZCL_FRAME_TYPE_CMD_TO_SERVER;
  stub_frame_builder_data.cluster_id = ZIGPC_ZCL_CLUSTER_THERMOSTAT;
  stub_frame_builder_data.command_id
    = ZIGPC_ZCL_CLUSTER_THERMOSTAT_COMMAND_SET_WEEKLY_SCHEDULE;
  stub_frame_builder_data.command_arg_count = cmd_arg_count;
  stub_frame_builder_data.command_arg_list  = cmd_arg_list;
  zigpc_zcl_build_command_frame_StubWithCallback(stub_frame_builder);

  zigpc_gateway_send_zcl_command_frame_IgnoreAndReturn(SL_STATUS_OK);

  // ACT
  sl_status_t status
    = zigpc_command_mapper_thermostat_set_weekly_schedule_handler(
      zigpc_ucl::mqtt::build_unid(eui64).c_str(),
      ep,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
      ucl_cmd_fields.number_of_transitions,
      ucl_cmd_fields.day_of_week,
      ucl_cmd_fields.mode,
      ucl_cmd_fields.transitions_count,
      ucl_cmd_fields.transitions);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

}  // extern "C"
