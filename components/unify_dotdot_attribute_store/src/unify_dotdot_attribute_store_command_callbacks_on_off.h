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
 * @defgroup on_off_cluster_mapper OnOff Cluster Mapper
 * @ingroup dotdot_mapper
 * @brief Maps On Off Cluster incoming Commands to attribute modifications.
 *
 * @{
 */

#ifndef ON_OFF_CLUSTER_MAPPER_H
#define ON_OFF_CLUSTER_MAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Intitialise the On Off cluster mapper
 *
 */
void on_off_cluster_mapper_init(void);

#ifdef __cplusplus
}
#endif

#endif  //ON_OFF_CLUSTER_MAPPER_H
/** @} end on_off_cluster_mapper */
