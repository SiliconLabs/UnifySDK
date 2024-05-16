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
 * @defgroup zpc_on_off_cluster_mapper ZPC Fan Control
 * @ingroup dotdot_mapper
 * @brief Maps OnOff Cluster incoming Commands to attribute modifications.
 *
 * @{
 */

#ifndef FAN_CONTROL_CLUSTER_SERVER_H
#define FAN_CONTROL_CLUSTER_SERVER_H

// Generic includes
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the FanControl cluster server
 *
 * @returns true on success
 * @returns false on failure
 *
 */
sl_status_t fan_control_cluster_server_init(void);

#ifdef __cplusplus
}
#endif

#endif  //FAN_CONTROL_CLUSTER_SERVER_H
/** @} end fan_control_cluster_server */
