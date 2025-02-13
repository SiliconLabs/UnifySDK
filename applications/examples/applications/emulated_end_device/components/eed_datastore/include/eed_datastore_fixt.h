/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup eed_datastore EED Datastore Fixture
 * @ingroup eed_components
 * @brief EED specific functions using the @ref unify_datastore.
 *
 * Fixture to initialize the eed_datastore from uic_main,
 * to be used in eed_datastore
 *
 * @{
 */

#ifndef EED_DATASTORE_FIXT_H
#define EED_DATASTORE_FIXT_H

#include "sl_status.h"

/**
 * @brief Versioning of the EED datastore.
 * Each datastore version identifies the unique schema of the attribute store
 * The version number should be bumped up when new schema is introduced in
 * datastore which is no more compatible with current or old schemas.
 */
typedef enum eed_datastore_version {
  /**
  * @brief Initial EED datastore revision at first release
  */
  DATASTORE_VERSION_V1 = 1,
  /**
   * @brief Keep this value after the latest version to detect what is the latest
   */
  DATASTORE_VERSION_LAST,
} eed_datastore_version_t;

/**
 * @brief Current datastore version used by the EED.
 */
#define EED_DATASTORE_VERSION (DATASTORE_VERSION_LAST - 1)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fixture for setting up the eed_datastore component.
 *
 * @return SL_STATUS_OK for success, SL_STATUS_FAIL if an error occurred
 */
sl_status_t eed_datastore_fixt_setup();

#ifdef __cplusplus
}
#endif

/** @} end of eed_datastore*/
#endif  // EED_DATASTORE_FIXT_H
