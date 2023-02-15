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
#include "unify_stdin_attribute_resolver_command_handler.h"

// Unify components
#include "attribute_resolver.h"
#include "sl_log.h"

///////////////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////////////
static sl_status_t handle_attribute_resolver_log(const handle_args_t &arg);

///////////////////////////////////////////////////////////////////////////////
// Additional Attribute Resolver commands map
///////////////////////////////////////////////////////////////////////////////
const command_map_t attribute_resolver_commands = {
  {"attribute_resolver_log",
   {"Logs the state of the attribute resolver",
    &handle_attribute_resolver_log}},
};

static sl_status_t handle_attribute_resolver_log(const handle_args_t &)
{
  attribute_resolver_state_log();
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Functions shared within the component
///////////////////////////////////////////////////////////////////////////////
void unify_stdin_add_attribute_resolver_commands()
{
  uic_stdin_add_commands(attribute_resolver_commands);
}