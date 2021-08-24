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

/**
 * @defgroup attribute_types List of defined attributes types
 * @ingroup attribute_store
 * @brief List of defines Attribute Types for the attribute store.
 *
 * <b>Attribute Type Ranges:</b>
 * Range From | Range To   | Type
 * ---------- | ---------- | ----------------------------------
 * 0x00000000 | 0x00000001 | Attribute Store
 * 0x00000002 | 0x0000FFFF | Z-Wave Command Class Attributes
 * 0x00010000 | 0xFFFFFFFF | ZigBee Cluster Command Attributes
 *
 * @{
 */

#ifndef ATTRIBUTE_STORE_TYPES_H
#define ATTRIBUTE_STORE_TYPES_H

#if !defined(ATTRIBUTE_TYPE_MAP) && !defined(ATTRIBUTE_VALUE_MAP)
#include "ZW_classcmd.h"
#include "attribute_store.h"
#endif

///< Set the proper name for the Root,
DEFINE_ATTRIBUTE(ATTRIBUTE_STORE_ROOT, 0x0001)

/**
 * @brief Type of a node in the attribute store.
 */
#define ZWAVE_CC_VERSION_ATTRIBUTE(cc) (attribute_store_type_t)((cc << 8) | 0x1)
#define COUNT_OF(x) \
  ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

// Generic Node attributes, should be attached under NodeID nodes and not endpoints.
// Suggested range 0x01..0xFF

///< This represents a Home ID. zwave_home_id_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_HOME_ID, 0x0002)
///< This represents a Node ID. zwave_node_id_t type.
DEFINE_ATTRIBUTE(ATTRIBUTE_NODE_ID, 0x0003)
///< This represents an endpoint. zwave_endpoint_id_t type.
DEFINE_ATTRIBUTE(ATTRIBUTE_ENDPOINT_ID, 0x0004)
///< This represents a list of granted security keys to a node. zwave_keyset_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_GRANTED_SECURITY_KEYS, 0x0005)
///< This represents the outcome of the S2 Bootstrapping process. zwave_kex_fail_type_t
DEFINE_ATTRIBUTE(ATTRIBUTE_KEX_FAIL_TYPE, 0x0006)
///< This represents the S2 DSK to a node. zwave_dsk_t
DEFINE_ATTRIBUTE(ATTRIBUTE_S2_DSK, 0x0007)
///< This represent the listening protocol byte from the NIF. uint8_t
DEFINE_ATTRIBUTE(ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING, 0x0008)
///< This represent the optional protocol byte from the NIF.  uint8_t
DEFINE_ATTRIBUTE(ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL, 0x0009)
///< This represents the Z-Wave Role Type of a node. zwave_role_type_t
DEFINE_ATTRIBUTE(ATTRIBUTE_ZWAVE_ROLE_TYPE, 0x000A)
///< This represents the name assigned to a node. This is a string.
DEFINE_ATTRIBUTE(ATTRIBUTE_NODE_NAME, 0x000B)
///< This represents the location assigned to a node. This is a string
DEFINE_ATTRIBUTE(ATTRIBUTE_NODE_LOCATION, 0x000C)
///< This represents the Network Status of a node. node_state_topic_state_t
DEFINE_ATTRIBUTE(ATTRIBUTE_NETWORK_STATUS, 0x000D)
///< This represents the last timestamp when we receive a frame from a node.
/// For sleeping nodes, this represents the last timestamp when node is awake
DEFINE_ATTRIBUTE(ATTRIBUTE_LAST_RECEIVED_FRAME_TIMESTAMP, 0x000F)
///< This is the protocol used for including a node. zwave_protocol_t
DEFINE_ATTRIBUTE(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL, 0x0010)
///< This is a token shown if a node is S2 capable. No value, it's a token
DEFINE_ATTRIBUTE(ATTRIBUTE_NODE_IS_S2_CAPABLE, 0x0011)

// Generic endpoint attributes, should be attached under endpoints
// Suggested range 0x100..0x1FF (using Z-Wave Protocol CC identifier)
/** This represents the list of supported command classes (NIF) for a node */
DEFINE_ATTRIBUTE(ATTRIBUTE_ZWAVE_NIF, 0x0101)
/** This represents the list of supported command classes only at the highest security level (secure NIF) for a node. */
DEFINE_ATTRIBUTE(ATTRIBUTE_ZWAVE_SECURE_NIF, 0x0102)
/** This represents the Z-Wave Basic device class of a node/endpoint */
DEFINE_ATTRIBUTE(ATTRIBUTE_ZWAVE_BASIC_DEVICE_CLASS, 0x0103)
/** This represents the Z-Wave Generic Device Type of a node/endpoint. */
DEFINE_ATTRIBUTE(ATTRIBUTE_ZWAVE_GENERIC_DEVICE_CLASS, 0x0104)
/** This represents the Z-Wave Specific Device Type of a node/endpoint. */
DEFINE_ATTRIBUTE(ATTRIBUTE_ZWAVE_SPECIFIC_DEVICE_CLASS, 0x0105)

//This macro definition allows for auto-generated attributes to co-exists with old definitions
#ifndef ATTRIBUTE_AUTO_GENERATED

// Command class specific attributes, should be attached under endpoints
// Suggested range (CC_identifier (4bytes) <<8) | 0x00..0xFF

///////////////////////////////////
// Basic Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BASIC_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_BASIC))

///< This represents the current state/value. uint32_t (due to mapper)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BASIC_VALUE,
                 ((COMMAND_CLASS_BASIC << 8) | 0x02))

///< This represents the duration (to reach the desired value). uint32_t (due to mapper)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BASIC_DURATION,
                 ((COMMAND_CLASS_BASIC << 8) | 0x03))

///< This is an attribute that remembers if we asked for Basic Support.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BASIC_PROBE_STATUS,
                 ((COMMAND_CLASS_BASIC << 8) | 0x04))

///////////////////////////////////
// Binary Switch Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SWITCH_BINARY))

///< This this is an umbrella/parent attribute, containing the state of
/// the binary switch
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_STATE,
                 ((COMMAND_CLASS_SWITCH_BINARY << 8) | 0x02))

///< This represents the current state/value. uint32_t (due to mapper)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_VALUE,
                 ((COMMAND_CLASS_SWITCH_BINARY << 8) | 0x03))

///< This represents the duration (to reach the desired value). uint32_t (due to mapper)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_DURATION,
                 ((COMMAND_CLASS_SWITCH_BINARY << 8) | 0x04))

///////////////////////////////////
// Multilevel Switch Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SWITCH_MULTILEVEL))

///< This this is an umbrella/parent attribute, containing the state of
/// the multilevel switch
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_STATE,
                 ((COMMAND_CLASS_SWITCH_MULTILEVEL << 8) | 0x02))

///< This represents the current state/value. uint32_t (due to mapper)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_VALUE,
                 ((COMMAND_CLASS_SWITCH_MULTILEVEL << 8) | 0x03))

///< This represents the duration (to reach the desired value). uint32_t (due to mapper)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_DURATION,
                 ((COMMAND_CLASS_SWITCH_MULTILEVEL << 8) | 0x04))

///< This is a virtual state, representing the OnOff state in ZCL. uint32_t (due to mapper)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_ON_OFF,
                 ((COMMAND_CLASS_SWITCH_MULTILEVEL << 8) | 0x05))

///< This keeps track of the last type of operation we started at the node. multilevel_switch_operation_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_SUBSTATE_UPDATE,
                 ((COMMAND_CLASS_SWITCH_MULTILEVEL << 8) | 0x06))

///< This is a boolean value indicating if we have asked the capabilties of a node. bool
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_CAPABILITIES_REQUESTED,
  ((COMMAND_CLASS_SWITCH_MULTILEVEL << 8) | 0x07))

///////////////////////////////////
// Multilevel Sensor Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SENSOR_MULTILEVEL))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_SENSOR_TYPE,
                 ((COMMAND_CLASS_SENSOR_MULTILEVEL << 8) | 0x02))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_SUPPORTED_SCALES,
                 ((COMMAND_CLASS_SENSOR_MULTILEVEL << 8) | 0x03))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_SCALE,
                 ((COMMAND_CLASS_SENSOR_MULTILEVEL << 8) | 0x04))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_SENSOR_VALUE,
                 ((COMMAND_CLASS_SENSOR_MULTILEVEL << 8) | 0x05))

DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_SUPPORTED_SENSOR_TYPES,
  ((COMMAND_CLASS_SENSOR_MULTILEVEL << 8) | 0x06))

///////////////////////////////////////////////
// Association Group Information Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_INFORMATION_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_ASSOCIATION_GRP_INFO))

///< This is the name of an association group. Null terminated string representation.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_NAME,
                 ((COMMAND_CLASS_ASSOCIATION_GRP_INFO << 8) | 0x02))

///< This is the list of Command Class / Commands sent to a group. Hexadecimal array representation
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
                 ((COMMAND_CLASS_ASSOCIATION_GRP_INFO << 8) | 0x03))

///< This is the profile of a group. uint16_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_PROFILE,
                 ((COMMAND_CLASS_ASSOCIATION_GRP_INFO << 8) | 0x04))

///////////////////////////////////
// Association Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_ASSOCIATION))

/** This represents the number of supported association group by an endpoint. */
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_SUPPORTED_GROUPS,
                 ((COMMAND_CLASS_ASSOCIATION << 8) | 0x02))
/** This represents the ID of an Association group. */
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                 ((COMMAND_CLASS_ASSOCIATION << 8) | 0x03))
/** This represents the list of NodeID associated in a particular association group. */
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_CONTENT,
                 ((COMMAND_CLASS_ASSOCIATION << 8) | 0x04))
/** This represents the number of supported association groups */
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_SUPPORTED_GROUPINGS,
                 ((COMMAND_CLASS_ASSOCIATION << 8) | 0x05))
/** This represents The maximum number of destinations supported by the advertised association group */
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_MAX_NODES_SUPPORTED,
                 ((COMMAND_CLASS_ASSOCIATION << 8) | 0x06))

/////////////////////////////////////////////////
// Multi Channel Association Command Class
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_VERSION,
  ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_MULTI_CHANNEL_V3))

/** Boolean indicating if all endpoints have identical capabilities */
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_IDENTICAL_ENDPOINTS,
                 ((COMMAND_CLASS_MULTI_CHANNEL_V3 << 8) | 0x02))

/** Represents the number of individual endpoints */
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_INDIVIDUAL_ENDPOINTS,
                 ((COMMAND_CLASS_MULTI_CHANNEL_V3 << 8) | 0x03))

/** Represents the number of aggregated endpoints */
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_ENDPOINTS,
                 ((COMMAND_CLASS_MULTI_CHANNEL_V3 << 8) | 0x04))

/** Boolean indicating if we found all individual endpoints */
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_ALL_INDIVIDUAL_ENDPOINTS_FOUND,
  ((COMMAND_CLASS_MULTI_CHANNEL_V3 << 8) | 0x05))

/**  Bitmask representing EndpointIDs that the endpoint aggregates */
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_MEMBERS,
                 ((COMMAND_CLASS_MULTI_CHANNEL_V3 << 8) | 0x06))

/////////////////////////////////////////////////
// Notification Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_NOTIFICATION_V4))

/** This represents the supported notification types by a device that supports Notification CC. */
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_NOTIFICATION_TYPES,
  ((COMMAND_CLASS_NOTIFICATION_V4 << 8) | 0x02))
/**This represents the notification type that is supported by the device. */
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_TYPE,
                 ((COMMAND_CLASS_NOTIFICATION_V4 << 8) | 0x03))
/** This represents the supported notification state or events in a given notification type. */
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_STATES_OR_EVENTS,
  ((COMMAND_CLASS_NOTIFICATION_V4 << 8) | 0x04))
/** This represents the notification state ID of a given notification type. */
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_STATE,
                 ((COMMAND_CLASS_NOTIFICATION_V4 << 8) | 0x05))
/** This represents the notification state or events in a given notification type. */
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT,
                 ((COMMAND_CLASS_NOTIFICATION_V4 << 8) | 0x06))
/** This represents the notification state/event parameters variable in a given notification type. */
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT_PARAMETERS,
                 ((COMMAND_CLASS_NOTIFICATION_V4 << 8) | 0x07))
/** This represents the notification V1 alarm type variable in a given notification type. */
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_V1_ALARM_TYPE,
                 ((COMMAND_CLASS_NOTIFICATION_V4 << 8) | 0x08))
/** This represents the notification V1 alarm level variable in a given notification type. */
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_V1_ALARM_LEVEL,
                 ((COMMAND_CLASS_NOTIFICATION_V4 << 8) | 0x09))

/////////////////////////////////////////////////
// Wakeup command class version 1
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKEUP_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_WAKE_UP_V2))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKEUP_INTERVAL,
                 ((COMMAND_CLASS_WAKE_UP_V2 << 8) | 0x02))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKEUP_NODEID,
                 ((COMMAND_CLASS_WAKE_UP_V2 << 8) | 0x03))

/////////////////////////////////////////////////
// Wakeup command class version 2
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_MINIMUM_INTERVAL,
                 ((COMMAND_CLASS_WAKE_UP_V2 << 8) | 0x04))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_MAXIMUM_INTERVAL,
                 ((COMMAND_CLASS_WAKE_UP_V2 << 8) | 0x05))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_DEFAULT_INTERVAL,
                 ((COMMAND_CLASS_WAKE_UP_V2 << 8) | 0x06))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_INTERVAL_STEP,
                 ((COMMAND_CLASS_WAKE_UP_V2 << 8) | 0x07))

/////////////////////////////////////////////////
// Wakeup command class version 3
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKEUP_V3_WAKE_ON_DEMAND,
                 ((COMMAND_CLASS_WAKE_UP_V2 << 8) | 0x08))

/////////////////////////////////////////////////
// Security 0 (S0) Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SECURITY_0_VERSION,
                 ((COMMAND_CLASS_SECURITY << 8) | 0x01))
/// < This represent the version of the Security 0 Command Class

/////////////////////////////////////////////////
// Security 2 (S2) Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SECURITY_2_VERSION,
                 ((COMMAND_CLASS_SECURITY_2 << 8) | 0x01))
/// < This represent the version of the Security 2 Command Class

/////////////////////////////////////////////////
// Z-Wave Plus Info CC
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_ZWAVEPLUS_INFO_VERSION,
                 ((COMMAND_CLASS_ZWAVEPLUS_INFO << 8) | 0x01))
/// < This represent the version of the Security 2 Command Class

/////////////////////////////////////////////////
// Z-Wave Version CC
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_VERSION_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_VERSION))
///< This is am umbrella attribute, under which the version data is saved
///< It gets set to 1 when a Version Report has been received
DEFINE_ATTRIBUTE(ATTRIBUTE_CC_VERSION_VERSION_REPORT_DATA,
                 ((COMMAND_CLASS_VERSION << 8) | 0x02))
DEFINE_ATTRIBUTE(ATTRIBUTE_CC_VERSION_ZWAVE_LIBRARY_TYPE,
                 ((COMMAND_CLASS_VERSION << 8) | 0x03))
DEFINE_ATTRIBUTE(ATTRIBUTE_CC_VERSION_ZWAVE_PROTOCOL_VERSION,
                 ((COMMAND_CLASS_VERSION << 8) | 0x04))
///< This represents a firmware, starting from 0 (Firmware 0, Firmware 1, etc.)
///< uint32_t type.
DEFINE_ATTRIBUTE(ATTRIBUTE_CC_VERSION_FIRMWARE,
                 ((COMMAND_CLASS_VERSION << 8) | 0x05))
///< This represents the version of a firmware, uint32_t type.
DEFINE_ATTRIBUTE(ATTRIBUTE_CC_VERSION_FIRMWARE_VERSION,
                 ((COMMAND_CLASS_VERSION << 8) | 0x06))
///< This represents if the Z_Wave Software Get command is supporteed
/// bool type.
DEFINE_ATTRIBUTE(ATTRIBUTE_CC_VERSION_ZWAVE_SOFTWARE_GET_SUPPORT,
                 ((COMMAND_CLASS_VERSION << 8) | 0x07))
DEFINE_ATTRIBUTE(ATTRIBUTE_CC_VERSION_SDK_VERSION,
                 ((COMMAND_CLASS_VERSION << 8) | 0x08))
DEFINE_ATTRIBUTE(ATTRIBUTE_CC_VERSION_APPLICATION_FRAMEWORK_API_VERSION,
                 ((COMMAND_CLASS_VERSION << 8) | 0x09))
DEFINE_ATTRIBUTE(ATTRIBUTE_CC_VERSION_APPLICATION_FRAMEWORK_BUILD_NUMBER,
                 ((COMMAND_CLASS_VERSION << 8) | 0x0A))
DEFINE_ATTRIBUTE(ATTRIBUTE_CC_VERSION_HOST_INTERFACE_VERSION,
                 ((COMMAND_CLASS_VERSION << 8) | 0x0B))
DEFINE_ATTRIBUTE(ATTRIBUTE_CC_VERSION_HOST_INTERFACE_BUILD_NUMBER,
                 ((COMMAND_CLASS_VERSION << 8) | 0x0C))
DEFINE_ATTRIBUTE(ATTRIBUTE_CC_VERSION_ZWAVE_PROTOCOL_BUILD_NUMBER,
                 ((COMMAND_CLASS_VERSION << 8) | 0x0D))
DEFINE_ATTRIBUTE(ATTRIBUTE_CC_VERSION_APPLICATION_BUILD_NUMBER,
                 ((COMMAND_CLASS_VERSION << 8) | 0x0E))
///< This is the Hardware version of a product reported in the Version CC.
///< uint32_t type.
DEFINE_ATTRIBUTE(ATTRIBUTE_CC_VERSION_HARDWARE_VERSION,
                 ((COMMAND_CLASS_VERSION << 8) | 0x0F))
///< This attribute is used to mark when we have first detected that
///< The Version CC is supported by an endpoint. Until then, all
///< Command Classes version are assumed to be 1.
/// This is stored under an Endpoint ID node.
DEFINE_ATTRIBUTE(ATTRIBUTE_CC_VERSION_FOUND,
                 ((COMMAND_CLASS_VERSION << 8) | 0x10))

// Firmware Update MD CC
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_FIRMWARE_UPDATE_MD))
///> This is an umbrella attribute for all firmware data. Reported
///> value is set to 1 when a Firmware Meta Data Report Command has been received
///< int32_t type.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x02))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x03))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW_ID,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x04))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW_UPGRADABLE,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x05))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_MANUFACTURER_ID,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x06))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FRAGMENT_SIZE,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x07))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_ACTIVATION,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x08))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_CC,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x09))

///> This represents the state of a firmware transfer.
///> zwave_firmware_transfer_state_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x0A))
///> This represents the firmware target used for a firmware transfer.
///> uint32_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_TARGET,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x0B))
///> This represents the timestamp for when to apply a firmware.
///> (activation set, or start to download)
///> unsigned long type, representing the clock in seconds.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_APPLY_TIMESTAMP,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x0C))
///> This is the filename/path that contains the file from which the firmware
///> data can be read. NULL terminated String type.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_FILENAME,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x0D))
///> This is the highest received offset during the process.
///> size_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_OFFSET,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x0E))
///> This is the total size (in bytes) of the firmware to transfer.
///> size_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_SIZE,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x0F))
///> This is the last status received from the supporting node
///> during a Firmware Transfer.
///> zwave_firmware_transfer_status_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_LAST_STATUS,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x10))
///> This represents the timestamp for aborting/expiring a firmware transfer.
///> clock_time_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_EXPIRY_TIME,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x11))

// ZigBee specific attributes
// Suggested range (ZigBee Cluster ID (4 bytes) <<16) | Zigbee attribute ID (4 bytes)
// OnOff Cluster
DEFINE_ATTRIBUTE(ATTRIBUTE_ZIGBEE_ON_OFF_CLUSTER_ON_OFF,
                 ((0x0006 << 16) | 0x0001))
///< This represents the OnOff attribute in the ZigBee OnOff cluster

// Identify Cluster
DEFINE_ATTRIBUTE(ATTRIBUTE_ZIGBEE_IDENTIFY_CLUSTER_IDENTIFY_TIME,
                 ((0x0003 << 16) | 0x0001))
///< This represents the IdentifyTime attribute in the ZigBee Identify cluster

// OccupancySensing Cluster
DEFINE_ATTRIBUTE(ATTRIBUTE_ZIGBEE_OCCUPANCY_SENSING_CLUSTER_OCCUPANCY,
                 ((0x0406 << 16) | 0x0001))
///< This represents the Occupancy attribute in the ZigBee Occupancy Sensing cluster

// Group Cluster
///< This represents a Group identifier. uint16_t
DEFINE_ATTRIBUTE(DOTDOT_ATTRIBUTE_ID_GROUPS_GROUP_ID, 0x00040001)
///< This represents a Group name. String representation with Null termination.
DEFINE_ATTRIBUTE(DOTDOT_ATTRIBUTE_ID_GROUPS_GROUP_NAME, 0x00040002)
#endif  //ATTRIBUTE_AUTO_GENERATED
#endif  //ATTRIBUTE_STORE_TYPES_H

/** @} end attribute_types */