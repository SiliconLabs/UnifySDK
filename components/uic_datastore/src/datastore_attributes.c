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
// Includes from this component
#include "datastore.h"
#include "datastore_internals.h"
#include "datastore_attributes.h"

// Includes from other components
#include "sl_log.h"
#include "sl_status.h"

// Generic Includes
#include <sqlite3.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

// Setup Log ID
#define LOG_TAG "datastore_attributes"

////////////////////////////////////////////////////////////////////////////////
// Private variables
////////////////////////////////////////////////////////////////////////////////
static sqlite3_stmt *upsert_statement             = NULL;
static sqlite3_stmt *select_statement             = NULL;
static sqlite3_stmt *select_child_index_statement = NULL;
static sqlite3_stmt *delete_statement             = NULL;

////////////////////////////////////////////////////////////////////////////////
// Private helper functions
////////////////////////////////////////////////////////////////////////////////
static void inline log_database_not_initialized()
{
  sl_log_error(LOG_TAG, "Database is not initialized\n");
}

static void inline log_binding_failed()
{
  sl_log_error(LOG_TAG, "Binding operation failed: %s\n", sqlite3_errmsg(db));
}

////////////////////////////////////////////////////////////////////////////////
// Init/teardown functions
////////////////////////////////////////////////////////////////////////////////
sl_status_t datastore_attribute_table_init()
{
  // Don't try to execute anything if the database is not ready
  if (db == NULL) {
    log_database_not_initialized();
    return SL_STATUS_FAIL;
  }
  // We need to use foreign keys for the attribute tables
  // (preventing create a node that does not have a valid parent)
  // SQLite does not activate foreign_keys enforcement by default.
  int rc = datastore_exec_sql("PRAGMA foreign_keys = ON;");
  if (rc != SQLITE_OK) {
    sqlite3_close(db);
    return SL_STATUS_FAIL;
  }

  // The attribute store table:
  rc = datastore_exec_sql(
    "CREATE TABLE IF NOT EXISTS " DATASTORE_TABLE_ATTRIBUTES "("
    " id              UNSIGNED INT, "  // (UNIQUE NOT NULL inferred by PRIMARY KEY down below)
    " type            UNSIGNED INT NOT NULL, "
    " parent_id       UNSIGNED INT, "  // parent_id can be NULL for the root node
    " reported_value  BLOB, "          // This is the reported value in full.
    " desired_value   BOOL DEFAULT false, "  // This captures if the desired value is defined
    " PRIMARY KEY(id) "
    // parent_id has to be an existing id in the same table (or NULL)
    " FOREIGN KEY (parent_id) REFERENCES " DATASTORE_TABLE_ATTRIBUTES " (id),"
    " CHECK (type > 0), "
    " CHECK (id > 0), "
    " CHECK (id != parent_id));");  // Last condition: We do not want to allow nodes being their own parent.
  if (rc != SQLITE_OK) {
    sqlite3_close(db);
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t datastore_attribute_statement_init()
{
  if (db == NULL) {
    log_database_not_initialized();
    return SL_STATUS_FAIL;
  }

  // Initialize our private statements
  // Upsert:
  const char *upsert_sql
    = "INSERT INTO " DATASTORE_TABLE_ATTRIBUTES
      " (id, type, parent_id, reported_value, desired_value) "
      " VALUES( ?, ?, ?, ?, ? ) "
      " ON CONFLICT(id) DO UPDATE SET type = excluded.type,"
      " parent_id = excluded.parent_id, "
      " reported_value =  excluded.reported_value,"
      " desired_value = excluded.desired_value; ";

  int rc = sqlite3_prepare_v2(db, upsert_sql, -1, &upsert_statement, NULL);
  if (rc != SQLITE_OK) {
    sl_log_error(LOG_TAG,
                 "Prepare Upsert statement failed: %s\n",
                 sqlite3_errmsg(db));
    return SL_STATUS_FAIL;
  }

  // Select statement:
  const char *select_sql
    = "SELECT id, type, parent_id, reported_value, "
      "desired_value FROM " DATASTORE_TABLE_ATTRIBUTES " WHERE id = ?";

  rc = sqlite3_prepare_v2(db, select_sql, -1, &select_statement, NULL);
  if (rc != SQLITE_OK) {
    sl_log_error(LOG_TAG,
                 "Prepare Select statement failed: %s\n",
                 sqlite3_errmsg(db));
    return SL_STATUS_FAIL;
  }

  // Select child index statement:
  // LIMIT ?,1 is to fetch a particular row
  // (child_index, 0 for the first, 1 for the second, etc.)
  const char *select_child_index_sql
    = "SELECT id, type, reported_value, "
      "desired_value FROM " DATASTORE_TABLE_ATTRIBUTES
      " WHERE parent_id = ? ORDER BY id LIMIT ?,1";

  rc = sqlite3_prepare_v2(db,
                          select_child_index_sql,
                          -1,
                          &select_child_index_statement,
                          NULL);
  if (rc != SQLITE_OK) {
    sl_log_error(LOG_TAG,
                 "Prepare Select Child Index statement failed: %s\n",
                 sqlite3_errmsg(db));
    return SL_STATUS_FAIL;
  }

  // Delete statement:
  const char *delete_sql
    = "DELETE FROM " DATASTORE_TABLE_ATTRIBUTES " WHERE id = ?;";

  rc = sqlite3_prepare_v2(db, delete_sql, -1, &delete_statement, NULL);

  if (rc != SQLITE_OK) {
    sl_log_error(LOG_TAG,
                 "Prepare Delete statement failed: %s\n",
                 sqlite3_errmsg(db));
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t datastore_attribute_statement_teardown()
{
  // Free up the allocated memory for these statements
  sqlite3_finalize(upsert_statement);
  sqlite3_finalize(select_statement);
  sqlite3_finalize(select_child_index_statement);
  sqlite3_finalize(delete_statement);

  // Set them back to NULL, so that we can detect if they are missing a re-init
  upsert_statement             = NULL;
  select_statement             = NULL;
  select_child_index_statement = NULL;
  delete_statement             = NULL;

  // Teardown always goes well
  return SL_STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////
sl_status_t datastore_store_attribute(datastore_attribute_id_t id,
                                      uint32_t type,
                                      uint32_t parent_id,
                                      const uint8_t *value,
                                      uint8_t value_size,
                                      bool desired_value)
{
  sl_status_t result = SL_STATUS_OK;

  // Verify that the db is active
  if (db == NULL) {
    log_database_not_initialized();
    return SL_STATUS_FAIL;
  }

  // Bind the id
  int rc = sqlite3_bind_int64(upsert_statement, 1, id);
  if (rc != SQLITE_OK) {
    log_binding_failed();
    result = SL_STATUS_FAIL;
  }

  // Bind the attribute type
  rc = sqlite3_bind_int64(upsert_statement, 2, type);
  if (rc != SQLITE_OK) {
    log_binding_failed();
    result = SL_STATUS_FAIL;
  }

  // Bind the parent_id, insert NULL value if set to 0x00
  if (parent_id == 0x00) {
    rc = sqlite3_bind_null(upsert_statement, 3);
  } else {
    rc = sqlite3_bind_int64(upsert_statement, 3, parent_id);
  }
  if (rc != SQLITE_OK) {
    log_binding_failed();
    result = SL_STATUS_FAIL;
  }

  // Bind the reported_value (blob object)
  rc = sqlite3_bind_blob(upsert_statement, 4, value, value_size, SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    log_binding_failed();
    result = SL_STATUS_FAIL;
  }

  // Bind the desired_value (boolean, we use int for this)
  rc = sqlite3_bind_int(upsert_statement, 5, (int)desired_value);
  if (rc != SQLITE_OK) {
    log_binding_failed();
    result = SL_STATUS_FAIL;
  }

  if (result == SL_STATUS_OK) {
    rc = sqlite3_step(upsert_statement);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
      sl_log_error(LOG_TAG,
                   "Step operation failed: %s, Query: %s",
                   sqlite3_errmsg(db),
                   sqlite3_expanded_sql(upsert_statement));
      result = SL_STATUS_FAIL;
    }
  }

  // We are done, reset the statement so we can execute it again
  sqlite3_reset(upsert_statement);

  return result;
}

sl_status_t datastore_fetch_attribute(datastore_attribute_id_t id,
                                      uint32_t *type,
                                      datastore_attribute_id_t *parent_id,
                                      uint8_t *value,
                                      uint8_t *value_size,
                                      bool *desired_value)
{
  sl_status_t status = SL_STATUS_FAIL;

  if (db == NULL) {
    log_database_not_initialized();
    return status;
  }

  int rc = sqlite3_bind_int64(select_statement, 1, id);
  if (rc != SQLITE_OK) {
    log_binding_failed();
    return status;
  }

  // Execute the query
  int step = sqlite3_step(select_statement);
  if (step == SQLITE_ROW) {
    // Pull the result from the first row (there should be only one)
    *type = (uint32_t)sqlite3_column_int64(select_statement, 1);
    // If parent_id is NULL in the database, it will return 0
    *parent_id     = (uint32_t)sqlite3_column_int64(select_statement, 2);
    *desired_value = sqlite3_column_int(select_statement, 4);

    // Get a pointer from sqlite and copy the data to the user pointer
    const uint8_t *value_buffer
      = (const uint8_t *)sqlite3_column_blob(select_statement, 3);
    *value_size = (uint8_t)sqlite3_column_bytes(select_statement, 3);
    memcpy(value, value_buffer, *value_size);
    status = SL_STATUS_OK;
  } else {
    // No result was found for the id
    status = SL_STATUS_NOT_FOUND;
  }

  // We are done, reset so we can execute the query again
  sqlite3_reset(select_statement);
  return status;
}

sl_status_t datastore_fetch_attribute_child(datastore_attribute_id_t parent_id,
                                            uint32_t child_index,
                                            datastore_attribute_id_t *child_id,
                                            uint32_t *type,
                                            uint8_t *value,
                                            uint8_t *value_size,
                                            bool *desired_value)
{
  sl_status_t status = SL_STATUS_FAIL;

  if (db == NULL) {
    log_database_not_initialized();
    return status;
  }

  // Fill up those question marks from the SQL statements
  int rc = sqlite3_bind_int64(select_child_index_statement, 1, parent_id);
  if (rc != SQLITE_OK) {
    log_binding_failed();
    return status;
  }
  rc = sqlite3_bind_int64(select_child_index_statement, 2, child_index);
  if (rc != SQLITE_OK) {
    log_binding_failed();
    return status;
  }

  // Execute the query
  int step = sqlite3_step(select_child_index_statement);

  if (step == SQLITE_ROW) {
    // Pull the result from the first row (there should be only one)
    *child_id = (uint32_t)sqlite3_column_int64(select_child_index_statement, 0);
    *type     = (uint32_t)sqlite3_column_int64(select_child_index_statement, 1);
    *desired_value = sqlite3_column_int(select_child_index_statement, 3);

    // Get a pointer from sqlite and copy the data to the user pointer
    const uint8_t *value_buffer
      = (const uint8_t *)sqlite3_column_blob(select_child_index_statement, 2);
    *value_size
      = (uint8_t)sqlite3_column_bytes(select_child_index_statement, 2);
    memcpy(value, value_buffer, *value_size);
    status = SL_STATUS_OK;
  } else {
    // No result was found for the parent_id / child_index
    status = SL_STATUS_NOT_FOUND;
  }

  sqlite3_reset(select_child_index_statement);
  return status;
}

bool datastore_contains_attribute(datastore_attribute_id_t id)
{
  bool result = false;

  if (db == NULL) {
    log_database_not_initialized();
    return result;
  }

  int rc = sqlite3_bind_int(select_statement, 1, id);
  if (rc != SQLITE_OK) {
    log_binding_failed();
    return result;
  }

  int step = sqlite3_step(select_statement);
  if (step == SQLITE_ROW) {
    result = true;
  }

  sqlite3_reset(select_statement);
  return result;
}

sl_status_t datastore_delete_attribute(const datastore_attribute_id_t id)
{
  sl_status_t result = SL_STATUS_FAIL;

  if (db == NULL) {
    log_database_not_initialized();
    return result;
  }

  int rc = sqlite3_bind_int(delete_statement, 1, id);
  if (rc != SQLITE_OK) {
    log_binding_failed();
    return result;
  }

  int step = sqlite3_step(delete_statement);
  if (step == SQLITE_DONE || step == SQLITE_ROW) {
    result = SL_STATUS_OK;
  }

  sqlite3_reset(delete_statement);
  return result;
}

sl_status_t
  datastore_delete_attribute_with_children(const datastore_attribute_id_t id)
{
  sl_status_t result = SL_STATUS_FAIL;

  if (db == NULL) {
    log_database_not_initialized();
    return result;
  }

  int rc = sqlite3_bind_int64(delete_statement, 1, id);
  if (rc != SQLITE_OK) {
    log_binding_failed();
    return result;
  }

  int step = sqlite3_step(delete_statement);
  if (step == SQLITE_DONE || step == SQLITE_ROW) {
    result = SL_STATUS_OK;
  }

  sqlite3_reset(delete_statement);
  return result;
}

sl_status_t datastore_delete_all_attributes()
{
  if (db == NULL) {
    log_database_not_initialized();
    return SL_STATUS_FAIL;
  }

  int rc = datastore_exec_sql("DELETE FROM " DATASTORE_TABLE_ATTRIBUTES ";");

  if (rc != SQLITE_OK) {
    sl_log_error(LOG_TAG,
                 "SQL Error: Failed to execute statement: %s\n",
                 sqlite3_errmsg(db));
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}