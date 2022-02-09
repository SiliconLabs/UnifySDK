/* # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

/* Generic includes */
#include <stdbool.h>
#include "contiki-conf.h"

static bool initialized = false;
static SSwTimer contikiLoopTimer;

/* Exported functions */
void uic_main_ext_select(clock_time_t delay)
{
  if (!initialized) {
    ZwTimerRegister(&contikiLoopTimer, false, uic_main_loop_run);
    initialized = true;
  }

  /**
   * Here we will set the timer to trigger after 'delay' time in ms.
   * Contiki is not preemptive, so there will be no reason to alter
   * the delay once it is set for the timer.
   */
  TimerStart(&contikiLoopTimer, delay);

  // TODO Do we have external events that we want to poll here?
}
