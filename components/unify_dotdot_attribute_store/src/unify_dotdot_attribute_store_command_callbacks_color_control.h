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
 * @defgroup color_control_cluster_mapper Color Control Cluster Mapper
 * @ingroup dotdot_mapper
 * @brief Maps Color Control Cluster incoming Commands to attribute modifications.
 *
 * @{
 */

#ifndef COLOR_CONTROL_CLUSTER_MAPPER_H
#define COLOR_CONTROL_CLUSTER_MAPPER_H

// Generic includes
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Intitialise the Color Control cluster mapper
 *
 */
void color_control_cluster_mapper_init(void);

#ifdef __cplusplus
}
#endif

#endif  //COLOR_CONTROL_CLUSTER_MAPPER_H
/** @} end color_control_cluster_mapper */
