/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "uic_stdin.hpp"
#include "unify_stdin_attribute_store_command_handler.h"

// Unify components
#include "attribute.hpp"
#include "attribute_store.h"
#include "attribute_store_configuration.h"
#include "attribute_store_type_registration.h"
#include "sl_log.h"

#include <cfloat>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

// Type aliasing
using attribute_store_get_string_function_t
  = sl_status_t (*)(attribute_store_node_t, char *, size_t);
using tuple_node_and_level = std::tuple<attribute_store_node_t, int>;

///////////////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////////////
static sl_status_t
  handle_attribute_store_configuration_set_auto_save_cooldown_interval(
    const handle_args_t &arg);

static sl_status_t
  handle_attribute_store_configuration_set_auto_save_safety_interval(
    const handle_args_t &arg);

static sl_status_t handle_attribute_store_configuration_set_type_validation(
  const handle_args_t &arg);

static sl_status_t handle_attribute_store_log(const handle_args_t &arg);
static sl_status_t handle_attribute_store_log_search(const handle_args_t &arg);
static sl_status_t
  handle_attribute_store_set_reported(const handle_args_t &arg);
static sl_status_t
  handle_attribute_store_set_reported_string(const handle_args_t &arg);
static sl_status_t handle_attribute_store_set_desired(const handle_args_t &arg);
static sl_status_t
  handle_attribute_store_set_all_desired_types(const handle_args_t &arg);
static sl_status_t handle_attribute_store_log_node(const handle_args_t &arg);
static sl_status_t
  handle_attribute_store_undefine_reported(const handle_args_t &arg);
static sl_status_t handle_attribute_store_delete(const handle_args_t &arg);
static sl_status_t handle_attribute_store_add(const handle_args_t &arg);

///////////////////////////////////////////////////////////////////////////////
// Additional Attribute Store commands map
///////////////////////////////////////////////////////////////////////////////
const command_map_t attribute_store_commands = {
  {"attribute_store_configuration_set_auto_save_cooldown_interval",
   {COLOR_START "<Interval>" COLOR_END
                " :Configures how long the Attribute Store waits after an "
                "update before saving to the datastore. Use the value 0 to "
                "save everything sequentially, any other "
                "value for delay in seconds to wait after attributes updates",
    &handle_attribute_store_configuration_set_auto_save_cooldown_interval}},
  {"attribute_store_configuration_set_auto_save_safety_interval",
   {COLOR_START "<Interval>" COLOR_END
                " :Configures how long the Attribute Store waits in the wost "
                "case scenario before it saves to the datastore.",
    &handle_attribute_store_configuration_set_auto_save_safety_interval}},
  {"attribute_store_configuration_set_type_validation",
   {COLOR_START
    "<Enabled>" COLOR_END
    " :Configures if the Attribute Store type write validation is enabled."
    "Use the value 0 for disabled, 1 for enabled",
    &handle_attribute_store_configuration_set_type_validation}},
  {"attribute_store_log",
   {COLOR_START
    "<Attribute Type>,<Attribute ID>" COLOR_END
    " :Dumps the content of the attribute store for a matching Attribute Type "
    "and ID. Attribute Type is specified by its name. "
    "The entire attribute store is dumped if no arguments are provided.",
    &handle_attribute_store_log}},
  {"attribute_store_log_search",
   {COLOR_START
    "<Attribute Name>" COLOR_END
    " :Prints the tree path to nodes with the specified Attribute Name.",
    &handle_attribute_store_log_search}},
  {"attribute_store_set_all_desired_types",
   {COLOR_START "<type>,<Value>" COLOR_END
                " :Goes throught the entire attribute store and set all "
                "attribute types to the indicated value. ",
    &handle_attribute_store_set_all_desired_types}},
  {"attribute_store_set_desired",
   {COLOR_START "<Attribute ID>,<Value>" COLOR_END
                " :Sets the Attribute ID to the given DESIRED value.",
    &handle_attribute_store_set_desired}},
  {"attribute_store_set_reported",
   {COLOR_START "<Attribute ID>,<Value>" COLOR_END
                " :Sets the Attribute ID to the given REPORTED value.",
    &handle_attribute_store_set_reported}},
  {"attribute_store_set_reported_string",
   {COLOR_START "<Attribute ID>,<Value>" COLOR_END
                " :Sets the Attribute ID to the given REPORTED string.",
    &handle_attribute_store_set_reported_string}},
  {"attribute_store_log_node",
   {COLOR_START "<Attribute ID>,<(Optional) {0, 1}>" COLOR_END
                " :prints the content of an attribute store node. If the "
                "optinal flag is set, prints all its children.",
    &handle_attribute_store_log_node}},
  {"attribute_store_undefine_reported",
   {COLOR_START "<Attribute ID>" COLOR_END
                " :Unsets the reported value of an attribute store node"
                "<!> Use with Caution, if you unset something that cannot be "
                "retrieved again, the Attribute Store will be missing "
                "information.",
    &handle_attribute_store_undefine_reported}},
  {"attribute_store_delete",
   {COLOR_START "<Attribute ID>" COLOR_END
                " :Deletes an Attribute ID from the Attribute Store"
                "<!> Use with Caution, it may leave the Attribute Store in an"
                " unexpected state.",
    &handle_attribute_store_delete}},
  {"attribute_store_add",
   {COLOR_START "<Parent ID>,<Type>" COLOR_END
                " :Creates an Attribute with the specified type under the "
                "parent ID in the Attribute Store.",
    &handle_attribute_store_add}}};

///////////////////////////////////////////////////////////////////////////////
// Globals and helper functions
///////////////////////////////////////////////////////////////////////////////
constexpr char LOG_TAG[] = "uic_stdin";

static std::string storage_cstr_to_str(const attribute_store::attribute a,
                                       attribute_store_node_value_state_t state)
{
  std::stringstream stream;
  std::string buf;
  buf.resize(ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);

  if (state == DESIRED_ATTRIBUTE) {
    attribute_store_get_desired_string(a, buf.data(), buf.size());
  } else {
    attribute_store_get_reported_string(a, buf.data(), buf.size());
  }

  if (size_t length = buf.find('\0'); length != std::string::npos) {
    buf.resize(length);
  }
  stream << '"' << buf << '"';
  return stream.str();
}

static std::string
  storage_number_to_str(const attribute_store::attribute a,
                        attribute_store_node_value_state_t state)
{
  std::stringstream stream;

  float value = FLT_MIN;
  if (state == DESIRED_ATTRIBUTE) {
    value = attribute_store_get_desired_number(a);
  } else {
    value = attribute_store_get_reported_number(a);
  }
  if (value != FLT_MIN) {
    stream << std::dec << value;
  } else {
    stream << "<!>Invalid value<!> ";
  }
  return stream.str();
}

static std::string
  storage_unknown_to_str(const attribute_store::attribute a,
                         attribute_store_node_value_state_t state)
{
  std::stringstream stream;

  try {
    std::vector<uint8_t> data = a.get<std::vector<uint8_t>>(state);
    stream << '[';
    stream << std::hex << std::setfill('0');
    for (auto v = data.begin(); v != data.end(); v++) {
      stream << std::setw(2) << static_cast<int>(*v);
      if (v != data.end() - 1) {
        stream << ",";
      }
    }
    stream << ']';
  } catch (const std::invalid_argument &) {
    stream << "<undefined>";
  }
  return stream.str();
}

static std::string attribute_value(const attribute_store::attribute a,
                                   attribute_store_node_value_state_t state)
{
  attribute_store_storage_type_t storage_type
    = attribute_store_get_storage_type(attribute_store_get_node_type(a));

  if (storage_type == EMPTY_STORAGE_TYPE) {
    // Attribute has no value.
    std::string empty("<>");
    return empty;
  }

  if (attribute_store_is_value_defined(a, state) == false) {
    std::string undefined("<undefined>");
    return undefined;
  }

  switch (storage_type) {
    // Attribute is a string
    case C_STRING_STORAGE_TYPE:
      return storage_cstr_to_str(a, state);

    // Numbers:
    case U8_STORAGE_TYPE:
    case U16_STORAGE_TYPE:
    case U32_STORAGE_TYPE:
    case U64_STORAGE_TYPE:
    case I8_STORAGE_TYPE:
    case I16_STORAGE_TYPE:
    case I32_STORAGE_TYPE:
    case I64_STORAGE_TYPE:
    case FLOAT_STORAGE_TYPE:
    case DOUBLE_STORAGE_TYPE:
      return storage_number_to_str(a, state);

    // Consider all the rest as unknown/byte arrays
    // UNKNOWN_STORAGE_TYPE:
    // BYTE_ARRAY_STORAGE_TYPE:
    default:
      return storage_unknown_to_str(a, state);
  }
}

static std::string attribute_info(attribute_store_node_t n,
                                  int indentation_level = 0)
{
  attribute_store_type_t t = attribute_store_get_node_type(n);
  std::stringstream info;
  for (int i = 0; i < indentation_level; i++) {
    if (i == 0) {
      info << "    ";
    } else {
      info << "│   ";
    }
    if (i == (indentation_level - 1)) {
      info << "│───";
    }
  }
  info << "(" << std::dec << n << ") ";
  info << attribute_store_get_type_name(t) << " ";
  return info.str();
}

static sl_status_t do_ats_log(attribute_store_node_t n,
                              int indentation_level        = 0,
                              bool print_total_child_count = false,
                              bool print_only_n_node       = false)
{
  if (n == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_FAIL;
  }

  std::stringstream line;
  if (indentation_level > 2 && print_total_child_count) {
    return SL_STATUS_OK;
  }
  line << attribute_info(n, indentation_level);
  line << std::setfill('.');
  // These │ and ─ characters use 2 bytes, making the line.tellp() inaccurate.
  int extra_spaces = 0;
  if (indentation_level == 1) {
    extra_spaces = 8;
  } else if (indentation_level > 1) {
    extra_spaces = 8 + (indentation_level - 1) * 2;
  }
  line << std::setw(80 - static_cast<int>(line.tellp()) + extra_spaces) << " ";
  line << std::setfill(' ') << std::setw(6)
       << attribute_value(n, REPORTED_ATTRIBUTE);

  line << " ";
  if (!attribute_store_is_value_defined(n, DESIRED_ATTRIBUTE)) {
    line << "(<>)";
  } else if (false == attribute_store_is_value_matched(n)) {
    line << "(";
    line << attribute_value(n, DESIRED_ATTRIBUTE);
    line << ")";
  }
  line << std::endl;

  if (indentation_level == 2 && print_total_child_count) {
    line << "{ " << attribute_store_get_node_total_child_count(n)
         << " children nodes } \n";
  }

  if (write(uic_stdin_get_output_fd(), line.str().data(), line.str().length())
      < 0) {
    sl_log_error(LOG_TAG, "Failed to write to out_stream");
  }
  if (!print_only_n_node) {
    for (size_t i = 0; i < attribute_store_get_node_child_count(n); i++) {
      do_ats_log(attribute_store_get_node_child(n, i),
                 indentation_level + 1,
                 print_total_child_count);
    }
  }

  return SL_STATUS_OK;
}

static sl_status_t print_vector(const std::vector<tuple_node_and_level> subtree)
{
  sl_status_t status;
  if (subtree.empty()) {
    return SL_STATUS_FAIL;
  }
  for (const auto &[node, level]: subtree) {
    status = do_ats_log(node, level, false, true);
    if (status == SL_STATUS_FAIL)
      return status;
  }
  return SL_STATUS_OK;
}

static sl_status_t
  collect_the_parent_nodes(std::vector<tuple_node_and_level> &tree,
                           attribute_store_node_t node,
                           int level,
                           bool is_the_first_call = true,
                           int pos                = 0)
{
  // Finding the correct place to add each node into the
  // vector, in order to start from the root and end to the node of interest and
  // avoid duplications of nodes in the vector
  if (tuple_node_and_level parent {attribute_store_get_node_parent(node),
                                   level - 1};
      (std::find(tree.begin(), tree.end(), parent)) != tree.end()) {
    tree.insert(tree.end() - pos, tuple_node_and_level(node, level));
    return SL_STATUS_OK;
  }

  if (is_the_first_call) {
    tree.insert(tree.end(), tuple_node_and_level(node, level));
  } else {
    tree.insert(tree.end() - pos, tuple_node_and_level(node, level));
  }

  if (node != attribute_store_get_root()) {
    collect_the_parent_nodes(tree,
                             attribute_store_get_node_parent(node),
                             level - 1,
                             false,
                             pos + 1);
  }

  return SL_STATUS_OK;
}

static sl_status_t
  search_attribute_store(std::vector<tuple_node_and_level> &tree,
                         const std::string &attribute,
                         attribute_store_node_t node
                         = attribute_store_get_root(),
                         int level = 0)
{
  if (node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_FAIL;
  }

  const char *type
    = attribute_store_get_type_name(attribute_store_get_node_type(node));

  // Each time that finds a node calls the collect_the_parent_node to collect the nodes
  // that lead to the node of interest, in a vector
  if (std::string str(type); str.find(attribute) != std::string::npos) {
    collect_the_parent_nodes(tree, node, level);
  }

  for (size_t i = 0; i < attribute_store_get_node_child_count(node); i++) {
    search_attribute_store(tree,
                           attribute,
                           attribute_store_get_node_child(node, i),
                           level + 1);
  }

  return SL_STATUS_OK;
}

static void log_matched_node(attribute_store_type_t node_type,
                             float value,
                             attribute_store_node_t node)
{
  attribute_store_type_t found_type = attribute_store_get_node_type(node);
  if (node_type != found_type) {
    return;
  }
  float found_value = attribute_store_get_reported_number(node);
  if (value == found_value) {
    unify_stdin_attribute_store_log_node(node);
  }
}

static void set_all_children_with_type(attribute_store_node_t node,
                                       attribute_store_type_t type,
                                       float value)
{
  if (node == ATTRIBUTE_STORE_INVALID_NODE) {
    return;
  }
  if (attribute_store_get_node_type(node) == type) {
    attribute_store_set_desired_number(node, value);
  }
  for (size_t i = 0; i < attribute_store_get_node_child_count(node); i++) {
    set_all_children_with_type(attribute_store_get_node_child(node, i),
                               type,
                               value);
  }
}

sl_status_t unify_stdin_attribute_store_log_node(attribute_store_node_t node,
                                                 bool print_total_child_count,
                                                 bool print_only_n_node)
{
  return do_ats_log(node, 0, print_total_child_count, print_only_n_node);
}

///////////////////////////////////////////////////////////////////////////////
// Command handling functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t
  handle_attribute_store_configuration_set_auto_save_cooldown_interval(
    const handle_args_t &arg)
{
  try {
    if (arg.size() == 2) {
      // Parse the Auto-save value argument
      int auto_save_interval = std::stoi(arg[1].c_str(), nullptr, 10);
      attribute_store_configuration_set_auto_save_cooldown_interval(
        auto_save_interval);
      return SL_STATUS_OK;
    }
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(), "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_FAIL;
}

static sl_status_t
  handle_attribute_store_configuration_set_auto_save_safety_interval(
    const handle_args_t &arg)
{
  try {
    if (arg.size() == 2) {
      // Parse the Auto-save value argument
      int auto_save_interval = std::stoi(arg[1].c_str(), nullptr, 10);
      attribute_store_configuration_set_auto_save_safety_interval(
        auto_save_interval);
      return SL_STATUS_OK;
    }
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(), "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_FAIL;
}

static sl_status_t handle_attribute_store_configuration_set_type_validation(
  const handle_args_t &arg)
{
  try {
    if (arg.size() == 2) {
      // Parse the "Enabled" argument
      bool enabled = static_cast<bool>(std::stoi(arg[1].c_str(), nullptr, 10));
      attribute_store_configuration_set_type_validation(enabled);
      return SL_STATUS_OK;
    }
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(), "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_FAIL;
}

static sl_status_t handle_attribute_store_log(const handle_args_t &arg)
{
  try {
    if (arg.size() == 3) {
      std::string id_type_name  = arg[1];
      static const float any_id = std::stof(arg[2].c_str());

      // Look up Attribute Type by its name.
      static const attribute_store_type_t id_type
        = attribute_store_get_type_by_name(id_type_name.c_str());
      if (id_type == ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE) {
        dprintf(uic_stdin_get_output_fd(),
                "Unknown attribute type %s\n",
                id_type_name.c_str());
        return SL_STATUS_FAIL;
      }

      // Look up Attribute Node by its type and value.
      attribute_store_walk_tree(attribute_store_get_root(),
                                [](attribute_store_node_t node) {
                                  log_matched_node(id_type, any_id, node);
                                });
      return SL_STATUS_OK;
    } else {
      return unify_stdin_attribute_store_log_node(attribute_store_get_root());
    }
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(), "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_attribute_store_log_search(const handle_args_t &arg)
{
  // Ensure that debug is active for the attribute store
  sl_log_set_tag_level("attribute_store", SL_LOG_DEBUG);

  if (arg.size() != 2) {
    dprintf(uic_stdin_get_output_fd(),
            "Invalid number of arguments, expected args: <Attribute Type>\n");
    return SL_STATUS_FAIL;
  }
  try {
    std::vector<tuple_node_and_level> tree;

    if (search_attribute_store(tree, arg[1]) == SL_STATUS_FAIL)
      return SL_STATUS_FAIL;
    return print_vector(tree);
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(), "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_attribute_store_set_desired(const handle_args_t &arg)
{
  if (arg.size() != 3) {
    dprintf(uic_stdin_get_output_fd(),
            "Invalid number of arguments, expected args:"
            "attribute_store_set_desired <ID>,<Value>\n");
    return SL_STATUS_FAIL;
  }
  auto node = ATTRIBUTE_STORE_INVALID_NODE;
  try {
    node = std::stoi(arg[1].c_str(), nullptr, 10);
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(),
            "%s: Invalid argument: %s\n",
            arg[1].c_str(),
            e.what());
    return SL_STATUS_FAIL;
  }
  try {
    float value = std::stof(arg[2]);
    return attribute_store_set_desired_number(node, value);
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(),
            "%s: Invalid argument: %s\n",
            arg[2].c_str(),
            e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t
  handle_attribute_store_set_all_desired_types(const handle_args_t &arg)
{
  if (arg.size() != 3) {
    dprintf(uic_stdin_get_output_fd(),
            "Invalid number of arguments, expected args:"
            "attribute_store_set_all_desired_types <Type>,<Value>\n");
    return SL_STATUS_FAIL;
  }
  attribute_store_type_t type = ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE;
  try {
    type = static_cast<uint32_t>(std::stoi(arg[1].c_str(), nullptr, 16));
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(),
            "%s: Invalid argument: %s\n",
            arg[1].c_str(),
            e.what());
    return SL_STATUS_FAIL;
  }
  try {
    float value = std::stof(arg[2]);

    set_all_children_with_type(attribute_store_get_root(), type, value);

  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(),
            "%s: Invalid argument: %s\n",
            arg[2].c_str(),
            e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

static sl_status_t handle_attribute_store_set_reported(const handle_args_t &arg)
{
  if (arg.size() != 3) {
    dprintf(uic_stdin_get_output_fd(),
            "Invalid number of arguments, expected args:"
            "attribute_store_set_reported <ID>,<Value> \n");
    return SL_STATUS_FAIL;
  }
  auto node = ATTRIBUTE_STORE_INVALID_NODE;
  try {
    node = std::stoi(arg[1].c_str(), nullptr, 10);
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(),
            "%s: Invalid argument: %s\n",
            arg[1].c_str(),
            e.what());
    return SL_STATUS_FAIL;
  }
  try {
    float value = std::stof(arg[2]);
    return attribute_store_set_reported_number(node, value);
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(),
            "%s: Invalid argument: %s\n",
            arg[2].c_str(),
            e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t
  handle_attribute_store_set_reported_string(const handle_args_t &arg)
{
  if (arg.size() != 3) {
    dprintf(uic_stdin_get_output_fd(),
            "Invalid number of arguments, expected args:"
            "attribute_store_set_reported_string <ID>,<Value> \n");
    return SL_STATUS_FAIL;
  }
  auto node = ATTRIBUTE_STORE_INVALID_NODE;
  try {
    node = std::stoi(arg[1].c_str(), nullptr, 10);
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(),
            "%s: Invalid argument: %s\n",
            arg[1].c_str(),
            e.what());
    return SL_STATUS_FAIL;
  }
  try {
    return attribute_store_set_reported_string(node, arg[2].c_str());
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(),
            "%s: Invalid argument: %s\n",
            arg[2].c_str(),
            e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_attribute_store_log_node(const handle_args_t &arg)
{
  bool log_children = false;
  auto node         = ATTRIBUTE_STORE_INVALID_NODE;

  if (arg.size() != 2 && arg.size() != 3) {
    dprintf(uic_stdin_get_output_fd(),
            "Invalid number of arguments, expected args: <Attribute "
            "ID>,<(Optional) {0, 1}>");
    return SL_STATUS_FAIL;
  }

  try {
    // Just print the individual node:
    node = std::stoi(arg[1].c_str(), nullptr, 10);

    if (arg.size() == 3)
      log_children = std::stoi(arg[2].c_str(), nullptr, 10) != 0;

    return unify_stdin_attribute_store_log_node(node, false, log_children);
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(), "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t
  handle_attribute_store_undefine_reported(const handle_args_t &arg)
{
  if (arg.size() != 2) {
    dprintf(uic_stdin_get_output_fd(),
            "Invalid number of arguments, expected args: <Attribute ID>");
    return SL_STATUS_FAIL;
  }

  try {
    // Find out the Attribute Store Attribute ID.
    attribute_store_node_t node_to_unset
      = std::stoi(arg[1].c_str(), nullptr, 10);

    // Set it to nothing, and let the attribute resolver do its job
    return attribute_store_set_node_attribute_value(node_to_unset,
                                                    REPORTED_ATTRIBUTE,
                                                    nullptr,
                                                    0);
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(), "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_attribute_store_delete(const handle_args_t &arg)
{
  if (arg.size() != 2) {
    dprintf(uic_stdin_get_output_fd(),
            "Invalid number of arguments, expected args: <Attribute ID>");
    return SL_STATUS_FAIL;
  }

  try {
    // Find out the Attribute Store Attribute ID.
    attribute_store_node_t node_to_delete
      = std::stoi(arg[1].c_str(), nullptr, 10);

    return attribute_store_delete_node(node_to_delete);
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(), "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_attribute_store_add(const handle_args_t &arg)
{
  if (arg.size() != 3) {
    dprintf(uic_stdin_get_output_fd(),
            "Invalid number of arguments, expected args: <Parent Attribute "
            "ID>,<Type>");
    return SL_STATUS_FAIL;
  }

  try {
    // Find out the Attribute Store Attribute ID.
    attribute_store_node_t parent_node = std::stoi(arg[1].c_str(), nullptr, 10);
    attribute_store_type_t type        = std::stoull(arg[2].c_str(), nullptr, 10);

    return attribute_store_add_node(type, parent_node);
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(), "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Functions shared within the component
///////////////////////////////////////////////////////////////////////////////
void unify_stdin_add_attribute_store_commands()
{
  uic_stdin_add_commands(attribute_store_commands);
}
