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
#include "zwave_command_class_manufacturer_specific.h"

// Generic includes
#include <string.h>
#include <stdio.h>
#include "assert.h"

// ZPC Components
#include "zwave_command_class_indices.h"
#include "zwave_controller_connection_info.h"
#include "zwave_controller_utils.h"
#include "zwave_command_classes_utils.h"
#include "zwave_command_handler.h"
#include "zwave_tx.h"
#include "zpc_config.h"
#include "zpc_hex_to_int.h"
#include "ZW_classcmd.h"
#include "zwave_security_validation.h"

// Unify Components
#include "sl_log.h"

#define LOG_TAG "zwave_command_class_manufacturer_specific"
#define MANUFACTURER_SPECIFIC_SCHEME \
  ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME

/**
 * @brief Command handler for the manufacter specific get
 *
 * The Gateway will send return a manufacturer speicifc Report frame containing
 * the manufacturer id, product type and product id
 *
 * @param connection_info Info about the connection properties of this frame.
 */
static sl_status_t
  zwave_command_class_manufacturer_specific_handle_manufacturer_specific_get(
    const zwave_controller_connection_info_t *connection_info)
{
  if (connection_info && connection_info->local.is_multicast) {
    return SL_STATUS_OK;
  }

  uint16_t m_id   = zpc_get_config()->manufacturer_id;
  uint16_t p_type = zpc_get_config()->product_type;
  uint16_t p_id   = zpc_get_config()->product_id;

  ZW_MANUFACTURER_SPECIFIC_REPORT_V2_FRAME report
    = {.cmdClass        = COMMAND_CLASS_MANUFACTURER_SPECIFIC_V2,
       .cmd             = MANUFACTURER_SPECIFIC_REPORT_V2,
       .manufacturerId1 = m_id >> 8,
       .manufacturerId2 = m_id & 0xff,
       .productTypeId1  = p_type >> 8,
       .productTypeId2  = p_type & 0xff,
       .productId1      = p_id >> 8,
       .productId2      = p_id & 0xff};

  return zwave_command_class_send_report(connection_info,
                                         sizeof(report),
                                         (uint8_t *)&report);
}
/**
 * @brief Command handler for the device speicific get
 *
 * The Gateway will send return a device specific report containing
 * the device specific data
 *
 * @param connection_info Info about the connection properties of this frame.
 */

#define MAX_DEVICE_ID_LEN 64

static sl_status_t
  zwave_command_class_manufacturer_specific_handle_device_specific_get(
    const zwave_controller_connection_info_t *connection_info)
{
  if (connection_info && connection_info->local.is_multicast) {
    return SL_STATUS_OK;
  }

  uint8_t buff[sizeof(ZW_DEVICE_SPECIFIC_REPORT_1BYTE_V2_FRAME)
               + (MAX_DEVICE_ID_LEN - 1)]
    = {0};
  const char *d = zpc_get_config()->device_id;
  uint8_t device_id[64];
  int device_id_len = 0;
  int val           = 0;

  ZW_DEVICE_SPECIFIC_REPORT_1BYTE_V2_FRAME *report
    = (ZW_DEVICE_SPECIFIC_REPORT_1BYTE_V2_FRAME *)&buff;

  report->cmdClass    = COMMAND_CLASS_MANUFACTURER_SPECIFIC_V2;
  report->cmd         = DEVICE_SPECIFIC_REPORT_V2;
  report->properties1 = DEVICE_SPECIFIC_GET_DEVICE_ID_TYPE_SERIAL_NUMBER_V2;

  sl_log_debug(LOG_TAG, "Reporting Device id: %s\n", d);
  while (*d && (device_id_len < MAX_DEVICE_ID_LEN)) {
    val                      = zpc_hex2int(*d++);
    device_id[device_id_len] = ((val)&0xf) << 4;
    val                      = zpc_hex2int(*d++);
    device_id[device_id_len] |= (val & 0xf);
    device_id_len++;
  }
  int offset = sizeof(ZW_DEVICE_SPECIFIC_REPORT_1BYTE_V2_FRAME) - 1;
  memcpy(&buff[offset], device_id, device_id_len);

  report->properties2
    = ((DEVICE_SPECIFIC_REPORT_DEVICE_ID_DATA_FORMAT_BINARY_V2
        << DEVICE_SPECIFIC_REPORT_PROPERTIES2_DEVICE_ID_DATA_FORMAT_SHIFT_V2)
       | device_id_len);
  return zwave_command_class_send_report(connection_info,
                                         sizeof(*report) + (device_id_len - 1),
                                         (uint8_t *)buff);
}

sl_status_t zwave_command_class_manufacturer_specific_support_handler(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  assert(frame_data[COMMAND_CLASS_INDEX]
         == COMMAND_CLASS_MANUFACTURER_SPECIFIC_V2);

  switch (frame_data[COMMAND_INDEX]) {
    case MANUFACTURER_SPECIFIC_GET:
      if (zwave_security_validation_is_security_valid_for_support(
            MANUFACTURER_SPECIFIC_SCHEME,
            connection)
          // CC:0072.01.00.41.004
          || ((connection->encapsulation == ZWAVE_CONTROLLER_ENCAPSULATION_NONE)
              && (zpc_highest_security_class()
                  == ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0))) {
        return zwave_command_class_manufacturer_specific_handle_manufacturer_specific_get(
          connection);
      }
      break;

    case DEVICE_SPECIFIC_GET_V2:
      if (zwave_security_validation_is_security_valid_for_support(
            MANUFACTURER_SPECIFIC_SCHEME,
            connection)) {
        /* Device ID Type in the GET command is ignored and ZPC will always return
       * Serial Number type. */
        return zwave_command_class_manufacturer_specific_handle_device_specific_get(
          connection);
      }
      break;

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t zwave_command_class_manufacturer_specific_init()
{
  zwave_command_handler_t handler = {};
  handler.support_handler
    = zwave_command_class_manufacturer_specific_support_handler;
  handler.control_handler            = NULL;
  handler.minimal_scheme             = MANUFACTURER_SPECIFIC_SCHEME;
  handler.command_class              = COMMAND_CLASS_MANUFACTURER_SPECIFIC_V2;
  handler.command_class_name         = "Manufacturer Specific";
  handler.version                    = MANUFACTURER_SPECIFIC_VERSION_V2;
  handler.manual_security_validation = true;  // CC:0072.01.00.41.004

  sl_log_debug(LOG_TAG, "Device id: %s\n", zpc_get_config()->device_id);

  return zwave_command_handler_register_handler(handler);
}
