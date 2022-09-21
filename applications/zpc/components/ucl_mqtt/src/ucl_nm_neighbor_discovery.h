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
 * @defgroup ucl_nm_neighbor_discovery UCL Network Management Node Neighbor Discovery
 * @ingroup ucl_mqtt
 * @brief This submodule provides APIs to trigger a node neighbor discovery
 *
 * @{
 */

#include "zwave_utils.h"

#ifndef UCL_NM_NEIGHBOR_DISCOVERY_H
#define UCL_NM_NEIGHBOR_DISCOVERY_H

/// setting a 5 minutes timeout to tackle a defect in protocol
/// which may not send a callback message to the host application.
/// The value is estimated considering that neigbour discovery
/// is triggered in a large network (i.e., 232 nodes)
#define UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_DEFAULT_TIMEOUT 3000000

/**
 * @brief All possible values for Node Request Node Neighbor Discovery Statuses
 *        \ref zwapi_request_neighbor_update() callback status values
 *         i.e., UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_STARTED = 0x21
 */
typedef enum {
  UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_STARTED       = 0x21,
  UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_COMPLETED     = 0x22,
  UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_FAILED        = 0x23,
  UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_NOT_SUPPORTED = 255,
} ucl_network_management_neighbor_discovery_t;

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Trigger the request node neighbor discovery
 * @param node_id The NodeID of the node for which the node neighbor discovery is requested.
 */
void ucl_nm_trigger_node_neighbor_update(zwave_node_id_t node_id);

/// intialize ucl network management node neighbor discovery submodule
void ucl_nm_neighbor_discovery_init();

#ifdef __cplusplus
}
#endif

#endif  //UCL_NM_NEIGHBOR_DISCOVERY_H
/** @} end ucl_nm_neighbor_discovery */
