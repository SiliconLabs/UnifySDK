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

/**
 * @file command_line_input.h
 * @ingroup ZWAVE_API_DEMO
 * @brief Platform specific input scanner, waiting up to a certain time for
 * reading data from the command line and return the string stripped of line
 * returns
 */

#ifndef COMMAND_LINE_INPUT_H
#define COMMAND_LINE_INPUT_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Read data from the command line (stdin) and copies it in the user
 * buffer. It strips the line return characters from the input.
 * @param user_buffer buffer for the data to be copied in, if any
 * @param user_buffer_length Length of the user buffer. If the user_buffer is
 * too small to contain the input data, only the fitting size of bytes will be
 * copied
 * @param timeout Maximum time to wait in ms for input.
 * @returns the length of the input data copied into the user buffer. Will be 0
 * if no data was copied or if timed out waiting for command line input.
 */
uint16_t read_command_line(char *user_buffer,
                           uint16_t user_buffer_length,
                           uint16_t timeout);

#ifdef __cplusplus
}
#endif

#endif  //COMMAND_LINE_INPUT_H