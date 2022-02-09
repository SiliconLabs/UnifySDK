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
 * @defgroup positioning_id Positioning Id
 * @brief Modules that allows to set the Id of the Positioning.
 *
 * @{
 */

#ifndef POSITIONING_ID_H
#define POSITIONING_ID_H

// Generic includes
#include <stddef.h>

#include "sl_status.h"

// Includes from other components
#include "aoa_util.h"

#define DEFAULT_POSITIONING_ID "PositioningService0"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Retrieves the Id for the Positioning
 *
 * @return Positioning Id
 */
aoa_id_t *get_positioning_id(void);

/**
 * @brief Set the Positioning Id.
 *
 * @param id Desired Id.
 */
void set_positioning_id(aoa_id_t id);

#ifdef __cplusplus
}
#endif

#endif  //POSITIONING_ID_H
/** @} end positioning_id */
