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
 * @file zigpc_binding_request.h
 * @defgroup zigpc_binding_request
 * @ingroup zigpc_components
 * @brief API and functions for sending bind and unbind requests.
 * Intended to operate on raw MQTT topics and payloads, this module
 * parses messages and converts them into events sent to the gateway
 *
 *
**/

#ifndef ZIGPC_BINDING_REQUEST_H 
#define ZIGPC_BINDING_REQUEST_H 

#include "sl_status.h"
#include "zigpc_common_zigbee.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief zigpc_binding_parse_bind_cmd - parse an MQTT message into
 * a zigbee ZDO bind command
 *
 * @param topic - the MQTT topic containing node, endpoint and the
 * "Binding" command
 * @param message - the MQTT payload containing the destination
 * node/endpoint/cluster combination
 * @param message_length - the length of the MQTT payload
 *
**/
void zigpc_binding_parse_bind_cmd(
    const char *topic,
    const char *message,
    const size_t message_length);

/**
 * @brief zigpc_binding_parse_unbind_cmd - parse an MQTT message into
 * a zigbee ZDO unbind command
 *
 * @param topic - the MQTT topic containing node, endpoint and the
 * "Unbinding" command
 * @param message - the MQTT payload containing the destination
 * node/endpoint/cluster combination
 * @param message_length - the length of the MQTT payload
 *
**/
void zigpc_binding_parse_unbind_cmd(
    const char *topic,
    const char *message,
    const size_t message_length);

/**
 * @brief zigpc_binding_parse_cmd_args - "Binding" and "Unbinding"
 * commands are very similar. This function extracts the 
 * zigbee_binding_t directly from MQTT information to be used in
 * either of the above commands
 *
 * @param topic - the MQTT topic containing node, endpoint and the
 * "Unbinding" command
 * @param message - the MQTT payload containing the destination
 * node/endpoint/cluster combination
 * @param message_length - the length of the MQTT payload
 * @param binding - the destination where to put all "binding"
 * information
 *
 * @return SL_STATUS_OK if the MQTT message could be successfully parsed
**/
sl_status_t zigpc_binding_parse_cmd_args(
    const char *topic,
    const char *message,
    const size_t message_length,
    zigbee_binding_t *binding);

#ifdef __cplusplus
}
#endif
#endif //ZIGPC_BINDING_REQUEST_H
