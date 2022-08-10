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
 * @defgroup zwave_command_class_wake_up_types Type definitions for attribute storage of the  Wake Up Command Class.
 * @ingroup zpc_attribute_store_command_classes_types
 * @brief Type definitions for the Wake Up Command Class.
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_WAKE_UP_TYPES_H
#define ZWAVE_COMMAND_CLASS_WAKE_UP_TYPES_H

///> Value in seconds to represent a Wake Up Period
typedef uint32_t wake_up_interval_t;

///> Byte in the Wake Up bitmask capabilities representing optional features.
///> bit 0 represents the Wake Up On demand capabilities.
typedef uint8_t wake_up_bitmask_t;


#endif  //ZWAVE_COMMAND_CLASS_WAKE_UP_TYPES_H
/** @} end zwave_command_class_wake_up_types */
