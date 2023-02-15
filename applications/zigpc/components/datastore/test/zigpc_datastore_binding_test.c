
#include "unity.h"

#include "zigpc_datastore.h"
#include "zigpc_datastore_id.h"
#include "attribute_store_mock.h"

#define MOCK_ROOT_NODE_ID          1
#define MOCK_NWK_NODE_ID           2
#define MOCK_DEV_1_NODE_ID         3
#define MOCK_DEV_1_EP1_NODE_ID     4
#define MOCK_DEV_1_EP1_CL1_NODE_ID 5

uint8_t default_network_index = PRIMARY_NETWORK_NUM;

void suiteSetUp(void) {}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void helper_set_cluster_parents_overhead(zigbee_eui64_uint_t *eui64_i,
                                         zigbee_endpoint_id_t *endpoint_id,
                                         attribute_store_node_t return_nodeid)
{
  attribute_store_get_root_ExpectAndReturn(MOCK_ROOT_NODE_ID);
  attribute_store_node_exists_ExpectAndReturn(MOCK_ROOT_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_ROOT_NODE_ID,
    ZIGPC_DS_TYPE_NETWORK,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&default_network_index,
    sizeof(uint8_t),
    0,
    MOCK_NWK_NODE_ID);

  attribute_store_node_exists_ExpectAndReturn(MOCK_NWK_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_NWK_NODE_ID,
    ZIGPC_DS_TYPE_DEVICE,
    DESIRED_ATTRIBUTE,
    (uint8_t *)eui64_i,
    sizeof(zigbee_eui64_uint_t),
    0,
    MOCK_DEV_1_NODE_ID);

  attribute_store_node_exists_ExpectAndReturn(MOCK_DEV_1_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_DEV_1_NODE_ID,
    ZIGPC_DS_TYPE_ENDPOINT,
    DESIRED_ATTRIBUTE,
    (uint8_t *)endpoint_id,
    sizeof(zigbee_endpoint_id_t),
    0,
    return_nodeid);
}

void test_datastore_get_binding_count(void)
{
  // ARRANGE
  zigbee_eui64_t eui64             = "\x03\x55\xFF\x29\x34\x11\x4A\xB4";
  zigbee_eui64_uint_t eui64_i      = 0x0355FF2934114AB4;
  zigbee_endpoint_id_t endpoint_id = 7;

  helper_set_cluster_parents_overhead(&eui64_i,
                                      &endpoint_id,
                                      ATTRIBUTE_STORE_INVALID_NODE);

  // ACT
    size_t count =
        zigpc_datastore_get_binding_count(
            eui64,
            endpoint_id);

  // ASSERT
  TEST_ASSERT_EQUAL(0, count);

  
}

void test_zigpc_datastore_read_binding_list(void)
{
sl_status_t zigpc_datastore_read_binding_list( 
                const zigbee_eui64_t eui64,
                const zigbee_endpoint_id_t endpoint_id,
                zigbee_binding_t *const binding_list,
                size_t binding_list_size);
}
