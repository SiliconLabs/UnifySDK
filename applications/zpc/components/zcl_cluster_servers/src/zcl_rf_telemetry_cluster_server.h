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
 * @defgroup zcl_rf_telemetry_cluster_server ZCL RF Telemetry Cluster server
 * @ingroup zcl_cluster_servers
 * @brief ZCL cluster server providing ProtocolController/RFTelemetry for the ZPC.
 *
 * This components provides an attribute to toggle the reporting of RF Tx Reports.
 *
 * @{
 */

#ifndef ZCL_RF_TELEMETRY_CLUSTER_SERVER_H
#define ZCL_RF_TELEMETRY_CLUSTER_SERVER_H

#include "sl_status.h"

// Type for the value stored for the tx_report_enabled attribute
typedef int32_t tx_report_enabled_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the ZCL ProtocolController RF Telemetry cluster service.
 *
 * @return SL_STATUS_OK on success, any other error code otherwise.
 */
sl_status_t zcl_rf_telemetry_cluster_server_init();

#ifdef __cplusplus
}
#endif

#endif  //ZCL_RF_TELEMETRY_CLUSTER_SERVER_H
/** @} end zcl_rf_telemetry_cluster_server */
