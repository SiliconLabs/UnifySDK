
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

#include "sl_log.h"
#include "attribute_mapper_engine.hpp"
#include "attribute_store_fixt.h"
#include "datastore.h"

#include "attribute_store_test_helpers.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

static const char *LOG_TAG = "attribute_mapper_test";

// Short alias for property_tree namespace
namespace bpt = boost::property_tree;

using namespace attribute_store;

bool build_tree(bpt::ptree branches) {
  // Get branch data, create and init branches
  BOOST_FOREACH(bpt::ptree::value_type &branch, branches) {
    std::string branch_descriptor = branch.second.get<std::string>("branch_descriptor");

    boost::optional<int32_t> v_d = branch.second.get_optional<int32_t>("value.desired");
    boost::optional<int32_t> v_r = branch.second.get_optional<int32_t>("value.reported");

    attribute node = create_branch(branch_descriptor);

    if (v_d) {
      int32_t value = v_d.get();
      node.set_desired<uint32_t>(value);
    }

    if (v_r) {
      int32_t value = v_r.get();
      node.set_reported<uint32_t>(value);
    } 
  }
  return true;
}

bool activate_rules(bpt::ptree tests) {
  BOOST_FOREACH(bpt::ptree::value_type &test, tests) {
    std::string branch_descriptor = test.second.get<std::string>("activator.branch_descriptor");

    boost::optional<int32_t> act_v_d = test.second.get_optional<int32_t>("activator.value.desired");
    boost::optional<int32_t> act_v_r = test.second.get_optional<int32_t>("activator.value.reported");

    attribute node = check_branch(branch_descriptor);

    if (act_v_d) {
      int32_t value = act_v_d.get();
      node.set_desired<uint32_t>(value);
    } else if (act_v_r) {
      int32_t value = act_v_r.get();
      node.set_reported<uint32_t>(value);
    }
  }
  return true;
}

int run_tests(bpt::ptree tests) {
  uint32_t test_errors = 0;

  BOOST_FOREACH(bpt::ptree::value_type &test, tests) {
    int scope_id = test.second.get<int>("scope");
    BOOST_FOREACH(bpt::ptree::value_type &test_assert, test.second.get_child("test_assert.")) {
      std::string branch_descriptor = test_assert.second.get<std::string>("branch_descriptor");
  
      boost::optional<int32_t> a_v_d = test_assert.second.get_optional<int32_t>("value.desired");
      boost::optional<int32_t> a_v_r = test_assert.second.get_optional<int32_t>("value.reported");

      std::cout << "Test for Scope: " << scope_id << std::endl;

      attribute node = check_branch(branch_descriptor);

      if( node.is_valid() ) {
        uint32_t test_value;
        uint32_t actual_value;

        if (a_v_d ) {
          actual_value = node.desired<uint32_t>();
          test_value = (uint32_t)a_v_d.get();

          std::cout << "Expect desired value " << test_value << ", Was " << actual_value;
          if (test_value != actual_value) {
            std::cout << " :FAIL" << std::endl;
            test_errors++;
          } else {
            std::cout << " :PASS" << std::endl;
          }
        }
        if (a_v_r) {
          actual_value = node.reported<uint32_t>();
          test_value = (uint32_t)a_v_r.get();

          std::cout << "Expect reported value " << test_value << ", Was " << actual_value;
          if (test_value != actual_value) {
            std::cout << " :FAIL" << std::endl;
            test_errors++;
          } else {
            std::cout << " :PASS" << std::endl;
          }
        }
      } else {
        return -1;
      }
    }
  }
  return test_errors;
}

 
int main(int argc, char* argv[]) {
  // Check the number of parameters
  if (argc < 2) {
      std::cout << "Usage: " << argv[0] << " <CONFIG_FILE_NAME>" << std::endl;
      return -1;
  }

  std::string config_file_name = argv[1];

  datastore_init(":memory:");
  attribute_store_init();
  attribute::root().delete_node();

  bpt::ptree json_tree;

  using namespace boost::filesystem;

  if (!is_regular_file(config_file_name)) {
    sl_log_error(LOG_TAG, "Specified config file is invalid");
    return -1;
  }

  // Try to load config file with tree data and test configuration
  try {
    sl_log_info(LOG_TAG, "Reading config file %s", config_file_name.c_str());
    bpt::read_json(config_file_name, json_tree);
  }
  catch(std::exception & e)
  {
    sl_log_error(LOG_TAG, "Config file parsing error: %s", e.what());
    return -1;
  }

  // Try to get tree data from json
  try {
    bpt::ptree branches = json_tree.get_child("tree_data");
    if (build_tree(branches)) {
      sl_log_info(LOG_TAG, "Building tree completed");
      // Print tree:
      attribute_store_log();
    } else {
      sl_log_error(LOG_TAG, "Building tree error");
      return -1;
    }
  }
  catch(std::exception & e)
  {
    sl_log_error(LOG_TAG, "Config file parsing error: %s", e.what());
    return -1;
  }

  // Try to get UAM filename from JSON and load UAM file
  try {
    std::string file_name = json_tree.get<std::string>("uam_file");
    MapperEngine &e = MapperEngine::get_instance();
    e.set_ep_type(attribute::root().type());
    e.reset();
    e.load_file(file_name);
  }
  catch(std::exception & e)
  {
    sl_log_error(LOG_TAG, "Config file parsing error: %s", e.what());
    return -1;
  }

  // Try to get test data from json, activate rules and 
  try {
    bpt::ptree tests = json_tree.get_child("test_data.");
    if(!activate_rules(tests)) {
      sl_log_error(LOG_TAG, "Rules activation error");
      return -1;
    }
    sl_log_info(LOG_TAG, "Rules activated");
    // Print tree:
    attribute_store_log();
    uint32_t results = run_tests(tests);
    if (results != 0) {
      sl_log_info(LOG_TAG, "Tests Failed!");
    } else {
      sl_log_info(LOG_TAG, "Tests Passed!");
    }
    return results;
  }
  catch(std::exception & e)
  {
    sl_log_error(LOG_TAG, "Config file parsing error: %s", e.what());
    return -1;
  }
}
