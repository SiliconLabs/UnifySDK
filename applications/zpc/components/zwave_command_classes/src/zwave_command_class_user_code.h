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
 * @defgroup zwave_command_class_user_code User Code Command Class
 * @ingroup command_classes
 * @brief Command Class handler for the User Code Command Class
 *
 * The User Code command Class handler uses the following data model for
 * supporting nodes:
@startuml{attribute_store_user_code_command_class.png} "User Code Command Class data model" width=10cm
title User Code data model
allow_mixing
skinparam objectBorderColor black
skinparam NoteBackgroundColor #fffae3
skinparam NoteBorderColor red

legend top
<font color=#FEFEFE>ATTRIBUTE(type)</font> : ATTRIBUTE_COMMAND_CLASS_USER_CODE_type
endlegend

package "Attribute Store" <<Database>> {
  object "NodeID" as node #f2ffe6
  node : Attribute Type = ATTRIBUTE_NODE_ID
  node : value = Desired: [], Reported: [03]

  object "Endpoint Attribute" as endpoint #e6fff7
  endpoint : Attribute Type = ATTRIBUTE_ENDPOINT_ID
  endpoint : value = Desired: [] - Reported: [04]

  object "Version" as version #FEFEFE
  version : Attribute Type = ATTRIBUTE(VERSION)
  version : value = Desired: [] - Reported: [4]

  object "User Code Data" as user_code_data #FFFFFF
  user_code_data : Attribute Type = ATTRIBUTE(DATA)
  user_code_data : value = Desired: [] - Reported: []

  Object "Number of Users" as number_of_codes #FFFFFF
  number_of_codes : Attribute Type = ATTRIBUTE(NUMBER_OF_USERS)
  number_of_codes : value = Desired: [] - Reported: [32]

  Object "Delete All Codes requested" as delete_all #FFFFFF
  delete_all : Attribute Type = ATTRIBUTE(DELETE_ALL_REQUESTED)
  delete_all : value = Desired: [] - Reported: []

  Object "User ID" as user_id #FFFFFF
  user_id : Attribute Type = ATTRIBUTE(USER_ID)
  user_id : value = Desired: [] - Reported: [1]

  package "For each User ID " <<Rectangle>> {
    Object "User Code" as user_code #FFFFFF
    user_code : Attribute Type = ATTRIBUTE(USER_ID)
    user_code : value = Desired: [] - Reported: ["1234"]

    Object "User ID Status" as user_id_status #FFFFFF
    user_id_status : Attribute Type = ATTRIBUTE(USER_ID_STATUS)
    user_id_status : value = Desired: [] - Reported: [1]
  }

  Object "Admin Code" as admin_code #FFFFFF
  admin_code : Attribute Type = ATTRIBUTE(ADMIN_CODE)
  admin_code : value = Desired: [] - Reported: ["1234"]

  Object "Capabilities" as capabilities #FFFFFF
  capabilities : Attribute Type = ATTRIBUTE(CAPABILITES)
  capabilities : value = Desired: [] - Reported: []

  Object "Supported Flags" as supported_flags #FFFFFF
  supported_flags : Attribute Type = ATTRIBUTE(SUPPORTED_FLAGS)
  supported_flags : value = Desired: [] - Reported: [59]

  Object "Supported User ID Status" as supported_user_id_status #FFFFFF
  supported_user_id_status : Attribute Type = ATTRIBUTE(SUPPORTED_USER_ID_STATUS)
  supported_user_id_status : value = Desired: [] - Reported: [03]

  Object "Supported Keypad Modes" as supported_keypad_modes #FFFFFF
  supported_keypad_modes : Attribute Type = ATTRIBUTE(SUPPORTED_KEYPAD_MODES)
  supported_keypad_modes : value = Desired: [] - Reported: [03]

  Object "Supported Keys" as supported_keys #FFFFFF
  supported_keys : Attribute Type = ATTRIBUTE(SUPPORTED_KEYS)
  supported_keys : value = Desired: [] - Reported: [00,00,00,00,00,00,ff,03,fe,ff,ff,07]

  Object "Checksum" as checksum #FFFFFF
  checksum : Attribute Type = ATTRIBUTE(CHECKSUM)
  checksum : value = Desired: [] - Reported: [32498]

  Object "Keypad Mode" as keypad_mode #FFFFFF
  keypad_mode : Attribute Type = ATTRIBUTE(KEYPAD_MODE)
  keypad_mode : value = Desired: [2] - Reported: [1]
}

node *-- endpoint
endpoint *-- version
endpoint *-- user_code_data
user_code_data *-- number_of_codes
user_code_data *-- delete_all
user_code_data *-- user_id

user_id *-- user_code
user_id *-- user_id_status

user_code_data *-- admin_code
user_code_data *-- keypad_mode
user_code_data *-- checksum
user_code_data *-- capabilities

capabilities *-- supported_flags
capabilities *-- supported_user_id_status
capabilities *-- supported_keypad_modes
capabilities *-- supported_keys

@enduml
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_USER_CODE_H
#define ZWAVE_COMMAND_CLASS_USER_CODE_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the User Code Command Class handler
 */
sl_status_t zwave_command_class_user_code_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_USER_CODE_H
/** @} end zwave_command_class_user_code */
