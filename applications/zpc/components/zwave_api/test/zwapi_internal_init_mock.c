/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "zwapi_internal.h"
#include "zwapi_session_mock.h"
#include "zwapi_connection.h"

#include "sl_log.h"

#define WEAK_ATTRIBUTE __attribute__((weak))

// This file adds mock functions for Z-Wave API init functions (zwapi_init.c)
// defined in the zwapi_internal.h header.
sl_status_t zwapi_send_command(uint8_t command,
                               const uint8_t *payload_buffer,
                               uint8_t payload_buffer_length)
{
  return SL_STATUS_OK;
}

sl_status_t zwapi_send_command_with_response(uint8_t command,
                                             const uint8_t *payload_buffer,
                                             uint8_t payload_buffer_length,
                                             uint8_t *response_buffer,
                                             uint8_t *response_length)
{
  return zwapi_session_send_frame_with_response(command,
                                                payload_buffer,
                                                payload_buffer_length,
                                                response_buffer,
                                                response_length);
}

/**
 * @brief Send a command to the Z-Wave module without waiting for any ACK.
 *
 * @param command               Z-Wave Serial API Command (zwapi_func_ids.h)
 * @param payload_buffer        Byte array with serial API command parameters
 * @param payload_buffer_length Length in bytes of parameter array
 * @returns SL_STATUS_OK if the frame is supported by the Z-Wave module.
 * SL_STATUS_FAIL otherwise
 *
 * Verifies that the command is supported by the Z-Wave module, sends the
 * command data frame to the Z-Wave module but does not wait for any ACK
 */
sl_status_t zwapi_send_command_no_ack(uint8_t command,
                                      const uint8_t *payload_buffer,
                                      uint8_t payload_buffer_length)

{
  return SL_STATUS_OK;
}
