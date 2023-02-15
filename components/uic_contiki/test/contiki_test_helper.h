/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @file contiki_test_helper.h
 * @ingroup contiki_test
 * @brief Helpers function to implement a Contiki test case
 *
 * @{
 */

#ifndef CONTIKI_TEST_HELPER_H
#define CONTIKI_TEST_HELPER_H

#include "sys/clock.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Contiki test helper.
 *
 * This function should be called at the beginning of every test case.
 * This emulates the startup of Contiki, with a test_process and etimer
 * system stated.
 *
 * When this function exits, the Contiki system is in the context of the
 * test process. Which means that all subsequent commands, such as
 * process_start, post_event etc, are executed in the context of the
 * test process.
 */
void contiki_test_helper_init();

/**
 * @brief Run the Contiki event loop once.
 *
 * This function does exactly the same thing as \ref contiki_test_helper_run()
 * but runs the event only once.
 *
 * @param ms Number of milliseconds to progress time.
 */
void contiki_test_helper_run_once(clock_time_t ms);

/**
 * @brief Run the Contiki event loop.
 *
 * This function runs the Contiki event system. It will switch context
 * to the scheduler, progress the system time by a specified amount and
 * execute all pending/triggered events. When exiting, the Contiki
 * system is left in the context of the test process.
 *
 * @param ms Number of milliseconds to progress time.
 */
void contiki_test_helper_run(clock_time_t ms);

/**
 * @brief Run the Contiki event loop.
 *
 * This function runs the Contiki event system. It will switch context
 * to the scheduler, progress the system time by a specified amount and
 * execute all pending/triggered events. When exiting, the Contiki
 * system is left in the context of the test process.
 *
 * @param ms Number of milliseconds to progress time, for each event
 */
void contiki_test_helper_run_clock_increases_for_each_event(clock_time_t ms);

/**
 * @brief Increases the fake test clock with an amount of time.
 *
 * @param ms Number of milliseconds to progress time
 */
void contiki_test_helper_increase_clock(clock_time_t ms);

#ifdef __cplusplus
}
#endif
/** @} end contiki_test_helper */

#endif //CONTIKI_TEST_HELPER_H
