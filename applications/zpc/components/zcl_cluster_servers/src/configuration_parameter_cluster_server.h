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
 * @defgroup configuration_parameter_cluster_server ZCL Configuration Parameter cluster
 * @ingroup zcl_cluster_servers
 * @brief The module handlers the Configuration Parmater Cluster commands and
 *        update directly the attribute on Z-Wave domain that are affected via the
 *        commands.
 *
 * @{
 */

#ifndef CONFIGURATION_PARAMETER_CLUSTER_SERVER_H
#define CONFIGURATION_PARAMETER_CLUSTER_SERVER_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Configuration Parameter Cluster Commands handler.
 *
 * @returns SL_STATUS_OK on success, any other code on failure
 */
sl_status_t configuration_parameter_cluster_server_init(void);

#ifdef __cplusplus
}
#endif

#endif  //CONFIGURATION_PARAMETER_CLUSTER_SERVER_H
/** @} end configuration_cluster_mapper */
