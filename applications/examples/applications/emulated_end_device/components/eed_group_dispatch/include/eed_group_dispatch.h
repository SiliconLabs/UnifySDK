/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup eed_group_dispatch EED Group dispatch
 * @brief Dispatches Group Commands to individual members
 *
 * @{
 */

#ifndef EED_GROUP_DISPATCH_H
#define EED_GROUP_DISPATCH_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the EED Group dispatch resources.
 */
sl_status_t eed_group_dispatch_init();

#ifdef __cplusplus
}
#endif

#endif  //EED_GROUP_DISPATCH_H
/** @} end eed_group_dispatch */
