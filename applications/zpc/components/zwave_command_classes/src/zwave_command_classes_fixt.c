/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "zwave_command_class_association.h"
#include "zwave_command_class_agi.h"
#include "zwave_command_class_binary_switch.h"
#include "zwave_command_class_indicator.h"
#include "zwave_command_class_manufacturer_specific.h"
#include "zwave_command_class_multilevel_sensor.h"
#include "zwave_command_class_multi_channel.h"
#include "zwave_command_class_multi_channel_association.h"
#include "zwave_command_class_node_info_resolver.h"
#include "zwave_command_class_notification.h"
#include "zwave_command_class_powerlevel.h"
#include "zwave_command_class_security_0.h"
#include "zwave_command_class_security_2.h"
#include "zwave_command_class_supervision.h"
#include "zwave_command_class_switch_multilevel.h"
#include "zwave_command_class_thermostat_mode.h"
#include "zwave_command_class_thermostat_setpoint.h"
#include "zwave_command_class_version.h"
#include "zwave_command_class_wakeup.h"
#include "zwave_command_class_time.h"
#include "zwave_command_class_basic.h"
#include "zwave_command_class_zwave_plus_info.h"
#include "zwave_command_class_device_reset_locally.h"
#include "zwave_command_class_inclusion_controller.h"

// Generic includes
#include <stdbool.h>

// Includes from other components
#include "attribute_store.h"
#include "attribute_store_defined_attribute_types.h"
#include "zwave_command_handler.h"
#include "sl_status.h"
#include "sl_log.h"

#include "zwave_COMMAND_CLASS_DOOR_LOCK_handlers.h"
#include "zwave_COMMAND_CLASS_MANUFACTURER_SPECIFIC_handlers.h"
#include "zwave_COMMAND_CLASS_ZWAVEPLUS_INFO_handlers.h"
#include "zwave_COMMAND_CLASS_BATTERY_handlers.h"
#include "zwave_COMMAND_CLASS_SWITCH_BINARY_handlers.h"
#include "zwave_COMMAND_CLASS_SENSOR_BINARY_handlers.h"

// defined in rust project rust_command_classes
extern sl_status_t zwave_command_class_init_rust_handlers();

sl_status_t zwave_command_classes_init()
{
  sl_status_t initialization_status = SL_STATUS_OK;

  // Do not abort initialization of other CCs if one fails.
  // Command Class handlers
  // Note: AGI should stay first, it allows others to register commands.
  initialization_status |= zwave_command_class_agi_init();
  initialization_status |= zwave_command_class_association_init();
  initialization_status |= zwave_command_class_binary_switch_init();
  initialization_status |= zwave_command_class_indicator_init();
  initialization_status |= zwave_command_class_manufacturer_specific_init();
  initialization_status |= zwave_command_class_multi_channel_init();
  initialization_status |= zwave_command_class_multi_channel_association_init();
  initialization_status |= zwave_command_class_multilevel_sensor_init();
  initialization_status |= zwave_command_class_node_info_resolver_init();
  initialization_status |= zwave_command_class_notification_init();
  initialization_status |= zwave_command_class_powerlevel_init();
  initialization_status |= zwave_command_class_security_0_init();
  initialization_status |= zwave_command_class_security_2_init();
  initialization_status |= zwave_command_class_supervision_init();
  initialization_status |= zwave_command_class_switch_multilevel_init();
  initialization_status |= zwave_command_class_thermostat_mode_init();
  initialization_status |= zwave_command_class_thermostat_setpoint_init();
  initialization_status |= zwave_command_class_time_init();
  initialization_status |= zwave_command_class_version_init();
  initialization_status |= zwave_command_class_wakeup_init();
  initialization_status |= zwave_command_class_zwave_plus_info_init();
  initialization_status |= zwave_command_class_basic_init();
  initialization_status |= zwave_command_class_device_reset_locally_init();
  initialization_status |= zwave_command_class_inclusion_controller_init();
  initialization_status |= zwave_command_class_init_rust_handlers();

  // Auto-generated handlers
  initialization_status |= zwave_COMMAND_CLASS_DOOR_LOCK_init();
  initialization_status |= zwave_COMMAND_CLASS_MANUFACTURER_SPECIFIC_init();
  initialization_status |= zwave_COMMAND_CLASS_ZWAVEPLUS_INFO_init();
  initialization_status |= zwave_COMMAND_CLASS_BATTERY_init();
  initialization_status |= zwave_COMMAND_CLASS_SENSOR_BINARY_init();
  // De-activated Binary Switch for now. We can play with it after cert again.
  //initialization_status |= zwave_COMMAND_CLASS_SWITCH_BINARY_init();

  zwave_command_handler_print_info(-1);
  return initialization_status;
}
