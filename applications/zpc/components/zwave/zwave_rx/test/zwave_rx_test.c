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
// Generic includes
#include <stdbool.h>
#include <string.h>

// Includes from other components
#include "zwave_controller.h"
#include "unity.h"
#include "sl_log.h"

// Contiki test resources
#include "contiki_test_helper.h"

// CMocks
#include "uic_main_externals_mock.h"
#include "zwapi_init_mock.h"
#include "zwapi_protocol_basis_mock.h"
#include "zwapi_protocol_controller_mock.h"
#include "zwave_controller_callbacks_mock.h"
#include "zwave_controller_internal_mock.h"
#include "zwave_controller_utils_mock.h"

// Includes from this component
#include "zwave_rx_test.h"

//static uint8_t my_serial_file_descriptor;
// Our private variables, used to verify results.
static tx_power_level_t test_zwave_tx_powerlevel
  = {.normal       = ZWAVE_RX_TEST_MAXIMUM_POWER_DBM,
     .measured0dBm = ZWAVE_RX_TEST_MEASURED_0DBM_POWER};
static zwapi_callbacks_t *registered_zwapi_callbacks              = NULL;
static zwapi_chip_data_t test_chip_data                           = {0};
static zwapi_protocol_version_information_t test_protocol_version = {0};

// Stub function used for retrieving callbacks registered to the zwapi module
sl_status_t zwapi_init_stub(const char *serial_port,
                            int *serial_fd,
                            const zwapi_callbacks_t *_callbacks)
{
  registered_zwapi_callbacks = (zwapi_callbacks_t *)_callbacks;
  *serial_fd                 = ZWAVE_RX_TEST_RFD;
  return SL_STATUS_OK;
}

void zwapi_get_chip_data_stub(zwapi_chip_data_t *chip_data)
{
  // Inject our chip data to the pointer
  *chip_data = test_chip_data;
}

sl_status_t
  zwapi_get_protocol_info_stub_ok(zwave_node_id_t node_id,
                                  zwapi_node_info_header_t *node_info_header)
{
  *node_info_header = expected_node_info_header_1;
  return SL_STATUS_OK;
}

sl_status_t
  zwapi_get_protocol_info_stub_fail(zwave_node_id_t node_id,
                                    zwapi_node_info_header_t *node_info_header)
{
  return SL_STATUS_FAIL;
}

// This function is called for the "happy case" initialization in many test cases.
static void rx_init_successful_test_helper()
{
  // Intialize Z-Wave RX, we expect it to call zwapi functions.
  zwapi_init_AddCallback((CMOCK_zwapi_init_CALLBACK)zwapi_init_stub);
  zwapi_init_ExpectAndReturn(ZWAVE_RX_TEST_SERIAL_PORT, 0, 0, SL_STATUS_OK);
  zwapi_init_IgnoreArg_serial_fd();
  zwapi_init_IgnoreArg_callbacks();
  zwapi_log_to_file_enable_ExpectAndReturn("", SL_STATUS_OK);
  // 2. zwapi_get_chip_data returns a controller library type
  zwapi_get_chip_data_Stub(
    (CMOCK_zwapi_get_chip_data_CALLBACK)zwapi_get_chip_data_stub);
  test_chip_data.library_type = ZWAVE_LIBRARY_TYPE_CONTROLLER_STATIC;
  test_chip_data.chip_type    = ZW_GECKO_CHIP_700;

  // 2.b we read the version and print at init
  zwapi_get_protocol_version_ExpectAndReturn(NULL, SL_STATUS_OK);
  test_protocol_version.type             = 10;
  test_protocol_version.major_version    = 7;
  test_protocol_version.minor_version    = 17;
  test_protocol_version.revision_version = 02;
  test_protocol_version.build_number     = 45001;
  test_protocol_version.git_commit[0]    = 0xAB;
  test_protocol_version.git_commit[1]    = 0xCD;
  test_protocol_version.git_commit[2]    = 0x11;
  test_protocol_version.git_commit[3]    = 0x22;
  zwapi_get_protocol_version_IgnoreArg_protocol_version();
  zwapi_get_protocol_version_ReturnThruPtr_protocol_version(
    &test_protocol_version);

  // 3. zwapi_set_tx_power_level goes well
  zwapi_set_tx_power_level_ExpectAndReturn(test_zwave_tx_powerlevel,
                                           SL_STATUS_OK);

  // 4. zwapi_set_tx_status_reporting goes well
  zwapi_set_tx_status_reporting_ExpectAndReturn(true, SL_STATUS_OK);

  // 5. zwapi_set_rf_region is happy
  zwapi_get_rf_region_ExpectAndReturn(ZWAVE_RX_TEST_RF_REGION_INT);

  // 6. zwapi_set_node_id_basetype well
  zwapi_set_node_id_basetype_ExpectAndReturn(NODEID_16BITS, SL_STATUS_OK);

  // 7. Register the read file descriptor to contiki
  uic_main_ext_register_rfd_ExpectAndReturn(ZWAVE_RX_TEST_RFD,
                                            NULL,
                                            &zwave_rx_process,
                                            SL_STATUS_OK);

  // Everything should be functional now.
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_rx_fixt_setup());
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp()
{
  zwave_controller_register_callbacks_IgnoreAndReturn(SL_STATUS_OK);
}

/// Test of rx init where enabling the log file fails
void test_zwave_rx_init_zwapi_init_log_file_fails(void)
{
  // Intialize Z-Wave RX, we expect it to call zwapi functions.
  zwapi_log_to_file_enable_ExpectAndReturn("", SL_STATUS_FAIL);

  // If zwapi_init fails, we expect a direct exit with false (error)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, zwave_rx_fixt_setup());
}

/// Test of rx init where zwapi_init fails
void test_zwave_rx_init_zwapi_init_fails(void)
{
  // Intialize Z-Wave RX, we expect it to call zwapi functions.
  zwapi_log_to_file_enable_ExpectAndReturn("", SL_STATUS_OK);
  zwapi_init_ExpectAndReturn(ZWAVE_RX_TEST_SERIAL_PORT, 0, 0, SL_STATUS_FAIL);
  zwapi_init_IgnoreArg_callbacks();
  zwapi_init_IgnoreArg_serial_fd();

  // If zwapi_init fails, we expect a direct exit with false (error)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, zwave_rx_fixt_setup());
}

/// Test of rx init for non-controller library (expected abort)
void test_zwave_rx_init_wrong_library()
{
  for (uint8_t library = 0; library < 20; library++) {
    if (library == ZWAVE_LIBRARY_TYPE_CONTROLLER
        || library == ZWAVE_LIBRARY_TYPE_CONTROLLER_BRIDGE
        || library == ZWAVE_LIBRARY_TYPE_CONTROLLER_STATIC) {
      continue;
    }
    // Intialize Z-Wave RX, we expect it to call zwapi functions.
    zwapi_log_to_file_enable_ExpectAndReturn("", SL_STATUS_OK);
    zwapi_init_ExpectAndReturn(ZWAVE_RX_TEST_SERIAL_PORT, 0, 0, SL_STATUS_OK);
    zwapi_init_IgnoreArg_callbacks();
    zwapi_init_IgnoreArg_serial_fd();

    // 2. zwapi_get_chip_data does not return a controller library type
    zwapi_get_chip_data_Stub(
      (CMOCK_zwapi_get_chip_data_CALLBACK)zwapi_get_chip_data_stub);
    test_chip_data.library_type = library;
    test_chip_data.chip_type    = ZW_GECKO_CHIP_700;

    zwapi_get_protocol_version_ExpectAndReturn(NULL, SL_STATUS_OK);
    zwapi_get_protocol_version_IgnoreArg_protocol_version();

    // Expect an abort here
    TEST_ASSERT_EQUAL(SL_STATUS_FAIL, zwave_rx_fixt_setup());
  }
}

// Test of successful initialization for non-gecko chip (RF setting will not be applied)
void test_zwave_rx_init_not_gecko_chip()
{
  // Intialize Z-Wave RX, we expect it to call zwapi functions.
  zwapi_init_ExpectAndReturn(ZWAVE_RX_TEST_SERIAL_PORT, 0, 0, SL_STATUS_OK);
  zwapi_init_IgnoreArg_callbacks();
  zwapi_init_IgnoreArg_serial_fd();
  zwapi_log_to_file_enable_ExpectAndReturn("", SL_STATUS_OK);

  // 2. zwapi_get_chip_data returns a controller library type
  zwapi_get_chip_data_Stub(
    (CMOCK_zwapi_get_chip_data_CALLBACK)zwapi_get_chip_data_stub);
  test_chip_data.library_type = ZWAVE_LIBRARY_TYPE_CONTROLLER;
  test_chip_data.chip_type    = 5;  // 500 series

  zwapi_get_protocol_version_ExpectAndReturn(NULL, SL_STATUS_FAIL);
  zwapi_get_protocol_version_IgnoreArg_protocol_version();

  // 3. zwapi_set_tx_power_level just fails
  zwapi_set_tx_power_level_ExpectAndReturn(test_zwave_tx_powerlevel,
                                           SL_STATUS_FAIL);

  // 4. zwapi_set_tx_status_reporting fails
  zwapi_set_tx_status_reporting_ExpectAndReturn(true, SL_STATUS_FAIL);

  // 5. Verify the RF region
  zwapi_get_rf_region_ExpectAndReturn(ZWAVE_RX_TEST_RF_REGION_INT);

  // 6. zwapi_set_node_id_basetype
  zwapi_set_node_id_basetype_ExpectAndReturn(NODEID_16BITS, SL_STATUS_OK);

  // 7. finally proceed with the fd registration, using the Z-Wave RX process.
  uic_main_ext_register_rfd_ExpectAndReturn(0,
                                            NULL,
                                            &zwave_rx_process,
                                            SL_STATUS_OK);
  uic_main_ext_register_rfd_IgnoreArg_fdr();

  // After this, the setup should return true (success)
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_rx_fixt_setup());
}

// Test of unsuccesfull rf region setting for 700-series chips
void test_zwave_rx_init_cannot_set_rf_resgion()
{
  // Intialize Z-Wave RX, we expect it to call zwapi functions.
  zwapi_init_ExpectAndReturn(ZWAVE_RX_TEST_SERIAL_PORT, 0, 0, SL_STATUS_OK);
  zwapi_init_IgnoreArg_callbacks();
  zwapi_init_IgnoreArg_serial_fd();
  zwapi_log_to_file_enable_ExpectAndReturn("", SL_STATUS_OK);

  // 2. zwapi_get_chip_data returns a controller library type
  zwapi_get_chip_data_Stub(
    (CMOCK_zwapi_get_chip_data_CALLBACK)zwapi_get_chip_data_stub);
  test_chip_data.library_type = ZWAVE_LIBRARY_TYPE_CONTROLLER;
  test_chip_data.chip_type    = ZW_GECKO_CHIP_700;

  zwapi_get_protocol_version_ExpectAndReturn(NULL, SL_STATUS_NOT_SUPPORTED);
  zwapi_get_protocol_version_IgnoreArg_protocol_version();

  // 3. zwapi_set_tx_power_level
  zwapi_set_tx_power_level_ExpectAndReturn(test_zwave_tx_powerlevel,
                                           SL_STATUS_OK);

  // 4. zwapi_set_tx_status_reporting fails
  zwapi_set_tx_status_reporting_ExpectAndReturn(true, SL_STATUS_OK);

  //Indicate that the module is set to another frequency
  zwapi_get_rf_region_ExpectAndReturn(ZWAVE_RX_TEST_RF_REGION_INT + 1);

  // 5. zwapi_set_rf_region works, else we stop
  zwapi_set_rf_region_ExpectAndReturn(ZWAVE_RX_TEST_RF_REGION_INT,
                                      SL_STATUS_FAIL);

  //Indicate that the module is set to another frequency
  zwapi_get_rf_region_ExpectAndReturn(ZWAVE_RX_TEST_RF_REGION_INT + 1);

  // After this, the setup should return true (success)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, zwave_rx_fixt_setup());
}

/// Test of rx init for with failure to set tx power / reporting
void test_zwave_rx_init_non_critical_failures()
{
  // Intialize Z-Wave RX, we expect it to call zwapi functions.
  zwapi_init_ExpectAndReturn(ZWAVE_RX_TEST_SERIAL_PORT, 0, 0, SL_STATUS_OK);
  zwapi_init_IgnoreArg_callbacks();
  zwapi_init_IgnoreArg_serial_fd();
  zwapi_log_to_file_enable_ExpectAndReturn("", SL_STATUS_OK);

  // 2. zwapi_get_chip_data returns a controller library type
  zwapi_get_chip_data_Stub(
    (CMOCK_zwapi_get_chip_data_CALLBACK)zwapi_get_chip_data_stub);
  test_chip_data.library_type = ZWAVE_LIBRARY_TYPE_CONTROLLER;
  test_chip_data.chip_type    = ZW_GECKO_CHIP_700;

  zwapi_get_protocol_version_ExpectAndReturn(NULL, SL_STATUS_NOT_SUPPORTED);
  zwapi_get_protocol_version_IgnoreArg_protocol_version();

  // 3. zwapi_set_tx_power_level just fails
  zwapi_set_tx_power_level_ExpectAndReturn(test_zwave_tx_powerlevel,
                                           SL_STATUS_FAIL);

  // 4. zwapi_set_tx_status_reporting fails
  zwapi_set_tx_status_reporting_ExpectAndReturn(true, SL_STATUS_FAIL);

  //Indicate that the module is set to another frequency
  zwapi_get_rf_region_ExpectAndReturn(ZWAVE_RX_TEST_RF_REGION_INT + 1);

  // 5. zwapi_set_rf_region works, else we stop
  zwapi_set_rf_region_ExpectAndReturn(ZWAVE_RX_TEST_RF_REGION_INT,
                                      SL_STATUS_OK);
  zwapi_soft_reset_ExpectAndReturn(SL_STATUS_FAIL);
  // zwave_rx_wait_for_zwave_api_to_be_ready will keep asking if we have completed
  // soft reset.
  zwapi_is_awaiting_zwave_api_started_ExpectAndReturn(true);
  zwapi_poll_ExpectAndReturn(true);
  zwapi_is_awaiting_zwave_api_started_ExpectAndReturn(true);
  zwapi_poll_ExpectAndReturn(true);
  zwapi_is_awaiting_zwave_api_started_ExpectAndReturn(true);
  zwapi_poll_ExpectAndReturn(false);
  zwapi_is_awaiting_zwave_api_started_ExpectAndReturn(false);

  // 6. zwapi_set_node_id_basetype fails
  zwapi_set_node_id_basetype_ExpectAndReturn(NODEID_16BITS, SL_STATUS_FAIL);

  // 7. finally proceed with the fd registration, using the Z-Wave RX process.
  uic_main_ext_register_rfd_ExpectAndReturn(0,
                                            NULL,
                                            &zwave_rx_process,
                                            SL_STATUS_OK);
  uic_main_ext_register_rfd_IgnoreArg_fdr();

  // After this, the setup should return true (success)
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_rx_fixt_setup());
}

/// Test of rx init for on_frame_received callbacks
void test_zwave_rx_on_frame_received()
{
  // Init zwave_rx:
  rx_init_successful_test_helper();

  zwave_rx_receive_options_t rx_opt = {.status_flags = ZWAVE_RX_TEST_RX_STATUS,
                                       .rssi         = ZWAVE_RX_TEST_RSSI,
                                       .nodes_in_multicast = {}};
  zwave_controller_connection_info_t conn = {
    .local  = {.node_id = ZWAVE_RX_TEST_DESTINATION_NODE_ID, .is_multicast = 1},
    .remote = {.node_id = ZWAVE_RX_TEST_SOURCE_NODE_ID},
    .encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
  };

  // RX event
  zwave_controller_on_frame_reception_Expect(ZWAVE_RX_TEST_SOURCE_NODE_ID);
  // Frame dispatch
  zwave_controller_on_frame_received_ExpectWithArray(
    &conn,
    sizeof(conn),
    &rx_opt,
    sizeof(rx_opt),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    sizeof(test_expected_frame_data_1));

  // Trigger an on_frame_received callback
  registered_zwapi_callbacks->application_command_handler_bridge(
    ZWAVE_RX_TEST_RX_STATUS,
    ZWAVE_RX_TEST_DESTINATION_NODE_ID,
    ZWAVE_RX_TEST_SOURCE_NODE_ID,
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    ZWAVE_RX_TEST_RSSI);

  // Now shut down the functionality
  zwapi_destroy_Expect();
  zwapi_log_to_file_disable_ExpectAndReturn(SL_STATUS_OK);
  TEST_ASSERT_EQUAL(0, zwave_rx_fixt_teardown());
}

/// Test of rx init for on_smart_start_event callbacks
void test_zwave_rx_on_smart_start_event()
{
  // Init:
  rx_init_successful_test_helper();

  zwave_controller_on_smart_start_inclusion_request_ExpectWithArray(
    expected_home_id_1,
    true,
    &expected_node_info_no_header_1,
    sizeof(zwave_node_info_t),
    false);

  zwave_command_class_list_unpack_Expect(0, 0, 0);
  zwave_command_class_list_unpack_IgnoreArg_nif();
  zwave_command_class_list_unpack_IgnoreArg_nif_length();
  zwave_command_class_list_unpack_IgnoreArg_node_info();
  zwave_command_class_list_unpack_ReturnArrayThruPtr_node_info(
    (zwave_node_info_t *)&expected_node_info_no_header_1,
    1);

  // Trigger a on_smart_start_inclusion_request callback:
  registered_zwapi_callbacks->application_controller_update(
    0x86,  //INIF received
    ZWAVE_RX_TEST_SOURCE_NODE_ID,
    (const uint8_t *)expected_node_info_array_1,
    sizeof(expected_node_info_array_1),
    expected_home_id_1);

  // Now shut down the functionality
  zwapi_destroy_Expect();
  zwapi_log_to_file_disable_ExpectAndReturn(SL_STATUS_OK);
  TEST_ASSERT_EQUAL(0, zwave_rx_fixt_teardown());
}

/// Test of rx init on_node_information callback
void test_zwave_rx_on_node_information()
{
  // Init:
  rx_init_successful_test_helper();

  // Rx event
  zwave_controller_on_frame_reception_Expect(ZWAVE_RX_TEST_SOURCE_NODE_ID);

  // Callbacks containing NIFs will trigger a call to zwapi_get_protocol_info.
  zwapi_get_protocol_info_Stub(
    (CMOCK_zwapi_get_protocol_info_CALLBACK)zwapi_get_protocol_info_stub_ok);

  zwave_command_class_list_unpack_Expect(0, 0, 0);
  zwave_command_class_list_unpack_IgnoreArg_nif();
  zwave_command_class_list_unpack_IgnoreArg_nif_length();
  zwave_command_class_list_unpack_IgnoreArg_node_info();
  zwave_command_class_list_unpack_ReturnArrayThruPtr_node_info(
    (zwave_node_info_t *)&expected_node_info_no_header_1,
    1);
  zwave_controller_on_node_information_ExpectWithArray(
    ZWAVE_RX_TEST_SOURCE_NODE_ID,
    &expected_node_info_no_header_1,
    sizeof(zwave_node_info_t));

  // Trigger a on_node_information callback:
  registered_zwapi_callbacks->application_controller_update(
    0x84,  //NIF received
    ZWAVE_RX_TEST_SOURCE_NODE_ID,
    (const uint8_t *)expected_node_info_array_1,
    sizeof(expected_node_info_array_1),
    0);

  // Rx event
  zwave_controller_on_frame_reception_Expect(ZWAVE_RX_TEST_DESTINATION_NODE_ID);
  // Change the get_protocol_info to return a failure
  zwapi_get_protocol_info_Stub(
    (CMOCK_zwapi_get_protocol_info_CALLBACK)zwapi_get_protocol_info_stub_fail);

  zwave_command_class_list_unpack_Expect(0, 0, 0);
  zwave_command_class_list_unpack_IgnoreArg_nif();
  zwave_command_class_list_unpack_IgnoreArg_nif_length();
  zwave_command_class_list_unpack_IgnoreArg_node_info();
  zwave_command_class_list_unpack_ReturnArrayThruPtr_node_info(
    (zwave_node_info_t *)&expected_node_info_no_header_1,
    1);
  zwave_controller_on_node_information_ExpectWithArray(
    ZWAVE_RX_TEST_DESTINATION_NODE_ID,
    &expected_node_info_no_header_1,
    sizeof(zwave_node_info_t));

  // Trigger a on_node_information callback:
  registered_zwapi_callbacks->application_controller_update(
    0x84,  //NIF received
    ZWAVE_RX_TEST_DESTINATION_NODE_ID,
    (const uint8_t *)expected_node_info_array_2,
    sizeof(expected_node_info_array_2),
    0);

  // Now shut down the functionality
  zwapi_destroy_Expect();
  zwapi_log_to_file_disable_ExpectAndReturn(SL_STATUS_OK);
  TEST_ASSERT_EQUAL(0, zwave_rx_fixt_teardown());
}

/// Test of rx init on_new_node_added callback
void test_zwave_rx_on_node_added()
{
  // Init:
  rx_init_successful_test_helper();

  zwave_controller_on_node_id_assigned_Expect(ZWAVE_RX_TEST_DESTINATION_NODE_ID,
                                              PROTOCOL_ZWAVE);
  // Trigger a on_new_node_added callback:
  registered_zwapi_callbacks->application_controller_update(
    0x40,  //New ID assigned
    ZWAVE_RX_TEST_DESTINATION_NODE_ID,
    NULL,
    0,
    0);

  // Now shut down the functionality
  zwapi_destroy_Expect();
  zwapi_log_to_file_disable_ExpectAndReturn(SL_STATUS_OK);
  TEST_ASSERT_EQUAL(0, zwave_rx_fixt_teardown());
}

/// Test of rx init on_node_removed callback
void test_zwave_rx_on_node_removed()
{
  // Init:
  rx_init_successful_test_helper();

  zwave_controller_on_node_deleted_Expect(ZWAVE_RX_TEST_SOURCE_NODE_ID);
  // Trigger a on_node_removed callback:
  registered_zwapi_callbacks->application_controller_update(
    0x20,  // Node deleted from network
    ZWAVE_RX_TEST_SOURCE_NODE_ID,
    NULL,
    0,
    0);

  // Now shut down the functionality
  zwapi_destroy_Expect();
  zwapi_log_to_file_disable_ExpectAndReturn(SL_STATUS_OK);
  TEST_ASSERT_EQUAL(0, zwave_rx_fixt_teardown());
}

/// Test of rx init on_new_suc callback
void test_zwave_rx_on_new_suc()
{
  // Init:
  rx_init_successful_test_helper();

  zwave_controller_on_new_suc_Expect(ZWAVE_RX_TEST_DESTINATION_NODE_ID);
  // Trigger a on_new_suc callback:
  registered_zwapi_callbacks->application_controller_update(
    0x10,  // New SUC NodeID
    ZWAVE_RX_TEST_DESTINATION_NODE_ID,
    NULL,
    0,
    0);

  // Now shut down the functionality
  zwapi_destroy_Expect();
  zwapi_log_to_file_disable_ExpectAndReturn(SL_STATUS_OK);
  TEST_ASSERT_EQUAL(0, zwave_rx_fixt_teardown());
}

/// Test of the Z-Wave RX process
void test_zwave_rx_process_poll()
{
  // Get the RX process started
  contiki_test_helper_init();
  process_start(&zwave_rx_process, 0);
  contiki_test_helper_run(0);

  // Send a poll event to our zwave_rx_process and verify that
  // it calls zwapi_poll as long as it returns true.
  zwapi_poll_ExpectAndReturn(true);
  zwapi_poll_ExpectAndReturn(true);
  zwapi_poll_ExpectAndReturn(true);
  zwapi_poll_ExpectAndReturn(false);

  process_poll(&zwave_rx_process);
  contiki_test_helper_run(0);
}

/// Test of rx receiving an INIF
void test_zwave_rx_on_inif_received()
{
  // Init:
  rx_init_successful_test_helper();

  zwave_home_id_t test_home_id = 483527;

  zwave_controller_on_smart_start_inclusion_request_Expect(
    test_home_id,
    true,  // INIF already included must be true
    NULL,
    false);
  zwave_controller_on_smart_start_inclusion_request_IgnoreArg_node_info();

  // Trigger a on_smart_start_inclusion_request callback:
  registered_zwapi_callbacks->application_controller_update(
    UPDATE_STATE_INCLUDED_NODE_INFO_RECEIVED,  // INIF received
    ZWAVE_RX_TEST_DESTINATION_NODE_ID,
    NULL,
    0,
    test_home_id);

  // Now shut down the functionality
  zwapi_destroy_Expect();
  zwapi_log_to_file_disable_ExpectAndReturn(SL_STATUS_OK);
  TEST_ASSERT_EQUAL(0, zwave_rx_fixt_teardown());
}

/// Test of rx receiving a SmartStart Prime
void test_zwave_rx_on_smartstart_inclusion_request_received()
{
  // Init:
  rx_init_successful_test_helper();

  zwave_home_id_t test_home_id = 93842;

  zwave_controller_on_smart_start_inclusion_request_Expect(test_home_id,
                                                           false,
                                                           NULL,
                                                           false);
  zwave_controller_on_smart_start_inclusion_request_IgnoreArg_node_info();

  // Trigger a on_smart_start_inclusion_request callback:
  registered_zwapi_callbacks->application_controller_update(
    UPDATE_STATE_NODE_INFO_FOREIGN_HOMEID_RECEIVED,  // SmartStart Prime received
    ZWAVE_RX_TEST_DESTINATION_NODE_ID,
    (const uint8_t *)expected_node_info_array_1,
    0,  // 0 length test
    test_home_id);

  // Now shut down the functionality
  zwapi_destroy_Expect();
  zwapi_log_to_file_disable_ExpectAndReturn(SL_STATUS_OK);
  TEST_ASSERT_EQUAL(0, zwave_rx_fixt_teardown());
}
