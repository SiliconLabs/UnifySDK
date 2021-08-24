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

/**
 * @file zigpc_gateway_process_send.h
 * @defgroup  zigpc_gateway_process_send ZigPC Gateway Process Event API
 * @ingroup zigpc_gateway
 * @brief Observer API for getting events from zigpc_gateway
 *
 * In order to send events to the ZigPC Gateway process, the following header
 * outlines the process event types and helpers to send an event to the Contiki
 * process.
 *
 * @{
 */

#ifndef ZIGPC_GATEWAY_PROCESS_SEND_H
#define ZIGPC_GATEWAY_PROCESS_SEND_H

#include "sl_status.h"

enum zigpc_gateway_process_event {
  ZIGPC_GW_EVENT_MIN_VAL = 0,

  /**
   * Call one of the supported Z3Gateway calls. See zigpc_gateway_dispatch_type
   * for more information.
   */
  ZIGPC_GW_EVENT_DISPATCH,

  /**
   * Dispatch a ZCL Frame via the Z3Gateway API.
   */
  ZIGPC_GW_EVENT_ZCL_FRAME,

  /**
   * Dispatch a ZCL Configure Report message via the Z3Gateway API.
   */
  ZIGPC_GW_EVENT_INIT_REPORTS,

  /**
   * Dispatch a binding request via Z3Gateway API
   */
  ZIGPC_GW_EVENT_CONFIG_BINDING,

  /**
   * Dispatch a file (through it's path) to the OTA server plugin
   */
  ZIGPC_GW_EVENT_OTA_FILE,

  ZIGPC_GW_EVENT_MAX_VAL,
};

/**
 * @brief Helper to send events to Contiki process in ZigPC Gateway by
 * dynamically allocating the event data based on the data_size passed in.
 *
 * @param event event type.
 * @param data associated data.
 * @param data_size size of associated data.
 * @return SL_STATUS_OK if the event is sent to the ZigPC Gateway contiki
 * process, SL_STATUS_NULL_POINTER on invalid arguments passed in, or
 * SL_STATUS_EMPTY if the data_size to send is 0.
 */
sl_status_t
  zigpc_gateway_process_send_event(const enum zigpc_gateway_process_event event,
                                   const void *data,
                                   size_t data_size);

#endif  // ZIGPC_GATEWAY_PROCESS_SEND_H

/** @} end zigpc_gateway_process_send */
