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
 * @defgroup zwave_command_class_association_types Type definitions for attribute storage of the Association/Multi Channel Association/AGI Command Classes.
 * @ingroup zpc_attribute_store_command_classes_types
 * @brief Type definitions for the Association/Multi Channel Association/AGI Command Classes.
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_ASSOCIATION_TYPES_H
#define ZWAVE_COMMAND_CLASS_ASSOCIATION_TYPES_H

#include "zwave_node_id_definitions.h"

// Association marker that separates the NodeID association and Endpoint association
// in the association report bytestream.
#define ASSOCIATION_MARKER 0x00

/**
 * @brief Association type possible values
 */
typedef enum { NODE_ID = 0, ENDPOINT = 1 } association_type_t;

/**
 * @brief Association object
 */
typedef struct {
  ///> Association Type
  association_type_t type;
  ///> NodeID for the association. Always valid
  zwave_node_id_t node_id;
  ///> NodeID for the association. Valid only if type=ENDPOINT
  zwave_endpoint_id_t endpoint_id;
} association_t;

///> Association Group ID Type
typedef uint8_t association_group_id_t;
///> Association Group Capacity Type
typedef uint32_t association_group_capacity_t;
///> Association Group count Type
typedef uint32_t association_group_count_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_ASSOCIATION_TYPES_H
/** @} end zwave_command_class_association_types */
