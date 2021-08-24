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

#ifndef NODE_MGMT_PROCESS_H
#define NODE_MGMT_PROCESS_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file zigpc_node_mgmt_process.h
 * @defgroup zigpc_node_mgmt_process ZigPC Node Management Process Handlers
 * @ingroup zigpc_node_mgmt
 * @brief API and funtions for node management process 
 *
 * @{
 *
**/

/**
 * @brief zigpc_node_mgmt_setup - sets up the parts of the software used 
 * by node management
 *
 * @return the status if the method succeeded and the component was set up
 * correctly
**/
sl_status_t zigpc_node_mgmt_setup(void);

/**
 * @brief zigpc_node_mgmt_shutdown - graceful shuts down all parts of the
 * node management, expected to be called before the program exits
 *
 * @return an integer representing the success of the operation. 0 for success
 * any other number represents a failure
 *
**/
int zigpc_node_mgmt_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif  //NODE_MGMT_PROCESS_H
/**@} zigpc_node_mgmt_process **/
