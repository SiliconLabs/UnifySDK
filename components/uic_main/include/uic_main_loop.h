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

#ifndef UIC_MAIN_LOOP_H
#define UIC_MAIN_LOOP_H

#include "stdbool.h"

/** Run the main process loop.
 *
 * \ingroup uic_main
 *
 * Loop until there are no more running contiki processes.
 *
 * In the loop
 * - call contiki \ref process_run(),
 * - check for termination,
 * - ask the "external event handling component" if there are any
 *   external events and send them to the right contiki poll handler,
 * - poll contiki etimer.
 */
bool uic_main_loop(void);

/**
 * Run the main process loop in gecko.
 * @see uic_main_loop for more info.
 */
bool uic_main_loop_run(void);

/**
 * @brief wait for a maximum timeout, see UIC_DEFAULT_CLOCK_TIME_SELECT_DELAY,
 * for file descriptors to change
 */
void uic_main_wait_for_file_descriptors(void);

#endif
