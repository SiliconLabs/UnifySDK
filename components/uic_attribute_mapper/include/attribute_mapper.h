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
 *
 */
void attribute_mapper_config_init();

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_MAPPER_H
/** @} end attribute_mapper */
