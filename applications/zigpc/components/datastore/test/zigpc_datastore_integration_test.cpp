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

#include "zigpc_datastore_test.hpp"

extern "C" {

// Includes from shared components
#include <unity.h>
#include <datastore.h>
#include <datastore_fixt.h>
#include <attribute_store_fixt.h>

// Component includes
#include "zigpc_datastore.h"

/**
 * @brief Setup the test suite (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void) {}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * @brief Setup before each test case
 *
 */
void setUp(void)
{
  // Initialize in-memory DB session
  datastore_init(":memory:");
  attribute_store_init();
}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown(void)
{
  attribute_store_teardown();
  datastore_teardown();
}

void helper_populate_cluster(zigbee_eui64_t eui64,
                             zigbee_endpoint_id_t endpoint_id,
                             zcl_cluster_side_t cluster_side,
                            nlohmann::json &jsn_endpoint)
{
  sl_status_t status;

  std::string cluster_str((cluster_side == ZCL_CLUSTER_SERVER_SIDE) ? "server"
                                                                    : "client");
  cluster_str += "_clusters";

  for (auto &jsn_cluster: jsn_endpoint[cluster_str]) {
    zcl_cluster_id_t cluster_id = jsn_cluster["cluster_id"].get<zcl_cluster_id_t>();

    status = zigpc_datastore_create_cluster(eui64,
                                            endpoint_id,
                                            cluster_side,
                                            cluster_id);

    std::string log_str = "Unable to create clusterID: " + cluster_id;
    log_str += "under endpoint " + endpoint_id;
    TEST_ASSERT_EQUAL_HEX_MESSAGE(SL_STATUS_OK, status, log_str.c_str());

    // Attribute List
    std::vector<zcl_attribute_id_t> attributes;
    for (auto &jsn_attribute: jsn_cluster["attributes"]) {
      zcl_attribute_id_t attribute_id = static_cast<zcl_attribute_id_t>(jsn_attribute);
      attributes.push_back(attribute_id);
    }
    if (!attributes.empty()) {
      status  = zigpc_datastore_write_attribute_list(eui64,
                                                    endpoint_id,
                                                    cluster_side,
                                                    cluster_id,
                                                    attributes.data(),
                                                    attributes.size());
      log_str = "Unable to create AttributeList under endpoint: " + endpoint_id;
      log_str += " clusterID: " + cluster_id;
      TEST_ASSERT_EQUAL_HEX_MESSAGE(SL_STATUS_OK, status, log_str.c_str());
    }

    // Receieved Command List
    std::vector<zcl_command_id_t> recv_commands;
    for (auto &jsn_command: jsn_cluster["recv_commands"]) {
      zcl_command_id_t command_id = static_cast<zcl_command_id_t>(jsn_command);
      recv_commands.push_back(command_id);
    }
    if (!recv_commands.empty()) {
      status = zigpc_datastore_write_command_list(eui64,
                                                  endpoint_id,
                                                  cluster_side,
                                                  cluster_id,
                                                  ZCL_CLUSTER_RECEIVED_COMMAND,
                                                  recv_commands.data(),
                                                  recv_commands.size());

      log_str
        = "Unable to create ReceviedCommandList under endpoint: " + endpoint_id;
      log_str += " clusterID: " + cluster_id;
      TEST_ASSERT_EQUAL_HEX_MESSAGE(SL_STATUS_OK, status, log_str.c_str());
    }

    // Generated Command List
    std::vector<zcl_command_id_t> gen_commands;
    for (auto &jsn_command: jsn_cluster["gen_commands"]) {
      zcl_command_id_t command_id = static_cast<zcl_command_id_t>(jsn_command);
      gen_commands.push_back(command_id);
    }
    if (!gen_commands.empty()) {
      status = zigpc_datastore_write_command_list(eui64,
                                                  endpoint_id,
                                                  cluster_side,
                                                  cluster_id,
                                                  ZCL_CLUSTER_GENERATED_COMMAND,
                                                  gen_commands.data(),
                                                  gen_commands.size());

      log_str = "Unable to create GeneratedCommandList under endpoint: "
                + endpoint_id;
      log_str += " clusterID: " + cluster_id;
      TEST_ASSERT_EQUAL_HEX_MESSAGE(SL_STATUS_OK, status, log_str.c_str());
    }
  }
}

void helper_populate_device(zigbee_eui64_t eui64,
                            std::stringstream &dev_config_str)
{
  sl_status_t status;

  nlohmann::json jsn = nlohmann::json::parse(dev_config_str);

  // Population
  status = zigpc_datastore_create_network();
  if (status == SL_STATUS_ALREADY_EXISTS) {
    status = SL_STATUS_OK;
  }
  TEST_ASSERT_EQUAL_HEX_MESSAGE(SL_STATUS_OK,
                                status,
                                "Unable to create network");

  status = zigpc_datastore_create_device(eui64);
  TEST_ASSERT_EQUAL_HEX_MESSAGE(SL_STATUS_OK,
                                status,
                                "Unable to create device");

  for (auto &jsn_endpoint: jsn["endpoints"]) {
    zigbee_endpoint_id_t endpoint_id = jsn_endpoint["endpoint_id"].get<zigbee_endpoint_id_t>();

    status = zigpc_datastore_create_endpoint(eui64, endpoint_id);

    std::string log_str = "Unable to create endpoint " + endpoint_id;
    TEST_ASSERT_EQUAL_HEX_MESSAGE(SL_STATUS_OK, status, log_str.c_str());

    helper_populate_cluster(eui64,
                            endpoint_id,
                            ZCL_CLUSTER_SERVER_SIDE,
                            jsn_endpoint);
    helper_populate_cluster(eui64,
                            endpoint_id,
                            ZCL_CLUSTER_CLIENT_SIDE,
                            jsn_endpoint);
  }
}
const std::string DEV_SINGLE_ENDPOINT_SIMPLE_CONFIG = R"({
  "endpoints": [
    {
      "endpoint_id": 3,
      "server_clusters": [
        {
          "cluster_id": 6,
          "attributes": [0,1],
          "recv_commands": [],
          "gen_commands": []
        }
      ],
      "client_clusters": [
        {
          "cluster_id": 3,
          "attributes": [3],
          "recv_commands": [],
          "gen_commands": []
        }
      ]
    }
]})";

const std::string DEV_MULTI_ENDPOINT_COMPLEX_CONFIG = R"({
  "endpoints": [
    {
      "endpoint_id": 2,
      "server_clusters": [
        {
          "cluster_id": 81,
          "attributes": [18,151,73,168,291],
          "recv_commands": [48,110,52],
          "gen_commands": [115,0,36]
        }
      ],
      "client_clusters": [
        {
          "cluster_id": 91,
          "attributes": [75],
          "recv_commands": [80,118],
          "gen_commands": [73,20,65,114]
        },
        {
          "cluster_id": 40,
          "attributes": [218,257,204,135,0,44],
          "recv_commands": [115,25],
          "gen_commands": [111,44]
        },
        {
          "cluster_id": 25,
          "attributes": [410,482,169,382,474],
          "recv_commands": [38],
          "gen_commands": []
        },
        {
          "cluster_id": 59,
          "attributes": [472,338,347,152,286,172,264,311,9],
          "recv_commands": [99],
          "gen_commands": [80]
        }
      ]
    },
    {
      "endpoint_id": 64,
      "server_clusters": [
        {
          "cluster_id": 42,
          "attributes": [350,239,356,120,436,109,392],
          "recv_commands": [93,48,23],
          "gen_commands": [123,114,22,127]
        },
        {
          "cluster_id": 103,
          "attributes": [272,9,105,182,216,494],
          "recv_commands": [36],
          "gen_commands": [39]
        },
        {
          "cluster_id": 87,
          "attributes": [153,314,303,493,3],
          "recv_commands": [24],
          "gen_commands": [28,80,51,38]
        },
        {
          "cluster_id": 20,
          "attributes": [339,130,419,175,3,336,190,52,299],
          "recv_commands": [],
          "gen_commands": [14]
        }
      ],
      "client_clusters": [
        {
          "cluster_id": 42,
          "attributes": [],
          "recv_commands": [85,110],
          "gen_commands": []
        },
        {
          "cluster_id": 79,
          "attributes": [327,464,232,411,122,364,263],
          "recv_commands": [103],
          "gen_commands": [90,53]
        },
        {
          "cluster_id": 48,
          "attributes": [164,10,304,75],
          "recv_commands": [66,13,70,42],
          "gen_commands": [59,29,89,55]
        },
        {
          "cluster_id": 45,
          "attributes": [95,462],
          "recv_commands": [121,98,56],
          "gen_commands": [87,73,115]
        }
      ]
    },
    {
      "endpoint_id": 82,
      "server_clusters": [
        {
          "cluster_id": 117,
          "attributes": [72],
          "recv_commands": [85,112,40,92],
          "gen_commands": []
        }
      ],
      "client_clusters": [
        {
          "cluster_id": 53,
          "attributes": [281,88,376,46],
          "recv_commands": [],
          "gen_commands": []
        }
      ]
    }
  ]})";

void test_sanity_reading_multiple_device(void)
{
  sl_status_t status;
  size_t device_count                     = 3;
  zigbee_eui64_t eui64_list[device_count] = {
    {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8},
    {0xA, 0xA, 0xA, 0xA, 0x5, 0x6, 0x7, 0x8},
    {0x1, 0x2, 0x3, 0x4, 0xF, 0xF, 0xF, 0xF},
  };
  zigbee_endpoint_id_t endpoint_id;
  zigbee_endpoint_id_t expected_endpoint_id = 3;

  // ARRANGE
  for (size_t i = 0; i < device_count; i++) {
    std::stringstream dev_config;
    dev_config << DEV_SINGLE_ENDPOINT_SIMPLE_CONFIG;
    helper_populate_device(eui64_list[i], dev_config);
  }

  // ACT
  for (size_t i = 0; i < device_count; i++) {
    status
      = zigpc_datastore_find_endpoint_by_index(eui64_list[i], 0, &endpoint_id);
    TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
    TEST_ASSERT_EQUAL(expected_endpoint_id, endpoint_id);
  }
}

void test_sanity_reading_device_endpoint(void)
{
  sl_status_t status;
  zigbee_eui64_t eui64  = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};
  size_t endpoint_count = 3;
  zigbee_endpoint_id_t endpoint_id[endpoint_count];
  zigbee_endpoint_id_t expected_endpoint_id[endpoint_count] = {2, 64, 82};

  std::stringstream dev_config;
  dev_config << DEV_MULTI_ENDPOINT_COMPLEX_CONFIG;

  // ARRANGE
  helper_populate_device(eui64, dev_config);

  // ACT
  for (size_t i = 0; i < endpoint_count; i++) {
    status = zigpc_datastore_find_endpoint_by_index(eui64, i, &endpoint_id[i]);
    TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
    TEST_ASSERT_EQUAL(expected_endpoint_id[i], endpoint_id[i]);
  }
}

void test_sanity_reading_device_cluster_attribute_list(void)
{
  sl_status_t status;
  zigbee_eui64_t eui64             = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};
  zigbee_endpoint_id_t endpoint_id = 64;

  zcl_cluster_id_t cluster_id      = 20;
  zcl_cluster_side_t cluster_side  = ZCL_CLUSTER_SERVER_SIDE;
  const size_t expected_attr_count = 9;

  size_t attr_count = expected_attr_count;
  zcl_attribute_id_t expected_attr_list[expected_attr_count]
    = {339, 130, 419, 175, 3, 336, 190, 52, 299};
  zcl_attribute_id_t attr_list[expected_attr_count];

  std::stringstream dev_config;
  dev_config << DEV_MULTI_ENDPOINT_COMPLEX_CONFIG;

  // ARRANGE
  helper_populate_device(eui64, dev_config);

  // ACT
  status = zigpc_datastore_read_attribute_list(eui64,
                                               endpoint_id,
                                               cluster_side,
                                               cluster_id,
                                               attr_list,
                                               &attr_count);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(expected_attr_count, attr_count);
  TEST_ASSERT_EQUAL_HEX16_ARRAY(expected_attr_list,
                                attr_list,
                                expected_attr_count);
}

void test_logging_multiple_endpoints_under_device(void)
{
  zigbee_eui64_t eui64 = {0x0D, 0x0E, 0x0A, 0x0D, 0x0B, 0x0E, 0x0E, 0x0F};

  std::stringstream dev_config;
  dev_config << DEV_MULTI_ENDPOINT_COMPLEX_CONFIG;

  // ARRANGE
  helper_populate_device(eui64, dev_config);

  // ACT
  zigpc_datastore_log_device_disc_state(eui64);
}

void test_logging_network(void)
{
  zigbee_eui64_t eui64 = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  std::stringstream dev_config;
  dev_config << DEV_MULTI_ENDPOINT_COMPLEX_CONFIG;

  // ARRANGE
  helper_populate_device(eui64, dev_config);

  // ACT
  sl_status_t status
    = zigpc_datastore_log_network("test_tag", SL_LOG_DEBUG, "");

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_logging_unkonwn_single_device(void)
{
  zigbee_eui64_t eui64 = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  std::stringstream dev_config;
  dev_config << DEV_MULTI_ENDPOINT_COMPLEX_CONFIG;

  // ARRANGE
  {
    zigbee_eui64_t input_eui64
      = {0x0D, 0x0E, 0x0A, 0x0D, 0x0B, 0x0E, 0x0E, 0x0F};
    helper_populate_device(input_eui64, dev_config);
  }

  // ACT
  sl_status_t status
    = zigpc_datastore_log_device("test_tag", SL_LOG_DEBUG, "", eui64);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NOT_FOUND, status);
}

void test_logging_single_device(void)
{
  zigbee_eui64_t eui64 = {0x0D, 0x0E, 0x0A, 0x0D, 0x0B, 0x0E, 0x0E, 0x0F};

  std::stringstream dev_config;
  dev_config << DEV_MULTI_ENDPOINT_COMPLEX_CONFIG;

  // ARRANGE
  helper_populate_device(eui64, dev_config);

  // ACT
  sl_status_t status
    = zigpc_datastore_log_device("test_tag", SL_LOG_DEBUG, "", eui64);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

}  // extern "C"
