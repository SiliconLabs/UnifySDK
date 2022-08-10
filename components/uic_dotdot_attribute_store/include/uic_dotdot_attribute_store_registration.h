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
 * @defgroup uic_dotdot_attribute_store_registration Attribute Store Registration for UCL DotDot Attributes
 * @brief Components performs a registration for the attribute store for all ZCL
 * attributes.
 *
 * @{
 */

#ifndef UIC_DOTDOT_ATTRIBUTE_STORE_REGISTRATION_H
#define UIC_DOTDOT_ATTRIBUTE_STORE_REGISTRATION_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Registers all ZCL attributes defined in the uic_dotdot XML files.
 *
 * @returns SL_STATUS_OK on success, any other value in case of error
 */
sl_status_t uic_dotdot_attribute_store_registration_init();

#ifdef __cplusplus
}
#endif

#endif  //UIC_DOTDOT_ATTRIBUTE_STORE_REGISTRATION_H
/** @} end uic_dotdot_attribute_store_registration */
