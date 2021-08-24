/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef ZWAVE_COMMAND_CLASSES_FIXT_H
#define ZWAVE_COMMAND_CLASSES_FIXT_H

// Generic includes
#include <stdbool.h>
#include "sl_status.h"

/**
 * @defgroup command_classes Z-Wave Command Classes
 * @ingroup components
 * @brief List of supported and controlled Z-Wave Command Classes
 *
 */

/**
 * @defgroup command_classes_fixt Command Class fixtures
 * @ingroup command_classes
 * @brief Initializations for Command Classes handlers
 *
 * The Command Classes fixtures provides a list of initialization functions allowing
 * to initialize Z-Wave Command handlers.
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Setup fixture for @ref command_classes.
 *
 * This setup function will call the setup/initialization functions for
 * every command class in this module.
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_command_classes_init();

#ifdef __cplusplus
}
#endif

/** @} end command_classes_fixt */

#endif  // ZWAVE_COMMAND_CLASSES_FIXT_H
