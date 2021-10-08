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
 * @file manufacturer_id_to_name_mapper.h
 * @brief The module maps Z-Wave Manufacturer ID to Manufacturer Name and 
 *        update the DotDot Basic Cluster Manufacturer name attribute.
 *
 * @{
 */
 
#ifndef MANUFACTURER_ID_TO_NAME_MAPPER_H
#define MANUFACTURER_ID_TO_NAME_MAPPER_H

// Generic includes
#include <stdbool.h>

#define ATTRIBUTE_MANUFACTURER_SPECIFIC_MANUFACTURER_ID 0x7202

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Intitialise the Manufacturer ID to Name Mapper
 * 
 * @returns true on success
 * @returns false on failure
 * 
 */
bool manufacturer_id_to_name_mapper_init(void);

/**
 * @brief Clears the Manufacturer ID to Name Mapper from all its content.
 * 
 */
int manufacturer_id_to_name_mapper_teardown(void);

#ifdef __cplusplus
}
#endif

#endif  //MANUFACTURER_ID_TO_NAME_MAPPER_H
/** @} end manufacturer_id_to_name_mapper */
