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

/**
 * @defgroup attribute_store_process Attribute Store Process
 * @ingroup attribute_store
 *
 * @brief Contiki process for the Attribute Store
 *
 * This component allows the Attribute Store to perform non-sequential
 * operations.
 *
 * @{
 */

#ifndef ATTRIBUTE_STORE_PROCESS_H
#define ATTRIBUTE_STORE_PROCESS_H

#include "process.h"

/**
 * @brief Name the of \ref contiki process for the Unify Attribute Store.
 *
 * This is used to identify the Attribute Store Process.
 */
PROCESS_NAME(unify_attribute_store_process);

/**
 * @brief Event definitions for the Unify Attribute Store Process.
 */
typedef enum {
  /// Restart the auto-save safety timer.
  RESTART_AUTO_SAVE_SAFETY_TIMER_EVENT,
  /// Restart the auto-save cooldown timer.
  RESTART_AUTO_SAVE_COOLDOWN_TIMER_EVENT,
} unify_attribute_store_events_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Notify the Attribute Store process that the attribute store
 * was just saved to SQLite.
 */
void attribute_store_process_on_attribute_store_saved();

/**
 * @brief Restarts the auto-save safety timer
 */
void attribute_store_process_restart_auto_save_safety_timer();

/**
 * @brief Restarts the auto-save cooldown timer
 */
void attribute_store_process_restart_auto_save_cooldown_timer();

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_STORE_PROCESS_H
/** @} end attribute_store_process */
