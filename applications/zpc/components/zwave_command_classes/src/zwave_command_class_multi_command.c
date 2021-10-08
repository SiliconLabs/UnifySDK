/******************************************************************************
* # License
* <b>Copyright 2021  Silicon Laboratories Inc. www.silabs.com</b>
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
#include "zwave_command_class_multi_command.h"

// Generic includes
#include <stdbool.h>
#include <string.h>

// Includes from other components
#include "ZW_classcmd.h"
#include "zwave_controller_command_class_indices.h"
#include "zwave_controller_connection_info.h"
#include "zwave_command_handler.h"
#include "zwave_controller_internal.h"
#include "zwave_rx.h"
#include "zwave_tx.h"

#define NUMBER_OF_COMMANDS_INDEX 2

///////////////////////////////////////////////////////////////////////////////
// Command Handler functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Decapsulate Multi Command encapsulation and inject the frames back to
 *        the Z-Wave Controller
 *
 * The provided payload will be Multi Command decapsulated and the endpoint
 * data will be copied in
 * connection->remote.endpoint_id and connection->local.endpoint_id and passed
 * to the \ref zwave_command_handler_dispatch() function.
 *
 *
 * @param connection_info  Connection object describing the source and
 *                         destination.
 * @param frame_data       Length of the frame to send
 * @param frame_length     Points to the payload to send
 *
 * @returns
 * - SL_STATUS_NOT_SUPPORTED  if the frame data is not multi command encapsulated
 *                            properly to generate a decapsulated frame
 * - SL_STATUS_FAIL           If not all incapsulated commands processed correctly
 * - SL_STATUS_OK             If all incapsulated commands processed correctly.
 */
static sl_status_t zwave_command_class_multi_command_support_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 5) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame_data[COMMAND_CLASS_INDEX] != COMMAND_CLASS_MULTI_CMD
      || frame_data[COMMAND_INDEX] != MULTI_CMD_ENCAP) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // This field MUST specify the number of encapsulated commands.
  // This field SHOULD be set to a value greater than 1.

  if (frame_data[NUMBER_OF_COMMANDS_INDEX] < 1) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  zwave_controller_connection_info_t info = *connection_info;
  sl_status_t command_handler_status      = SL_STATUS_OK;

  uint8_t decapsulated_frame[ZWAVE_MAX_FRAME_SIZE] = {0};
  uint8_t encap_cmd_number = frame_data[NUMBER_OF_COMMANDS_INDEX];

  uint16_t offset = NUMBER_OF_COMMANDS_INDEX + 1;  // Command Length index
  uint16_t decapsulated_frame_length = 0;

  for (uint8_t i = 0; i < encap_cmd_number; i++) {
    decapsulated_frame_length = frame_data[offset++];

    if (decapsulated_frame_length > ZWAVE_MAX_FRAME_SIZE) {
      return SL_STATUS_FAIL;
    }

    if (offset + decapsulated_frame_length > frame_length) {
      return SL_STATUS_FAIL;
    }

    memcpy(decapsulated_frame, &frame_data[offset], decapsulated_frame_length);

    offset += decapsulated_frame_length;

    command_handler_status
      |= zwave_command_handler_dispatch(&info,
                                        decapsulated_frame,
                                        decapsulated_frame_length);
  }

  if (command_handler_status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_multi_command_init()
{
  // Register Multi command CC handler to the Z-Wave CC framework
  zwave_command_handler_t handler = {};
  handler.support_handler = &zwave_command_class_multi_command_support_handler;
  handler.control_handler = NULL;
  handler.minimal_scheme  = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.command_class   = COMMAND_CLASS_MULTI_CMD;
  handler.command_class_name = "Multi Command";
  handler.version            = MULTI_CMD_VERSION;

  return zwave_command_handler_register_handler(handler);
}
