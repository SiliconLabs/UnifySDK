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

#ifndef ZIGPC_PROTCTRL_PROCESS_SEND_H
#define ZIGPC_PROTCTRL_PROCESS_SEND_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Protocol Controller events
 *
 */
enum zigpc_protctrl_process_event {
  ZIGPC_PROTCTRL_EVENT_MIN_VAL = 0,
  ZIGPC_PROTCTRL_EVENT_NET_INIT,
  ZIGPC_PROTCTRL_EVENT_NET_STATE_UPDATE,
  ZIGPC_PROTCTRL_EVENT_WRITE_MESSAGE,
  ZIGPC_PROTCTRL_EVENT_MAX_VAL,
};

/**
 * @brief Helper to send Protocol Controller Monitor process
 * an event with associated data
 *
 * @param event
 * @param data
 */
void zigpc_protctrl_process_send_event(
  const enum zigpc_protctrl_process_event event, void *data);

#ifdef __cplusplus
}
#endif

#endif /* ZIGPC_PROTCTRL_PROCESS_SEND_H */
