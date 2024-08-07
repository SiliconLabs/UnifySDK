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

#include PLATFORM_HEADER
#include "app/framework/include/af.h"
#include "app/framework/security/af-security.h"
#include "app/framework/plugin/address-table/address-table.h"

#include "zigbee_host_common.h"

extern struct zigbeeHostState z3gwState;

/**
 * @brief Parse ZCL Cluster specific command frames received.
 *
 * @param opcode          Service callback operation code.
 * @param context         Context that is passed to service function.
 *                        Depends on actual op code which actual part is used.
 * @return sl_zigbee_af_status_t  SL_ZIGBEE_ZCL_STATUS_SUCCESS if the command was handled
 * with a response already, SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND if the command is not
 * supported by the gateway, SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_CLUSTER if the
 * cluster is not supported by the gateway.
 */
sl_zigbee_af_status_t
  emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                sl_service_function_context_t *context)
{
  if (opcode != SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND) {
    return SL_ZIGBEE_ZCL_STATUS_FAILURE;
  }

  if (!ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks,
                                   onClusterCommandReceived)) {
    appDebugPrint(LOG_FMTSTR_UNREGISTERED_CALLBACK,
                  "Cluster-specific Command Receive");
    return SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_CLUSTER;
  }

  sl_802154_long_addr_t sourceEui64;
  sl_status_t status = sl_zigbee_af_get_current_sender_eui64(sourceEui64);
  if (status != SL_STATUS_OK) {
    appDebugPrint(LOG_FMTSTR_SOURCE_EUI64_RES_FAIL,
                  "Cluster-specific Command Receive");
    return SL_ZIGBEE_ZCL_STATUS_FAILURE;
  }

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  uint8_t sourceEndpoint     = cmd->apsFrame->sourceEndpoint;
  sl_zigbee_af_cluster_id_t clusterId = cmd->apsFrame->clusterId;
  uint8_t commandId          = cmd->commandId;
  bool commandFromServerToClient
    = (cmd->direction == (uint8_t)ZCL_DIRECTION_SERVER_TO_CLIENT);

  appDebugPrint(
    "Received command from ep[%u] cluster[%04X] command[%02X] buffer: [",
    sourceEndpoint,
    clusterId,
    commandId);
  appDebugPrintBuffer(cmd->buffer, cmd->bufLen, true);
  appDebugPrint("]\n");

  return z3gwState.callbacks->onClusterCommandReceived(
    sourceEui64,
    sourceEndpoint,
    clusterId,
    commandId,
    commandFromServerToClient,
    cmd->buffer,
    cmd->bufLen,
    cmd->payloadStartIndex);
}
