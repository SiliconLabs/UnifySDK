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
 * @defgroup eed_attribute_store_clusters EED Attribute Store Clusters
 * @brief Handles creating all supported cluster attributes when endpoints appear
 *
 * @{
 */

#ifndef EED_ATTRIBUTE_STORE_CLUSTERS_H
#define EED_ATTRIBUTE_STORE_CLUSTERS_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

sl_status_t eed_attribute_store_clusters_init();

#ifdef __cplusplus
}
#endif

#endif  //EED_ATTRIBUTE_STORE_CLUSTERS_H
/** @} end eed_attribute_store_clusters */
