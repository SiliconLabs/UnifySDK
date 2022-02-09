/***************************************************************************//**
 * @file
 * @brief Angle queue
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#ifndef ANGLE_QUEUE_H
#define ANGLE_QUEUE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "aoa_types.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Defines
#define ANGLE_QUEUE_DEFAULT_CONFIG    { 0, 6, 20 }

// -----------------------------------------------------------------------------
// Type definitions.

typedef struct {
  uint32_t locator_count;                 // Locator count in the system.
  uint32_t sequence_ids;                  // Sequence id in the system. Set to 2 in case it is configured less than 2.
  uint32_t max_sequence_diff;             // Maximum difference from the new sequence id.
} angle_queue_config_t;

// -----------------------------------------------------------------------------
// Public functions.

/**************************************************************************//**
 * Initializes the module.
 *
 * @param[in] config Configuration.
 *
 * @retval SL_STATUS_ALREADY_INITIALIZED - Already initialized.
 * @retval SL_STATUS_INVALID_CONFIGURATION - Wrong configuration values.
 * @retval SL_STATUS_OK - Initialization successful.
 *****************************************************************************/
sl_status_t angle_queue_init(angle_queue_config_t *config);

/**************************************************************************//**
 * Pushes a new angle data to the queue.
 *
 * @param[in] tag_id id of the tag.
 * @param[in] locator_id id of the locator.
 * @param[in] angle angle data to be pushed.
 *
 * @retval SL_STATUS_FAIL - Error.
 * @retval SL_STATUS_OK - Push is successful.
 *****************************************************************************/
sl_status_t angle_queue_push(aoa_id_t tag_id,
                             aoa_id_t locator_id,
                             aoa_angle_t *angle);

/**************************************************************************//**
 * Returns the configuration.
 * 
 * @retval Angle queue configuration.
 *****************************************************************************/
angle_queue_config_t *angle_queue_get_config(void);

/**************************************************************************//**
 * Deinitializes the queue.
 *****************************************************************************/
void angle_queue_deinit(void);

/**************************************************************************//**
 * Angles ready callback. Weak, implement it in the application.
 * 
 * @param[in] tag_id id of the tag.
 * @param[in] angle_count Number of angles and locators.
 * @param[in] angle_list List of angles.
 * @param[in] locator_list List of locators.
 *****************************************************************************/
void angle_queue_on_angles_ready(aoa_id_t tag_id,         
                                 uint32_t angle_count,           
                                 aoa_angle_t *angle_list,       
                                 aoa_id_t *locator_list);

#ifdef __cplusplus
};
#endif

#endif /* ANGLE_QUEUE_H*/
