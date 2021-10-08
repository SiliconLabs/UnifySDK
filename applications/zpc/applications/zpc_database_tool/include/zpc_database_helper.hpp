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
 * @file zpc_database_helper.hpp
 * @brief Helper functions for the ZPC database tools
 *
 * @{
 */

#ifndef ZPC_DATABASE_HELPER_HPP
#define ZPC_DATABASE_HELPER_HPP

#include "attribute.hpp"
#include <vector>

/**
 * @brief Get a vector of all ATTRIBUTE_ENDPOINT_ID in the attribute Store
 * @returns std::vector<attribute> for the Endpoint ID attributes
 */
std::vector<attribute_store::attribute> endpoint_id_list();

#endif  //ZPC_DATABASE_HELPER_HPP
/** @} end zpc_database_helper */
