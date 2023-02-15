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

#ifndef ATTRIBUTE_STORE_INTERNAL_H
#define ATTRIBUTE_STORE_INTERNAL_H

// Includes from this component
#include "attribute_store.h"

// Fixed unique IDs for the datastore
/// NULL database ID, used to represent a non-existing node (i.e root's node parent)
#define ATTRIBUTE_STORE_NULL_ID (attribute_store_node_t)0
/// Root node database ID. We always set that to the same value
#define ATTRIBUTE_STORE_ROOT_ID (attribute_store_node_t)1

/**
 * @brief Saves a node that's in-memory in the datastore.
 */
#define STORE_ATTRIBUTE(node)                            \
  datastore_store_attribute(node->id,                    \
                            node->type,                  \
                            node->parent_node->id,       \
                            &node->reported_value[0],    \
                            node->reported_value.size(), \
                            &node->desired_value[0],     \
                            node->desired_value.size())

/**
 * @brief Saves the root node in the datastore.
 */
#define STORE_ROOT_ATTRIBUTE(root_node)                       \
  datastore_store_attribute(root_node->id,                    \
                            root_node->type,                  \
                            ATTRIBUTE_STORE_NULL_ID,          \
                            &root_node->reported_value[0],    \
                            root_node->reported_value.size(), \
                            &root_node->desired_value[0],     \
                            root_node->desired_value.size())


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Saves the entire attribute store to the datastore
 *
 * @returns SL_STATUS_OK on success, other codes in case of error
 */
sl_status_t attribute_store_save_to_datastore();

/**
 * @brief Loads the entire attribute store from the datastore
 *
 * @returns SL_STATUS_OK on success, other codes in case of error
 */
sl_status_t attribute_store_load_from_datastore();

#ifdef __cplusplus
}
#endif

#endif  // ATTRIBUTE_STORE_INTERNAL_H