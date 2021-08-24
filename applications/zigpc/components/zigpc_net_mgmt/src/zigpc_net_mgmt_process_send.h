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

/**
 * @file zigpc_net_mgmt_process_send.h
 * @defgroup  zigpc_net_mgmt_process_send ZigPC Net Mgmt Process Event API
 * @ingroup zigpc_net_mgmt
 * @brief Contiki Process API to send events to ZigPC Net Mgmt.
 *
 * In order to send events to the ZigPC Net Mgmt process, the following header
 * outlines the process event types and helpers to send an event to the Contiki
 * process.
 *
 * @{
 */

#ifndef ZIGPC_NET_MGMT_PROCESS_SEND_H
#define ZIGPC_NET_MGMT_PROCESS_SEND_H

#include "sl_status.h"

/**
 * @brief ZigPC Network Management Process events
 */
enum zigpc_net_mgmt_process_event {
  ZIGPC_NET_MGMT_EVENT_MIN_VAL = 0,

  /**
   * Send the event to the Network Managment FSM. See
   * zigpc_net_mgmt_process_data_fsm_t for more information.
   */
  ZIGPC_NET_MGMT_EVENT_FSM,

  ZIGPC_NET_MGMT_EVENT_MAX_VAL,
};

/**
 * @brief Helper to send events to Contiki process in ZigPC Net Mgmt by
 * dynamically allocating the event data based on the data_size passed in.
 *
 * @param event event type.
 * @param data associated data.
 * @param data_size size of associated data.
 * @return SL_STATUS_OK if the event is sent to the ZigPC Net Mgmt contiki
 * process, SL_STATUS_NULL_POINTER on invalid arguments passed in, or
 * SL_STATUS_EMPTY if the data_size to send is 0.
 */
sl_status_t zigpc_net_mgmt_process_send_event(
  const enum zigpc_net_mgmt_process_event event,
  const void *data,
  size_t data_size);

#endif  // ZIGPC_NET_MGMT_PROCESS_SEND_H

/** @} end zigpc_net_mgmt_process_send */
