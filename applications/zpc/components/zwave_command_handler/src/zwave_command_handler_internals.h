/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef ZWAVE_COMMAND_HANDLER_INTERNALS_H
#define ZWAVE_COMMAND_HANDLER_INTERNALS_H

// Includes from this component
#include "zwave_command_handler.h"

// Includes from other components
#include "zwave_controller_connection_info.h"
#include "zwave_controller.h"

// Generic includes
#include <vector>
#include <set>

/**
 * @file zwave_command_handler_internals.h
 * @addtogroup zwave_command_handler_internals Internal definitions for the Z-Wave Command Handler
 * @ingroup zwave_command_handler
 * @brief Shared internal functions and variables for the Z-Wave Command Handler.
 *  * 
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

// Our shared variables between source files in this module:
extern zwave_controller_encapsulation_scheme_t zpc_highest_scheme;

// Compare 2 command class handlers.
struct zwave_command_handler_compare {
  bool operator()(const zwave_command_handler_t &a,
                  const zwave_command_handler_t &b) const
  {
    return a.command_class < b.command_class;
  }
};

typedef std::set<zwave_command_handler_t,
                 zwave_command_handler_compare>::iterator
  zwave_command_handler_it_t;

// The command class handler list (shared amongst files in the component)
extern std::multiset<zwave_command_handler_t, zwave_command_handler_compare>
  command_handler_list;

/**
 * @brief Verifies if a Command Class is supported at a certain Security level
 * 
 * If a handler is supported with NO_SCHEME we allow the frame to
 * reach the handler no matter what scheme it was received on.
 *
 * If a handler has a minimal security scheme it has to be sent with the net_scheme
 * i.e. the highest scheme supported by the node.
 * @param it      The Command Class handler iterator
 * @param scheme  The Security Scheme at which the frame was received.
 * 
 * @returns true if the Command Class is supported for the given scheme
 * @returns false if the Command Class is not supported for the given scheme
 */
bool supports_frame_at_security_level(
  zwave_command_handler_it_t it,
  zwave_controller_encapsulation_scheme_t scheme);

#ifdef __cplusplus
}
#endif
/** @} end zwave_command_handler_internals */

#endif  //ZWAVE_COMMAND_HANDLER_INTERNALS_H