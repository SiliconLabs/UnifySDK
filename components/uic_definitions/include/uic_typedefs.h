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
 * @defgroup unify_components Unify Components
 * @brief Shared Unify components to be used by multiple applications
 *
 * The Unify components is a set of components aimed at being reused my multiple
 * applications.
 *
 */

/**
 * @file uic_typedefs.h
 * @brief Generic type definitions for Unify
 *
 * Generic type definitions used by multiple UIC components.
 *
 * @{
 */

// Standard library
#include <stdint.h>

#ifndef UIC_TYPEDEFS_H
#define UIC_TYPEDEFS_H

typedef uint8_t dotdot_endpoint_id_t;
typedef const char *dotdot_unid_t;
typedef const char *ota_uiid_t;
typedef uint16_t dotdot_group_id_t;

#endif  //UIC_TYPEDEFS_H
/** @} end uic_typedefs */
