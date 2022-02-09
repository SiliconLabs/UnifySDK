/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup zigpc_datastore_fixture ZigPC Config Fixture
 * @brief ZigPC Datastore Configuration fixture.
 *
 * Fixture to initialize the datastore from uic_main, with the proper configuration
 * for the ZigPC.
 *
 * @{
 */

#ifndef ZIGPC_DATASTORE_FIXT_H
#define ZIGPC_DATASTORE_FIXT_H

#include "sl_status.h"

/**
 * @brief Versioning of the ZigPC datastore.
 *
 * Each datastore version identifies the unique schema of the attribute store
 * The version number should be bumped up when new schema is introduced in
 * datastore which is no more compatible with current or old schemas.
 */
typedef enum zigpc_datastore_version {
  /**
  * @brief Initial ZigPC datastore revision at first release
  */
  ZIGPC_DATASTORE_VERSION_V1 = 1,
  /**
   * @brief Keep this value after the latest version to detect what is the latest
   */
  ZIGPC_DATASTORE_VERSION_LAST,
} zigpc_datastore_version_t;

/**
 * @brief Current datastore version used by the ZigPC.
 */
#define ZIGPC_CURRENT_DATASTORE_VERSION (ZIGPC_DATASTORE_VERSION_LAST - 1)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fixture for setting up the zigpc_datastore component.
 *
 * @return SL_STATUS_OK for success, SL_STATUS_FAIL if an error occurred.
 */
sl_status_t zigpc_uic_datastore_fixt_setup(void);

#ifdef __cplusplus
}
#endif

/** @} end of zigpc_datastore*/
#endif  // ZIGPC_DATASTORE_FIXT_H
