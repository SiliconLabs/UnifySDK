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
 * @defgroup aoxpc_attribute_types List of defined attributes types for the AoXPC
 * @ingroup aoxpc_attribute_store
 * @brief List of defines Attribute Types for the attribute store.
 *
 * <b>Attribute Type Ranges:</b>
 * Range From | Range To   | Type
 * ---------- | ---------- | ----------------------------------
 * 0x00000000 | 0x00000001 | Attribute Store
 * 0x00000002 | 0xFFFFFFFF | AoXPC defined attributes
 * 0x00010000 | 0xFFFFFFFF | ZigBee Cluster Attributes
 *
 * @{
 */

#ifndef ATTRIBUTE_STORE_TYPES_H
#define ATTRIBUTE_STORE_TYPES_H

#include "aoxpc_attribute_store_dotdot_attributes.h"

// Generic Node attributes, should be attached under NodeID nodes and not endpoints.
// Suggested range 0x01..0xFF

///< This represents a BLE address, located under the root. aoa_id_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_BLE_UNID, 0x0002)
///< This represents a endpoint. dotdot_endpoint_id_t type.
DEFINE_ATTRIBUTE(ATTRIBUTE_ENDPOINT_ID, 0x0003)

#endif  // ATTRIBUTE_STORE_TYPES_H
/** @} */  //end attribute_types
