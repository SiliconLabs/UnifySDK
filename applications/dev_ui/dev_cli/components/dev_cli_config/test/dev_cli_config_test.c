/*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "dev_cli_config.h"
#include "dev_cli_config_fixt.h"
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
  char *argv_inject[3] = {"test_config", "--conf", TEST_CONFIG_FILE};
  const char *ini_content
    = "dev_cli:\n"
      "    command: help\n"
      "    dsk: 49930-50236-27477-27800-61487-65122-22863-43703\n"
      "    include: false\n"
      "    protocolunid: 213\n"
      "    unid: 1231\n"
      "mqtt:\n"
      "    host: localhost\n"
      "    port: 1883\n";

  TEST_ASSERT_TRUE_MESSAGE(
    create_file_with_content(TEST_CONFIG_FILE, ini_content),
    "Failed to create config file");
  TEST_ASSERT_EQUAL_MESSAGE(0,
                            dev_cli_config_init(),
                            "dev_cli_config_init failed");

  config_parse(sizeof(argv_inject) / sizeof(char *),
               argv_inject,
               "test version");
  dev_cli_config_fixt_setup();
  TEST_ASSERT_EQUAL_STRING("localhost", dev_cli_get_config()->mqtt_host);
  TEST_ASSERT_EQUAL(1883, dev_cli_get_config()->mqtt_port);
  TEST_ASSERT_EQUAL_STRING("help", dev_cli_get_config()->command);
  TEST_ASSERT_EQUAL_STRING("49930-50236-27477-27800-61487-65122-22863-43703",
                           dev_cli_get_config()->dsk);
  TEST_ASSERT_EQUAL_STRING("false", dev_cli_get_config()->include);
  TEST_ASSERT_EQUAL_STRING("213", dev_cli_get_config()->protocolunid);
  TEST_ASSERT_EQUAL_STRING("1231", dev_cli_get_config()->unid);
}
