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
 * @brief Checks if the auto save setting is enabled
 *
 * @returns true if enabled, false if not
 */
bool attribute_store_is_auto_save_enabled();

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_STORE_CONFIGURATION_INTERNAL_H
/** @} end attribute_store_configuration_internal */
