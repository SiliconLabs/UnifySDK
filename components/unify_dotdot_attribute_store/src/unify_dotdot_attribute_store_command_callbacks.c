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
#include "unify_dotdot_attribute_store_command_callbacks.h"
#include "unify_dotdot_attribute_store_command_callbacks_barrier_control.h"
#include "unify_dotdot_attribute_store_command_callbacks_door_lock.h"
#include "unify_dotdot_attribute_store_command_callbacks_level.h"
#include "unify_dotdot_attribute_store_command_callbacks_color_control.h"
#include "unify_dotdot_attribute_store_command_callbacks_on_off.h"
#include "unify_dotdot_attribute_store_command_callbacks_identify.h"
#include "unify_dotdot_attribute_store_command_callbacks_poll_control.h"
#include "unify_dotdot_attribute_store_command_callbacks_thermostat.h"
#include "unify_dotdot_attribute_store_command_callbacks_window_covering.h"
#include "unify_dotdot_attribute_store_configuration.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_defined_attribute_types.h"

// Other Unify components.
#include "dotdot_mqtt.h"
#include "sl_log.h"

#define LOG_TAG "unify_dotdot_attribute_store_command_callbacks"

///////////////////////////////////////////////////////////////////////////////
// Internal component function that register callbacks to DotDot MQTT
////////////////////////////////////////////////////////////////////////////////
sl_status_t unify_dotdot_attribute_store_command_callbacks_init()
{
  barrier_control_cluster_mapper_init();
  on_off_cluster_mapper_init();
  color_control_cluster_mapper_init();
  door_lock_cluster_mapper_init();
  level_cluster_mapper_init();
  identify_cluster_mapper_init();
  poll_control_cluster_mapper_init();
  thermostat_cluster_mapper_init();
  window_covering_cluster_mapper_init();

  return SL_STATUS_OK;
}
