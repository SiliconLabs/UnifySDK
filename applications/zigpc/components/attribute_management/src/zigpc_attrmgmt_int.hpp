/*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/**
 * @file attribute_management_int.hpp
 * @defgroup attribute_management_int  ZigPC Attribute Management Internals
 * @ingroup attribute_management
 * @brief Internal API for attribute management functionality.
 *
 * @{
 *
**/
#ifndef ZIGPC_ATTRMGMT_INT_HPP
#define ZIGPC_ATTRMGMT_INT_HPP

#include <vector>

#include "sl_status.h"

#include "zcl_attribute_info.h"
#include "zcl_definitions.h"

/**
 * @brief Build list of ZCL Configure Report Records for a given cluster.
 *
 * @param cluster_id    ZCL Cluster ID
 * @param record_list   Configure Reporting List to populate
 * @return sl_status_t  SL_STATUS_OK if reportable attribute records were
 * populated, if not SL_STATUS_NOT_SUPPORTED is returned
 */
sl_status_t zigpc_attrmgmt_build_configure_report_records(
  zcl_cluster_id_t cluster_id,
  std::vector<zigpc_zcl_configure_reporting_record_t> &record_list);

/**
 * @brief Split a list of Attribute Report Configuration records into sets of
 * \ref ZIGPC_ATTRMGMT_REPORT_RECORDS_LIMIT_PER_MESSAGE sized messages (if
 * needed) and deliver the configuration records to the destination endpoint.
 * This is useful when configuring reports for a cluster that exceed the
 * maximum ZCL packet size.
 *
 * @param eui64         Targetted Zigbee device .
 * @param endpoint_id   Targetted endpoint in device.
 * @param cluster_id    Targetted ZCL cluster in endpoint.
 * @param record_list   Full list of attribute report records to split if
 *                      needed.
 * @return sl_status_t  SL_STATUS_OK if the split and send operations are
 * successful, SL_STATUS_NULL_POINTER if invalid arguments passed in, or
 * another error if the zigpc_gateway send process fails.
 */
sl_status_t zigpc_attrmgmt_send_split_report_config_cmds(
  const zigbee_eui64_t eui64,
  zigbee_endpoint_id_t endpoint_id,
  zcl_cluster_id_t cluster_id,
  const std::vector<zigpc_zcl_configure_reporting_record_t> &records);

/**
 * @brief Split a list of Attribute Read records into sets of
 * \ref ZIGPC_ATTRMGMT_READ_RECORDS_LIMIT_PER_MESSAGE sized ZCL frames (if
 * needed) and send each ZCL frame to the destination endpoint. The splitting
 * of the attribute read records allow frames to not exceed the maximum ZCL
 * packet size.
 *
 * @param eui64         Targetted Zigbee device .
 * @param endpoint_id   Targetted endpoint in device.
 * @param cluster_id    Targetted ZCL cluster in endpoint.
 * @param attr_ids      List of attribute IDs to read (and split if needed)
 * @return sl_status_t  SL_STATUS_OK if the split and send operations are
 * successful, SL_STATUS_NULL_POINTER if invalid arguments passed in, or
 * another error if the zigpc_gateway send process fails.
 */
sl_status_t zigpc_attrmgmt_send_split_read_cmds(
  const zigbee_eui64_t eui64,
  zigbee_endpoint_id_t endpoint_id,
  zcl_cluster_id_t cluster_id,
  const std::vector<zcl_attribute_id_t> &attr_ids);

#endif  //ZIGPC_ATTRMGMT_INT_HPP

/** @} end attribute_management_int */