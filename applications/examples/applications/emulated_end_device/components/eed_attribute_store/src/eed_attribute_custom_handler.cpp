/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "eed_attribute_store.h"
#include "eed_attribute_store_helper.h"

#include "unify_dotdot_defined_attribute_types.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_timeouts.h"
#include "zap-types.h"

#define BOOTUP_WAIT_TIME 2000 //2sec
#define SET_COLOR_TEMP_TO_PREVIOUS_VALUE 0xFFFF

void set_on_off_on_startup(attribute_store_node_t startup_node)
{
  if (!attribute_store_is_reported_defined(startup_node))
  {
    attribute_timeout_set_callback(startup_node,
                                   BOOTUP_WAIT_TIME,
                                   set_on_off_on_startup);
    return;
  }
  attribute_store_node_t ep_node
    = attribute_store_get_node_parent(startup_node);
  attribute_store_node_t onoff_node
    = attribute_store_get_node_child_by_type(ep_node,
                                             DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                                             0);
  uint8_t startup_value = 0, value_size = 1, onoff_value = 0;
  attribute_store_get_node_attribute_value(startup_node,
                                           REPORTED_ATTRIBUTE,
                                           &startup_value,
                                           &value_size);
  attribute_store_get_node_attribute_value(onoff_node,
                                           REPORTED_ATTRIBUTE,
                                           &onoff_value,
                                           &value_size);
  switch (startup_value)
  {
  case ZCL_ON_OFF_START_UP_ON_OFF_SET_ON_OFF_TO0:
    onoff_value = 0;
    break;
  case ZCL_ON_OFF_START_UP_ON_OFF_SET_ON_OFF_TO1:
    onoff_value = 1;
    break;
  case ZCL_ON_OFF_START_UP_ON_OFF_TOGGLE_PREVIOUS_ON_OFF:
    onoff_value = !onoff_value;
    break;
  default:
    break;
  }
  attribute_store_set_node_attribute_value(onoff_node,
                                           REPORTED_ATTRIBUTE,
                                           &onoff_value,
                                           value_size);
  attribute_store_set_node_attribute_value(onoff_node,
                                           DESIRED_ATTRIBUTE,
                                           &onoff_value,
                                           value_size);
}

void set_color_temperature_mireds_on_start_up(attribute_store_node_t startup_node)
{
  if (!attribute_store_is_reported_defined(startup_node))
  {
    attribute_timeout_set_callback(startup_node,
                                   BOOTUP_WAIT_TIME,
                                   set_color_temperature_mireds_on_start_up);
    return;
  }
  attribute_store_node_t ep_node
    = attribute_store_get_node_parent(startup_node);
  attribute_store_node_t colorcontrol_color_temperature_mireds_node
    = attribute_store_get_node_child_by_type(ep_node,
                                             DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_TEMPERATURE_MIREDS,
                                             0);

  attribute_store_node_t colorcontrol_color_mode_node
    = attribute_store_get_node_child_by_type(ep_node,
                                             DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_MODE,
                                             0);

  attribute_store_node_t colorcontrol_color_enhanced_color_mode
    = attribute_store_get_node_child_by_type(ep_node,
                                             DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_ENHANCED_COLOR_MODE,
                                             0);

  uint8_t value_size = 1, color_mode_value = ZCL_COLOR_CONTROL_COLOR_MODE_COLOR_TEMPERATURE_MIREDS,
    enhanced_color_mode_value = ZCL_COLOR_CONTROL_ENHANCED_COLOR_MODE_COLOR_TEMPERATURE_MIREDS;
  uint16_t startup_color_temperature_value = {};

  attribute_store_get_node_attribute_value(startup_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&startup_color_temperature_value,
                                           &value_size);

  if (startup_color_temperature_value != SET_COLOR_TEMP_TO_PREVIOUS_VALUE){
    attribute_store_set_node_attribute_value(colorcontrol_color_temperature_mireds_node,
                                            REPORTED_ATTRIBUTE,
                                            (uint8_t *)&startup_color_temperature_value,
                                            sizeof(uint16_t));
    attribute_store_set_node_attribute_value(colorcontrol_color_temperature_mireds_node,
                                            DESIRED_ATTRIBUTE,
                                            (uint8_t *)&startup_color_temperature_value,
                                            sizeof(uint16_t));
  }

  attribute_store_set_node_attribute_value(colorcontrol_color_mode_node,
                                           REPORTED_ATTRIBUTE,
                                           &color_mode_value,
                                           sizeof(uint8_t));
  attribute_store_set_node_attribute_value(colorcontrol_color_mode_node,
                                           DESIRED_ATTRIBUTE,
                                           &color_mode_value,
                                           sizeof(uint8_t));

  attribute_store_set_node_attribute_value(colorcontrol_color_enhanced_color_mode,
                                           REPORTED_ATTRIBUTE,
                                           &enhanced_color_mode_value,
                                           sizeof(uint8_t));
  attribute_store_set_node_attribute_value(colorcontrol_color_enhanced_color_mode,
                                           DESIRED_ATTRIBUTE,
                                           &enhanced_color_mode_value,
                                           sizeof(uint8_t));   
}

void startup_on_off_handling(attribute_store_node_t node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_CREATED)
  {
    attribute_timeout_set_callback(node,
                                   BOOTUP_WAIT_TIME,
                                   set_on_off_on_startup);
  }
}

void startup_color_temperature_mireds_handling(attribute_store_node_t node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_CREATED)
  {
    attribute_timeout_set_callback(node,
                                   BOOTUP_WAIT_TIME,
                                   set_color_temperature_mireds_on_start_up);
  }
}

sl_status_t eed_attribute_store_attribute_custom_handler_init()
{
  attribute_store_register_callback_by_type(
    startup_on_off_handling,
    DOTDOT_ATTRIBUTE_ID_ON_OFF_START_UP_ON_OFF);

  attribute_store_register_callback_by_type(
    startup_color_temperature_mireds_handling,
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_START_UP_COLOR_TEMPERATURE_MIREDS);
  return SL_STATUS_OK;
}