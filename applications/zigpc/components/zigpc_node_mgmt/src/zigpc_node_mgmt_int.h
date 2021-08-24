/*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

/**
 * @file zigpc_node_mgmt_int.h
 * @defgroup zigpc_node_mgmt_int
 * @ingroup zigpc_node_mgmt
 * @brief Internal API for node management. Used for helper functions
 * and data structured
 *
 * @{
 *
**/
#ifndef ZIGPC_NODE_MGMT_INT_H
#define ZIGPC_NODE_MGMT_INT_H

#include <stdbool.h>

#include "process.h"
#include "zigpc_common_zigbee.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Process name handler for node managment
 *
 */
PROCESS_NAME(zigpc_node_mgmt_process);

/**
 * @brief zigpc_node_mgmt_evt
 * Enumerated type for storing the events handled by
 * the zigpc_node_mgmt process
 *
 */
enum zigpc_node_mgmt_evt {
  NODE_MGMT_MANAGE_EVT = 0,
  NODE_MGMT_MANAGE_EUI64_EVT,
  NODE_MGMT_ADD_END_EVT,
  NODE_MGMT_FETCH_EVT,
  NODE_MGMT_UPDATE_EVT,
  NODE_MGMT_REMOVE_EVT
};

/**
 * @brief node_mgmt_manage_node_data_t
 * Data for a manage_node event
**/
typedef struct {
  zigbee_node_t node;
} node_mgmt_manage_node_data_t;

/**
 * @brief node_mgmt_manage_eui64_data_t
 * Data for a manage_eui64 event
**/
typedef struct {
  zigbee_eui64_t eui64;
} node_mgmt_manage_eui64_data_t;

/**
 * @brief node_mgmt_add_endpoint_data_t
 * Data for a add_endpoint event
**/
typedef struct {
  zigbee_eui64_t eui64;
  zigbee_endpoint_t endpoint;
} node_mgmt_add_endpoint_data_t;

/**
 * @brief node_mgmt_update_node_data_t
 * Data for an update_node event
**/
typedef struct {
  zigbee_node_t node;
} node_mgmt_update_node_data_t;

/**
 * @brief node_mgmt_remove_node_data_t
 * Data for a remove_node event
**/
typedef struct {
  zigbee_eui64_t eui64;
} node_mgmt_remove_node_data_t;

/**
 * @brief Helper to send events to Contiki process in ZigPC Node Mgmt by
 * dynamically allocating the event data based on the data_size passed in.
 *
 * @param event event type.
 * @param data associated data.
 * @param data_size size of associated data.
 * @return SL_STATUS_OK if the event is sent to the ZigPC Node Mgmt contiki
 * process, SL_STATUS_NULL_POINTER on invalid arguments passed in, or
 * SL_STATUS_EMPTY if the data_size to send is 0.
 */
sl_status_t zigpc_node_process_send_event(const enum zigpc_node_mgmt_evt event,
                                          void *data,
                                          size_t data_size);

/**
 * @brief zigpc_node_mgmt_init_observers 
 * Function to initialize the observers used by the node_mgmt
 * component
 *
 * @return SL_STATUS_OK if able to register the necessary
 * observers
**/
sl_status_t zigpc_node_mgmt_init_observers(void);


sl_status_t zigpc_node_process_configure_endpoint(const node_mgmt_add_endpoint_data_t add_data);

/**
 * @brief zigpc_node_mgmt_on_endpoint_discovered 
 * Callback for managing a newly discovered endpoint
 *
 * @param event_data -  the data necessary to handle
 *                      a newly discovered endpoint
**/
void zigpc_node_mgmt_on_endpoint_discovered(void *event_data);

/**
 * @brief zigpc_node_mgmt_on_node_add_complete
 * Callback for being notified when a node is added
 * to the network
 *
 * @param event_data -  the data necessary to handle the
 *                      newly added node
**/
void zigpc_node_mgmt_on_node_add_complete(void *event_data);

/**
 * @brief Unretain all messages of the nodes in nodemap plus the protocol controller itself
 * 
 */
void zigpc_node_mgmt_unretain_all_topics();
#ifdef __cplusplus
}
#endif

#endif  // ZIGPC_NODE_MGMT_INT_H
/** @} end zigpc_node_mgmt_int **/
