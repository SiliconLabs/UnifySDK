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
#include "datastore_fixt.h"
#include "datastore_internals.h"

// Includes from other components
#include "sl_log.h"
#include "sl_status.h"
#include "uic_version.h"

// Generic includes
#include <sqlite3.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

// Setup Log ID
#define LOG_TAG "datastore_fixt"

////////////////////////////////////////////////////////////////////////////////
// Private Variables
////////////////////////////////////////////////////////////////////////////////
sqlite3 *db = NULL;

////////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////////

static sl_status_t datastore_attribute_init()
{
  // Initialize the attribute table.
  if (datastore_attribute_table_init() != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  // Prepare all our statements:
  if (datastore_attribute_statement_init() != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

static sl_status_t datastore_open(const char *datastore_file)
{
  if (db != NULL) {
    // We are already initialized, teardown before attempting init again.
    sl_status_t teardown_status = datastore_fixt_teardown();
    if (teardown_status != SL_STATUS_OK) {
      return teardown_status;
    }
  }

  int rc = sqlite3_initialize();
  if (rc != SQLITE_OK) {
    sl_log_error(LOG_TAG, "Cannot initialize SQLite. Return code: %d\n", rc);
    return false;
  }

  sqlite3_stmt *stmt = NULL;
  sl_log_info(LOG_TAG, "Using datastore file: %s\n", datastore_file);
  int sqlite_flags = SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE;
  if (strncmp(":memory:", datastore_file, sizeof(":memory:")) == 0) {
    sqlite_flags |= SQLITE_OPEN_MEMORY;
  }
  rc = sqlite3_open_v2(datastore_file, &db, sqlite_flags, NULL);
  if (rc != SQLITE_OK) {
    sl_log_error(LOG_TAG, "Cannot open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return SL_STATUS_FAIL;
  }

  rc = sqlite3_prepare_v2(db, "SELECT SQLITE_VERSION()", -1, &stmt, 0);
  if (rc != SQLITE_OK) {
    sl_log_error(LOG_TAG, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return SL_STATUS_FAIL;
  }
  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW) {
    sl_log_debug(LOG_TAG, "SQLITE Version: %s\n", sqlite3_column_text(stmt, 0));
  }
  sqlite3_finalize(stmt);

  return SL_STATUS_OK;
}

sl_status_t datastore_init(const char *datastore_path)
{
  sl_status_t ret = datastore_open(datastore_path);
  if (ret != SL_STATUS_OK) {
    return ret;
  }
  // Create generic tables if not exist
  if (datastore_table_init() != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  return datastore_attribute_init();
}

sl_status_t datastore_teardown()
{
  // Finalize statements that we won't use anymore
  datastore_attribute_statement_teardown();

  // Close the database
  int rc = SQLITE_OK;
  if (db != NULL) {
    rc = sqlite3_close(db);
    db = NULL;
    if (rc != SQLITE_OK) {
      sl_log_error(LOG_TAG,
                   "SQL Error: Failed to execute statement: %s\n",
                   sqlite3_errmsg(db));
      return SL_STATUS_FAIL;
    }
    rc = sqlite3_shutdown();
    if (rc != SQLITE_OK) {
      sl_log_error(LOG_TAG, "Cannot shutdown SQLite. Return code: %d\n", rc);
      return SL_STATUS_FAIL;
    }
  }
  return SL_STATUS_OK;
}

sl_status_t datastore_fixt_setup_and_handle_version(const char *datastore_file,
                                                    int64_t db_version)
{
  sl_status_t ret = datastore_open(datastore_file);
  if (ret != SL_STATUS_OK) {
    return ret;
  }
  // Create generic tables if not exist
  if (datastore_table_init() != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  int64_t version_fetched                          = 0;
  uint8_t fetched_uic_version[UIC_VERSION_MAX_LEN] = {0};
  unsigned int fetched_uic_version_size            = UIC_VERSION_MAX_LEN;
  if (datastore_contains_int("version") == true) {
    datastore_fetch_int("version", &version_fetched);
    sl_log_info(LOG_TAG,
                "Datastore version reported from the datastore file: %" PRId64
                "\n",
                version_fetched);

    datastore_fetch_arr("uic_version",
                        fetched_uic_version,
                        &fetched_uic_version_size);
    sl_log_info(LOG_TAG,
                "Last UIC version that has written in the datastore file: %s\n",
                fetched_uic_version);
    if (version_fetched != db_version) {
      sl_log_error(LOG_TAG,
                   "Datastore version: %" PRId64 " in datastore file is non "
                   "compatible with the datastore version: %" PRId64 ". ",
                   version_fetched,
                   db_version);
      return SL_STATUS_FAIL;
    }
  } else {
    sl_log_info(LOG_TAG,
                "Writing Datastore version: %" PRId64 " for first time",
                db_version);
    datastore_store_int("version", db_version);
  }
  sl_log_info(LOG_TAG,
              "Writing last UIC version: %s to the datastore file\n",
              UIC_VERSION);

  datastore_store_arr("uic_version",
                      (const uint8_t *)UIC_VERSION,
                      sizeof(UIC_VERSION));

  return datastore_attribute_init();
}

////////////////////////////////////////////////////////////////////////////////
// Fixtures for uic_main integration
////////////////////////////////////////////////////////////////////////////////

sl_status_t datastore_fixt_setup(const char *datastore_file)
{
  sl_status_t ret = datastore_open(datastore_file);
  if (ret != SL_STATUS_OK) {
    return ret;
  }
  // Create generic tables if not exist
  if (datastore_table_init() != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  return datastore_attribute_init();
}

int datastore_fixt_teardown()
{
  return datastore_teardown() == SL_STATUS_OK ? 0 : -1;
}
