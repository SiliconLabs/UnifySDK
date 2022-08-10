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
 * @defgroup positioning_configuration_handler Positioning Configuration handler
 * @ingroup positioning_core
 * @brief Components that takes care of Positioning (via config file)
 *
 * This components allows to parse the Positioning Configuration from file.
 *
 * @{
 */

#ifndef POSITIONING_CONFIGURATION_HANDLER_H
#define POSITIONING_CONFIGURATION_HANDLER_H

#include "angle_queue.h"

/**
 * @brief Global configuration settings for the angle queue component.
 */
extern angle_queue_config_t angle_queue_config;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Parses and apply the configuration for the Positioning
 *
 * @param file_name   Path to the filename that contains the Positioning configuration
 */
void positioning_configuration_handler_parse_config_file(const char *file_name);

#ifdef __cplusplus
}
#endif

#endif  //POSITIONING_CONFIGURATION_HANDLER_H
/** @} end positioning_configuration_handler */
