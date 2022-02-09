/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup dotdot_mapper Dotdot Mapper
 * @ingroup zpc_components
 * @brief Maps the DotDot clusters to known command classes
 *
 * @{
 */

#ifndef DOTDOT_MAPPER_H
#define DOTDOT_MAPPER_H

// Generic includes
#include <stdbool.h>
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize all known cluster mappers.
 *
 * @returns SL_STATUS_OK on success
 * @returns SL_STATUS_FAIL on failure
 *
 */
sl_status_t dotdot_mapper_init(void);

/**
 * @brief Tear down the Occupancy cluster mapper from all its content.
 *
 * @returns 0 on success
 */
int dotdot_mapper_teardown(void);

#ifdef __cplusplus
}
#endif

#endif  //DOTDOT_MAPPER_H
/** @} end dotdot_mapper */
