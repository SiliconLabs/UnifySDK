
#include <sl_status.h>
#include <string.h>
#include <stdio.h>

extern "C" { 
#include "unity.h"
#include "cmock.h"

#include "zigpc_gateway_mock.h"
#include "zigpc_common_zigbee_mock.h"
#include "zigpc_datastore_mock.h"
#include "zigpc_binding_mqtt.h"

void suiteSetUp(void) {}

int suiteTearDown(int num_failures)
{
  return num_failures;
}

void test_zigpc_binding_mqtt_init_test(void)
{
    
    const zigbee_eui64_uint_t test_src_eui_uint = 1;
    zigbee_endpoint_id_t test_ep = 1;
   
    zigbee_uint_to_eui64_IgnoreAndReturn(SL_STATUS_OK);
    zigbee_eui64_to_uint_IgnoreAndReturn(test_src_eui_uint);
    zigpc_datastore_get_cluster_count_IgnoreAndReturn(0);

    sl_status_t status = 
        zigpc_binding_init_mqtt(
            test_src_eui_uint ,
            test_ep );

    TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_binding_mqtt_init_fails_test(void)
{
    
    const zigbee_eui64_uint_t test_src_eui_uint = 0;
    zigbee_endpoint_id_t test_ep = 1;
   
    zigbee_uint_to_eui64_IgnoreAndReturn(SL_STATUS_OK);
    zigbee_eui64_to_uint_IgnoreAndReturn(test_src_eui_uint);

    sl_status_t status = 
        zigpc_binding_init_mqtt(
            test_src_eui_uint ,
            test_ep );

    TEST_ASSERT_EQUAL_HEX(SL_STATUS_FAIL, status);
}

void test_zigpc_publish_binding_table(void)
{
    const zigbee_eui64_uint_t test_src_eui_uint = 1;
    zigbee_endpoint_id_t test_ep = 1;
    
    zigbee_uint_to_eui64_IgnoreAndReturn(SL_STATUS_OK);
    zigbee_eui64_to_uint_IgnoreAndReturn(test_src_eui_uint);
    zigpc_datastore_get_binding_count_IgnoreAndReturn(0);

    zigpc_datastore_read_binding_list_IgnoreAndReturn(SL_STATUS_OK);

    sl_status_t status = 
        zigpc_publish_binding_table(
                test_src_eui_uint,
                test_ep);
    
    TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_publish_binding_table_fails(void)
{
    const zigbee_eui64_uint_t test_src_eui_uint = 1;
    zigbee_endpoint_id_t test_ep = 1;
    
    zigbee_uint_to_eui64_IgnoreAndReturn(SL_STATUS_OK);
    zigbee_eui64_to_uint_IgnoreAndReturn(test_src_eui_uint);
    zigbee_eui64_to_uint_IgnoreAndReturn(test_src_eui_uint);
    zigpc_datastore_get_binding_count_IgnoreAndReturn(0);
    zigpc_datastore_read_binding_list_IgnoreAndReturn(SL_STATUS_FAIL);

    sl_status_t status = 
        zigpc_publish_binding_table(
                test_src_eui_uint,
                test_ep);
    
    TEST_ASSERT_EQUAL_HEX(SL_STATUS_FAIL, status);
}

void test_zigpc_binding_mqtt_publish_table_full(void)
{

    const zigbee_eui64_uint_t test_src_eui_uint = 1;
    zigbee_endpoint_id_t test_ep = 1;
    
    zigbee_uint_to_eui64_IgnoreAndReturn(SL_STATUS_OK);
    zigbee_eui64_to_uint_IgnoreAndReturn(test_src_eui_uint);
    
    sl_status_t status = 
        zigpc_binding_mqtt_publish_table_full(
            test_src_eui_uint ,
            test_ep ,
            true,
            true);
    
    TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_binding_mqtt_publish_bindable_clusters(void)
{
    const zigbee_eui64_uint_t test_src_eui_uint = 1;
    zigbee_endpoint_id_t test_ep = 1;
    
    zigbee_uint_to_eui64_IgnoreAndReturn(SL_STATUS_OK);
    zigbee_eui64_to_uint_IgnoreAndReturn(test_src_eui_uint);
    zigpc_datastore_get_cluster_count_IgnoreAndReturn(0);

    sl_status_t status =  
        zigpc_binding_mqtt_publish_bindable_clusters(
            test_src_eui_uint ,
            test_ep );
    
    TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

}
