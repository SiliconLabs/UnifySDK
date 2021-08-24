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

#include "zwapi_utils.h"
#include "zwapi_protocol_basis.h"

zwave_node_id_t zwapi_read_node_id(uint8_t *pData, uint8_t *index)
{
  zwave_node_id_t node_id = 0;
  if (NODEID_16BITS == zwapi_get_node_id_basetype()) {
    node_id |= pData[(*index)++] << 8;
  }
  node_id |= pData[(*index)++];
  return node_id;
}

void zwapi_write_node_id(uint8_t *pData, uint8_t *index, zwave_node_id_t node_id)
{
  if (NODEID_16BITS == zwapi_get_node_id_basetype()) {
    pData[(*index)++] = node_id >> 8;
  }
  pData[(*index)++] = node_id & 0xFF;
}
