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
 * @file door_lock_cluster_mapper.h
 * @brief This module maps the zigbee Door Lock cluster to
 *        Z-Wave Door Lock CC and vs.
 * @{
 */
 
#ifndef DOOR_LOCK_CLUSTER_MAPPER_H
#define DOOR_LOCK_CLUSTER_MAPPER_H

// Generic includes
#include <stdbool.h>

// Defines to find z-wave door lock types
#define DOOR_LOCK_COMPONENTS_BOLT_MASK 0x02
#define DOOR_LOCK_COMPONENTS_LATCH_MASK 0x04
#define DOOR_LOCK_COMPONENTS_BOLT_LATCH_MASK 0x06

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Intitialise the DoorLock cluster mapper
 * 
 * @returns true on success
 * @returns false on failure
 * 
 */
bool door_lock_cluster_mapper_init(void);

/**
 * @brief Clears the DoorLock cluster mapper from all its content.
 * 
 */
int door_lock_cluster_mapper_teardown(void);
#ifdef __cplusplus
}
#endif

#endif  //DOOR_LOCK_CLUSTER_MAPPER_H
/** @} end door_lock_cluster_mapper */
