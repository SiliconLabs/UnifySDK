/* # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

// Standard libraries includes
#include <stdio.h>
#include <string.h>

// NULL definition
#include <stddef.h>

// Includes from this component
#include "dev_cli_command_runner.h"
#include "dev_cli_run.h"
#include "dev_cli_config.h"

// Includes UIC components
#include "sys/process.h"
#include "sl_log.h"

#define LOG_TAG "dev_cli_run"

PROCESS(dev_cli_run_process, "Developer CLI Run");

PROCESS_THREAD(dev_cli_run_process, ev, data)
{
  PROCESS_BEGIN()

  while (1) {
    if (ev == PROCESS_EVENT_INIT) {
      int state = run_command(dev_cli_get_config()->command);
      if (state == 0) {
        sl_log_debug(LOG_TAG, "Running the command failed\n");
        process_post(PROCESS_BROADCAST, PROCESS_EVENT_EXIT, 0);
      }

    } else if (ev == PROCESS_COMMAND_DONE) {
      process_post(PROCESS_BROADCAST, PROCESS_EVENT_EXIT, 0);
    } else if (ev == PROCESS_EVENT_EXIT) {
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}