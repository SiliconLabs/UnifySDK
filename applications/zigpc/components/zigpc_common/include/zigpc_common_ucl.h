/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file zigpc_common_ucl.h
 * @defgroup zigpc_common_ucl ZigPC UCL MQTT Topic Definitions
 * @ingroup zigpc_common
 * @brief This header contains the definitions and functionality associated with
 * the Unified Cluster Language (UCL). This includes a centralized set of
 * definitions for format strings used by ZigPC components to build MQTT topics
 * when attaching pub/sub handlers. These definitions are using nested macros
 * to build the format strings for each topic.
 *
 * @{
 */

#ifndef ZIGPC_COMMON_UCL_H
#define ZIGPC_COMMON_UCL_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The separator used in MQTT namespacing.
 *
 */
#define UCL_TOPIC_SEP "/"

/**
 * @brief The root topic used by UCL.
 *
 */
#define UCL_TOPIC_ROOT "ucl"

/*************************************************************
 * UCL topic definitions for Smart Start.
 ************************************************************/

#define UCL_TOPIC_FMTSTR_SMARTSTART UCL_TOPIC_ROOT UCL_TOPIC_SEP "SmartStart"

#define UCL_TOPIC_SMARTSTART_LIST \
  UCL_TOPIC_FMTSTR_SMARTSTART UCL_TOPIC_SEP "List"

#define UCL_TOPIC_SMARTSTART_LIST_UPDATE \
  UCL_TOPIC_SMARTSTART_LIST UCL_TOPIC_SEP "Update"

/*************************************************************
 * UCL topic definitions using the by-unid namespace.
 ************************************************************/

/**
 * @brief UCL Topic format string for building topics under the by-unid
 * namespace.
 *
 */
#define UCL_TOPIC_FMTSTR_BY_UNID \
  UCL_TOPIC_ROOT UCL_TOPIC_SEP "by-unid" UCL_TOPIC_SEP "%s%s"

#define UCL_TOPIC_FMTSTR_BY_UNID_EP                                            \
  UCL_TOPIC_ROOT UCL_TOPIC_SEP "by-unid" UCL_TOPIC_SEP "%s" UCL_TOPIC_SEP "ep" \
                               "%u"

/*************************************************************
 * UCL topic definitions for ProtocolController functionality.
 ************************************************************/

/**
 * @brief UCL Topic format string for building topics under the
 * ProtocolController namespace.
 *
 */
#define UCL_TOPIC_FMTSTR_BY_UNID_PROTCTRL \
  UCL_TOPIC_FMTSTR_BY_UNID UCL_TOPIC_SEP "ProtocolController"

/**
 * @brief UCL Topic format string to publish  NetworkManagement updates.
 *
 */
#define UCL_TOPIC_FMTSTR_BY_UNID_PROTCTRL_NETMGMT \
  UCL_TOPIC_FMTSTR_BY_UNID_PROTCTRL UCL_TOPIC_SEP "NetworkManagement"

/**
 * @brief UCL Topic format string to subscribe for Networkmanagement write
 * messages.
 *
 */
#define UCL_TOPIC_FMTSTR_BY_UNID_PROTCTRL_NETMGMT_WRITE \
  UCL_TOPIC_FMTSTR_BY_UNID_PROTCTRL_NETMGMT UCL_TOPIC_SEP "Write"

/*************************************************************
 * UCL topic definitions for Node functionality.
 ************************************************************/

/**
 * @brief UCL Topic format string to publish  Node state updates.
 *
 */
#define UCL_TOPIC_FMTSTR_BY_UNID_STATE \
  UCL_TOPIC_FMTSTR_BY_UNID UCL_TOPIC_SEP "State"

#define ZIGPC_UNID_MQTT_TOPIC "by-unid/"
#define UCL_STRING_MAX_LENGTH 32

sl_status_t ucl_extract_unid_string(const char *topic_string,
                                    char *unid_string,
                                    const size_t unid_size);

sl_status_t ucl_extract_endpoint_string(const char *topic_string,
                                        char *endpoing_str,
                                        const size_t endpoint_size);

sl_status_t ucl_extract_cluster_string(const char *topic_string,
                                       char *cluster_string,
                                       const size_t cmd_size);

sl_status_t ucl_extract_cmd_string(const char *topic_string,
                                   char *cmd_string,
                                   const size_t cmd_size);

sl_status_t ucl_endpoint_str_to_int(const char *endpoint_str,
                                    int *endpoint_dest);
#ifdef __cplusplus
}
#endif

/** @} end zigpc_common_ucl */

#endif /* ZIGPC_COMMON_UCL_H */
