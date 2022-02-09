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
#include "zwave_controller_transport.h"
#include "zwave_controller_internal.h"
#include "zwave_controller_transport_internal.h"

// ZPC includes
#include "zwave_tx.h"
#include "zwave_rx.h"

// Generic includes
#include <set>

// UIC includes
#include "sl_log.h"
constexpr char LOG_TAG[] = "zwave_controller_transport";

struct transport_compare {
  bool operator()(const zwave_controller_transport_t &lhs,
                  const zwave_controller_transport_t &rhs) const
  {
    return lhs.priority > rhs.priority;
  }
};

static std::set<zwave_controller_transport_t, struct transport_compare>
  transports;

sl_status_t zwave_controller_transport_init()
{
  transports.clear();
  return SL_STATUS_OK;
}

sl_status_t zwave_controller_transport_register(
  const zwave_controller_transport_t *new_transport)
{
  for (auto t: transports) {
    if ((t.priority == new_transport->priority)
        || (t.command_class == new_transport->command_class)) {
      sl_log_error(
        LOG_TAG,
        "Registered two transport with same priority or command class");
      return SL_STATUS_FAIL;
    }
  }
  transports.insert(*new_transport);
  return SL_STATUS_OK;
}

/**
 * @brief Get the transport by class object
 *
 * This function retrieves the transport for matching a paticular class.
 * If no transport is found this function return nullptr
 *
 * @param cmd_class Command class to fe
 * @return const zwave_controller_transport_t*
 */
static const zwave_controller_transport_t *
  get_transport_by_class(zwave_command_class_t cmd_class)
{
  for (const zwave_controller_transport_t &t: transports) {
    if (t.command_class == cmd_class) {
      return &t;
    }
  }
  return nullptr;
}

zwave_cc_version_t zwave_controller_transport_is_encapsulation_cc(
  zwave_command_class_t command_class)
{
  const zwave_controller_transport_t *t;
  //Check if this is an encapsulation command class and registered by
  //the Z-Wave controller transport framework
  t = get_transport_by_class(command_class);
  if (t) {
    return t->version;
  }
  return 0;
}

sl_status_t zwave_controller_transport_on_frame_received(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  const zwave_controller_transport_t *t;
  t = get_transport_by_class(frame_data[0]);

  // Do we support this, we decode this only based of the command class
  if (t && t->on_frame_received) {
    return t->on_frame_received(connection_info,
                                rx_options,
                                frame_data,
                                frame_length);
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t zwave_controller_transport_send_data(
  const zwave_controller_connection_info_t *connection,
  uint16_t data_length,
  const uint8_t *data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user,
  zwave_tx_session_id_t parent_session_id)
{
  // The transports are arranged in an odered set according to priority,
  // so the transport with the highest priority will be executed first.
  for (const zwave_controller_transport_t &t: transports) {
    sl_status_t send_data_status;
    if (t.send_data) {
      send_data_status = t.send_data(connection,
                                     data_length,
                                     data,
                                     tx_options,
                                     on_send_complete,
                                     user,
                                     parent_session_id);
      if (send_data_status != SL_STATUS_NOT_SUPPORTED) {
        return send_data_status;
      }
    }
  }
  // If none of the transport supports the
  // frame, so it is not supported
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t
  zwave_controller_transport_abort_send_data(zwave_tx_session_id_t session_id)
{
  // The transports are arranged in an odered set according to priority,
  // to abort, we want to call them in a Reverse order (lower layers first)
  for (auto rit = transports.rbegin(); rit != transports.rend(); rit++) {
    if (rit->abort_send_data != nullptr) {
      sl_status_t abort_status = rit->abort_send_data(session_id);
      // First transport that had some sessions ongoing will make the rest abort.
      if (abort_status == SL_STATUS_OK) {
        return SL_STATUS_OK;
      }
    }
  }
  return SL_STATUS_NOT_FOUND;
}