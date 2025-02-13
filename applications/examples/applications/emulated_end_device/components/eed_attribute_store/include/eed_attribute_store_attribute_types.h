/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup eed_attribute_types List of defined attributes types
 * @ingroup eed_attribute_store
 * @brief List of defines Attribute Types for the attribute store.
 *
 * <b>Attribute Type Ranges:</b>
 * Range From | Range To   | Type
 * ---------- | ---------- | ----------------------------------
 * 0x00000000 | 0x00000001 | Attribute Store
 * 0x00000002 | 0x0000FFFF | Application Attributes
 * 0x00010000 | 0xFFFFFFFF | ZigBee Cluster Command Attributes
 *
 * @{
 */

#ifndef ATTRIBUTE_STORE_TYPES_H
#define ATTRIBUTE_STORE_TYPES_H

#include "attribute_store.h"
#include "uic_attribute_definitions.h"
#include "unify_dotdot_defined_attribute_types.h"

// Generic Node attributes, should be attached under NodeID nodes and not endpoints.
// Suggested range 0x01..0xFF

///< This represents a UNID. C-String type
DEFINE_ATTRIBUTE(ATTRIBUTE_UNID, 0x0002)
///< This represents an Endpoint ID. dotdot_endpoint_id_t type.
DEFINE_ATTRIBUTE(ATTRIBUTE_ENDPOINT_ID, 0x0003)

// ZigBee specific attributes
// Suggested range (ZigBee Cluster ID (2 bytes) <<16) | Zigbee attribute ID (2 bytes)

#endif  //ATTRIBUTE_STORE_TYPES_H
        /** @} end zpc_attribute_types */
