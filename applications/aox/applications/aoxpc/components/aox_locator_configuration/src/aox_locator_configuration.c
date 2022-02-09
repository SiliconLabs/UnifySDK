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
#include "aox_locator_configuration.h"
#include "aox_locator_configuration_internal.h"

// AoXPC components
#include "aoxpc_unid.h"

// Unify components
#include "sl_log.h"

#define LOG_TAG "aox_locator_configuration"

// callback for the configuration udpates
static aox_locator_on_configuration_updated_t configuration_update_callback
  = NULL;

///////////////////////////////////////////////////////////////////////////////
// Function shared with the component
///////////////////////////////////////////////////////////////////////////////
void aox_locator_notify_configuration_was_updated()
{
  if (configuration_update_callback != NULL) {
    // It's up to the application to decide if they want to back-up the
    // attribute configuration in the attribute store.
    configuration_update_callback();
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t aox_locator_configuration_init()
{
  sl_log_debug(LOG_TAG, "Initializing AoXLocator cluster");

  aox_locator_reset_configuration();

  // Register a callback to DotDot MQTT for WriteAttributes commands:
  uic_mqtt_dotdot_set_aox_locator_write_attributes_callback(
    &aox_locator_write_attributes_callback);

  publish_aox_locator_attributes_to_mqtt();

  return SL_STATUS_OK;
}

sl_status_t aox_locator_register_on_configuration_updated_callback(
  const aox_locator_on_configuration_updated_t new_callback)
{
  if (configuration_update_callback == NULL) {
    configuration_update_callback = new_callback;
    return SL_STATUS_OK;
  }

  // Callback is already set, we support only 1
  return SL_STATUS_FAIL;
}