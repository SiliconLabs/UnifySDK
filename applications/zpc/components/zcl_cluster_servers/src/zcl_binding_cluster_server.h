/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup zcl_binding_cluster_server ZCL Binding Cluster server
 * @brief Provides the Binding cluster on behalf of PAN Nodes
 *
 * This component maps and handles the binding clusters on behalf of PAN nodes,
 * which in turns gets translated into Associations
 *
 * @{
 */

#ifndef ZCL_BINDING_CLUSTER_SERVER_H
#define ZCL_BINDING_CLUSTER_SERVER_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Binding Cluster Commands handler.
 *
 * @returns SL_STATUS_OK on success, any other code on failure
 */
sl_status_t binding_cluster_server_init();

#ifdef __cplusplus
}
#endif

#endif  //ZCL_BINDING_CLUSTER_SERVER_H
/** @} end zcl_binding_cluster_server */
