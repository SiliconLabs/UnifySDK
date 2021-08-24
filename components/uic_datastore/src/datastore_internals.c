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

// Includes from other components
#include "sl_log.h"
#include "sl_status.h"

// Generic includes
#include <sqlite3.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

// Setup Log ID
#define LOG_TAG "datastore_internal"

////////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////////
int datastore_exec_sql(const char *sql)
{
  char *err_msg = NULL;
  int rc        = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
  if (rc != SQLITE_OK) {
    if (err_msg != NULL) {
      sl_log_error(LOG_TAG, "SQL Error: %s\n", err_msg);
    } else {
      sl_log_error(LOG_TAG, "SQL Error: Unknown\n");
    }
    sqlite3_free(err_msg);
  }
  return rc;
}
