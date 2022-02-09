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

#ifndef ATTRIBUTE_STORE_CALLBACKS_H
#define ATTRIBUTE_STORE_CALLBACKS_H

// Includes from this component
#include "attribute_store.h"

// Generic includes
#include <map>

/**
 * @defgroup attribute_store_callbacks Attribute Store callbacks registration module
 * @ingroup attribute_store
 * @brief Module handling the registration and invokation of callbacks functions
 * for other components.
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
// Shared callback functions among the component
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Invokes all 3 types of callback functions.
 *
 * @param updated_node  The node handle of the node for which something
 *                      has changed
 * @param type          Th etype of the node that has just changed
 * @param value_state   The value state of the node that has just changed.
 * @param change        The type of change that the node underwent
 */
void attribute_store_invoke_callbacks(
  attribute_store_node_t updated_node,
  attribute_store_type_t type,
  attribute_store_node_value_state_t value_state,
  attribute_store_change_t change);

/**
 * @brief Invoke all generic callback functions
 *
 * @param change_event  pointer to attribute meta data which describes what changed
 */
void attribute_store_invoke_generic_callbacks(
  attribute_changed_event_t *change_event);

/**
 * @brief Invokes all types of touch callbacks.
 *
 * @param touched_node    The node handle of the node that was touched.
 */
void attribute_store_invoke_touch_callbacks(
  attribute_store_node_t touched_node);

/**
 * @brief Invokes all generic touch callbacks
 *
 * @param touched_node    The node handle of the node that was touched.
 */
void attribute_store_invoke_touch_generic_callbacks(
  attribute_store_node_t touched_node);

/**
 * @brief Invoke all callback functions associated to an
 * attribute id
 *
 * @param updated_node  The node handle of the node for which something
 *                      has changed
 * @param type          The type of the node that has just changed
 * @param change        The type of change that the node underwent
 */
void attribute_store_invoke_type_callbacks(attribute_store_node_t updated_node,
                                           attribute_store_type_t type,
                                           attribute_store_change_t change);

/**
 * @brief Invoke all callback functions associated to an
 * attribute id / value state combination
 *
 * @param updated_node  The node handle of the node for which something
 *                      has changed
 * @param type          Th etype of the node that has just changed
 * @param value_state   The value state of the node that has just changed.
 * @param change        The type of change that the node underwent
 */
void attribute_store_invoke_value_callbacks(
  attribute_store_node_t updated_node,
  attribute_store_type_t type,
  attribute_store_node_value_state_t value_state,
  attribute_store_change_t change);

/**
 * @brief Reset resources allocated for the attribute_store_callback module
 * @returns SL_STATUS_OK as it is always successful.
 */
sl_status_t attribute_store_callbacks_init(void);

/**
 * @brief Frees resources allocated for the attribute_store_callback module
 * @returns 0 as it is always successful.
 */
int attribute_store_callbacks_teardown(void);

#ifdef __cplusplus
}
#endif

/** @} end attribute_store_callbacks */

#endif  // ATTRIBUTE_STORE_CALLBACKS_H