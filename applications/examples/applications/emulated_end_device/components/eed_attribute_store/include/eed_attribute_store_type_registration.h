/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup eed_attribute_store_type_registration EED attribute store known attribute types
 * @ingroup eed_attribute_store
 * @brief Lists and registers all attribute types
 *
 * @{
 */

#ifndef EED_ATTRIBUTE_STORE_TYPE_REGISTRATION_H
#define EED_ATTRIBUTE_STORE_TYPE_REGISTRATION_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register all known Applicaiton attribute types
 * @return SL_STATUS_OK on success, any other code on failure.
 */
sl_status_t eed_attribute_store_register_known_attribute_types();

#ifdef __cplusplus
}
#endif

#endif  //EED_ATTRIBUTE_STORE_TYPE_REGISTRATION_H
/** @} end eed_attribute_store_type_registration */
