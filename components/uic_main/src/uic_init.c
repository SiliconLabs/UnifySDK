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

/* Generic includes */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
/* contiki includes */
#include "process.h"
#include "procinit.h"
#include "autostart.h"
#include "sys/ctimer.h"
#include "uic_version.h"

/* Includes from other components */
#include "sl_log.h"
#include "config.h"

/* Includes from this component */
#include "uic_main.h"
#include "uic_init.h"
#include "uic_main_loop.h"
#include "uic_component_fixtures_internal.h"
//#include "zpc_config.h"

#define LOG_TAG "uic_init"

/* Prototypes */

static void uic_main_contiki_setup(void);

/* Static functions */

/* Initialize and launch contiki processes */
static void uic_main_contiki_setup(void)
{
  /* To initialize Contiki OS first call these init functions. */
  process_init();
  /* Start ctimers */
  ctimer_init();
  /* Start etimer process */
  procinit_init();

  /* Make standard output unbuffered. */
  setvbuf(stdout, (char *)NULL, _IONBF, 0);
}

/**
 * Read in the configuration parameters. Initialize and start up
 * contiki OS.  Initialize and start up the UIC processes.
 */
bool uic_init(const uic_fixt_setup_step_t *fixt_app_setup,
              int argc,
              char **argv,
              const char *version)
{
  int ret = 0;
  /* Import the system configuration. */
  printf("# uic build: %s\n", UIC_VERSION);
  ret = config_parse(argc, argv, version);
  if (ret) {
    if (ret != CONFIG_STATUS_NOK) {
      sl_log_critical(LOG_TAG, "Cannot initialize UIC Main - goodbye!\n");
    }
    return false;
  }
  // Init log as early as possible after parsing config
  sl_log_read_config(NULL);
  uic_main_contiki_setup();

  /* Initialize system components.
    *
    * If set-up works, initialize contiki OS and start autostart
    * processes. */
  if (uic_fixt_setup_loop(fixt_app_setup)) {
    /* Initialize contiki OS - timers and processes. */

    return true;
  } else {
    return false;
    sl_log_critical(LOG_TAG, "Cannot initialize UIC Main - goodbye!\n");
  }
}
