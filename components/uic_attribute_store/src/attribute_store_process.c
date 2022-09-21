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
#include "attribute_store_process.h"
#include "attribute_store_internal.h"
#include "attribute_store_configuration_internal.h"

// Contiki
#include "etimer.h"
#include "clock.h"

// Unify components
#include "sl_log.h"

// Generic includes
#include <stdbool.h>

// Log tag for this file.
#define LOG_TAG "attribute_store_process"

// Private variables
// Safety timer, that will ensure that we back-up to the datastore following
// at least this interval.
static struct etimer attribute_store_auto_save_safety_timer;
// Cooldown timer since last attribute modification.
static struct etimer attribute_store_auto_save_cooldown_timer;

////////////////////////////////////////////////////////////////////////////////
// Private event functions
////////////////////////////////////////////////////////////////////////////////
static void attribute_store_process_on_restart_auto_save_safety_timer_event()
{
  etimer_stop(&attribute_store_auto_save_safety_timer);
  unsigned int auto_save_safety_interval
    = attribute_store_get_auto_save_safety_interval();
  if (auto_save_safety_interval > 0) {
    sl_log_debug(LOG_TAG,
                 "Restarting Attribute Store auto-save safety timer for "
                 "%d seconds.\n",
                 auto_save_safety_interval);
    etimer_set(&attribute_store_auto_save_safety_timer,
               auto_save_safety_interval * CLOCK_SECOND);
  }
}

static void attribute_store_process_on_restart_auto_save_cooldown_timer_event()
{
  etimer_stop(&attribute_store_auto_save_cooldown_timer);
  unsigned int auto_save_cooldown_interval
    = attribute_store_get_auto_save_cooldown_interval();
  if (auto_save_cooldown_interval > 0) {
    sl_log_debug(LOG_TAG,
                 "Restarting Attribute Store auto-save cooldown timer for "
                 "%d seconds.\n",
                 auto_save_cooldown_interval);
    etimer_set(&attribute_store_auto_save_cooldown_timer,
               auto_save_cooldown_interval * CLOCK_SECOND);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Functions shared within the component
////////////////////////////////////////////////////////////////////////////////
void attribute_store_process_restart_auto_save_safety_timer()
{
  if (process_count_events(&unify_attribute_store_process,
                           RESTART_AUTO_SAVE_SAFETY_TIMER_EVENT,
                           NULL)
      == 0) {
    process_post(&unify_attribute_store_process,
                 RESTART_AUTO_SAVE_SAFETY_TIMER_EVENT,
                 NULL);
  }
}

void attribute_store_process_restart_auto_save_cooldown_timer()
{
  if (process_count_events(&unify_attribute_store_process,
                           RESTART_AUTO_SAVE_COOLDOWN_TIMER_EVENT,
                           NULL)
      == 0) {
    process_post(&unify_attribute_store_process,
                 RESTART_AUTO_SAVE_COOLDOWN_TIMER_EVENT,
                 NULL);
  }
}

void attribute_store_process_on_attribute_store_saved()
{
  etimer_stop(&attribute_store_auto_save_cooldown_timer);
  attribute_store_process_restart_auto_save_safety_timer();
}

////////////////////////////////////////////////////////////////////////////////
// Contiki Process
////////////////////////////////////////////////////////////////////////////////
PROCESS(unify_attribute_store_process, "unify_attribute_store_process");

PROCESS_THREAD(unify_attribute_store_process, ev, data)
{
  PROCESS_BEGIN();

  while (true) {
    if (ev == PROCESS_EVENT_INIT) {
      sl_log_info(LOG_TAG, "Process started. Setting up timers\n");
      attribute_store_process_on_attribute_store_saved();

    } else if (ev == PROCESS_EVENT_EXIT) {
      sl_log_info(LOG_TAG, "Process exited. Stopping timers\n");
      etimer_stop(&attribute_store_auto_save_safety_timer);
      etimer_stop(&attribute_store_auto_save_cooldown_timer);

    } else if (ev == PROCESS_EVENT_EXITED) {
      // Do not do anything with this event, just wait to go down.

    } else if (ev == PROCESS_EVENT_TIMER) {
      if ((data == &attribute_store_auto_save_safety_timer)
          || (data == &attribute_store_auto_save_cooldown_timer)) {
        sl_log_debug(LOG_TAG,
                     "Auto-save (cooldown or safety) interval elapsed. "
                     "Saving attributes to the datastore.\n");
        attribute_store_save_to_datastore();
      }

    } else if (ev == RESTART_AUTO_SAVE_SAFETY_TIMER_EVENT) {
      attribute_store_process_on_restart_auto_save_safety_timer_event();

    } else if (ev == RESTART_AUTO_SAVE_COOLDOWN_TIMER_EVENT) {
      attribute_store_process_on_restart_auto_save_cooldown_timer_event();

    } else {
      sl_log_warning(LOG_TAG,
                     "Unknown Unify Attribute Store event: %d, data=%p\n",
                     ev,
                     data);
    }

    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
