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
#include "zwave_s2_keystore.h"
#include "zwave_s2_nonce_management.h"
// Contiki includes
#include "clock.h"  // For CLOCK_CONF_SECOND

// C++ stdlib includes
#include <cstdio>
#include <cstring>
#include <sstream>
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

/// File descripter for output stream
static int out_stream;

/// The DSK reported by the node during the S2 inclusion
static uint8_t node_reported_dsk[16] = {0};

// Private handler functions
static sl_status_t handle_zwave_set_default(const handle_args_t &arg);
static sl_status_t handle_zwave_log_security_keys(const handle_args_t &arg);
static sl_status_t
  handle_zwave_save_security_keys_to_file(const handle_args_t &arg);
static sl_status_t handle_zwave_tx(const handle_args_t &arg);
static sl_status_t handle_zwave_tx_association(const handle_args_t &arg);
static sl_status_t handle_zwave_tx_multi(const handle_args_t &arg);
static sl_status_t handle_zwave_tx_log(const handle_args_t &arg);
static sl_status_t
  handle_zwave_command_handler_dispatch(const handle_args_t &arg);
static sl_status_t handle_remove_zwave_node(const handle_args_t &arg);
static sl_status_t handle_attribute_store_log_network(const handle_args_t &arg);

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
static sl_status_t handle_zwave_reset_span(const handle_args_t &arg);
static sl_status_t handle_zwave_reset_mpan(const handle_args_t &arg);
static sl_status_t handle_zwave_home_id(const handle_args_t &arg);

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
  {"zwave_save_security_keys_to_file",
   {"Save Z-Wave network security keys to a file",
    &handle_zwave_save_security_keys_to_file}},
  {"zwave_remove_failed",
   {COLOR_START "<NodeID>" COLOR_END " :Remove a failing Z-Wave node.",
    &handle_zwave_remove_failed}},
  {"zwave_home_id", {"Print Z-Wave Home ID", &handle_zwave_home_id}},
  {"zwave_reset_span",
   {COLOR_START "<NodeID>" COLOR_END " :Reset SPAN for the node id.",
    &handle_zwave_reset_span}},
  {"zwave_reset_mpan",
   {COLOR_START "<Group ID><NodeID>" COLOR_END " :Reset MPAN for the Group id "
                "and node id. Only" COLOR_START "< Group ID >" COLOR_END
                "is also accepted,"
                " then ZPC node id will be used",
    &handle_zwave_reset_mpan}},
  {"zwave_tx",
   {COLOR_START
    "\n\r1. <NodeID>,<Hexadecimal stream payload>" COLOR_END
    " :Send Z-Wave payload to a given NodeID." COLOR_START
    "\n\r2. <NodeID>,<Dst End Point>,<Src End Point>,<Hex payload>" COLOR_END
    " :Send Z-Wave payload to a given NodeID, using Multi Channel "
    "encapsulation.",
    &handle_zwave_tx}},
  {"zwave_tx_association",
   {COLOR_START
    "<Group ID>,<Hexadecimal stream payload>" COLOR_END
    " :Sends Z-Wave payload to all destinations in an Association Group ID.",
    &handle_zwave_tx_association}},
  {"zwave_tx_multi",
   {COLOR_START "\n\r <NodeID>,<NodeID>,...,<NodeID>,<Hexadecimal stream "
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
  {"attribute_store_log_network",
   {COLOR_START
    "<NodeID>" COLOR_END
    " :Logs the attribute store up to 3 levels and then just displays"
    " the number of children nodes "
    "The entire attribute store is dumped if no NodeID is provided",
    &handle_attribute_store_log_network}},
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
   {COLOR_START "<NodeID>" COLOR_END
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
};

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

static sl_status_t
  handle_zwave_save_security_keys_to_file(const handle_args_t &arg)
{
  if (arg.size() != 2) {
    dprintf(out_stream,
            "Invalid number of arguments, expected args: <File name>"
            "For example zwave_save_security_keys_to_file keys.txt\n");
    return SL_STATUS_FAIL;
  }

  const char *key_file_name = static_cast<const char *>(arg[1].c_str());
  zwave_s2_save_security_keys(key_file_name);
  return SL_STATUS_OK;
}

static sl_status_t handle_zwave_reset_mpan(const handle_args_t &arg)
{
  uint8_t group_id        = 0;
  zwave_node_id_t node_id = zwave_network_management_get_node_id();
  if ((arg.size() < 2) || (arg.size() > 3)) {
    dprintf(out_stream,
            "Invalid number of arguments, least expected args: <GroupID>"
            " Or <GroupID>,<NodeID>\n"
            "For example to reset MPAN of group id 2, zwave_reset_span 2 or\n"
            " to reset MPAN of group 2 for owner id 2, zwave_reset_span 2,1\n");
    return SL_STATUS_FAIL;
  }
  try {
    group_id
      = static_cast<zwave_node_id_t>(std::stoi(arg[1].c_str(), nullptr, 10));
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s: Invalid argument: %s\n", arg[0].c_str(), e.what());
    return SL_STATUS_FAIL;
  }

  if (arg.size() == 3) {
    try {
      node_id
        = static_cast<zwave_node_id_t>(std::stoi(arg[2].c_str(), nullptr, 10));
    } catch (const std::invalid_argument &e) {
      dprintf(out_stream,
              "%s: Invalid argument: %s\n",
              arg[0].c_str(),
              e.what());
      return SL_STATUS_FAIL;
    }
  }
  zwave_s2_reset_mpan(node_id, group_id);
  return SL_STATUS_OK;
}

static sl_status_t handle_zwave_home_id(const handle_args_t &arg)
{
  dprintf(out_stream,
          "Z-Wave Home ID: %08X\n",
          zwave_network_management_get_home_id());
  return SL_STATUS_OK;
}

static sl_status_t handle_zwave_reset_span(const handle_args_t &arg)
{
  if (arg.size() != 2) {
    dprintf(out_stream,
            "Invalid number of arguments, expected args: <NodeID>"
            "For example zwave_reset_span 02\n");
    return SL_STATUS_FAIL;
  }
  try {
    zwave_node_id_t node_id
      = static_cast<zwave_node_id_t>(std::stoi(arg[1].c_str(), nullptr, 10));
    zwave_s2_reset_span(node_id);
    return SL_STATUS_OK;
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s: Invalid argument: %s\n", arg[0].c_str(), e.what());
    return SL_STATUS_FAIL;
  }
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
      return unify_stdin_attribute_store_log_node(node_to_print, true);
    } else {
      return unify_stdin_attribute_store_log_node(attribute_store_get_root(),
                                                  true);
    }
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
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
