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
 * @defgroup uic_attribute_timeouts UIC Attribute timeouts
 * @ingroup uic_components
 * @brief Attribute timeout/callback helper component.
 * @{
 */

#ifndef ATTRIBUTE_TIMEOUTS_H
#define ATTRIBUTE_TIMEOUTS_H

// Includes from other UIC components
#include "sl_status.h"
#include "attribute_store.h"

// Contiki
#include "clock.h"

/**
 * @brief Function prototype callback functions.
 */
typedef void (*attribute_timeout_callback_t)(attribute_store_node_t);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Sets a callback to be called after a duration for an attribute.
 *
 * There can be only one node/callback_function combination active at a time.
 * It means that for example, calling the API in the following sequence:
 * - attribute_timeout_set_callback(1,100,&function1);
 * - attribute_timeout_set_callback(1,1000,&function1);
 *
 * Will lead to only 1 call of function1 in 1000ms from now.
 *
 * As another example, calling the API in the following sequence:
 * - attribute_timeout_set_callback(1,100,&function1);
 * - attribute_timeout_set_callback(1,1000,&function2);
 * - attribute_timeout_set_callback(2,1000,&function2);
 *
 * Will lead to 3 function calls:
 * - function1(1) in 100ms
 * - function2(1) in 1000ms
 * - function2(2) in 1000ms
 *
 * @param node               The attribute store node to inspect after the duration.
 * @param duration           The time in ms for the duration to wait.
 * @param callback_function  The function to invoke for the attribute node inspection.
 *
 * @returns SL_STATUS_OK if the timer is started and the callback will be invoked.
 *          SL_STATUS_FAIL in case of error.
 */
sl_status_t attribute_timeout_set_callback(
  attribute_store_node_t node,
  clock_time_t duration,
  attribute_timeout_callback_t callback_function);

/**
 * @brief Cancels a callback to be called after a duration for an attribute.
 *
 * @param node               The attribute store node to inspect after the duration.
 * @param callback_function  The function to invoke for the attribute node inspection.
 *
 * @returns SL_STATUS_OK, indicating that the callback function will not be called.
 *          SL_STATUS_NOT_FOUND if this callback was not active for the node.
 */
sl_status_t attribute_timeout_cancel_callback(
  attribute_store_node_t node, attribute_timeout_callback_t callback_function);

/**
 * @brief Initializes the UIC attribute timeouts component
 *
 * @returns SL_STATUS_OK on success, any other value otherwise
 */
sl_status_t attribute_timeouts_init();

/**
 * @brief Teardown the UIC attribute timeouts component
 * @returns 0 on success.
 */
int attribute_timeouts_teardown();

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_TIMEOUTS_H
/** @} end attribute_timeouts */
