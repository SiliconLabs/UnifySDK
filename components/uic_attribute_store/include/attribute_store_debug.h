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
 * @file attribute_store_debug.h
 * Provides routines to print debug information about the attribute store,
 * translating attribute ids to type names.
 * 
 * @{
 */

#ifndef ATTRIBUTE_STORE_DEBUG_H
#define ATTRIBUTE_STORE_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "attribute_store.h"

/**
 * @brief Register a name for a attribute typid.
 * 
 * @param id 
 * @param name 
 */
void attribute_store_register_attribute(attribute_store_type_t id,
                                        const char *name);

/**
 * @brief Get the type name of an attribute
 * 
 * @param id 
 * @return const char* Name 
 */
const char *attribute_store_name_by_type(attribute_store_type_t id);

/**
 * @brief Lookup the type of an attribute by its name
 * 
 * @param name Name to look up
 * @return attribute_store_type_t resulting type, if name is not found 0xffffffff is returned.
 */
attribute_store_type_t attribute_store_type_by_name(const char *name);

#ifdef __cplusplus
}
#endif

#endif  //ZPC_ATTRIBUTE_STORE_DEBUG_H
/** @} end attribute_store_debug */
