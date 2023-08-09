/***************************************************************************//**
 * @file sl_cluster_service_gen.c
 * @brief Cluster Service generated code.
 *******************************************************************************
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

// This template contains a pre-generated ZCL command specific init block of the
// service functions.

#include PLATFORM_HEADER
#include "sl_service_function.h"
#include "af.h"


extern uint32_t emberAfBasicClusterServerCommandParse(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfOtaClusterServerCommandParse(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
/*
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
extern uint32_t emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                                sl_service_function_context_t *context);
*/

const sl_service_function_entry_t sli_cluster_service_entries[] = {
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0000, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_SERVER << 16)), emberAfBasicClusterServerCommandParse },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0019, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_SERVER << 16)), emberAfOtaClusterServerCommandParse }};
 /*
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0003, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_SERVER << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0003, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_CLIENT << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0004, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_SERVER << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0004, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_CLIENT << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0006, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_SERVER << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0006, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_CLIENT << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0008, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_SERVER << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0008, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_CLIENT << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0020, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_CLIENT << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0101, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_SERVER << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0101, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_CLIENT << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0201, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_SERVER << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0201, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_CLIENT << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0300, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_SERVER << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0300, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_CLIENT << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0406, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_SERVER << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0406, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_CLIENT << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0500, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_SERVER << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0500, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_CLIENT << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0501, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_SERVER << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0501, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_CLIENT << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0502, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_SERVER << 16)), emberAfClusterServiceCallback },
  { SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND, 0x0502, (NOT_MFG_SPECIFIC | (SL_CLUSTER_SERVICE_SIDE_CLIENT << 16)), emberAfClusterServiceCallback },
  };
*/
