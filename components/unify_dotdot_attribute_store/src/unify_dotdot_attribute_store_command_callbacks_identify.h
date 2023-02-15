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
 * @defgroup identify_cluster_mapper Identify Cluster Mapper
 * @ingroup dotdot_mapper
 * @brief Maps Identify Cluster incoming Commands to attribute modifications.
 *
 * @{
 */

#ifndef IDENTIFY_CLUSTER_MAPPER_H
#define IDENTIFY_CLUSTER_MAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the component
 *
 */
void identify_cluster_mapper_init(void);


/**
 * @brief Clears the Identify cluster mapper from all its content.
 */
void identify_cluster_mapper_teardown(void);

#ifdef __cplusplus
}
#endif

#endif  //IDENTIFY_CLUSTER_MAPPER_H
/** @} end identify_cluster_mapper */
