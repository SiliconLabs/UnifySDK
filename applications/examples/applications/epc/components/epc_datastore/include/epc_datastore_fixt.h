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
 * @defgroup epc_datastore EPC Datastore Fixture
 * @ingroup epc_components
 * @brief EPC specific functions using the @ref unify_datastore.
 *
 * Fixture to initialize the epc_datastore from uic_main,
 * to be used in epc_datastore
 *
 * @{
 */

#ifndef EPC_DATASTORE_FIXT_H
#define EPC_DATASTORE_FIXT_H

#include "sl_status.h"

/**
 * @brief Versioning of the EPC datastore.
 * Each datastore version identifies the unique schema of the attribute store
 * The version number should be bumped up when new schema is introduced in
 * datastore which is no more compatible with current or old schemas.
 */
typedef enum epc_datastore_version {
  /**
  * @brief Initial EPC datastore revision at first release
  */
  DATASTORE_VERSION_V1 = 1,
  /**
   * @brief Keep this value after the latest version to detect what is the latest
   */
  DATASTORE_VERSION_LAST,
} epc_datastore_version_t;

/**
 * @brief Current datastore version used by the EPC.
 */
#define EPC_DATASTORE_VERSION (DATASTORE_VERSION_LAST - 1)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fixture for setting up the zpc_datastore component.
 *
 * @return SL_STATUS_OK for success, SL_STATUS_FAIL if an error occurred
 */
sl_status_t epc_datastore_fixt_setup();

#ifdef __cplusplus
}
#endif

/** @} end of epc_datastore*/
#endif  // EPC_DATASTORE_FIXT_H
