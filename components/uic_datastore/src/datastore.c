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
// Includes from this component
#include "datastore.h"
#include "datastore_internals.h"

// Includes from other components
#include "sl_log.h"
#include "sl_status.h"

// Generic includes
#include <sqlite3.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

// Setup Log ID
#define LOG_TAG "datastore"

////////////////////////////////////////////////////////////////////////////////
// Types/Structs
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  DATASTORE_VALUE_TYPE_INT,
  DATASTORE_VALUE_TYPE_BLOB
} datastore_value_type_t;

////////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////////
sl_status_t datastore_table_init()
{
  // The integer table:
  int rc
    = datastore_exec_sql("CREATE TABLE IF NOT EXISTS " DATASTORE_TABLE_INT " ("
                         "key    TEXT      NOT NULL UNIQUE,"
                         "value  INTEGER,"
                         "PRIMARY KEY(key)"
                         ");");
  if (rc != SQLITE_OK) {
    sqlite3_close(db);
    return SL_STATUS_FAIL;
  }

  // The blob table:
  rc = datastore_exec_sql("CREATE TABLE IF NOT EXISTS " DATASTORE_TABLE_BLOB "("
                          "key    TEXT      NOT NULL UNIQUE,"
                          "value  BLOB,"
                          "PRIMARY KEY(key)"
                          ");");
  if (rc != SQLITE_OK) {
    sqlite3_close(db);
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/**
 * @brief Fetch from the datastore
 *
 * @param key Key to fetch data for
 * @param value Pointer to where to store data
 * @param size If value_type is DATASTORE_VALUE_TYPE_INT this is ignored
 *             If value_type is DATASTORE_VALUE_TYPE_BLOB
 *                In: the size of value, Out: Number of bytes written to value
 * @param value_type type of data to fetch
 * @return sl_status_t
 *          SL_STATUS_OK success,
 *          SL_STATUS_NOT_FOUND key is not found,
 *          SL_STATUS_WOULD_OVERFLOW data is larger than size (only for blob)
 *          SL_STATUS_FAIL other failure,
 */
static sl_status_t datastore_fetch_internal(const datastore_key_t key,
                                            void *value,
                                            unsigned int *size,
                                            datastore_value_type_t value_type)
{
  sl_status_t result = SL_STATUS_FAIL;
  sqlite3_stmt *stmt = NULL;
  const char *sql;
  if (db == NULL) {
    sl_log_error(LOG_TAG,
                 "Datastore is not initialized. Fetching data failed.\n");
    return SL_STATUS_FAIL;
  }
  if (value_type == DATASTORE_VALUE_TYPE_INT) {
    sql = "SELECT value FROM " DATASTORE_TABLE_INT " WHERE key = ?";
  } else {
    sql = "SELECT value FROM " DATASTORE_TABLE_BLOB " WHERE key = ?";
  }
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    sl_log_error(LOG_TAG,
                 "SQL Error: Failed to execute statement: %s\n",
                 sqlite3_errmsg(db));
    return SL_STATUS_FAIL;
  }
  sqlite3_bind_text(stmt, 1, key, -1, NULL);
  int step = sqlite3_step(stmt);
  if (step == SQLITE_ROW) {
    if (value_type == DATASTORE_VALUE_TYPE_INT) {
      *(int64_t *)value = sqlite3_column_int64(stmt, 0);
      result            = SL_STATUS_OK;
    } else {
      const uint8_t *tmp_res = (const uint8_t *)sqlite3_column_blob(stmt, 0);
      unsigned int tmp_len   = sqlite3_column_bytes(stmt, 0);
      if (tmp_len <= *size) {
        memcpy(value, tmp_res, tmp_len);
        *size  = tmp_len;
        result = SL_STATUS_OK;
      } else {
        result = SL_STATUS_WOULD_OVERFLOW;
      }
    }
  } else {
    result = SL_STATUS_NOT_FOUND;
  }
  sqlite3_finalize(stmt);
  return result;
}

/**
 * @brief Internal function to handle storing to the datastore
 *
 * @param key Key to store data for
 * @param value Pointer to data to store
 * @param size If value_type is DATASTORE_VALUE_TYPE_INT this is ignored
 *             If value_type is DATASTORE_VALUE_TYPE_BLOB, size of value
 * @param value_type type of data to store
 * @return sl_status_t SQLITE_OK on success, SL_STATUS_FAIL on failure
 */
static sl_status_t datastore_store_internal(const datastore_key_t key,
                                            const void *value,
                                            unsigned int size,
                                            datastore_value_type_t value_type)
{
  sl_status_t result = SL_STATUS_OK;
  sqlite3_stmt *stmt = NULL;
  const char *sql;
  if (db == NULL) {
    sl_log_error(LOG_TAG,
                 "Datastore is not initialized. Writing data failed.\n");
    return SL_STATUS_FAIL;
  }
  if (value_type == DATASTORE_VALUE_TYPE_INT) {
    sql = "REPLACE INTO " DATASTORE_TABLE_INT " VALUES(?, ?)";
  } else {
    sql = "REPLACE INTO " DATASTORE_TABLE_BLOB " VALUES(?, ?)";
  }
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    sl_log_error(LOG_TAG, "prepare failed: %s\n", sqlite3_errmsg(db));
    result = SL_STATUS_FAIL;
  } else {
    rc = sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
      sl_log_error(LOG_TAG, "bind failed: %s\n", sqlite3_errmsg(db));
      result = SL_STATUS_FAIL;
    }
    if (rc == SQLITE_OK) {
      if (value_type == DATASTORE_VALUE_TYPE_INT)
        rc = sqlite3_bind_int64(stmt, 2, *((const int64_t *)value));
      else
        rc = sqlite3_bind_blob(stmt, 2, value, size, SQLITE_STATIC);
      if (rc != SQLITE_OK) {
        sl_log_error(LOG_TAG, "bind failed: %s\n", sqlite3_errmsg(db));
        result = SL_STATUS_FAIL;
      }
    }
    if (rc == SQLITE_OK) {
      rc = sqlite3_step(stmt);
      if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        sl_log_error(LOG_TAG, "execution failed: %s\n", sqlite3_errmsg(db));
        result = SL_STATUS_FAIL;
      }
    }
  }
  sqlite3_finalize(stmt);
  return result;
}

/**
 * @brief Test if a table contains a key
 *
 * @param table Table to look in
 * @param key Key to look for
 * @return true key is found
 * @return false key is not found
 */
static bool datastore_contains_internal(const char *table,
                                        const datastore_key_t key)
{
  char sql[100]      = {0};
  bool result        = false;
  sqlite3_stmt *stmt = NULL;
  if (db == NULL) {
    sl_log_error(LOG_TAG,
                 "Datastore is not initialized. "
                 "Reading data existence failed.\n");
    return false;
  }
  snprintf(sql, sizeof(sql), "SELECT value FROM %s WHERE key = ?", table);
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
  if (rc == SQLITE_OK) {
    rc = sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
      sl_log_debug(LOG_TAG, "Failed to bind: %d\n", rc);
    }
  } else {
    sl_log_error(LOG_TAG,
                 "SQL Error: Failed to execute statement: %s\n",
                 sqlite3_errmsg(db));
  }
  if (rc == SQLITE_OK) {
    int step = sqlite3_step(stmt);
    if (step == SQLITE_ROW) {
      result = true;
    }
  }
  sqlite3_finalize(stmt);
  return result;
}

////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////
sl_status_t datastore_store_int(const datastore_key_t key, int64_t value)
{
  return datastore_store_internal(key, &value, 0, DATASTORE_VALUE_TYPE_INT);
}

sl_status_t datastore_fetch_int(const datastore_key_t key, int64_t *value)
{
  return datastore_fetch_internal(key, value, NULL, DATASTORE_VALUE_TYPE_INT);
}

sl_status_t datastore_store_arr(const datastore_key_t key,
                                const uint8_t *value,
                                unsigned int size)
{
  return datastore_store_internal(key, value, size, DATASTORE_VALUE_TYPE_BLOB);
}

sl_status_t datastore_fetch_arr(const datastore_key_t key,
                                uint8_t *value,
                                unsigned int *size)
{
  return datastore_fetch_internal(key, value, size, DATASTORE_VALUE_TYPE_BLOB);
}

bool datastore_contains_int(const datastore_key_t key)
{
  return datastore_contains_internal(DATASTORE_TABLE_INT, key);
}

bool datastore_contains_arr(const datastore_key_t key)
{
  return datastore_contains_internal(DATASTORE_TABLE_BLOB, key);
}
