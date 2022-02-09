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
 * @defgroup zpc_attribute_store_debug ZPC attribute type name to attribute type id convertor
 * @ingroup zpc_attribute_store
 * @brief Register Attribute types names to help converting the name to attribute type id
 * @ref attribute_store contents
 * *
 * @{
 */

#ifndef ZPC_ATTRIBUTE_STORE_ATTR_TYPE_NAME_HELPERH
#define ZPC_ATTRIBUTE_STORE_ATTR_TYPE_NAME_HELPERH

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register all attribute used by the zpc and enable the APIs to convert the 
 *        type name to attribute type id.
 * @return SL_STATUS_OK, always.
 */
sl_status_t zpc_attribute_store_attr_type_name_helper_init();

#ifdef __cplusplus
}
#endif

#endif  //ZPC_ATTRIBUTE_STORE_ATTR_TYPE_NAME_HELPERH
/** @} end zpc_attribute_store_attr_type_name_helper */
