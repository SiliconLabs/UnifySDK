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
 * @defgroup attribute_store_configuration Attribute Store Configuration
 * @ingroup attribute_store_api
 * @brief Allows to configure how the attribute store behaves
 *
 * This modules provides functions to change runtime the behavior/configuration
 * of the Attribute Store.
 *
 * @{
 */

#ifndef ATTRIBUTE_STORE_CONFIGURATION_H
#define ATTRIBUTE_STORE_CONFIGURATION_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configures if the Attribute Store will use the known type registration
 * to validate any writing to node values and adding nodes under parents.
 *
 * Enabling write validation may impact performance.
 *
 * @param enabled Set to true to enable it, false to disable.
 */
void attribute_store_configuration_set_type_validation(bool enabled);

/**
 * @brief Configures if the Attribute Store will save changes to SQLite automatically
 * after each change
 *
 * Auto-save enabled will slow down the attribute store, but ensure accurate
 * data after a non-graceful shutdown/teardown.
 *
 * @param enabled Set to true to save all changes immediately to the datastore.
 *                Set to false to save all changes only on attribute
 *                store teardown
 */
void attribute_store_configuration_set_auto_save(bool enabled);

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_STORE_CONFIGURATION_H
/** @} end attribute_store_configuration */
