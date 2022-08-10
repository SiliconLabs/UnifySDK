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

#ifndef ATTRIBUTE_STORE_TYPE_REGISTRATION_INTERNAL_H
#define ATTRIBUTE_STORE_TYPE_REGISTRATION_INTERNAL_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Resets the previously registrered attribute types
 *
 * @returns SL_STATUS_OK on success, other codes in case of error
 */
sl_status_t attribute_store_reset_registered_attribute_types();

#ifdef __cplusplus
}
#endif

#endif  // ATTRIBUTE_STORE_TYPE_REGISTRATION_INTERNAL_H