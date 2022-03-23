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

#include "uic_main_loop.h"
#include "sys/process.h"
#include "sys/ctimer.h"
#include "sys/procinit.h"

#include "SwTimer.h"
#include "ZW_timer.h"

//#define DEBUGPRINT
#include "DebugPrint.h"

static bool initialized = false;
static SSwTimer contikiLoopTimer;

/**
 * Timer timeout callback function to run the contiki process.
 * This function is repeatedly called to act as a loop or a separate task.
 */
static void uic_main_loop_runner(SSwTimer* pTimer)
{
  (void)pTimer;
}

/* Initialize and launch contiki processes */
static void uic_main_contiki_setup(void)
{
  /* To initialize Contiki OS first call these init functions. */
  process_init();
  /* Start ctimers */
  ctimer_init();
  /* Start etimer process */
  procinit_init();
}

/* Exported functions */
void uic_main_ext_select(clock_time_t delay)
{
  if (!initialized) {
    uic_main_contiki_setup();
    ZwTimerRegister(&contikiLoopTimer, false, uic_main_loop_runner);
    initialized = true;
  }

  /**
   * Here we will set the timer to trigger after 'delay' time in ms.
   * Contiki is not preemptive, so there will be no reason to alter
   * the delay once it is set for the timer.
   */
  TimerStart(&contikiLoopTimer, delay);
}
