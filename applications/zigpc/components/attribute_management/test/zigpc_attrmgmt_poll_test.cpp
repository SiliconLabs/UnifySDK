
#include "workaround.hpp"

extern "C" {

#include <unity.h>
#include "attribute_management_int.h"

#include "zcl_definitions.h"

#include "zcl_util_mock.h"
#include "zcl_attribute_info_mock.h"
#include "zigpc_gateway_mock.h"
#include "zcl_profiles_mock.h"
#include <stdio.h>


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
}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown(void) {}


void helper_expect_zcl_frame_calls(const zigbee_eui64_t eui64,
                                   zigbee_endpoint_id_t endpoint_id,
                                   zcl_cluster_id_t cluster_id,
                                   size_t expected_sets,
                                   std::vector<zcl_attribute_id_t> attr_ids)
{

  unsigned int TEST_SEND_LIMIT = 25;

  size_t attr_id_left = attr_ids.size();
  // Expect the number of times called to init + second ZCL frame as the
  // Expected number of messages to be constructed and sent per set
  for (size_t i = 0U; i < expected_sets; i++) {
    size_t attr_ids_to_send
      = (attr_id_left > TEST_SEND_LIMIT) ? TEST_SEND_LIMIT : attr_id_left;
    attr_id_left -= TEST_SEND_LIMIT;

    zigpc_zcl_build_command_frame_ExpectAndReturn(
      nullptr,
      ZIGPC_ZCL_FRAME_TYPE_GLOBAL_CMD_TO_SERVER,
      cluster_id,
      ZIGPC_ZCL_GLOBAL_COMMAND_READ_ATTRIBUTES,
      attr_ids_to_send,
      nullptr,
      SL_STATUS_OK);
    zigpc_zcl_build_command_frame_IgnoreArg_frame();
    zigpc_zcl_build_command_frame_IgnoreArg_command_arg_list();

    zigpc_gateway_send_zcl_command_frame_ExpectAndReturn(eui64,
                                                         endpoint_id,
                                                         cluster_id,
                                                         nullptr,
                                                         SL_STATUS_OK);
    zigpc_gateway_send_zcl_command_frame_IgnoreArg_frame();
  }
}

void test_zigpc_attrmgmt_add_poll_entry_succeeds()
{

    sl_status_t add_status = SL_STATUS_OK;
    zigbee_eui64_t expect_eui64
        = {0x04, 0x54, 0x02, 0xF2, 0x0A, 0x4C, 0x99, 0x10};
    

    zigbee_endpoint_id_t expect_endpoint_id = 1;
    zcl_cluster_id_t expect_cluster_id = 1;
    
    zigpc_zclprofiles_get_cluster_attribute_count_IgnoreAndReturn(1);
    zigpc_zclprofiles_get_cluster_attribute_list_IgnoreAndReturn(SL_STATUS_OK);
    zigpc_zcl_build_command_frame_IgnoreAndReturn(SL_STATUS_OK);
    zigpc_gateway_send_zcl_command_frame_IgnoreAndReturn(SL_STATUS_OK);

    zigpc_attrmgmt_empty_poll_list();
    
    add_status = zigpc_attrmgmt_add_poll_entry(
                    expect_eui64,
                    expect_endpoint_id,
                    expect_cluster_id);

    unsigned int test_size =
        zigpc_attrmgmt_get_poll_list_current_size();

    unsigned int test_max_size = 
        zigpc_attrmgmt_get_poll_list_max_size();

    zigbee_eui64_uint_t test_eui64 = 0;
    zigbee_endpoint_id_t test_endpoint_id = 0;
    zcl_cluster_id_t test_cluster_id = 0;

    sl_status_t get_status = 
                zigpc_attrmgmt_get_next_poll_entry(
                    test_eui64,
                    test_endpoint_id,
                    test_cluster_id);

    zigbee_eui64_uint_t expect_eui64_uint =
        zigbee_eui64_to_uint(expect_eui64);

  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, add_status);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, get_status);
  TEST_ASSERT_EQUAL(1, test_size);
  TEST_ASSERT_EQUAL(ZIGPC_ATTRMGMT_DEFAULT_POLL_SIZE, test_max_size);

  TEST_ASSERT_EQUAL(expect_eui64_uint, test_eui64);
  TEST_ASSERT_EQUAL_UINT(expect_endpoint_id,test_endpoint_id);
  TEST_ASSERT_EQUAL_UINT(expect_cluster_id,test_cluster_id);
    
  zigpc_attrmgmt_empty_poll_list();
}

void test_zigpc_attrmgmt_remove_poll_entry_succeeds()
{
    zigbee_eui64_t expect_eui64
        = {0x04, 0x54, 0x02, 0xF2, 0x0A, 0x4C, 0x99, 0x10};
    
    zigbee_eui64_t dummy_eui64
        = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    zigbee_eui64_t dummy1_eui64
        = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
    zigbee_eui64_t dummy2_eui64
        = {0x02, 0x02, 0x02, 0x02, 0x0A, 0x4C, 0x99, 0x10};

    zigbee_endpoint_id_t expect_endpoint_id = 1;
    zcl_cluster_id_t expect_cluster_id = 1;
    
    zigpc_zclprofiles_get_cluster_attribute_count_IgnoreAndReturn(1);
    zigpc_zclprofiles_get_cluster_attribute_list_IgnoreAndReturn(SL_STATUS_OK);
    zigpc_zcl_build_command_frame_IgnoreAndReturn(SL_STATUS_OK);
    zigpc_gateway_send_zcl_command_frame_IgnoreAndReturn(SL_STATUS_OK);

    zigpc_attrmgmt_empty_poll_list();
    
    sl_status_t  add_status = zigpc_attrmgmt_add_poll_entry(
                    expect_eui64,
                    expect_endpoint_id,
                    expect_cluster_id);
    
    add_status = zigpc_attrmgmt_add_poll_entry(
                    dummy_eui64,
                    expect_endpoint_id,
                    expect_cluster_id);
    add_status = zigpc_attrmgmt_add_poll_entry(
                    dummy1_eui64,
                    expect_endpoint_id,
                    expect_cluster_id);
    add_status = zigpc_attrmgmt_add_poll_entry(
                    dummy2_eui64,
                    expect_endpoint_id,
                    expect_cluster_id);

    //index at: expect 
    sl_status_t send_status = zigpc_attrmgmt_send_poll_attributes();
    //index at: dummy
    send_status = zigpc_attrmgmt_send_poll_attributes();
    //index at: dummy1
    send_status = zigpc_attrmgmt_send_poll_attributes();
    //index at: dummy2

    unsigned int test_size =
        zigpc_attrmgmt_get_poll_list_current_size();
  
    TEST_ASSERT_EQUAL(4, test_size);


    sl_status_t remove_status =
                    zigpc_attrmgmt_remove_poll_entry(
                        dummy1_eui64,
                        expect_endpoint_id,
                        expect_cluster_id);
   


    test_size =
        zigpc_attrmgmt_get_poll_list_current_size();
  
    TEST_ASSERT_EQUAL(3, test_size);

    zigbee_eui64_uint_t test_eui64 = 0;
    zigbee_endpoint_id_t test_endpoint_id = 0;
    zcl_cluster_id_t test_cluster_id = 0;

    sl_status_t get_status = 
                zigpc_attrmgmt_get_next_poll_entry(
                    test_eui64,
                    test_endpoint_id,
                    test_cluster_id);
   
    //to verify no segfault after remove
    send_status = zigpc_attrmgmt_send_poll_attributes();

    zigbee_eui64_uint_t expect_eui64_uint =
        zigbee_eui64_to_uint(expect_eui64);
    
    unsigned int test_max_size = 
        zigpc_attrmgmt_get_poll_list_max_size();

  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, add_status);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, get_status);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, send_status);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, remove_status);
  TEST_ASSERT_EQUAL(ZIGPC_ATTRMGMT_DEFAULT_POLL_SIZE, test_max_size);

  TEST_ASSERT_EQUAL(expect_eui64_uint, test_eui64);
  TEST_ASSERT_EQUAL_UINT(expect_endpoint_id,test_endpoint_id);
  TEST_ASSERT_EQUAL_UINT(expect_cluster_id,test_cluster_id);
    
  zigpc_attrmgmt_empty_poll_list();
}

void test_zigpc_attrmgmt_send_poll_increments_list()
{

    sl_status_t add_status = SL_STATUS_OK;
    sl_status_t send_status = SL_STATUS_OK;
    zigbee_eui64_t dummy_eui64
        = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    zigbee_eui64_t dummy1_eui64
        = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
    zigbee_eui64_t dummy2_eui64
        = {0x02, 0x02, 0x02, 0x02, 0x0A, 0x4C, 0x99, 0x10};
    
    zigpc_zclprofiles_get_cluster_attribute_count_IgnoreAndReturn(1);
    zigpc_zclprofiles_get_cluster_attribute_list_IgnoreAndReturn(SL_STATUS_OK);
    zigpc_zcl_build_command_frame_IgnoreAndReturn(SL_STATUS_OK);
    zigpc_gateway_send_zcl_command_frame_IgnoreAndReturn(SL_STATUS_OK);

    zigbee_eui64_t expect_eui64
        = {0x04, 0x54, 0x02, 0xF2, 0x0A, 0x4C, 0x99, 0x10};
    zigbee_endpoint_id_t expect_endpoint_id = 1;
    zcl_cluster_id_t expect_cluster_id = 1;
    
    zigpc_attrmgmt_empty_poll_list();
    
    add_status = zigpc_attrmgmt_add_poll_entry(
                    dummy_eui64,
                    expect_endpoint_id,
                    expect_cluster_id);
    add_status = zigpc_attrmgmt_add_poll_entry(
                    dummy1_eui64,
                    expect_endpoint_id,
                    expect_cluster_id);
    add_status = zigpc_attrmgmt_add_poll_entry(
                    dummy2_eui64,
                    expect_endpoint_id,
                    expect_cluster_id);

    add_status = zigpc_attrmgmt_add_poll_entry(
                    expect_eui64,
                    expect_endpoint_id,
                    expect_cluster_id);

    //index at: dummy
    send_status = zigpc_attrmgmt_send_poll_attributes();
    //index at: dummy1
    send_status = zigpc_attrmgmt_send_poll_attributes();
    //index at: dummy2
    send_status = zigpc_attrmgmt_send_poll_attributes();
    //index at: expect
    
    unsigned int test_size =
        zigpc_attrmgmt_get_poll_list_current_size();

    unsigned int test_max_size = 
        zigpc_attrmgmt_get_poll_list_max_size();

    zigbee_eui64_uint_t test_eui64 = 0;
    zigbee_endpoint_id_t test_endpoint_id = 0;
    zcl_cluster_id_t test_cluster_id = 0;

    sl_status_t get_status = 
                zigpc_attrmgmt_get_next_poll_entry(
                    test_eui64,
                    test_endpoint_id,
                    test_cluster_id);

    zigbee_eui64_uint_t expect_eui64_uint =
        zigbee_eui64_to_uint(expect_eui64);
    
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, add_status);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, get_status);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, send_status);
  TEST_ASSERT_EQUAL(4, test_size);
  TEST_ASSERT_EQUAL(ZIGPC_ATTRMGMT_DEFAULT_POLL_SIZE, test_max_size);

  TEST_ASSERT_EQUAL(expect_eui64_uint, test_eui64);
  TEST_ASSERT_EQUAL_UINT(expect_endpoint_id,test_endpoint_id);
  TEST_ASSERT_EQUAL_UINT(expect_cluster_id,test_cluster_id);
    
  zigpc_attrmgmt_empty_poll_list();
}

void test_zigpc_attrmgmt_send_poll_increments_list_resets()
{

    sl_status_t add_status = SL_STATUS_OK;
    sl_status_t send_status = SL_STATUS_OK;
    zigbee_eui64_t dummy_eui64
        = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
    zigbee_eui64_t dummy1_eui64
        = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
    zigbee_eui64_t dummy2_eui64
        = {0x02, 0x02, 0x02, 0x02, 0x0A, 0x4C, 0x99, 0x10};
    
    zigpc_zclprofiles_get_cluster_attribute_count_IgnoreAndReturn(1);
    zigpc_zclprofiles_get_cluster_attribute_list_IgnoreAndReturn(SL_STATUS_OK);
    zigpc_zcl_build_command_frame_IgnoreAndReturn(SL_STATUS_OK);
    zigpc_gateway_send_zcl_command_frame_IgnoreAndReturn(SL_STATUS_OK);

    zigbee_eui64_t expect_eui64
        = {0x04, 0x54, 0x02, 0xF2, 0x0A, 0x4C, 0x99, 0x10};
    zigbee_endpoint_id_t expect_endpoint_id = 1;
    zcl_cluster_id_t expect_cluster_id = 1;
    
    zigpc_attrmgmt_empty_poll_list();
    
    add_status = zigpc_attrmgmt_add_poll_entry(
                    dummy_eui64,
                    expect_endpoint_id,
                    expect_cluster_id);
    add_status = zigpc_attrmgmt_add_poll_entry(
                    dummy1_eui64,
                    expect_endpoint_id,
                    expect_cluster_id);
    add_status = zigpc_attrmgmt_add_poll_entry(
                    dummy2_eui64,
                    expect_endpoint_id,
                    expect_cluster_id);

    add_status = zigpc_attrmgmt_add_poll_entry(
                    expect_eui64,
                    expect_endpoint_id,
                    expect_cluster_id);

    //index at: dummy
    send_status = zigpc_attrmgmt_send_poll_attributes();
    //index at: dummy1
    send_status = zigpc_attrmgmt_send_poll_attributes();
    //index at: dummy2
    send_status = zigpc_attrmgmt_send_poll_attributes();
    //index at: expect
    send_status = zigpc_attrmgmt_send_poll_attributes();
    //index at: dummy
    send_status = zigpc_attrmgmt_send_poll_attributes();
    //index at: dummy1
    send_status = zigpc_attrmgmt_send_poll_attributes();
    //index at: dummy2
    send_status = zigpc_attrmgmt_send_poll_attributes();
    //index at: expect
    
    unsigned int test_size =
        zigpc_attrmgmt_get_poll_list_current_size();

    unsigned int test_max_size = 
        zigpc_attrmgmt_get_poll_list_max_size();

    zigbee_eui64_uint_t test_eui64 = 0;
    zigbee_endpoint_id_t test_endpoint_id = 0;
    zcl_cluster_id_t test_cluster_id = 0;

    sl_status_t get_status = 
                zigpc_attrmgmt_get_next_poll_entry(
                    test_eui64,
                    test_endpoint_id,
                    test_cluster_id);

    zigbee_eui64_uint_t expect_eui64_uint =
        zigbee_eui64_to_uint(expect_eui64);
    
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, add_status);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, get_status);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, send_status);
  TEST_ASSERT_EQUAL(4, test_size);
  TEST_ASSERT_EQUAL(ZIGPC_ATTRMGMT_DEFAULT_POLL_SIZE, test_max_size);

  TEST_ASSERT_EQUAL(expect_eui64_uint, test_eui64);
  TEST_ASSERT_EQUAL_UINT(expect_endpoint_id,test_endpoint_id);
  TEST_ASSERT_EQUAL_UINT(expect_cluster_id,test_cluster_id);
    
  zigpc_attrmgmt_empty_poll_list();
}

void test_zigpc_attrmgmt_poll_sends_read_attribute_cmd(void)
{
  // ARRANGE
  size_t attr_count = 21U;
  std::vector<zcl_attribute_id_t> attr_ids;
  std::vector<zcl_attribute_t> attr_info_list;

    zigbee_eui64_t expect_eui64
        = {0x04, 0x54, 0x02, 0xF2, 0x0A, 0x4C, 0x99, 0x10};
    zigbee_endpoint_id_t expect_endpoint_id = 1;
    zcl_cluster_id_t expect_cluster_id = 1;

  for (size_t i = 0; i < attr_count; i++) {
    attr_ids.push_back(0xFFFF - i);
    attr_info_list.push_back({0, attr_ids[i]});
  }

  // Expect functions called in zigpc_attrmgmt_send_split_read_cmds()
  helper_expect_zcl_frame_calls(expect_eui64,
                                expect_endpoint_id,
                                expect_cluster_id,
                                1,
                                attr_ids);

  zigpc_zclprofiles_get_cluster_attribute_count_ExpectAndReturn(expect_cluster_id ,
                                                                attr_count);

  zigpc_zclprofiles_get_cluster_attribute_list_ExpectAndReturn(expect_cluster_id ,
                                                               NULL,
                                                               SL_STATUS_OK);
  zigpc_zclprofiles_get_cluster_attribute_list_IgnoreArg_attribute_list();
  zigpc_zclprofiles_get_cluster_attribute_list_ReturnArrayThruPtr_attribute_list(
    attr_info_list.data(),
    attr_info_list.size());

  // ACT
    zigpc_attrmgmt_empty_poll_list();
    
    sl_status_t add_status = zigpc_attrmgmt_add_poll_entry(
                    expect_eui64,
                    expect_endpoint_id,
                    expect_cluster_id);

    sl_status_t send_status = zigpc_attrmgmt_send_poll_attributes();
  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, add_status);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, send_status);
}

}
