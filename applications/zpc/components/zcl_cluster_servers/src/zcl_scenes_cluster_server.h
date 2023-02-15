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
 * @defgroup zcl_scenes_cluster_server ZCL Scenes cluster server
 * @ingroup zcl_cluster_servers
 * @brief Pretends that all our (actuator) nodes support the ZCL Scenes cluster
 *
 * This components provides all ZCL scenes functionalities (cached) on behalf of
 * nodes with actuator functionalities
 *
 * @{
 */

#ifndef ZCL_SCENES_CLUSTER_SERVER_H
#define ZCL_SCENES_CLUSTER_SERVER_H

#include "sl_status.h"

/**
 * @brief Struct used to capture each unique Scene (Scene ID / Group ID).
 */
typedef struct {
  uint8_t scene_id;
  uint16_t group_id;
} scene_id_group_id_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the ZCL Scenes cluster server
 *
 * @returns SL_STATUS_OK if it worked, any other code otherwise
 */
sl_status_t zcl_scenes_cluster_server_init();

/**
 * @brief Teardown the ZCL Scenes cluster server.
 *
 */
void zcl_scenes_cluster_server_teardown();

#ifdef __cplusplus
}
#endif

#endif  //ZCL_SCENES_CLUSTER_SERVER_H
/** @} end zcl_scenes_cluster_server */
