/***************************************************************************//**
 * @file
 * @brief AoA CTE configuration parser.
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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aoa_parse.h"
#include "aoa_cte_parse.h"

// -----------------------------------------------------------------------------
// Defines

#define CHECK_TYPE(x, t)  if (((x) == NULL) || ((x)->type != (t))) return SL_STATUS_FAIL

// -----------------------------------------------------------------------------
// Public function definitions

/**************************************************************************//**
 * Parse cte mode configuration.
 *****************************************************************************/
sl_status_t aoa_cte_parse_mode(aoa_cte_type_t *cte_mode, aoa_id_t locator_id)
{
  cJSON *param = NULL;
  cJSON *locator = NULL;
  sl_status_t sc;

  if (NULL == cte_mode) {
    return SL_STATUS_NULL_POINTER;
  }

  sc = aoa_parse_find_locator_config(&locator, locator_id);
  if (sc != SL_STATUS_OK) {
    return sc;
  }

  // Try parse locator specific config
  param = cJSON_GetObjectItem(locator, "cteMode");
  if (NULL == param) {
    return SL_STATUS_NOT_FOUND;
  }
  CHECK_TYPE(param, cJSON_String);

  if (strcmp(param->valuestring, "CONN") == 0) {
    *cte_mode = AOA_CTE_TYPE_CONN;
  } else if (strcmp(param->valuestring, "CONNLESS") == 0) {
    *cte_mode = AOA_CTE_TYPE_CONN_LESS;
  } else if (strcmp(param->valuestring, "SILABS") == 0) {
    *cte_mode = AOA_CTE_TYPE_SILABS;
  } else {
    return SL_STATUS_INVALID_CONFIGURATION;
  }

  return SL_STATUS_OK;
}
