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
 * @defgroup unify_dotdot_attribute_store_group_cluster Unify Dotdot Attribute Store Group Cluster
 * @brief Provides group command handling and group membership publications
 *
 * @{
 */

#ifndef UNIFY_DOTDOT_ATTRIBUTE_STORE_GROUP_CLUSTER_H
#define UNIFY_DOTDOT_ATTRIBUTE_STORE_GROUP_CLUSTER_H

#include "sl_status.h"
#include <stdbool.h>
#include "uic_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Helper function indicating if a UNID/Endpoint is part of a group
 *
 * @param unid          The UNID of the node to verify
 * @param endpoint_id   The endpoint ID of the node to look for.
 * @param group_id      The group ID to look for in the unid/endpoint data
 *
 * @returns true if the unid/endpoint is part of the GroupID, false otherwise
 */
bool is_unid_endpoint_in_group(dotdot_unid_t unid,
                               dotdot_endpoint_id_t endpoint_id,
                               uint16_t group_id);
/**
 * @brief Initialize the ZCL Group cluster server
 *
 * It will publish
 *
 * @returns SL_STATUS_OK if it worked, any other code otherwise
 */
sl_status_t unify_dotdot_attribute_store_group_cluster_init();

#ifdef __cplusplus
}
#endif

#endif  //UNIFY_DOTDOT_ATTRIBUTE_STORE_GROUP_CLUSTER_H
/** @} end unify_dotdot_attribute_store_group_cluster */
