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

/**
 * @defgroup occupancy_sensing_cluster_mapper OccupancySensing cluster mapper
 * @ingroup dotdot_mapper
 * @brief Maps the OccupancySensing cluster to known command classes
 *
 * This mapper will map the Z-Wave Notification Command Class,
 * events Door Open / Door Close (Type Access Control) to the OccupancySensing cluster,
 * attribute Occupancy
 *
 * @{
 */

#ifndef OCCUPANCY_SENSING_CLUSTER_MAPPER_H
#define OCCUPANCY_SENSING_CLUSTER_MAPPER_H

// Includes from other components
#include "attribute_store.h"

// Generic includes
#include <stdbool.h>

// Occupancy Sensing Cluster defines:
// PIR Sensor in the Occupancy Cluster
#define OCCUPANCY_SENSOR_TYPE_PIR 0x00

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Intitialise the OccupancySensing cluster mapper
 *
 * @returns true on success
 * @returns false on failure
 *
 */
bool occupancy_sensing_cluster_mapper_init(void);

/**
 * @brief Tears down the Occupancy cluster mapper from all its content.
 *
 * @returns 0 on success
 *
 */
int occupancy_sensing_cluster_mapper_teardown(void);

/**
 * @brief Callback for the Attribute Store reading the Notification Motion Detected attributes
 *
 */
void occupancy_sensing_motion_detection_update_callback(
  attribute_store_node_t updated_node, attribute_store_change_t change);

/**
 * @brief Callback for the Attribute Store reading the Notification Motion Detected attributes
 *
 */
void occupancy_sensing_motion_idle_update_callback(
  attribute_store_node_t updated_node, attribute_store_change_t change);

#ifdef __cplusplus
}
#endif

#endif  //OCCUPANCY_SENSING_CLUSTER_MAPPER_H
/** @} end occupancy_sensing_cluster_mapper */
