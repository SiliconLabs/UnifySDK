/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup zwave_network_management_return_route_queue Return Route Assignment Queue
 * @ingroup zwave_network_management
 * @brief Small queue component for assigning return routes
 *
 * Small queue component that will cache and process the assign return
 * route requests issued by the application.
 *
 * @{
 */

#ifndef ZWAVE_NETWORK_MANAGEMENT_RETURN_ROUTE_QUEUE_H
#define ZWAVE_NETWORK_MANAGEMENT_RETURN_ROUTE_QUEUE_H

#include <stdbool.h>
#include "sl_status.h"
#include "zwave_node_id_definitions.h"

// Size of the Assign Return Route queue.
#define ASSIGN_RETURN_ROUTE_QUEUE_SIZE 5

typedef struct _assign_return_route_ {
  zwave_node_id_t node_id;
  zwave_node_id_t destination_node_id;
  // Flag that indicates if we requested the Z-Wave API to assign this route
  bool in_progress;
} assign_return_route_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Clears the Assign Return Route Queue
 */
void zwave_network_management_return_route_clear_queue();

/**
 * @brief Adds a Return Route request into the queue
 *
 * @param request   Pointer to a assign_return_route_t request.
 * @returns SL_STATUS_OK if the request was queued (or already queued)
 *          SL_STATUS_FAIL otherwise
 */
sl_status_t zwave_network_management_return_route_add_to_queue(
  const assign_return_route_t *request);

/**
 * @brief Requests the Z-Wave API to assign the Next Return Route in the queue
 *
 * @returns SL_STATUS_IN_PROGRESS if return route establish is in progress
 *          SL_STATUS_IDLE otherwise
 */
sl_status_t zwave_network_management_return_route_assign_next();

/**
 * @brief callback function for @ref zwapi_assign_return_route()
 * @param status The status of the Assign Return Route operation.
 */
void on_assign_return_route_complete(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_NETWORK_MANAGEMENT_RETURN_ROUTE_QUEUE_H
/** @} end zwave_network_management_return_route_queue */
