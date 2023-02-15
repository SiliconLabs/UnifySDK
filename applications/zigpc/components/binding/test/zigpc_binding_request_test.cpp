
#include <sl_status.h>
#include <string.h>
#include <stdio.h>

extern "C" { 
#include "unity.h"
#include "cmock.h"

#include "zigpc_gateway_mock.h"
#include "zigpc_common_zigbee_mock.h"
#include "zigpc_binding_request.h"

void suiteSetUp(void) {}

int suiteTearDown(int num_failures)
{
  return num_failures;
}


void test_zigpc_binding_parse_bind_cmd(void)
{
    
    const char test_topic[] = "ucl/by-unid/zb-0000000000000001/ep1/Binding/Commands/Bind";
    const char test_message[] = 
        "{\"ClusterName\":\"OnOff\",\"DestinationUnid\":\"zb-0000000000000002\",\"DestinationEp\": 1}";
    const size_t message_length = strlen(test_message);

    const zigbee_eui64_uint_t test_src_eui_uint = 1;
    const zigbee_eui64_uint_t test_dest_eui_uint = 2;
    const zigbee_eui64_t test_src_eui = {0,0,0,0,0,0,0,1};
    const zigbee_eui64_t test_dest_eui = {0,0,0,0,0,0,0,2};

    zigbee_uint_to_eui64_ExpectAndReturn(
            test_src_eui_uint,
            NULL,
            SL_STATUS_OK);
    zigbee_uint_to_eui64_IgnoreArg_dest();
    
    zigbee_uint_to_eui64_ExpectAndReturn(
            test_dest_eui_uint,
            NULL,
            SL_STATUS_OK);
    zigbee_uint_to_eui64_IgnoreArg_dest();

    zigpc_gateway_request_binding_ExpectAndReturn(
            test_src_eui, 
            1,
            0x06,
            test_dest_eui, 
            1,
            SL_STATUS_OK);

    zigpc_binding_parse_bind_cmd(
            test_topic,
            test_message,
            message_length);

    //assert is in the above command through ExpectAndReturn
}

void test_zigpc_binding_parse_bind_cmd_fails(void)
{
    
    //compare and contrast with above
    //see that no calls to the gateway are made
    const char *test_topic = NULL;
    const char test_message[] = 
        "{\"ClusterName\":\"OnOff\",\"DestinationUnid\":\"zb-0000000000000002\",\"DestinationEp\": 1}";
    const size_t message_length = strlen(test_message);

    zigpc_binding_parse_bind_cmd(
            test_topic,
            test_message,
            message_length);

    //assert is in the above command through ExpectAndReturn
}

void test_zigpc_binding_parse_unbind_cmd(void)
{
    
    const char test_topic[] = "ucl/by-unid/zb-0000000000000001/ep1/Binding/Commands/Unbind";
    const char test_message[] = 
        "{\"ClusterName\":\"OnOff\",\"DestinationUnid\":\"zb-0000000000000002\",\"DestinationEp\": 1}";
    const size_t message_length = strlen(test_message);

    const zigbee_eui64_uint_t test_src_eui_uint = 1;
    const zigbee_eui64_uint_t test_dest_eui_uint = 2;
    const zigbee_eui64_t test_src_eui = {0,0,0,0,0,0,0,1};
    const zigbee_eui64_t test_dest_eui = {0,0,0,0,0,0,0,2};

    zigbee_uint_to_eui64_ExpectAndReturn(
            test_src_eui_uint,
            NULL,
            SL_STATUS_OK);
    zigbee_uint_to_eui64_IgnoreArg_dest();
    
    zigbee_uint_to_eui64_ExpectAndReturn(
            test_dest_eui_uint,
            NULL,
            SL_STATUS_OK);
    zigbee_uint_to_eui64_IgnoreArg_dest();

    zigpc_gateway_request_unbind_ExpectAndReturn(
            test_src_eui, 
            1,
            0x06,
            test_dest_eui, 
            1,
            SL_STATUS_OK);

    zigpc_binding_parse_unbind_cmd(
            test_topic,
            test_message,
            message_length);

    //assert is in the above command through ExpectAndReturn
}


void test_zigpc_binding_parse_cmd_args(void)
{
    const char test_topic[] = "ucl/by-unid/zb-0000000000000001/ep1/Binding/Commands/Bind";
    const char test_message[] = 
        "{\"ClusterName\":\"OnOff\",\"DestinationUnid\":\"zb-0000000000000002\",\"DestinationEp\": 1}";
    const size_t message_length = strlen(test_message);
    zigbee_binding_t binding = NULL_BINDING;

    sl_status_t status =
        zigpc_binding_parse_cmd_args(
            test_topic,
            test_message,
            message_length,
            &binding);

    TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
    TEST_ASSERT_EQUAL(1, binding.source_address);
    TEST_ASSERT_EQUAL(1, binding.source_endpoint);
    TEST_ASSERT_EQUAL(6, binding.source_cluster);
    TEST_ASSERT_EQUAL(2, binding.dest_address);
    TEST_ASSERT_EQUAL(1, binding.dest_endpoint);
}

}
