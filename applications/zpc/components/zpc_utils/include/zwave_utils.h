/**
 * @file zwave_utils.h
 * @brief Various Z-Wave related utilities.
 *
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 */

#ifndef ZWAVE_UTILS_H
#define ZWAVE_UTILS_H

#include <stdint.h>
#include <stdbool.h>

#include "sl_status.h"

/**
 * @ingroup zpc_utils
 * @{
 */

/**
 * @addtogroup zwave_utils Z-Wave Utilities
 * @{
 */
/// Max number of nodes in a Z-wave system
#define ZW_LR_MAX_NODE_ID 4000

#define ZW_LR_MIN_NODE_ID 256

/// Lowest valid node id in a Z-Wave network
#define ZW_MIN_NODE_ID 1

#define ZW_MAX_NODES 232

/// Broadcast destination address in a Z-Wave Network.
#define ZWAVE_BROADCAST_NODE_ID 255

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

/// Node type structure, used in Node Information Frames
typedef struct {
  uint8_t
    basic;  ///< Basic Device Type, is it a Controller, Controller_Static, End_Node or a End_Node_Routing Device Type
  uint8_t generic;   ///< Generic Device Type
  uint8_t specific;  ///< Specific Device Type
} node_type_t;

/**
 * @brief Z-Wave Operating Modes (AL, FL or NL).
 * Refer to @ref zwave_nwk_spec for details.
 */
typedef enum zwave_operating_mode {
  /// Node is Always Listening (AL)
  ///
  OPERATING_MODE_AL,
  /// Node is Frequently Listening (FL)
  ///
  OPERATING_MODE_FL,
  /// Node is Non-Listening (NL)
  ///
  OPERATING_MODE_NL,
  /// We do not know the Operating mode of the node.
  ///
  OPERATING_MODE_UNKNOWN
} zwave_operating_mode_t;

/**
 * @brief Z-Wave Inclusion Protocol (Z-Wave / Z-Wave Long Range).
 */
typedef enum zwave_protocol {
  /// Node is operating on the Z-Wave PHY/MAC/NWK
  ///
  PROTOCOL_ZWAVE,
  /// Node is operating on the Z-Wave Long Range PHY/MAC/NWK
  ///
  PROTOCOL_ZWAVE_LONG_RANGE,
  /// We do not know the protocol of that node.
  ///
  PROTOCOL_UNKNOWN
} zwave_protocol_t;

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
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns the operating mode of a node
 *
 * Refer to @ref zwave_nwk_spec for Operating Modes.
 * It can be Always listening (AL), Frequently listening (FL) or
 * Non-Listening (NL).
 *
 * It will search for the node in the Attribute store and deduct its operating
 * mode from the ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING
 * and ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL
 * attributes stored under the NodeID.
 *
 * @param node_id The NodeID for which the operating mode is requested
 * @returns zwave_operating_mode_t value.
 */
zwave_operating_mode_t get_operating_mode(zwave_node_id_t node_id);

/**
 * @brief Returns the protocol that the node is running in this network
 *
 * This function will return the protocol that has been used for including
 * a node in our network. After inclusion, nodes run only 1 protocol.
 *
 * It will search for the node in the Attribute store and deduct its protocol
 * from the ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL
 * attribute stored under the NodeID.
 *
 * @param node_id The NodeID for which the operating protocol is requested
 * @returns zwave_protocol_t value.
 */
zwave_protocol_t get_protocol(zwave_node_id_t node_id);

/**
 * @brief Returns the name of a protocol value
 *
 * @param protocol zwave_protocol_t value to print out
 *
 * @returns A string pointer to the name of the Z-Wave Protocol.
 */
const char *zwave_get_protocol_name(zwave_protocol_t protocol);

/**
 * @brief Stores a protocol value for a NodeID in the attribute store.
 *
 * @param node_id                 The NodeID for which the operating protocol
 *                                must be saved in the attribute store
 * @param inclusion_protocol      zwave_protocol_t value to save
 * @returns SL_STATUS_OK on success, any other code if not successful.
 */
sl_status_t store_protocol(zwave_node_id_t node_id,
                           zwave_protocol_t inclusion_protocol);
#ifdef __cplusplus
}
#endif

/** @} (end addtogroup zwave_utils) */
/** @} (end addtogroup zpc_utils) */

#endif /* ZWAVE_UTILS_H */
