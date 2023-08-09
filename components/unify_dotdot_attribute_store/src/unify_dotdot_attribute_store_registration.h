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
 * @defgroup unify_dotdot_attribute_store_registration DotDot Attribute Store Registration
 * @ingroup unify_dotdot_attribute_store
 * @brief Registers the type of the DotDot attributes to the Attribute Store.
 *
 * Initializing this component will enable:
 * 1. The @ref attribute_store to verify that DotDot Attributes are written/saved correctly
 * 2. Generic components like the @ref unify_attribute_mapper to interpret data their operations.
 *
 * @{
 */

#ifndef UNIFY_DOTDOT_ATTRIBUTE_STORE_REGISTRATION_H
#define UNIFY_DOTDOT_ATTRIBUTE_STORE_REGISTRATION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Registers all ZCL attributes defined in the uic_dotdot XML files to
 * the Attribute Store.
 *
 * @returns SL_STATUS_OK on success, any other value in case of error
 */
sl_status_t unify_dotdot_attribute_store_registration_init();

#ifdef __cplusplus
}
#endif

#endif  //UNIFY_DOTDOT_ATTRIBUTE_STORE_REGISTRATION_H
/** @} end unify_dotdot_attribute_store_registration */
