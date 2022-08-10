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

/* contiki includes */
#include "process.h"
#include "procinit.h"
#include "etimer.h"

/* Includes from other components */
#include "sl_log.h"

/* Includes from this component */
#include "uic_main_externals_internal.h"

/** Default wait time in external event select.
 *
 * Unit is contiki clock_time_t units.
 * FIXME: move this to some global definitions place. */
#ifdef ZWAVE_BUILD_SYSTEM
#define UIC_DEFAULT_CLOCK_TIME_SELECT_DELAY  0
#else
#define UIC_DEFAULT_CLOCK_TIME_SELECT_DELAY 300
#endif

/* Prototypes */

static clock_time_t uic_main_loop_find_delay(void);

/* Static functions */

/* FIXME - sleep longer */
static clock_time_t uic_main_loop_find_delay(void)
{
  clock_time_t delay                    = UIC_DEFAULT_CLOCK_TIME_SELECT_DELAY;
  clock_time_t time_next_etimer_timeout = etimer_next_expiration_time();

  /* If time_next_etimer_timeout is 0, there are no outstanding timers. */
  if (time_next_etimer_timeout != 0) {
    clock_time_t time_now = clock_time();
    if (clock_lte(time_next_etimer_timeout, time_now)) {
      /* A contiki timer already expired, better get on with it.
       * But we just want to check if anything came in on the file
       * descriptors first (e.g., LAN/PAN events). */
      delay = 0;
    } else {
      /* Wait in select until the next etimer timeout. */
      delay = time_next_etimer_timeout - time_now;
    }
  }
  return delay;
}

/* Public functions. */

bool uic_main_loop_run(void)
{
#ifdef ZWAVE_BUILD_SYSTEM
  /* Check if there are time-out events by polling etimer. */
  etimer_request_poll();
#endif

  int retval = 1;
  /* Keep going as long as there are events on the event queue or
   * poll has been requested (process_run() processes all polls
   * and one event every time it's called).
   */
  while (retval > 0) {
    retval = process_run();
  }

  /* Bail out if all the processes have stopped running */
  if (PROCESS_LIST() == NULL) {
    return false;
  }

  clock_time_t delay = uic_main_loop_find_delay();

  /* Check for external events, e.e., on file descriptors, and
    * send them to their respective handlers.
    *
    * Poll handlers are only expected to read the data and post a
    * conktiki event to handle it later. */
#ifdef ZWAVE_BUILD_SYSTEM 
  if (delay) {
    uic_main_ext_select(delay);
  }
#else
  uic_main_ext_select(delay);
  /* Check if there are time-out events by polling etimer. */
  etimer_request_poll();
#endif

  return true;
}


bool uic_main_loop(void)
{
  while (1) {
    if(!uic_main_loop_run()) {
      break;
    }
  }
  return true;
}
