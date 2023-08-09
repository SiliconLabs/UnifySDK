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
 * @defgroup zpc_on_off_cluster_mapper ZPC OnOff cluster mapper
 * @ingroup dotdot_mapper
 * @brief Maps OnOff Cluster incoming Commands to attribute modifications.
 *
 * @{
 */

#ifndef ON_OFF_CLUSTER_MAPPER_H
#define ON_OFF_CLUSTER_MAPPER_H

// Generic includes
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Intitialise the OnOff cluster mapper
 *
 * @returns true on success
 * @returns false on failure
 *
 */
bool on_off_cluster_basic_mapper_init(void);

#ifdef __cplusplus
}
#endif

#endif  //ON_OFF_CLUSTER_MAPPER_H
/** @} end on_off_cluster_mapper */
