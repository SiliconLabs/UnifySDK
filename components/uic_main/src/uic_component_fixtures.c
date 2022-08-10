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
#include <stdint.h>
#include <stddef.h>

/* Includes from other components */
#include "sl_log.h"

#include "uic_component_fixtures.h"
#include "uic_component_fixtures_internal.h"

#define LOG_TAG "uic_component_fixtures"

static sl_status_t
  uic_fixt_setup_loop_list(const uic_fixt_setup_step_t *fixt_list)
{
  uint8_t ii = 0;
  while (fixt_list[ii].func != NULL) {
    sl_status_t fixture_result = fixt_list[ii].func();
    if (fixture_result == SL_STATUS_OK) {
      sl_log_info(LOG_TAG, "Completed: %s\n", fixt_list[ii].description);
      ii++;
    } else if (fixture_result == SL_STATUS_NOT_AVAILABLE) {
      sl_log_warning(LOG_TAG, 
                     "Non critical error in setup for: %s. Continuing setup.\n",
                     fixt_list[ii].description);
      ii++;
    } else if (fixture_result == SL_STATUS_ABORT) {
      sl_log_debug(LOG_TAG,
                   "Startup sequence aborted by: %s.\n",
                   fixt_list[ii].description);
      return fixture_result;
    } else {
      sl_log_critical(LOG_TAG,
                      "Failed  [%d]: %s.\n",
                      fixture_result,
                      fixt_list[ii].description);
      return fixture_result;
    }
  }
  return SL_STATUS_OK;
}

/* Run the set-up functions for the Unify components. */
sl_status_t uic_fixt_setup_loop(const uic_fixt_setup_step_t *fixt_app_setup)
{
  sl_status_t status = uic_fixt_setup_loop_list(uic_fixt_setup_steps);
  if (status != SL_STATUS_OK) {
    return status;
  }

  return uic_fixt_setup_loop_list(fixt_app_setup);
}

static int
  uic_fixt_shutdown_loop_helper(const uic_fixt_shutdown_step_t *fixt_list)
{
  int res    = 0;
  uint8_t ii = 0;

  while (fixt_list[ii].func != NULL) {
    /* Count the shutdown issues, if any. */
    res += fixt_list[ii].func() != SL_STATUS_OK;
    sl_log_info(LOG_TAG, "Shutdown %s\n", fixt_list[ii].description);
    ii++;
  }
  return res;
}

/* Run the Unify shutdown steps. */
int uic_fixt_shutdown_loop(const uic_fixt_shutdown_step_t *fixt_app_shutdown)
{
  // The uic_fixt_shutdown_steps needs to be executed after, because some
  // of the application-shutdown fixtures might use some Unify features in order
  // to gracefully shut down (e.g. OnOffCluster requires MQTT).
  int status = uic_fixt_shutdown_loop_helper(fixt_app_shutdown);
  status += uic_fixt_shutdown_loop_helper(uic_fixt_shutdown_steps);
  return status;
}
