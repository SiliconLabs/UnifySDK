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

// Component being tested:
#include "aox_locator_configuration.h"
#include "aox_locator_configuration_internal.h"

// Generic includes
#include <stdlib.h>

// Unify includes
#include "datastore.h"
#include "attribute_store_fixt.h"

// Mock components
#include "aoxpc_unid_mock.h"
#include "dotdot_mqtt_mock.h"

// Test constants
#define AOXPC_UNID          "AoXPCUNID"
#define EXPECTED_BASE_TOPIC "ucl/by-unid/AoXPCUNID/ep0"

// test variables
static uic_mqtt_dotdot_aox_locator_write_attributes_callback_t
  write_attributes_callback
  = NULL;

// Stubs
void uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_stub(
  const uic_mqtt_dotdot_aox_locator_write_attributes_callback_t callback,
  int cmock_num_calls)
{
  write_attributes_callback = callback;
}

sl_status_t get_aoxpc_unid_stub(aoa_id_t aoxpc_unid, int cmock_num_calls)
{
  snprintf(aoxpc_unid, sizeof(aoa_id_t), AOXPC_UNID);
  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  write_attributes_callback = NULL;
  aox_locator_reset_configuration();
}

static void attributes_publish_verifications()
{
  const aox_locator_attributes_t *config
    = aox_locator_configuration_get_configuration();

  uic_mqtt_dotdot_aox_locator_position_and_orientation_valid_publish_ExpectAndReturn(
    EXPECTED_BASE_TOPIC,
    config->position_and_orientation_valid,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_aox_locator_position_and_orientation_publish_ExpectAndReturn(
    EXPECTED_BASE_TOPIC,
    config->position_and_orientation,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_aox_locator_azimuth_mask_publish_ExpectAndReturn(
    EXPECTED_BASE_TOPIC,
    config->azimuth_mask_count,
    config->azimuth_mask,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_aox_locator_elevation_mask_publish_ExpectAndReturn(
    EXPECTED_BASE_TOPIC,
    config->elevation_mask_count,
    config->elevation_mask,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_aox_locator_allow_list_publish_ExpectAndReturn(
    EXPECTED_BASE_TOPIC,
    config->allow_list_count,
    (const char **)config->allow_list,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);

  uic_mqtt_dotdot_aox_locator_cte_mode_publish_ExpectAndReturn(
    EXPECTED_BASE_TOPIC,
    config->cte_mode,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);

  // Special ClusterRevision value.
  uic_mqtt_dotdot_aox_locator_publish_cluster_revision_Expect(
    EXPECTED_BASE_TOPIC,
    1);
}

void test_aox_locator_configuration_test()
{
  is_aoxpc_unid_IgnoreAndReturn(true);
  get_aoxpc_unid_Stub(&get_aoxpc_unid_stub);

  // Intercept the registration of the WriteAttributes callback
  uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_Stub(
    &uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_stub);

  // Init our AoXLocator component
  attributes_publish_verifications();
  TEST_ASSERT_EQUAL(SL_STATUS_OK, aox_locator_configuration_init());
  TEST_ASSERT_NOT_NULL(write_attributes_callback);

  // Try to check on our config, it's all zero'ed out at the beginning:
  const aox_locator_attributes_t *config
    = aox_locator_configuration_get_configuration();

  TEST_ASSERT_EQUAL(0, config->position_and_orientation.CoordinateX);
  TEST_ASSERT_EQUAL(0, config->position_and_orientation.CoordinateY);
  TEST_ASSERT_EQUAL(0, config->position_and_orientation.CoordinateZ);
  TEST_ASSERT_EQUAL(0, config->position_and_orientation.OrientationX);
  TEST_ASSERT_EQUAL(0, config->position_and_orientation.OrientationY);
  TEST_ASSERT_EQUAL(0, config->position_and_orientation.OrientationZ);
  TEST_ASSERT_EQUAL(0, config->allow_list_count);
  TEST_ASSERT_EQUAL_PTR(NULL, config->allow_list);
  TEST_ASSERT_EQUAL(0, config->cte_mode);

  // Let's try to update the configuration:
  const dotdot_unid_t unid            = "test";
  const dotdot_endpoint_id_t endpoint = AOXPC_ENDPOINT_ID;
  uic_mqtt_dotdot_callback_call_type_t call_type
    = UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL;
  uic_mqtt_dotdot_aox_locator_state_t state                 = {};
  uic_mqtt_dotdot_aox_locator_updated_state_t updated_state = {};
  state.position_and_orientation.CoordinateX                = 34.456;
  state.position_and_orientation.CoordinateY                = 32;
  state.position_and_orientation.CoordinateZ                = 1.000001;
  state.position_and_orientation.OrientationX               = 1.1;
  state.position_and_orientation.OrientationY               = 2.2;
  state.position_and_orientation.OrientationZ               = 3.3;
  updated_state.position_and_orientation                    = true;

  attributes_publish_verifications();
  uic_mqtt_dotdot_aox_locator_position_and_orientation_publish_IgnoreArg_value();
  write_attributes_callback(unid, endpoint, call_type, state, updated_state);

  TEST_ASSERT_EQUAL_DOUBLE(34.456,
                           config->position_and_orientation.CoordinateX);
  TEST_ASSERT_EQUAL_DOUBLE(32, config->position_and_orientation.CoordinateY);
  TEST_ASSERT_EQUAL_DOUBLE(1.000001,
                           config->position_and_orientation.CoordinateZ);
  TEST_ASSERT_EQUAL_DOUBLE(1.1, config->position_and_orientation.OrientationX);
  TEST_ASSERT_EQUAL_DOUBLE(2.2, config->position_and_orientation.OrientationY);
  TEST_ASSERT_EQUAL_DOUBLE(3.3, config->position_and_orientation.OrientationZ);

  // Okay one more time!
  state.allow_list_count = 2;
  char **allow_list;
  allow_list    = malloc(sizeof(char *) * state.allow_list_count);
  allow_list[0] = malloc(sizeof(char) * sizeof(aoa_id_t));
  snprintf(allow_list[0], sizeof(aoa_id_t), "AllowedTag1");
  allow_list[1] = malloc(sizeof(char) * sizeof(aoa_id_t));
  snprintf(allow_list[1], sizeof(aoa_id_t), "AllowedTag2");
  state.allow_list         = (const char **)allow_list;
  updated_state.position_and_orientation = false;
  updated_state.allow_list = true;
  TEST_ASSERT_EQUAL(0, config->allow_list_count);
  TEST_ASSERT_EQUAL_PTR(NULL, config->allow_list);

  attributes_publish_verifications();
  uic_mqtt_dotdot_aox_locator_allow_list_publish_IgnoreArg_value_count();
  uic_mqtt_dotdot_aox_locator_allow_list_publish_IgnoreArg_value();
  write_attributes_callback(unid, endpoint, call_type, state, updated_state);

  TEST_ASSERT_EQUAL(2, config->allow_list_count);
  TEST_ASSERT_EQUAL_STRING("AllowedTag1", config->allow_list[0]);
  TEST_ASSERT_EQUAL_STRING("AllowedTag2", config->allow_list[1]);

  // Third time, reducing size of the string array
  state.allow_list_count = 1;
  snprintf(allow_list[0], sizeof(aoa_id_t), "Tag1");

  attributes_publish_verifications();
  uic_mqtt_dotdot_aox_locator_allow_list_publish_IgnoreArg_value_count();
  uic_mqtt_dotdot_aox_locator_allow_list_publish_IgnoreArg_value();
  write_attributes_callback(unid, endpoint, call_type, state, updated_state);

  TEST_ASSERT_EQUAL(1, config->allow_list_count);
  TEST_ASSERT_EQUAL_STRING("Tag1", config->allow_list[0]);

  free(allow_list[1]);
  free(allow_list[0]);
  free(allow_list);
}

void test_aox_locator_configuration_publication_test()
{
  is_aoxpc_unid_IgnoreAndReturn(true);
  get_aoxpc_unid_Stub(&get_aoxpc_unid_stub);

  // Intercept the registration of the WriteAttributes callback
  uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_Stub(
    &uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_stub);

  // Init our AoXLocator component
  attributes_publish_verifications();
  TEST_ASSERT_EQUAL(SL_STATUS_OK, aox_locator_configuration_init());
  TEST_ASSERT_NOT_NULL(write_attributes_callback);

  const dotdot_unid_t unid            = "test";
  const dotdot_endpoint_id_t endpoint = AOXPC_ENDPOINT_ID;
  uic_mqtt_dotdot_callback_call_type_t call_type
    = UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL;
  uic_mqtt_dotdot_aox_locator_state_t state                 = {};
  uic_mqtt_dotdot_aox_locator_updated_state_t updated_state = {};
  updated_state.position_and_orientation                    = true;

  // Updating to the same value will trigger a publish.
  attributes_publish_verifications();
  write_attributes_callback(unid, endpoint, call_type, state, updated_state);
}

void test_aox_locator_configuration_wrong_unid_test()
{
  is_aoxpc_unid_IgnoreAndReturn(false);
  get_aoxpc_unid_Stub(&get_aoxpc_unid_stub);

  // Intercept the registration of the WriteAttributes callback
  uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_Stub(
    &uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_stub);

  // Init our AoXLocator component
  attributes_publish_verifications();
  TEST_ASSERT_EQUAL(SL_STATUS_OK, aox_locator_configuration_init());
  TEST_ASSERT_NOT_NULL(write_attributes_callback);

  const dotdot_unid_t unid            = "test";
  const dotdot_endpoint_id_t endpoint = AOXPC_ENDPOINT_ID;
  uic_mqtt_dotdot_callback_call_type_t call_type
    = UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL;
  uic_mqtt_dotdot_aox_locator_state_t state                 = {};
  uic_mqtt_dotdot_aox_locator_updated_state_t updated_state = {};
  updated_state.position_and_orientation                    = true;

  // Nothing happens when invoking the publication for the wrong UNID
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    write_attributes_callback(unid, endpoint, call_type, state, updated_state));
}

void test_aox_locator_configuration_support_check_test()
{
  is_aoxpc_unid_IgnoreAndReturn(true);
  get_aoxpc_unid_Stub(&get_aoxpc_unid_stub);

  // Intercept the registration of the WriteAttributes callback
  uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_Stub(
    &uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_stub);

  // Init our AoXLocator component
  attributes_publish_verifications();
  TEST_ASSERT_EQUAL(SL_STATUS_OK, aox_locator_configuration_init());
  TEST_ASSERT_NOT_NULL(write_attributes_callback);

  const dotdot_unid_t unid            = "test";
  const dotdot_endpoint_id_t endpoint = AOXPC_ENDPOINT_ID;
  uic_mqtt_dotdot_callback_call_type_t call_type
    = UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK;
  uic_mqtt_dotdot_aox_locator_state_t state                 = {};
  uic_mqtt_dotdot_aox_locator_updated_state_t updated_state = {};
  updated_state.position_and_orientation                    = true;

  // Nothing happens when invoking the publication for support check
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    write_attributes_callback(unid, endpoint, call_type, state, updated_state));
}

void test_aox_locator_configuration_wrong_endpoint_test()
{
  is_aoxpc_unid_IgnoreAndReturn(true);
  get_aoxpc_unid_Stub(&get_aoxpc_unid_stub);

  // Intercept the registration of the WriteAttributes callback
  uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_Stub(
    &uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_stub);

  // Init our AoXLocator component
  attributes_publish_verifications();
  TEST_ASSERT_EQUAL(SL_STATUS_OK, aox_locator_configuration_init());
  TEST_ASSERT_NOT_NULL(write_attributes_callback);

  const dotdot_unid_t unid            = "test";
  const dotdot_endpoint_id_t endpoint = AOXPC_ENDPOINT_ID + 1;
  uic_mqtt_dotdot_callback_call_type_t call_type
    = UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL;
  uic_mqtt_dotdot_aox_locator_state_t state                 = {};
  uic_mqtt_dotdot_aox_locator_updated_state_t updated_state = {};
  updated_state.position_and_orientation                    = true;

  // Nothing happens when invoking the publication with the wrong endpoint
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    write_attributes_callback(unid, endpoint, call_type, state, updated_state));
}

void test_aox_locator_set_position_and_orientation_test()
{
  is_aoxpc_unid_IgnoreAndReturn(true);
  get_aoxpc_unid_Stub(&get_aoxpc_unid_stub);

  // Intercept the registration of the WriteAttributes callback
  uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_Stub(
    &uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_stub);

  // Init our AoXLocator component
  attributes_publish_verifications();
  TEST_ASSERT_EQUAL(SL_STATUS_OK, aox_locator_configuration_init());

  // Set our Coordinates configuration
  CoordinateAndOrientation test_position = {.CoordinateX  = 1.1,
                                            .CoordinateY  = 2.0003,
                                            .CoordinateZ  = -12.23,
                                            .OrientationX = .1,
                                            .OrientationY = .3,
                                            .OrientationZ = .4};
  attributes_publish_verifications();
  uic_mqtt_dotdot_aox_locator_position_and_orientation_publish_IgnoreArg_value();
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    aox_locator_set_position_and_orientation_attribute(test_position));

  // Check our configuration is reflected
  const aox_locator_attributes_t *config
    = aox_locator_configuration_get_configuration();

  TEST_ASSERT_EQUAL_DOUBLE(test_position.CoordinateX,
                           config->position_and_orientation.CoordinateX);
  TEST_ASSERT_EQUAL_DOUBLE(test_position.CoordinateY,
                           config->position_and_orientation.CoordinateY);
  TEST_ASSERT_EQUAL_DOUBLE(test_position.CoordinateZ,
                           config->position_and_orientation.CoordinateZ);
  TEST_ASSERT_EQUAL_DOUBLE(test_position.OrientationX,
                           config->position_and_orientation.OrientationX);
  TEST_ASSERT_EQUAL_DOUBLE(test_position.OrientationY,
                           config->position_and_orientation.OrientationY);
  TEST_ASSERT_EQUAL_DOUBLE(test_position.OrientationZ,
                           config->position_and_orientation.OrientationZ);
}

void test_aox_locator_set_position_and_orientation_valid_test()
{
  is_aoxpc_unid_IgnoreAndReturn(true);
  get_aoxpc_unid_Stub(&get_aoxpc_unid_stub);

  // Intercept the registration of the WriteAttributes callback
  uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_Stub(
    &uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_stub);

  // Init our AoXLocator component
  attributes_publish_verifications();
  TEST_ASSERT_EQUAL(SL_STATUS_OK, aox_locator_configuration_init());

  // Set our Coordinates configuration
  bool test_value = true;
  attributes_publish_verifications();
  uic_mqtt_dotdot_aox_locator_position_and_orientation_valid_publish_IgnoreArg_value();
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    aox_locator_set_position_and_orientation_valid_attribute(test_value));

  // Check our configuration is reflected
  const aox_locator_attributes_t *config
    = aox_locator_configuration_get_configuration();

  TEST_ASSERT_EQUAL(test_value, config->position_and_orientation_valid);
}

void test_aox_locator_set_azimuth_mask_test()
{
  is_aoxpc_unid_IgnoreAndReturn(true);
  get_aoxpc_unid_Stub(&get_aoxpc_unid_stub);

  // Intercept the registration of the WriteAttributes callback
  uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_Stub(
    &uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_stub);

  // Init our AoXLocator component
  attributes_publish_verifications();
  TEST_ASSERT_EQUAL(SL_STATUS_OK, aox_locator_configuration_init());

  // Set our Azimuth mask configuration
  MinMaxPair test_azimuth_mask[3] = {{.Min = 1.1, .Max = 2.0003},
                                     {.Min = 1.2, .Max = 3.0003},
                                     {.Min = 1.3, .Max = 4.0003}};
  attributes_publish_verifications();
  uic_mqtt_dotdot_aox_locator_azimuth_mask_publish_IgnoreArg_value();
  uic_mqtt_dotdot_aox_locator_azimuth_mask_publish_IgnoreArg_value_count();
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    aox_locator_set_azimuth_mask_attribute(3, test_azimuth_mask));

  // Check our configuration is reflected
  const aox_locator_attributes_t *config
    = aox_locator_configuration_get_configuration();

  TEST_ASSERT_EQUAL_UINT8(3, config->azimuth_mask_count);
  TEST_ASSERT_EQUAL_DOUBLE(test_azimuth_mask[0].Min,
                           config->azimuth_mask[0].Min);
  TEST_ASSERT_EQUAL_DOUBLE(test_azimuth_mask[0].Max,
                           config->azimuth_mask[0].Max);
  TEST_ASSERT_EQUAL_DOUBLE(test_azimuth_mask[1].Min,
                           config->azimuth_mask[1].Min);
  TEST_ASSERT_EQUAL_DOUBLE(test_azimuth_mask[1].Max,
                           config->azimuth_mask[1].Max);
  TEST_ASSERT_EQUAL_DOUBLE(test_azimuth_mask[2].Min,
                           config->azimuth_mask[2].Min);
  TEST_ASSERT_EQUAL_DOUBLE(test_azimuth_mask[2].Max,
                           config->azimuth_mask[2].Max);

  // Subsequently change the value

  // Set our Azimuth mask configuration
  MinMaxPair test_azimuth_mask_2[4] = {{.Min = 2.1, .Max = 2.0003},
                                       {.Min = 2.2, .Max = 3.0003},
                                       {.Min = 1.3, .Max = 4.0003},
                                       {.Min = 1.3, .Max = 4.0003}};
  attributes_publish_verifications();
  uic_mqtt_dotdot_aox_locator_azimuth_mask_publish_IgnoreArg_value();
  uic_mqtt_dotdot_aox_locator_azimuth_mask_publish_IgnoreArg_value_count();
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    aox_locator_set_azimuth_mask_attribute(4, test_azimuth_mask_2));

  // Check our configuration is reflected
  TEST_ASSERT_EQUAL_UINT8(4, config->azimuth_mask_count);
  TEST_ASSERT_EQUAL_DOUBLE(test_azimuth_mask_2[0].Min,
                           config->azimuth_mask[0].Min);
  TEST_ASSERT_EQUAL_DOUBLE(test_azimuth_mask_2[0].Max,
                           config->azimuth_mask[0].Max);
  TEST_ASSERT_EQUAL_DOUBLE(test_azimuth_mask_2[1].Min,
                           config->azimuth_mask[1].Min);
  TEST_ASSERT_EQUAL_DOUBLE(test_azimuth_mask_2[1].Max,
                           config->azimuth_mask[1].Max);
  TEST_ASSERT_EQUAL_DOUBLE(test_azimuth_mask_2[2].Min,
                           config->azimuth_mask[2].Min);
  TEST_ASSERT_EQUAL_DOUBLE(test_azimuth_mask_2[2].Max,
                           config->azimuth_mask[2].Max);
  TEST_ASSERT_EQUAL_DOUBLE(test_azimuth_mask_2[3].Min,
                           config->azimuth_mask[3].Min);
  TEST_ASSERT_EQUAL_DOUBLE(test_azimuth_mask_2[3].Max,
                           config->azimuth_mask[3].Max);
}

void test_aox_locator_set_allow_list_test()
{
  is_aoxpc_unid_IgnoreAndReturn(true);
  get_aoxpc_unid_Stub(&get_aoxpc_unid_stub);

  // Intercept the registration of the WriteAttributes callback
  uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_Stub(
    &uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_stub);

  // Init our AoXLocator component
  attributes_publish_verifications();
  TEST_ASSERT_EQUAL(SL_STATUS_OK, aox_locator_configuration_init());

  // Set our AllowList configuration
  uint16_t allow_list_count = 20000; // Let's aim big, 20000 tags in the system
  char **allow_list;
  allow_list = malloc(sizeof(char *) * allow_list_count);
  for (int i = 0; i < allow_list_count; i++) {
    allow_list[i] = malloc(sizeof(char) * sizeof(aoa_id_t));
    snprintf(allow_list[i], sizeof(aoa_id_t), "AllowedTag%i", i);
  }
  attributes_publish_verifications();
  uic_mqtt_dotdot_aox_locator_allow_list_publish_IgnoreArg_value();
  uic_mqtt_dotdot_aox_locator_allow_list_publish_IgnoreArg_value_count();
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    aox_locator_set_allow_list_attribute(allow_list_count,
                                         (const char **)allow_list));

  // Check our configuration is reflected
  const aox_locator_attributes_t *config
    = aox_locator_configuration_get_configuration();

  TEST_ASSERT_EQUAL_UINT8(allow_list_count, config->allow_list_count);
  for (size_t i = 0; i < allow_list_count; i++) {
    TEST_ASSERT_EQUAL_STRING(allow_list[i], config->allow_list[i]);
    free(allow_list[i]);
  }
  free(allow_list);
}

void test_aox_locator_multi_reset_test()
{
  get_aoxpc_unid_Stub(&get_aoxpc_unid_stub);

  // Intercept the registration of the WriteAttributes callback
  uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_Stub(
    &uic_mqtt_dotdot_set_aox_locator_write_attributes_callback_stub);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, aox_locator_reset_configuration());
  TEST_ASSERT_EQUAL(SL_STATUS_OK, aox_locator_reset_configuration());
  TEST_ASSERT_EQUAL(SL_STATUS_OK, aox_locator_reset_configuration());
}