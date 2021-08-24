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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>

uint16_t read_command_line(char *user_buffer,
                           uint16_t user_buffer_length,
                           uint16_t timeout)
{
  uint16_t read_length = 0;
  fd_set fileset;
  FD_ZERO(&fileset);
  FD_SET(STDIN_FILENO, &fileset);

  struct timeval tv;
  tv.tv_sec  = 0;
  tv.tv_usec = timeout * 1000;

  int select_status = select(STDIN_FILENO + 1, &fileset, NULL, NULL, &tv);

  if (select_status > 0) {
    // Data is available, copy it in the user_buffer
    read_length = read(STDIN_FILENO, user_buffer, user_buffer_length - 1);
    user_buffer[read_length++] = '\0';

    // Strip line returns, as this may be platform specific and the
    // application does not care about this
    for (uint16_t i = 0; i < read_length; i++) {
      if (user_buffer[i] == '\n') {
        user_buffer[i] = '\0';
        read_length    = i;
      }
    }
  }

  return read_length;
}