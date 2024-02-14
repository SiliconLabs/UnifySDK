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

#include "zigpc_cmdmapper_test_cpp_includes.hpp"

#include <dotdot_mqtt.h>

/**
 * NOTE: The following commands have been chosen arbitrarily to verify common
 * cases in generated code.
 */

void zigpc_command_mapper_bygroup_on_off_toggle_handler(
  const dotdot_group_id_t group_id);

void zigpc_command_mapper_bygroup_door_lock_set_pin_code_handler(
  const dotdot_group_id_t group_id,
  const uic_mqtt_dotdot_door_lock_command_set_pin_code_fields_t *fields);

void zigpc_command_mapper_bygroup_identify_write_attributes_handler(
  const dotdot_group_id_t group_id,
  uic_mqtt_dotdot_identify_state_t values,
  uic_mqtt_dotdot_identify_updated_state_t values_to_write);

extern "C" {

// Unify includes
#include <unity.h>
#include <sl_status.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>
#include "zcl_util_mock.h"
#include "zigpc_group_mgmt_mock.h"
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
  stub_frame_builder_cb(zcl_frame_t *const frame,
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
      case ZIGPC_ZCL_DATA_TYPE_DATA8:
      case ZIGPC_ZCL_DATA_TYPE_UINT8:
      case ZIGPC_ZCL_DATA_TYPE_ENUM8:
        TEST_ASSERT_EQUAL_MEMORY(
          stub_frame_builder_data.command_arg_list[i].data,
          command_arg_list[i].data,
          1);
        break;
      case ZIGPC_ZCL_DATA_TYPE_UINT16:
      case ZIGPC_ZCL_DATA_TYPE_ATTRIB_ID:
        TEST_ASSERT_EQUAL_MEMORY(
          stub_frame_builder_data.command_arg_list[i].data,
          command_arg_list[i].data,
          2);
        break;
      case ZIGPC_ZCL_DATA_TYPE_OCTSTR:
        TEST_ASSERT_EQUAL_STRING(
          stub_frame_builder_data.command_arg_list[i].data,
          command_arg_list[i].data);
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

void test_bygroup_command_without_arguments(void)
{
  dotdot_group_id_t group_id = 2;

  // ARRANGE
  zigpc_groupmgmt_get_member_count_ExpectAndReturn(group_id, 1);

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

  zigpc_gateway_send_zcl_frame_multicast_IgnoreAndReturn(SL_STATUS_OK);

  // ACT
  zigpc_command_mapper_bygroup_on_off_toggle_handler(group_id);

  // ASSERT
}

void test_bygroup_command_with_primitive_arguments(void)
{
  dotdot_group_id_t group_id = 1;
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
  zigpc_groupmgmt_get_member_count_ExpectAndReturn(group_id, 1);

  stub_frame_builder_data.frame_type = ZIGPC_ZCL_FRAME_TYPE_CMD_TO_SERVER;
  stub_frame_builder_data.cluster_id = ZIGPC_ZCL_CLUSTER_DOOR_LOCK;
  stub_frame_builder_data.command_id
    = ZIGPC_ZCL_CLUSTER_DOOR_LOCK_COMMAND_SET_PIN_CODE;
  stub_frame_builder_data.command_arg_count = cmd_arg_count;
  stub_frame_builder_data.command_arg_list  = cmd_arg_list;
  zigpc_zcl_build_command_frame_StubWithCallback(stub_frame_builder_cb);

  zigpc_gateway_send_zcl_frame_multicast_IgnoreAndReturn(SL_STATUS_OK);

  // ACT
  zigpc_command_mapper_bygroup_door_lock_set_pin_code_handler(group_id,
                                                             &ucl_cmd_fields);

  // ASSERT
}

void test_bygroup_writeattributes_command(void)
{
  dotdot_group_id_t group_id = 1;
  // ucl command data
  uic_mqtt_dotdot_identify_state_t values = {
    .identify_time = 0x3454,
  };
  uic_mqtt_dotdot_identify_updated_state_t values_to_write = {
    .identify_time = true,
  };

  zcl_attribute_id_t attr_id = ZIGPC_ZCL_CLUSTER_IDENTIFY_ATTR_IDENTIFY_TIME;
  zigpc_zcl_data_type_t attr_data_type = ZIGPC_ZCL_DATA_TYPE_UINT16;

  // zigpc cmd arg list
  size_t cmd_arg_count                               = 3;
  zigpc_zcl_frame_data_t cmd_arg_list[cmd_arg_count] = {
    {.type = ZIGPC_ZCL_DATA_TYPE_ATTRIB_ID, .data = &attr_id},
    {.type = ZIGPC_ZCL_DATA_TYPE_DATA8, .data = &attr_data_type},
    {.type = ZIGPC_ZCL_DATA_TYPE_UINT16, .data = &values.identify_time},
  };

  // ARRANGE
  zigpc_groupmgmt_get_member_count_ExpectAndReturn(group_id, 1);

  stub_frame_builder_data.frame_type
    = ZIGPC_ZCL_FRAME_TYPE_GLOBAL_CMD_TO_SERVER;
  stub_frame_builder_data.cluster_id = ZIGPC_ZCL_CLUSTER_IDENTIFY;
  stub_frame_builder_data.command_id
    = ZIGPC_ZCL_GLOBAL_COMMAND_WRITE_ATTRIBUTES;
  stub_frame_builder_data.command_arg_count = cmd_arg_count;
  stub_frame_builder_data.command_arg_list  = cmd_arg_list;
  zigpc_zcl_build_command_frame_StubWithCallback(stub_frame_builder_cb);

  zigpc_gateway_send_zcl_frame_multicast_IgnoreAndReturn(SL_STATUS_OK);

  // ACT
  zigpc_command_mapper_bygroup_identify_write_attributes_handler(
    group_id,
    values,
    values_to_write);

  // ASSERT
}

}  // extern "C"
