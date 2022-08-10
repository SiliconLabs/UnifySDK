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
 * @defgroup datastore_fixt Unify Datastore Fixture
 * @ingroup unify_datastore
 * @brief Persistent Datastore fixture
 *
 * Fixture to initialize the datastore from uic_main,
 * in order to enable the Datastore in a Unify application,
 * add \ref datastore_fixt_setup to the fixt_setup array,
 * when calling \ref uic_main
 *
 * @{
 */

#ifndef DATASTORE_FIXT_H
#define DATASTORE_FIXT_H

#include <stdbool.h>
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief: MAX Length of typical version string: for e.g. ver_0.0.6_RC1-269-gb5a9d4be
 */
#define UIC_VERSION_MAX_LEN 100

/**
 * @brief Fixture for setting up the datastore component.
 *
 * This function gets the config string for datastore file and opens the
 * datastore (Sqlite database)
 * then calls \ref datastore_table_init
 * And calls datastore_attribure_init
 *
 * @param datastore_file path for the datastore file.
 *
 * @return SL_STATUS_OK for success, SL_STATUS_FAIL if an error occurred
 */
sl_status_t datastore_fixt_setup(const char *datastore_file);

/**
 * @brief Fixture to tear down the datastore.
 *
 * This is basically a wrapper that calls \ref datastore_teardown,
 * see \ref datastore_teardown for more details
 *
 * @return int 0 for success, -1 for failure
 */
int datastore_fixt_teardown();
/**
 * @brief Function for setting up the datastore component handling the
 * datastore version.
 *
 * This function calls reads the datastore config option and calls
 * \ref datastore_open which opens the database
 * and calls \ref datastore_table_init initializes generic tables and
 * then this function checks if the database version
 * is same as current version. (Generic tables have the database version stored)
 *
 * If the versioning is good then writes the current Unify version in generic
 * table and calls datastore_attribure_init to initialize attribute store
 * table
 * If there is no database version found, it writes the current database version
 * and writes the current Unify version in generic table
 * and calls datastore_attribure_init to initialize attribute store
 *
 *  @param datastore_file path for the datastore file.
 *  @param db_version                Expected version number for the database file
 *
 * @return SL_STATUS_OK for success, SL_STATUS_FAIL if an error occurred
 */
sl_status_t
  datastore_fixt_setup_and_handle_version(const char *datastore_file,
                                          int64_t db_version);
#ifdef __cplusplus
}
#endif

/** @} end of datastore */
#endif  // DATASTORE_FIXT_H
