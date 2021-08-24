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
 * @file ucl_network_management.h
 * @ingroup ucl_mqtt
 * @brief Handle `ucl/by-unid/<node>/ProtocolController/NetworkManagement` topic
 *
 * Interface between `ucl/by-unid/<node>/ProtocolController/NetworkManagement`
 * MQTT topic and Z-Wave NetworkManagement for adding and removing nodes.
 * Subscribes to `ucl/by-unid/<node>/ProtocolController/NetworkManagement/Write`
 * and handle UCL NetworkManagement requests.
 * Publish UCL NetworkManagement State to
 * `ucl/by-unid/<node>/ProtocolController/NetworkManagement`.
 *
 * @startuml Add Node Example
    box "MQTT Broker" #LightYellow
    participant MQTT
    end box

    box "ZPC" #LightBlue

    participant UclNetworkManagement
    participant NetworkManagement
    end box
    '<i>ucl/by-unid/<gw-node>/</i>\n<i>ProtocolController/NetworkManagement</i>\n{"State": "Idle"}
    note over MQTT: User trigger Add Node
    MQTT                  -> UclNetworkManagement : <i>ucl/by-unid/<gw-node>/</i>\n<i>ProtocolController/</i>\n<i>NetworkManagement/Write</i>\n{"State": "add node", ...}
    UclNetworkManagement  -> NetworkManagement    : zwave_network_management_add_node()
    NetworkManagement     -> UclNetworkManagement : on_state_updated(NM_WAITING_FOR_ADD)
    UclNetworkManagement  -> MQTT                 : <i>ucl/by-unid/<gw-node>/</i>\n<i>ProtocolController/</i>\n<i>NetworkManagement</i>\n{"State": "add node", ...}
    NetworkManagement     -> UclNetworkManagement : on_keys_reported()
    UclNetworkManagement  -> NetworkManagement    : zwave_network_management_keys_set()
    NetworkManagement     -> UclNetworkManagement : on_dsk_report()
    UclNetworkManagement  -> MQTT                 : <i>ucl/by-unid/<gw-node>/</i>\n<i>ProtocolController/</i>\n<i>NetworkManagement</i>\n{"State": "add node",\n<Request DSK>}
    note over MQTT
    User decides to supply DSK
    or Reject the DSK, which
    will end up with a non secure
    node inclusion
    end note
    MQTT                  -> UclNetworkManagement : <i>ucl/by-unid/<gw-node>/</i>\n<i>ProtocolController/</i>\n<i>NetworkManagement/Write</i>\n{"State":"add node"\nDSK or "UserAccept"=false}
    UclNetworkManagement  -> NetworkManagement    : zwave_network_management_dsk_set()
    NetworkManagement     -> UclNetworkManagement : on_node_added()
    UclNetworkManagement  -> MQTT                 : <i>ucl/by-unid/<gw-node>/</i>\n<i>ProtocolController/NetworkManagement</i>\nNetworkManagement {"State": "Idle"}
  @enduml
  @startuml Remove Node Example
    box "MQTT Broker" #LightYellow
    participant MQTT
    end box
    box "ZPC" #LightBlue
    participant UclNetworkManagement
    participant NetworkManagement
    end box

    note over MQTT: User trigger Remove Node
    MQTT                  -> UclNetworkManagement : <i>ucl/by-unid/<gw-node>/</i>\n<i>ProtocolController/</i>\n<i>NetworkManagement/Write</i>\n{"State": "remove node", ...}
    UclNetworkManagement  -> NetworkManagement    : zwave_network_management_remove_node()
    NetworkManagement     -> UclNetworkManagement : on_state_updated(NM_WAITING_FOR_NODE_REMOVAL)
    UclNetworkManagement  -> MQTT                 : <i>ucl/by-unid/<gw-node>/</i>\n<i>ProtocolController/</i>\n<i>NetworkManagement</i>\n{"State": "remove node", ...}
    ... Waiting for node to be removed ...
    NetworkManagement     -> UclNetworkManagement : on_state_updated(NM_IDLE)
    UclNetworkManagement  -> MQTT                 : <i>ucl/by-unid/<gw-node>/</i>\n<i>ProtocolController/</i>\n<i>NetworkManagement</i>\n{"State": "idle", ...}
  @enduml
  @startuml Remove Offline Node Example
    box "MQTT Broker" #LightYellow
    participant MQTT
    end box
    box "ZPC" #LightBlue
    participant UclNodeState
    participant UclNetworkManagement
    participant NetworkManagement
    end box

    note over MQTT: User trigger Remove Offline Node
    MQTT                  -> UclNodeState         : <i>ucl/by-unid/<offline-node-unid>/State/Commands/OfflineRemove</i>\n"
    UclNodeState          -> UclNetworkManagement : ucl_network_management_remove_offline_node(node_id)
    UclNetworkManagement  -> NetworkManagement    : zwave_network_management_remove_failed(node_id)
    NetworkManagement     -> UclNetworkManagement : on_state_updated(NM_WAITING_FOR_FAILED_NODE_REMOVAL)
    UclNetworkManagement  -> MQTT                 : <i>ucl/by-unid/<gw-node>/</i>\n<i>ProtocolController/</i>\n<i>NetworkManagement</i>\n{"State": "remove node", ...}
    ... Waiting for offfline node to be removed ...
    NetworkManagement     -> UclNetworkManagement : on_state_updated(NM_IDLE)
    UclNetworkManagement  -> MQTT                 : <i>ucl/by-unid/<gw-node>/</i>\n<i>ProtocolController/</i>\n<i>NetworkManagement</i>\n{"State": "idle", ...}
  @enduml
 * @{
 */

#ifndef UCL_NETWORK_MANAGEMENT_H
#define UCL_NETWORK_MANAGEMENT_H
#include "process.h"
#include "zwave_utils.h"
#include "sl_status.h"

// Timeout before triggering remove failed node process
#define UCL_NM_REMOVE_FAILED_NODE_TIMEOUT_DEFAULT 1000

/**
 * @brief Name the of @ref contiki process for the Ucl Network Management.
 *
 * This is used to register the name of the Ucl Network Management Process.
 */
PROCESS_NAME(ucl_network_management_process);

/**
 * @brief Initiates the removal process of an offline node
 *
 * @param node_id   The NodeID of the node to be removed.
 * @returns SL_STATUS_OK if the removal of the node has been initiated,
 *          SL_STATUS_FAIL if an error occureed.
 */
sl_status_t ucl_network_management_remove_offline_node(zwave_node_id_t node_id);

/**
 * @brief Initiate the request node neighbor discovery
 * @param node_id The NodeID of the node for which the node neighbor discovery is requested.
 * @returns SL_STATUS_OK if he node neighbor discovery has been requested,
 *          SL_STATUS_FAIL if an error occureed.
 */ 
sl_status_t ucl_network_management_request_node_neighbor_update(zwave_node_id_t node_id);

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //UCL_NETWORK_MANAGEMENT_H
/** @} end ucl_network_management */
