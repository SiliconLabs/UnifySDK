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
#include "door_lock_cluster_mapper.h"
#include "dotdot_attributes.h"

// Includes from Unify
#include "sl_log.h"
#include "sl_status.h"

// Includes from auto-generated files
#include "dotdot_mqtt.h"
#include "zap-types.h"

// Setup Log ID
#define LOG_TAG "door_lock_cluster_mapper"

static sl_status_t doorlock_lock_command(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  const uic_mqtt_dotdot_callback_call_type_t callback_type,
  const char *pin_code)
{
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type) {
    return dotdot_is_supported_door_lock_lock_state(unid, endpoint) ? SL_STATUS_OK : SL_STATUS_FAIL;
  }
  if (dotdot_is_supported_door_lock_lock_state(unid, endpoint)) {
    dotdot_set_door_lock_lock_state(unid,
                                    endpoint,
                                    DESIRED_ATTRIBUTE,
                                    ZCL_DOOR_LOCK_LOCK_STATE_LOCKED);
  }

  return SL_STATUS_OK;
}

static sl_status_t
  doorlock_unLock_command(const dotdot_unid_t unid,
                          const dotdot_endpoint_id_t endpoint,
                          uic_mqtt_dotdot_callback_call_type_t callback_type,
                          const char *pin_code)
{
  if (callback_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return dotdot_is_supported_door_lock_lock_state(unid, endpoint) ? SL_STATUS_OK : SL_STATUS_FAIL;
  }
  if (dotdot_is_supported_door_lock_lock_state(unid, endpoint)) {
    dotdot_set_door_lock_lock_state(unid,
                                    endpoint,
                                    DESIRED_ATTRIBUTE,
                                    ZCL_DOOR_LOCK_LOCK_STATE_UNLOCKED);
  }

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
bool door_lock_cluster_mapper_init()
{
  sl_log_debug(LOG_TAG, "DoorLock cluster mapper initialization");

  // Register the callbacks for receiving commands from IoT service
  uic_mqtt_dotdot_door_lock_lock_door_callback_set(doorlock_lock_command);
  uic_mqtt_dotdot_door_lock_unlock_door_callback_set(doorlock_unLock_command);

  return true;
}

int door_lock_cluster_mapper_teardown()
{
  sl_log_debug(LOG_TAG, "Teardown of the DoorLock cluster mapper");
  return 0;
}
