/***************************************************************************//**
 * @file
 * @brief CTE Handle module for AoA locator
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef AOA_CTE_PARSE_H
#define AOA_CTE_PARSE_H

#include "aoa_cte.h"
#include "aoa_types.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Public function declarations

/**************************************************************************//**
 * Parse CTE Mode configuration.
 *
 * @param[out] cte_mode CTE mode.
 * @param[in] locator_id locator id.
 *
 * @retval SL_STATUS_NULL_POINTER - Input is a null pointer.
 * @retval SL_STATUS_NOT_FOUND - Antenna mode configuration not found.
 * @retval SL_STATUS_OK - Parsing was successful.
 *****************************************************************************/
sl_status_t aoa_cte_parse_mode(aoa_cte_type_t *cte_mode, aoa_id_t locator_id);


#ifdef __cplusplus
};
#endif

#endif /* AOA_CTE_PARSE_H */
