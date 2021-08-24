#include <string.h>
#include <stdbool.h>

#include "unity.h"
#include "sl_status.h"
#include "zigpc_common_zigbee.h"
#include "hash_map.h"

const zcl_command_t test_command = {.command_id = 0x01, .cluster_id = 0x01};

void suiteSetUp(void) {}

int suiteTearDown(int num_failures)
{
  return 0;
}

void create_hash_map_test(void)
{
  //ARRANGE
  //ACT
  hash_map_t test_map = create_hash_map();

  //ASSERT
  TEST_ASSERT_NOT_NULL(&test_map);
  TEST_ASSERT_NULL(test_map.base.pair_list);
  TEST_ASSERT_EQUAL(0, test_map.base.size);
}

void insert_standard_hash_succeeds_test(void)
{
  //ARRANGE
  hash_map_t test_map = create_hash_map();
  const char key[]    = "TestKey";

  TEST_ASSERT_EQUAL(0, test_map.base.size);

  //ACT
  sl_status_t status = insert_hash_map(&test_map, key, test_command);

  //ASSERT
  unsigned int index = test_map.hash_func(key);

  int same_key = strncmp(test_map.base.pair_list[index].key, key, MAX_KEY_SIZE);
  bool same_callback = (test_map.base.pair_list[index].command.command_id
                        == test_command.command_id)
                       && (test_map.base.pair_list[index].command.cluster_id
                           == test_command.cluster_id);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(1, test_map.base.size);
  TEST_ASSERT_EQUAL(0, same_key);
  TEST_ASSERT_EQUAL(true, same_callback);
}

void insert_standard_hash_fails_test(void)
{
  //ARRANGE
  hash_map_t test_map = create_hash_map();
  const char *key     = NULL;

  //ACT
  sl_status_t status = insert_hash_map(&test_map, key, test_command);

  //ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, status);
  TEST_ASSERT_EQUAL(0, test_map.base.size);
}

void insert_pair_hash_succeeds_test(void)
{
  //ARRANGE
  hash_map_t test_map = create_hash_map();
  const char key[]    = "TestKey";

  zcl_key_value_t test_pair;
  strncpy(test_pair.key, key, MAX_KEY_SIZE);
  test_pair.command = test_command;

  TEST_ASSERT_EQUAL(0, test_map.base.size);

  //ACT
  sl_status_t status = insert_pair_hash_map(&test_map, &test_pair);

  //ASSERT
  unsigned int index = test_map.hash_func(key);
  int same_key = strncmp(test_map.base.pair_list[index].key, key, MAX_KEY_SIZE);

  bool same_callback = (test_map.base.pair_list[index].command.command_id
                        == test_command.command_id)
                       && (test_map.base.pair_list[index].command.cluster_id
                           == test_command.cluster_id);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(1, test_map.base.size);
  TEST_ASSERT_EQUAL(0, same_key);
  TEST_ASSERT_EQUAL(true, same_callback);
}

void remove_hash_map_test(void)
{
  //ARRANGE
  hash_map_t test_map = create_hash_map();
  const char key[]    = "TestKey";
  sl_status_t status  = insert_hash_map(&test_map, key, test_command);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(1, test_map.base.size);

  //ACT
  status = remove_hash_map(&test_map, key);

  //ASSERT
  unsigned int index = test_map.hash_func(key);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(0, test_map.base.size);
  TEST_ASSERT_EQUAL('\0', *test_map.base.pair_list[index].key);
  TEST_ASSERT_EQUAL(0, test_map.base.pair_list[index].command.cluster_id);
  TEST_ASSERT_EQUAL(0, test_map.base.pair_list[index].command.command_id);
}

void retrieve_hash_succeeds_map(void)
{
  //ARRANGE
  hash_map_t test_map = create_hash_map();
  const char key[]    = "TestKey";
  sl_status_t status  = insert_hash_map(&test_map, key, test_command);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(1, test_map.base.size);

  //ACT
  zcl_command_t retrieved = retrieve_hash_map(&test_map, key);

  //ASSERT
  TEST_ASSERT_EQUAL(test_command.command_id, retrieved.command_id);
  TEST_ASSERT_EQUAL(test_command.cluster_id, retrieved.cluster_id);
}

void retrieve_hash_fails_map(void)
{
  //ARRANGE
  hash_map_t test_map    = create_hash_map();
  const char key[]       = "TestKey";
  const char other_key[] = "OtherKey";

  sl_status_t status = insert_hash_map(&test_map, key, test_command);

  //ACT
  zcl_command_t retrieved = retrieve_hash_map(&test_map, other_key);

  //ASSERT
  TEST_ASSERT_EQUAL(status, SL_STATUS_OK);
  TEST_ASSERT_EQUAL(0, retrieved.command_id);
  TEST_ASSERT_EQUAL(0, retrieved.cluster_id);
}
