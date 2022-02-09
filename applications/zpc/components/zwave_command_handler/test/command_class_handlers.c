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
#include "zwave_command_handler.h"

// Includes from other components
#include "ZW_classcmd.h"
#include "sl_log.h"

// Generic includes
#include <stdbool.h>

#define LOG_TAG "zwave_command_handler_test"

extern uint8_t cc_zwave_plus_init_counter;
extern uint8_t cc_zwave_plus_handle_counter;
extern uint8_t cc_time_handle_counter;
extern uint8_t cc_time2_handle_counter;
extern uint8_t cc_time_init_counter;
extern uint8_t cc_version_init_counter;
extern uint8_t cc_version_control_handle_counter;
extern uint8_t cc_version_support_handle_counter;
extern uint8_t cc_version2_handle_counter;

///////////////////////////////////////////////////////////////////////////////
// Z-Wave Plus Info Command Class
///////////////////////////////////////////////////////////////////////////////
sl_status_t cc_zwave_plus_command_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  cc_zwave_plus_handle_counter++;
  return SL_STATUS_OK;
}

void cc_zwave_plus_init()
{
  cc_zwave_plus_init_counter++;

  zwave_command_handler_t my_handler    = {};
  my_handler.command_class              = COMMAND_CLASS_ZWAVEPLUS_INFO;
  my_handler.command_class_name         = "Z-Wave Plus Info";
  my_handler.version                    = 0x01;
  my_handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  my_handler.manual_security_validation = false;
  my_handler.support_handler            = cc_zwave_plus_command_handler;
  my_handler.control_handler            = NULL;

  zwave_command_handler_register_handler(my_handler);
  return;
}

///////////////////////////////////////////////////////////////////////////////
// Time Command Class
///////////////////////////////////////////////////////////////////////////////
sl_status_t cc_time_command_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  cc_time_handle_counter++;
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t cc_time2_command_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  cc_time2_handle_counter++;
  return SL_STATUS_FAIL;
}

void cc_time_init()
{
  cc_time_init_counter++;

  zwave_command_handler_t my_handler = {};
  my_handler.command_class           = COMMAND_CLASS_TIME_V2;
  my_handler.version                 = 0x02;
  my_handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
  my_handler.command_class_name = "Security 0";
  my_handler.manual_security_validation = true;
  my_handler.support_handler            = cc_time_command_handler;
  my_handler.control_handler            = NULL;

  zwave_command_handler_register_handler(my_handler);

  my_handler.support_handler = cc_time2_command_handler;
  zwave_command_handler_register_handler(my_handler);

  return;
}

///////////////////////////////////////////////////////////////////////////////
// Version Command Class
///////////////////////////////////////////////////////////////////////////////
sl_status_t cc_version_support_command_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  cc_version_support_handle_counter++;
  return SL_STATUS_OK;  //SL_STATUS_NOT_SUPPORTED;
}

sl_status_t cc_version_control_command_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  cc_version_control_handle_counter++;
  return SL_STATUS_OK;  //SL_STATUS_NOT_SUPPORTED;
}

void cc_version_init()
{
  cc_version_init_counter++;

  zwave_command_handler_t my_handler = {};
  my_handler.command_class           = COMMAND_CLASS_VERSION_V2;
  my_handler.command_class_name      = "Version";
  my_handler.version                 = 0x02;
  my_handler.minimal_scheme = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  my_handler.manual_security_validation = false;
  my_handler.support_handler            = cc_version_support_command_handler;
  my_handler.control_handler            = cc_version_control_command_handler;

  zwave_command_handler_register_handler(my_handler);
  zwave_command_handler_print_info(-1);
  return;
}
