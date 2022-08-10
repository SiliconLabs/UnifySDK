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
 * @defgroup binding_cluster_mapper Binding Cluster Mapper
 * @ingroup dotdot_mapper
 * @brief Sets up the Binding Clusters mapper
 *
 * This component is responsible for publishing information relative to
 * the Binding Cluster
 *
 * @{
 */

#ifndef BINDING_CLUSTER_MAPPER_H
#define BINDING_CLUSTER_MAPPER_H

// Generic includes
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the Binding cluster mapper
 *
 * @returns true on success, false on failure
 */
bool binding_cluster_mapper_init(void);

#ifdef __cplusplus
}
#endif

#endif  //BINDING_CLUSTER_MAPPER_H
/** @} end binding_cluster_mapper */
