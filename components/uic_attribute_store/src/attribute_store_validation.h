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
 * @defgroup attribute_store_validation Attribute Store validation
 * @ingroup attribute_store
 * @brief Attribute Store validation utility
 *
 * Helper functions that validate valid operations in the attribute store.
 *
 * @{
 */

#ifndef ATTRIBUTE_STORE_VALIDATION_H
#define ATTRIBUTE_STORE_VALIDATION_H

#include "attribute_store.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Verifies if a write operation respects the registered type storage.
 *
 * @param node_type    Attribute Store Node type on which the write will be done.
 * @param value       \ref attribute_store_set_node_attribute_value
 * @param value_size  \ref attribute_store_set_node_attribute_value
 * @returns true is the write operation is valid and should be allowed, false
 *          otherwise
 */
bool is_write_operation_valid(attribute_store_type_t node_type,
                              const uint8_t *value,
                              uint8_t value_size);

/**
 * @brief Verifies if a node addition in the attribute store is allowed
 *
 * @param node_type     Node type of the node to add
 * @param parent_type   Node type of the parent node
 * @returns true is the nodde addition is valid and should be allowed, false
 *          otherwise
 */
bool is_node_addition_valid(attribute_store_type_t node_type,
                            attribute_store_type_t parent_type);

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_STORE_VALIDATION_H
/** @} end attribute_store_validation */
