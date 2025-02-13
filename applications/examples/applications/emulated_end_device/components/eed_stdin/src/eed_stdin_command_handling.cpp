/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "eed_stdin_command_handling.h"

// Unify components
#include "sl_log.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "uic_mqtt.h"
#include "uic_stdin.hpp"
#include "unify_dotdot_attribute_store.h"

// EED Components:
#include "eed_attribute_store.h"
#include "eed_attribute_store_helper.h"
#include "eed_attribute_store_attribute_types.h"
#include "eed_matter_device_type.h"

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

#define UNID_FORMAT "eed-%08X"

#define MAXIMUM_CLUSTER_LIST_SIZE 512

/// File descripter for output stream
static int out_stream;

// Private handler functions
static sl_status_t handle_add_unid(const handle_args_t &arg);
static sl_status_t handle_remove_unid(const handle_args_t &arg);
static sl_status_t handle_add_endpoint(const handle_args_t &arg);
static sl_status_t handle_remove_endpoint(const handle_args_t &arg);
static sl_status_t handle_add_cluster_support(const handle_args_t &arg);
static sl_status_t handle_remove_cluster_support(const handle_args_t &arg);

/// Map that holds all the commands (used for printing help)
///
/// Entries are {<command>, {<help message>, <handler_func>}},
/// <command>: The command written on the command interface (e.g. "help")
/// <help_message>: Help message printed to output when "help" is executed
/// <handler_func>: callback, that is called whenever the command is receied
const std::map<std::string, std::pair<std::string, handler_func>> commands = {
  {"add_unid", {"Add a UNID in the Attribute Store", &handle_add_unid}},
  {"remove_unid",
   {"Removes a UNID from the Attribute Store", &handle_remove_unid}},
  {"add_endpoint",
   {"Add an endpoint under a UNID in the Attribute Store with all clusters or can take \
     cluster-name(s) in UpperCamelCase separated with semi-colon \
     (Eg: add_endpoint 1,0,\"OnOff;Level;ColorControl\") or device type(Eg: OnOffLight) for customized cluster selection",
    &handle_add_endpoint}},
  {"remove_endpoint",
   {"Removes an endpoint under a UNID from the Attribute Store",
    &handle_remove_endpoint}},
  {"add_cluster_support",
   {"adds a cluster to an endpoint under a UNID from the Attribute Store",
    &handle_add_cluster_support}},
  {"remove_cluster_support",
   {"removes a cluster from  an endpoint under a UNID from the Attribute Store",
    &handle_remove_cluster_support}},
};

sl_status_t eed_stdin_command_handling_init()
{
  out_stream = uic_stdin_get_output_fd();
  return SL_STATUS_OK;
}

// Adding eed commands to the Unify CLI
void eed_setup_cli()
{
  uic_stdin_add_commands(commands);
  uic_stdin_set_prompt_string("EED>");
  eed_stdin_command_handling_init();
}

// Internal EED commands
static sl_status_t handle_add_unid(const handle_args_t &arg)
{
  if (arg.size() < 2 || arg[1] == "") {
    dprintf(out_stream,
            "Invalid number of arguments, expected args: <UNID>"
            "For example add_unid 1\n");
    return SL_STATUS_FAIL;
  }

  char unid[MAXIMUM_UNID_SIZE] = {};
  int unid_value               = std::stoi(arg[1].c_str(), nullptr, 10);
  snprintf(unid, MAXIMUM_UNID_SIZE, UNID_FORMAT, unid_value);

  if (ATTRIBUTE_STORE_INVALID_NODE != eed_attribute_store_get_unid_node(unid)) {
    return SL_STATUS_ALREADY_EXISTS;
  }
  attribute_store_node_t unid_node
    = attribute_store_add_node(ATTRIBUTE_UNID, attribute_store_get_root());
  return attribute_store_set_reported_string(unid_node, unid);
}

static sl_status_t handle_remove_unid(const handle_args_t &arg)
{
  if (arg.size() < 2 || arg[1] == "") {
    dprintf(out_stream,
            "Invalid number of arguments, expected args: <UNID>"
            "For example remove_unid 0001\n");
    return SL_STATUS_FAIL;
  }

  char unid[MAXIMUM_UNID_SIZE] = {};
  int unid_value               = std::stoi(arg[1].c_str(), nullptr, 10);
  snprintf(unid, MAXIMUM_UNID_SIZE, UNID_FORMAT, unid_value);

  attribute_store_node_t unid_node = eed_attribute_store_get_unid_node(unid);
  return attribute_store_delete_node(unid_node);
}

static sl_status_t handle_add_endpoint(const handle_args_t &arg)
{
  if (arg.size() < 3 || arg[1] == "" || arg[2] == "") {
    dprintf(out_stream,
            "Invalid number of arguments, expected args: <UNID>,<Ep>,<Cluster1Name>;<Cluster2Name>;<Cluster3Name>..."
            "For example add_endpoint 1,0,\"OnOff;Level;ColorControl\" or add_endpoint 1,0 for all clusters support\n");
    return SL_STATUS_FAIL;
  }

  char unid[MAXIMUM_UNID_SIZE]     = {};
  int unid_value                   = std::stoi(arg[1].c_str(), nullptr, 10);
  dotdot_endpoint_id_t endpoint_id = std::stoi(arg[2].c_str(), nullptr, 10);
  snprintf(unid, MAXIMUM_UNID_SIZE, UNID_FORMAT, unid_value);
  if (arg.size() > 3) {
    std::string config = arg[3];
    std::string cluster_list;

    // Remove trailing and leading '\"' from cluster list argument
    if (config.front() == '"' && config.back() == '"') {
        config = config.substr(1, config.size() - 2);
    }

    // Check if the config is a device type
    if (isDeviceType(config)) {
      char cluster_list_buffer[MAXIMUM_CLUSTER_LIST_SIZE] = {0};
      if (getClustersForDeviceType(config.c_str(), cluster_list_buffer, sizeof(cluster_list_buffer)) != SL_STATUS_OK) {
        dprintf(out_stream, "Failed to fetch cluster list for deviceType: %s", config.c_str());
        return SL_STATUS_FAIL;
      }
      cluster_list = cluster_list_buffer;
    } else {
        cluster_list = config;
    }

    cluster_config_configure(unid, endpoint_id, cluster_list.c_str());
  }

  if (ATTRIBUTE_STORE_INVALID_NODE
      != eed_attribute_store_get_endpoint_node(unid, endpoint_id)) {
    return SL_STATUS_ALREADY_EXISTS;
  }

  attribute_store_node_t unid_node = eed_attribute_store_get_unid_node(unid);
  if (ATTRIBUTE_STORE_INVALID_NODE == unid_node) {
    unid_node
      = attribute_store_add_node(ATTRIBUTE_UNID, attribute_store_get_root());
    attribute_store_set_reported_string(unid_node, unid);
  }

  return attribute_store_emplace(unid_node,
                                 ATTRIBUTE_ENDPOINT_ID,
                                 &endpoint_id,
                                 sizeof(dotdot_endpoint_id_t));
}

static sl_status_t handle_remove_endpoint(const handle_args_t &arg)
{
  if (arg.size() < 3 || arg[1] == "" || arg[2] == "") {
    dprintf(out_stream,
            "Invalid number of arguments, expected args: <UNID>,<Ep>"
            "For example remove_endpoint 1,0\n");
    return SL_STATUS_FAIL;
  }

  char unid[MAXIMUM_UNID_SIZE]     = {};
  int unid_value                   = std::stoi(arg[1].c_str(), nullptr, 10);
  dotdot_endpoint_id_t endpoint_id = std::stoi(arg[2].c_str(), nullptr, 10);
  snprintf(unid, MAXIMUM_UNID_SIZE, UNID_FORMAT, unid_value);

  attribute_store_node_t endpoint_id_node
    = eed_attribute_store_get_endpoint_node(unid, endpoint_id);
  return attribute_store_delete_node(endpoint_id_node);
}

static sl_status_t handle_add_cluster_support(const handle_args_t &arg)
{
  if (arg.size() < 4 || arg[1] == "" || arg[2] == "" || arg[3] == "") {
    dprintf(out_stream,
            "Invalid number of arguments, expected args: <UNID>,<Ep>,<ClusterName>"
            "For example add_cluster_support 1,0,OnOff\n");
    return SL_STATUS_FAIL;
  }

  char unid[MAXIMUM_UNID_SIZE]     = {};
  int unid_value                   = std::stoi(arg[1].c_str(), nullptr, 10);
  dotdot_endpoint_id_t endpoint_id = std::stoi(arg[2].c_str(), nullptr, 10);
    const char* cluster_name = arg[3].c_str();

  snprintf(unid, MAXIMUM_UNID_SIZE, UNID_FORMAT, unid_value);

  return eed_attribute_store_add_cluster_to_endpoint(unid, endpoint_id, cluster_name);
}

static sl_status_t handle_remove_cluster_support(const handle_args_t &arg)
{
  if (arg.size() < 4 || arg[1] == "" || arg[2] == "" || arg[3] == "") {
    dprintf(out_stream,
            "Invalid number of arguments, expected args: <UNID>,<Ep>,<ClusterName>"
            "For example remove_cluster_support 1,0,OnOff\n");
    return SL_STATUS_FAIL;
  }

  char unid[MAXIMUM_UNID_SIZE]     = {};
  int unid_value                   = std::stoi(arg[1].c_str(), nullptr, 10);
  dotdot_endpoint_id_t endpoint_id = std::stoi(arg[2].c_str(), nullptr, 10);
  const char* cluster_name = arg[3].c_str();
  snprintf(unid, MAXIMUM_UNID_SIZE, UNID_FORMAT, unid_value);

  return eed_attribute_store_remove_cluster_on_endpoint(unid, endpoint_id, cluster_name);
}
