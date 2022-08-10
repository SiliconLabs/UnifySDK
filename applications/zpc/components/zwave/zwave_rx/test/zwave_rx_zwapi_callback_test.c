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
#include "zwave_rx_zwapi_callbacks.h"

//Mocks
#include "zwapi_protocol_controller_mock.h"
#include "zwapi_init_mock.h"
#include "zwapi_protocol_basis_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_zwave_rx_zwave_api_started()
{
  zwapi_set_node_id_basetype_ExpectAndReturn(NODEID_16BITS, SL_STATUS_OK);
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  zwapi_remove_node_from_network_ExpectAndReturn(REMOVE_NODE_STOP,
                                                 NULL,
                                                 SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);

  zwave_rx_zwave_api_started(NULL, 0);
}