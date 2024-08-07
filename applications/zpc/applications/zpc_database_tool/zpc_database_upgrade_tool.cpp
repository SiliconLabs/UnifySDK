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
#include "zpc_database_updater.hpp"

// Unify Includes
#include "datastore.h"
#include "datastore_fixt.h"
#include "attribute_store_fixt.h"
#include "config.h"
#include "zpc_config.h"
#include "uic_version.h"
#include "attribute.hpp"
#include "attribute_store.h"
#include "attribute_store_type_registration.h"
#include "sl_log.h"

// ZPC includes
#include "zpc_datastore_fixt.h"
#include "attribute_store_defined_attribute_types.h"

// Cpp includes
#include <string>
#include <filesystem>

using namespace attribute_store;
constexpr const char *LOG_TAG = "zpc_database_upgrade_tool";

///////////////////////////////////////////////////////////////////////////////
// Backup functions
///////////////////////////////////////////////////////////////////////////////
static std::string backup_file;  // NOSONAR
static sl_status_t create_backup(const std::string &datastore_file)
{
  try {
    backup_file = std::string(datastore_file) + ".bak";
    sl_log_info(LOG_TAG, "Creating database backup : %s", backup_file.c_str());

    std::filesystem::copy(datastore_file,
                          backup_file,
                          std::filesystem::copy_options::overwrite_existing);
  } catch (std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Could not create a backup of the datastore file. "
                 "Please verify your configuration. Error: %s",
                 e.what());
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

static sl_status_t remove_backup()
{
  sl_log_info(LOG_TAG, "Removing database backup : %s", backup_file.c_str());
  try {
    std::filesystem::remove(backup_file);
  } catch (std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Could not create a backup of the datastore file. "
                 "Please verify your configuration. Error: %s",
                 e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t init(int argc, char *argv[], int *target_version)
{
  config_add_int("target_version",
                 "Version to convert the current datastore file to.",
                 ZPC_DATASTORE_VERSION);

  config_add_string(CONFIG_KEY_ZPC_DATASTORE_FILE,
                    "ZPC datastore database file",
                    DEFAULT_ZPC_DATASTORE_FILE);

  if (SL_STATUS_OK != config_parse(argc, argv, UIC_VERSION)) {
    sl_log_info(LOG_TAG,
                "Could not parse configuration. "
                "Aborting database upgrade operation.");
    return SL_STATUS_FAIL;
  }

  config_get_as_int("target_version", target_version);
  if (*target_version == 0) {
    sl_log_error(LOG_TAG,
                 "You need to specify a target_version. "
                 "E.g. zpc_database_upgrade_tool --target_version 2 "
                 "Please verify your configuration.");
    return SL_STATUS_FAIL;
  }

  const char *datastore_file;
  if (SL_STATUS_OK
      != config_get_as_string(CONFIG_KEY_ZPC_DATASTORE_FILE, &datastore_file)) {
    sl_log_error(LOG_TAG,
                 "You need to specify a valid datastore file path. "
                 "E.g. zpc_database_tool --" CONFIG_KEY_ZPC_DATASTORE_FILE
                 " " UIC_VAR_DIR "/zpc.db "
                 "Please verify your configuration.");
    return SL_STATUS_FAIL;
  }

  // Create backup
  if (SL_STATUS_OK != create_backup(datastore_file)) {
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
  uint8_t return_status = EXIT_SUCCESS;

  int target_version      = 0;
  sl_status_t init_status = init(argc, argv, &target_version);
  if (init_status != SL_STATUS_OK) {
    teardown();
    return 1;
  }

  // Fetch the version of the datastore file.
  int64_t datastore_version = 0;
  if (datastore_contains_int("version") != true) {
    sl_log_error(LOG_TAG,
                 "Cannot read current datastore file version. "
                 "We cannot upgrade such a datastore.");
    teardown();
    return 1;
  }
  datastore_fetch_int("version", &datastore_version);
  sl_log_info(LOG_TAG,
              "Datastore version reported from the datastore file: %d\n",
              datastore_version);

  // Make the conversion, if we know how.
  if (datastore_version == target_version) {
    sl_log_info(LOG_TAG,
                "Datastore is already at version %d. No conversion is needed.",
                datastore_version);
  } else {
    auto update_status
      = zpc_database_updater::update_datastore(datastore_version,
                                               target_version);

    if (update_status != SL_STATUS_OK) {
      return_status = EXIT_FAILURE;
    }
  }

  // Teardown when we are done.
  teardown();

  if (return_status == EXIT_SUCCESS) {
    remove_backup();
    sl_log_info(LOG_TAG, "Database upgrade completed successfully.");
  } else {
    sl_log_error(LOG_TAG, "Database upgrade failed.");
  }

  return return_status;
}
