/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef ATTRIBUTE_STORE_INTERNALS_H
#define ATTRIBUTE_STORE_INTERNALS_H

// Includes from this component
#include "attribute_store.h"

// Generic includes
#include <map>

// Fixed unique IDs for the datastore
/// NULL database ID, used to represent a non-existing node (i.e root's node parent)
#define ATTRIBUTE_STORE_NULL_ID (attribute_store_node_t)0
/// Root node database ID. We always set that to the same value
#define ATTRIBUTE_STORE_ROOT_ID (attribute_store_node_t)1

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  // ATTRIBUTE_STORE_INTERNALS_H