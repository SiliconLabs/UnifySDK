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

// Includes from this component
#include "zwave_command_class_powerlevel.h"
#include "zwave_command_classes_utils.h"

// Generic includes
#include <string.h>
#include <stdio.h>

// Includes from other components
#include "ZW_classcmd.h"
#include "zwave_command_handler.h"
#include "zwave_controller_command_class_indices.h"
#include "zwave_controller_connection_info.h"
#include "zwave_tx.h"
#include "zwapi_protocol_basis.h"
#include "zwapi_init.h"
#include "ctimer.h"
#include "clock.h"
#include "sl_log.h"

#define LOG_TAG "zwave_command_class_powerlevel"

// Private function prototypes
static sl_status_t
  zwave_command_class_powerlevel_handle_powerlevel_test_node_get(
    const zwave_controller_connection_info_t *connection_info);

// Private variables
static power_level_test_t power_level_test      = {};
static powerlevel_setting_t power_level_setting = {};

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
static void reset_power_level()
{
  ctimer_stop(&power_level_setting.timer);
  power_level_setting.power_level     = NORMAL_POWER;
  power_level_setting.expiration_time = 0;
  zwapi_set_rf_power_level(power_level_setting.power_level);
}

static void set_power_level(rf_power_level_t new_power_level, uint8_t timeout)
{
  power_level_setting.power_level = new_power_level;

  if (new_power_level == NORMAL_POWER) {
    reset_power_level();
    return;
  } else {
    zwapi_set_rf_power_level(power_level_setting.power_level);
    ctimer_set(&power_level_setting.timer,
               timeout * CLOCK_SECOND,
               &reset_power_level,
               0);
    power_level_setting.expiration_time = clock_time() + timeout * CLOCK_SECOND;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Callback functions
///////////////////////////////////////////////////////////////////////////////
static void power_level_test_send_data_callback(
  uint8_t status, const zwapi_tx_report_t *tx_status, void *user)
{
  // Decrease the remaining amount of frames to send
  power_level_test.frame_count--;

  if (status == TRANSMIT_COMPLETE_OK) {
    power_level_test.acknowledged_frames_count++;
  }

  if (power_level_test.frame_count > 0) {
    if (zwave_tx_send_test_frame(power_level_test.destination_node,
                                 power_level_test.power_level,
                                 power_level_test_send_data_callback,
                                 NULL,
                                 NULL)
        != SL_STATUS_OK) {
      // TX queue refused our frame, it's kind of bad. We don't want to stress
      // it more by going into a crazy recursion, especially for a power level
      // test. Let's just abort the test by makind a callback for the final frame.
      power_level_test.frame_count = 1;
      power_level_test_send_data_callback(TRANSMIT_COMPLETE_FAIL, NULL, NULL);
    }
    return;
  }

  if (power_level_test.acknowledged_frames_count == 0) {
    power_level_test.status = TEST_FAILED;
  } else {
    power_level_test.status = TEST_SUCCESSFUL;
  }

  // Send a notification to the controlling node when we are done
  zwave_command_class_powerlevel_handle_powerlevel_test_node_get(
    &power_level_test.connection);
}

///////////////////////////////////////////////////////////////////////////////
// Powerlevel command handlers
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_powerlevel_handle_powerlevel_get(
  const zwave_controller_connection_info_t *connection_info)
{
  if (connection_info && connection_info->local.is_multicast) {
    return SL_STATUS_OK;
  }

  uint8_t timeout = 0;
  if (power_level_setting.expiration_time > 0) {
    timeout
      = (uint8_t)((power_level_setting.expiration_time - clock_time()) / 1000);
  }

  ZW_POWERLEVEL_REPORT_FRAME report
    = {.cmdClass   = COMMAND_CLASS_POWERLEVEL,
       .cmd        = POWERLEVEL_REPORT,
       .powerLevel = (uint8_t)power_level_setting.power_level,
       .timeout    = timeout};

  return zwave_command_class_send_report(connection_info,
                                         sizeof(report),
                                         (uint8_t *)&report);
}

static sl_status_t
  zwave_command_class_powerlevel_handle_powerlevel_set(const uint8_t *frame,
                                                       uint16_t frame_length)
{
  const ZW_POWERLEVEL_SET_FRAME *powerlevel_set_frame
    = (const ZW_POWERLEVEL_SET_FRAME *)frame;

  if (frame_length < sizeof(ZW_POWERLEVEL_SET_FRAME)) {
    sl_log_error(LOG_TAG,
                 "Powerlevel Set frame length (%d bytes) is shorter than the "
                 "minimum required. Ignoring\n",
                 sizeof(ZW_POWERLEVEL_SET_FRAME));
    return SL_STATUS_NOT_SUPPORTED;
  }
  if (powerlevel_set_frame->powerLevel > MINIMUM_POWER) {
    sl_log_error(LOG_TAG,
                 "Powerlevel Set command powerlevel value is unknown"
                 "(value = %d). Ignoring.",
                 powerlevel_set_frame->powerLevel);
    return SL_STATUS_FAIL;
  }

  if (powerlevel_set_frame->powerLevel == NORMAL_POWER) {
    reset_power_level();
    return SL_STATUS_OK;
  }
  if (powerlevel_set_frame->timeout == 0) {
    return SL_STATUS_FAIL;
  }
  set_power_level(powerlevel_set_frame->powerLevel,
                  powerlevel_set_frame->timeout);

  return SL_STATUS_OK;
}

static sl_status_t
  zwave_command_class_powerlevel_handle_powerlevel_test_node_get(
    const zwave_controller_connection_info_t *connection_info)
{
  if (connection_info && connection_info->local.is_multicast) {
    return SL_STATUS_OK;
  }

  ZW_POWERLEVEL_TEST_NODE_REPORT_FRAME report = {
    .cmdClass          = COMMAND_CLASS_POWERLEVEL,
    .cmd               = POWERLEVEL_TEST_NODE_REPORT,
    .testNodeid        = (uint8_t)power_level_test.destination_node,
    .statusOfOperation = (uint8_t)power_level_test.status,
    .testFrameCount1 = (power_level_test.acknowledged_frames_count >> 8) & 0xFF,
    .testFrameCount2 = (power_level_test.acknowledged_frames_count >> 0) & 0xFF,
  };

  return zwave_command_class_send_report(connection_info,
                                         sizeof(report),
                                         (uint8_t *)&report);
}

static sl_status_t
  zwave_command_class_powerlevel_handle_powerlevel_test_node_set(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  const ZW_POWERLEVEL_TEST_NODE_SET_FRAME *set_frame
    = (const ZW_POWERLEVEL_TEST_NODE_SET_FRAME *)frame_data;

  if (frame_length < sizeof(ZW_POWERLEVEL_TEST_NODE_SET_FRAME)) {
    sl_log_error(LOG_TAG,
                 "Power Level Test Node Set frame is %d bytes long, which is "
                 "shorter than the minimum required. Ignoring\n",
                 sizeof(ZW_POWERLEVEL_TEST_NODE_SET_FRAME));
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (power_level_test.frame_count > 0) {
    // Another test ongoing, we support only 1 concurrent test
    return SL_STATUS_FAIL;
  }

  if (set_frame->powerLevel > MINIMUM_POWER) {
    return SL_STATUS_FAIL;
  }

  power_level_test.frame_count
    = (uint16_t)(set_frame->testFrameCount1 << 8 | set_frame->testFrameCount2);
  if (power_level_test.frame_count == 0) {
    return SL_STATUS_FAIL;
  }

  power_level_test.status                    = TEST_IN_PROGRESS;
  power_level_test.destination_node          = set_frame->testNodeid;
  power_level_test.power_level               = set_frame->powerLevel;
  power_level_test.acknowledged_frames_count = 0;
  power_level_test.connection                = *connection_info;

  if (zwave_tx_send_test_frame(power_level_test.destination_node,
                               power_level_test.power_level,
                               power_level_test_send_data_callback,
                               NULL,
                               NULL)
      != SL_STATUS_OK) {
    // TX queue refused our frame, we consider it as a Transmit Failed, but
    // the send data callback will try once more before aborting the test
    // completely.
    power_level_test_send_data_callback(TRANSMIT_COMPLETE_FAIL, NULL, NULL);
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_powerlevel_support_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame_data[COMMAND_CLASS_INDEX] != COMMAND_CLASS_POWERLEVEL) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case POWERLEVEL_GET:
      return zwave_command_class_powerlevel_handle_powerlevel_get(
        connection_info);

    case POWERLEVEL_SET:
      return zwave_command_class_powerlevel_handle_powerlevel_set(frame_data,
                                                                  frame_length);

    case POWERLEVEL_TEST_NODE_GET:
      return zwave_command_class_powerlevel_handle_powerlevel_test_node_get(
        connection_info);

    case POWERLEVEL_TEST_NODE_SET:
      return zwave_command_class_powerlevel_handle_powerlevel_test_node_set(
        connection_info,
        frame_data,
        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

sl_status_t zwave_command_class_powerlevel_init()
{
  memset(&power_level_test, 0, sizeof(power_level_test));
  reset_power_level();

  zwave_command_handler_t handler = {};
  handler.support_handler    = &zwave_command_class_powerlevel_support_handler;
  handler.control_handler    = NULL;
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  handler.command_class      = COMMAND_CLASS_POWERLEVEL;
  handler.version            = POWERLEVEL_VERSION;
  handler.command_class_name = "Powerlevel";
  handler.manual_security_validation = false;

  return zwave_command_handler_register_handler(handler);
}
