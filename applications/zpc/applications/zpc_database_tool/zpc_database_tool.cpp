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
#include "uic_version.h"

#include "attribute.hpp"
#include "attribute_store.h"
#include "attribute_store_debug.h"
#include "sl_log.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>

using namespace attribute_store;
using namespace std;
namespace bpt = boost::property_tree;

constexpr const char *LOG_TAG = "zpc_database_tool";

/*
 * Translators for boost property tree which allows for parsing hex strings and
 * translating attribute type names to attribute id's 
 * 
 */
struct Uint8VectorTranslator {
  typedef string internal_type;
  typedef vector<uint8_t> external_type;

  boost::optional<external_type> get_value(const internal_type &str)
  {
    if (((str.size() & 1) == 0)) {
      external_type r;
      for (auto it = str.begin(); it != str.end();) {
        int byte;
        stringstream ss;
        ss << hex;
        ss << (*it++) << (*it++);
        ss >> byte;
        r.push_back(static_cast<uint8_t>(byte));
      }
      return boost::optional<external_type>(r);
    } else {
      return boost::optional<external_type>(boost::none);
    }
  }

  boost::optional<internal_type> put_value(const external_type &vector)
  {
    stringstream ss;

    for (uint8_t v: vector) {
      ss << setfill('0') << setw(2) << hex << int(v);
    }
    return boost::optional<internal_type>(ss.str());
  }
};

struct AttributeTypeTranslator {
  typedef string internal_type;
  typedef attribute_store_type_t external_type;

  boost::optional<external_type> get_value(const internal_type &str)
  {
    attribute_store_type_t type = attribute_store_type_by_name(str.c_str());

    if (type != 0xFFFFFFFF) {
      return boost::optional<external_type>(type);
    }

    try {
      size_t size;
      external_type v = stoul(str, &size, 16);
      if (size == str.size()) {
        return boost::optional<external_type>(v);
      }
    } catch (invalid_argument &e) {
      // fallthough
    }
    return boost::optional<external_type>(boost::none);
  }

  boost::optional<internal_type> put_value(const external_type &type)
  {
    const char *name = attribute_store_name_by_type(type);
    if (name) {
      return boost::optional<internal_type>(name);
    } else {
      stringstream ss;
      ss << "0x" << setfill('0') << setw(8) << hex << type;
      return boost::optional<internal_type>(ss.str());
    }
  }
};

template<typename Ch, typename Traits, typename Alloc>
struct bpt::translator_between<basic_string<Ch, Traits, Alloc>,
                               vector<uint8_t>> {
  typedef Uint8VectorTranslator type;
};

template<typename Ch, typename Traits, typename Alloc>
struct bpt::translator_between<basic_string<Ch, Traits, Alloc>,
                               attribute_store_type_t> {
  typedef AttributeTypeTranslator type;
};

static bpt::ptree dump_data_store(const attribute &node)
{
  bpt::ptree pt;
  pt.add("type", node.type());
  auto reported = node.get<vector<uint8_t>>(REPORTED_ATTRIBUTE);
  auto desired  = node.get<vector<uint8_t>>(DESIRED_ATTRIBUTE);
  if (reported.size() > 0) {
    pt.add("reported", reported);
  }
  if (desired.size() > 0) {
    pt.add("desired", desired);
  }

  bpt::ptree child_nodes;
  for (auto child: node.children()) {
    child_nodes.push_back(make_pair("", dump_data_store(child)));
  }
  if (child_nodes.size() > 0) {
    pt.add_child("children", child_nodes);
  }

  return pt;
}

static void import_data_store(attribute &parent, const bpt::ptree &pt)
{
  auto type = pt.get_optional<attribute_store_type_t>("type");
  attribute node;
  if (!type) {
    sl_log_error("zpc_database_tool", "Missing type attribute in json");
    return;
  }
  if (type.value() == ATTRIBUTE_TREE_ROOT) {
    node = parent;
  } else {
    node          = parent.add_node(type.value());
    auto reported = pt.get_optional<vector<uint8_t>>("reported");
    auto desired  = pt.get_optional<vector<uint8_t>>("desired");
    if (reported) {
      node.set<vector<uint8_t>>(REPORTED_ATTRIBUTE, reported.value());
    }
    if (desired) {
      node.set<vector<uint8_t>>(DESIRED_ATTRIBUTE, desired.value());
    }
  }

  auto children = pt.get_child_optional("children");
  if (children) {
    for (auto pt_child: children.value()) {
      import_data_store(node, pt_child.second);
    }
  }
}

int main(int argc, char *argv[])
{
  const char *import_file;
  const char *export_file;

  config_add_string("import",
                    "import json file, if the database file already exists the "
                    "existing data will be erased",
                    "");
  config_add_string("export", "export json file ", "");

  if (SL_STATUS_OK != config_parse(argc, argv, UIC_VERSION)) {
    return 1;
  }
  if (SL_STATUS_OK != datastore_fixt_setup()) {
    return 1;
  }

  attribute_store_init();
  zpc_attribute_store_init();

  config_get_as_string("import", &import_file);
  config_get_as_string("export", &export_file);

  if ((strlen(import_file) > 0) && (strlen(import_file) > 0)) {
    sl_log_error(LOG_TAG, "import and export are mutually exclusive");
    return 1;
  }

  if (strlen(import_file) > 0) {
    bpt::ptree pt;
    //Fist wipe datastore
    attribute::root().delete_node();
    attribute root = attribute::root();
    bpt::read_json(import_file, pt);
    import_data_store(root, pt);
    sl_log_info(LOG_TAG, "Data base successfully imported");
  }

  if (strlen(export_file) > 0) {
    auto pt = dump_data_store(attribute::root());
    bpt::write_json(export_file, pt);
    sl_log_info(LOG_TAG, "import and export are mutually exported");
  }

  datastore_fixt_teardown();
  return 0;
}
