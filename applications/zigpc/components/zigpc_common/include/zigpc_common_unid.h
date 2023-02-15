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

#ifndef ZIGPC_COMMON_UNID_H
#define ZIGPC_COMMON_UNID_H

/**
 * @file zigpc_common_unid.h
 * @defgroup zigpc_common_unid ZigPC UNID definitions and helpers
 * @ingroup zigpc_common
 * @brief ZigPC UNID definitions and helpers
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#define ZIGPC_UNID_PREFIX "zb-"
#define ZIGPC_UNID_PREFIX_LENGTH 3

#define ZIGPC_EUI64_STRING_LENGTH 16    

#define UCL_TOP_LEVEL "ucl/"
#define UCL_BY_UNID "by-unid/"
#define UCL_EP_PREFIX "/ep"
#define UCL_NODE_STATE "/State"
#define UCL_PROTOCOL_CONTROLLER "/ProtocolController"

#define UCL_ATTRIBUTE_TOPIC "/Attributes"
#define UCL_ATTRIBUTE_REPORTED_TOPIC "/Reported"
#define UCL_ATTRIBUTE_DESIRED_TOPIC "/Desired"

#define UCL_COMMAND_TOPIC "/Commands"
#define UCL_SUPPORTED_COMMAND_TOPIC "/SupportedCommands"
#define UCL_GENERATED_COMMAND_TOPIC "/SupportedGeneratedCommands"

#define UCL_VALUE "\"value\""

#ifdef __cplusplus
}
#endif

#endif /* ZIGPC_COMMON_UNID_H */

/** @} end zigpc_common_unid */
