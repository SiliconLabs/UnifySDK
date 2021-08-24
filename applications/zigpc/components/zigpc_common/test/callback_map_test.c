#include <string.h>
#include <stdbool.h>

#include "unity.h"
#include "sl_status.h"
#include "callback_map.h"
#include "zigpc_common_zigbee.h"

const zcl_command_t test_command = {.command_id = 0x01, .cluster_id = 0x01};

void suiteSetUp(void) {}

int suiteTearDown(int num_failures)
{
  return 0;
}

void create_callback_map_test(void)
{
  //ARRANGE
  //ACT
  callback_map_t test_map = create_callback_map();

  //ASSERT
  TEST_ASSERT_NOT_NULL(&test_map);
  TEST_ASSERT_NULL(test_map.pair_list);
  TEST_ASSERT_EQUAL(0, test_map.size);
}

void insert_standard_callback_succeeds_test(void)
{
  //ARRANGE
  callback_map_t test_map = create_callback_map();
  const char key[]        = "TestKey";

  TEST_ASSERT_EQUAL(0, test_map.size);

  //ACT
  sl_status_t status = insert_callback_map(&test_map, key, test_command);

  //ASSERT
  int same_key = strncmp(test_map.pair_list[0].key, key, MAX_KEY_SIZE);
  bool same_callback
    = (test_map.pair_list[0].command.cluster_id == test_command.cluster_id)
      && (test_map.pair_list[0].command.command_id == test_command.command_id);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(1, test_map.size);
  TEST_ASSERT_EQUAL(0, same_key);
  TEST_ASSERT_EQUAL(true, same_callback);
}

void insert_standard_callback_fails_test(void)
{
  //ARRANGE
  callback_map_t test_map = create_callback_map();
  const char *key         = NULL;

  //ACT
  sl_status_t status = insert_callback_map(&test_map, key, test_command);

  //ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, status);
  TEST_ASSERT_EQUAL(0, test_map.size);
}

void insert_pair_callback_succeeds_test(void)
{
  //ARRANGE
  callback_map_t test_map = create_callback_map();
  const char key[]        = "TestKey";

  zcl_key_value_t test_pair;
  strncpy(test_pair.key, key, MAX_KEY_SIZE);
  test_pair.command = test_command;

  TEST_ASSERT_EQUAL(0, test_map.size);

  //ACT
  sl_status_t status = insert_pair_callback_map(&test_map, &test_pair);

  //ASSERT
  int same_key = strncmp(test_map.pair_list[0].key, key, MAX_KEY_SIZE);
  bool same_callback
    = (test_map.pair_list[0].command.cluster_id == test_command.cluster_id)
      && (test_map.pair_list[0].command.command_id == test_command.command_id);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(1, test_map.size);
  TEST_ASSERT_EQUAL(0, same_key);
  TEST_ASSERT_EQUAL(true, same_callback);
}

void remove_callback_map_test(void)
{
  //ARRANGE
  callback_map_t test_map = create_callback_map();
  const char key[]        = "TestKey";
  sl_status_t status      = insert_callback_map(&test_map, key, test_command);

  zcl_command_t null_command = {.cluster_id = 0, .command_id = 0};
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(1, test_map.size);

  //ACT
  status = remove_callback_map(&test_map, key);

  //ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(0, test_map.size);
  TEST_ASSERT_EQUAL('\0', *test_map.pair_list[0].key);
  TEST_ASSERT_EQUAL(null_command.cluster_id,
                    test_map.pair_list[0].command.cluster_id);
  TEST_ASSERT_EQUAL(null_command.command_id,
                    test_map.pair_list[0].command.command_id);
}

void retrieve_callback_succeeds_map(void)
{
  //ARRANGE
  callback_map_t test_map = create_callback_map();
  const char key[]        = "TestKey";
  sl_status_t status      = insert_callback_map(&test_map, key, test_command);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(1, test_map.size);

  //ACT
  int index;
  zcl_command_t retrieved = retrieve_callback_map(&test_map, key, &index);

  //ASSERT
  TEST_ASSERT_EQUAL(test_command.cluster_id, retrieved.cluster_id);
  TEST_ASSERT_EQUAL(test_command.command_id, retrieved.command_id);
  TEST_ASSERT_EQUAL(0, index);
}

void retrieve_callback_fails_map(void)
{
  //ARRANGE
  callback_map_t test_map = create_callback_map();
  const char key[]        = "TestKey";
  const char other_key[]  = "OtherKey";

  sl_status_t status = insert_callback_map(&test_map, key, test_command);

  //ACT
  int index;
  zcl_command_t retrieved = retrieve_callback_map(&test_map, other_key, &index);

  zcl_command_t null_command = {.cluster_id = 0, .command_id = 0};
  //ASSERT
  TEST_ASSERT_EQUAL(status, SL_STATUS_OK);
  TEST_ASSERT_EQUAL(-1, index);
  TEST_ASSERT_EQUAL(null_command.cluster_id, retrieved.cluster_id);
  TEST_ASSERT_EQUAL(null_command.command_id, retrieved.command_id);
}
