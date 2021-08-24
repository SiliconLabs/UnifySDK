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
#include "zwave_command_class_security_0.h"
#include "attribute_store_defined_attribute_types.h"

// ZPC Includes
#include "zwave_unid.h"
#include "zwave_controller_command_class_indices.h"
#include "zpc_attribute_store_network_helper.h"
#include "zwave_command_handler.h"
#include "ZW_classcmd.h"

// UIC Includes
#include "attribute_store_helper.h"
#include "attribute_resolver.h"
#include "sl_log.h"

// Log tag
#define LOG_TAG                                "zwave_command_class_security_0"
#define SECURE_SUPPORTED_COMMAND_CLASSES_INDEX 3

///////////////////////////////////////////////////////////////////////////////
// Command Handler functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_security_0_commands_supported_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= 3) {
    sl_log_debug(LOG_TAG,
                 "S0 Commands Supported Report Frame Length is too short\n");
    return SL_STATUS_OK;
  }

  if (connection_info->encapsulation
      < ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0) {
    sl_log_debug(LOG_TAG,
                 "S0 Commands Supported Report Frame encapsulated with a key "
                 "lower than S0 \n");
    return SL_STATUS_OK;
  }

  // Get the unid of the sending node
  unid_t sending_node_unid;
  zwave_unid_from_node_id(connection_info->remote.node_id, sending_node_unid);

  // Get the assosiated Attribute Store entry for this unid/endpoint
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(
      sending_node_unid,
      connection_info->remote.endpoint_id);

  // Get the Secure NIF atribute node under the endpoint
  attribute_store_node_t secure_nif_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE_ZWAVE_SECURE_NIF,
                                             0);
  // Note that Securely Supported CC list will not be larger than 255
  uint8_t supported_cc_len
    = frame_length - SECURE_SUPPORTED_COMMAND_CLASSES_INDEX;
  if (!supported_cc_len) {
    sl_log_debug(LOG_TAG, "S0 supported command report list was empty\n");
    return SL_STATUS_OK;
  }
  attribute_store_set_node_attribute_value(
    secure_nif_node,
    REPORTED_ATTRIBUTE,
    &frame_data[SECURE_SUPPORTED_COMMAND_CLASSES_INDEX],
    supported_cc_len);

  // We are done parsing the security 0 commands supported report frame
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_security_0_support_handler(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

sl_status_t zwave_command_class_security_0_control_handler(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case SECURITY_COMMANDS_SUPPORTED_REPORT:
      if (connection->encapsulation
          == ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0) {
        return zwave_command_class_security_0_commands_supported_report(
          connection,
          frame_data,
          frame_length);
      }
      break;

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
  return SL_STATUS_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
//////////////////////////////////////////////////////////////////////////////
sl_status_t
  zwave_command_class_security_0_commands_supported_get(uint8_t *frame,
                                                        uint16_t *frame_length)
{
  ZW_SECURITY_COMMANDS_SUPPORTED_GET_FRAME *security_0_get_frame
    = (ZW_SECURITY_COMMANDS_SUPPORTED_GET_FRAME *)frame;
  security_0_get_frame->cmdClass = COMMAND_CLASS_SECURITY;
  security_0_get_frame->cmd      = SECURITY_COMMANDS_SUPPORTED_GET;
  *frame_length = sizeof(ZW_SECURITY_COMMANDS_SUPPORTED_GET_FRAME);

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_security_0_init()
{
  // Register the S0 CC handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {0};
  handler.support_handler    = &zwave_command_class_security_0_support_handler;
  handler.control_handler    = &zwave_command_class_security_0_control_handler;
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.command_class      = COMMAND_CLASS_SECURITY;
  handler.command_class_name = "Security 0";
  handler.version            = SECURITY_VERSION;
  handler.manual_security_validation = true;

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}
