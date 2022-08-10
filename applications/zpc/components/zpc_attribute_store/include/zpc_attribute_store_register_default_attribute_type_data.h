/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup zpc_attribute_store_register_default_attribute_type_data ZPC attribute store default type data
 * @ingroup zpc_attribute_store
 * @brief Registers attribute store types for all types defines in the header files.
 *
 * @{
 */

#ifndef ZPC_ATTRIBUTE_STORE_REGISTER_DEFAULT_ATTRIBUTE_TYPE_DATA_H
#define ZPC_ATTRIBUTE_STORE_REGISTER_DEFAULT_ATTRIBUTE_TYPE_DATA_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register default names and information for all attributes types
 *        that are not registerd yet.
 *
 * @returns SL_STATUS_OK if successful, any other code if an error occurred.
 */
sl_status_t zpc_attribute_store_register_default_attribute_type_data();

#ifdef __cplusplus
}
#endif

#endif  //ZPC_ATTRIBUTE_STORE_REGISTER_DEFAULT_ATTRIBUTE_TYPE_DATA_H
/** @} end zpc_attribute_store_register_default_attribute_type_data */
