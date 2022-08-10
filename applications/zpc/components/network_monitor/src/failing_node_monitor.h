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

#ifndef FAILING_NODE_MONITOR_H
#define FAILING_NODE_MONITOR_H

// Interfaces
#include "zwave_node_id_definitions.h"

// Contiki defines (CLOCK_SECOND)
#include "clock.h"

/**
 * @defgroup failing_network_monitor Failing Node Monitor
 * @ingroup network_monitor
 * @brief Monitors AL and FL failing nodes, queues NOPs to verify if they
 * started responding again
 *
 * @{
 *
 */

// Pinging nodes is not that important, we do not want NOPs messages to clutter
// the queue, so if they are not sent within 2 seconds, remove them from the
// Tx Queue
#define NOP_DISCARD_TIMEOUT_MS 2000

// When we already queued a NOP, back-off for that duration before
// re-attempting the next NOP.
#define NOP_REQUEUE_BACK_OFF_MS 10000

// Start interval for pinging AL nodes
#define AL_NODE_PING_TIME_INTERVAL (4 * CLOCK_SECOND)
// Multication factor for AL nodes intervals
#define AL_NODE_PING_TIME_FACTOR 2

// Start interval for pinging FL nodes
#define FL_NODE_PING_TIME_INTERVAL (40 * CLOCK_SECOND)
// Multication factor for FL nodes intervals
#define FL_NODE_PING_TIME_FACTOR 4

// Maximum time we settle on between ping intervals:
#define MAX_PING_TIME_INTERVAL (24 * 60 * CLOCK_SECOND)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the failing node monitoring by registering callbacks to the
 * attribute store.
 *
 */
void failing_node_monitor_init();

#ifdef __cplusplus
}
#endif
/** @} end failing_network_monitor */

#endif  //FAILING_NODE_MONITOR_H
