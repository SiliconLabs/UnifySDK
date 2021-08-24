/*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "sl_log.h"

#define MAX_DEVICE_ID_SIZE 512
#define LOG_TAG            "zpc_config"
const char *get_device_id()
{
  static char buff[MAX_DEVICE_ID_SIZE]          = {0};
  static char device_id[MAX_DEVICE_ID_SIZE + 1] = {0};
  FILE *fp                                      = NULL;
#ifdef __APPLE__
  const char *command = "ioreg -d2 -c IOPlatformExpertDevice | awk -F\\\" "
                        "'/IOPlatformUUID/{print $(NF-1)}' |tr -d '\n'";
#else
  const char *command = "ls /dev/disk/by-uuid | tail -1 |tr -d '\n'";
#endif
  int i = 0;
  int j = 0;

  fp = popen(command, "r");
  if (fp == NULL) {
    goto ERROR_EXIT;
  }
  if (fgets(buff, sizeof(buff), fp) == NULL) {
    goto ERROR_EXIT;
  }
  pclose(fp);
  if (strlen(buff) == 0) {
    goto ERROR_EXIT;
  }
  for (j = 0; j < strlen(buff); j++) {
    if (buff[j] != '-') {
      device_id[i++] = buff[j];
    }
  }
  return (const char *)device_id;
ERROR_EXIT:
  sl_log_error(LOG_TAG,
               "Failure in finding device_id by /dev/disk/by-uuid."
               " Try setting zpc.device_id in UIC config file\n");
  return (const char *)device_id;
}
