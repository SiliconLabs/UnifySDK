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
 * @defgroup aoxpc_configuration_handler AoXPC Configuration handler
 * @ingroup aoxpc_core
 * @brief Components that takes care of AoXPC Configuration (via config file or MQTT)
 *
 * This components allows to parse the AoXPC Configuration from files and
 * MQTT, and will subsequently act upon updates.
 *
 * @{
 */

#ifndef AOXPC_CONFIGURATION_HANDLER_H
#define AOXPC_CONFIGURATION_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Callback function to indicate that the AoXLocator Configuration
 * has been updated.
 *
 */
void aoxpc_configuration_handler_on_aox_locator_configuration_updated(void);


/**
 * @brief Parses and apply the configuration for the AoXPC
 *
 * @param file_name   Path to the filename that contains the AoX configuration
 */
void aoxpc_configuration_handler_parse_config_file(const char *file_name);

#ifdef __cplusplus
}
#endif

#endif  //AOXPC_CONFIGURATION_HANDLER_H
/** @} end aoxpc_configuration_handler */
