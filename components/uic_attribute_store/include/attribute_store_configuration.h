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
 * @brief Configures if the Attribute Store will save changes to the datastore
 * following a periodic interval as a backup if too many updates are performed.
 *
 * @param seconds  Interval in seconds used to make periodic saves of the
 *                 Attribute Store in the Datastore. The value 0 indicates to
 *                 disable any safety back-up interval.
 */
void attribute_store_configuration_set_auto_save_safety_interval(
  unsigned int seconds);

/**
 * @brief Configures a delay after Attribute Store updates before saving
 * changes to the datastore.
 *
 * The 0 cooldown value will slow down dramatically the Attribute Store, but
 * ensure accurate data after a non-graceful shutdown/teardown, as
 * everything will be saved sequentially.
 *
 * @param seconds  Interval in seconds to wait after the last attribute update
 *                 to save it into the attribute store. The value 0 indicates to
 *                 save everything instantly.
 */
void attribute_store_configuration_set_auto_save_cooldown_interval(
  unsigned int seconds);

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_STORE_CONFIGURATION_H
/** @} end attribute_store_configuration */
