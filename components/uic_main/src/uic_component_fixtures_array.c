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

#include <stdbool.h>

#include "sl_log.h"
#include "uic_mqtt.h"
#include "uic_stdin_process.h"

#include "uic_component_fixtures_internal.h"
#include "uic_signal_handler.h"
/** Pre-contiki set-up steps.
 *
 * \ingroup uic_fixt
 *
 * The set-up steps are executed in order, before launching contiki
 * and starting the contiki processes.
 *
 *
 * \note For some features, it is important that they are started in
 * the correct order, since they may depend on each other.
 * If this is the case, the dependency should be listed with the
 * set-up function.
 */
static uic_fixt_setup_step_t uic_fixt_setup_steps_list[]
  = {{uic_signal_handler_setup, "Unify Signal Handler"},
     {uic_mqtt_setup, "Unify MQTT Client"},
     {uic_stdin_setup, "Unify STDIN"},
     {NULL, "Terminator"}};

/** Final tear-down steps.
 *
 * @ingroup uic_fixt_int
 *
 * Functions to take down components and free allocated resources. The
 * function are executed in order, after all contiki processes have
 * been stopped.
 */
static uic_fixt_shutdown_step_t uic_fixt_shutdown_steps_list[]
  = {{uic_mqtt_teardown, "Unify MQTT Client"},
     {uic_stdin_teardown, "Unify STDIN"},
     {NULL, "Terminator"}};

/**
 * Reference to the set-up steps.
 * \ingroup uic_fixt
 */
uic_fixt_setup_step_t *uic_fixt_setup_steps = uic_fixt_setup_steps_list;

/**
 * Reference to the tear-down steps.
 * @ingroup uic_fixt
 */
uic_fixt_shutdown_step_t *uic_fixt_shutdown_steps
  = uic_fixt_shutdown_steps_list;
