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
 * @defgroup attribute_poll Attribute Poll Engine
 * @ingroup unify_components
 * @brief The Attribute Polling Engine supplies the functionality of
 * polling attribute reported values with a supplied interval.
 *
 * With APE it is possible to register attributes, that shall be polled together
 * with an interval with which the attribute is desired to be polled. APE has
 * following features:
 *   - Backoff interval between polls to prevent flodding the network with polls
 *   - Maximum number of polls per hour
 *
 * @{
 */

#ifndef ATTRIBUTE_POLL_ENGINE_H
#define ATTRIBUTE_POLL_ENGINE_H

#include "sl_status.h"
#include "attribute_store.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register a node for polling
 *
 * @param node Attribute store node id to poll
 * @param interval desired polling interval in seconds
 * @returns sl_status_t
 */
sl_status_t attribute_poll_register(attribute_store_node_t node,
                                    uint32_t interval);

/** \ref attribute_poll_register with default polling interval */
#define attribute_poll_register_default(node) attribute_poll_register(node, 0)

/**
 * @brief Deregister polling of an attribute
 *
 * @param node Node to deregister polling for
 * @returns sl_status_t
 */
sl_status_t attribute_poll_deregister(attribute_store_node_t node);

/**
 * @brief Schedule a poll as soon as possible
 *
 * @param node    Attribute store Node to add to the poll list.
 * @returns sl_status_t
 */
sl_status_t attribute_poll_schedule(attribute_store_node_t node);

/**
 * @brief Initialize the attribute poll engine
 *
 * @param backoff                  The minimum time in seconds between 2 polls.
 * @param default_interval         Default interval in secoénd, at which
 *                                 attributes will be polled, if no value is
 *                                 provided for a specific attribute type
 * @param poll_mark_attribute_type The Attribute Store type to use to mark attributes
 *        as pollable. Note that this attribute must not be registered with a
 *        fixed parent (use ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE) or
 *        storage data type (use UNKNOWN_STORAGE_TYPE).
 *
 * @returns sl_status_t SL_STATUS_OK on success
 */
sl_status_t
  attribute_poll_init(uint32_t backoff,
                      uint32_t default_interval,
                      attribute_store_type_t poll_mark_attribute_type);

/**
 * @brief Disable the Poll Engine
 *
 * Stop the Poll Engine from triggering any new polls
 *
 * @return sl_status_t
 */
sl_status_t attribute_poll_disable();

/**
 * @brief Enable the Poll Engine
 *
 * Start the Poll Engine, the engine is default started, so this only makes
 * sense to call after calling \ref attribute_poll_disable.
 *
 * @return sl_status_t
 */
sl_status_t attribute_poll_enable();

/**
 * @brief print the entries present in the attribute poll list
 *
 */
void attribute_poll_print();

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_POLL_ENGINE_H
/** @} end attribute_poll_engine */
