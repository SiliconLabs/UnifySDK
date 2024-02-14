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

#include <stdlib.h>

#include "workaround.hpp"
#include "unity.h"
#include "unity_helpers.h"
#include "cmock.h"

#include "sl_status.h"

#include "mqtt_test_helper.h"

extern "C" {
#include "dotdot_mqtt.h"
#include "dotdot_mqtt_group_commands.h"

/// Called before each and every test
void setUp()
{
  mqtt_test_helper_init();
}

static struct {
  int num_callbacks;
  dotdot_group_id_t group_id;
  uic_mqtt_dotdot_door_lock_command_lock_door_fields_t fields;
} stub_by_group_dl_lock = {0};
static void stub_by_group_dl_lock_door_callback(
  const dotdot_group_id_t group_id,
  const uic_mqtt_dotdot_door_lock_command_lock_door_fields_t *fields)
{
  TEST_ASSERT_EQUAL(stub_by_group_dl_lock.group_id, group_id);
  TEST_ASSERT_EQUAL_STRING(stub_by_group_dl_lock.fields.pin_or_rfid_code,
                           fields->pin_or_rfid_code);
  stub_by_group_dl_lock.num_callbacks++;
}

void test_dotdot_mqtt_by_group_command_with_argument(void)
{
  // We're arbitrarily using the DoorLock_LockDoor because it's as good as any.
  stub_by_group_dl_lock.num_callbacks          = 0;
  stub_by_group_dl_lock.group_id               = 12;
  stub_by_group_dl_lock.fields.pin_or_rfid_code = "1 Two Three!";

  uic_mqtt_dotdot_by_group_door_lock_lock_door_callback_set(
    &stub_by_group_dl_lock_door_callback);
  uic_mqtt_dotdot_init();

  std::string msg    = R"({"PINOrRFIDCode":"1 Two Three!"})";
  const char topic[] = "ucl/by-group/12/DoorLock/Commands/LockDoor";
  mqtt_test_helper_publish(topic, msg.c_str(), msg.length());
  msg = R"({"PIN":"1234"})";
  mqtt_test_helper_publish(topic, msg.c_str(), msg.length());
  // Only the valid payload should have gone all the way through.
  TEST_ASSERT_EQUAL_INT(1, stub_by_group_dl_lock.num_callbacks);
}

static struct {
  int num_callbacks;
  dotdot_group_id_t group_id;
  uic_mqtt_dotdot_door_lock_state_t state;
  uic_mqtt_dotdot_door_lock_updated_state_t updated;
} stub_by_group_dl_writeattr = {0};
void stub_by_group_dl_writeattr_callback(
  const dotdot_group_id_t group_id,
  uic_mqtt_dotdot_door_lock_state_t state,
  uic_mqtt_dotdot_door_lock_updated_state_t updated)
{
  TEST_ASSERT_EQUAL_INT16(stub_by_group_dl_writeattr.group_id, group_id);

  TEST_ASSERT_EQUAL_MEMORY(&stub_by_group_dl_writeattr.updated,
                           &updated,
                           sizeof(uic_mqtt_dotdot_door_lock_updated_state_t));
  TEST_ASSERT_EQUAL_STRING(stub_by_group_dl_writeattr.state.language,
                           state.language);

  stub_by_group_dl_writeattr.num_callbacks++;
}

// Test WriteAttributes accept enum as integer string
void test_dotdot_mqtt_by_group_write_attributes()
{
  uic_mqtt_dotdot_by_group_door_lock_write_attributes_callback_set(
    stub_by_group_dl_writeattr_callback);
  uic_mqtt_dotdot_init();

  stub_by_group_dl_writeattr.group_id         = 13;
  stub_by_group_dl_writeattr.num_callbacks    = 0;
  stub_by_group_dl_writeattr.updated.language = true;
  stub_by_group_dl_writeattr.state.language   = "None";

  // Test as string name
  const std::string payload = R"({"Language": "None"})";
  mqtt_test_helper_publish("ucl/by-group/13/DoorLock/Commands/WriteAttributes",
                           payload.c_str(),
                           payload.length());

  TEST_ASSERT_EQUAL_INT(1, stub_by_group_dl_lock.num_callbacks);
}
}
