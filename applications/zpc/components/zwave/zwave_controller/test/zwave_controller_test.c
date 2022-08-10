/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "unity.h"
#include "zwave_controller.h"

// Mock includes
#include "zpc_config_mock.h"
#include "zwapi_protocol_basis_mock.h"
#include "zwave_s2_transport_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_zwave_controller_set_nif()
{
  const uint8_t command_classes[] = {1, 2, 3, 4, 5, 6};

  zpc_config_t test_configuration             = {0};
  test_configuration.zpc_basic_device_type    = 2;
  test_configuration.zpc_generic_device_type  = 3;
  test_configuration.zpc_specific_device_type = 4;
  zpc_get_config_IgnoreAndReturn(&test_configuration);

  node_type_t expected_node_type
    = {.basic    = test_configuration.zpc_basic_device_type,
       .generic  = test_configuration.zpc_generic_device_type,
       .specific = test_configuration.zpc_specific_device_type};

  // Tell the Z-Wave module what our NIF should be:
  zwapi_set_application_node_information_ExpectAndReturn(
    APPLICATION_NODEINFO_LISTENING,
    expected_node_type,
    command_classes,
    sizeof(command_classes),
    SL_STATUS_OK);

  // Test that the Z-Wave controller checks the config to set the NIF:
  zwave_controller_set_application_nif(command_classes,
                                       sizeof(command_classes));
}

void test_zwave_controller_set_secure_nif()
{
  const uint8_t command_classes[] = {1, 2, 3, 4, 5, 6, 12, 65, 127};
  // Simply forward our secure NIF to the S2 library.
  zwave_s2_set_secure_nif_ExpectAndReturn(command_classes,
                                          sizeof(command_classes),
                                          SL_STATUS_OK);

  zwave_controller_set_secure_application_nif(command_classes,
                                              sizeof(command_classes));
}