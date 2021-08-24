/*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/**
 * @file zcl_profile_list_int.h
 * @defgroup zcl_profile_list_int Zcl Profiles Internal
 * @ingroup zcl_profiles
 * @brief This header contains private functions used for
 * private zcl_profile manipulation
 *
 * @{
 */

#ifndef ZCL_PROFILE_LIST_H
#define ZCL_PROFILE_LIST_H

#include <list>
#include "zigpc_common_zigbee.h"

/**
 * @brief zigpc_zcl_profiles_get_profile_list
 * Gives a reference to the internal profiles list so it can
 * be manipulated internally
 *
 * @return the reference to the internal list of zcl_profiles
 **/
std::list<zcl_profile_t> &zigpc_zcl_profiles_get_profile_list(void);

#endif  // ZCL_PROFILE_LIST_H

/** @} end zcl_profile_list_int */
