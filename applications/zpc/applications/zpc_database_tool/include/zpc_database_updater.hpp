/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file zpc_database_updater.hpp
 * @brief Helper functions for the ZPC database tools update
 *
 * @{
 */

#ifndef ZPC_DATABASE_UPDATER_HPP
#define ZPC_DATABASE_UPDATER_HPP

#include "attribute.hpp"
#include <vector>

namespace zpc_database_updater {

/**
 * @brief Update the datastore to the target version
 * 
 * @param datastore_version The current version of the datastore
 * @param target_version The target version to update the datastore to
 * 
 * @returns SL_STATUS_OK if the update was successful
 */
sl_status_t update_datastore(int64_t datastore_version, int64_t target_version);

}  // namespace zpc_database_updater

#endif  //ZPC_DATABASE_UPDATER_HPP
/** @} end zpc_database_updater */
