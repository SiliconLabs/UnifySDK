#include "unity.h"

// Include the header file with the functions to test
#include "zpc_database_updater.hpp"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "zpc_attribute_store_type_registration.h"
#include "attribute_store_type_registration.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"

// Cpp
#include "workaround_for_test.hpp"

extern "C" {

// This file should be copied by cmake next to this executable
const std::string TEST_DB_V2_BASE_NAME = "v2.db";
// Copy the database into a new one to avoid modifying the original
const std::string TEST_DB_TEST_COPY_NAME = "test_database.db";

/**
 * @brief Helper function to setup the database for testing
 * 
 * @note if name is set to :memory:, the database will be created in memory and the expected_base_version will be set to it.
 * 
 * @param name The name of the database to setup. It will be copied to a new file (TEST_DB_TEST_COPY_NAME) if it is not in memory.
 * @param expected_base_version The expected version of the database
 */
void helper_setup_database(const std::string &name,
                           int64_t expected_base_version)
{
  if (name == ":memory:") {
    datastore_init(name.c_str());
    datastore_store_int("version", expected_base_version);
  } else {
    try {
      // Prepare test file
      std::filesystem::copy(name,
                            TEST_DB_TEST_COPY_NAME,
                            std::filesystem::copy_options::overwrite_existing);
    } catch (const std::exception &e) {
      TEST_FAIL_MESSAGE(e.what());
    }

    // We open the copy to avoid modifying the original
    datastore_init(TEST_DB_TEST_COPY_NAME.c_str());

    // Check version
    int64_t datastore_version = 0;
    TEST_ASSERT_TRUE_MESSAGE(datastore_contains_int("version"),
                             "Datastore does not contain version attribute.");
    datastore_fetch_int("version", &datastore_version);
    TEST_ASSERT_EQUAL_INT_MESSAGE(expected_base_version,
                                  datastore_version,
                                  "Input database version incorrect.");

    attribute_store_init();
  }
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  zpc_attribute_store_register_known_attribute_types();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
    return num_failures;
}

/// Called before each and every test
void setUp()
{

}

/// Called after each and every test
void tearDown() {
}

// WARNING : This test is here to cover the v1 to v2 function.
// No test was done by the time, so we don't have a v1 database to test.
// We simulate a v1 empty database and check if the migration is successful.
void test_migration_v1_v2_test()
{
  // Current database version
  constexpr int64_t BASE_VERSION        = 1;
  // New version
  constexpr int64_t DESTINATION_VERSION = 2;

  // This test is here to 
  helper_setup_database(":memory:", 1);

  // Migrate the datastore
  sl_status_t status
    = zpc_database_updater::update_datastore(BASE_VERSION, DESTINATION_VERSION);
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            status,
                            "Datastore migration should have worked.");
}

void test_migration_v2_v3()
{
  // Current database version
  constexpr int64_t BASE_VERSION        = 2;
  // New version
  constexpr int64_t DESTINATION_VERSION = 3;

  helper_setup_database(TEST_DB_V2_BASE_NAME, BASE_VERSION);


  attribute_store_log();

  // Migrate the datastore
  sl_status_t status
    = zpc_database_updater::update_datastore(BASE_VERSION, DESTINATION_VERSION);
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            status,
                            "Datastore migration failed.");

  TEST_ASSERT_TRUE_MESSAGE(datastore_contains_int("version"),
                           "Datastore does not contain version attribute.");
  int64_t datastore_version = 0;
  datastore_fetch_int("version", &datastore_version);
  TEST_ASSERT_EQUAL_INT_MESSAGE(DESTINATION_VERSION,
                                datastore_version,
                                "Output database version incorrect.");

  // Test new attributes

  // First of all get the correct endpoint 
  // attribute_store_log() extract : 
  //
  // 59: 2024-Jul-09 14:20:25.685584 <d> [attribute_store] ID: 1 - Root node - Reported (hex): []
  // 59: 2024-Jul-09 14:20:25.685690 <d> [attribute_store]   ID: 55 - HomeID - Reported (hex): [DF AF 03 F7]
  // 59: 2024-Jul-09 14:20:25.685750 <d> [attribute_store]     ID: 56 - NodeID - Reported (hex): [01 00]
  // 59: 2024-Jul-09 14:20:25.687162 <d> [attribute_store]     ID: 73 - NodeID - Reported (hex): [02 00]
  // 59: 2024-Jul-09 14:20:25.687233 <d> [attribute_store]       ID: 74 - Endpoint ID - Reported (hex): [00]
  // 59: 2024-Jul-09 14:29:39.847599 <d> [attribute_store]       ID: 75 - Unknown 0xFD020001 - Reported (hex): [00 00 00 00] 
  // 59: 2024-Jul-09 14:29:39.847676 <d> [attribute_store]       ID: 76 - Inclusion protocol - Reported (hex): [00 00 00 00] 
  // 59: 2024-Jul-09 14:29:39.847755 <d> [attribute_store]       ID: 79 - Granted keys - Reported (hex): [00] 
  // 59: 2024-Jul-09 14:29:39.847828 <d> [attribute_store]       ID: 80 - KEX Fail type - Reported (hex): [00 00 00 00] 
  // 59: 2024-Jul-09 14:29:39.847905 <d> [attribute_store]       ID: 81 - NIF: Protocol Listening byte - Reported (hex): [53] 
  // 59: 2024-Jul-09 14:29:39.847978 <d> [attribute_store]       ID: 82 - NIF: Optional Protocol byte - Reported (hex): [DC] 
  // 59: 2024-Jul-09 14:29:39.848061 <d> [attribute_store]       ID: 85 - Version CC is supported - Reported (hex): [01] 
  // 59: 2024-Jul-09 14:29:39.848135 <d> [attribute_store]       ID: 111 - Last Rx/Tx timestamp - Reported (hex): [AF 56 00 00 00 00 00 00] 
  // 59: 2024-Jul-09 14:20:25.696159 <d> [attribute_store]       ID: 175 - Endpoint ID - Reported (hex): [01] << We want him
  auto root_node = attribute_store_get_root();
  auto home_id_node = attribute_store_get_node_child(root_node, 0);
  auto node_id_node = attribute_store_get_node_child(home_id_node, 1);
  const zwave_endpoint_id_t endpoint_id = 1;
  // Need to specify the endpoint ID here
  auto endpoint_id_node =  attribute_store_get_node_child_by_value(node_id_node,
                                                 ATTRIBUTE_ENDPOINT_ID,
                                                 REPORTED_ATTRIBUTE,
                                                 &endpoint_id,
                                                 sizeof(endpoint_id),
                                                 0);

  // Test new bitmask values
  const std::map<attribute_store_type_t, uint32_t> bitmask_attributes = {
    {ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES, 0x180F },
    {ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_SUPPORTED_SETPOINT_TYPES, 0x0186 }
  };

  for (const auto &[attribute_type, expected_value]: bitmask_attributes) {
    printf("Testing attribute %s\n",
           attribute_store_get_type_name(attribute_type));
    uint32_t new_bitmask;
    sl_status_t status
      = attribute_store_get_child_reported(endpoint_id_node,
                                           attribute_type,
                                           &new_bitmask,
                                           sizeof(new_bitmask));

    TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                              status,
                              "Failed to get new bitmask value.");

    TEST_ASSERT_EQUAL_MESSAGE(expected_value,
                              new_bitmask,
                              "New bitmask value mismatch.");
  }



  // Test new attribute ID values
  struct setpoint_value {
    int32_t value;
    uint8_t scale;
    uint8_t precision;
  };

  // Expected value in each fields
  const std::map<uint8_t, std::vector<setpoint_value>> expected_setpoint_attributes
    = {{0x01, {{73000, 1, 3}, {-10000, 0, 3}, {100000, 0, 3}}},
       {0x02, {{73000, 1, 3}, {-10000, 0, 3}, {100000, 0, 3}}},
       {0x0B, {{62000, 1, 3}, {-10000, 0, 3}, {100000, 0, 3}}},
       {0x0C, {{85000, 1, 3}, {-10000, 0, 3}, {100000, 0, 3}}}};

  // New attributes ID start a this value.
  // We can then find the value, scale and precision by incrementing the id by 1 :
  // VALUE_TYPE : 0x4304
  // VALUE_SCALE_TYPE : 0x4305
  // VALUE_PRECISION_TYPE : 0x4306
  // MIN_VALUE_TYPE : 0x4307
  // MIN_VALUE_SCALE_TYPE : 0x4308
  // MIN_VALUE_PRECISION_TYPE : 0x4309
  // MAX_VALUE_TYPE : 0x430A
  // MAX_VALUE_SCALE_TYPE : 0x430B
  // MAX_VALUE_PRECISION_TYPE : 0x430C
  const attribute_store_type_t START_VALUE_TYPE = 0x4304;

  for (const auto &[setpoint_type, expected_values]: expected_setpoint_attributes) {
    printf("Testing setpoint type %d\n", setpoint_type);

    auto setpoint_type_node = attribute_store_get_node_child_by_value(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPE,
      REPORTED_ATTRIBUTE,
      &setpoint_type,
      sizeof(setpoint_type),
      0);

    TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                  setpoint_type_node,
                                  "Failed to get setpoint type node.");

    // Count of (Value, Min Value, Max Value) for each setpoint type
    const uint8_t VALUE_COUNT = 3;
    // Count of (Value, Scale, Precision) for each value
    const uint8_t ATTRIBUTE_FIELDS = 3;
    for (uint8_t value_index = 0; value_index < VALUE_COUNT; value_index++) {
      setpoint_value reported_values;
      const uint8_t CURRENT_OFFSET = ATTRIBUTE_FIELDS * value_index;

      // Get value
      sl_status_t status
        = attribute_store_get_child_reported(setpoint_type_node,
                                             START_VALUE_TYPE + CURRENT_OFFSET,
                                             &reported_values.value,
                                             sizeof(reported_values.value));
      TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                                status,
                                "Failed to get new setpoint value.");

      TEST_ASSERT_EQUAL_MESSAGE(expected_values[value_index].value,
                                reported_values.value,
                                "New setpoint value mismatch.");


      // Get scale
      // TODO : use defined offset in thermostat setpoint header when available
      status
        = attribute_store_get_child_reported(setpoint_type_node,
                                             START_VALUE_TYPE + CURRENT_OFFSET + 1,
                                             &reported_values.scale,
                                             sizeof(reported_values.scale));
      TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                                status,
                                "Failed to get new setpoint scale.");

      TEST_ASSERT_EQUAL_MESSAGE(expected_values[value_index].scale,
                                reported_values.scale,
                                "New setpoint scale mismatch.");

      // Get precision
      status
        = attribute_store_get_child_reported(setpoint_type_node,
                                             START_VALUE_TYPE + CURRENT_OFFSET + 2,
                                             &reported_values.precision,
                                             sizeof(reported_values.precision));
      TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                                status,
                                "Failed to get new setpoint precision.");

      TEST_ASSERT_EQUAL_MESSAGE(expected_values[value_index].precision,
                                reported_values.precision,
                                "New setpoint precision mismatch.");
    }
  }
}



} // extern "C"