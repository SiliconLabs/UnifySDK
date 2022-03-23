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

/* Includes from other components */
#include "sl_log.h"

/* Includes from this component */
#include "uic_main.h"
#include "uic_init.h"
#include "uic_main_loop.h"
#include "uic_component_fixtures_internal.h"

#define LOG_TAG "uic_main"

extern int uic_stdin_teardown();

int uic_main(const uic_fixt_setup_step_t *fixt_app_setup,
             const uic_fixt_shutdown_step_t *fixt_app_shutdown,
             int argc,
             char **argv,
             const char *version)
{
  int res = 0;

  sl_status_t status = uic_init(fixt_app_setup, argc, argv, version);

  if (status == SL_STATUS_PRINT_INFO_MESSAGE) {
    uic_stdin_teardown();  //FIXME reconsider startup and teardown structue
    return 0;
  } else if (status != SL_STATUS_OK) {
    uic_stdin_teardown();  //FIXME reconsider startup and teardown structue
    return status == SL_STATUS_ABORT ? 2 : 1;
  }

  uic_main_loop();

  /* Clean up stuff allocated or set up in the init stage. */
  res = uic_fixt_shutdown_loop(fixt_app_shutdown);
  return res;
}
