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
 * @defgroup on_off_cluster_mapper OnOff cluster mapper
 * @ingroup dotdot_mapper
 * @brief Maps the OnOff cluster to known command classes 
 *
 * This module is used to parse attributes regarding OnOff cluster commands 
 * coming from dotdot serializer, and perform attribute write to the attribute store.
 * The module is also expected to register a callback regarding OnOff attrbite update
 * and pass the report to dotdot serializer.
 * 
 * @{
 */

#ifndef ON_OFF_CLUSTER_MAPPER_H
#define ON_OFF_CLUSTER_MAPPER_H

#ifdef __cplusplus
extern "C" {
#endif
// Generic includes
#include <stdbool.h>

/**
 * @brief Intitialise the OnOff cluster mapper
 * 
 * @returns true on success
 * @returns false on failure
 * 
 */
bool on_off_cluster_mapper_init(void);

/**
 * @brief Clears the OnOff cluster mapper from all its content.
 * 
 */
int on_off_cluster_mapper_teardown(void);

#ifdef __cplusplus
}
#endif

#endif  //ON_OFF_CLUSTER_MAPPER_H
/** @} end on_off_cluster_mapper */
