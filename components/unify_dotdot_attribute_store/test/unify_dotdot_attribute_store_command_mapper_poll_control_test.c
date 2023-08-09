/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
// Test includes
#include "unity.h"
#include "unify_dotdot_attribute_store_test.h"

// Unify components
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store_helper.h"
#include "sl_log.h"

// Mock includes
#include "dotdot_mqtt_mock.h"

// Generic includes
#include <string.h>

#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_defined_attribute_types.h"

extern dotdot_unid_t expected_unid;
extern dotdot_endpoint_id_t expected_endpoint_id;
extern attribute_store_node_t expected_node_id;
extern unify_dotdot_attribute_store_configuration_t test_configuration;
attribute_store_node_t test_get_endpoint_node(const dotdot_unid_t unid,
                                              dotdot_endpoint_id_t endpoint_id);
sl_status_t test_get_unid_endpoint(attribute_store_node_t node,
                                   char *unid,
                                   dotdot_endpoint_id_t *endpoint_id);

void test_poll_control_set_long_poll_interval_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_poll_control_set_long_poll_interval_callback_t
    set_long_poll_interval_command
    = get_uic_mqtt_dotdot_poll_control_set_long_poll_interval_callback();
  TEST_ASSERT_NOT_NULL(set_long_poll_interval_command);

  // Default value for support check is fail:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    set_long_poll_interval_command(expected_unid,
                                   expected_endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                   44));

  dotdot_create_poll_control_long_poll_interval(expected_unid,
                                                expected_endpoint_id);

  // Now supported
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    set_long_poll_interval_command(expected_unid,
                                   expected_endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                   44));

  // Now create attribute and invoke the command
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    set_long_poll_interval_command(expected_unid,
                                   expected_endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                   44));

  TEST_ASSERT_FALSE(dotdot_poll_control_long_poll_interval_is_reported_defined(
    expected_unid,
    expected_endpoint_id));
  TEST_ASSERT_EQUAL(
    44,
    dotdot_get_poll_control_long_poll_interval(expected_unid,
                                               expected_endpoint_id,
                                               DESIRED_ATTRIBUTE));
}
