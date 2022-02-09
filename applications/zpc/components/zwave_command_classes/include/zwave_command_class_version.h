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
 * @defgroup zwave_command_class_version Version Command Class
 * @ingroup command_classes
 * @brief Version Command Class support and control handlers
 *
 * This module performs command class version probing using the
 * attribute resolver, and populate the versions to the version attributes
 * per supported command class.
 *
 * This module also perform version command class, version 3 probing.
 *
 * Version attributes like PROTOCOL_VERSION or FIRMWARE_VERSION are saved as
 * uint32_t values.
 * In case the version is given as 2 digits (e.g. 7.16),
 * the value will be 7<<16 + 16 <<8.
 * In case the version is given as 3 digits (e.g. 7.16.3)
 * the value will be  7<<16 + 16 <<8 + 3.
 *
 * The following subtree is used to keep track of version data
 * transfers:
 *
@startuml{attribute_store_version_data.png} "Version Command Class data model" width=10cm
title Version Command Class data model
allow_mixing
skinparam objectBorderColor black

legend top
<font color=#FEFEFE>ATTRIBUTE(type)</font> : ATTRIBUTE_CC_VERSION_type
endlegend

package "Attribute Store" <<Database>> {
  object "NodeID" as node_id #f2ffe6
  node_id : Attribute Type = ATTRIBUTE_NODE_ID
  node_id : value = Desired: [], Reported: [03]

  object "Endpoint Attribute" as endpoint #e6fff7
  endpoint : Attribute Type = ATTRIBUTE_ENDPOINT_ID
  endpoint : value = Desired: [] - Reported: [04]

  object "Version CC version" as version_cc_version #FFFFFF
  version_cc_version : Attribute Type = ATTRIBUTE_COMMAND_CLASS_VERSION_VERSION
  version_cc_version : value = Desired: [], Reported: [1]

  object "Version Report Data" as version_report_data #FFFFFF
  version_report_data : Attribute Type = ATTRIBUTE(VERSION_REPORT_DATA)
  version_report_data : value = Desired: [] - Reported: [1]

  object "Z-Wave Library type" as zwave_library_type #FFFFFF
  zwave_library_type : Attribute Type = ATTRIBUTE(ZWAVE_LIBRARY_TYPE)
  zwave_library_type : value = Desired: [] - Reported: [7]

  object "Z-Wave Protocol version" as zwave_protocol_version #FFFFFF
  zwave_protocol_version : Attribute Type = ATTRIBUTE(ZWAVE_PROTOCOL_VERSION)
  zwave_protocol_version : value = Desired: [] - Reported: [00 07 16 03]

  package "For each Firmware target" <<Rectangle>> {
    object "Firmware (target)" as firmware #FFFFFF
    firmware : Attribute Type = ATTRIBUTE(FIRMWARE)
    firmware : value = Desired: [] - Reported: [00]

    object "Firmware version" as firmware_version #FFFFFF
    firmware_version : Attribute Type = ATTRIBUTE(FIRMWARE_VERSION)
    firmware_version : value = Desired: [] - Reported: [00 07 15 03]
  }

  object "Z-Wave Software Get Support" as zwave_software_get_support #FFFFFF
  zwave_software_get_support : Attribute Type = ATTRIBUTE(ZWAVE_SOFTWARE_GET_SUPPORT)
  zwave_software_get_support : value = Desired: [] - Reported: [01]

  object "SDK Version" as sdk_version #FFFFFF
  sdk_version : Attribute Type = ATTRIBUTE(SDK_VERSION)
  sdk_version : value = Desired: [] - Reported: [00 07 17 00]

  object "App Framework API Version" as app_framework_api_version #FFFFFF
  app_framework_api_version : Attribute Type = ATTRIBUTE(APPLICATION_FRAMEWORK_API_VERSION)
  app_framework_api_version : value = Desired: [] - Reported: [00 07 12 15]

  object "App Framework build number" as app_framework_build #FFFFFF
  app_framework_build : Attribute Type = ATTRIBUTE(APPLICATION_FRAMEWORK_BUILD_NUMBER)
  app_framework_build : value = Desired: [] - Reported: [00 00 10 00]

  object "Host interface Version" as host_interface_version #FFFFFF
  host_interface_version : Attribute Type = ATTRIBUTE(HOST_INTERFACE_VERSION)
  host_interface_version : value = Desired: [] - Reported: [00 07 12 15]

  object "Host interface build number" as host_interface_build #FFFFFF
  host_interface_build : Attribute Type = ATTRIBUTE(HOST_INTERFACE_BUILD_NUMBER)
  host_interface_build : value = Desired: [] - Reported: [00 00 10 00]

  object "Z-Wave Protocol build number" as zwave_protocol_build #FFFFFF
  zwave_protocol_build : Attribute Type = ATTRIBUTE(ZWAVE_PROTOCOL_BUILD_NUMBER)
  zwave_protocol_build : value = Desired: [] - Reported: [00 00 10 00]

  object "Application build number" as application_build #FFFFFF
  application_build : Attribute Type = ATTRIBUTE(APPLICATION_BUILD_NUMBER)
  application_build : value = Desired: [] - Reported: [00 00 10 00]

  object "Hardware" as hardware_version #FFFFFF
  hardware_version : Attribute Type = ATTRIBUTE(HARDWARE_VERSION)
  hardware_version : value = Desired: [] - Reported: [00 00 00 01]

  object "Version CC found" as version_cc_found #FFFFFF
  version_cc_found : Attribute Type = ATTRIBUTE(FOUND)
  version_cc_found : value = Desired: [] - Reported: []
}

node_id *-down- endpoint
node_id *-down- version_cc_found
endpoint *-down- version_cc_version
endpoint *-down- version_report_data
version_cc_version -[hidden]--> version_report_data
version_report_data *-down- zwave_library_type
version_report_data *-down- zwave_protocol_version
zwave_protocol_version -[hidden]--> zwave_library_type
zwave_library_type -[hidden]--> hardware_version
version_report_data *-down- firmware
firmware *-down- firmware_version
version_report_data *-down- hardware_version

endpoint *-down- zwave_software_get_support
endpoint *-down- sdk_version
endpoint *-down- app_framework_api_version
endpoint *-down- app_framework_build
endpoint *-down- host_interface_version
endpoint *-down- host_interface_build
endpoint *-down- zwave_protocol_build
endpoint *-down- application_build

zwave_software_get_support -[hidden]--> host_interface_version
sdk_version -[hidden]--> host_interface_build
host_interface_version -[hidden]--> app_framework_api_version
host_interface_build -[hidden]--> zwave_protocol_build
app_framework_api_version -[hidden]--> app_framework_build
zwave_protocol_build -[hidden]--> application_build

@enduml
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_VERSION_H
#define ZWAVE_COMMAND_CLASS_VERSION_H

#define COMMAND_CLASS_VERSION_ATTRIBUTE_TYPE_MASK    0xFF0000FF
#define COMMAND_CLASS_IDENTIFIER_ATTRIBUTE_TYPE_MASK 0x00FFFF00

#define IS_ATTRIBUTE_TYPE_VERSION_CC_ATTRIBUTE(type)                 \
  ((COMMAND_CLASS_VERSION_ATTRIBUTE_TYPE_MASK & type) == 0x00000001) \
    && (((COMMAND_CLASS_IDENTIFIER_ATTRIBUTE_TYPE_MASK & type) >> 8) > 0x19)

#ifdef __cplusplus
extern "C" {
#endif

// Includes from other components
#include "sl_status.h"
#include "attribute_store.h"

// Generic includes
#include <stdbool.h>

/**
 * @brief Checks we know that the version CC is supported by the node.
 *
 * When this returns true, a component can trust that the version of the
 * command classes for endpoints under a node is accurate and final.
 *
 * @param node  An attribute store node that is under a NodeID attribute.
 * @returns true is the version information of Command Classes is accurate.
 *          false otherwise
 */
bool is_version_cc_found(attribute_store_node_t node);

/**
 * @brief Intitialize the Version Command Class control APIs
 *
 * This setup will register the Version Command Class handler
 * to the Z-Wave CC framework, register rule to the \ref attribute_resolver and
 * callbacks to the \ref attribute_store.
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_command_class_version_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_VERSION_H
/** @} end zwave_command_class_version */
