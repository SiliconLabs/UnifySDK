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

#include <time.h>
#include "assert.h"
#include "platform_date_time.h"
#include "ZW_classcmd.h"
#include "zwave_command_class_time.h"
#include "zwave_command_handler.h"
#include "zwave_controller_command_class_indices.h"
#include "zwave_command_classes_utils.h"
#include "zwave_controller_connection_info.h"
#include "zwave_tx.h"

///////////////////////////////////////////////////////////////////////////////
// Private functions, used to handle individual incoming commands.
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Command handler for the Date Get command.
 *
 * The Gateway will send return a Date Report frame containing
 * the current date (day, month and year).
 *
 * @param connection_info Info about the connection properties of this frame.
 * @returns sl_status_t indicating the outcome of returning the date.
 */
static sl_status_t zwave_command_class_time_handle_date_get(
  const zwave_controller_connection_info_t *connection_info)
{
  if (connection_info && connection_info->local.is_multicast) {
    return SL_STATUS_OK;
  }
  date_time_t time = platform_get_date_time();

  ZW_DATE_REPORT_FRAME report = {.cmdClass = COMMAND_CLASS_TIME,
                                 .cmd      = DATE_REPORT,
                                 .year1    = (time.year + 1900) >> 8,    //MSB
                                 .year2    = (time.year + 1900) & 0xFF,  //LSB
                                 .month    = time.mon + 1,
                                 .day      = time.day};

  return zwave_command_class_send_report(connection_info,
                                         sizeof(report),
                                         (uint8_t *)&report);
}

/**
 * @brief Command handler for the Time Get command.
 *
 * The Gateway will send return a Time Report frame containing
 * the current time (hours, minutes and seconds).
 *
 * @param connection_info Info about the connection properties of this frame.
 * @returns sl_status_t indicating the outcome of returning the time.
 */
static sl_status_t zwave_command_class_time_handle_time_get(
  const zwave_controller_connection_info_t *connection_info)
{
  if (connection_info && connection_info->local.is_multicast) {
    return SL_STATUS_OK;
  }

  date_time_t time            = platform_get_date_time();
  ZW_TIME_REPORT_FRAME report = {.cmdClass        = COMMAND_CLASS_TIME,
                                 .cmd             = TIME_REPORT,
                                 .properties1     = time.hour & 0xF,
                                 .minuteLocalTime = time.min,
                                 .secondLocalTime = time.sec};

  return zwave_command_class_send_report(connection_info,
                                         sizeof(report),
                                         (uint8_t *)&report);
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_time_support_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  assert(frame_data[COMMAND_CLASS_INDEX] == COMMAND_CLASS_TIME);

  switch (frame_data[COMMAND_INDEX]) {
    case TIME_GET:
      return zwave_command_class_time_handle_time_get(connection_info);

    case DATE_GET:
      return zwave_command_class_time_handle_date_get(connection_info);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

sl_status_t zwave_command_class_time_init()
{
  zwave_command_handler_t handler = {0};
  handler.support_handler         = &zwave_command_class_time_support_handler;
  handler.control_handler         = NULL;
  handler.minimal_scheme          = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_TIME;
  handler.version                    = TIME_VERSION;
  handler.command_class_name         = "Time";

  return zwave_command_handler_register_handler(handler);
}
