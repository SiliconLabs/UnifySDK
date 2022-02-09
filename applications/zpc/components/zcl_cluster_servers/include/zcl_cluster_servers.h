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
 * @defgroup zcl_cluster_servers ZCL Cluster Servers
 * @ingroup zpc_components
 * @brief List of ZCL Clusters that the Protocol Controller provides on behalf
 * of its PAN Nodes.
 *
 * This component contains a list of ZCL Cluster servers, which provide ZCL
 * functionalities for PAN nodes.
 *
 * @{
 */

#ifndef ZCL_CLUSTER_SERVERS_H
#define ZCL_CLUSTER_SERVERS_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the ZCL Cluster servers
 *
 * Each individual cluster that is being "simulated" on behalf of PAN nodes
 * will be initialized in this componnent.
 * @returns SL_STATUS_OK if initialization successful, any other code otherwise
 */
sl_status_t zcl_cluster_servers_init();

/**
 * @brief Teardown the ZCL Cluster servers
 *
 * @returns Always 0
 */
int zcl_cluster_servers_teardown();

#ifdef __cplusplus
}
#endif

#endif  //ZCL_CLUSTER_SERVERS_H
/** @} end zcl_cluster_servers */
