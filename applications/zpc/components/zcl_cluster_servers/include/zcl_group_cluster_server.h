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
 * @defgroup zcl_group_cluster_server ZCL Group Cluster server
 * @ingroup zcl_cluster_servers
 * @brief Provides ZCL Group functionalities on behalf on end nodes.
 *
 * Allows to create and remove groups (with an associated name) to each UNID
 * and Endpoint in our network. Any included PAN node will be shown as a
 * Group Server.
 *
 * This component listens to Group Commands coming from the MQTT interface
 * and stores attributes directly in the ZCL space of the
 * Attribute Store. No Mapping to Z-Wave whatsoever is used here.
 *
 * The Group functionality will not be provided for the Protocol Controller
 * itself
 *
 * @{
 */

#ifndef ZCL_GROUP_CLUSTER_SERVER_H
#define ZCL_GROUP_CLUSTER_SERVER_H

#include "zwave_unid.h"
#include "zwave_controller_types.h"

#include <stdbool.h>

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
bool is_unid_endpoint_in_group(const unid_t unid,
                               zwave_endpoint_id_t endpoint_id,
                               uint16_t group_id);
/**
 * @brief Initialize the ZCL Group cluster server
 *
 * It will publish
 *
 * @returns SL_STATUS_OK if it worked, any other code otherwise
 */
sl_status_t zcl_group_cluster_server_init();

/**
 * @brief Teardown the ZCL group cluster server.
 *
 */
void zcl_group_cluster_server_teardown();

#ifdef __cplusplus
}
#endif

#endif  //ZCL_GROUP_CLUSTER_SERVER_H
/** @} end zcl_group_cluster_server */
