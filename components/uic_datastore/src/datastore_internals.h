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

#ifndef DATASTORE_INTERNALS_H
#define DATASTORE_INTERNALS_H

#include <sqlite3.h>

// Datastore tables
/// Table for storing integers
#define DATASTORE_TABLE_INT "uic_kv_int"
/// Table for storing arrays
#define DATASTORE_TABLE_BLOB "uic_kv_blob"
/// Table for storing the attribute store
#define DATASTORE_TABLE_ATTRIBUTES "uic_attributes"

// Private variable shared among this component.
extern sqlite3 *db;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Helper function to execute SQL statement and check for errors
 *
 * @param sql SQL-query to execute
 * @returns int sqlite3 error code
 */
int datastore_exec_sql(const char *sql);

/**
 * @brief   Generic datastore table initialization
 *
 * @returns SL_STATUS_OK on success
 * @returns SL_STATUS_FAIL on failure
 */
sl_status_t datastore_table_init();

/**
 * @brief   Attribute table initialization
 *
 * @returns SL_STATUS_OK on success
 * @returns SL_STATUS_FAIL on failure
 */
sl_status_t datastore_attribute_table_init();

/**
 * @brief   Statement initialization for the attribute table
 *
 * @returns SL_STATUS_OK on success
 * @returns SL_STATUS_FAIL on failure
 */
sl_status_t datastore_attribute_statement_init();

/**
 * @brief   Statement teardown for the attribute table
 *
 * @returns SL_STATUS_OK
 */
sl_status_t datastore_attribute_statement_teardown();

#ifdef __cplusplus
}
#endif

#endif  // DATASTORE_INTERNALS_H
