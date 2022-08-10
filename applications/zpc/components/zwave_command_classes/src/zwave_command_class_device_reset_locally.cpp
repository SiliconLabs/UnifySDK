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
#include "zwave_command_class_device_reset_locally.h"
#include "zwave_command_class_agi.h"
#include "zwave_command_class_association_send.h"

// Includes from other components
#include "ZW_classcmd.h"
#include "zwave_network_management.h"
#include "zwave_network_management_state.h"
#include "sys/ctimer.h"
#include "zwave_command_class_indices.h"
#include "zwave_controller.h"
#include "zwave_controller_callbacks.h"
#include "zwave_command_handler.h"
#include "sl_log.h"

// Generic includes
#include <map>
#include <assert.h>

constexpr char LOG_TAG[] = "zwave_command_class_device_reset_locally";
// Maximum time trying to notify Lifeline destinations that we are resetting, in ms
// We will wait 30 seconds, then move on.
constexpr uint32_t MAXIMUM_TIME_FOR_RESET_NOTIFICATION = 30000;

static std::map<zwave_node_id_t, uint8_t> nodes_to_be_removed;
struct ctimer timer;
static void
  zwave_command_class_device_reset_on_node_deleted(zwave_node_id_t node_id);
static zwave_controller_callbacks_t zwave_command_class_device_reset_callbacks
  = {
    .on_node_deleted = zwave_command_class_device_reset_on_node_deleted,
};

static void on_timeout_device_reset(void *data)
{
  auto it = nodes_to_be_removed.begin();
  if (it != nodes_to_be_removed.end()) {
    if (it->second < DEVICE_RESET_LOCALLY_REMOVE_RETRIES_MAX) {
      if (zwave_network_management_get_state() == NM_IDLE) {
        zwave_network_management_remove_failed(it->first);
        it->second++;
        if (it->second == DEVICE_RESET_LOCALLY_REMOVE_RETRIES_MAX) {
          ctimer_set(&timer,
                     DEVICE_RESET_LOCALLY_REMOVE_TRIGGER_TIMEOUT_DEFAULT,
                     on_timeout_device_reset,
                     0);
        } else {
          ctimer_set(&timer,
                     5 * DEVICE_RESET_LOCALLY_REMOVE_TRIGGER_TIMEOUT_DEFAULT,
                     on_timeout_device_reset,
                     0);
        }
      } else {
        ctimer_restart(&timer);
      }
    } else {
      nodes_to_be_removed.erase(it);
      // Iterative call to trigger removing next node (if any) immediately
      on_timeout_device_reset(nullptr);
    }
  }
}

static void
  zwave_command_class_device_reset_on_node_deleted(zwave_node_id_t node_id)
{
  nodes_to_be_removed.erase(node_id);
}

static void on_reset_notification_send_complete(sl_status_t status, void *user)
{
  // Tell the Z-Wave controller to continue its reset journey.
  sl_log_debug(LOG_TAG,
               "Reset step: Device Reset Locally Notification "
               "to the lifeline destinations completed.");
  zwave_controller_on_reset_step_complete(
    ZWAVE_CONTROLLER_DEVICE_RESET_LOCALLY_STEP_PRIORITY);
}

static sl_status_t zwave_command_class_device_reset_on_zpc_reset()
{
  sl_log_info(LOG_TAG,
              "Reset step: Sending a Device Reset Locally Notification "
              "to the lifeline destinations.");

  // We leave our network behind, don't care about nodes to be removed.
  nodes_to_be_removed.clear();

  // Notify our Lifeline destinations.
  const uint8_t reset_notification[]
    = {COMMAND_CLASS_DEVICE_RESET_LOCALLY, DEVICE_RESET_LOCALLY_NOTIFICATION};
  zwave_tx_options_t tx_options = {};
  tx_options.discard_timeout_ms = MAXIMUM_TIME_FOR_RESET_NOTIFICATION;
  tx_options.qos_priority = ZWAVE_TX_QOS_RECOMMENDED_TIMING_CRITICAL_PRIORITY;
  tx_options.number_of_responses = 0;

  sl_status_t send_status
    = zwave_association_send_to_group(LIFELINE_GROUP_ID,
                                      sizeof(reset_notification),
                                      reset_notification,
                                      true,
                                      &tx_options,
                                      0,
                                      &on_reset_notification_send_complete,
                                      nullptr);

  // If send_status is not SL_STATUS_OK,
  // the Z-Wave Controller won't wait for a callback
  return send_status;
}

sl_status_t zwave_command_class_device_reset_locally_support_handler(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  assert(frame_data[COMMAND_CLASS_INDEX] == COMMAND_CLASS_DEVICE_RESET_LOCALLY);

  // No commands here, though we must have a handler function in order
  // to support this Command Class.
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t zwave_command_class_device_reset_locally_control_handler(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  assert(frame_data[COMMAND_CLASS_INDEX] == COMMAND_CLASS_DEVICE_RESET_LOCALLY);

  switch (frame_data[COMMAND_INDEX]) {
    case DEVICE_RESET_LOCALLY_NOTIFICATION:
      nodes_to_be_removed.try_emplace(connection->remote.node_id, 0);
      ctimer_set(&timer,
                 DEVICE_RESET_LOCALLY_REMOVE_TRIGGER_TIMEOUT_DEFAULT,
                 &on_timeout_device_reset,
                 0);
      return SL_STATUS_OK;

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

sl_status_t zwave_command_class_device_reset_locally_init()
{
  // Tell AGI that we send something via the lifeline :
  zwave_command_class_agi_add_group_commands(LIFELINE_GROUP_ID,
                                             COMMAND_CLASS_DEVICE_RESET_LOCALLY,
                                             DEVICE_RESET_LOCALLY_NOTIFICATION);

  // Register zwave controller callbacks for node delete
  zwave_controller_register_callbacks(
    &zwave_command_class_device_reset_callbacks);

  // Tell the Z-Wave Controller that we have to do something on reset
  zwave_controller_register_reset_step(
    &zwave_command_class_device_reset_on_zpc_reset,
    ZWAVE_CONTROLLER_DEVICE_RESET_LOCALLY_STEP_PRIORITY);

  zwave_command_handler_t handler = {0};
  handler.support_handler
    = &zwave_command_class_device_reset_locally_support_handler;
  handler.control_handler
    = &zwave_command_class_device_reset_locally_control_handler;
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  handler.command_class      = COMMAND_CLASS_DEVICE_RESET_LOCALLY;
  handler.command_class_name = "Device Reset Locally";
  handler.version            = DEVICE_RESET_LOCALLY_VERSION;
  handler.manual_security_validation = false;

  return zwave_command_handler_register_handler(handler);
}