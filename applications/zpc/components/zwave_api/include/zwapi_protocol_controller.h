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
 * @file zwapi_protocol_controller.h
 * @brief Z-Wave API for Z-Wave controllers (network setup and maintenance)
 */

#ifndef ZWAPI_PROTOCOL_CONTROLLER_H
#define ZWAPI_PROTOCOL_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "zwapi_protocol_transport.h"  //We need zwapi_tx_report_t here

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup ZWAPI_CONTROLLER Controller API
 * @ingroup zwave_api
 * @brief Functions in the Z-Wave controller API
 *
 * Functions in this module are generally used by Z-Wave controllers.
 * @{
 */

/// @name zwapi_add_node_to_network() mode values
///@{
#define ADD_NODE_ANY         1
#define ADD_NODE_CONTROLLER  2
#define ADD_NODE_END_NODE    3
#define ADD_NODE_EXISTING    4
#define ADD_NODE_STOP        5
#define ADD_NODE_STOP_FAILED 6
#define ADD_NODE_ANY_S2      7
#define ADD_NODE_HOME_ID     8
#define ADD_NODE_SMART_START 9
///@}

/// zwapi_add_node_to_network() mode mask
#define ADD_NODE_MODE_MASK 0x0F

/// @name zwapi_add_node_to_network() option values
///@{
#define ADD_NODE_OPTION_NORMAL_POWER 0x80
#define ADD_NODE_OPTION_NETWORK_WIDE 0x40
#define ADD_NODE_OPTION_LONG_RANGE   0x20
///@}

/// @name zwapi_add_node_to_network() callback status values
///@{
#define ADD_NODE_STATUS_LEARN_READY       1
#define ADD_NODE_STATUS_NODE_FOUND        2
#define ADD_NODE_STATUS_ADDING_END_NODE   3
#define ADD_NODE_STATUS_ADDING_CONTROLLER 4
#define ADD_NODE_STATUS_PROTOCOL_DONE     5
#define ADD_NODE_STATUS_DONE              6
#define ADD_NODE_STATUS_FAILED            7
#define ADD_NODE_STATUS_SECURITY_FAILED   9
#define ADD_NODE_STATUS_INTERVIEW_DONE    10
#define ADD_NODE_STATUS_NOT_PRIMARY       0x23
///@}

/// @name zwapi_remove_node_from_network() mode values
///@{
#define REMOVE_NODE_ANY        ADD_NODE_ANY
#define REMOVE_NODE_CONTROLLER ADD_NODE_CONTROLLER
#define REMOVE_NODE_END_NODE   ADD_NODE_END_NODE
#define REMOVE_NODE_STOP       ADD_NODE_STOP
///@}

/// @name zwapi_remove_node_from_network() mode mask
#define REMOVE_NODE_MODE_MASK ADD_NODE_MODE_MASK

/// @name zwapi_remove_node_from_network() option values
///@{
#define REMOVE_NODE_OPTION_NORMAL_POWER ADD_NODE_OPTION_NORMAL_POWER
#define REMOVE_NODE_OPTION_NETWORK_WIDE ADD_NODE_OPTION_NETWORK_WIDE
///@}

/// @name zwapi_remove_node_from_network() callback status values
///@{
#define REMOVE_NODE_STATUS_LEARN_READY         ADD_NODE_STATUS_LEARN_READY
#define REMOVE_NODE_STATUS_NODE_FOUND          ADD_NODE_STATUS_NODE_FOUND
#define REMOVE_NODE_STATUS_REMOVING_END_NODE   ADD_NODE_STATUS_ADDING_END_NODE
#define REMOVE_NODE_STATUS_REMOVING_CONTROLLER ADD_NODE_STATUS_ADDING_CONTROLLER
#define REMOVE_NODE_STATUS_DONE                ADD_NODE_STATUS_DONE
#define REMOVE_NODE_STATUS_FAILED              ADD_NODE_STATUS_FAILED
///@}

/// @name zwapi_create_new_primary_ctrl() mode values
///@{
#define CREATE_PRIMARY_START       ADD_NODE_CONTROLLER
#define CREATE_PRIMARY_STOP        ADD_NODE_STOP
#define CREATE_PRIMARY_STOP_FAILED ADD_NODE_STOP_FAILED
///@}

/// @name zwapi_transfer_primary_ctrl_role() mode values
///@{
#define CONTROLLER_CHANGE_START       ADD_NODE_CONTROLLER
#define CONTROLLER_CHANGE_STOP        ADD_NODE_STOP
#define CONTROLLER_CHANGE_STOP_FAILED ADD_NODE_STOP_FAILED
///@}

/// @name zwapi_set_learn_mode() mode values
///@{
#define LEARN_MODE_DISABLE      0x00
#define LEARN_MODE_DIRECT_RANGE 0x01
#define LEARN_MODE_NWI          0x81
#define LEARN_MODE_NWE          0x83
#define LEARN_MODE_SMART_START  0x84
///@}

/// zwapi_set_learn_mode() return interview status bit
#define ZW_LEARN_MODE_RETURN_INTERVIEW_STATUS 0x01

/// @name zwapi_set_learn_mode() callback status values
///@{
#define LEARN_MODE_STARTED             ADD_NODE_STATUS_LEARN_READY
#define LEARN_MODE_DONE                ADD_NODE_STATUS_DONE
#define LEARN_MODE_FAILED              ADD_NODE_STATUS_FAILED
#define LEARN_MODE_INTERVIEW_COMPLETED 0x0A
///@}

/// @name zwapi_request_neighbor_update() callback status values
///@{
#define REQUEST_NEIGHBOR_UPDATE_STARTED 0x21
#define REQUEST_NEIGHBOR_UPDATE_DONE    0x22
#define REQUEST_NEIGHBOR_UPDATE_FAILED  0x23
///@}

/// @name application_controller_update_function() callback status values
///@{
#define UPDATE_STATE_SUC_ID                                  0x10
#define UPDATE_STATE_DELETE_DONE                             0x20
#define UPDATE_STATE_NEW_ID_ASSIGNED                         0x40
#define UPDATE_STATE_ROUTING_PENDING                         0x80
#define UPDATE_STATE_NODE_INFO_REQ_FAILED                    0x81
#define UPDATE_STATE_NODE_INFO_REQ_DONE                      0x82
#define UPDATE_STATE_NOP_POWER_RECEIVED                      0x83
#define UPDATE_STATE_NODE_INFO_RECEIVED                      0x84
#define UPDATE_STATE_NODE_INFO_FOREIGN_HOMEID_RECEIVED       0x85
#define UPDATE_STATE_INCLUDED_NODE_INFO_RECEIVED             0x86
#define UPDATE_STATE_NODE_INFO_SMARTSTART_HOMEID_RECEIVED_LR 0x87
///@}

/// @name zwapi_remove_failed_node() and zwapi_replace_failed_node() return value and bit flags
///@{
/// The removing/replacing failed node process started
#define ZW_FAILED_NODE_REMOVE_STARTED 0x00
/// The removing process was aborted because the controller is not the primary
/// one
#define ZW_NOT_PRIMARY_CONTROLLER 0x02
/// The removing process was aborted because no call back function is used
#define ZW_NO_CALLBACK_FUNCTION 0x04
/// The removing process aborted because the node was node found
#define ZW_FAILED_NODE_NOT_FOUND 0x08
/// The removing process is busy
#define ZW_FAILED_NODE_REMOVE_PROCESS_BUSY 0x10
/// The removing process could not be started
#define ZW_FAILED_NODE_REMOVE_FAIL 0x20
///@}

/// @name zwapi_remove_failed_node() and zwapi_replace_failed_node() callback status values
///@{
/// The node is working properly (removed from the failed nodes list)
#define ZW_NODE_OK 0
/// The failed node was removed from the failed nodes list
#define ZW_FAILED_NODE_REMOVED 1
/// The failed node was not removed from the failing nodes list
#define ZW_FAILED_NODE_NOT_REMOVED 2
///@}

/// @name zwapi_replace_failed_node() callback status definitions
///@{
/// The failed node are ready to be replaced and controller is ready to add new
/// node with nodeID of the failed node
#define ZW_FAILED_NODE_REPLACE 3
/// The failed node has been replaced
#define ZW_FAILED_NODE_REPLACE_DONE 4
/// The failed node has not been replaced
#define ZW_FAILED_NODE_REPLACE_FAILED 5
///@}

/// @name zwapi_request_network_update() callback values
///@{
#define ZW_SUC_UPDATE_DONE     0x00
#define ZW_SUC_UPDATE_ABORT    0x01
#define ZW_SUC_UPDATE_WAIT     0x02
#define ZW_SUC_UPDATE_DISABLED 0x03
#define ZW_SUC_UPDATE_OVERFLOW 0x04
#define ZW_SUC_SET_SUCCEEDED   0x05
#define ZW_SUC_SET_FAILED      0x06
///@}

/// SUC capabilities used in zwapi_set_suc_node_id()
#define ZW_SUC_FUNC_NODEID_SERVER 0x01

/// @name zwapi_get_controller_capabilities() bit flag values
///@{
#define CONTROLLER_IS_SECONDARY          0x01
#define CONTROLLER_ON_OTHER_NETWORK      0x02
#define CONTROLLER_NODEID_SERVER_PRESENT 0x04
#define CONTROLLER_IS_REAL_PRIMARY       0x08
#define CONTROLLER_IS_SUC                0x10
#define NO_NODES_INCLUDED                0x20
///@}

/// @name Z-Wave RF speed definitions
///@{
#define ZW_RF_SPEED_NONE 0x0000
#define ZW_RF_SPEED_9600 0x0001
#define ZW_RF_SPEED_40K  0x0002
#define ZW_RF_SPEED_100K 0x0003
#define ZW_RF_SPEED_MASK 0x0007
///@}

/// @name zwapi_get_old_routing_info() options
///@{
#define GET_ROUTING_INFO_REMOVE_BAD      0x80
#define GET_ROUTING_INFO_REMOVE_NON_REPS 0x40
#define ZW_GET_ROUTING_INFO_ANY          ZW_RF_SPEED_NONE
#define ZW_GET_ROUTING_INFO_9600         ZW_RF_SPEED_9600
#define ZW_GET_ROUTING_INFO_40K          ZW_RF_SPEED_40K
#define ZW_GET_ROUTING_INFO_100K         ZW_RF_SPEED_100K
#define ZW_GET_ROUTING_INFO_SPEED_MASK   ZW_RF_SPEED_MASK
///@}

/// @name zwapi_get_priority_route() and zwapi_set_priority_route() route structure definitions
///@{
#define ROUTECACHE_LINE_CONF_SIZE 1
#define ROUTECACHE_LINE_SIZE      (MAX_REPEATERS + ROUTECACHE_LINE_CONF_SIZE)
///@}

/// @name PriorityRoute index definitions
///@{
#define ROUTECACHE_LINE_REPEATER_0_INDEX 0
#define ROUTECACHE_LINE_REPEATER_1_INDEX 1
#define ROUTECACHE_LINE_REPEATER_2_INDEX 2
#define ROUTECACHE_LINE_REPEATER_3_INDEX 3
#define ROUTECACHE_LINE_CONF_INDEX       4
///@}

/// @name zwapi_get_priority_route() and zwapi_set_priority_route() speed definitions
///@{
#define ZW_PRIORITY_ROUTE_SPEED_9600 ZW_RF_SPEED_9600
#define ZW_PRIORITY_ROUTE_SPEED_40K  ZW_RF_SPEED_40K
#define ZW_PRIORITY_ROUTE_SPEED_100K ZW_RF_SPEED_100K
///@}

/// @name zwapi_get_priority_route() function return values
///@{
/// Route returned is a Application defined Priority Route - APP_PR
#define ZW_PRIORITY_ROUTE_APP_PR 0x10
/// Route returned is a Last Working Route - ZW_LWR
#define ZW_PRIORITY_ROUTE_ZW_LWR 0x01
/// Route returned is a Next to Last Working Route - ZW_NLWR
#define ZW_PRIORITY_ROUTE_ZW_NLWR 0x02
///@}

/// @name virtual End Nodes inclusion/exclusion defines
///@{
/// Are returned as callback parameter when callback, setup with
/// zwapi_set_virtual_node_to_learn_mode(), is called during inclusion/exclusion
/// process
#define ASSIGN_COMPLETE          0x00
#define ASSIGN_NODEID_DONE       0x01  ///< Node ID have been assigned
#define ASSIGN_RANGE_INFO_UPDATE 0x02  ///< Node is doing Neighbor discovery
///@}

/// @name zwapi_set_virtual_node_to_learn_mode() mode values
///@{
/// Disable End Node Learn Mode (disable possibility to add/remove Virtual End
/// Nodes) Allowed when bridge is a primary controller, an inclusion controller
/// or a secondary controller
#define VIRTUAL_END_NODE_LEARN_MODE_DISABLE 0x00
/// Enable End Node Learn Mode - Enable possibility for including/excluding a Virtual
/// End Node by an external primary/inclusion controller. Allowed when bridge
/// is an inclusion controller or a secondary controller
#define VIRTUAL_END_NODE_LEARN_MODE_ENABLE 0x01
/// Add new Virtual End Node if possible. Allowed when bridge is a primary or
/// an inclusion controller
#define VIRTUAL_END_NODE_LEARN_MODE_ADD 0x02
/// Remove existing Virtual End Node. Allowed when bridge is a primary or an
/// inclusion controller
#define VIRTUAL_END_NODE_LEARN_MODE_REMOVE 0x03
///@}

/// @name Bit mask for multiDestsOffset_NodeMaskLen byte in the ZW_MULTI_DEST structure
///@{
/// Mask masking the number of bytes in multiDestNodeMask array. Valid value
/// range [0 - 29]
#define MULTI_DEST_MASK_LEN_MASK 0x1F
/// Mask masking the nodeID-1 represented by the first bit in the first byte in
/// the multiDestNodeMask. Valid value range [0, 32, 64, 96...]
#define MULTI_DEST_MASK_OFFSET_MASK 0xE0
///@}

typedef struct _ZW_MULTI_DEST_ {
  /// - bit 5-7 states the nodeId-1 (offset 0, 32, 64, 96... - translates into
  ///   respectively nodeID 1, 33, 65, 97...), which is represented by the
  ///   first bit in the multiDestsNodeMask.
  /// - bit 0-4 is the size of the multicast nodemask.
  uint8_t multiDestsOffset_NodeMaskLen;
  /// first byte in the multicast nodemask containing local nodes which are to
  /// receive the received multicast frame. Following bytes can be accessed
  /// *(&multiDestsNodeMask + n) where n is index.
  uint8_t multiDestsNodeMask;
} ZW_MULTI_DEST;

/// Learn mode state information passed by the call back function
typedef struct LEARN_INFO {
  uint8_t bStatus;          ///< Status of learn mode
  zwave_node_id_t bSource;  ///< Node id of the node that send node info
  uint8_t *pCmd;            ///< Pointer to Application Node information
  uint8_t bLen;             ///< Node info length
} LEARN_INFO;

/// Node info stored within the non-volatile memory.
/// These are the first (protocol part) payload bytes from the Node Information
/// frame
typedef struct zwapi_node_info_header {
  uint8_t capability;  ///< Network capabilities
  uint8_t security;    ///< Network security
  uint8_t reserved;
  node_type_t
    node_type;  ///< Basic, Generic and Specific Device types - Basic is generated...
} zwapi_node_info_header_t;

/**
 * @brief Send command completed to primary controller.
 *
 * @note Called in replication mode when a command from the sender has been
 * processed.
 *
 * aka ZW_ReplicationReceiveComplete
 */
sl_status_t zwapi_replication_receive_complete(void);

/**
 * @brief Send the payload and expect the receiver to respond with a
 * command complete message.
 *
 * @param destNodeID destination node ID ( only single cast allowed)
 * @param pData data buffer pointer
 * @param dataLength data buffer length
 * @param txOptions transmit option flags
 * @param completedFunc transmit completed call back function
 *
 * @returns sl_status_t
 *
 * @note Used when the controller is replication mode
 *
 * aka ZW_ReplicationSend
 */
sl_status_t zwapi_send_controller_replication(zwave_node_id_t destNodeID,
                                              const uint8_t *pData,
                                              uint8_t dataLength,
                                              uint8_t txOptions,
                                              void (*completedFunc)(uint8_t));

/**
 * @brief Copy the Node's current protocol information from the non-volatile memory.
 *
 * @param node_id it the Node ID
 * @param node_info_header is the Node info buffer
 *
 * aka ZW_GetNodeProtocolInfo
 */
sl_status_t zwapi_get_protocol_info(zwave_node_id_t node_id,
                                    zwapi_node_info_header_t *node_info_header);
/**
 * @brief Assign static return routes within a Routing End Node.
 *
 * @param bSrcNodeID is the Routing End Node ID
 * @param bDstNodeID is the Route destination Node ID
 * @param completedFunc is the Callback function called when done
 * @returns returns  true if assign return route was initiated, or false if a
 * return route assign/delete is already active
 *
 * Calculate the shortest transport routes from the Routing End Node to the
 * route destination node and transmit the return routes to the Routing End
 * Node.
 *
 * aka ZW_AssignReturnRoute
 */
sl_status_t zwapi_assign_return_route(zwave_node_id_t bSrcNodeID,
                                      zwave_node_id_t bDstNodeID,
                                      void (*completedFunc)(uint8_t bStatus));

/**
 * @brief Delete static return routes within a Routing End Node.
 *
 * @param nodeID is the node id of the Routing End Node
 * @param completedFunc is the Completion handler. Its parameter bStatus is the
 * the Transmit complete status.
 * @returns returns true if delete return routes was initiated, or  false if not
 *
 * Transmits "NULL" routes to the Routing End Node.
 *
 * aka ZW_DeleteReturnRoute
 */
sl_status_t zwapi_delete_return_route(zwave_node_id_t nodeID,
                                      void (*completedFunc)(uint8_t bStatus));

/**
 * @brief Remove all Nodes and timers from the EEPROM memory.
 *
 * @param completedFunc is the Command completed call back function
 *
 * aka ZW_SetDefault
 */
sl_status_t zwapi_set_default(void (*completedFunc)(void));

/**
 * @brief Check if a node is in the failed nodes table.
 *
 * @param nodeID is the node to be checked if it is failed or not
 *
 * @returns returns  true if node in failed node table, else false
 *
 * aka ZW_isFailedNode
 */
bool zwapi_is_node_failed(zwave_node_id_t nodeID);

/**
 * @brief Remove a node from the failed node list, if it already exist.
 *
 * @param NodeID failed node ID that will be removed from the network.
 * @param completedFunc callback function to be called when the remove process
 * has ended.
 * @returns @ref ZW_FAILED_NODE_REMOVE_STARTED if the removing process started
 * successfully
 * @returns One or more of the following bit flags If the removing process can
 * not be started:
 *  - @ref ZW_NOT_PRIMARY_CONTROLLER The removing process was aborted because
 *    the controller is not the primary one
 *  - @ref ZW_NO_CALLBACK_FUNCTION The removing process was aborted because no
 *    call back function is used
 *  - @ref ZW_FAILED_NODE_NOT_FOUND The removing process aborted because the
 *    node was node found
 *  - @ref ZW_FAILED_NODE_REMOVE_PROCESS_BUSY The removing process is busy
 *
 * @note A call back function should be provided otherwise the function will
 * return without removing the node.

 * The completedFunc parameter values are:
 *  - @ref ZW_NODE_OK The node is working properly (removed from the failed
 *    nodes list )
 *  - @ref ZW_FAILED_NODE_REMOVED The failed node was removed from the failed
 *    nodes list
 *  - @ref ZW_FAILED_NODE_NOT_REMOVED The failed node was not removed
 *
 * aka ZW_RemoveFailedNode
 */
uint8_t zwapi_remove_failed_node(zwave_node_id_t NodeID,
                                 void (*completedFunc)(uint8_t));

/**
 * @brief Replace a node from the failed node list.
 *
 * @param bNodeID is the nodeID on the failed node to replace
 * @param bNormalPower should be set to true if the replacement is included with
 * normal power.
 * @param completedFunc callback function to be called when the remove process
 * has ended.
 *
 * @note A call back function should be provided otherwise the function will
 * return without replacing the node.
 *
 * @returns @ref ZW_FAILED_NODE_REMOVE_STARTED  The replacing process started
 * and now the new node must emit its node information frame to start the assign
 * process.
 *
 * @return One or more of the following flags if the replace process can not be started:
 *  - @ref ZW_NOT_PRIMARY_CONTROLLER The replacing process was aborted because
 *    the controller is not the primary controller.
 *  - @ref ZW_NO_CALLBACK_FUNCTION The replacing process was aborted because no
 *    call back function is used.
 *  - @ref ZW_FAILED_NODE_NOT_FOUND The replacing process aborted because the
 *    node was node found.
 *  - @ref ZW_FAILED_NODE_REMOVE_PROCESS_BUSY The replacing process is busy.
 *  - @ref ZW_FAILED_NODE_REMOVE_FAIL The replacing process could not be
 *    started.
 *
 * The completedFunc parameter values are:
 *  - @ref ZW_NODE_OK The node is working properly (removed from the failed
 *    nodes list).
 *  - @ref ZW_FAILED_NODE_REPLACE The failed node are ready to be replaced and
 *    controller is ready to add new node with nodeID of the failed node.
 *  - @ref ZW_FAILED_NODE_REPLACE_DONE The failed node has been replaced.
 *  - @ref ZW_FAILED_NODE_REPLACE_FAILED The failed node has not been replaced.
 *
 * aka ZW_ReplaceFailedNode
 */
uint8_t zwapi_replace_failed_node(zwave_node_id_t bNodeID,
                                  bool bNormalPower,
                                  void (*completedFunc)(uint8_t));

/**
 * @brief Enable/disable the Static Update Controller functionality in a
 * specified static controller
 *
 * @param nodeID the node ID of the static controller to be a SUC
 * @param SUCState shall be set true to enable SUC, and false to disable SUC
 * @param bTxOption shall be set to true if to use low power transmission, and
 * false for normal TX power.
 * @param bCapabilities capabilities of the new SUC.
 * @param completedFunc call back function
 * @returns SL_STATUS_OK if success.
 * @returns SL_STATUS_FAIL if failed (e.g. if the target is not a static
 * controller)
 *
 * aka ZW_SetSUCNodeID
 */
sl_status_t zwapi_set_suc_node_id(zwave_node_id_t nodeID,
                                  uint8_t SUCState,
                                  uint8_t bTxOption,
                                  uint8_t bCapabilities,
                                  void (*completedFunc)(uint8_t txStatus));

/**
 * @brief Get the currently registered SUC node ID.
 *
 * @returns returns the  SUC node ID, and ZERO if no SUC available
 *
 * aka ZW_GetSUCNodeID
 */
zwave_node_id_t zwapi_get_suc_node_id(void);

/**
 * @brief Request network topology updates from the SUC/SIS node.
 *
 * @param completedFunc Transmit complete call back. The parameter txStatus values:
 * - @ref ZW_SUC_UPDATE_DONE The update process succeeded.
 * - @ref ZW_SUC_UPDATE_ABORT The update process aborted because of an error.
 * - @ref ZW_SUC_UPDATE_WAIT The SUC node is busy.
 * - @ref ZW_SUC_UPDATE_DISABLED The SUC functionality is disabled.
 * - @ref ZW_SUC_UPDATE_OVERFLOW The controller requested an update after more
 *   than 64 changes have occurred in the network. The update information is
 *   then out of date in respect to that controller. In this situation the
 *   controller have to make a replication before trying to request any new
 *   network updates.
 * @returns SL_STATUS_OK if the updating process is started
 * @return SL_STATUS_FAIL if the requesting controller is the SUC node or the
 * SUC node is unknown.
 *
 * The update is done on protocol level and any changes are notified to the
 * application by calling the @ref application_controller_update_function).
 *
 * Secondary controllers can only use this call when a SUC is present in the
 * network. All controllers can use this call in case a SUC ID Server (SIS) is
 * available.
 *
 * Routing End Node can only use this call, when a SUC is present in the network.
 * In case the Routing End Node has called ZW_RequestNewRouteDestinations prior to
 * zwapi_request_network_update(), then Return Routes for the destinations
 * specified by the application in ZW_RequestNewRouteDestinations will be
 * updated along with the SUC Return Route.
 *
 * @note The SUC can only handle one network update at a time, so care should be
 * taken not to have all the controllers in the network ask for updates at the
 * same time.
 *
 * @warning This API call will generate a lot of network activity that will use
 * bandwidth and stress the SUC in the network. Therefore, network updates
 * should be requested as seldom as possible and never more often that once
 * every hour from a controller.
 *
 * Timeout: 65 s. Exception recovery: Resume normal operation, no recovery needed.
 *
 * Tx: {REQ | 0x53 | funcID}
 *
 * Rx: {RES | 0x53 | retVal}
 *
 * Rx: {REQ | 0x53 | funcID | txStatus}
 *
 * aka ZW_RequestNetWorkUpdate
 */
sl_status_t
  zwapi_request_network_update(void (*completedFunc)(uint8_t txStatus));

/**
 * @brief Assign static return routes within a Routing End Node.
 *
 * @param bSrcNodeID is the Routing End Node NodeID
 * @param completedFunc is the Completion handler (the parameter bStatus is the
 * Transmit complete status)
 * @returns SL_STATUS_OK if assign SUC return route was initiated.
 * @returns SL_STATUS_FAIL if a return route assign/delete is already active.
 *
 * Calculate the shortest transport routes to a Routing End Node
 * from the Static Update Controller (SUC) Node and
 * transmit the return routes to the Routing End Node.
 *
 * aka ZW_AssignSUCReturnRoute
 */
sl_status_t
  zwapi_assign_suc_return_route(zwave_node_id_t bSrcNodeID,
                                void (*completedFunc)(uint8_t bStatus));

/**
 * @brief Delete the (Static Update Controller -SUC-) static return routes
 * within a Routing End Node.
 *
 * @param bNodeID is the Routing End Node NodeID
 * @param completedFunc  is the Completion handler (the parameter bStatus is the
 * Transmit complete status)
 * @returns SL_STATUS_OK if delete SUC return route was initiated
 * @returns SL_STATUS_FAIL if a return route assign/delete is already active.
 *
 * Transmit "NULL" routes to the Routing End Node.
 *
 * aka ZW_DeleteSUCReturnRoute
 */
sl_status_t
  zwapi_delete_suc_return_route(zwave_node_id_t bNodeID,
                                void (*completedFunc)(uint8_t bStatus));

/**
 * @brief Start neighbor discovery for bNodeID.
 *
 * @param bNodeID is the ID of node the controller wants to get neighbors for.
 * @param completedFunc is the  Callback function
 *
 * @returns SL_STATUS_OK (neighbor discovery started) or SL_STATUS_FAIL
 *
 * aka ZW_RequestNodeNeighborUpdate
 */
sl_status_t zwapi_request_neighbor_update(zwave_node_id_t bNodeID,
                                          void (*completedFunc)(uint8_t));

/**
 * @brief Query whether the controller is a primary controller or a secondary
 * controller in the network.
 *
 * @returns true when the controller is a primary controller in the network
 * @returns false when the controller is a secondary controller in the network.
 *
 * aka ZW_IsPrimaryCtrl
 */
bool zwapi_is_node_primary_ctrl(void);

/**
 * @brief Create a new primary controller.
 *
 * @param bMode one of:
 *  - @ref CREATE_PRIMARY_START Start the creation of a new primary
 *  - @ref CREATE_PRIMARY_STOP Stop the creation of a new primary
 *  - @ref CREATE_PRIMARY_STOP_FAILED Report that the replication failed
 *  - @ref ADD_NODE_OPTION_NORMAL_POWER Set this flag in bMode for High Power
 *    inclusion.
 * @param completedFunc Callback function.
 * @returns SL_STATUS_OK or SL_STATUS_FAIL.
 *
 * aka ZW_CreateNewPrimaryCtrl
 */
sl_status_t zwapi_create_new_primary_ctrl(uint8_t bMode,
                                          void (*completedFunc)(LEARN_INFO *));

/**
 * @brief Get capabilities of a controller.
 *
 * @returns returns  a bitmask where following bits are defined:
 *   = CONTROLLER_IS_SECONDARY :
 *      If bit is set then the controller is a secondary controller \n
 *   = CONTROLLER_ON_OTHER_NETWORK :
 *      If this bit is set then this controller is not using its build-in home ID \n
 *   = CONTROLLER_IS_SUC :
 *      If this bit is set then this controller is a SUC \n
 *   = CONTROLLER_NODEID_SERVER_PRESENT :
 *      If this bit is set then there is a SUC ID server (SIS)
 *      in the network and this controller can therefore
 *      include/exclude nodes in the network (inclusion controller). \n
 *   = CONTROLLER_IS_REAL_PRIMARY :
 *   = CONTROLLER_IS_SUC :
 *   = NO_NODES_INCLUDED :
 *
 * aka ZW_GetControllerCapabilities
 */
uint8_t zwapi_get_controller_capabilities(void);

/**
 * @brief Request node information from a node.
 *
 * @param node_id node ID of the node to request node info from.
 * @ref application_controller_update_function will be called with
 * @ref UPDATE_STATE_NODE_INFO_RECEIVED as status together with the received
 * nodeinformation.
 * @returns SL_STATUS_OK or SL_STATUS_FAIL.
 *
 * aka ZW_RequestNodeInfo
 */
sl_status_t zwapi_request_node_info(zwave_node_id_t node_id);

/**
 * @brief Enable/disable Z-Wave Learn Mode.
 *
 * @param mode Learn Mode intent.
 * @param completedFunc callback function which is called with the learn mode
 * results.
 *
 * Learn Mode Intent values can be:
 * - 0x00: Stop learn mode (LEARN_MODE_DISABLE)
 * - 0x01: Start direct range inclusion/exclusion (LEARN_MODE_DIRECT_RANGE)
 * - 0x02: Start learn mode expecting NWI (LEARN_MODE_NWI)
 * - 0x03: Start learn mode expecting NWE (LEARN_MODE_NWE)
 *
 * @returns SL_STATUS_OK    if the requested operation was accepted and has started
 * @returns SL_STATUS_FAIL  if the requested operation has not been started
 *
 * aka ZW_SetLearnMode
 */
sl_status_t zwapi_set_learn_mode(uint8_t mode,
                                 void (*completedFunc)(LEARN_INFO *));

/**
 * @brief Add any type of node to the network.
 *
 * @param bMode network inclusion mode, one of:
 *  - @ref ADD_NODE_ANY Add any node to the network
 *  - @ref ADD_NODE_CONTROLLER Add a controller to the network
 *  - @ref ADD_NODE_END_NODE Add an End Node to the network
 *  - @ref ADD_NODE_STOP Stop learn mode without reporting an error.
 *  - @ref ADD_NODE_STOP_FAILED Stop learn mode and report an error to the new
 *    controller.
 *  - @ref ADD_NODE_OPTION_NORMAL_POWER Set this flag in bMode for High Power
 *    inclusion.
 * @param completedFunc callback function which is called with the add node
 * results.
 * @returns SL_STATUS_OK or SL_STATUS_FAIL.
 *
 * aka ZW_AddNodeToNetwork
 */
sl_status_t zwapi_add_node_to_network(uint8_t bMode,
                                      void (*completedFunc)(LEARN_INFO *));

/**
 * @brief Remove any type of node from the network.
 *
 * @param bMode network exclusion mode, one of:
 *  - @ref REMOVE_NODE_ANY Remove any node from the network
 *  - @ref REMOVE_NODE_CONTROLLER Remove a controller from the network
 *  - @ref REMOVE_NODE_END_NODE Remove an End Node from the network
 *  - @ref REMOVE_NODE_STOP Stop learn mode without reporting an error.
 *  - @ref ADD_NODE_OPTION_NORMAL_POWER Set this flag in bMode for High Power
 *    exclusion.
 * @param completedFunc callback function which is called with the remove node
 * results.
 * @returns SL_STATUS_OK or SL_STATUS_FAIL.
 *
 * aka ZW_RemoveNodeFromNetwork
 */
sl_status_t zwapi_remove_node_from_network(uint8_t bMode,
                                           void (*completedFunc)(LEARN_INFO *));

/**
 * @brief Transfer the role as primary controller to another controller.
 *
 * @param bMode one of:
 *  - @ref CONTROLLER_CHANGE_START Start the creation of a new primary
 *  - @ref CONTROLLER_CHANGE_STOP Stop the creation of a new primary
 *  - @ref CONTROLLER_CHANGE_STOP_FAILED Report that the replication failed
 *  - @ref ADD_NODE_OPTION_NORMAL_POWER Set this flag in bMode for High Power
 *    exchange.
 * @param completedFunc callback function which is called with the role transfer
 * results.
 * @returns SL_STATUS_OK or SL_STATUS_FAIL.
 *
 * aka ZW_ControllerChange
 */
sl_status_t
  zwapi_transfer_primary_ctrl_role(uint8_t bMode,
                                   void (*completedFunc)(LEARN_INFO *));

/**
 * @brief Set the maximum number of source routing attempts before the explorer
 * frame mechanism kicks in.
 *
 * @param maxRouteTries is the maximum number of route retries. Default value
 * with respect to maximum number of source routing attempts is five. Remember
 * to enable the explorer frame mechanism by setting the transmit option flag
 * TRANSMIT_OPTION_EXPLORE in the send data calls.
 *
 * A ZDK 4.5 controller uses the routing algorithm from 5.02 to address nodes
 * from ZDK's not supporting explorer frame.
 *
 * The routing algorithm from 5.02 ignores the transmit option
 * TRANSMIT_OPTION_EXPLORE flag and maximum number of source routing attempts
 * value (maxRouteTries)
 *
 * aka ZW_SetRoutingMAX
 */
sl_status_t zwapi_set_max_source_route(uint8_t maxRouteTries);

/**
 * @brief Get the Last Working Route (LWR) for a destination node if any exist.
 *
 * @param bNodeID Node ID (1...232) from whom the LWR is wanted.
 * @param pPriorityRoute pointer to a 5-byte array where the wanted LWR
 * will be written. See below for details.
 * @returns true if a LWR exists for bNodeID and the found route is copied to
 * the 5-byte array pointed out by pPriorityRoute.
 * @returns false if No LWR found for bNodeID.
 *
 * <b>CLEANUP COMMENT BEGIN</b>
 *
 * Return values could also be these ones (check the API guide):
 *  - Returns NON ZERO if a Priority Route is found. Priority route is either an
 *    Application injected Route or a LWR.
 *    - ZW_PRIORITY_ROUTE_APP_PR = Route is an App defined Priority Route
 *    - ZW_PRIORITY_ROUTE_ZW_LWR = Route is a Last Working Route
 *    - ZW_PRIORITY_ROUTE_ZW_NLWR = Route is a Next to Last Working Route
 *  - Returns zero if no Priority Route is found.
 *
 * <b>CLEANUP COMMENT END</b>
 *
 * The LWR is the last successful route used between sender and destination
 * node. The LWR is stored in NVM on the Z-Wave module.
 *
 * The (@ref ROUTECACHE_LINE_SIZE) 5-byte array contains the 4 repeaters (index
 * 0-3) and 1 routespeed byte (index 4) used in the LWR.
 *
 * The first repeater byte (starting from index 0) equaling zero indicates no
 * more repeaters in route. If the repeater at index 0 is zero then the LWR is
 * direct. The routespeed byte (index 4) can be either
 * @ref ZW_PRIORITY_ROUTE_SPEED_9600, @ref ZW_PRIORITY_ROUTE_SPEED_40K or
 * @ref ZW_PRIORITY_ROUTE_SPEED_100K
 *
 * pPriorityRoute Examples:
 * - <tt>0, 0, 0, 0, ZW_PRIORITY_ROUTE_SPEED_100K</tt> &rarr; Direct 100K
 * - <tt>2, 3, 0, 0, ZW_PRIORITY_ROUTE_SPEED_40K</tt> &rarr; 40K route through
 *   repeaters 2 and 3
 *
 * Tx: {REQ | 0x92 | bNodeID}
 *
 * Rx: {RES | 0x92 | bNodeID | retVal | repeater0 | repeater1 | repeater2 | repeater3 | routespeed}
 *
 * aka ZW_GetPriorityRoute
 * aka ZW_GetLastWorkingRoute
 */
uint8_t zwapi_get_priority_route(zwave_node_id_t bNodeID,
                                 uint8_t *pPriorityRoute);

/**
 * @brief Set the Last Working Route (LWR) for a destination node.
 *
 * @param bNodeID Node ID (1...232) for whom the LWR is to be set.
 * @param pPriorityRoute pointer to a 5-byte array containing the new LWR
 * to be set. See below for details
 * @returns true if the LWR for bNodeID was successfully set.
 * @returns false if the specified bNodeID was not valid and no LWR was set.
 *
 * The LWR is the last successful route used between sender and destination
 * node. The LWR is stored in NVM on the Z-Wave module.
 *
 * The (@ref ROUTECACHE_LINE_SIZE) 5-byte array contains the 4 repeaters (index
 * 0-3) and 1 routespeed byte (index 4) used in the LWR.
 *
 * The first repeater byte (starting from index 0) equaling zero indicates no
 * more repeaters in route. If the repeater at index 0 is zero then the LWR is
 * direct. The routespeed byte (index 4) can be either
 * @ref ZW_PRIORITY_ROUTE_SPEED_9600, @ref ZW_PRIORITY_ROUTE_SPEED_40K or
 * @ref ZW_PRIORITY_ROUTE_SPEED_100K
 *
 * pPriorityRoute Examples:
 * - <tt>0, 0, 0, 0, ZW_PRIORITY_ROUTE_SPEED_100K</tt> &rarr; Direct 100K
 *   Priority route
 * - <tt>2, 3, 0, 0, ZW_PRIORITY_ROUTE_SPEED_40K</tt> &rarr; 40K Priority route
 *   through repeaters 2 and 3
 * - <tt>2, 3, 4, 0, ZW_PRIORITY_ROUTE_SPEED_9600</tt> &rarr; 9600 Priority
 *   route through repeaters 2, 3 and 4
 *
 * Tx: {REQ | 0x93 | bNodeID | repeater0 | repeater1 | repeater2 | repeater3 | routespeed}
 *
 * Rx: {RES | 0x93 | bNodeID | retVal}
 *
 * aka ZW_SetPriorityRoute
 * aka ZW_SetLastWorkingRoute
 */
sl_status_t zwapi_set_priority_route(zwave_node_id_t bNodeID,
                                     uint8_t *pPriorityRoute);

/**
 * @brief Enable/disable learn mode for a virtual node.
 *
 * @param node NodeID of the virtual node to set to Learn Mode. Use 0 (zero) if new node
 * is to be learned
 * @param mode learn mode. One of:
 *  - @ref VIRTUAL_END_NODE_LEARN_MODE_DISABLE Disable
 *  - @ref VIRTUAL_END_NODE_LEARN_MODE_ENABLE Enable
 *  - @ref VIRTUAL_END_NODE_LEARN_MODE_ADD Create New Virtual End Node
 *  - @ref VIRTUAL_END_NODE_LEARN_MODE_REMOVE Remove Virtual End Node
 * @param learnFunc callback function for the Node learn mode process
 * @returns SL_STATUS_OK if successful.
 * @returns SL_STATUS_FAIL if node is invalid or controller is primary.
 *
 * When learn mode is enabled, received "Assign ID's Command" are handled:
 * - If the current stored ID's are zero, the received ID's will be stored.
 * - If the received ID's are zero the stored ID's will be set to zero.
 *
 * The learnFunc is called when the received assign command has been handled.
 * The newID parameter is the learned Node ID.
 *
 * aka ZW_SetSlaveLearnMode
 */
sl_status_t zwapi_set_virtual_node_to_learn_mode(
  zwave_node_id_t node,
  uint8_t mode,
  void (*learnFunc)(uint8_t bStatus,
                    zwave_node_id_t orgID,
                    zwave_node_id_t newID));

/**
 * @brief Request nodemask containing virtual nodes in controller bridge.
 *
 * @param nodeMask points to nodemask array where the current virtual node
 * nodemask is to be copied.
 * @return SL_STATUS_OK or SL_STATUS_FAIL
 *
 * aka ZW_GetVirtualNodes
 */
sl_status_t zwapi_get_virtual_nodes(zwave_nodemask_t nodeMask);

/**
 * @brief Read out neighbor information
 *
 * @param bNodeID is the Node ID of a node whom routing info is needed
 * @param buf pointer to where routing info is stored
 * @param bRemoveBad is true the bad repeaters are removed
 * @param bRemoveNonReps is the upper nibble is bit flag options, lower nibble
 * is speed. Combine exactly one speed with any number of options.
 *
 * Bit flags options for upper nibble:
 *   - ZW_GET_ROUTING_INFO_REMOVE_BAD      - Remove bad link from routing info
 *   - ZW_GET_ROUTING_INFO_REMOVE_NON_REPS - Remove non-repeaters from the routing info
 *
 * Speed values for lower nibble:
 *   - ZW_GET_ROUTING_INFO_ANY  - Return all nodes regardless of speed
 *   - ZW_GET_ROUTING_INFO_9600 - Return nodes supporting 9.6k
 *   - ZW_GET_ROUTING_INFO_40K  - Return nodes supporting 40k
 *   - ZW_GET_ROUTING_INFO_100K - Return nodes supporting 100k
 *
 * aka ZW_GetRoutingInfo_old
 */
sl_status_t zwapi_get_old_routing_info(zwave_node_id_t bNodeID,
                                       uint8_t *buf,
                                       uint8_t bRemoveBad,
                                       uint8_t bRemoveNonReps);

/**
 * @brief Enable the SUC functionality in a controller.
 *
 * @param state if true SUC is enabled, if false SUC is disabled.
 * @param capabilities is the capability of the SUC:
 *  - If ZW_SUC_FUNC_BASIC_SUC Only enable the basic SUC functionality
 *  - If ZW_SUC_FUNC_NODEID_SERVER : Enable the node ID server
 *    functionality to become a SIS.
 * @returns SL_STATUS_OK if the SUC functionality was enabled/disabled.
 * @returns SL_STATUS_FAIL potentially attempting to disable a running SUC, not
 * allowed.
 *
 * aka ZW_EnableSUC
 */
sl_status_t zwapi_enable_suc(uint8_t state, uint8_t capabilities);

/**
 * @brief Add any type of node to the network and accept prekit inclusion.
 *
 * @param bMode inclusion mode. Must always be @ref
 * ADD_NODE_HOME_ID, potentially coupled with these flags:
 *  - @ref ADD_NODE_OPTION_NORMAL_POWER Set this flag in bMode for High Power
 *    inclusion.
 *  - @ref ADD_NODE_OPTION_NETWORK_WIDE Set this flag in bMode for Network wide
 *    inclusion
 * @param dsk pointer to the 16-byte DSK.
 * @param completedFunc Callback function.
 * @returns SL_STATUS_OK or SL_STATUS_FAIL.
 *
 * @note This should be called after receiving a application_controller_update_function
 * with a whitelisted DSK.
 *
 * aka ZW_AddNodeToNetworkSmartStart
 */
sl_status_t zwapi_add_smartstart_node_to_network(
  uint8_t bMode, const uint8_t *dsk, void (*completedFunc)(LEARN_INFO *));

/**
 * @brief Sets the Application Node Information data for all virtual nodes owned
 * by the Z-Wave API Module.
 *
 * @param dstNode is the virtual NodeID (This value is IGNORED).
 * @param listening is true if this node is always on air
 * @param node_type  is the Device Type (the .basic member is ignored here)
 * @param nodeParm is the device parameter buffer
 * @param parmLength is the number of device parameter bytes
 *
 * This takes effect for all virtual nodes, regardless of dstNode value.
 *
 * aka SerialAPI_ApplicationSlaveNodeInformation
 */
sl_status_t zwapi_set_virtual_node_application_node_information(
  zwave_node_id_t dstNode,
  uint8_t listening,
  const node_type_t *node_type,
  uint8_t *nodeParm,
  uint8_t parmLength);

/// @} End of group ZWAPI_CONTROLLER

#ifdef __cplusplus
}
#endif

#endif  // ZWAPI_PROTOCOL_CONTROLLER_H
