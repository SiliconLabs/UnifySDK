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
 * @file ucl_mqtt_node_interview.h
 * @defgroup ucl_mqtt_node_interview UCL MQTT Node/Endpoint interview
 * @ingroup ucl_mqtt
 * @brief This submodule provides APIs to initiate a node re-interview
 * *
 * @{
 */

#ifndef UCL_MQTT_NODE_INTERVIEW_H
#define UCL_MQTT_NODE_INTERVIEW_H

#include "sl_status.h"
#include "zwave_unid.h"
#include "zwave_controller_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initiates the re-interview of a NodeID
 *
 * @param node_unid   The UNID of the node to interview.
 * @returns SL_STATUS_OK if the interview has been initiated,
 * SL_STATUS_NOT_FOUND if the NodeID does not exist, SL_STATUS_FAIL if
 * an error occureed.
 */
sl_status_t ucl_mqtt_initiate_node_interview(const unid_t node_unid);

/**
 * @brief Initiates the re-interview of an endpoint
 *
 * Warning: if you call this function on an aggregated endpoint,
 * it will lose its aggregated endpoint data until the endpoint 0 or
 * the whole node is interviewed again.
 *
 * @param node_unid     The UNID of the node to interview.
 * @param endpoint_id   The End Point ID under the NodeID that is to
 *                       be interviewed.
 * @returns SL_STATUS_OK if the interview has been initiated,
 * SL_STATUS_NOT_FOUND if the NodeID does not exist, SL_STATUS_FAIL if
 * an error occurred.
 */
sl_status_t
  ucl_mqtt_initiate_endpoint_interview(const unid_t node_unid,
                                       zwave_endpoint_id_t endpoint_id);

#ifdef __cplusplus
}
#endif

#endif  //UCL_MQTT_NODE_INTERVIEW_H
/** @} end ucl_mqtt_node_interview */
