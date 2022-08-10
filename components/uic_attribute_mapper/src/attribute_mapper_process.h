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
 * @defgroup attribute_mapper_process Process definitions for the Unify Attribute Mapper
 * @brief Process definitions for the Unify Attribute Mapper
 *
 * The Unify Attribute Mapper helps arbitrating when to make calculations
 * based on attribute updates.
 *
 * @{
 */

#ifndef ATTRIBUTE_MAPPER_PROCESS_H
#define ATTRIBUTE_MAPPER_PROCESS_H

#include "attribute_store.h"
#include "process.h"

/**
 * @brief Name the of \ref contiki process for the Unify Attribute Mapper.
 *
 * This is used to identify the Attribute Mapper Process.
 */
PROCESS_NAME(unify_attribute_mapper_process);

/**
 * @brief Event definitions for the Unify Attribute Mapper Process.
 */
typedef enum {
  /// Look at the pending attribute updates and evaludate them.
  MAPPER_EVALUATE_PENDING_UPDATES_EVENT,
} unify_attribute_mapper_events_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Functions used to indicate that a reported attribute has been updated.
 *
 * @param updated_node      The attribute that was updated.
 * @param change            The type of change applied ot the attribute.
 */
void on_reported_attribute_update(attribute_store_node_t updated_node,
                                  attribute_store_change_t change);

/**
 * @brief Functions used to indicate that a desired attribute has been updated.
 *
 * @param updated_node      The attribute that was updated.
 * @param change            The type of change applied ot the attribute.
 */
void on_desired_attribute_update(attribute_store_node_t updated_node,
                                 attribute_store_change_t change);

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_MAPPER_PROCESS_H
/** @} end attribute_mapper_process */
