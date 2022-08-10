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
 * @defgroup zwave_definitions Generic Z-Wave definitions
 * @ingroup zwave_controller
 * @brief Type definitions and constants for Z-Wave.
 *
 */

/**
 * @defgroup zwave_node_id_definitions Z-Wave NodeID definitions
 * @ingroup zwave_definitions
 * @brief Z-Wave definitions and macros around NodeIDs / Nodemasks.
 *
 * @{
 */

#ifndef ZWAVE_NODE_ID_DEFINITIONS_H
#define ZWAVE_NODE_ID_DEFINITIONS_H

#include <stdint.h>

/// Max number of nodes in a Z-Wave Long Range network
#define ZW_LR_MAX_NODE_ID 4000

/// First valid NodeID using 2 bytes
#define ZW_LR_MIN_NODE_ID 256

/// Lowest valid node id in a Z-Wave network
#define ZW_MIN_NODE_ID 1

#define ZW_MAX_NODES 232

/// Broadcast destination address in a Z-Wave Network.
#define ZWAVE_BROADCAST_NODE_ID 255

/// Broadcast destination address in a Z-Wave Network.
#define ZWAVE_LR_BROADCAST_NODE_ID 0xFFF

/// The maximum singlecast/broadcast MSDU size for data rate R3
/// Ref: ITU G.9959 table A.47
#define ZWAVE_MAX_FRAME_SIZE 158

//TODO: find ITU spec reference.
#define ZWAVE_MAX_MAC_PAYLOAD_SIZE 46

/// The Node Info Frame / Protocol CCs do not use any segmentation
/// So the maximum ever NIF will be the same size as
/// the max frame - (Command Class id / Command Class)
#define ZWAVE_MAX_NIF_SIZE (ZWAVE_MAX_FRAME_SIZE - 2)

/// Length of nodemask to hold max number of nodes in a Z-Wave system
#define ZW_LR_MAX_NODEMASK_LENGTH ((ZW_LR_MAX_NODE_ID / 8) + 1)
#define ZW_MAX_NODEMASK_LENGTH    (ZW_MAX_NODES / 8)
#define ZW_LR_ONLY_MAX_NODEMASK_LENGTH \
  ((ZW_LR_MAX_NODE_ID - ZW_LR_MIN_NODE_ID) / 8)

#define ZW_LR_NODEMASK_OFFSET     ((ZW_LR_MIN_NODE_ID) / 8)
#define NODEMASK_GET_LR(nodemask) ((nodemask) + ZW_LR_NODEMASK_OFFSET)

// Maximum Z-Wave Endpoint ID
#define ZWAVE_MAXIMUM_ENDPOINT_ID 127

// Tell if a NodeID is a broadcast NodeID.
#define IS_BROADCAST_NODE_ID(node_id)   \
  ((node_id == ZWAVE_BROADCAST_NODE_ID) \
   || (node_id == ZWAVE_LR_BROADCAST_NODE_ID))

/// Node mask
typedef uint8_t zwave_nodemask_t[ZW_LR_MAX_NODEMASK_LENGTH];

/// Z-Wave NodeID type
typedef uint16_t zwave_node_id_t;

/// Z-Wave HomeID type
typedef uint32_t zwave_home_id_t;

// Verifies if the NodeID is in the Z-Wave NodeID range
#define IS_ZWAVE_NODE_ID(node_id) \
  (((node_id) > 0) && ((node_id) <= ZW_MAX_NODES))

// Verifies if the NodeID is in the extended NodeID range
#define IS_ZWAVE_EXTENDED_NODE_ID(node_id) \
  (((node_id) >= ZW_LR_MIN_NODE_ID) && ((node_id) <= ZW_LR_MAX_NODE_ID))

// Verifies if a NodeID is valid.
#define IS_ZWAVE_NODE_ID_VALID(node_id) \
  ((IS_ZWAVE_NODE_ID(node_id)) || (IS_ZWAVE_EXTENDED_NODE_ID(node_id)))

//// Macro to determine if a NodeID is part of a nodemask
#define ZW_IS_NODE_IN_MASK(node_id, nodemask)                             \
  (!IS_ZWAVE_NODE_ID_VALID(node_id)                                       \
     ? 0                                                                  \
     : (IS_ZWAVE_NODE_ID(node_id)                                         \
          ? ((nodemask[(node_id - 1) >> 3] >> ((node_id - 1) & 7)) & 0x1) \
          : ((nodemask[(node_id) >> 3] >> ((node_id)&7)) & 0x1)))

#define ZW_ADD_NODE_TO_MASK(node_id, nodemask)                            \
  (!IS_ZWAVE_NODE_ID_VALID(node_id)                                       \
     ? 0                                                                  \
     : (IS_ZWAVE_NODE_ID(node_id)                                         \
          ? ((nodemask[(node_id - 1) / 8]) |= (1 << ((node_id - 1) % 8))) \
          : ((nodemask[(node_id) / 8]) |= (1 << ((node_id) % 8)))))

#define ZW_REMOVE_NODE_FROM_MASK(node_id, nodemask)  \
  (!IS_ZWAVE_NODE_ID_VALID(node_id)                  \
     ? 0                                             \
     : (IS_ZWAVE_NODE_ID(node_id)                    \
          ? ((nodemask[(node_id - 1) / 8])           \
             &= (0xFF - (1 << ((node_id - 1) % 8)))) \
          : ((nodemask[(node_id) / 8]) &= (0xFF - (1 << ((node_id) % 8))))))

/// Z-Wave Endpoint ID type
typedef uint8_t zwave_endpoint_id_t;

/// Node type structure, used in Node Information Frames
typedef struct {
  uint8_t
    basic;  ///< Basic Device Type, is it a Controller, Controller_Static, End_Node or a End_Node_Routing Device Type
  uint8_t generic;   ///< Generic Device Type
  uint8_t specific;  ///< Specific Device Type
} node_type_t;

// Node Information Frame masks for the listening_protocol byte
#define ZWAVE_NODE_INFO_LISTENING_PROTOCOL_LISTENING_MASK        0x80
#define ZWAVE_NODE_INFO_LISTENING_PROTOCOL_ROUTING_MASK          0x40
#define ZWAVE_NODE_INFO_LISTENING_PROTOCOL_MAX_SPEED_MASK        0x38
#define ZWAVE_NODE_INFO_LISTENING_PROTOCOL_PROTOCOL_VERSION_MASK 0x07

// Node Information Frame masks for the optional_protocol byte
#define ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_OPTIONAL_FUNCTIONALITY_MASK 0x80
#define ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_SENSOR_1000MS_MASK          0x40
#define ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_SENSOR_250MS_MASK           0x20
#define ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_BEAM_CAPABILITY_MASK        0x10
#define ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_END_NODE_MASK               0x08
#define ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_SPECIFIC_DEVICE_MASK        0x04
#define ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_CONTROLLER_MASK             0x02
#define ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_SECURITY_MASK               0x01

#endif  //ZWAVE_NODE_ID_DEFINITIONS_H
/** @} end zwave_node_id_definitions */
