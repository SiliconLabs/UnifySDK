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
 * @defgroup level_cluster_mapper Level Cluster Mapper
 * @ingroup dotdot_mapper
 * @brief Maps Level Cluster incoming Commands to attribute modifications.
 *
 * This module is used to register callbacks for incoming Level Cluster Commands
 * and update the corresponding ZCL attributes in the @ref attribute_store .
 * @{
 */

#ifndef LEVEL_CLUSTER_MAPPER_H
#define LEVEL_CLUSTER_MAPPER_H

// Generic includes
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Intitialise the Level cluster mapper
 *
 * @returns true on success
 * @returns false on failure
 *
 */
bool level_cluster_mapper_init(void);

/**
 * @brief Clears the Level cluster mapper from all its content.
 *
 * @returns 0 on success, any other value on failure.
 */
int level_cluster_mapper_teardown(void);

#ifdef __cplusplus
}
#endif

#endif  //LEVEL_CLUSTER_MAPPER_H
/** @} end level_cluster_mapper */
