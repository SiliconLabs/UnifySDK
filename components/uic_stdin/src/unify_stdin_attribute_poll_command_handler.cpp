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
#include "unify_stdin_attribute_poll_command_handler.h"

// Unify components
#include "attribute_poll.h"
#include "sl_log.h"

#include <cfloat>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

///////////////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////////////
static sl_status_t handle_attribute_poll_register(const handle_args_t &arg);
static sl_status_t handle_attribute_poll_deregister(const handle_args_t &arg);
static sl_status_t handle_attribute_poll_print(const handle_args_t &arg);

///////////////////////////////////////////////////////////////////////////////
// Additional Attribute Store commands map
///////////////////////////////////////////////////////////////////////////////
const command_map_t attribute_poll_commands
  = {{"attribute_poll_print",
      {"print list of pending attributes to be polled",
       &handle_attribute_poll_print}},
     {"attribute_poll_register",
      {COLOR_START
       "<AttributeID>(,<INTERVAL>)" COLOR_END
       "Register a <Attribute ID> to be polled every <INTERVAL> seconds, "
       "if no <INTERVAL> is supplied, the default interval is used",
       &handle_attribute_poll_register}},
     {"attribute_poll_deregister",
      {COLOR_START "<Attribute ID>" COLOR_END
                   "Deregister <Attribute ID> for polling",
       &handle_attribute_poll_deregister}}};

///////////////////////////////////////////////////////////////////////////////
// Globals and helper functions
///////////////////////////////////////////////////////////////////////////////
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
    dprintf(uic_stdin_get_output_fd(),
            "%s: Invalid argument: %s\n",
            arg[0].c_str(),
            e.what());
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
    dprintf(uic_stdin_get_output_fd(),
            "%s: Invalid argument: %s\n",
            arg[0].c_str(),
            e.what());
    return SL_STATUS_FAIL;
  }
}

static sl_status_t handle_attribute_poll_print(const handle_args_t &arg)
{
  attribute_poll_print();
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Functions shared within the component
///////////////////////////////////////////////////////////////////////////////
void unify_stdin_add_attribute_poll_commands()
{
  uic_stdin_add_commands(attribute_poll_commands);
}
