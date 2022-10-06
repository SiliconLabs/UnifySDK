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

#ifndef UIC_MAIN_H
#define UIC_MAIN_H

#include "uic_component_fixtures.h"

/**
 * @defgroup uic_main Main Function for Contiki
 * @ingroup unify_components
 * This component contains the a "main" function for using Contiki.
 *
 * It is a supporting component, that initializes the Contiki OS
 * and provides the execution loop for the Contiki scheduling mechanism.
 *
 * It supports initializing other components through setup fixture functions,
 * see @ref uic_fixt_setup_t.
 * These functions are either added to the @ref uic_fixt_setup_steps array,
 * if the component is part of the Unify library, or supplied the @ref uic_main
 * function, if it resides outside the Unify library
 *
 * If a component is implemented with a Contiki process, that process
 * shall be started in the component setup fixture using @ref process_start
 *
 * In addition, functions waking up the main process based on events on
 * sockets are described in @ref uic_main_ext
 */

/** Struct to hold the component pre-initializers (fixture setup).
 *
 *  @ingroup uic_fixt_int
 *
 * These functions are executed in order before entering the Unify main loop.
 */
typedef struct uic_fixt_setup_step {
  /** Initializer function */
  const uic_fixt_setup_t func;
  /** Pointer to a textual description of the initialization step. */
  const char *description;
} uic_fixt_setup_step_t;

/** Struct to hold the system tear-down functions.
 *
 *  \ingroup uic_fixt_int
 *
 * A component is not required to have a tear-down function just
 * because it has an initializer function (or vice versa).
 * If a component has a Contiki process, teardown of the component should be
 * done by handling the Contiki event \ref PROCESS_EVENT_EXIT
 */
typedef struct uic_fixt_shutdown_step {
  /** Tear-down function */
  const uic_fixt_shutdown_t func;
  /** Pointer to a textual description of the shutdown step. */
  const char *description;
} uic_fixt_shutdown_step_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Run Unify/Contiki Main loop
 *
 * This function will initialize Contiki OS and loop until the program exits.
 * It has following stages:
 *   1) Initialize Contiki OS (Timers, Process list etc.)
 *   2) Call all setup fixtures (loop over fixt_setup)
 *   3) Run Contiki main loop
 *   4) Upon shutdown call all shutdown fixtures (loop over  fixt_shutdown)
 *
 * @param fixt_setup Array of setup fixtures to be called during init
 * @param fixt_shutdown Array of shutdown fixtures to be called on shutdown
 * @param argc command line arguments count
 * @param argv command line arguments
 * @param version Version string to print on "--version" argument
 * @return int 0 on success, other for error
 */
int uic_main(const uic_fixt_setup_step_t *fixt_setup,
             const uic_fixt_shutdown_step_t *fixt_shutdown,
             int argc,
             char **argv,
             const char *version);



#ifdef __cplusplus
}
#endif
#endif
