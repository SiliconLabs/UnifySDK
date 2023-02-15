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
#include "zwave_command_class_association.h"
#include "zwave_command_class_agi.h"
#include "zwave_command_class_alarm_sensor.h"
#include "zwave_command_class_barrier_operator.h"
#include "zwave_command_class_binary_switch.h"
#include "zwave_command_class_central_scene.h"
#include "zwave_command_class_configuration.h"
#include "zwave_command_class_granted_keys_resolver.h"
#include "zwave_command_class_indicator.h"
#include "zwave_command_class_manufacturer_specific.h"
#include "zwave_command_class_meter_control.h"
#include "zwave_command_class_multilevel_sensor.h"
#include "zwave_command_class_multi_channel.h"
#include "zwave_command_class_multi_channel_association.h"
#include "zwave_command_class_node_info_resolver.h"
#include "zwave_command_class_notification.h"
#include "zwave_command_class_powerlevel.h"
#include "zwave_command_class_scene_activation_control.h"
#include "zwave_command_class_scene_actuator_configuration_control.h"
#include "zwave_command_class_scene_controller_configuration_control.h"
#include "zwave_command_class_security_0.h"
#include "zwave_command_class_security_2.h"
#include "zwave_command_class_supervision.h"
#include "zwave_command_class_switch_color.h"
#include "zwave_command_class_switch_multilevel.h"
#include "zwave_command_class_thermostat_mode.h"
#include "zwave_command_class_thermostat_setpoint.h"
#include "zwave_command_class_version.h"
#include "zwave_command_class_wake_up.h"
#include "zwave_command_class_time.h"
#include "zwave_command_class_basic.h"
#include "zwave_command_class_zwave_plus_info.h"
#include "zwave_command_class_device_reset_locally.h"
#include "zwave_command_class_inclusion_controller.h"
#include "zwave_command_class_multi_command.h"
#include "zwave_command_class_transport_service.h"
#include "zwave_command_class_indicator_control.h"
#include "zwave_command_class_manufacturer_specific_control.h"
// Generic includes
#include <stdbool.h>

// Includes from other components
#include "attribute_store.h"
#include "attribute_store_defined_attribute_types.h"
#include "zwave_command_handler.h"
#include "sl_status.h"
#include "sl_log.h"

#include "zwave_COMMAND_CLASS_DOOR_LOCK_handlers.h"
#include "zwave_COMMAND_CLASS_ZWAVEPLUS_INFO_handlers.h"
#include "zwave_command_class_battery.h"
#include "zwave_COMMAND_CLASS_SENSOR_BINARY_handlers.h"

sl_status_t zwave_command_classes_init()
{
  sl_status_t status = SL_STATUS_OK;

  // Special handlers:
  status |= zwave_command_class_granted_keys_resolver_init();
  status |= zwave_command_class_node_info_resolver_init();

  // Do not abort initialization of other CCs if one fails.
  // Command Class handlers
  // Note: AGI should stay first, it allows others to register commands.
  status |= zwave_command_class_agi_init();
  status |= zwave_command_class_alarm_sensor_init();
  status |= zwave_command_class_association_init();
  status |= zwave_command_class_barrier_operator_init();
  status |= zwave_command_class_basic_init();
  status |= zwave_command_class_binary_switch_init();
  status |= zwave_command_class_central_scene_init();
  status |= zwave_command_class_configuration_init();
  status |= zwave_command_class_device_reset_locally_init();
  status |= zwave_command_class_inclusion_controller_init();
  status |= zwave_command_class_indicator_init();
  status |= zwave_command_class_manufacturer_specific_init();
  status |= zwave_command_class_meter_control_init();
  status |= zwave_command_class_multi_channel_init();
  status |= zwave_command_class_multi_channel_association_init();
  status |= zwave_command_class_multi_command_init();
  status |= zwave_command_class_multilevel_sensor_init();
  status |= zwave_command_class_notification_init();
  status |= zwave_command_class_powerlevel_init();
  status |= zwave_command_class_scene_activation_control_init();
  status |= zwave_command_class_scene_actuator_configuration_control_init();
  status |= zwave_command_class_scene_controller_configuration_control_init();
  status |= zwave_command_class_security_0_init();
  status |= zwave_command_class_security_2_init();
  status |= zwave_command_class_supervision_init();
  status |= zwave_command_class_switch_color_init();
  status |= zwave_command_class_switch_multilevel_init();
  status |= zwave_command_class_thermostat_mode_init();
  status |= zwave_command_class_thermostat_setpoint_init();
  status |= zwave_command_class_time_init();
  status |= zwave_command_class_transport_service_init();
  status |= zwave_command_class_version_init();
  status |= zwave_command_class_wake_up_init();
  status |= zwave_command_class_zwave_plus_info_init();

  // Auto-generated handlers with overrides
  status |= zwave_command_class_battery_control_init();
  status |= zwave_command_class_indicator_control_init();
  status |= zwave_command_class_manufacturer_specific_control_init();

  // Auto-generated handlers
  status |= zwave_COMMAND_CLASS_DOOR_LOCK_init();
  status |= zwave_COMMAND_CLASS_ZWAVEPLUS_INFO_init();
  status |= zwave_COMMAND_CLASS_SENSOR_BINARY_init();

  zwave_command_handler_print_info(-1);
  return status;
}
