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
 * @file zcl_OTA_cluster_server.hpp
 * @brief Monitor OTA upgrade information and publish status updates
 *
 * The ZPC OTA cluster service is responsible for triggering firmware update 
 * status information from the attribute store to MQTT. This is done though the uic_ota
 * component which performs the actual publishing.
 *
 * @{
 */

#ifndef ZCL_OTA_CLUSTER_SERVER_HPP
#define ZCL_OTA_CLUSTER_SERVER_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"

/**
 * @brief Initialize the OTA cluster service
 * 
 * @return sl_status_t 
 */
sl_status_t zcl_OTA_cluster_server_init();

#ifdef __cplusplus
}
#endif

#endif  //ZCL_OTA_CLUSTER_SERVER_HPP
/** @} end zcl_OTA_cluster_server */
