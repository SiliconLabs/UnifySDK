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
 * @defgroup aoxpc_datastore AoXPC Datastore Fixture
 * @brief AoXPC specific functions using the @ref unify_datastore.
 *
 * Fixture to initialize the aoxpc_datastore from uic_main,
 *
 * @{
 */

#ifndef AOXPC_DATASTORE_FIXT_H
#define AOXPC_DATASTORE_FIXT_H

// Interface includes
#include "uic_version.h"
#include "sl_status.h"

/**
 * @brief Versioning of the AoXPC datastore.
 * Each datastore version identifies the unique schema of the SQL Schema / set
 * of keys/values that is saved using the datastore.
 * The version number should be incremented when new schema is introduced in
 * datastore which is no more compatible with current or old schemas.
 */
typedef enum aoxpc_datastore_version {
  /**
  * @brief Initial AoXPC datastore revision at first release
  */
  DATASTORE_VERSION_V1 = 1,
  /**
   * @brief Keep this value after the latest version to detect what is the latest
   */
  DATASTORE_VERSION_LAST,
} aoxpc_datastore_version_t;

/**
 * @brief Current datastore version used by the AoXPC.
 */
#define AOXPC_DATASTORE_VERSION (DATASTORE_VERSION_LAST - 1)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fixture for setting up the aoxpc_datastore component.
 *
 * @return SL_STATUS_OK for success, SL_STATUS_FAIL if an error occurred
 */
sl_status_t aoxpc_datastore_fixt_setup();

#ifdef __cplusplus
}
#endif

/** @} end of aoxpc_datastore*/
#endif  // AOXPC_DATASTORE_FIXT_H
