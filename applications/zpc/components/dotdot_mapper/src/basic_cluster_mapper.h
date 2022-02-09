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
 * @defgroup basic_cluster_mapper Basic Cluster Mapper
 * @ingroup dotdot_mapper
 * @brief Create manual maps between the Basic Cluster and several Z-Wave Attributes.
 *
 * This component is responsible for manually mapping some of the Z-Wave
 * attributes to the Basic Cluster.
 *
 * @{
 */

#ifndef BASIC_CLUSTER_MAPPER_H
#define BASIC_CLUSTER_MAPPER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Intitialise the Basic cluster mapper
 *
 * @returns true on success
 * @returns false on failure
 *
 */
bool basic_cluster_mapper_init(void);

/**
 * @brief Teardowns the Basic Cluster mapper.
 *
 */
int basic_cluster_mapper_teardown(void);

#ifdef __cplusplus
}
#endif

#endif  //BASIC_CLUSTER_MAPPER_H
/** @} end basic_cluster_mapper */
