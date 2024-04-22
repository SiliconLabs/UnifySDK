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
 * @defgroup zwave_command_class_user_credential_types Type definitions for attribute storage of the Sound Switch Command Class
 * @ingroup zpc_attribute_store_command_classes_types
 * @brief Type definitions for the Sound Switch Command Class.
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_USER_CREDENTIALS_TYPES_H
#define ZWAVE_COMMAND_CLASS_USER_CREDENTIALS_TYPES_H

#include <stdint.h>

///> Supported credential rules bitmask. uint8_t
typedef uint8_t user_credential_supported_credential_rules_t;

///> Supported user type bitmask. uint32_t
typedef uint32_t user_credential_supported_user_type_bitmask_t;

///> All user checksum. uint16_t
typedef uint16_t user_credential_all_users_checksum_t;

///> User Unique ID. uint16_t
typedef uint16_t user_credential_user_unique_id_t;
///> User Modifier Type. uint8_t
typedef uint8_t user_credential_user_modifier_type_t;
///> User Modifier Node ID. uint16_t
typedef uint16_t user_credential_user_modifier_node_id_t;
///> User Type. uint8_t
#define USER_CREDENTIAL_USER_TYPE_GENERAL_USER 0x00
#define USER_CREDENTIAL_USER_TYPE_PROGRAMMING_USER 0x03
#define USER_CREDENTIAL_USER_TYPE_NON_ACCESS_USER 0x04
#define USER_CREDENTIAL_USER_TYPE_DURESS_USER 0x05
#define USER_CREDENTIAL_USER_TYPE_DISPOSABLE_USER 0x06
#define USER_CREDENTIAL_USER_TYPE_EXPIRING_USER 0x07
#define USER_CREDENTIAL_USER_TYPE_REMOTE_ONLY_USER 0x09
typedef uint8_t user_credential_user_type_t;
///> User active state (0 or 1). uint8_t
typedef uint8_t user_credential_user_active_state_t;
///> User name encoding. uint8_t
typedef uint8_t user_credential_user_name_encoding_t;

///> Credential type. uint8_t
typedef uint8_t user_credential_type_t;

///> Credential rule. uint8_t
typedef uint8_t user_credential_rule_t;

///> Credential slot. uint16_t
typedef uint16_t user_credential_slot_t;

///> Operation type. uint8_t
#define USER_CREDENTIAL_OPERATION_TYPE_ADD 0x00
#define USER_CREDENTIAL_OPERATION_TYPE_MODIFY 0x01
#define USER_CREDENTIAL_OPERATION_TYPE_DELETE 0x02
typedef uint8_t user_credential_operation_type_t;

///> Credential Modifier Type. uint8_t
typedef uint8_t user_credential_modifier_type_t;
///> Credential Modifier Node ID. uint16_t
typedef uint16_t user_credential_modifier_node_id_t;

///> Expiring Timeout for User (Minutes). uint16_t
typedef uint16_t user_credential_expiring_timeout_minutes_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_USER_CREDENTIALS_TYPES_H
/** @} end zwave_command_class_user_credentials_types */
