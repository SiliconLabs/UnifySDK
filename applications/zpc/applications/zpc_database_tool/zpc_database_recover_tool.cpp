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
// Tool includes
#include "zpc_database_helper.hpp"

// Unify Includes
#include "datastore.h"
#include "datastore_fixt.h"
#include "attribute_store_fixt.h"
#include "config.h"
#include "uic_version.h"
#include "attribute.hpp"
#include "attribute_store.h"
#include "attribute_store_debug.h"
#include "sl_log.h"

// ZPC includes
#include "zpc_datastore_fixt.h"
#include "zpc_config.h"
#include "attribute_store_defined_attribute_types.h"

using namespace attribute_store;
constexpr const char *LOG_TAG = "zpc_database_recover_tool";

///////////////////////////////////////////////////////////////////////////////
// Init and teardown
///////////////////////////////////////////////////////////////////////////////
static sl_status_t init(int argc, char *argv[], int *target_version)
{
  config_add_int("target_version",
                 "Version to write in the current datastore that we recover.",
                 ZPC_DATASTORE_VERSION);

  config_add_string(CONFIG_KEY_ZPC_DATASTORE_FILE,
                    "Datastore database file",
                    DEFAULT_ZPC_DATASTORE_FILE);

  if (SL_STATUS_OK != config_parse(argc, argv, UIC_VERSION)) {
    sl_log_error(LOG_TAG,
                 "Could not parse configuration. "
                 "Aborting database upgrade Operation.");
    return SL_STATUS_FAIL;
  }

  config_get_as_int("target_version", target_version);
  if (*target_version == 0) {
    sl_log_error(LOG_TAG,
                 "You need to specify a valid target_version. "
                 "E.g. zpc_database_upgrade_tool --target_version 2 "
                 "Please verify your configuration.");
    return SL_STATUS_FAIL;
  }

  const char *datastore_file;
  if (SL_STATUS_OK
      != config_get_as_string(CONFIG_KEY_ZPC_DATASTORE_FILE, &datastore_file)) {
    sl_log_error(
      LOG_TAG,
      "You need to specify a valid datastore file path. "
      "E.g. zpc_database_upgrade_tool --zpc.datastore_file " UIC_VAR_DIR
      "/zpc.db "
      "Please verify your configuration.");
    return SL_STATUS_FAIL;
  }

  if (SL_STATUS_OK != datastore_fixt_setup(datastore_file)) {
    sl_log_error(LOG_TAG,
                 "Could not initialize the datastore. "
                 "Please verify your configuration.");
    return SL_STATUS_FAIL;
  }

  if (SL_STATUS_OK != attribute_store_init()) {
    sl_log_error(LOG_TAG,
                 "Could not initialize the Attribute Store. "
                 "Please verify your configuration.");
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

static void teardown()
{
  attribute_store_teardown();
  datastore_fixt_teardown();
}

///////////////////////////////////////////////////////////////////////////////
// Main
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  int target_version      = 0;
  sl_status_t init_status = init(argc, argv, &target_version);

  if (init_status != SL_STATUS_OK) {
    teardown();
    return 1;
  }

  // Fetch the version of the datastore file.
  int64_t datastore_version = 0;
  datastore_fetch_int("version", &datastore_version);
  sl_log_info(LOG_TAG,
              "Datastore version reported from the datastore file: %d\n",
              datastore_version);

  // Make the recover. Clip everything under endpoints and create fresh NIFs
  sl_log_info(LOG_TAG,
              "Erasing endpoint data in the datastore.\n",
              target_version);
  for (attribute endpoint_id: endpoint_id_list()) {
    // Delete everything under endpoints:
    for (attribute child: endpoint_id.children()) {
      child.delete_node();
    }
    // Create a NIF:
    endpoint_id.add_node(ATTRIBUTE_ZWAVE_NIF);
  }

  sl_log_info(LOG_TAG,
              "Writing version %d to the datastore.\n",
              target_version);
  datastore_store_int("version", target_version);

  // Teardown when we are done.
  teardown();
  return 0;
}
