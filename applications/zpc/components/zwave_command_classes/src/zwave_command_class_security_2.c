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
#include "zwave_command_class_security_2.h"
#include "zwave_command_class_indices.h"
#include "zwave_command_classes_utils.h"
#include "zwave_command_class_granted_keys_resolver.h"

// Generic includes
#include <assert.h>

// ZPC Includes
#include "zwave_controller_connection_info.h"
#include "zwave_controller_utils.h"
#include "zwave_rx.h"
#include "zwave_command_handler.h"
#include "attribute_store_defined_attribute_types.h"
#include "zpc_attribute_store.h"
#include "zpc_attribute_store_network_helper.h"
#include "ZW_classcmd.h"
#include "zwave_tx_scheme_selector.h"

// Unify includes
#include "sl_log.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_resolver.h"

// Log tag
#define LOG_TAG                                "zwave_command_class_security_2"
#define SECURE_SUPPORTED_COMMAND_CLASSES_INDEX 2

///////////////////////////////////////////////////////////////////////////////
// Command Handler functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_security_2_commands_supported_report(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection);

  // We just received a report, it means that the key/protocol
  // combination that we are trying are working.
  zwave_command_class_mark_key_protocol_as_supported(
    attribute_store_get_first_parent_with_type(endpoint_node,
                                               ATTRIBUTE_NODE_ID),
    connection->encapsulation);

  // Get the Secure NIF atribute node under the endpoint
  attribute_store_node_t secure_nif_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE_ZWAVE_SECURE_NIF);
  if (frame_length <= SECURE_SUPPORTED_COMMAND_CLASSES_INDEX) {
    // Empty payload, if security scheme is equal to or higher than the ZPC
    // highest security scheme then delete the attribute
    // ATTRIBUTE_ZWAVE_SECURE_NIF. This is e.g. the case with CTT v3.
    if (is_using_zpc_highest_security_class(connection)) {
      attribute_store_delete_node(secure_nif_node);
      sl_log_debug(LOG_TAG,
                   "Received empty S2 Commands Supported Report with an equal "
                   "or higher security scheme than the ZPC, deleting the "
                   "Secure NIF from the Attribute Store");
      return SL_STATUS_OK;
    } else {
      sl_log_debug(LOG_TAG,
                   "Received empty S2 Commands Supported Report with "
                   "security scheme lower than highest ZPC security scheme");
      return SL_STATUS_OK;
    }
  } else {
    // Note that Securely Supported CC list will not be larger than 255
    uint8_t supported_cc_len
      = frame_length - SECURE_SUPPORTED_COMMAND_CLASSES_INDEX;

    // Accept the capabilities only if it is received at the highest granted key
    if (connection->encapsulation
        != zwave_tx_scheme_get_node_highest_security_class(
          connection->remote.node_id)) {
      // Here it could be a downgrade attack, where we receive a non-secure
      // S2 Command Supported Report. Do not accept the contents!
      sl_log_warning(LOG_TAG,
                     "Received S2 Commands Supported Report with "
                     "content on a 'non-secure' level. Discarding.");
      return SL_STATUS_OK;
    }

    attribute_store_set_child_reported(
      endpoint_node,
      ATTRIBUTE_ZWAVE_SECURE_NIF,
      &frame_data[SECURE_SUPPORTED_COMMAND_CLASSES_INDEX],
      supported_cc_len);
  }

  // We are done parsing the security 2 commands supported report frame
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_security_2_support_handler(
  const zwave_controller_connection_info_t *connection_info,
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

static sl_status_t zwave_command_class_security_2_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case SECURITY_2_COMMANDS_SUPPORTED_REPORT:
      return zwave_command_class_security_2_commands_supported_report(
        connection_info,
        frame_data,
        frame_length);
    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
//////////////////////////////////////////////////////////////////////////////
sl_status_t
  zwave_command_class_security_2_commands_supported_get(uint8_t *frame,
                                                        uint16_t *frame_len)
{
  ZW_SECURITY_2_COMMANDS_SUPPORTED_GET_FRAME *security_2_get_frame
    = (ZW_SECURITY_2_COMMANDS_SUPPORTED_GET_FRAME *)frame;
  security_2_get_frame->cmdClass = COMMAND_CLASS_SECURITY_2;
  security_2_get_frame->cmd      = SECURITY_2_COMMANDS_SUPPORTED_GET;
  *frame_len = sizeof(ZW_SECURITY_2_COMMANDS_SUPPORTED_GET_FRAME);

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_security_2_init()
{
  // Register the S2 CC handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.support_handler    = &zwave_command_class_security_2_support_handler;
  handler.control_handler    = &zwave_command_class_security_2_control_handler;
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.command_class      = COMMAND_CLASS_SECURITY_2;
  handler.version            = SECURITY_2_VERSION;
  handler.command_class_name = "Security 2";
  handler.manual_security_validation = true;

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}
