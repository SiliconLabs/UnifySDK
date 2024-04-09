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
 * @defgroup zpc_attribute_types List of defined attributes types
 * @ingroup zpc_attribute_store
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

#include "uic_attribute_definitions.h"
#include "unify_dotdot_defined_attribute_types.h"

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
///< This represents a timestamp (in seconds!) of the last time when we received and/or
///successfully transmitted a frame towards a node. storage type of the
///timestamp is clock_time_t
DEFINE_ATTRIBUTE(ATTRIBUTE_LAST_RECEIVED_FRAME_TIMESTAMP, 0x000F)
///< This is the protocol used for including a node. zwave_protocol_t
DEFINE_ATTRIBUTE(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL, 0x0010)
///< This is a token shown if a node is S2 capable. No value, it's a token
DEFINE_ATTRIBUTE(ATTRIBUTE_NODE_IS_S2_CAPABLE, 0x0011)
///< This is the S2 SPAN data for a node. span_entry_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_S2_SPAN_ENTRY, 0x0012)
///< This is the S2 MPAN table for the ZPC. No type.
DEFINE_ATTRIBUTE(ATTRIBUTE_S2_MPAN_TABLE, 0x0013)
///< This is one entry of the S2 MPAN table for the ZPC. mpan_entry_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_S2_MPAN_ENTRY, 0x0014)
///< This is an attribute that keeps a list of assigned groups. no type
DEFINE_ATTRIBUTE(ATTRIBUTE_MULTICAST_GROUP_LIST, 0x0015)
///< This is an assigned Multicast group. zwave_multicast_group_id_t
DEFINE_ATTRIBUTE(ATTRIBUTE_MULTICAST_GROUP, 0x0016)
///< This is the interval for sending NOPs to failing node clock_time_t
DEFINE_ATTRIBUTE(ATTRIBUTE_ZWAVE_FAILING_NODE_PING_INTERVAL, 0x0017)

///< Used by poll engine to request of for polling of the  attributer
DEFINE_ATTRIBUTE(ATTRIBUTE_POLL_ENGINE_MARK, 0x0018)

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
///< This represent a zwave_key_protocol_combination_t
DEFINE_ATTRIBUTE(ATTRIBUTE_ZWAVE_KEY_AND_PROTOCOL_TO_DISCOVER, 0x0106)

// Generic attributes that can be placed anywhere under an endpoint.
/** This indicates if more reports are expected to "complete" the value of an attribute */
DEFINE_ATTRIBUTE(ATTRIBUTE_REPORTS_TO_FOLLOW, 0x0201)

//This macro definition allows for auto-generated attributes to co-exists with old definitions
#ifndef ATTRIBUTE_AUTO_GENERATED

// Command class specific attributes, should be attached under endpoints
// Suggested range (CC_identifier (4bytes) <<8) | 0x00..0xFF

///////////////////////////////////
// Alarm Sensor Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SENSOR_ALARM))

// Bitmask Attribute contains the bytes that describe the alarm
// types that the device support. uint8_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_BITMASK,
                 ((COMMAND_CLASS_SENSOR_ALARM << 8) | 0x02))

// Alarm type contains the type of the alarm. alarm_sensor_type_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_TYPE,
                 ((COMMAND_CLASS_SENSOR_ALARM << 8) | 0x03))

// Sensor state contains the state of the alarm if alarm is active or not
// alarm_sensor_state_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_STATE,
                 ((COMMAND_CLASS_SENSOR_ALARM << 8) | 0x04))

// Node ID, which detected the alarm condition. alarm_sensor_id_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_NODE_ID,
                 ((COMMAND_CLASS_SENSOR_ALARM << 8) | 0x05))

//Seconds indicates time the remote alarm must be active since last received report.
// alarm_sensor_seconds_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_SECONDS,
                 ((COMMAND_CLASS_SENSOR_ALARM << 8) | 0x06))

///////////////////////////////////
// Barrier Operator Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BARRIER_OPERATOR_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_BARRIER_OPERATOR))

// Bitmask Attribute contains the bytes that describe the supported subsystems
// types that the device support. uint8_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BARRIER_OPERATOR_BITMASK,
                 ((COMMAND_CLASS_BARRIER_OPERATOR << 8) | 0x02))

// The Operator State attribute contain the state of the barrier operator.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BARRIER_OPERATOR_STATE,
                 ((COMMAND_CLASS_BARRIER_OPERATOR << 8) | 0x03))

// Subsystem type attribute is the type of the signaling subsystem
// that the barrier contain.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BARRIER_OPERATOR_SUBSYSTEM_TYPE,
                 ((COMMAND_CLASS_BARRIER_OPERATOR << 8) | 0x04))

// Subsystem state attribute contain the state of the subsystem.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BARRIER_OPERATOR_SUBSYSTEM_STATE,
                 ((COMMAND_CLASS_BARRIER_OPERATOR << 8) | 0x05))

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

//Door Lock Command Class

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES,
                 ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x0d))

///< This represents the Supported Operation type Bit Mask
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_OPERATION_TYPE_BITMASK,
  ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x0e))

///< This represents the Supported Door Lock Mode
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_LOCK_MODE,
                 ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x0f))

///< This represents the Supported Outside Handle Modes Bitmask
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_OUTSIDE_HANDLE_MODES_BITMASK,
  ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x10))

///< This represents the Supported Inside Handle Modes Bitmask
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_INSIDE_HANDLE_MODES_BITMASK,
  ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x11))

///< This represents the Supported door components
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_DOOR_COMPONENT_BITMASK,
  ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x12))

///< This represents the ARS
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_ARS,
                 ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x13))

///< This represents the HRS
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_HRS,
                 ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x14))

///< This represents the TAS
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_TAS,
                 ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x15))

///< This represents the BTBS
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_BTBS,
                 ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x16))

///< This represents the Configuration
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION,
                 ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x17))

///< This represents the Configuration Auto-Relock
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK,
                 ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x18))

///< This represents the Configuration Hold Release Time
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_HOLD_RELEASE_TIME,
  ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x19))

///< This represents the Configuration BTB
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_BTB,
                 ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x1a))

///< This represents the Configuration TA
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_TA,
                 ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x1b))

///< This represents Operation Report
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT,
                 ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x1c))

///< This represents the Operation Report Target Mode
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_TARGET_MODE,
                 ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x1d))

///< This represents the Operation Report Duration
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_DURATION,
                 ((COMMAND_CLASS_DOOR_LOCK << 8) | 0x1e))

///////////////////////////////////
// Binary Switch Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SWITCH_BINARY))

///< This this is an umbrella/parent attribute, containing the state of
/// the binary switch command_status_values_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_STATE,
                 ((COMMAND_CLASS_SWITCH_BINARY << 8) | 0x02))

///< This represents the current state/value. uint32_t (due to mapper)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_VALUE,
                 ((COMMAND_CLASS_SWITCH_BINARY << 8) | 0x03))

///< This represents the duration (to reach the desired value). uint32_t (due to mapper)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_DURATION,
                 ((COMMAND_CLASS_SWITCH_BINARY << 8) | 0x04))

///////////////////////////////////
// Central Scene Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CENTRAL_SCENE))

///< This represents the total number of scenes supported by the node.
///< central_scene_scene_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_NUMBER_OF_SCENES,
                 ((COMMAND_CLASS_CENTRAL_SCENE << 8) | 0x02))

///< This represents the maximum number of key attributes per scene.
///< central_scene_key_attribute_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_MAX_KEY_ATTRIBUTE,
                 ((COMMAND_CLASS_CENTRAL_SCENE << 8) | 0x03))

///< This represents the Slow Refresh setting.
///< central_scene_slow_refresh_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_SLOW_REFRESH,
                 ((COMMAND_CLASS_CENTRAL_SCENE << 8) | 0x04))

///< This represents the active scene.
///< central_scene_scene_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_ACTIVE_SCENE,
                 ((COMMAND_CLASS_CENTRAL_SCENE << 8) | 0x06))

///< This represents the sequence number received for the active scene.
///< central_scene_sequence_number_t type
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_ACTIVE_SCENE_SEQUENCE_NUMBER,
  ((COMMAND_CLASS_CENTRAL_SCENE << 8) | 0x07))

///////////////////////////////////
// Multilevel Switch Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SWITCH_MULTILEVEL))

///< This this is an umbrella/parent attribute, containing the state of
/// the multilevel switch. command_status_values_t type
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
// Color Switch Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SWITCH_COLOR))

///< This is a bitmask of the supported color components IDs. color_component_bitmask_t
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_SUPPORTED_COLOR_COMPONENT_MASK,
  ((COMMAND_CLASS_SWITCH_COLOR << 8) | 0x2))

///< This this is an umbrella/parent attribute, containing the state of
/// the color switch. command_status_values_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_STATE,
                 ((COMMAND_CLASS_SWITCH_COLOR << 8) | 0x03))

///< This represents a Color Component ID, color_component_id_t type.
/// Located under ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_STATE
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_COLOR_COMPONENT_ID,
                 ((COMMAND_CLASS_SWITCH_COLOR << 8) | 0x4))

///< This represents a Color Component ID's value, uint32_t (due to mapper)
/// Located under ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_COLOR_COMPONENT_ID
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
                 ((COMMAND_CLASS_SWITCH_COLOR << 8) | 0x5))

///< This represents a Color Component ID's duration (to get to the desired value)
/// uint32_t (due to mapper)
/// Located under ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_COLOR_COMPONENT_ID
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_DURATION,
                 ((COMMAND_CLASS_SWITCH_COLOR << 8) | 0x6))

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
/** Count for commands trying to set group content. set_commands_count_t */
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_CONTENT_COMMAND_COUNT,
  ((COMMAND_CLASS_ASSOCIATION << 8) | 0x07))

///////////////////////////////////
// Clock Command Class
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CLOCK_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CLOCK))

///////////////////////////////////
// Configuration Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION))

// Next Parameter ID to try to discover. configuration_parameter_id_t type
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_NEXT_SUPPORTED_PARAMETER_ID,
  ((COMMAND_CLASS_CONFIGURATION << 8) | 0x02))

// If the node supports Bulk Commands. configuration_bulk_support_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_BULK_SUPPORT,
                 ((COMMAND_CLASS_CONFIGURATION << 8) | 0x03))

// Configuration parameter ID (parameter number). configuration_parameter_id_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ID,
                 ((COMMAND_CLASS_CONFIGURATION << 8) | 0x04))

// Configuration parameter size. configuration_parameter_size_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_SIZE,
                 ((COMMAND_CLASS_CONFIGURATION << 8) | 0x05))

// Configuration parameter format. configuration_parameter_format_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_FORMAT,
                 ((COMMAND_CLASS_CONFIGURATION << 8) | 0x06))

// Configuration parameter name. String type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_NAME,
                 ((COMMAND_CLASS_CONFIGURATION << 8) | 0x07))

// Configuration parameter information. String type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_INFO,
                 ((COMMAND_CLASS_CONFIGURATION << 8) | 0x08))

// Configuration parameter current value. configuration_parameter_value_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_VALUE,
                 ((COMMAND_CLASS_CONFIGURATION << 8) | 0x09))

// Configuration parameter minimum value. configuration_parameter_value_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_MIN_VALUE,
                 ((COMMAND_CLASS_CONFIGURATION << 8) | 0x0A))

// Configuration parameter maximum value. configuration_parameter_value_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_MAX_VALUE,
                 ((COMMAND_CLASS_CONFIGURATION << 8) | 0x0B))

// Configuration parameter default value. configuration_parameter_value_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_DEFAULT_VALUE,
                 ((COMMAND_CLASS_CONFIGURATION << 8) | 0x0C))

// Configuration parameter advanced flag. configuration_parameter_flag_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ADVANCED,
                 ((COMMAND_CLASS_CONFIGURATION << 8) | 0x0D))

// Configuration parameter read-only flag. configuration_parameter_flag_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_READ_ONLY,
                 ((COMMAND_CLASS_CONFIGURATION << 8) | 0x0E))

// Configuration parameter altering capabilities flag. configuration_parameter_flag_t type
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ALTERING_CAPABILITIES,
  ((COMMAND_CLASS_CONFIGURATION << 8) | 0x0F))

// Default Reset all parameter have been requested. command_status_values_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_DEFAULT_RESET_REQUESTED,
                 ((COMMAND_CLASS_CONFIGURATION << 8) | 0x10))

// Attribute used to store underneath a list of configuration parameters ID
// that we need to discover. This attribute is used by the ZCL cluster server
// it has no value. A set of ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_NEXT_SUPPORTED_PARAMETER_ID
// are stored under this attribute
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETERS_TO_DISCOVER,
                 ((COMMAND_CLASS_CONFIGURATION << 8) | 0x11))

/////////////////////////////////////////////////
// CRC16 Command Class
///< This represents the version of the CRC16 Locally Command Class.
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_CRC16_VERSION,
                 ((COMMAND_CLASS_CRC_16_ENCAP << 8) | 0x01))

/////////////////////////////////////////////////
// Device Reset Locally Command Class
///< This represents the version of the Device Reset Locally Command class.
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_DEVICE_RESET_LOCALLY_VERSION,
                 ((COMMAND_CLASS_DEVICE_RESET_LOCALLY << 8) | 0x01))

/////////////////////////////////////////////////
// Humidity Control Mode Command Class
///< This represents the version of the Humidity Control Mode Command class.
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_VERSION,
                ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_HUMIDITY_CONTROL_MODE))

// Supported mode
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_SUPPORTED_MODES,
                 ((COMMAND_CLASS_HUMIDITY_CONTROL_MODE << 8) | 0x02))

// Current mode
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_CURRENT_MODE,
                 ((COMMAND_CLASS_HUMIDITY_CONTROL_MODE << 8) | 0x03))

/////////////////////////////////////////////////
// Humidity Control Operating State Command Class
///< This represents the version of the Humidity Control Mode Command class.
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_OPERATING_STATE_VERSION,
                ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_HUMIDITY_CONTROL_OPERATING_STATE))

// Current state
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_OPERATING_STATE_CURRENT_STATE,
                 ((COMMAND_CLASS_HUMIDITY_CONTROL_OPERATING_STATE << 8) | 0x02))

/////////////////////////////////////////////////
// Humidity Control Setpoint Command Class
///< This represents the version of the Humidity Control Mode Command class.
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_VERSION,
                ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_SUPPORTED_TYPES,
                 ((COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT << 8) | 0x02))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_TYPE,
                 ((COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT << 8) | 0x03))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_SUPPORTED_SCALE,
                 ((COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT << 8) | 0x04))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_VALUE,
                 ((COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT << 8) | 0x05))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_VALUE_SCALE,
                 ((COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT << 8) | 0x06))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_VALUE_PRECISION,
                 ((COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT << 8) | 0x07))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_MIN_VALUE,
                 ((COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT << 8) | 0x08))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_MIN_VALUE_SCALE,
                 ((COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT << 8) | 0x09))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_MIN_VALUE_PRECISION,
                 ((COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT << 8) | 0x0A))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_MAX_VALUE,
                 ((COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT << 8) | 0x0B))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_MAX_VALUE_SCALE,
                 ((COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT << 8) | 0x0C))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_MAX_VALUE_PRECISION,
                 ((COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT << 8) | 0x0D))

/////////////////////////////////////////////////
// Inclusion Controller Command Class
///< This represents the version of the Inclusion Controller Command class.
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_INCLUSION_CONTROLLER_VERSION,
                 ((COMMAND_CLASS_INCLUSION_CONTROLLER << 8) | 0x01))

/////////////////////////////////////////////////
// Meter Command Class
///< This represents the version of the Meter Command class.
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_METER_VERSION,
                 ((COMMAND_CLASS_METER << 8) | 0x01))

/// Indicates if the nodes supports Meter Reset
/// meter_reset_supported_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_METER_RESET_SUPPORTED,
                 ((COMMAND_CLASS_METER << 8) | 0x02))

/// This represents the supported scales for a Meter type.
/// byte array
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_METER_SUPPORTED_SCALES,
                 ((COMMAND_CLASS_METER << 8) | 0x03))

/// This represents the supported rate types for a Meter type.
/// meter_supported_rate_types_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_METER_SUPPORTED_RATE_TYPES,
                 ((COMMAND_CLASS_METER << 8) | 0x04))

/// This represents a Meter type.
/// meter_type_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_METER_TYPE,
                 ((COMMAND_CLASS_METER << 8) | 0x05))

/// This represents a Meter scale.
/// meter_scale_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_METER_SCALE,
                 ((COMMAND_CLASS_METER << 8) | 0x06))

/// This represents a Meter Rate type.
/// meter_rate_type_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_METER_RATE_TYPE,
                 ((COMMAND_CLASS_METER << 8) | 0x07))

/// This represents a Meter Value.
/// meter_value_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_METER_VALUE,
                 ((COMMAND_CLASS_METER << 8) | 0x08))

/// This represents a Meter Value's precision.
/// meter_precision_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_METER_PRECISION,
                 ((COMMAND_CLASS_METER << 8) | 0x09))

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

/** If set will send the endpoint Generic/Specific device class in ZW_MULTI_CHANNEL_END_POINT_FIND */
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_FLAG_SEND_TARGETED_DEVICE_CLASS,
                 ((COMMAND_CLASS_MULTI_CHANNEL_V3 << 8) | 0x07))

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
// Supervision Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SUPERVISION_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SUPERVISION))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SUPERVISION_ENABLED,
                 ((COMMAND_CLASS_SUPERVISION << 8) | 0x02))

/////////////////////////////////////////////////
// Thermostat Mode Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_THERMOSTAT_MODE))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE,
                 ((COMMAND_CLASS_THERMOSTAT_MODE << 8) | 0x02))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES,
                 ((COMMAND_CLASS_THERMOSTAT_MODE << 8) | 0x03))

/////////////////////////////////////////////////
// Thermostat Setpoint Command Class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_THERMOSTAT_SETPOINT))

DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_SUPPORTED_SETPOINT_TYPES,
  ((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | 0x02))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPE,
                 ((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | 0x03))

// WARNING : applications/zpc/components/zwave_command_classes/src/zwave_command_class_thermostat_setpoint.cpp
// use the attributes ID to determine value scale and precision. (+1 for scale and +2 for precision)
// Be careful if you change the ID
#define SETPOINT_SCALE_ATTRIBUTE_ID_OFFSET     1
#define SETPOINT_PRECISION_ATTRIBUTE_ID_OFFSET 2

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE,
                 ((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | 0x04))

// 0x05
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE_SCALE,
                 ((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | (0x04 + SETPOINT_SCALE_ATTRIBUTE_ID_OFFSET)))
// 0x06
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE_PRECISION,
                 ((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | (0x04 + SETPOINT_PRECISION_ATTRIBUTE_ID_OFFSET)))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_MIN_VALUE,
                 ((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | 0x07))
// 0x08
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_MIN_VALUE_SCALE,
                 ((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | (0x07 + SETPOINT_SCALE_ATTRIBUTE_ID_OFFSET)))
// 0x09
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_MIN_VALUE_PRECISION,
                 ((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | (0x07 + SETPOINT_PRECISION_ATTRIBUTE_ID_OFFSET)))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_MAX_VALUE,
                 ((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | 0x0A))
// 0x0B
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_MAX_VALUE_SCALE,
                 ((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | (0x0A + SETPOINT_SCALE_ATTRIBUTE_ID_OFFSET)))
// 0x0C            
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_MAX_VALUE_PRECISION,
                 ((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | (0x0A + SETPOINT_PRECISION_ATTRIBUTE_ID_OFFSET)))


DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_USE_B_INTERPRETATION,
                 ((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | 0x0D))

/////////////////////////////////////////////////
// Thermostat Fan Mode Command Class
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_THERMOSTAT_FAN_MODE))
/// Current Fan mode
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_CURRENT_MODE,
                 ((COMMAND_CLASS_THERMOSTAT_FAN_MODE << 8) | 0x02))
/// Supported Fans modes
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_SUPPORTED_MODES,
                 ((COMMAND_CLASS_THERMOSTAT_FAN_MODE << 8) | 0x03))
/// Off flag (v2+)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_OFF_FLAG,
                 ((COMMAND_CLASS_THERMOSTAT_FAN_MODE << 8) | 0x04))

/////////////////////////////////////////////////
// Thermostat Fan State Command Class
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_STATE_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_THERMOSTAT_FAN_STATE))
/// Current Fan operating state
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_STATE_FAN_OPERATING_STATE,
                 ((COMMAND_CLASS_THERMOSTAT_FAN_STATE << 8) | 0x02))


/////////////////////////////////////////////////
// Thermostat Operating State Command Class
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_THERMOSTAT_OPERATING_STATE))
/// Current State
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_CURRENT_STATE,
                 ((COMMAND_CLASS_THERMOSTAT_OPERATING_STATE << 8) | 0x02))
/// Log supported count (v2)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_SUPPORTED_BITMASK,
                 ((COMMAND_CLASS_THERMOSTAT_OPERATING_STATE << 8) | 0x03))
/// Log supported (v2)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_SUPPORTED,
                 ((COMMAND_CLASS_THERMOSTAT_OPERATING_STATE << 8) | 0x04))
/// Log count (v2)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_BITMASK,
                 ((COMMAND_CLASS_THERMOSTAT_OPERATING_STATE << 8) | 0x05))
/// Log State (v2)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_STATE,
                 ((COMMAND_CLASS_THERMOSTAT_OPERATING_STATE << 8) | 0x06))
/// Log Usage Today (hours) (v2)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_USAGE_TODAY_HOURS,
                 ((COMMAND_CLASS_THERMOSTAT_OPERATING_STATE << 8) | 0x07))
/// Log Usage Today (min) (v2)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_USAGE_TODAY_MIN,
                 ((COMMAND_CLASS_THERMOSTAT_OPERATING_STATE << 8) | 0x08))
/// Log Usage Yesterday (hours) (v2)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_USAGE_YESTERDAY_HOURS,
                 ((COMMAND_CLASS_THERMOSTAT_OPERATING_STATE << 8) | 0x09))
/// Log Usage Yesterday (min) (v2)
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_USAGE_YESTERDAY_MIN,
                 ((COMMAND_CLASS_THERMOSTAT_OPERATING_STATE << 8) | 0x0A))

/////////////////////////////////////////////////
// Wakeup command class
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_WAKE_UP_V2))

///> Wake Up Configuration umbrella attribute. command_status_values_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_SETTING,
                 ((COMMAND_CLASS_WAKE_UP_V2 << 8) | 0x09))

///> Configured Wake Up Interval for a node. wake_up_interval_t type.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL,
                 ((COMMAND_CLASS_WAKE_UP_V2 << 8) | 0x02))
///> Configured Wake Up NodeID for a node. zwave_node_id_t type.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_NODE_ID,
                 ((COMMAND_CLASS_WAKE_UP_V2 << 8) | 0x03))

///> Configured Wake Up NodeID for a node. command_status_values_t type.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_CAPABILITIES,
                 ((COMMAND_CLASS_WAKE_UP_V2 << 8) | 0x0A))

///> Minimum Wake Up Interval for a node. wake_up_interval_t type.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_MINIMUM_INTERVAL,
                 ((COMMAND_CLASS_WAKE_UP_V2 << 8) | 0x04))
///> Maximum Wake Up Interval for a node. wake_up_interval_t type.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_MAXIMUM_INTERVAL,
                 ((COMMAND_CLASS_WAKE_UP_V2 << 8) | 0x05))
///> Default Wake Up Interval for a node. wake_up_interval_t type.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_DEFAULT_INTERVAL,
                 ((COMMAND_CLASS_WAKE_UP_V2 << 8) | 0x06))
///> Step size in between allowed Wake Up Intervals for a node. wake_up_interval_t type.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL_STEP,
                 ((COMMAND_CLASS_WAKE_UP_V2 << 8) | 0x07))
///> Bitmask of additional capabilties in the Wake Up Commadn Class. wake_up_bitmask_t type.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_CAPABILITIES_BITMASK,
                 ((COMMAND_CLASS_WAKE_UP_V2 << 8) | 0x08))

/////////////////////////////////////////////////
// Scene Activation Command Class
/// < This represent the version of the  Scene Activation Command Class
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SCENE_ACTIVATION_VERSION,
                 ((COMMAND_CLASS_SCENE_ACTIVATION << 8) | 0x01))

/// < Umbrella attribute under which we save the active scene and
/// Scene configuration (serves both for Scene Activation and Scene Actuator
/// Configuration Command Classes)
/// Empty storage type.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SCENE_LOCAL_SCENES,
                 ((COMMAND_CLASS_SCENE_ACTIVATION << 8) | 0x02))

/// < Current scene active at the node. local_scene_id_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SCENE_ACTIVATION_ACTIVE_SCENE,
                 ((COMMAND_CLASS_SCENE_ACTIVATION << 8) | 0x03))

/// < Dimming Duration setting for a scene. local_scene_dimming_duration_t
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_SCENE_ACTIVATION_SETTING_DIMMING_DURATION,
  ((COMMAND_CLASS_SCENE_ACTIVATION << 8) | 0x04))

/////////////////////////////////////////////////
// Scene Actuator Configuration Command Class
/// < This represent the version of the  Scene Actuator Configuration Command Class
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SCENE_ACTUATOR_CONF_VERSION,
                 ((COMMAND_CLASS_SCENE_ACTUATOR_CONF << 8) | 0x01))

/// < Umbrella attribute under which we save all the scenes settings
/// Empty storage type.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SCENE_ACTUATOR_CONFIGURATION,
                 ((COMMAND_CLASS_SCENE_ACTUATOR_CONF << 8) | 0x02))

/// < Scene ID under which the Actuator configuration will be saved.
/// local_scene_id_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SCENE_ACTUATOR_ID,
                 ((COMMAND_CLASS_SCENE_ACTUATOR_CONF << 8) | 0x03))

/// < Dimming Duration setting for a scene. local_scene_dimming_duration_t
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_SCENE_ACTUATOR_SETTING_DIMMING_DURATION,
  ((COMMAND_CLASS_SCENE_ACTUATOR_CONF << 8) | 0x04))

/// < Level setting for a scene. local_scene_level_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SCENE_ACTUATOR_SETTING_LEVEL,
                 ((COMMAND_CLASS_SCENE_ACTUATOR_CONF << 8) | 0x05))

/// < Override setting for a scene. local_scene_override_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SCENE_ACTUATOR_SETTING_OVERRIDE,
                 ((COMMAND_CLASS_SCENE_ACTUATOR_CONF << 8) | 0x06))

/////////////////////////////////////////////////
// Scene Controller Configuration Command Class
/// < This represent the version of the  Scene Controller Configuration Command Class
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SCENE_CONTROLLER_CONF_VERSION,
                 ((COMMAND_CLASS_SCENE_CONTROLLER_CONF << 8) | 0x01))

/// Umbrella attribute indicating how the scenes commands are configured
/// for a given Association group ID.
/// command_status_values_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SCENE_CONTROLLER_CONFIGURATION,
                 ((COMMAND_CLASS_SCENE_CONTROLLER_CONF << 8) | 0x02))

/// < Scene ID under which the Controller configuration will be saved.
/// local_scene_id_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SCENE_CONTROLLER_SETTING_ID,
                 ((COMMAND_CLASS_SCENE_CONTROLLER_CONF << 8) | 0x03))

/// < Dimming Duration setting for a scene. local_scene_dimming_duration_t
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_SCENE_CONTROLLER_SETTING_DIMMING_DURATION,
  ((COMMAND_CLASS_SCENE_CONTROLLER_CONF << 8) | 0x04))

/////////////////////////////////////////////////
// Security 0 (S0) Command Class
/// < This represent the version of the Security 0 Command Class
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SECURITY_0_VERSION,
                 ((COMMAND_CLASS_SECURITY << 8) | 0x01))

/////////////////////////////////////////////////
// Security 2 (S2) Command Class
/// < This represent the version of the Security 2 Command Class
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SECURITY_2_VERSION,
                 ((COMMAND_CLASS_SECURITY_2 << 8) | 0x01))

/////////////////////////////////////////////////
// Transport Service Command Class
///< This represents the version of the Transport Service Command class. zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_TRANSPORT_SERVICE_VERSION,
                 ((COMMAND_CLASS_TRANSPORT_SERVICE << 8) | 0x01))

/////////////////////////////////////////////////
// User Code Command Class
///< This represents the version of the User Code Command class.
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CODE_VERSION,
                 ((COMMAND_CLASS_USER_CODE << 8) | 0x01))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CODE_DATA,
                 ((COMMAND_CLASS_USER_CODE << 8) | 0x02))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CODE_NUMBER_OF_USERS,
                 ((COMMAND_CLASS_USER_CODE << 8) | 0x03))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CODE_DELETE_ALL_REQUESTED,
                 ((COMMAND_CLASS_USER_CODE << 8) | 0x04))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CODE_USER_ID,
                 ((COMMAND_CLASS_USER_CODE << 8) | 0x05))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CODE_CODE,
                 ((COMMAND_CLASS_USER_CODE << 8) | 0x06))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CODE_USER_ID_STATUS,
                 ((COMMAND_CLASS_USER_CODE << 8) | 0x07))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CODE_ADMIN_CODE,
                 ((COMMAND_CLASS_USER_CODE << 8) | 0x08))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CODE_CAPABILITIES,
                 ((COMMAND_CLASS_USER_CODE << 8) | 0x09))

// Supported flags are saved sequentially in this attribute, from the
// order of finding them in the USER_CODE_CAPABILITIES_REPORT
// So it'll be bit 0 : AC Support / bit 1 : ACD Support / bit 2:  Res
//             bit 3 : UCC Support / bit 4 : MUCR Support / bit 5: MUCS Support
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CODE_SUPPORTED_FLAGS,
                 ((COMMAND_CLASS_USER_CODE << 8) | 0x0A))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CODE_SUPPORTED_USER_ID_STATUS,
                 ((COMMAND_CLASS_USER_CODE << 8) | 0x0B))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CODE_SUPPORTED_KEYPAD_MODES,
                 ((COMMAND_CLASS_USER_CODE << 8) | 0x0C))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CODE_SUPPORTED_KEYS,
                 ((COMMAND_CLASS_USER_CODE << 8) | 0x0D))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CODE_CHECKSUM,
                 ((COMMAND_CLASS_USER_CODE << 8) | 0x0E))

DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CODE_KEYPAD_MODE,
                 ((COMMAND_CLASS_USER_CODE << 8) | 0x0F))

/////////////////////////////////////////////////
// Powerlevel Command Class
///< This represents the version of the Powerlevel Command class.
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_POWERLEVEL_VERSION,
                 ((COMMAND_CLASS_POWERLEVEL << 8) | 0x01))

/////////////////////////////////////////////////
// All Switch Command Class
///< This represents the version of the All Switch Command class.
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_ALL_SWITCH_VERSION,
                 ((COMMAND_CLASS_SWITCH_ALL << 8) | 0x01))

/////////////////////////////////////////////////
// Sound Switch Command Class
///< This represents the version of the Sound Switch Command class.
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SOUND_SWITCH))
// Configured volume for the Sound Switch
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_VOLUME,
                 ((COMMAND_CLASS_SOUND_SWITCH << 8) | 0x02))
// Configured tone for the Sound Switch
DEFINE_ATTRIBUTE(
  ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_TONE_IDENTIFIER,
  ((COMMAND_CLASS_SOUND_SWITCH << 8) | 0x03))
// Number of tones supported by the receiving node
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONES_NUMBER,
                 ((COMMAND_CLASS_SOUND_SWITCH << 8) | 0x04))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_INFO_IDENTIFIER,
                 ((COMMAND_CLASS_SOUND_SWITCH << 8) | 0x05))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_INFO_DURATION_SECONDS,
                 ((COMMAND_CLASS_SOUND_SWITCH << 8) | 0x06))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_INFO_NAME,
                 ((COMMAND_CLASS_SOUND_SWITCH << 8) | 0x07))
// Command is used to instruct a supporting node to play (or stop playing) a tone.
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_PLAY,
                 ((COMMAND_CLASS_SOUND_SWITCH << 8) | 0x08))


/////////////////////////////////////////////////
// User Credential Command Class
///< This represents the version of the User CredentialCommand class.
/// zwave_cc_version_t
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_VERSION,
                 ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_USER_CREDENTIAL))


/// > User Capabilities
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_NUMBER_OF_USERS,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x02))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_SUPPORTED_CREDENTIAL_RULES,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x03))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_MAX_USERNAME_LENGTH,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x04))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_SUPPORT_USER_SCHEDULE,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x05))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_SUPPORT_ALL_USERS_CHECKSUM,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x06))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_SUPPORT_USER_CHECKSUM,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x07))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_SUPPORTED_USER_TYPES,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x08))

/// > Credential Capabilities
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_SUPPORT_CREDENTIAL_CHECKSUM,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x09))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_SUPPORTED_CREDENTIAL_TYPE,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x0A))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_CREDENTIAL_LEARN_SUPPORT,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x0B))                
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_CREDENTIAL_SUPPORTED_SLOT_COUNT,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x0C))        
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_CREDENTIAL_MIN_LENGTH,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x0D))        
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_CREDENTIAL_MAX_LENGTH,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x0E))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x0F))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_CREDENTIAL_LEARN_NUMBER_OF_STEPS,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x10))

/// > All Users Checksum
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_ALL_USERS_CHECKSUM,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x11))

/// > Users
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_USER_UNIQUE_ID,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x12))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_USER_MODIFIER_TYPE,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x13))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_USER_MODIFIER_NODE_ID,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x14))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_USER_TYPE,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x15))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_USER_ACTIVE_STATE,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x16))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_USER_NAME,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x17))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_USER_NAME_ENCODING,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x18))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_CREDENTIAL_RULE,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x19))

/// > Credentials
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_CREDENTIAL_TYPE,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x1A))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_CREDENTIAL_DATA_LENGTH,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x1B))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_CREDENTIAL_DATA,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x1C))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_CREDENTIAL_SLOT,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x1D))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_CREDENTIAL_READ_BACK,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x1E))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_CREDENTIAL_MODIFIER_TYPE,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x1F))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_CREDENTIAL_MODIFIER_NODE_ID,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x20))


// Specific to Credential SET                 
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_CREDENTIAL_OPERATION_TYPE,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x21))

// Specific to USER SET                 
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_USER_OPERATION_TYPE,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x22))
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_USER_EXPIRING_TIMEOUT_MINUTES,
                 ((COMMAND_CLASS_USER_CREDENTIAL << 8) | 0x23))

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
///> uint32_t type
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_OFFSET,
                 ((COMMAND_CLASS_FIRMWARE_UPDATE_MD << 8) | 0x0E))
///> This is the total size (in bytes) of the firmware to transfer.
///> uint32_t type
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

///> The indicator attributes are defined in the auto-genrated code
///> This attribute a meta attribute that is used to calculate the indicator timeout
DEFINE_ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_INDICATOR_TIMEOUT, 0x87F0)

// ZigBee specific attributes
// Suggested range (ZigBee Cluster ID (2 bytes) <<16) | Zigbee attribute ID (2 bytes)

#endif  //ATTRIBUTE_AUTO_GENERATED
#endif  //ATTRIBUTE_STORE_TYPES_H
        /** @} end zpc_attribute_types */
