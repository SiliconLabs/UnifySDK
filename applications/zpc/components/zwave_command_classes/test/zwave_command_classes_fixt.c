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

// Includes from this component
#include "zwave_command_classes_fixt.h"

// Includes from other components
#include "sl_log.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "attribute_store_helper_mock.h"
#include "zwave_unid.h"
#include "zwave_tx_mock.h"
#include "zwave_controller_keyset_mock.h"
#include "zwave_controller_connection_info.h"

#include "ZW_classcmd.h"

#define LOG_TAG "zwave_command_classes_fixt"

// Test includes
#include "unity.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void test_zwave_command_class_binary_switch_init()
{
  /// FIXME: Write some real tests here.
  TEST_ASSERT_EQUAL(0, 0);
}
