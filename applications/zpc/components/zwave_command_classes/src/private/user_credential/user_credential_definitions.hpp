/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef USER_CREDENTIAL_DEFINITIONS_HPP
#define USER_CREDENTIAL_DEFINITIONS_HPP

// Log
#include "sl_log.h"

#ifndef ATTRIBUTE
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_USER_CREDENTIAL_##type
#endif

constexpr char LOG_TAG[] = "zwave_command_class_user_credential";

#endif // USER_CREDENTIAL_DEFINITIONS_HPP