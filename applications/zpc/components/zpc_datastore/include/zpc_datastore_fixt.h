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
 * @defgroup zpc_datastore ZPC Datastore Fixture
 * @ingroup zpc_components
 * @brief ZPC specific functions using the @ref unify_datastore.
 *
 * Fixture to initialize the zpc_datastore from uic_main,
 * to be used in zpc_datastore
 *
 * @{
 */

#ifndef ZPC_DATASTORE_FIXT_H
#define ZPC_DATASTORE_FIXT_H

#include "sl_status.h"

/**
 * @brief Versioning of the ZPC datastore.
 * Each datastore version identifies the unique schema of the attribute store
 * The version number should be bumped up when new schema is introduced in
 * datastore which is no more compatible with current or old schemas.
 */
typedef enum zpc_datastore_version {
  /**
  * @brief Initial ZPC datastore revision at first release
  */
  DATASTORE_VERSION_V1 = 1,
  /**
   * @brief Version 2 of datastore database.
   * Changes from v1:
   * - Wake Up Command Class attribute tree was updated in a non-compatible way
   * - Assocation Group ID ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID is now
   *   uint8_t instead of uint32_t
  */
  DATASTORE_VERSION_V2,
  /**
   * @brief Keep this value after the latest version to detect what is the latest
   */
  DATASTORE_VERSION_LAST,
} zpc_datastore_version_t;

/**
 * @brief Current datastore version used by the ZPC.
 */
#define ZPC_DATASTORE_VERSION (DATASTORE_VERSION_LAST - 1)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fixture for setting up the zpc_datastore component.
 *
 * @return SL_STATUS_OK for success, SL_STATUS_FAIL if an error occurred
 */
sl_status_t zpc_datastore_fixt_setup();

#ifdef __cplusplus
}
#endif

/** @} end of zpc_datastore*/
#endif  // ZPC_DATASTORE_FIXT_H
