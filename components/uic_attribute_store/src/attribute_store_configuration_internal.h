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
 * @defgroup attribute_store_configuration_internal Internal configuration functions
 * @ingroup attribute_store_api
 * @brief Internal functions for the attribute store configuration.
 *
 * @{
 */

#ifndef ATTRIBUTE_STORE_CONFIGURATION_INTERNAL_H
#define ATTRIBUTE_STORE_CONFIGURATION_INTERNAL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Checks if the Type validation setting is enabled.
 *
 * @returns true if enabled, false if not.
 */
bool attribute_store_is_type_validation_enabled();

/**
 * @brief Returns the auto-save safety interval configuration.
 *
 * @returns The interval in seconds at which auto-save should run even if
 * the attribute store is being modified constantly.
 */
unsigned int attribute_store_get_auto_save_safety_interval();

/**
 * @brief Returns the auto-save safety cooldown configuration.
 *
 * @returns The back-off in seconds to apply before saving to SQLite after
 * the last update in the Attribute Store
 */
unsigned int attribute_store_get_auto_save_cooldown_interval();

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_STORE_CONFIGURATION_INTERNAL_H
/** @} end attribute_store_configuration_internal */
