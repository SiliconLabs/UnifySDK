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
#include "zwave_command_class_zwave_plus_info.h"
#include "zwave_command_classes_utils.h"

// Includes from other components
#include "ZW_classcmd.h"
#include "zwave_command_handler.h"
#include "zwave_controller_command_class_indices.h"
#include "zwave_tx.h"

// Generic includes
#include <assert.h>

#define LOG_TAG "zwave_command_class_zwave_plus_info"

static sl_status_t
  zwave_command_class_zwave_plus_info_handle_zwave_plus_info_get(
    const zwave_controller_connection_info_t *connection_info)
{
  if (connection_info && connection_info->local.is_multicast) {
    return SL_STATUS_OK;
  }

  const ZW_ZWAVEPLUS_INFO_REPORT_V2_FRAME report
    = {.cmdClass     = COMMAND_CLASS_ZWAVEPLUS_INFO,
       .cmd          = ZWAVEPLUS_INFO_REPORT,
       .zWaveVersion = ZWAVEPLUS_VERSION_V2,
       .roleType = ZWAVEPLUS_INFO_REPORT_ROLE_TYPE_CONTROLLER_CENTRAL_STATIC,
       .nodeType = ZWAVEPLUS_INFO_REPORT_NODE_TYPE_ZWAVEPLUS_NODE,
       .installerIconType1 = (ICON_TYPE_GENERIC_CENTRAL_CONTROLLER >> 8) & 0xFF,
       .installerIconType2 = ICON_TYPE_GENERIC_CENTRAL_CONTROLLER & 0xFF,
       .userIconType1      = (ICON_TYPE_GENERIC_CENTRAL_CONTROLLER >> 8) & 0xFF,
       .userIconType2      = ICON_TYPE_GENERIC_CENTRAL_CONTROLLER & 0xFF};

  return zwave_command_class_send_report(connection_info,
                                         sizeof(report),
                                         (const uint8_t *)&report);
}

sl_status_t zwave_command_class_zwave_plus_info_support_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  assert(frame_data[COMMAND_CLASS_INDEX] == COMMAND_CLASS_ZWAVEPLUS_INFO);

  if (frame_data[COMMAND_INDEX] == ZWAVEPLUS_INFO_GET) {
    return zwave_command_class_zwave_plus_info_handle_zwave_plus_info_get(
      connection_info);
  } else {
    return SL_STATUS_NOT_SUPPORTED;
  }
}

sl_status_t zwave_command_class_zwave_plus_info_init()
{
  zwave_command_handler_t handler = {0};
  handler.support_handler
    = &zwave_command_class_zwave_plus_info_support_handler;
  handler.control_handler            = NULL;
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_ZWAVEPLUS_INFO;
  handler.version                    = ZWAVEPLUS_VERSION_V2;
  handler.command_class_name         = "Z-Wave Plus Info";

  return zwave_command_handler_register_handler(handler);
}
