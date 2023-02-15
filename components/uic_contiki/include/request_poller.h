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
#ifndef REQUEST_POLLER_H
#define REQUEST_POLLER_H
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Start rust process. This process drives the task executor in rust
 */
sl_status_t initialize_request_poller_process();

/**
 * @brief register a handler thats called when a poll is requested.
 * setting it a second time will overwrite the previous time.
 */
void register_poll_handler(void (*handler)());

/**
 * @brief request a poll
 */
void request_poller_poll_request();

#ifdef __cplusplus
}
#endif
#endif
