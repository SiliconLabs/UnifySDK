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
 * @defgroup unify_attribute_mapper Unify Attribute Mapper
 * @ingroup unify_components
 * @brief Automatic attribute manipulation component
 *
 * This component offers a text file system that allows to define Attribute
 * relations, allowing automatic rules and attribute manipulations based
 * on text script files.
 *
 * @{
 */

#ifndef ATTRIBUTE_MAPPER_H
#define ATTRIBUTE_MAPPER_H

#include <stdbool.h>
#include "sl_status.h"
#include "attribute_store.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the Attribute Mapper engine.
 *
 * @return sl_status_t SL_STATUS_OK on success, any other code in case of error.
 */
sl_status_t attribute_mapper_init();

/**
 * @brief   Checks if the mapper has pending evaluations
 *
 * Components can use this function if they want to check if the mapper may
 * still update the attribute store tree after some attribute updates.
 *
 * @returns true if more evaluations are to be done, false otherwise
 */
bool attribute_mapper_has_pending_evaluations();

/**
 * @brief Adds the required configuration to the @ref unify_config for the
 * @ref unify_attribute_mapper.
 */
void attribute_mapper_config_init();

/**
 * @brief Sets the Attribute Store type for endpoint IDs, under which the the
 * mapper performs its evaluations.
 *
 * @param endpoint_id_type      Attribute Store type representing Endpoints.
 */
void attribute_mapper_set_endpoint_id_attribute_type(
  attribute_store_type_t endpoint_id_type);

/**
 * @brief Instructs the attribute mapper not to evaluate dependencies for a
 * given Attribute ID.
 *
 * @param node  Attribute ID whose updates will not trigger the mapper.
 */
void attribute_mapper_pause_reactions_to_attribute_updates(
  attribute_store_node_t node);

/**
 * @brief Instructs the attribute mapper to evaluate dependencies for a
 * given Attribute ID.
 *
 * @param node  Attribute ID whose updates will trigger the mapper.
 */
void attribute_mapper_resume_reactions_to_attribute_updates(
  attribute_store_node_t node);

/**
 * @brief Instructs the attribute mapper to pause any mapping
 */
void attribute_mapper_pause_mapping();

/**
 * @brief Instructs the attribute mapper to resume mapping
 */
void attribute_mapper_resume_mapping();

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_MAPPER_H
/** @} end attribute_mapper */
