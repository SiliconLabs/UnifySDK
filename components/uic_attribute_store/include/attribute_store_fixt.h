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

#ifndef ATTRIBUTE_STORE_FIXT_H
#define ATTRIBUTE_STORE_FIXT_H

// Generic includes
#include <stdbool.h>
#include "sl_status.h"

/**
 * @defgroup attribute_store_fixt Attribute Store Fixture
 * @ingroup attribute_store
 * @brief Initialization and teardown of the Attribute Store.
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Attribute store.
 *
 * This component depends on the following components:
 * - @ref sl_log
 * - @ref unify_datastore
 *
 * @returns SL_STATUS_OK on success
 * @returns SL_STATUS_FAIL on failure
*/
sl_status_t attribute_store_init(void);

/**
 * @brief Clears up the Attribute Store from all its content.
 *
 * @returns 0 on success
*/
int attribute_store_teardown(void);

#ifdef __cplusplus
}
#endif

/** @} end attribute_store_fixt */

#endif  //ATTRIBUTE_STORE_FIXT_H
