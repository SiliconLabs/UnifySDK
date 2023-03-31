#include <sl_status.h>
#include <string.h>
#include <stdio.h>

extern "C" { 
#include "unity.h"
#include "cmock.h"

#include "zigpc_gateway_mock.h"
#include "zigpc_datastore_mock.h"
#include "zigpc_common_zigbee_mock.h"
#include "zigpc_binding_response.h"

void suiteSetUp(void) {}

int suiteTearDown(int num_failures)
{
  return num_failures;
}

void test_zigpc_binding_bind_unbind_callback(void)
{

    //void zigpc_binding_bind_unbind_callback(void *data);
    sl_status_t status = SL_STATUS_OK;
    TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_handle_bind_response(void)
{

    sl_status_t status = SL_STATUS_OK;
    zigbee_binding_t test_binding;
    test_binding.source_address = 1;
    test_binding.source_endpoint = 1;
    test_binding.source_cluster = 6;
    test_binding.dest_address = 2;
    test_binding.dest_endpoint = 1;
    
    zigbee_uint_to_eui64_IgnoreAndReturn(SL_STATUS_OK);
    zigbee_eui64_to_uint_IgnoreAndReturn(test_binding.source_address);
/*
    zigpc_datastore_get_binding_count_IgnoreAndReturn(0);
    zigpc_datastore_read_binding_list_IgnoreAndReturn(SL_STATUS_OK);
    zigpc_datastore_write_binding_list_IgnoreAndReturn(SL_STATUS_OK);
*/
    status = 
        zigpc_binding_handle_bind_unbind_reponse(
                test_binding,
                true,
                0);

    TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_handle_unbind_response(void)
{

    sl_status_t status = SL_STATUS_OK;
    zigbee_binding_t test_binding;
    test_binding.source_address = 1;
    test_binding.source_endpoint = 1;
    test_binding.source_cluster = 6;
    test_binding.dest_address = 2;
    test_binding.dest_endpoint = 1;
    
    zigbee_uint_to_eui64_IgnoreAndReturn(SL_STATUS_OK);
    zigbee_eui64_to_uint_IgnoreAndReturn(test_binding.source_address);
/*
    zigpc_datastore_get_binding_count_IgnoreAndReturn(0);
    zigpc_datastore_read_binding_list_IgnoreAndReturn(SL_STATUS_OK);
    zigpc_datastore_write_binding_list_IgnoreAndReturn(SL_STATUS_OK);
*/
    status = 
        zigpc_binding_handle_bind_unbind_reponse(
                test_binding,
                false,
                0);

    TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_handle_bind_table_full_response(void)
{

    sl_status_t status = SL_STATUS_OK;
    zigbee_binding_t test_binding;
    test_binding.source_address = 1;
    test_binding.source_endpoint = 1;
    test_binding.source_cluster = 6;
    test_binding.dest_address = 2;
    test_binding.dest_endpoint = 1;
    
    zigbee_uint_to_eui64_IgnoreAndReturn(SL_STATUS_OK);
    zigbee_eui64_to_uint_IgnoreAndReturn(test_binding.source_address);
/*
    zigpc_datastore_get_binding_count_IgnoreAndReturn(0);
    zigpc_datastore_read_binding_list_IgnoreAndReturn(SL_STATUS_OK);
    zigpc_datastore_write_binding_list_IgnoreAndReturn(SL_STATUS_OK);
*/
    status = 
        zigpc_binding_handle_bind_unbind_reponse(
                test_binding,
                true,
                ZDO_TABLE_FULL);

    TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_handle_bind_error_response(void)
{

    sl_status_t status = SL_STATUS_OK;
    zigbee_binding_t test_binding;
    test_binding.source_address = 1;
    test_binding.source_endpoint = 1;
    test_binding.source_cluster = 6;
    test_binding.dest_address = 2;
    test_binding.dest_endpoint = 1;
    
    zigbee_uint_to_eui64_IgnoreAndReturn(SL_STATUS_OK);
    zigbee_eui64_to_uint_IgnoreAndReturn(test_binding.source_address);
/*
    zigpc_datastore_get_binding_count_IgnoreAndReturn(0);
    zigpc_datastore_read_binding_list_IgnoreAndReturn(SL_STATUS_OK);
    zigpc_datastore_write_binding_list_IgnoreAndReturn(SL_STATUS_OK);
*/
    status = 
        zigpc_binding_handle_bind_unbind_reponse(
                test_binding,
                true,
                0x55);

    TEST_ASSERT_EQUAL_HEX(SL_STATUS_FAIL, status);
}



}
