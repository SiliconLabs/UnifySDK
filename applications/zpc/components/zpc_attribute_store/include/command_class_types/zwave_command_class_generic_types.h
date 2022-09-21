/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup zwave_command_class_generic_types Generic Type definitions for Command Class attribute storage
 * @ingroup zpc_attribute_store_command_classes_types
 * @brief Generic Type definitions for Command Classes.
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_GENERIC_TYPES_H
#define ZWAVE_COMMAND_CLASS_GENERIC_TYPES_H

#include <stdbool.h>
#include <stdint.h>

// Number of Reports to Follow
typedef uint8_t reports_to_follow_t;

// Set Command counter (i.e. how many commands have been sent) for an attribute
// in a multi-set resolution.
typedef uint8_t set_commands_count_t;

// Values that can be used for "Umbrella" attributes
typedef enum {
  FINAL_STATE             = 0,
  NEEDS_ONE_COMMAND       = 1,
  NEEDS_MULTIPLE_COMMANDS = 2,
} command_status_values_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_GENERIC_TYPES_H
/** @} end zwave_command_class_generic_types */
