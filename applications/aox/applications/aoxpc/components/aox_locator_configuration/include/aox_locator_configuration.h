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
 * @defgroup aox_locator_configuration AoXLocator Configuration component
 * @brief Component handling AoXLocator attribute configuration.
 *
 * Components that allows to read the configuration of the AoXLocator cluster
 * and get notified when it has been updated (via MQTT)
 *
 * @{
 */

#ifndef AOX_LOCATOR_CONFIGURATION_H
#define AOX_LOCATOR_CONFIGURATION_H

#include "sl_status.h"
#include "aox_locator_configuration_attributes.h"

/**
 * @brief Callback function for configuration updates
 */
typedef void (*aox_locator_on_configuration_updated_t)(void);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the AoX Locator Configuration.
 *
 * @return SL_STATUS_OK on success
 */
sl_status_t aox_locator_configuration_init();

/**
 * @brief Returns the current configuration.
 *
 * @return Pointer to the aox_locator_attributes_t configuration
 */
const aox_locator_attributes_t *aox_locator_configuration_get_configuration();

/**
 * @brief Registers a callback that will be invoked if the configuration
 * has been updated (either by the application or via MQTT)
 *
 * @param callback    Function to invoke when a configuration has changed
 *
 * @return SL_STATUS_OK if callback is registered, any other value otherwise
 */
sl_status_t aox_locator_register_on_configuration_updated_callback(
  const aox_locator_on_configuration_updated_t callback);

#ifdef __cplusplus
}
#endif

#endif  //AOX_LOCATOR_CONFIGURATION_H
/** @} end aox_locator_configuration */
