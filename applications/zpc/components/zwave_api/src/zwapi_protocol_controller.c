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

#include <string.h>
#include <assert.h>
#include "zwapi_protocol_controller.h"
#include "zwapi_func_ids.h"
#include "zwapi_session.h"
#include "zwapi_init.h"
#include "zwapi_internal.h"
#include "zwapi_utils.h"

sl_status_t zwapi_request_neighbor_update(zwave_node_id_t bNodeID,
                                           void (*completedFunc)(uint8_t))
{
  uint8_t func_id, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0};
  func_id                                 = (completedFunc == NULL ? 0 : 0x03);
  zwapi_write_node_id(request_buffer, &index, bNodeID);
  request_buffer[index++]                 = func_id;
  zwapi_request_neighbor_update_callback = completedFunc;

  return zwapi_send_command(FUNC_ID_ZW_REQUEST_NODE_NEIGHBOR_UPDATE,
                            request_buffer,
                            index);
}

sl_status_t zwapi_get_protocol_info(zwave_node_id_t bNodeID,
                                    zwapi_node_info_header_t *node_info_header)
{
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};

  zwapi_write_node_id(request_buffer, &index, bNodeID);
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_GET_NODE_PROTOCOL_INFO,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > (IDX_DATA + 5)) {
    node_info_header->capability         = response_buffer[IDX_DATA];
    node_info_header->security           = response_buffer[IDX_DATA + 1];
    node_info_header->reserved           = response_buffer[IDX_DATA + 2];
    node_info_header->node_type.basic    = response_buffer[IDX_DATA + 3];
    node_info_header->node_type.generic  = response_buffer[IDX_DATA + 4];
    node_info_header->node_type.specific = response_buffer[IDX_DATA + 5];
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_get_virtual_nodes(zwave_nodemask_t node_mask)
{
  uint8_t response_length                   = 0;
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_GET_VIRTUAL_NODES,
                                       NULL,
                                       0,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK
      && response_length >= (IDX_DATA + ZW_MAX_NODEMASK_LENGTH)) {
    memcpy(node_mask, response_buffer + IDX_DATA, ZW_MAX_NODEMASK_LENGTH);
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_set_default(
  void (*completedFunc)(void)) /* IN Command completed call back function */
{
  uint8_t func_id, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0};
  func_id                    = (completedFunc == NULL ? 0 : 0x03);
  request_buffer[index++]    = func_id;
  zwapi_set_default_callback = completedFunc;

  return zwapi_send_command(FUNC_ID_ZW_SET_DEFAULT, request_buffer, index);
}

sl_status_t
  zwapi_transfer_primary_ctrl_role(uint8_t bMode,
                                   void (*completedFunc)(LEARN_INFO *))
{
  uint8_t func_id, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0};
  func_id                 = (completedFunc == NULL ? 0 : 0x03);
  request_buffer[index++] = bMode;
  request_buffer[index++] = func_id;
  zwapi_transfer_primary_ctrl_role_callback = completedFunc;

  return zwapi_send_command(FUNC_ID_ZW_CONTROLLER_CHANGE,
                            request_buffer,
                            index);
}

sl_status_t zwapi_create_new_primary_ctrl(uint8_t bMode,
                                          void (*completedFunc)(LEARN_INFO *))
{
  uint8_t func_id, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0};
  func_id                                 = (completedFunc == NULL ? 0 : 0x03);
  request_buffer[index++]                 = bMode;
  request_buffer[index++]                 = func_id;
  zwapi_create_new_primarly_ctrl_callback = completedFunc;

  return zwapi_send_command(FUNC_ID_ZW_NEW_CONTROLLER, request_buffer, index);
}

sl_status_t zwapi_add_node_to_network(uint8_t bMode,
                                      void (*completedFunc)(LEARN_INFO *))
{
  uint8_t func_id, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0};
  func_id                            = (completedFunc == NULL ? 0 : 0x03);
  request_buffer[index++]            = bMode;
  request_buffer[index++]            = func_id;
  zwapi_add_node_to_network_callback = completedFunc;

  return zwapi_send_command(FUNC_ID_ZW_ADD_NODE_TO_NETWORK,
                            request_buffer,
                            index);
}

sl_status_t zwapi_remove_node_from_network(uint8_t bMode,
                                           void (*completedFunc)(LEARN_INFO *))
{
  uint8_t func_id, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0};
  func_id                                 = (completedFunc == NULL ? 0 : 0x03);
  request_buffer[index++]                 = bMode;
  request_buffer[index++]                 = func_id;
  zwapi_remove_node_from_network_callback = completedFunc;

  return zwapi_send_command(FUNC_ID_ZW_REMOVE_NODE_FROM_NETWORK,
                            request_buffer,
                            index);
}

uint8_t zwapi_remove_failed_node(zwave_node_id_t NodeID, void (*completedFunc)(uint8_t))
{
  uint8_t func_id, response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  func_id                           = (completedFunc == NULL ? 0 : 0x03);
  zwapi_write_node_id(request_buffer, &index, NodeID);
  request_buffer[index++]           = func_id;
  zwapi_remove_failed_node_callback = completedFunc;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_REMOVE_FAILED_NODE_ID,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);
  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA) {
    return response_buffer[IDX_DATA];
  }

  return ZW_FAILED_NODE_REMOVE_FAIL;
}

uint8_t zwapi_replace_failed_node(zwave_node_id_t NodeID,
                                  bool bNormalPower,
                                  void (*completedFunc)(uint8_t txStatus))
{
  uint8_t func_id, response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  func_id                            = (completedFunc == NULL ? 0 : 0x03);
  zwapi_write_node_id(request_buffer, &index, NodeID);
  request_buffer[index++]            = func_id;
  zwapi_replace_failed_node_callback = completedFunc;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_REPLACE_FAILED_NODE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);
  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA) {
    return response_buffer[IDX_DATA];
  }

  // Not a typo. Replace failed and remove fail use the same return status codes
  return ZW_FAILED_NODE_REMOVE_FAIL;
}

sl_status_t zwapi_assign_return_route(zwave_node_id_t bSrcNodeID,
                                      zwave_node_id_t bDstNodeID,
                                      void (*completedFunc)(uint8_t bStatus))
{
  uint8_t func_id, response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  func_id                            = (completedFunc == NULL ? 0 : 0x03);
  zwapi_write_node_id(request_buffer, &index, bSrcNodeID);
  zwapi_write_node_id(request_buffer, &index, bDstNodeID);
  request_buffer[index++]            = func_id;
  zwapi_assign_return_route_callback = completedFunc;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_ASSIGN_RETURN_ROUTE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_delete_return_route(zwave_node_id_t nodeID,
                                      void (*completedFunc)(uint8_t))
{
  uint8_t func_id, response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  func_id                            = (completedFunc == NULL ? 0 : 0x03);
  zwapi_write_node_id(request_buffer, &index, nodeID);
  request_buffer[index++]            = func_id;
  zwapi_delete_return_route_callback = completedFunc;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_DELETE_RETURN_ROUTE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_assign_suc_return_route(zwave_node_id_t bSrcNodeID,
                                          void (*completedFunc)(uint8_t))
{
  uint8_t func_id, response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  func_id                                = (completedFunc == NULL ? 0 : 0x03);
  zwapi_write_node_id(request_buffer, &index, bSrcNodeID);
  request_buffer[index++]                = func_id;
  zwapi_assign_suc_return_route_callback = completedFunc;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_ASSIGN_SUC_RETURN_ROUTE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_delete_suc_return_route(zwave_node_id_t nodeID,
                                          void (*completedFunc)(uint8_t))
{
  uint8_t func_id, response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  func_id                                = (completedFunc == NULL ? 0 : 0x03);
  zwapi_write_node_id(request_buffer, &index, nodeID);
  request_buffer[index++]                = func_id;
  zwapi_delete_suc_return_route_callback = completedFunc;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_DELETE_SUC_RETURN_ROUTE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

zwave_node_id_t zwapi_get_suc_node_id(void)
{
  uint8_t response_length                   = 0;
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};
  zwave_node_id_t suc_node_id = 0;
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_GET_SUC_NODE_ID,
                                       NULL,
                                       0,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK &&
      (((NODEID_8BITS == zwapi_get_node_id_basetype()) &&(response_length > IDX_DATA)) ||
      ((NODEID_16BITS == zwapi_get_node_id_basetype()) &&(response_length > IDX_DATA + 1)))) {
    int current_index = IDX_DATA;
    if (NODEID_16BITS == zwapi_get_node_id_basetype()) {
      suc_node_id = response_buffer[current_index++] << 8;
    }
    suc_node_id |= response_buffer[current_index++] & 0xFF;
  }

  return suc_node_id;
}

sl_status_t zwapi_set_suc_node_id(zwave_node_id_t nodeID,
                                  uint8_t SUCState,
                                  uint8_t bTxOption,
                                  uint8_t bCapabilities,
                                  void (*completedFunc)(uint8_t txStatus))
{
  uint8_t func_id, response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  func_id                 = (completedFunc == NULL ? 0 : 0x03);
  zwapi_write_node_id(request_buffer, &index, nodeID);
  request_buffer[index++] = SUCState; /* Do we want to enable or disable?? */
  request_buffer[index++] = bTxOption;
  request_buffer[index++] = bCapabilities;
  request_buffer[index++] = func_id;
  zwapi_set_suc_node_id_callback = completedFunc;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_SET_SUC_NODE_ID,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_set_learn_mode(uint8_t mode,
                                 void (*completedFunc)(LEARN_INFO *))
{
  uint8_t func_id, response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  func_id                       = (completedFunc == NULL ? 0 : 0x03);
  request_buffer[index++]       = mode;
  request_buffer[index++]       = func_id;
  zwapi_set_learn_mode_callback = completedFunc;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_SET_LEARN_MODE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }
  return SL_STATUS_FAIL;
}

sl_status_t zwapi_set_virtual_node_to_learn_mode(
  zwave_node_id_t node, uint8_t mode, void (*completedFunc)(uint8_t, zwave_node_id_t, zwave_node_id_t))
{
  uint8_t func_id, response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  func_id                 = (completedFunc == NULL ? 0 : 0x03);
  zwapi_write_node_id(request_buffer, &index, node);
  request_buffer[index++] = mode;
  request_buffer[index++] = func_id;
  zwapi_set_virtual_node_to_learn_mode_callback = completedFunc;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_SET_VIRTUAL_NODE_TO_LEARN_MODE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_request_node_info(zwave_node_id_t node_id)
{
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  zwapi_write_node_id(request_buffer, &index, node_id);

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_REQUEST_NODE_INFO,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t
  zwapi_send_controller_replication(zwave_node_id_t nodeID,
                                    const uint8_t *pData,
                                    uint8_t dataLength,
                                    uint8_t txOptions,
                                    void (*completedFunc)(uint8_t txStatus))
{
  uint8_t func_id, response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  func_id = (completedFunc == NULL ? 0 : 0x03);

  zwapi_write_node_id(request_buffer, &index, nodeID);
  request_buffer[index++] = dataLength;
  for (uint8_t i = 0; i < dataLength; i++) {
    request_buffer[index++] = pData[i];
  }
  request_buffer[index++]                    = txOptions;
  request_buffer[index++]                    = func_id;
  zwapi_send_controller_replication_callback = completedFunc;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_REPLICATION_SEND_DATA,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

uint8_t zwapi_get_controller_capabilities(void)
{
  uint8_t response_length                   = 0;
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_GET_CONTROLLER_CAPABILITIES,
                                       NULL,
                                       0,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA) {
    return response_buffer[IDX_DATA];
  }

  // Old types of controllers also return a 0 bitmask, but the application
  // should double check if the controller returns no capabilities
  return 0;
}

sl_status_t zwapi_request_network_update(void (*complFunc)(uint8_t))
{
  uint8_t func_id, response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  func_id                                     = (complFunc == NULL ? 0 : 0x03);
  request_buffer[index++]                     = func_id;
  zwapi_request_network_update_callback       = complFunc;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_REQUEST_NETWORK_UPDATE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_replication_receive_complete()
{
  return zwapi_send_command(FUNC_ID_ZW_REPLICATION_COMMAND_COMPLETE, NULL, 0);
}

bool zwapi_is_node_primary_ctrl(void)
{
  return ((zwapi_get_controller_capabilities() & CONTROLLER_IS_SECONDARY) == 0);
}

bool zwapi_is_node_failed(zwave_node_id_t nodeID)
{
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  zwapi_write_node_id(request_buffer, &index, nodeID);

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_IS_FAILED_NODE_ID,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA) {
    return response_buffer[IDX_DATA];
  }

  // Assume the node is not failed if we could not talk to the Z-Wave module
  return false;
}

sl_status_t zwapi_set_max_source_route(uint8_t maxRouteTries)
{
  if (maxRouteTries == 0 || maxRouteTries > 20) {
    return false;
  }

  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  request_buffer[index++]                     = maxRouteTries;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_SET_ROUTING_MAX,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

uint8_t zwapi_get_priority_route(zwave_node_id_t bNodeID, uint8_t *pLastWorkingRoute)
{
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  zwapi_write_node_id(request_buffer, &index, bNodeID);

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_GET_LAST_WORKING_ROUTE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > (IDX_DATA + 6)) {
    index = IDX_DATA;
    zwave_node_id_t response_node_id = zwapi_read_node_id(response_buffer, &index);
    if (bNodeID == response_node_id) {
      memcpy(pLastWorkingRoute, &response_buffer[IDX_DATA + 2], 5);
      return response_buffer[IDX_DATA + 1];
    }
  }
  // FIXME: ZW_PRIORITY_ROUTE_* codes are missing a define for
  // "No Priority Route found for NodeID", which is 0
  return 0;
}

sl_status_t zwapi_set_priority_route(zwave_node_id_t bNodeID,
                                     uint8_t *pLastWorkingRoute)
{
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  zwapi_write_node_id(request_buffer, &index, bNodeID);
  request_buffer[index++]                     = pLastWorkingRoute[0];
  request_buffer[index++]                     = pLastWorkingRoute[1];
  request_buffer[index++]                     = pLastWorkingRoute[2];
  request_buffer[index++]                     = pLastWorkingRoute[3];
  request_buffer[index++]                     = pLastWorkingRoute[4];

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_SET_LAST_WORKING_ROUTE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_get_old_routing_info(zwave_node_id_t bNodeID,
                                       uint8_t *buf,
                                       uint8_t bRemoveBad,
                                       uint8_t bRemoveNonReps)
{
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  zwapi_write_node_id(request_buffer, &index, bNodeID);
  request_buffer[index++]                     = bRemoveBad;
  request_buffer[index++]                     = bRemoveNonReps;
  request_buffer[index++]                     = 0;  // func_id must be set to 0

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_GET_ROUTING_TABLE_LINE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK
      && response_length >= (IDX_DATA + ZW_MAX_NODEMASK_LENGTH)) {
    memcpy(buf, response_buffer + IDX_DATA, ZW_MAX_NODEMASK_LENGTH);
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_enable_suc(uint8_t state, uint8_t capabilities)
{
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  request_buffer[index++]                     = state;
  request_buffer[index++]                     = capabilities;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_ENABLE_SUC,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_add_smartstart_node_to_network(
  uint8_t bMode, const uint8_t *dsk, void (*completedFunc)(LEARN_INFO *))
{
  uint8_t func_id, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0};
  func_id                            = (completedFunc == NULL ? 0 : 0x03);
  request_buffer[index++]            = bMode;
  request_buffer[index++]            = func_id;
  request_buffer[index++]            = dsk[0];
  request_buffer[index++]            = dsk[1];
  request_buffer[index++]            = dsk[2];
  request_buffer[index++]            = dsk[3];
  request_buffer[index++]            = dsk[4];
  request_buffer[index++]            = dsk[5];
  request_buffer[index++]            = dsk[6];
  request_buffer[index++]            = dsk[7];
  zwapi_add_node_to_network_callback = completedFunc;

  return zwapi_send_command(FUNC_ID_ZW_ADD_NODE_TO_NETWORK,
                            request_buffer,
                            index);
}

sl_status_t zwapi_set_virtual_node_application_node_information(
  zwave_node_id_t dstNode,
  uint8_t listening,
  const node_type_t *node_type,
  uint8_t *nodeParm,
  uint8_t parmLength)
{
  uint8_t index                               = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0};
  zwapi_write_node_id(request_buffer, &index, dstNode);
  request_buffer[index++]                     = listening;
  request_buffer[index++]                     = node_type->generic;
  request_buffer[index++]                     = node_type->specific;
  request_buffer[index++]                     = parmLength;

  for (uint8_t i = 0; i < parmLength; i++) {
    request_buffer[index++] = nodeParm[i];
  }

  return zwapi_send_command(FUNC_ID_SERIAL_API_APPL_VIRTUAL_NODE_INFORMATION,
                            request_buffer,
                            index);
}
