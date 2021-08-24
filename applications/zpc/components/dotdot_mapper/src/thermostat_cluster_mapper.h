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
 * @defgroup thermostat_cluster_mapper Thermostat cluster mapper
 * @ingroup dotdot_mapper
 * @brief Maps the Thermostat cluster to known command classes 
 *
 * 
 * @{
 */

#ifndef THERMOSTAT_CLUSTER_MAPPER_H
#define THERMOSTAT_CLUSTER_MAPPER_H

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
bool thermostat_cluster_mapper_init(void);

/**
 * @brief Clears the OnOff cluster mapper from all its content.
 * 
 */
int thermostat_cluster_mapper_teardown(void);

#ifdef __cplusplus
}
#endif

#endif  //THERMOSTAT_CLUSTER_MAPPER_H
/** @} end thermostat_cluster_mapper */
