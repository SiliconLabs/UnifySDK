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
// Includes from this component
#include "zwave_command_handler.h"
#include "zwave_command_handler_internals.h"
#include "zwave_command_handler_callbacks.h"

// Includes from other components
#include "zwave_controller.h"
#include "zwave_controller_callbacks.h"
#include "zwave_controller_storage.h"
#include "zwave_controller_keyset.h"
#include "zwave_controller_utils.h"
#include "zwave_security_validation.h"
#include "zwave_rx.h"
#include "sl_log.h"
#include "zwave_network_management.h"  // for zwave_network_management_get_home_id / get_node_id

// Interface includes
#include "ZW_classcmd.h"
#include "zwave_command_class_version_types.h"
#include "zwave_command_type.hpp"

// Generic includes
#include <iomanip>
#include <sstream>
#include <vector>
#include <set>
#include <assert.h>
#include <cstdio>

/// Setup Log ID
#define LOG_TAG "zwave_command_handler"

// Instance of shared variables within the component
zwave_controller_encapsulation_scheme_t zpc_highest_scheme
  = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;

std::multiset<zwave_command_handler_t, zwave_command_handler_compare>
  command_handler_list;

// Private variables
static zwave_controller_callbacks_t zwave_command_handler_callbacks = {
  .on_new_network_entered        = zwave_command_handler_on_new_network_entered,
  .on_application_frame_received = zwave_command_handler_on_frame_received,
};

///////////////////////////////////////////////////////////////////////////////
// Shared functions within the component
///////////////////////////////////////////////////////////////////////////////
bool supports_frame_at_security_level(
  zwave_command_handler_it_t it, zwave_controller_encapsulation_scheme_t scheme)
{
  // Return false if the Command Class is not supported
  if (it->support_handler == nullptr) {
    return false;
  }

  if (it->minimal_scheme == ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME) {
    return (scheme == zpc_highest_scheme);
  } else if ((it->minimal_scheme == ZWAVE_CONTROLLER_ENCAPSULATION_NONE)
             || (zwave_controller_encapsulation_scheme_greater_equal(
                   scheme,
                   zpc_highest_scheme)
                 && zwave_controller_encapsulation_scheme_greater_equal(
                   scheme,
                   it->minimal_scheme))) {
    return true;
  } else {
    return false;
  }
}

sl_status_t zwave_command_handler_dispatch(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  zwave_command_handler_t handler_to_invoke = {};
  handler_to_invoke.command_class           = frame_data[0];
  sl_status_t rc                            = SL_STATUS_NOT_SUPPORTED;

  //Check if this frame is a multicast get
  if (connection->local.is_multicast) {
    if( ZwaveCommandClassType::get_type(frame_data[0],frame_data[1]) == ZwaveCommandClassType::type_t::GET ) {
        sl_log_debug(LOG_TAG,"Multicast get frame dropped");
        return SL_STATUS_NOT_SUPPORTED;
      }
  }

  auto [it, end] = command_handler_list.equal_range(handler_to_invoke);
  for (; it != end; it++) {
    // Dispatch the message to the handler.
    // Try the control handler:
    if (it->control_handler) {
      if (it->manual_security_validation) {
        // Handler validates security itself.
        rc = it->control_handler(connection, frame_data, frame_length);
      } else if (zwave_security_validation_is_security_valid_for_control(
                   connection)) {
        rc = it->control_handler(connection, frame_data, frame_length);
      }
    }

    if (rc != SL_STATUS_NOT_SUPPORTED) {
      return rc;
    }

    // Try the support handler:
    if (it->support_handler) {
      if (it->manual_security_validation) {
        // Handler validates security itself.
        rc = it->support_handler(connection, frame_data, frame_length);
      } else if (zwave_security_validation_is_security_valid_for_support(
                   it->minimal_scheme,
                   connection)) {
        rc = it->support_handler(connection, frame_data, frame_length);
      }
    }

    if (rc != SL_STATUS_NOT_SUPPORTED) {
      return rc;
    }
  }

  // If we arrive here, we have no handler for the Command Class.
  return rc;
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_handler_init(void)
{
  /// We will assume all keys if the Z-Wave Controller storage can't find our own keys.
  zwave_keyset_t my_granted_keys = 0x87;
  if (SL_STATUS_OK
      != zwave_controller_storage_get_node_granted_keys(
        zwave_network_management_get_node_id(),
        &my_granted_keys)) {
    sl_log_warning(LOG_TAG, "Cannot read Z-Wave Controller's granted keys");
  };
  zwave_kex_fail_type_t kex_fail = ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE;

  zwave_command_handler_on_new_network_entered(
    zwave_network_management_get_home_id(),
    zwave_network_management_get_node_id(),
    my_granted_keys,
    kex_fail);

  // Register on_frame_received and on_new_network_entered callbacks
  zwave_controller_register_callbacks(&zwave_command_handler_callbacks);

  return SL_STATUS_OK;
}

int zwave_command_handler_teardown(void)
{
  // Stop being notified of incoming frames and network changes
  command_handler_list.clear();
  zwave_controller_deregister_callbacks(&zwave_command_handler_callbacks);
  return 0;
}

sl_status_t zwave_command_handler_register_handler(
  zwave_command_handler_t new_command_class_handler)
{
  auto it = command_handler_list.find(new_command_class_handler);
  if (it != command_handler_list.end()) {
    if (it->version != new_command_class_handler.version) {
      sl_log_error(LOG_TAG,
                   "Attempt to register inconsistent command handler for CC %i",
                   new_command_class_handler.command_class);
      sl_log_error(LOG_TAG,
                   "version %2d vs %2d",
                   new_command_class_handler.version,
                   it->version);
      assert(0);
    }
    if ((it->control_handler && new_command_class_handler.control_handler)
        || (it->support_handler && new_command_class_handler.support_handler)) {
      // Do not make a fuzz about it, CCs will be attempted to be registered again after reset
      sl_log_debug(LOG_TAG,
                   "Command Class 0x%X already registered in Command Class "
                   "handlers. Skipping",
                   it->command_class);
      return SL_STATUS_ALREADY_INITIALIZED;
    }
  }

  command_handler_list.insert(new_command_class_handler);
  return SL_STATUS_OK;
}

zwave_cc_version_t
  zwave_command_handler_get_version(zwave_command_class_t command_class)
{
  zwave_command_handler_t handler_to_read = {};
  handler_to_read.command_class           = command_class;
  zwave_command_handler_it_t it = command_handler_list.find(handler_to_read);

  if (it != command_handler_list.end()) {
    // If the command class is disabled, we pretend it does not exist
    if (it->support_handler == nullptr && it->control_handler == nullptr) {
      return 0;
    }
    // Else return its version number
    return it->version;
  }
  // If the handler is not registered, we return 0.
  return 0;
}

bool zwave_command_handler_controls(zwave_command_class_t command_class)
{
  zwave_command_handler_t handler_to_read = {};
  handler_to_read.command_class           = command_class;

  auto [it, end] = command_handler_list.equal_range(handler_to_read);
  for (; it != end; it++) {
    if (it->control_handler)
      return true;
  }
  return false;
}

void zwave_command_handler_print_info(int fd)
{
  std::stringstream ss;
  ss << std::endl;
  // clang-format off
  ss << "| " << "Command Class                 " << " | Version | Support | Control | Security Level              | Comment |" << std::endl;
  ss << "| " << "------------------------------" << " | ------- | ------- | ------- | --------------------------- | ------- |" << std::endl;
  // clang-format on

  // Sort our list alphabetically
  std::multiset<zwave_command_handler_t,
                zwave_command_handler_compare_alphabetic>
    sorted_list;
  for (auto iter = command_handler_list.begin();
       iter != command_handler_list.end();
       iter++) {
    sorted_list.insert(*iter);
  }

  // Then print the info of each element
  for (auto iter = sorted_list.begin(); iter != sorted_list.end(); iter++) {
    zwave_command_handler_t handler = *iter;

    // The purpose of this is to combine multiple registered handlers into one.
    // This could be done much smoother if we had registed two independent command handlers lists.
    auto iter2 = iter;
    iter2++;
    while (iter2 != sorted_list.end()) {
      if (iter2->command_class == iter->command_class) {
        if (handler.control_handler == nullptr
            && iter2->control_handler != nullptr) {
          handler.control_handler = iter2->control_handler;
        }
        if (handler.support_handler == nullptr
            && iter2->support_handler != nullptr) {
          handler.support_handler = iter2->support_handler;
        }
        if (iter2->support_handler) {
          handler.minimal_scheme = iter2->minimal_scheme;
        }
        iter2++;
        iter++;
      } else {
        break;
      }
    }

    ss << "| ";
    ss << std::setw(30) << std::left << handler.command_class_name;
    ss << " | ";
    ss << std::setw(sizeof("Version") - 1) << std::right
       << std::to_string(handler.version);
    ss << " | ";
    ss << std::setw(sizeof("Support") - 1)
       << (handler.support_handler != nullptr ? "x" : " ");
    ss << " | ";
    ss << std::setw(sizeof("Control") - 1)
       << (handler.control_handler != nullptr ? "x" : " ");
    ss << " | ";
    std::string security_level;
    // Security Level only makes sense for command classes that have Support set
    if (handler.support_handler) {
      security_level = zwave_network_scheme_str(handler.minimal_scheme);
    } else {
      security_level = "N/A";
    }
    // With is the maximum with of the value returned by zwave_network_scheme_str
    ss << std::setw(27) << std::left << security_level;
    ss << " | ";
    ss << std::setw(sizeof("Comment") - 1)
       << (handler.comments == nullptr ? "" : handler.comments);
    ss << " |" << std::endl;
  }
  if (fd < 0) {
    sl_log_info(LOG_TAG, ss.str().c_str());
  } else {
    dprintf(fd, "%s", ss.str().c_str());
  }
}