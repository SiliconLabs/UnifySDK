/*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "config.h"
#include "zpc_config.h"
#include "zpc_config_fixt.h"
#include "unity.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define TEST_CONFIG_FILE "_test_config.ini"

static void remove_test_config_file()
{
  if (access(TEST_CONFIG_FILE, F_OK) != -1) {
    remove(TEST_CONFIG_FILE);
  }
}

void setUp()
{
  // Ensure test config file doesn't exist
  remove_test_config_file();
}

void tearDown()
{
  // Remove test config file on teardown
  remove_test_config_file();
}

static bool create_file_with_content(const char *filename, const char *content)
{
  FILE *fpth = fopen(filename, "w");
  if (fpth == NULL) {
    return false;
  }
  int result = fputs(content, fpth);
  fclose(fpth);

  return (result > 0);
}

void test_init()
{
  char *argv_inject[3]    = {"test_config", "--conf", TEST_CONFIG_FILE};
  const char *ini_content = "zpc:\n"
                            "    serial: /dev/ttyUSB0\n"
                            "    datastore_file: myfile.db\n"
                            "    manufacturer_id: 4660\n"
                            "    product_type: 5789\n"
                            "    product_id: 2130\n"
                            "    device_id: 1234567890ABCDEF\n"
                            "\n"
                            "mqtt:\n"
                            "    host: localhost\n"
                            "    port: 2000\n"
                            "    hardware_version: 1\n";
  TEST_ASSERT_TRUE_MESSAGE(
    create_file_with_content(TEST_CONFIG_FILE, ini_content),
    "Failed to create config file");
  TEST_ASSERT_EQUAL_MESSAGE(0, zpc_config_init(), "zpc_config_init failed");
  config_parse(sizeof(argv_inject) / sizeof(char *),
               argv_inject,
               "test version");
  zpc_config_fixt_setup();
  TEST_ASSERT_EQUAL_STRING("localhost", zpc_get_config()->mqtt_host);
  TEST_ASSERT_EQUAL(2000, zpc_get_config()->mqtt_port);
  TEST_ASSERT_EQUAL_STRING("/dev/ttyUSB0", zpc_get_config()->serial_port);
  TEST_ASSERT_EQUAL(4660, zpc_get_config()->manufacturer_id);
  TEST_ASSERT_EQUAL(5789, zpc_get_config()->product_type);
  TEST_ASSERT_EQUAL(2130, zpc_get_config()->product_id);
  TEST_ASSERT_EQUAL_STRING("1234567890ABCDEF", zpc_get_config()->device_id);
  TEST_ASSERT_EQUAL(1, zpc_get_config()->hardware_version);
}
