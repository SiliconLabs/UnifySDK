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
 * @defgroup uic_attribute_transitions UIC Attribute transitions
 * @ingroup uic_components
 * @brief Attribute transition helper component.
 *
 * Component that performs linear value transitions respecting a given duration
 * for attributes, between the REPORTED and DESIRED values.
 *
 * @{
 */

#ifndef ATTRIBUTE_TRANSITIONS_H
#define ATTRIBUTE_TRANSITIONS_H

// Includes from other UIC components
#include "sl_status.h"
#include "attribute_store.h"

// Contiki
#include "clock.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Starts a transition from the REPORTED to the DESIRED value of an attribute
 * following the indicated duration
 *
 * If a transition is already ongoing for that attribute node, it will be stopped
 * and a new transition will be started.
 *
 * @param node      The attribute store node value to transition. The type of
 *                  the value of must be uint32_t.
 * @param duration  The time that the duration must take.
 *
 * @returns SL_STATUS_OK if the transition was initiated, SL_STATUS_FAIL in case
 *          of error.
 */
sl_status_t attribute_start_transition(attribute_store_node_t node,
                                       clock_time_t duration);

/**
 * @brief Stops an ongoing transition for an attribute
 *
 * If a transition is already ongoing for that attribute node, it will be stopped
 * and a new transition will be started.
 *
 * @param node      The attribute store node for which the transition must
 *                  be stopped, if ongoing
 *
 * @returns SL_STATUS_OK
 */
sl_status_t attribute_stop_transition(attribute_store_node_t node);

/**
 * @brief Verifies if a transition is ongoing for an attribute
 *
 * @param node      The attribute store node to check for ongoing transitions
 *
 * @returns true if the node is undergoing a transition, false otherwise.
 */
bool is_attribute_transition_ongoing(attribute_store_node_t node);

/**
 * @brief Retrieves the remaining time for a transition
 *
 * @param node       The attribute store node to retrieve the remaining duration
 *
 * @returns The clock_time_t in ms remaining for the transitions.
 *          0 if no transition is ongoing for the node
 */
clock_time_t
  attribute_transition_get_remaining_duration(attribute_store_node_t node);

/**
 * @brief Initializes the UIC attribute transition component
 *
 * @returns SL_STATUS_OK on success, any other value otherwise
 */
sl_status_t attribute_transitions_init();

/**
 * @brief Teardown the UIC attribute transition component
 * @returns 0 on success.
 */
int attribute_transitions_teardown();

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_TRANSITIONS_H
/** @} end attribute_transitions */
