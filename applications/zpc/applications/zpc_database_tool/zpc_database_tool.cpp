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
#include "zpc_attribute_store.h"

#include "attribute_store_fixt.h"
#include "datastore_fixt.h"
#include "config.h"
#include "zpc_config.h"
#include "uic_version.h"

#include "attribute.hpp"
#include "attribute_store.h"
#include "attribute_store_debug.h"
#include "sl_log.h"
#include "sl_status.h"

#include <nlohmann/json.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
using namespace attribute_store;
using namespace std;

constexpr const char *LOG_TAG = "zpc_database_tool";

static nlohmann::json dump_data_store(const attribute &node)
{
  nlohmann::json jsn;

  const char *type_str = attribute_store_name_by_type(node.type());
  jsn["type"] = std::string(type_str);

  vector<uint8_t> reported = node.get<vector<uint8_t>>(REPORTED_ATTRIBUTE);
  vector<uint8_t> desired  = node.get<vector<uint8_t>>(DESIRED_ATTRIBUTE);
  if (reported.size() > 0) {
    std::stringstream ss;

    for (auto i : reported)
      ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(i);

    jsn["reported"] = ss.str();
  }
  if (desired.size() > 0) {
    std::stringstream ss;

    for (auto i : desired)
      ss << std::hex << static_cast<int>(i);

    jsn["desired"] = desired;
  }

  nlohmann::json child_jsn;
  for (auto child: node.children()) {
    child_jsn.push_back(dump_data_store(child));
  }

  if (child_jsn.size() > 0) {
    jsn["children"] = child_jsn;
  }

  return jsn;
}

/**
 * @brief Checks the desired/reported string from json payload for validity.
 *        Valid string is a string that contains only hex numbers chars (a-h, A-H, 0-9)
 *        and has an has an even number of characters >= 2.
 *
 * @param str string that should be checked
 *
 * @return true if string is valid, false othervise.
 */
static bool is_reported_desired_str_valid(const std::string &str)
{
  if ((str.size() < 2) || (str.size() % 2 != 0)) {
    return false;
  }

  for (auto chr : str) {
    if (!((chr >= 'a' && chr <= 'h') ||
          (chr >= 'A' && chr <= 'H') ||
          (chr >= '0' && chr <= '9'))) {
      return false;
    }
  }

  return true;
}

/**
 * @brief Converts string with hex values to vector of uint8_t values.
 *
 * @param str     string that should be converted
 * @param vect    vector that will be written
 * @param log_tag determines logger message in a case of failure, should be "desired"/"reported"
 *
 * @return SL_STATUS_OK if success, error code otherwise.
 */
static sl_status_t reported_desired_str_to_vector(const std::string &str, std::vector<uint8_t> &vect, const char *log_tag)
{
  if (false == is_reported_desired_str_valid(str)) {
    sl_log_error("zpc_database_tool", "Provided invalid '%s' value: '%s'", log_tag, str.c_str());
    return SL_STATUS_INVALID_PARAMETER;
  }

  const char *ptr = str.c_str();

  for (size_t i = 0; i < str.size() - 1; i += 2) {
    char buf[2];
    buf[0] = ptr[i];
    buf[1] = ptr[i + 1];

    try {
      vect.push_back(static_cast<uint8_t>(std::stoi(buf, nullptr, 16)));
    } catch ([[maybe_unused]] const std::invalid_argument & e) {
      sl_log_error("zpc_database_tool", "Provided invalid '%s' value: '%s'", log_tag, str.c_str());
      return SL_STATUS_INVALID_PARAMETER;
    }
  }

  return SL_STATUS_OK;
}

static void import_data_store(attribute &parent, nlohmann::json &jsn)
{
  if (jsn["type"].is_null()) {
    sl_log_error("zpc_database_tool", "Missing type attribute in json");
    return;
  }
  std::string type_str = jsn["type"].get<std::string>();
  attribute_store_type_t type = attribute_store_type_by_name(type_str.c_str());
  attribute node;

  if (type == ATTRIBUTE_TREE_ROOT) {
    node = parent;
  } else {
    node = parent.add_node(type);

    if (!jsn["reported"].is_null()) {
      vector<uint8_t> reported_arr;

      if (SL_STATUS_OK != reported_desired_str_to_vector(
            jsn["reported"].get<std::string>(),
            reported_arr,
            "reported")) {
        return;
      }

      node.set<vector<uint8_t>>(REPORTED_ATTRIBUTE, reported_arr);
    }

    if (!jsn["desired"].is_null()) {
      std::string desired_str = jsn["desired"].get<std::string>();
      vector<uint8_t> desired_arr;

      if (SL_STATUS_OK != reported_desired_str_to_vector(desired_str, desired_arr, "desired"))
        return;

      node.set<vector<uint8_t>>(DESIRED_ATTRIBUTE, desired_arr);
    }
  }

  if (!jsn["children"].is_null()) {
    const auto &children = jsn["children"];

    for (auto child: children) {
      import_data_store(node, child);
    }
  }
}

int main(int argc, char *argv[])
{
  const char *import_file;
  const char *export_file;

  config_add_string(CONFIG_KEY_ZPC_DATASTORE_FILE,
                    "ZPC datastore database file",
                    DEFAULT_ZPC_DATASTORE_FILE);

  config_add_string("import",
                    "Import JSON file. If the database file already exists, "
                    "its contents will be erased and replaced with the data "
                    "from the import JSON file.",
                    "");
  config_add_string("export",
                    "Export JSON file. The database file contents will "
                    "be written to the export file.",
                    "");

  if (SL_STATUS_OK != config_parse(argc, argv, UIC_VERSION)) {
    sl_log_info(LOG_TAG,
                "Could not parse configuration. "
                "Aborting Import/Export operation.");
    return 1;
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

  if (SL_STATUS_OK != datastore_fixt_setup(datastore_file)) {
    sl_log_warning(LOG_TAG,
                   "Could not initialize database. Please verify "
                   "configuration. Aborting Import/Export operation.");
    return 1;
  }

  attribute_store_init();
  zpc_attribute_store_init();

  config_get_as_string("import", &import_file);
  config_get_as_string("export", &export_file);

  if ((strlen(import_file) > 0) && (strlen(export_file) > 0)) {
    sl_log_info(LOG_TAG,
                "Import and Export functionalities are mutually exclusive. "
                "Please perform one operation at a time.");
    return 1;
  }

  if (strlen(import_file) > 0) {
    //Fist wipe datastore
    attribute::root().delete_node();
    attribute root = attribute::root();

    std::ifstream in_fs(import_file);
    nlohmann::json jsn = nlohmann::json::parse(in_fs);
    import_data_store(root, jsn);
    sl_log_info(LOG_TAG, "Database was successfully imported.");
  }

  if (strlen(export_file) > 0) {
    auto jsn = dump_data_store(attribute::root());
    std::ofstream out(export_file);
    out << jsn.dump(4);
    sl_log_info(LOG_TAG, "Database was successfully exported.");
  }

  datastore_fixt_teardown();
  return 0;
}
