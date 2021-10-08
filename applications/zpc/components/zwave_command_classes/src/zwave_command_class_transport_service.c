/******************************************************************************
* # License
* <b>Copyright 2021  Silicon Laboratories Inc. www.silabs.com</b>
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
#include "zwave_command_class_transport_service.h"

// Generic includes
#include <assert.h>
#include <stdbool.h>

// Includes from other components
#include "ZW_classcmd.h"
#include "zwave_command_handler.h"

#include "sl_log.h"
#define LOG_TAG "zwave_command_class_transport_service"

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
// None

///////////////////////////////////////////////////////////////////////////////
// Command Handler functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_transport_service_control_handler(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Nothing is really controlled here. Transport service is a transport exclusively.
  sl_log_warning(LOG_TAG,
                 "Incoming application level frame for the Transport Service "
                 "Command Class. This must not have happened, it should have "
                 "been processed by the transport layer.");
  return SL_STATUS_NOT_SUPPORTED;
}

static sl_status_t zwave_command_class_transport_service_support_handler(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Nothing is really supported here. Transport service is a transport exclusively.
  return SL_STATUS_NOT_SUPPORTED;
}
///////////////////////////////////////////////////////////////////////////////
// Attribute resolution functions
///////////////////////////////////////////////////////////////////////////////
// None

///////////////////////////////////////////////////////////////////////////////
// Attribute update callbacks
///////////////////////////////////////////////////////////////////////////////
// None

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_transport_service_init()
{
  // Register our handler to the Z-Wave CC framework:
  // This is only to advertise command classes properly in our NIF.
  zwave_command_handler_t handler = {};
  handler.support_handler
    = &zwave_command_class_transport_service_support_handler;
  handler.control_handler
    = &zwave_command_class_transport_service_control_handler;
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_TRANSPORT_SERVICE_V2;
  handler.version                    = TRANSPORT_SERVICE_VERSION_V2;
  handler.command_class_name         = "Transport Service";

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}