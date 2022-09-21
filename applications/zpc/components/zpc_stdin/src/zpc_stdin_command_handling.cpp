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

#include "zpc_stdin_command_handling.h"

// Unify components
#include "sl_log.h"
#include "uic_mqtt.h"
#include "uic_stdin.hpp"
#include "ota_time.h"

// Other component includes
#include "zwave_controller.h"
#include "zwave_controller_keyset.h"
#include "zwave_controller_utils.h"
#include "zwave_tx.h"
#include "zwave_tx_groups.h"
#include "attribute_store.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_resolver.h"
#include "zwave_unid.h"
#include "zwave_utils.h"
#include "zwave_tx_scheme_selector.h"
#include "zwave_command_class_supervision.h"
#include "zwave_command_handler.h"
#include "zwave_command_class_association_send.h"
#include "zwave_command_class_firmware_update.h"
#include "zpc_converters.h"
#include "ucl_mqtt_node_interview.h"
#include "attribute_store_configuration.h"
#include "attribute_store_type_registration.h"
#include "attribute.hpp"
#include "attribute_poll.h"
#include "zwave_s2_keystore.h"
// Contiki includes
#include "clock.h"  // For CLOCK_CONF_SECOND

// C++ stdlib includes
#include <cstdio>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <map>
#include <string>
#include <vector>
#include <array>
#include <stdexcept>
#include <float.h>
#include <unistd.h>

// Type aliasing
using attribute_store_get_string_function_t
  = sl_status_t (*)(attribute_store_node_t, char *, size_t);

using tuple_node_and_level = std::tuple<attribute_store_node_t, int>;

constexpr char LOG_TAG[] = "zpc_stdin";
/// File descripter for output stream
static int out_stream;

/// The DSK reported by the node during the S2 inclusion
static uint8_t node_reported_dsk[16] = {0};

// Private handler functions
static sl_status_t handle_zwave_set_default(const handle_args_t &arg);
static sl_status_t handle_zwave_log_security_keys(const handle_args_t &arg);
static sl_status_t handle_zwave_tx(const handle_args_t &arg);
static sl_status_t handle_zwave_tx_association(const handle_args_t &arg);
static sl_status_t handle_zwave_tx_multi(const handle_args_t &arg);
static sl_status_t handle_zwave_tx_log(const handle_args_t &arg);
static sl_status_t
  handle_zwave_command_handler_dispatch(const handle_args_t &arg);
static sl_status_t handle_remove_zwave_node(const handle_args_t &arg);
static sl_status_t handle_attribute_store_log(const handle_args_t &arg);
static sl_status_t handle_attribute_store_log_network(const handle_args_t &arg);
static sl_status_t handle_attribute_store_log_search(const handle_args_t &arg);

static sl_status_t
  handle_attribute_store_set_reported(const handle_args_t &arg);
static sl_status_t handle_attribute_store_set_desired(const handle_args_t &arg);
static sl_status_t
  handle_attribute_store_set_all_desired_types(const handle_args_t &arg);
static sl_status_t handle_attribute_store_log_node(const handle_args_t &arg);
static sl_status_t handle_attribute_store_log_subtree(const handle_args_t &arg);
static sl_status_t
  handle_attribute_store_resolve_attribute(const handle_args_t &arg);
static sl_status_t handle_attribute_resolver_log(const handle_args_t &arg);
static sl_status_t handle_zwave_remove_failed(const handle_args_t &arg);
static sl_status_t handle_add_zwave_node(const handle_args_t &arg);
static sl_status_t handle_add_zwave_node_abort(const handle_args_t &arg);
static sl_status_t handle_grant_keys(const handle_args_t &arg);
static sl_status_t handle_accept_dsk(const handle_args_t &arg);
static sl_status_t handle_zwave_wake_up(const handle_args_t &arg);
static sl_status_t handle_zwave_learn_mode(const handle_args_t &arg);
static sl_status_t handle_zwave_initiate_interview(const handle_args_t &arg);
static sl_status_t handle_cc_versions_log(const handle_args_t &arg);
static sl_status_t
  handle_zwave_initiate_firmware_update(const handle_args_t &arg);
static sl_status_t handle_zwave_abort_firmware_update(const handle_args_t &arg);
static sl_status_t handle_attribute_poll_register(const handle_args_t &arg);
static sl_status_t handle_attribute_poll_deregister(const handle_args_t &arg);

/// Map that holds all the commands (used for printing help)
///
/// Entries are {<command>, {<help message>, <handler_func>}},
/// <command>: The command written on the command interface (e.g. "help")
/// <help_message>: Help message printed to output when "help" is executed
/// <handler_func>: callback, that is called whenever the command is receied
const std::map<std::string, std::pair<std::string, handler_func>> commands = {
  {"zwave_set_default", {"Reset Z-Wave network", &handle_zwave_set_default}},
  {"zwave_log_security_keys",
   {"Log Z-Wave network security keys", &handle_zwave_log_security_keys}},
  {"zwave_remove_failed",
   {COLOR_START "<Node ID >" COLOR_END " :Remove a failing Z-Wave node.",
    &handle_zwave_remove_failed}},
  {"zwave_tx",
   {COLOR_START
    "\n\r1. <Node ID>,<Hexadecimal stream payload>" COLOR_END
    " :Send Z-Wave payload to a given Node ID." COLOR_START
    "\n\r2. <NodeID>,<Dst End Point>,<Src End Point>,<Hex payload>" COLOR_END
    " :Send Z-Wave payload to a given Node ID, using Multi Channel "
    "encapsulation.",
    &handle_zwave_tx}},
  {"zwave_tx_association",
   {COLOR_START
    "<Group ID>,<Hexadecimal stream payload>" COLOR_END
    " :Sends Z-Wave payload to all destinations in an Association Group ID.",
    &handle_zwave_tx_association}},
  {"zwave_tx_multi",
   {COLOR_START "\n\r <Node ID>,<Node ID>,...,<Node ID>,<Hexadecimal stream "
                "payload>" COLOR_END
                " :Send Z-Wave payload to a list of NodeID. The last argument "
                "will be the hexadecimal payload.",
    &handle_zwave_tx_multi}},
  {"zwave_tx_log",
   {COLOR_START
    "<1|0>" COLOR_END
    " :Logs the content of the TX queue. Use 1 to print frame contents. "
    "0 to print the frames only. Default option is 0.",
    &handle_zwave_tx_log}},
  {"zwave_command_handler_dispatch",
   {COLOR_START "<payload>" COLOR_END
                " :Injects a payload to the local Z-Wave dispatch handler.",
    &handle_zwave_command_handler_dispatch}},
  {"zwave_remove_node",
   {" :Remove a Z-Wave node from the network", &handle_remove_zwave_node}},
  {"attribute_store_log",
   {COLOR_START
    "<Node ID>" COLOR_END
    " :Dumps the content of the attribute store for a given NodeID. "
    "The entire attribute store is dumped if no NodeID is provided",
    &handle_attribute_store_log}},
  {"attribute_store_log_network",
   {COLOR_START
    "<Node ID>" COLOR_END
    " :Logs the attribute store up to 3 levels and then just displays"
    " the number of children nodes "
    "The entire attribute store is dumped if no NodeID is provided",
    &handle_attribute_store_log_network}},
  {"attribute_store_log_search",
   {COLOR_START
    "<Attribute Name>" COLOR_END
    " :Prints the tree path to nodes with the specified <Attribute Type",
    &handle_attribute_store_log_search}},
  {"attribute_store_set_all_desired_types",
   {COLOR_START "<type>,<Value>" COLOR_END
                " :Goes throught the entire attribute store and set all "
                "attribute types to the indicated value. ",
    &handle_attribute_store_set_all_desired_types}},
  {"attribute_store_set_desired",
   {COLOR_START "<ID>,<Value>" COLOR_END
                " :Sets the Attribute ID to the given DESIRED value.",
    &handle_attribute_store_set_desired}},
  {"attribute_store_set_reported",
   {COLOR_START "<ID>,<Value>" COLOR_END
                " :Sets the Attribute ID to the given REPORTED value.",
    &handle_attribute_store_set_reported}},
  {"attribute_store_log_node",
   {COLOR_START "<Node ID>" COLOR_END
                " :prints the content of an attribute store node.",
    &handle_attribute_store_log_node}},
  {"attribute_store_log_subtree",
   {COLOR_START
    "<Node ID>" COLOR_END
    " :prints the content of an attribute store node and all its children.",
    &handle_attribute_store_log_subtree}},
  {"attribute_store_resolve_attribute",
   {COLOR_START
    "<Node ID>" COLOR_END
    " :Unsets an attribute store node, so that it gets resolved again. "
    "<!> Use with Caution, if you unset something that cannot be resolved, "
    "e.g. the HomeID, the attribute store will be broken and not retrievable.",
    &handle_attribute_store_resolve_attribute}},
  {"attribute_resolver_log",
   {"Logs the state of the attribute resolver",
    &handle_attribute_resolver_log}},
  {"zwave_network_management_abort",
   {" :Abort an ongoing zwave network management operation if possible",
    &handle_add_zwave_node_abort}},
  {"zwave_add_node",
   {" :Add a Z-Wave node to the network", &handle_add_zwave_node}},
  {"zwave_grant_keys",
   {COLOR_START
    "<1/0 to accept/reject the requested keys>,<Set of keys to"
    " grant in [Hexadecimal]>,<1 if csa requested,"
    "if not no need to insert this value>" COLOR_END
    " :Accept security bootstraping and grant the requested keys. For e.g. "
    "zwave_grant_keys 1,87,1 for accepting all keys with csa",
    &handle_grant_keys}},
  {"zwave_accept_dsk",
   {" :Verify the DSK and insert the missing part. Usage: zwave_accept_dsk "
    "<insert the first two byte of the DSK in [decimal]>",
    &handle_accept_dsk}},
  {"zwave_wake_up",
   {COLOR_START "<Node ID>" COLOR_END
                " :Attempts to fast track a Wake Up from a Wake "
                "Up Node.",
    &handle_zwave_wake_up}},
  {"zwave_learn_mode",
   {COLOR_START
    "<Learn Mode Intent>" COLOR_END
    " :Selects the Learn Mode intent. The following values are available:"
    "    - 'inclusion': Direct range Inclusion"
    "    - 'exclusion': Direct range Exclusion"
    "    - 'nwi': Network-Wide Inclusion"
    "    - 'nwe': Network-Wide Exclusion"
    "    - 'stop': Stop Ongoing Learn Mode operation"
    "  For e.g. zwave_learn_mode inclusion (It will trigger Learn Mode with "
    "direct range communication.)",
    &handle_zwave_learn_mode}},
  {"zwave_initiate_interview",
   {COLOR_START
    "\n\r1. <NodeID>" COLOR_END
    " :Initiates a full interview for the given NodeID." COLOR_START
    "\n\r2. <NodeID>,<End Point ID>" COLOR_END
    " :Initiates a full interview for a given Endpoint under a NodeID",
    &handle_zwave_initiate_interview}},
  {"zwave_initiate_firmware_update",
   {COLOR_START "<NodeID>,<filename>" COLOR_END
                "Initiates a firmware update of <NodeID> Endpoint 0, "
                "firmware 0, using the "
                "specified file name for binary data. ",
    &handle_zwave_initiate_firmware_update}},
  {"zwave_abort_firmware_update",
   {COLOR_START "<NodeID>" COLOR_END
                "Ensures that any firmware update of <NodeID> is stopped.",
    &handle_zwave_abort_firmware_update}},
  {"zwave_cc_versions_log",
   {"Print the CC version table", &handle_cc_versions_log}},
  {"attribute_poll_register",
   {COLOR_START "<NodeID>(,<INTERVAL>)" COLOR_END
                "Register a <NodeID> to be polled every <INTERVAL> secondes, "
                "if no <INTERVAL> is supplied, the default interval is used",
    handle_attribute_poll_register}},
  {"attribute_poll_deregister",
   {COLOR_START "<NodeID>" COLOR_END "Deregister <NodeID> for polling",
    handle_attribute_poll_deregister}}};

// Pre declaration of setup function
sl_status_t zpc_stdin_command_handling_init();

// Adding zpc commands to the Unify CLI
void zpc_setup_cli()
{
  uic_stdin_add_commands(commands);
  uic_stdin_set_prompt_string("ZPC>");
  zpc_stdin_command_handling_init();
}

// Internal ZPC commands
static sl_status_t handle_zwave_set_default(const handle_args_t &arg)
{
  zwave_controller_reset();
  return SL_STATUS_OK;
}

static sl_status_t handle_zwave_log_security_keys(const handle_args_t &arg)
{
  zwave_s2_log_security_keys(SL_LOG_INFO);
  return SL_STATUS_OK;
}

static sl_status_t handle_zwave_remove_failed(const handle_args_t &arg)
{
  if (arg.size() != 2) {
    dprintf(out_stream,
            "Invalid number of arguments, expected args: <NodeID>"
            "For example zwave_remove_failed 02\n");
    return SL_STATUS_FAIL;
  }
  try {
    zwave_node_id_t node_id
      = static_cast<zwave_node_id_t>(std::stoi(arg[1].c_str(), nullptr, 10));
    return zwave_network_management_remove_failed(node_id);
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s: Invalid argument: %s\n", arg[0].c_str(), e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_zwave_tx(const handle_args_t &arg)
{
  if (arg.size() != 3 && arg.size() != 5) {
    dprintf(out_stream,
            "Invalid number of arguments. You have the choice between:\n"
            "1. <Dst NodeID>,<Hexpayload>. For example zwave_tx 02,2501FF\n"
            "2. <Dst NodeID>,<Dst End Point>,<Src End Point>,<Hex payload>. "
            "For example zwave_tx 02,03,00,2501FF\n");
    return SL_STATUS_FAIL;
  }

  std::array<uint8_t, ZWAVE_MAX_FRAME_SIZE> payload;
  zwave_tx_options_t tx_options = {};
  zwave_controller_connection_info_t connection_info;
  zwave_endpoint_id_t local_endpoint_id  = 0;
  zwave_endpoint_id_t remote_endpoint_id = 0;

  uint8_t payload_index = 2;
  if (arg.size() == 5) {
    payload_index = 4;
  }

  try {
    // Get the data from the input args
    connection_info.remote.node_id
      = static_cast<zwave_node_id_t>(std::stoi(arg[1].c_str(), nullptr, 10));

    if (arg.size() == 5) {
      local_endpoint_id = static_cast<zwave_endpoint_id_t>(
        std::stoi(arg[3].c_str(), nullptr, 10));

      remote_endpoint_id = static_cast<zwave_endpoint_id_t>(
        std::stoi(arg[2].c_str(), nullptr, 10));
    }

    for (std::size_t i = 0; i < (arg[payload_index].length()); i += 2) {
      payload.at(i / 2) = static_cast<uint8_t>(
        std::stoi(arg[payload_index].substr(i, 2), nullptr, 16));
    }

    // Prepare the Connection Info data for remote node
    connection_info.local.endpoint_id = local_endpoint_id;
    zwave_tx_scheme_get_node_connection_info(connection_info.remote.node_id,
                                             remote_endpoint_id,
                                             &connection_info);

    // Some pre-defined Z-Wave TX options.
    uint8_t number_of_expected_responses = 1;
    uint32_t discard_timeout_ms          = 5 * CLOCK_CONF_SECOND;
    zwave_tx_scheme_get_node_tx_options(ZWAVE_TX_QOS_MIN_PRIORITY,
                                        number_of_expected_responses,
                                        discard_timeout_ms,
                                        &tx_options);

    return zwave_tx_send_data(&connection_info,
                              (uint16_t)(arg[payload_index].length() / 2),
                              payload.data(),
                              &tx_options,
                              nullptr,
                              nullptr,
                              nullptr);

  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_zwave_tx_association(const handle_args_t &arg)
{
  if (arg.size() != 3) {
    dprintf(
      out_stream,
      "Invalid number of arguments. Expected:\n"
      "1. <Group ID>,<Hexpayload>. For example zwave_tx_association 01,5A01\n");
    return SL_STATUS_FAIL;
  }

  std::array<uint8_t, ZWAVE_MAX_FRAME_SIZE> payload;
  zwave_tx_options_t tx_options   = {};
  uint8_t payload_index           = 2;
  association_group_id_t group_id = 0;

  try {
    // Get the data from the input args
    group_id = static_cast<association_group_id_t>(
      std::stoi(arg[1].c_str(), nullptr, 10));

    for (std::size_t i = 0; i < (arg[payload_index].length()); i += 2) {
      payload.at(i / 2) = static_cast<uint8_t>(
        std::stoi(arg[payload_index].substr(i, 2), nullptr, 16));
    }
    // Some pre-defined Z-Wave TX options.
    uint8_t number_of_expected_responses = 0;
    uint32_t discard_timeout_ms          = 5 * CLOCK_CONF_SECOND;
    zwave_tx_scheme_get_node_tx_options(ZWAVE_TX_QOS_MIN_PRIORITY,
                                        number_of_expected_responses,
                                        discard_timeout_ms,
                                        &tx_options);

    return zwave_association_send_to_group(
      group_id,
      (uint16_t)(arg[payload_index].length() / 2),
      payload.data(),
      true,
      &tx_options,
      0,
      nullptr,
      nullptr);

  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_zwave_tx_multi(const handle_args_t &arg)
{
  if (arg.size() < 3) {
    dprintf(out_stream,
            "Invalid number of arguments. We need at least 2 (1 NodeID / 1 Hex "
            "payload)"
            "For example zwave_tx_multi 02,03,2501FF\n");
    return SL_STATUS_FAIL;
  }

  std::array<uint8_t, ZWAVE_MAX_FRAME_SIZE> payload;
  zwave_tx_options_t tx_options = {};
  zwave_controller_connection_info_t connection_info;

  zwave_nodemask_t node_mask = {};
  memset(node_mask, 0, sizeof(zwave_nodemask_t));

  uint8_t arg_index = 1;
  try {
    zwave_node_id_t current_node = 0;
    while (arg_index < arg.size() - 1) {
      current_node = static_cast<zwave_node_id_t>(
        std::stoi(arg[arg_index].c_str(), nullptr, 10));
      ZW_ADD_NODE_TO_MASK(current_node, node_mask);
      arg_index += 1;
    }

    for (std::size_t i = 0; i < (arg[arg_index].length()); i += 2) {
      payload.at(i / 2) = static_cast<uint8_t>(
        std::stoi(arg[arg_index].substr(i, 2), nullptr, 16));
    }

    // Ask a Group ID to the Z-Wave TX Group manager
    zwave_multicast_group_id_t group_id = 0;
    if (SL_STATUS_OK != zwave_tx_assign_group(node_mask, &group_id)) {
      return SL_STATUS_FAIL;
    }

    // Prepare the Connection Info data for remote nodes
    connection_info.local.endpoint_id = 0;
    // Take any node in the group for the connection_info, since
    // they all have the same security level / protocol.
    zwave_tx_scheme_get_node_connection_info(current_node, 0, &connection_info);
    connection_info.remote.is_multicast    = true;
    connection_info.remote.multicast_group = group_id;

    // Some pre-defined Z-Wave TX options.
    uint8_t number_of_expected_responses = 0;
    uint32_t discard_timeout_ms          = 5 * CLOCK_CONF_SECOND;
    zwave_tx_scheme_get_node_tx_options(ZWAVE_TX_QOS_MIN_PRIORITY,
                                        number_of_expected_responses,
                                        discard_timeout_ms,
                                        &tx_options);

    // We want the TX queue to do follow up for us.
    tx_options.send_follow_ups = true;

    return zwave_command_class_supervision_send_data(
      &connection_info,
      (uint16_t)(arg[arg_index].length() / 2),
      payload.data(),
      &tx_options,
      nullptr,
      nullptr,
      nullptr);

  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_zwave_tx_log(const handle_args_t &arg)
{
  zwave_tx_log_queue(true);
  return SL_STATUS_OK;
}

static sl_status_t
  handle_zwave_command_handler_dispatch(const handle_args_t &arg)
{
  if (arg.size() != 2) {
    dprintf(
      out_stream,
      "Invalid number of arguments: Payload needs to be specified.\n"
      " Example: zwave_command_handler_dispatch 87010003500308500403500506\n");
    return SL_STATUS_FAIL;
  }
  std::array<uint8_t, ZWAVE_MAX_FRAME_SIZE> frame_data;
  try {
    for (std::size_t i = 0; i < (arg[1].length()); i += 2) {
      frame_data.at(i / 2)
        = static_cast<uint8_t>(std::stoi(arg[1].substr(i, 2), nullptr, 16));
    }
    zwave_controller_connection_info_t connection_info = {{{0}}};
    connection_info.remote.node_id = zwave_network_management_get_node_id();
    connection_info.encapsulation  = zpc_highest_security_class();
    return zwave_command_handler_dispatch(&connection_info,
                                          frame_data.data(),
                                          (uint16_t)(arg[1].length() / 2));
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s: Invalid argument: %s\n", arg[0].c_str(), e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_remove_zwave_node(const handle_args_t &arg)
{
  return zwave_network_management_remove_node();
}

static sl_status_t handle_zwave_wake_up(const handle_args_t &arg)
{
  if (arg.size() != 2) {
    dprintf(out_stream,
            "Invalid number of arguments, expected args: <NodeID>"
            "For example zwave_wake_up 23\n");
    return SL_STATUS_FAIL;
  }
  try {
    zwave_node_id_t node_id
      = static_cast<zwave_node_id_t>(std::stoi(arg[1].c_str(), nullptr, 10));
    return zwave_command_class_supervision_wake_on_demand(node_id);
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s: Invalid argument: %s\n", arg[1].c_str(), e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_zwave_learn_mode(const handle_args_t &arg)
{
  if (arg.size() != 2) {
    dprintf(out_stream,
            "Invalid number of arguments, expected args: <Learn Mode Intent>\n"
            "Supported modes nwi, nwe or stop\n"
            "For example zwave_learn_mode nwi\n");
    return SL_STATUS_FAIL;
  }
  std::string learn_mode_intent;
  learn_mode_intent.assign(arg[1].c_str());
  if (learn_mode_intent.compare("nwi") == 0) {
    dprintf(out_stream, "Starting NWI Learn Mode Operation.\n");
    return zwave_network_management_learn_mode(
      ZWAVE_NETWORK_MANAGEMENT_LEARN_NWI);
  } else if (learn_mode_intent.compare("nwe") == 0) {
    dprintf(out_stream, "Starting NWE Learn Mode Operation.\n");
    return zwave_network_management_learn_mode(
      ZWAVE_NETWORK_MANAGEMENT_LEARN_NWE);
  } else if (learn_mode_intent.compare("stop") == 0) {
    dprintf(out_stream, "Stopping Learn Mode Operation.\n");
    return zwave_network_management_learn_mode(
      ZWAVE_NETWORK_MANAGEMENT_LEARN_NONE);
  } else {
    dprintf(out_stream, "Unknown learn mode intent!\n");
  }
  return SL_STATUS_OK;
}

static sl_status_t handle_add_zwave_node(const handle_args_t &arg)
{
  return zwave_network_management_add_node();
}

static sl_status_t handle_add_zwave_node_abort(const handle_args_t &arg)
{
  return zwave_network_management_abort();
}

static sl_status_t handle_grant_keys(const handle_args_t &arg)
{
  uint8_t accept;
  uint8_t csa = 0x00;
  uint8_t keys;

  if (arg.size() < 3 || arg.size() > 4) {
    dprintf(out_stream,
            "Invalid number of arguments, expected args:"
            "<1/0 to accept/reject the requested keys>,<Set of keys to grant "
            "in [Hexadecimal]>,<1 if csa requested,"
            "if not no need to insert this value>\n");

    return SL_STATUS_FAIL;
  }

  try {
    accept = static_cast<uint8_t>(std::stoi(arg[1].c_str(), nullptr, 10));
    keys   = static_cast<uint8_t>(std::stoi(arg[2].c_str(), nullptr, 16));
    if (arg.size() == 4) {
      csa = static_cast<uint8_t>(std::stoi(arg[3].c_str(), nullptr, 10));
    }
    return zwave_network_management_keys_set((bool)accept, (bool)csa, keys);
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_accept_dsk(const handle_args_t &arg)
{
  uint16_t dsk;

  if (arg.size() != 2) {
    dprintf(out_stream,
            "Invalid number of arguments, expected args:"
            "zwave_accept_dsk <the first two byte of the DSK in [decimal]>\n");

    return SL_STATUS_FAIL;
  }

  try {
    dsk                  = std::stoi(arg[1].c_str());
    node_reported_dsk[1] = (dsk)&0xff;
    node_reported_dsk[0] = (dsk >> 8) & 0xff;
    return zwave_network_management_dsk_set(node_reported_dsk);
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static std::string attribute_value(const attribute_store::attribute a,
                                   attribute_store_node_value_state_t state)
{
  attribute_store_storage_type_t storage_type
    = attribute_store_get_storage_type(attribute_store_get_node_type(a));

  std::stringstream stream;

  if (storage_type == EMPTY_STORAGE_TYPE) {
    // Attribute has no value.
    stream << "<>";
    return stream.str();
  }

  if (attribute_store_is_value_defined(a, state) == false) {
    stream << "<undefined>";
    return stream.str();
  }

  switch (storage_type) {
    // Attribute is a string
    case C_STRING_STORAGE_TYPE: {
      attribute_store_get_string_function_t get_string
        = &attribute_store_get_reported_string;
      if (state == DESIRED_ATTRIBUTE) {
        get_string = attribute_store_get_desired_string;
      }
      char string_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
      get_string(a, string_value, sizeof(string_value));
      stream << '"' << string_value << '"';
      return stream.str();
    }

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
    case DOUBLE_STORAGE_TYPE: {
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

    // Consider all the rest as unknown/byte arrays
    // UNKNOWN_STORAGE_TYPE:
    // BYTE_ARRAY_STORAGE_TYPE:
    default:
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
      } catch (std::exception &e) {
        stream << "<undefined>";
      }
      return stream.str();
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
  line << std::setw(80 - line.tellp() + extra_spaces) << " ";
  line << attribute_value(n, REPORTED_ATTRIBUTE);

  if (attribute_store_is_value_defined(n, DESIRED_ATTRIBUTE)
      && (false == attribute_store_is_value_matched(n))) {
    line << " (desired: ";
    line << attribute_value(n, DESIRED_ATTRIBUTE);
    line << ")";
  }
  line << std::endl;

  if (indentation_level == 2 && print_total_child_count) {
    line << "{ " << attribute_store_get_node_total_child_count(n)
         << " children nodes } \n";
  }

  if (write(out_stream, line.str().data(), line.str().length()) < 0) {
    sl_log_error(LOG_TAG, "Failed to write to out_stream");
  };
  if (!print_only_n_node) {
    for (size_t i = 0; i < attribute_store_get_node_child_count(n); i++) {
      do_ats_log(attribute_store_get_node_child(n, i),
                 indentation_level + 1,
                 print_total_child_count);
    }
  }

  return SL_STATUS_OK;
}

static sl_status_t handle_attribute_store_log(const handle_args_t &arg)
{
  try {
    if (arg.size() == 2) {
      // Parse the NodeID and get its node.
      zwave_node_id_t node_id
        = static_cast<zwave_node_id_t>(std::stoi(arg[1].c_str(), nullptr, 10));
      unid_t unid;
      zwave_unid_from_node_id(node_id, unid);
      attribute_store_node_t node_to_print
        = attribute_store_network_helper_get_node_id_node(unid);
      return do_ats_log(node_to_print);
    } else {
      return do_ats_log(attribute_store_get_root());
    }
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_attribute_store_log_network(const handle_args_t &arg)
{
  try {
    if (arg.size() == 2) {
      // Parse the NodeID and get its node.
      zwave_node_id_t node_id
        = static_cast<zwave_node_id_t>(std::stoi(arg[1].c_str(), nullptr, 10));
      unid_t unid;
      zwave_unid_from_node_id(node_id, unid);
      attribute_store_node_t node_to_print
        = attribute_store_network_helper_get_node_id_node(unid);
      return do_ats_log(node_to_print, 0, true);
    } else {
      return do_ats_log(attribute_store_get_root(), 0, true);
    }
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t print_vector(const std::vector<tuple_node_and_level> subtree)
{
  sl_status_t status;
  std::stringstream line;
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
  tuple_node_and_level parent {attribute_store_get_node_parent(node),
                               level - 1};
  bool result = ((std::find(tree.begin(), tree.end(), parent)) != tree.end());
  if (result) {
    tree.insert(tree.end() - pos, tuple_node_and_level(node, level));
    return SL_STATUS_OK;
  } else if (!result && is_the_first_call) {
    pos++;
    tree.insert(tree.end(), tuple_node_and_level(node, level));
  } else {
    tree.insert(tree.end() - pos, tuple_node_and_level(node, level));
    pos++;
  }

  if (node != attribute_store_get_root())
    collect_the_parent_nodes(tree,
                             attribute_store_get_node_parent(node),
                             level - 1,
                             false,
                             pos);

  return SL_STATUS_OK;
}

static sl_status_t find_the_node(std::vector<tuple_node_and_level> &tree,
                                 const std::string &attribute,
                                 attribute_store_node_t node
                                 = attribute_store_get_root(),
                                 int level = 0)
{
  std::stringstream line;
  if (node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_FAIL;
  }

  const char *type
    = attribute_store_get_type_name(attribute_store_get_node_type(node));

  // Each time that finds a node calls the collect_the_parent_node to collect the nodes
  // that lead to the node of interest, in a vector
  std::string str(type);
  if (str.find(attribute) != std::string::npos) {
    collect_the_parent_nodes(tree, node, level);
  }

  for (size_t i = 0; i < attribute_store_get_node_child_count(node); i++) {
    find_the_node(tree,
                  attribute,
                  attribute_store_get_node_child(node, i),
                  level + 1);
  }

  return SL_STATUS_OK;
}

static sl_status_t handle_attribute_store_log_search(const handle_args_t &arg)
{
  // Ensure that debug is active for the attribute store
  sl_log_set_tag_level("attribute_store", SL_LOG_DEBUG);

  if (arg.size() != 2) {
    dprintf(out_stream,
            "Invalid number of arguments, expected args: <Attribute Type>\n");
    return SL_STATUS_FAIL;
  }
  try {
    std::vector<tuple_node_and_level> tree;

    if (find_the_node(tree, arg[1]) == SL_STATUS_FAIL)
      return SL_STATUS_FAIL;
    return print_vector(tree);
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_attribute_store_set_desired(const handle_args_t &arg)
{
  if (arg.size() != 3) {
    dprintf(out_stream,
            "Invalid number of arguments, expected args:"
            "attribute_store_set_desired <ID>,<Value>\n");
    return SL_STATUS_FAIL;
  }
  attribute_store_node_t node = ATTRIBUTE_STORE_INVALID_NODE;
  try {
    node = std::stoi(arg[1].c_str(), nullptr, 10);
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s: Invalid argument: %s\n", arg[1].c_str(), e.what());
    return SL_STATUS_FAIL;
  }
  try {
    float value = std::stof(arg[2]);
    return attribute_store_set_desired_number(node, value);
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s: Invalid argument: %s\n", arg[2].c_str(), e.what());
    return SL_STATUS_FAIL;
  }
}

static void
  set_all_children_with_type(attribute_store_node_t node,
                             attribute_store_type_t type,
                             attribute_store_node_value_state_t value_state,
                             float value)
{
  if (node == ATTRIBUTE_STORE_INVALID_NODE) {
    return;
  }
  if (attribute_store_get_node_type(node) == type) {
    if (value_state == REPORTED_ATTRIBUTE) {
      attribute_store_set_reported_number(node, value);
    } else {
      attribute_store_set_desired_number(node, value);
    }
  }
  for (size_t i = 0; i < attribute_store_get_node_child_count(node); i++) {
    set_all_children_with_type(attribute_store_get_node_child(node, i),
                               type,
                               value_state,
                               value);
  }
}

static sl_status_t
  handle_attribute_store_set_all_desired_types(const handle_args_t &arg)
{
  if (arg.size() != 3) {
    dprintf(out_stream,
            "Invalid number of arguments, expected args:"
            "attribute_store_set_all_desired_types <Type>,<Value>\n");
    return SL_STATUS_FAIL;
  }
  attribute_store_type_t type = ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE;
  try {
    type = static_cast<uint32_t>(std::stoi(arg[1].c_str(), nullptr, 16));
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s: Invalid argument: %s\n", arg[1].c_str(), e.what());
    return SL_STATUS_FAIL;
  }
  try {
    float value = std::stof(arg[2]);

    set_all_children_with_type(attribute_store_get_root(),
                               type,
                               DESIRED_ATTRIBUTE,
                               value);

  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s: Invalid argument: %s\n", arg[2].c_str(), e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

static sl_status_t handle_attribute_store_set_reported(const handle_args_t &arg)
{
  if (arg.size() != 3) {
    dprintf(out_stream,
            "Invalid number of arguments, expected args:"
            "attribute_store_set_reported <ID>,<Value> \n");
    return SL_STATUS_FAIL;
  }
  attribute_store_node_t node = ATTRIBUTE_STORE_INVALID_NODE;
  try {
    node = std::stoi(arg[1].c_str(), nullptr, 10);
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s: Invalid argument: %s\n", arg[1].c_str(), e.what());
    return SL_STATUS_FAIL;
  }
  try {
    float value = std::stof(arg[2]);
    return attribute_store_set_reported_number(node, value);
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s: Invalid argument: %s\n", arg[2].c_str(), e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_attribute_store_log_node(const handle_args_t &arg)
{
  // Ensure that debug is active for the attribute store
  sl_log_set_tag_level("attribute_store", SL_LOG_DEBUG);

  if (arg.size() != 2) {
    dprintf(out_stream, "Invalid number of arguments, expected args: <NodeID>");
    return SL_STATUS_FAIL;
  }

  try {
    // Just print the individual node:
    attribute_store_node_t node_to_print
      = std::stoi(arg[1].c_str(), nullptr, 10);
    attribute_store_log_node(node_to_print, false);
    return SL_STATUS_OK;
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_attribute_store_log_subtree(const handle_args_t &arg)
{
  // Ensure that debug is active for the attribute store
  sl_log_set_tag_level("attribute_store", SL_LOG_DEBUG);

  if (arg.size() != 2) {
    dprintf(out_stream, "Invalid number of arguments, expected args: <NodeID>");
    return SL_STATUS_FAIL;
  }
  try {
    // Just print the individual node:
    attribute_store_node_t node_to_print
      = std::stoi(arg[1].c_str(), nullptr, 10);
    return do_ats_log(node_to_print);
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t
  handle_attribute_store_resolve_attribute(const handle_args_t &arg)
{
  if (arg.size() != 2) {
    dprintf(out_stream, "Invalid number of arguments, expected args: <NodeID>");
    return SL_STATUS_FAIL;
  }

  try {
    // Parse the which Attribute Store node to unset (so it gets resolved again)
    attribute_store_node_t node_to_unset
      = std::stoi(arg[1].c_str(), nullptr, 10);

    // Set it to nothing, and let the attribute resolver do its job
    return attribute_store_set_node_attribute_value(node_to_unset,
                                                    REPORTED_ATTRIBUTE,
                                                    nullptr,
                                                    0);
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_attribute_resolver_log(const handle_args_t &)
{
  attribute_resolver_state_log();
  return SL_STATUS_OK;
}

static sl_status_t handle_zwave_initiate_interview(const handle_args_t &arg)
{
  if (arg.size() <= 1) {
    dprintf(out_stream,
            "Error: this command requires at least 1 argument (NodeID)\n");
    return SL_STATUS_FAIL;
  }
  try {
    // For sure we have a NodeID parameter:
    zwave_node_id_t node_id
      = static_cast<zwave_node_id_t>(std::stoi(arg[1].c_str(), nullptr, 10));
    unid_t node_unid;
    zwave_unid_from_node_id(node_id, node_unid);

    if (arg.size() == 2) {
      return ucl_mqtt_initiate_node_interview(node_unid);
    } else {
      zwave_endpoint_id_t endpoint_id = static_cast<zwave_endpoint_id_t>(
        std::stoi(arg[2].c_str(), nullptr, 10));
      return ucl_mqtt_initiate_endpoint_interview(node_unid, endpoint_id);
    }

  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t
  handle_zwave_initiate_firmware_update(const handle_args_t &arg)
{
  if (arg.size() <= 2) {
    dprintf(out_stream,
            "Error: this command requires at least 2 arguments "
            "(NodeID and file name)\n");
    return SL_STATUS_FAIL;
  }
  try {
    // Parse the NodeID
    zwave_node_id_t node_id
      = static_cast<zwave_node_id_t>(std::stoi(arg[1].c_str(), nullptr, 10));

    // Wait 1 second before starting
    unsigned long apply_after = get_current_utc_current_time() + 2;
    return command_class_firmware_update_initiate_firmware_update(
      node_id,
      0,  // Endpoint ID 0
      0,  // Firmware ID 0
      apply_after,
      arg[2].c_str());

  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_zwave_abort_firmware_update(const handle_args_t &arg)
{
  if (arg.size() <= 1) {
    dprintf(out_stream,
            "Error: this command requires at least 1 argument (NodeID)\n");
    return SL_STATUS_FAIL;
  }
  try {
    // Parse the NodeID
    zwave_node_id_t node_id
      = static_cast<zwave_node_id_t>(std::stoi(arg[1].c_str(), nullptr, 10));

    command_class_firmware_update_abort_ongoing_firmware_operation(node_id, 0);
    return SL_STATUS_OK;

  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t
  handle_cc_versions_log([[maybe_unused]] const handle_args_t &arg)
{
  zwave_command_handler_print_info(out_stream);
  return SL_STATUS_OK;
}

// Notifying the user to choice which keys to grant --- during S2 inclusion
static void on_keys_report(bool csa, zwave_keyset_t keys)
{
  dprintf(out_stream, "Requested Key Set : 0x%x\n", keys);
  dprintf(out_stream, "Requested Key Classes: \n");
  if (keys & ZWAVE_CONTROLLER_S2_ACCESS_KEY) {
    dprintf(out_stream, "     S2_ACCESS \n");
  }
  if (keys & ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY) {
    dprintf(out_stream, "     S2_AUTHENTICATED \n");
  }
  if (keys & ZWAVE_CONTROLLER_S2_UNAUTHENTICATED_KEY) {
    dprintf(out_stream, "     S2_UNAUTHENTICATED \n");
  }
  if (keys & ZWAVE_CONTROLLER_S0_KEY) {
    dprintf(out_stream, "     SECURITY_0 \n");
  }
  if (csa) {
    dprintf(out_stream, "Client Side Authentication is requested \n");
  } else {
    dprintf(out_stream, "Client Side Authentication is not requested \n");
  }

  dprintf(out_stream,
          "Please accept the security bootstrapping and select which requested "
          "keys to grant\n");
  dprintf(out_stream,
          "Usage: zwave_grant_keys <1/0 to accept/reject the requested "
          "keys>,<Set of keys to grant in Hexadecimal>,<1 if csa requested>\n");
  dprintf(out_stream,
          "zwave_grant_keys 1, 87 (to grant all keys and accept "
          "the security bootstrapping)\n");
}

// Requesting a user to insert the missing part of the DSK if the inclusion
// needs authentication. If the inclusion is unauthenticated, ZPC does not
// notify the user about the received DSK, rather the ZPC will accept the received DSK.
static void
  on_dsk_report(uint8_t input_length, zwave_dsk_t dsk, zwave_keyset_t keys)
{
  if ((keys & ZWAVE_CONTROLLER_S2_ACCESS_KEY)
      || (keys & ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY)) {
    char dsk_string[DSK_STR_LEN];
    zpc_converters_dsk_to_str(dsk, dsk_string, DSK_STR_LEN);

    dprintf(out_stream, "Received DSK: %s \n", dsk_string);
    dprintf(
      out_stream,
      "Please verify the DSK and insert the missing part (first 5 digits)\n");
    dprintf(out_stream,
            "Usage: zwave_accept_dsk "
            "<insert the first two byte of the DSK in [decimal]>\n");

    memcpy(node_reported_dsk, dsk, sizeof(zwave_dsk_t));
  }
}

static sl_status_t handle_attribute_poll_register(const handle_args_t &arg)
{
  if (arg.size() < 2) {
    return SL_STATUS_FAIL;
  }
  try {
    uint32_t interval = 0;
    if (arg.size() == 3) {
      interval = std::stoul(arg[2].c_str(), nullptr, 10);
    }
    attribute_store_node_t node = std::stoul(arg[1].c_str(), nullptr, 10);
    return attribute_poll_register(node, interval);
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s: Invalid argument: %s\n", arg[0].c_str(), e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_attribute_poll_deregister(const handle_args_t &arg)
{
  if (arg.size() < 2) {
    return SL_STATUS_FAIL;
  }
  try {
    attribute_store_node_t node = std::stoul(arg[1].c_str(), nullptr, 10);
    return attribute_poll_deregister(node);
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s: Invalid argument: %s\n", arg[0].c_str(), e.what());
    return SL_STATUS_FAIL;
  }
}

static zwave_controller_callbacks_t stdin_zwave_controller_cb = {
  .on_keys_report = on_keys_report,
  .on_dsk_report  = on_dsk_report,
};

// For test functionality
zwave_controller_callbacks_t get_zpc_stdin_callbacks()
{
  return stdin_zwave_controller_cb;
}

sl_status_t zpc_stdin_command_handling_init()
{
  out_stream = uic_stdin_get_output_fd();
  // Register zwave_controller_callbacks
  zwave_controller_register_callbacks(&stdin_zwave_controller_cb);

  return SL_STATUS_OK;
}
