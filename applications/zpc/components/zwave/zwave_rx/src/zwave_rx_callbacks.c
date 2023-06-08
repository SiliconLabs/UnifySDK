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
#include "zwave_rx_callbacks.h"

// Includes from other components
#include "sl_log.h"

// Includes from this component
#include "zwave_rx_internals.h"

// Setup Log ID
#define LOG_TAG "zwave_rx_callbacks"

// Maximum number of callbacks for Z-Wave API started events.
#define ZWAVE_RX_MAXIMUM_ZWAVE_API_STARTED_CALLBACK_SUBSCRIBERS 2

static zwave_api_started_callback_t zwave_api_started_callbacks
  [ZWAVE_RX_MAXIMUM_ZWAVE_API_STARTED_CALLBACK_SUBSCRIBERS]
  = {};

///////////////////////////////////////////////////////////////////////////////
// Public functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_rx_register_zwave_api_started_callback(
  zwave_api_started_callback_t callback)
{
  if (callback == NULL) {
    return SL_STATUS_FAIL;
  }

  // Do not register twice the same function.
  for (uint8_t i = 0;
       i < ZWAVE_RX_MAXIMUM_ZWAVE_API_STARTED_CALLBACK_SUBSCRIBERS;
       i++) {
    if (zwave_api_started_callbacks[i] == callback) {
      return SL_STATUS_OK;
    }
  }
  // If not registered, look for the first empty spot
  for (uint8_t i = 0;
       i < ZWAVE_RX_MAXIMUM_ZWAVE_API_STARTED_CALLBACK_SUBSCRIBERS;
       i++) {
    if (zwave_api_started_callbacks[i] == NULL) {
      zwave_api_started_callbacks[i] = callback;
      return SL_STATUS_OK;
    }
  }

  sl_log_error(LOG_TAG,
               "Cannot register Z-Wave API started callbacks. Increase "
               "ZWAVE_RX_MAXIMUM_ZWAVE_API_STARTED_CALLBACK_SUBSCRIBERS "
               "to make this work.");
  return SL_STATUS_NO_MORE_RESOURCE;
}

///////////////////////////////////////////////////////////////////////////////
// Component only functions
///////////////////////////////////////////////////////////////////////////////
void zwave_rx_on_zwave_api_started()
{
  sl_log_info(LOG_TAG,
              "Notifying other components that the Z-Wave API just sent a "
              "Started message. ");

  for (uint8_t i = 0;
       i < ZWAVE_RX_MAXIMUM_ZWAVE_API_STARTED_CALLBACK_SUBSCRIBERS;
       i++) {
    if (zwave_api_started_callbacks[i] != NULL) {
      zwave_api_started_callbacks[i]();
    }
  }
}
