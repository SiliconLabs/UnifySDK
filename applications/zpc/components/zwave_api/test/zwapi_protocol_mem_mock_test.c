#include "zwapi_protocol_mem.h"
#include "unity.h"
#include <string.h>

void test_read_write()
{
  const char *test1_data = "my_first_data";
  char test1_result[14] = {1};
  zwapi_memory_put_buffer(0, (uint8_t *)test1_data, strlen(test1_data), NULL);
  zwapi_memory_get_buffer(0, (uint8_t *)test1_result, strlen(test1_data));
  TEST_ASSERT_EQUAL_STRING(test1_data, test1_result);
}
