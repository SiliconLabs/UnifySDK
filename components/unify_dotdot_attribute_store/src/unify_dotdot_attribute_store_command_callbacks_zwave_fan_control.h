/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef ZWAVE_FAN_CONTROL_CLUSTER_MAPPER_H
#define ZWAVE_FAN_CONTROL_CLUSTER_MAPPER_H

// Generic includes
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Zwave Fan Control cluster mapper
 *
 */
void zwave_fan_control_cluster_mapper_init(void);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_FAN_CONTROL_CLUSTER_MAPPER_H
/** @} end unify_cluster_mapper */