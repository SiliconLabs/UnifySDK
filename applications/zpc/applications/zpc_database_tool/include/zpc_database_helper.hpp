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

/**
 * @brief Get a vector of all attributes of a given type in the attribute Store
 * 
 * @note The given attribute must be under ATTRIBUTE_ENDPOINT_ID
 * 
 * @param attribute_type The type of attribute to get
 * 
 * @returns std::vector<attribute> for the given attribute type
 */
std::vector<attribute_store::attribute>
  get_attribute_list(attribute_store_type_t attribute_type);

/**
 * @brief Update bitmask from an array to a uint32_t
 * 
 * @param attribute_to_update Attribute to update
 * 
 * @returns true if the update was successful
 * @returns false if the update was unsuccessful
 */
bool update_bitmask_attribute(attribute_store::attribute attribute_to_update);

#endif  //ZPC_DATABASE_HELPER_HPP
/** @} end zpc_database_helper */
