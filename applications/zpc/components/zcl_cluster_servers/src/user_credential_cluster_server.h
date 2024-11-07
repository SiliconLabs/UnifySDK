/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup user_credential_cluster_server User Code Cluster Server
 * @ingroup zcl_cluster_servers
 * @brief The module handlers the User Code command mapping
 * @{
 */

#ifndef USER_CREDENTIAL_CLUSTER_SERVER_H
#define USER_CREDENTIAL_CLUSTER_SERVER_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the User Code Cluster Commands handler.
 *
 * @returns SL_STATUS_OK on success, any other code on failure
 */
sl_status_t user_credential_cluster_server_init();

#ifdef __cplusplus
}
#endif

#endif  //USER_CREDENTIAL_CLUSTER_SERVER_H
/** @} end user_credential_cluster_server */
