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
// Includes from this component
#include "attribute_store_defined_attribute_types.h"
#include "zwave_COMMAND_CLASS_MANUFACTURER_SPECIFIC_attribute_id.h"
#include "zwave_COMMAND_CLASS_BATTERY_attribute_id.h"
#include "zwave_COMMAND_CLASS_DOOR_LOCK_attribute_id.h"
#include "zwave_COMMAND_CLASS_INDICATOR_attribute_id.h"
#include "zwave_COMMAND_CLASS_SENSOR_BINARY_attribute_id.h"
#include "zwave_COMMAND_CLASS_ZWAVEPLUS_INFO_attribute_id.h"

// Unify components
#include "attribute_store_type_registration.h"
#include "attribute_store.h"

// defines
#define UNSIGNED_LONG_STORAGE_TYPE                                \
  ((sizeof(unsigned long) == sizeof(uint32_t)) ? U32_STORAGE_TYPE \
                                               : U64_STORAGE_TYPE)

// This main function will serve as a registration bookkeeper for all our
// Z-Wave attributes
sl_status_t zpc_attribute_store_register_known_attribute_types()
{
  // Root node:
  sl_status_t status = SL_STATUS_OK;
  status
    |= attribute_store_register_type(ATTRIBUTE_STORE_ROOT,
                                     "Root node",
                                     ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                                     EMPTY_STORAGE_TYPE);

  // Generic Z-Wave attributes
  status |= attribute_store_register_type(ATTRIBUTE_HOME_ID,
                                          "HomeID",
                                          ATTRIBUTE_STORE_ROOT,
                                          BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_NODE_ID,
                                          "NodeID",
                                          ATTRIBUTE_HOME_ID,
                                          U16_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_ENDPOINT_ID,
                                          "Endpoint ID",
                                          ATTRIBUTE_NODE_ID,
                                          U8_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                          "Granted keys",
                                          ATTRIBUTE_NODE_ID,
                                          U8_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_KEX_FAIL_TYPE,
                                          "KEX Fail type",
                                          ATTRIBUTE_NODE_ID,
                                          U32_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_S2_DSK,
                                          "S2 DSK",
                                          ATTRIBUTE_NODE_ID,
                                          BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                          "NIF: Protocol Listening byte",
                                          ATTRIBUTE_NODE_ID,
                                          U8_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
                                          "NIF: Optional Protocol byte",
                                          ATTRIBUTE_NODE_ID,
                                          U8_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_ZWAVE_ROLE_TYPE,
                                          "Role Type",
                                          ATTRIBUTE_NODE_ID,
                                          ENUM_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_NODE_NAME,
                                          "Node Name",
                                          ATTRIBUTE_NODE_ID,
                                          C_STRING_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_NODE_LOCATION,
                                          "Node Location",
                                          ATTRIBUTE_NODE_ID,
                                          C_STRING_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_NETWORK_STATUS,
                                          "Network Status",
                                          ATTRIBUTE_NODE_ID,
                                          U8_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_LAST_RECEIVED_FRAME_TIMESTAMP,
                                     "Last Rx/Tx timestamp",
                                     ATTRIBUTE_NODE_ID,
                                     UNSIGNED_LONG_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                                          "Inclusion protocol",
                                          ATTRIBUTE_NODE_ID,
                                          ENUM_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_NODE_IS_S2_CAPABLE,
                                          "Node has S2 capability",
                                          ATTRIBUTE_NODE_ID,
                                          EMPTY_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_S2_SPAN_ENTRY,
                                          "S2 SPAN Entry",
                                          ATTRIBUTE_NODE_ID,
                                          BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_S2_MPAN_TABLE,
                                          "S2 MPAN Table",
                                          ATTRIBUTE_NODE_ID,
                                          EMPTY_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_S2_MPAN_ENTRY,
                                          "S2 MPAN Entry",
                                          ATTRIBUTE_S2_MPAN_TABLE,
                                          BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_MULTICAST_GROUP_LIST,
                                          "Multicast Group list",
                                          ATTRIBUTE_NODE_ID,
                                          EMPTY_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_MULTICAST_GROUP,
                                          "Multicast Group",
                                          ATTRIBUTE_MULTICAST_GROUP_LIST,
                                          U8_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_ZWAVE_FAILING_NODE_PING_INTERVAL,
                                     "NOP interval",
                                     ATTRIBUTE_NODE_ID,
                                     UNSIGNED_LONG_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Generic Z-Wave Endpoint ID attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(ATTRIBUTE_ZWAVE_NIF,
                                          "NIF",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_ZWAVE_SECURE_NIF,
                                          "Secure NIF",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_ZWAVE_BASIC_DEVICE_CLASS,
                                          "Basic Device Class",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          U8_STORAGE_TYPE);
  status |= attribute_store_register_type(ATTRIBUTE_ZWAVE_GENERIC_DEVICE_CLASS,
                                          "Generic Device Class",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          U8_STORAGE_TYPE);
  status |= attribute_store_register_type(ATTRIBUTE_ZWAVE_SPECIFIC_DEVICE_CLASS,
                                          "Specific Device Class",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_ZWAVE_KEY_AND_PROTOCOL_TO_DISCOVER,
    "Network Key / Protocol to probe",
    ATTRIBUTE_ENDPOINT_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Generic Z-Wave attributes
  /////////////////////////////////////////////////////////////////////
  status
    |= attribute_store_register_type(ATTRIBUTE_REPORTS_TO_FOLLOW,
                                     "Reports to follow",
                                     ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                                     U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         All Switch Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_ALL_SWITCH_VERSION,
                                     "All Switch Command Class version",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Association Group Information Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_INFORMATION_VERSION,
    "AGI Command Class version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_NAME,
    "Name",
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
    C_STRING_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    "Command list",
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_PROFILE,
    "Profile",
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
    U32_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Association Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_VERSION,
    "Association Command Class version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_SUPPORTED_GROUPINGS,
    "Supported Association Groups",
    ATTRIBUTE_ENDPOINT_ID,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
    "Association Group ID",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_CONTENT,
    "Content",
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_MAX_NODES_SUPPORTED,
    "Max Associations",
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_CONTENT_COMMAND_COUNT,
    "Set Command Count",
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_CONTENT,
    U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //        Alarm Sensor Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_VERSION,
    "Alarm Sensor Command Class version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_BITMASK,
    "Alarm Sensor Bitmask",
    ATTRIBUTE_ENDPOINT_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_TYPE,
                                     "Alarm Sensor Type",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     U8_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_STATE,
                                     "State",
                                     ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_TYPE,
                                     U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_NODE_ID,
    "NodeID",
    ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_TYPE,
    U16_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_SECONDS,
    "Seconds",
    ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_TYPE,
    U16_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Battery Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_BATTERY_VERSION,
                                     "Battery Command Class version",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     U8_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_BATTERY_BATTERY_LEVEL,
                                          "Battery Level",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          U32_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_BATTERY_HEALTH_MAXIMUM_CAPACITY,
                                     "Battery maximum capacity",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     U8_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_BATTERY_HEALTH_SCALE,
                                          "Battery temperature value Scale",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          U8_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_BATTERY_HEALTH_PRECISION,
                                          "Battery temperature value precision",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_BATTERY_HEALTH_BATTERY_TEMPERATURE,
    "Battery temperature value",
    ATTRIBUTE_ENDPOINT_ID,
    I32_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_BATTERY_CHARGING_STATUS,
                                          "Battery charging status ",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          U8_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_BATTERY_REPLACE_RECHARGE,
                                          "Battery replace notification",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //        Basic Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_BASIC_VERSION,
                                          "Basic Command Class version",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          U8_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_BASIC_VALUE,
                                          "Basic Value",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          U32_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_BASIC_DURATION,
                                     "Basic Duration",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     U32_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_BASIC_PROBE_STATUS,
                                     "Basic Probe status",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     ENUM_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //        Binary Sensor Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_SENSOR_BINARY_VERSION,
    "Binary Sensor Command Class version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_SENSOR_BINARY_SENSOR_VALUE,
                                          "Binary Sensor value",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          I32_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //        Binary Switch Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_VERSION,
    "Binary Switch Command Class version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_STATE,
    "Binary Switch State",
    ATTRIBUTE_ENDPOINT_ID,
    ENUM_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_VALUE,
    "Value",
    ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_STATE,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_DURATION,
    "Duration",
    ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_STATE,
    U32_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //        Central Scene Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_VERSION,
    "Central Scene Command Class version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_NUMBER_OF_SCENES,
    "Number of Scenes",
    ATTRIBUTE_ENDPOINT_ID,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_MAX_KEY_ATTRIBUTE,
    "Maximum Key Attributes",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_SLOW_REFRESH,
    "Central Scene Slow Refresh",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_ACTIVE_SCENE,
    "Central Scene Active Scene",
    ATTRIBUTE_ENDPOINT_ID,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_ACTIVE_SCENE_SEQUENCE_NUMBER,
    "Sequence Number",
    ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_ACTIVE_SCENE,
    U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //        Clock Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_CLOCK_VERSION,
                                          "Clock Command Class version",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //        Color Switch Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VERSION,
    "Color Switch Command Class version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_SUPPORTED_COLOR_COMPONENT_MASK,
    "Color Switch Color component bitmask",
    ATTRIBUTE_ENDPOINT_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_STATE,
                                     "Color Switch State",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     ENUM_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_COLOR_COMPONENT_ID,
    "Component ID",
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_STATE,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
    "Value",
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_COLOR_COMPONENT_ID,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_DURATION,
    "Duration",
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_STATE,
    U32_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Configuration Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_VERSION,
    "Configuration Command Class version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_NEXT_SUPPORTED_PARAMETER_ID,
    "Next Configuration Parameter ID",
    ATTRIBUTE_ENDPOINT_ID,
    U16_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_BULK_SUPPORT,
    "Configuration Bulk Support",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ID,
    "Configuration Parameter ID",
    ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
    U16_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_SIZE,
    "Size",
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_FORMAT,
    "Format",
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_NAME,
    "Name",
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ID,
    C_STRING_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_INFO,
    "Info",
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ID,
    C_STRING_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_VALUE,
    "Value",
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ID,
    I64_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_MIN_VALUE,
    "Min Value",
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ID,
    I64_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_MAX_VALUE,
    "Max Value",
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ID,
    I64_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_DEFAULT_VALUE,
    "Default Value",
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ID,
    I64_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ADVANCED,
    "Advanced flag",
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_READ_ONLY,
    "Read-only flag",
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ALTERING_CAPABILITIES,
    "Altering capabilites flag",
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_DEFAULT_RESET_REQUESTED,
    "Configuration Default Reset requested",
    ATTRIBUTE_ENDPOINT_ID,
    ENUM_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETERS_TO_DISCOVER,
    "List of Configuration Parameters ID to discover",
    ATTRIBUTE_ENDPOINT_ID,
    EMPTY_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  // Device Reset Locally Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_CRC16_VERSION,
                                          "CRC16 Version",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  // Inclusion Controller Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_INCLUSION_CONTROLLER_VERSION,
    "Inclusion Controller Version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  // Device Reset Locally Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_DEVICE_RESET_LOCALLY_VERSION,
    "Device Reset Locally Version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Door Lock Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_VERSION,
                                     "Door Lock Command Class version",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  // Indicator Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_INDICATOR_VERSION,
                                     "Indicator Version",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     U8_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_INDICATOR_INDICATOR_ID,
                                          "Indicator ID",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          U8_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_INDICATOR_PROPERTY_ID,
                                          "Property ID",
                                          ATTRIBUTE_INDICATOR_INDICATOR_ID,
                                          U8_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_INDICATOR_INDICATOR_VALUE,
                                          "Value",
                                          ATTRIBUTE_INDICATOR_PROPERTY_ID,
                                          U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_INDICATOR_SUPPORTED_PROPERTY_SUPPORTED_BIT_MASK,
    "Supported Properties bitmask",
    ATTRIBUTE_INDICATOR_INDICATOR_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_INDICATOR_TIMEOUT,
                                     "Indicator timeout",
                                     ATTRIBUTE_INDICATOR_INDICATOR_ID,
                                     U32_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Meter Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_METER_VERSION,
                                          "Meter Version",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_METER_RESET_SUPPORTED,
    "Meter Reset supported",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_METER_SUPPORTED_SCALES,
    "Meter Supported Scales",
    ATTRIBUTE_ENDPOINT_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_METER_SUPPORTED_RATE_TYPES,
    "Meter Supported Rate Types",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  // Meter type can be either under the Endpoint or under the Reset requested
  status |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_METER_TYPE,
                                          "Meter Type",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          U8_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_METER_SCALE,
                                          "Scale",
                                          ATTRIBUTE_COMMAND_CLASS_METER_TYPE,
                                          U16_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_METER_RATE_TYPE,
                                     "Rate Type",
                                     ATTRIBUTE_COMMAND_CLASS_METER_SCALE,
                                     U8_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_METER_VALUE,
                                     "Value",
                                     ATTRIBUTE_COMMAND_CLASS_METER_RATE_TYPE,
                                     I32_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_METER_PRECISION,
                                     "Precision",
                                     ATTRIBUTE_COMMAND_CLASS_METER_RATE_TYPE,
                                     U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //        Multilevel Sensor Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_VERSION,
    "Multilevel Sensor Command Class version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_SENSOR_TYPE,
    "Multilevel Sensor Type",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_SUPPORTED_SCALES,
    "Supported Scales",
    ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_SENSOR_TYPE,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_SCALE,
    "Scale",
    ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_SENSOR_TYPE,
    I32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_SENSOR_VALUE,
    "Value",
    ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_SENSOR_TYPE,
    I32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_SUPPORTED_SENSOR_TYPES,
    "Multilevel Sensor Supported Sensor Types",
    ATTRIBUTE_ENDPOINT_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //        Multilevel Switch Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_VERSION,
    "Multilevel Switch Command Class version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_STATE,
    "Multilevel Switch State",
    ATTRIBUTE_ENDPOINT_ID,
    ENUM_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_VALUE,
    "Value",
    ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_STATE,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_DURATION,
    "Duration",
    ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_STATE,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_ON_OFF,
    "OnOff (virtual state)",
    ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_STATE,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_SUBSTATE_UPDATE,
    "Substate update",
    ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_STATE,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_CAPABILITIES_REQUESTED,
    "Multilevel Switch capabilities requested",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Manufacturer Specific Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_MANUFACTURER_SPECIFIC_VERSION,
    "Manufacturer Specific Version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_MANUFACTURER_SPECIFIC_MANUFACTURER_ID,
    "Manufacturer ID",
    ATTRIBUTE_ENDPOINT_ID,
    I32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_MANUFACTURER_SPECIFIC_PRODUCT_TYPE_ID,
    "Product Type",
    ATTRIBUTE_ENDPOINT_ID,
    I32_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_MANUFACTURER_SPECIFIC_PRODUCT_ID,
                                     "Product ID",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     I32_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_TYPE,
                                     "Device Specific ID Type",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_DATA_LENGTH_INDICATOR,
    "Data length indicator",
    ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_TYPE,
    I32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_DATA_FORMAT,
    "Data format",
    ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_TYPE,
    I32_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_DATA,
                                     "Data",
                                     ATTRIBUTE_DEVICE_SPECIFIC_DEVICE_ID_TYPE,
                                     BYTE_ARRAY_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Multi Channel Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_VERSION,
    "Multi Channel Version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Multi Channel Association Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_VERSION,
    "Multi Channel Association Version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_IDENTICAL_ENDPOINTS,
    "Identical Endpoints mask",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_INDIVIDUAL_ENDPOINTS,
    "Individual Endpoints",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_ENDPOINTS,
    "Aggregated Endpoints",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_ALL_INDIVIDUAL_ENDPOINTS_FOUND,
    "All Endpoints discovered",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_MEMBERS,
    "Aggregated endpoint members bitmask",
    ATTRIBUTE_ENDPOINT_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Notification Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_VERSION,
    "Notification Version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_NOTIFICATION_TYPES,
    "Supported Notification Types",
    ATTRIBUTE_ENDPOINT_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_TYPE,
                                     "Notification Type",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_STATES_OR_EVENTS,
    "Supported States/Events",
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_TYPE,
    BYTE_ARRAY_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_STATE,
                                     "State (Variable)",
                                     ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_TYPE,
                                     U8_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT,
                                     "Event",
                                     ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_STATE,
                                     I32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT_PARAMETERS,
    "Parameters",
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT,
    I32_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Scene Activation Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_SCENE_ACTIVATION_VERSION,
    "Scene Activation Command Class Version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Scene Actuator Configuration Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_SCENE_ACTUATOR_CONF_VERSION,
    "Scene Actuator Configuration Command Class Version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Scene Controller Configuration Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_SCENE_CONTROLLER_CONF_VERSION,
    "Scene Controller Configuration Command Class Version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Thermostat Mode Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE_VERSION,
    "Thermostat Mode Version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE,
                                     "Thermostat Mode",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     I32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES,
    "Thermostat Supported Modes",
    ATTRIBUTE_ENDPOINT_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Thermostat Setpoint Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VERSION,
    "Thermostat Setpoint Version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_SUPPORTED_SETPOINT_TYPES,
    "Thermostat Supported Setpoint Types",
    ATTRIBUTE_ENDPOINT_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPE,
    "Thermostat Setpoint Type",
    ATTRIBUTE_ENDPOINT_ID,
    I8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE,
    "Value",
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPE,
    I32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE_SCALE,
    "Value Scale",
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPE,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_MIN_VALUE,
    "Min Value",
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPE,
    I32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_MIN_VALUE_SCALE,
    "Min Value Scale",
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPE,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_MAX_VALUE,
    "Max Value",
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPE,
    I32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_MAX_VALUE_SCALE,
    "Max Value Scale",
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPE,
    U32_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Supervision Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_SUPERVISION_VERSION,
    "Supervision Version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Wake Up Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_VERSION,
                                     "Wake Up Version",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     U8_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_SETTING,
                                     "Wake Up Setting",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     ENUM_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL,
                                     "Interval",
                                     ATTRIBUTE_COMMAND_CLASS_WAKE_UP_SETTING,
                                     U32_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_NODE_ID,
                                     "NodeID",
                                     ATTRIBUTE_COMMAND_CLASS_WAKE_UP_SETTING,
                                     U16_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_CAPABILITIES,
    "Wake Up Capabilities",
    ATTRIBUTE_ENDPOINT_ID,
    ENUM_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_MINIMUM_INTERVAL,
    "Minimum Interval",
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_CAPABILITIES,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_MAXIMUM_INTERVAL,
    "Maximum Interval",
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_CAPABILITIES,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_DEFAULT_INTERVAL,
    "Default Interval",
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_CAPABILITIES,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL_STEP,
    "Interval Step",
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_CAPABILITIES,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_CAPABILITIES_BITMASK,
    "Bitmask",
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_CAPABILITIES,
    U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Security 0 Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_SECURITY_0_VERSION,
                                     "Security 0 Version",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Security 2 Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_SECURITY_2_VERSION,
                                     "Security 2 Version",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Powerlevel Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_COMMAND_CLASS_POWERLEVEL_VERSION,
    "Powerlevel Version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //        Transport Service Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_TRANSPORT_SERVICE_VERSION,
    "Transport Service Version",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Z-Wave Plus Info Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_ZWAVEPLUS_INFO_VERSION,
                                     "Z-Wave Plus Info Version",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     U8_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_ZWAVEPLUS_INFO_Z_WAVE_VERSION,
                                     "Z-Wave Plus version",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     I32_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_ZWAVEPLUS_INFO_ROLE_TYPE,
                                          "Z-Wave Plus Role Type",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          I32_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_ZWAVEPLUS_INFO_NODE_TYPE,
                                          "Z-Wave Plus Node Type",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          I32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_ZWAVEPLUS_INFO_INSTALLER_ICON_TYPE,
    "Z-Wave Plus Installer Icon Type",
    ATTRIBUTE_ENDPOINT_ID,
    I32_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_ZWAVEPLUS_INFO_USER_ICON_TYPE,
                                     "Z-Wave Plus User Icon Type",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     I32_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Version Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_VERSION_VERSION,
                                     "Version Version",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     U8_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_CC_VERSION_VERSION_REPORT_DATA,
                                     "Version Report Data",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     U8_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_CC_VERSION_ZWAVE_LIBRARY_TYPE,
                                     "Library type",
                                     ATTRIBUTE_CC_VERSION_VERSION_REPORT_DATA,
                                     U8_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_CC_VERSION_ZWAVE_PROTOCOL_VERSION,
    "Protocol Version",
    ATTRIBUTE_CC_VERSION_VERSION_REPORT_DATA,
    BYTE_ARRAY_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_CC_VERSION_FIRMWARE,
                                     "Firmware ID",
                                     ATTRIBUTE_CC_VERSION_VERSION_REPORT_DATA,
                                     U32_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_CC_VERSION_FIRMWARE_VERSION,
                                          "Firwmare Version",
                                          ATTRIBUTE_CC_VERSION_FIRMWARE,
                                          BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_CC_VERSION_ZWAVE_SOFTWARE_GET_SUPPORT,
    "Z-Wave Software Version Get Support",
    ATTRIBUTE_ENDPOINT_ID,
    U8_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_CC_VERSION_SDK_VERSION,
                                          "Z-Wave Software SDK Version",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_CC_VERSION_APPLICATION_FRAMEWORK_API_VERSION,
    "Z-Wave Software Application Framework Version",
    ATTRIBUTE_ENDPOINT_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_CC_VERSION_APPLICATION_FRAMEWORK_BUILD_NUMBER,
    "Z-Wave Software Application Framework Build Number",
    ATTRIBUTE_ENDPOINT_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_CC_VERSION_HOST_INTERFACE_VERSION,
    "Z-Wave Software Host Interface Version",
    ATTRIBUTE_ENDPOINT_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_CC_VERSION_HOST_INTERFACE_BUILD_NUMBER,
    "Z-Wave Software Host Interface Build Number",
    ATTRIBUTE_ENDPOINT_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_CC_VERSION_ZWAVE_PROTOCOL_BUILD_NUMBER,
    "Z-Wave Software Z-Wave Protocol Build Number",
    ATTRIBUTE_ENDPOINT_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_CC_VERSION_APPLICATION_BUILD_NUMBER,
    "Z-Wave Software Application Build Number",
    ATTRIBUTE_ENDPOINT_ID,
    BYTE_ARRAY_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_CC_VERSION_HARDWARE_VERSION,
                                     "Z-Wave Hardware Version",
                                     ATTRIBUTE_CC_VERSION_VERSION_REPORT_DATA,
                                     U32_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_CC_VERSION_FOUND,
                                          "Version CC is supported",
                                          ATTRIBUTE_NODE_ID,
                                          EMPTY_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         Firmware Update Command Class attributes
  /////////////////////////////////////////////////////////////////////
  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_FWU_MD_VERSION,
                                     "Firmware Update Version",
                                     ATTRIBUTE_ENDPOINT_ID,
                                     U8_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU,
                                          "Firmware Update Meta Data Report",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          I32_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW,
                                          "Firmware",
                                          ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU,
                                          I32_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW_ID,
                                     "Firmware ID",
                                     ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW,
                                     I32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW_UPGRADABLE,
    "Firmware Upgradable",
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW,
    I32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_MANUFACTURER_ID,
    "Manufacturer ID",
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU,
    I32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FRAGMENT_SIZE,
    "Max Fragment Size",
    ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
    I32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_ACTIVATION,
    "Activation",
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU,
    I32_STORAGE_TYPE);

  status |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_CC,
                                          "CC Capability",
                                          ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU,
                                          I32_STORAGE_TYPE);

  status
    |= attribute_store_register_type(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
                                     "Firmware Transfer",
                                     ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU,
                                     ENUM_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_TARGET,
    "Firmware Target",
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
    ENUM_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_APPLY_TIMESTAMP,
    "Apply After Timestamp",
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
    UNSIGNED_LONG_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_FILENAME,
    "Filename",
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
    C_STRING_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_OFFSET,
    "Transfer Offset",
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_SIZE,
    "Transfer Size",
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
    U32_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_LAST_STATUS,
    "Transfer Last Status",
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
    ENUM_STORAGE_TYPE);

  status |= attribute_store_register_type(
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER_EXPIRY_TIME,
    "Transfer Expiry Time",
    ATTRIBUTE_COMMAND_CLASS_FWU_MD_FW_TRANSFER,
    UNSIGNED_LONG_STORAGE_TYPE);

  /////////////////////////////////////////////////////////////////////
  //         ZCL Group cluster attributes
  /////////////////////////////////////////////////////////////////////
  status |= attribute_store_register_type(DOTDOT_ATTRIBUTE_ID_GROUPS_GROUP_ID,
                                          "ZCL Group ID",
                                          ATTRIBUTE_ENDPOINT_ID,
                                          U16_STORAGE_TYPE);

  status |= attribute_store_register_type(DOTDOT_ATTRIBUTE_ID_GROUPS_GROUP_NAME,
                                          "Name",
                                          DOTDOT_ATTRIBUTE_ID_GROUPS_GROUP_ID,
                                          C_STRING_STORAGE_TYPE);

  return status;
}
