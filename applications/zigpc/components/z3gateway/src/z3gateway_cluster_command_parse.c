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

#ifndef Z3GATEWAY_TEST
#include PLATFORM_HEADER
#include EMBER_AF_API_AF_HEADER
#include EMBER_AF_API_AF_SECURITY_HEADER
#include EMBER_AF_API_EMBER_TYPES

#else /* Z3GATEWAY_TEST defined */
#include <stddef.h>
#include "debug-printing.h"

#endif /* Z3GATEWAY_TEST */

#include EMBER_AF_API_ADDRESS_TABLE

#include "z3gateway_common.h"

/**
 * @brief Parse ZCL Cluster specific command frames received.
 *
 * @param cmd             Contains ZCL cluster command information.
 * @return EmberAfStatus  EMBER_ZCL_STATUS_SUCCESS if the command was handled
 * with a response already, EMBER_ZCL_STATUS_UNSUP_COMMAND if the command is not
 * supported by the gateway, EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER if the
 * cluster is not supported by the gateway.
 */
EmberAfStatus emberAfClusterSpecificCommandParse(EmberAfClusterCommand *cmd)
{
  EmberStatus status     = EMBER_SUCCESS;
  EmberAfStatus afStatus = EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER;
  EmberEUI64 sourceEui64;
  uint8_t sourceEndpoint;
  EmberAfClusterId clusterId;
  uint8_t commandId;
  bool commandFromServerToClient = false;

  if (!Z3GATEWAY_CALLBACK_EXISTS(z3gwState.callbacks,
                                 onClusterCommandReceived)) {
    emberAfCorePrintln("Ignoring cluster-specifc command received event due to "
                       "missing callback");
    status = EMBER_NOT_FOUND;
  } else {
    status = emberAfGetCurrentSenderEui64(sourceEui64);
    if (status != EMBER_SUCCESS) {
      status = EMBER_NOT_FOUND;
      emberAfCorePrintln("Error: Failed to find sender EUI64 for "
                         "cluster-specific command parsing");
    }
  }

  if (status == EMBER_SUCCESS) {
    sourceEndpoint = cmd->apsFrame->sourceEndpoint;
    clusterId      = cmd->apsFrame->clusterId;
    commandId      = cmd->commandId;
    commandFromServerToClient
      = (cmd->direction == (uint8_t)ZCL_DIRECTION_SERVER_TO_CLIENT);
    emberAfCorePrint(
      "Received buffer from ep[%u] cluster[%04X] command[%02X] buffer: [",
      sourceEndpoint,
      clusterId,
      commandId);
    for (size_t i = 0; i < cmd->bufLen; i++) {
      emberAfCorePrint(" %02X", cmd->buffer[i]);
    }
    emberAfCorePrintln(" ]");

    afStatus
      = z3gwState.callbacks->onClusterCommandReceived(sourceEui64,
                                                      sourceEndpoint,
                                                      clusterId,
                                                      commandId,
                                                      commandFromServerToClient,
                                                      cmd->buffer,
                                                      cmd->bufLen,
                                                      cmd->payloadStartIndex);
  }

  // If the callback is not called, indicate no cluster support as a response
  return afStatus;
}
