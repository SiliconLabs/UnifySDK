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
 * @defgroup zwave_nif_resolver Z-Wave NIF Resolver
 * @ingroup command_classes
 * @brief  This component handles resolving Node Info Frame attributes
 *         and kickstart Z-Wave node interviews.
 *
 *
 * This component is the start point of a Node/endpoint interview.
 * It can resolve ATTRIBUTE_ZWAVE_NIF and
 * ATTRIBUTE_ZWAVE_SECURE_NIF attributes. It will traverse the tree and
 * find out which commands to send for resolving that these 2 attribute types.
 *
 * This component also creates a ATTRIBUTE_ZWAVE_SECURE_NIF under an endpoint, when
 * a Node Information Update has been received from the controller and the node has
 * some security keys granted. In order to trigger this, ensure that a Node Info Frame
 * is received after a node has received its security keys.
 *
 * The version Command Class handler will be used in coordination to get
 * the interview of all individual command classes started.
 *
@startuml{node_interview_anatomy.png} "The anatomy of a Node (Endpoint) Interview" width=10cm
title The anatomy of a Node (Endpoint) interview
allow_mixing
skinparam objectBorderColor black

legend top
<font color=#118648>--></font> Listens to updates
<font color=#7d2508><--></font> Resolves attribute
<font color=#7e770a><--</font> Creates attribute
endlegend

package "Another Component" {
  object "Another component" as other_component
  other_component : Wants an interview to start
}

package "Attribute Store" <<Database>> {
  object "NodeID" as node #f2ffe6
  node : Attribute Type = ATTRIBUTE_NODE_ID
  node : value = Desired: [], Reported: [03]

  object "Endpoint Attribute" as endpoint #e6fff7
  endpoint : Attribute Type = ATTRIBUTE_ENDPOINT_ID
  endpoint : value = Desired: [] - Reported: [04]

  object "Granted keys" as granted_keys #E8EAF6
  granted_keys : Attribute Type = ATTRIBUTE_GRANTED_SECURITY_KEYS
  granted_keys : value = Desired: [] - Reported: [83]

  object "NIF" as nif #FFFFFF
  nif : Attribute Type = ATTRIBUTE_ZWAVE_NIF
  nif : value = Desired: [], Reported: []

  object "Secure NIF" as snif #FFFFFF
  snif : Attribute Type = ATTRIBUTE_SECURE_ZWAVE_NIF
  snif : value = Desired: [], Reported: []

  object "Version CC Found" as version_found #FFFFFF
  version_found : Attribute Type = ATTRIBUTE_CC_VERSION_FOUND
  version_found : value = Desired: [], Reported: []
}

package "Z-Wave NIF Resolver" {
  object "NIF Resolution" as nif_resolution
  nif_resolution : Finds the right command for resolving the NIFs

  object "NIF Updates" as nif_updates
  nif_updates : Verifies granted keys
  nif_updates : Create Secure NIF
}


package "Version Command Class handler" {
  object "NIF Updates" as version_nif_updates
  version_nif_updates : Search for Version CC
  version_nif_updates : Create supported CC version attributes

  object "Version found update" as version_found_update
  version_found_update : unresolve CC Versions attribute, when added
  version_found_update : set back CC Versions attributes to 1, when removed
}

node *-- endpoint
node *-- granted_keys
endpoint *-- nif
endpoint *-- snif
node *-- version_found

nif <-[#7e770a]- other_component

nif <-[#7d2508]-> nif_resolution
snif <-[#7d2508]-> nif_resolution

nif -[#118648]-> nif_updates
nif_updates -[#7e770a]-> snif

nif -[#118648]-> version_nif_updates
snif -[#118648]-> version_nif_updates
version_nif_updates -[#7e770a]-> version_found
version_found -[#118648]-> version_found_update

@enduml
 *
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_NODE_INFO_RESOLVER_H
#define ZWAVE_COMMAND_CLASS_NODE_INFO_RESOLVER_H

#include "sl_status.h"

// Node information frame structure. It is not provided by ZW_classcmd.h
typedef struct _request_node_info_frame_t_ {
  uint8_t command_class; /* The command class */
  uint8_t command;       /* The command */
} request_node_info_frame_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Registers callbacks to the attribute store and rules
 * to the attribute resolver to be able to resolve Node's capabilities
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_command_class_node_info_resolver_init(void);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_NODE_INFO_RESOLVER_H
/** @} end zwave_nif_resolver */
