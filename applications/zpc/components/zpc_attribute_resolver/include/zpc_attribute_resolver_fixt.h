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
* @defgroup zpc_attribute_resolver_fixt ZPC Attribute Resolver Fixtured
* @ingroup zpc_attribute_resolver
* @brief ZPC Attribute Resolver fixtures
*
* Setup and Teardown fixtures for the @ref zpc_attribute_resolver
*
* @{
*/
#ifndef ZPC_ATTRIBUTE_RESOLVER_FIXT_H
#define ZPC_ATTRIBUTE_RESOLVER_FIXT_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the ZPC attribute resolver, which
 * in turn calls Unify attribute resolver init
 * @returns sl_status_t indicating the outcome of initialization
 */
sl_status_t zpc_attribute_resolver_init();

/**
 * @brief Teardown the ZPC attribute resolver
 * It will also teardown the Unify Attribute resolver.
 * @returns 0 on success, any other value in case of error
 */
int zpc_attribute_resolver_teardown();

#ifdef __cplusplus
}
#endif

#endif  // ZPC_ATTRIBUTE_RESOLVER_FIXT
/** @} end zpc_attribute_resolver */
