/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "config.h"
#include "unity.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "cpp_includes.h"

#define TEST_CONFIG_FILE "_test_config.ini"
extern "C" {
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
  config_reset();
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

void test_int_arg()
{
  TEST_ASSERT_EQUAL(
    CONFIG_STATUS_OK,
    config_add_int("dummy_int", "Serial port for Z-Wave Radio", 42));
  int res;
  const char *argv_inject[] = {"test_config", "--dummy_int", "1337"};
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_OK,
                            config_parse(sizeof(argv_inject) / sizeof(char *),
                                         const_cast<char **>(argv_inject),
                                         "1.0"),
                            "zpc_config_init failed");
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK, config_get_as_int("dummy_int", &res));
  TEST_ASSERT_EQUAL(1337, res);
}

void test_string_arg()
{
  config_add_string("serial", "Serial port for Z-Wave Radio", "/dev/ttyUSB0");
  const char *argv_inject[]
    = {"test_config", "--conf", TEST_CONFIG_FILE, "--serial", "myserial"};
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_OK,
                            config_parse(sizeof(argv_inject) / sizeof(char *),
                                         const_cast<char **>(argv_inject),
                                         "1.0"),
                            "zpc_config_init failed");
  const char *str;
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_OK,
                            config_get_as_string("serial", &str),
                            "Failed to get serial");
  TEST_ASSERT_EQUAL_STRING_MESSAGE("myserial",
                                   str,
                                   "Serial string doesn't match");
}

void test_double_arg()
{
  config_add_double("pi", "PI", 3.14);
  const char *argv_inject[]
    = {"test_config", "--conf", TEST_CONFIG_FILE, "--pi", "3.14159265"};
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_OK,
                            config_parse(sizeof(argv_inject) / sizeof(char *),
                                         const_cast<char **>(argv_inject),
                                         "1.0"),
                            "zpc_config_init failed");
  double res;
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_OK,
                            config_get_as_double("pi", &res),
                            "Failed to get pi");
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(3.14159265,
                                   res,
                                   "Double argument doesn't match");
}

void test_bool_arg()
{
  config_add_bool("myflag", "My flag", false);
  const char *argv_inject[]
    = {"test_config", "--conf", TEST_CONFIG_FILE, "--myflag", "true"};
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_OK,
                            config_parse(sizeof(argv_inject) / sizeof(char *),
                                         const_cast<char **>(argv_inject),
                                         "1.0"),
                            "zpc_config_init failed");
  bool res;
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_OK,
                            config_get_as_bool("myflag", &res),
                            "Failed to get myflag");
  TEST_ASSERT_TRUE_MESSAGE(res, "bool argument doesn't match");
}

void test_flag()
{
  config_add_flag("myflag", "My flag");
  const char *argv_inject[]
    = {"test_config", "--conf", TEST_CONFIG_FILE, "--myflag"};
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_OK,
                            config_parse(sizeof(argv_inject) / sizeof(char *),
                                         const_cast<char **>(argv_inject),
                                         "1.0"),
                            "zpc_config_init failed");

  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_OK,
                            config_has_flag("myflag"),
                            "Failed to get myflag");
}

void test_config_file()
{
  const char *ini_content = "zpc:\n"
                            "    serial: /dev/ttyUSB0\n"
                            "    baudrate: 115200\n"
                            "    testflag: true\n"
                            "    pi: 3.14159"
                            "\n"
                            "mqtt:\n"
                            "    host: localhost\n"
                            "unused:\n"
                            "    for_test: doesn't exist";
  TEST_ASSERT_TRUE_MESSAGE(
    create_file_with_content(TEST_CONFIG_FILE, ini_content),
    "Failed to create config file");
  const char *argv_inject[] = {"test_config", "--conf", TEST_CONFIG_FILE};
  config_add_string("zpc.serial", "Zwave radio serial port", "/dev/ttyAMC0");
  // Setup options
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK,
                    config_add_int("zpc.baudrate", "Baudrate", 57600));
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK,
                    config_add_bool("zpc.testflag", "some testflag", false));
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK, config_add_double("zpc.pi", "PI", 3.14));
  // Parse options
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_OK,
                            config_parse(sizeof(argv_inject) / sizeof(char *),
                                         const_cast<char **>(argv_inject),
                                         "1.0"),
                            "zpc_config_init failed");
  // Check results
  const char *result;
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK,
                    config_get_as_string("zpc.serial", &result));
  TEST_ASSERT_EQUAL_STRING("/dev/ttyUSB0", result);

  int result_int;
  TEST_ASSERT_EQUAL(CONFIG_STATUS_DOES_NOT_EXIST,
                    config_get_as_int("unused.for_test", &result_int));
  TEST_ASSERT_EQUAL(CONFIG_STATUS_INVALID_TYPE,
                    config_get_as_int("zpc.serial", &result_int));
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK,
                    config_get_as_int("zpc.baudrate", &result_int));
  TEST_ASSERT_EQUAL(115200, result_int);

  double result_double;
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK,
                    config_get_as_double("zpc.pi", &result_double));
  TEST_ASSERT_EQUAL_DOUBLE(3.14159, result_double);

  bool result_bool;
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK,
                    config_get_as_bool("zpc.testflag", &result_bool));
  TEST_ASSERT_TRUE(result_bool);
}

void test_config_file_override()
{
  const char *ini_content = "zpc:\n"
                            "    serial: /dev/ttyUSB0\n";
  TEST_ASSERT_TRUE_MESSAGE(
    create_file_with_content(TEST_CONFIG_FILE, ini_content),
    "Failed to create config file");
  const char *argv_inject[] = {"test_config",
                               "--conf",
                               TEST_CONFIG_FILE,
                               "--zpc.serial",
                               "/dev/ttyUSB1"};
  config_add_string("zpc.serial", "Zwave radio serial port", "/dev/ttyAMC0");
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_OK,
                            config_parse(sizeof(argv_inject) / sizeof(char *),
                                         const_cast<char **>(argv_inject),
                                         "1.0"),
                            "zpc_config_init failed");
  const char *result;
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK,
                    config_get_as_string("zpc.serial", &result));
  TEST_ASSERT_EQUAL_STRING("/dev/ttyUSB1", result);
}

void test_config_help()
{
  // Help message is not actually validated, only return code
  config_add_int(

    "myint",
    "Answer to the Ultimate Question of Life, the Universe, and Everything.",
    42);
  config_add_bool("mybool", "This statment is false!", true);
  const char *argv_inject[] = {
    "test_config",
    "--help",
  };
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_INFO_MESSAGE,
                            config_parse(sizeof(argv_inject) / sizeof(char *),
                                         const_cast<char **>(argv_inject),
                                         "1.0"),
                            "zpc_config_init failed");
}

void test_config_version()
{
  // Version message is not actually validated, only return code
  const char *argv_inject[] = {
    "test_config",
    "--version",
  };
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_INFO_MESSAGE,
                            config_parse(sizeof(argv_inject) / sizeof(char *),
                                         const_cast<char **>(argv_inject),
                                         "1.0"),
                            "zpc_config_init failed");
}

void test_config_invalid_config_file()
{
  const char *ini_content = "zpc:\n"
                            "    serial:\n"
                            "\n"
                            "mqtt:\n"
                            "    host: localhost\n";
  TEST_ASSERT_TRUE_MESSAGE(
    create_file_with_content(TEST_CONFIG_FILE, ini_content),
    "Failed to create config file");
  const char *argv_inject[] = {"test_config", "--conf", TEST_CONFIG_FILE};
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_ERROR,
                            config_parse(sizeof(argv_inject) / sizeof(char *),
                                         const_cast<char **>(argv_inject),
                                         "1.0"),
                            "Expect parse to fail");
}

void test_config_invalid_arg()
{
  const char *argv_inject[] = {
    "test_config",
    "--fubar",
  };
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_ERROR,
                            config_parse(sizeof(argv_inject) / sizeof(char *),
                                         const_cast<char **>(argv_inject),
                                         "1.0"),
                            "Expect parse to fail");
}

void test_config_mqtt_arg()
{
  const char *ini_content = "mqtt:\n"
                            "    host: localhost\n";
  TEST_ASSERT_TRUE_MESSAGE(
    create_file_with_content(TEST_CONFIG_FILE, ini_content),
    "Failed to create config file");
  const char *argv_inject[] = {"test_config", "--conf", TEST_CONFIG_FILE};
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_OK,
                            config_parse(sizeof(argv_inject) / sizeof(char *),
                                         const_cast<char **>(argv_inject),
                                         "1.0"),
                            "zpc_config_init failed");
  const char *str;
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_OK,
                            config_get_as_string(CONFIG_KEY_MQTT_HOST, &str),
                            "Failed to get serial");
  TEST_ASSERT_EQUAL_STRING_MESSAGE("localhost",
                                   str,
                                   "Serial string doesn't match");
}

void test_config_log_arg()
{
  const char *ini_content = "log:\n"
                            "    tags:\n"
                            "        uic_main: debug\n"
                            "        datastore: info\n"
                            "    level: debug\n"
                            "    tag_level: \"mqtt:w, uic_main:i,\n"
                            "      zwapi:d\"";
  TEST_ASSERT_TRUE_MESSAGE(
    create_file_with_content(TEST_CONFIG_FILE, ini_content),
    "Failed to create config file");
  const char *argv_inject[] = {"test_config", "--conf", TEST_CONFIG_FILE};
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_OK,
                            config_parse(sizeof(argv_inject) / sizeof(char *),
                                         const_cast<char **>(argv_inject),
                                         "1.0"),
                            "zpc_config_init failed");
  const char *level;
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK,
                    config_get_as_string(CONFIG_KEY_LOG_LEVEL, &level));
  TEST_ASSERT_EQUAL_STRING("debug", level);

  const char *tag_level;
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK,
                    config_get_as_string(CONFIG_KEY_LOG_TAG_LEVEL, &tag_level));
  TEST_ASSERT_EQUAL_STRING("mqtt:w, uic_main:i, zwapi:d", tag_level);
}

void test_invalid_config_get()
{
  int result_int;
  TEST_ASSERT_EQUAL(CONFIG_STATUS_DOES_NOT_EXIST,
                    config_get_as_int("Does not exist", &result_int));
  const char *result_str;
  TEST_ASSERT_EQUAL(CONFIG_STATUS_DOES_NOT_EXIST,
                    config_get_as_string("test.not_exist", &result_str));
}
void test_config_tls_certs()
{
  const char *ini_content = "mqtt:\n"
                            "    cafile: /tmp/cafile\n"
                            "    certfile: /tmp/certfile\n"
                            "    keyfile: /tmp/keyfile\n";
  TEST_ASSERT_TRUE_MESSAGE(
    create_file_with_content(TEST_CONFIG_FILE, ini_content),
    "Failed to create config file");
  const char *argv_inject[] = {"test_config", "--conf", TEST_CONFIG_FILE};
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_OK,
                            config_parse(sizeof(argv_inject) / sizeof(char *),
                                         const_cast<char **>(argv_inject),
                                         "1.0"),
                            "zpc_config_init failed");
  const char *cafile;
  const char *certfile;
  const char *keyfile;
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK,
                    config_get_as_string(CONFIG_KEY_MQTT_CAFILE, &cafile));
  TEST_ASSERT_EQUAL_STRING("/tmp/cafile", cafile);
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK,
                    config_get_as_string(CONFIG_KEY_MQTT_CERTFILE, &certfile));
  TEST_ASSERT_EQUAL_STRING("/tmp/certfile", certfile);
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK,
                    config_get_as_string(CONFIG_KEY_MQTT_KEYFILE, &keyfile));
  TEST_ASSERT_EQUAL_STRING("/tmp/keyfile", keyfile);
}

void test_config_tls_certs_default()
{
  const char *ini_content = "mqtt:\n"
                            "    port: 1083\n";
  TEST_ASSERT_TRUE_MESSAGE(
    create_file_with_content(TEST_CONFIG_FILE, ini_content),
    "Failed to create config file");
  const char *argv_inject[] = {"test_config", "--conf", TEST_CONFIG_FILE};
  TEST_ASSERT_EQUAL_MESSAGE(CONFIG_STATUS_OK,
                            config_parse(sizeof(argv_inject) / sizeof(char *),
                                         const_cast<char **>(argv_inject),
                                         "1.0"),
                            "zpc_config_init failed");
  const char *cafile;
  const char *certfile;
  const char *keyfile;
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK,
                    config_get_as_string(CONFIG_KEY_MQTT_CAFILE, &cafile));
  TEST_ASSERT_EQUAL_STRING("", cafile);
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK,
                    config_get_as_string(CONFIG_KEY_MQTT_CERTFILE, &certfile));
  TEST_ASSERT_EQUAL_STRING("", certfile);
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK,
                    config_get_as_string(CONFIG_KEY_MQTT_KEYFILE, &keyfile));
  TEST_ASSERT_EQUAL_STRING("", keyfile);
}

void test_dump_config()
{
  // vm["zpc.serial"] = "/dev/ttyUSB0"
  // vm["zpc.poll.backoff"] = 42
  // vm["zpc.poll.default_interval"] = 500

  std::string expected(R"(# Unify sample conf file

conf: '/etc/uic/uic.cfg'
log:
  level: 'i'
  tag_level: ''
mqtt:
  cafile: ''
  certfile: ''
  client_id: 'test_config_$pid'
  host: 'localhost'
  keyfile: ''
  port: 1883
zpc:
  poll:
    backoff: 43
    default_interval: 44
  serial: '/dev/ttyUSB0'
)");
  expected.find("$pid_number");
  getpid();
  char pid[16];
  snprintf(pid, sizeof(pid), "%i", getpid());
  expected.replace(expected.find("$pid"), 4, pid);
  TEST_ASSERT_EQUAL(CONFIG_STATUS_OK,
                    config_add_string("zpc.serial", "TTY", "/dev/ttyUSB0"));
  TEST_ASSERT_EQUAL(
    CONFIG_STATUS_OK,
    config_add_int("zpc.poll.backoff", "level 2 configuration", 43));
  TEST_ASSERT_EQUAL(
    CONFIG_STATUS_OK,
    config_add_int("zpc.poll.default_interval", "level 3 configuration", 44));

  const char *argv_inject[] = {"test_config", "--dump-config"};
  std::ostringstream output;
  TEST_ASSERT_EQUAL_MESSAGE(
    CONFIG_STATUS_INFO_MESSAGE,
    config_parse_with_output(sizeof(argv_inject) / sizeof(char *),
                             const_cast<char **>(argv_inject),
                             "1.0",
                             output),
    "zpc_config_init failed");
  std::ostringstream error_message;
  ///printf("%s\n", output.str().c_str());
  TEST_ASSERT_EQUAL_STRING(expected.c_str(), output.str().c_str());
}
}