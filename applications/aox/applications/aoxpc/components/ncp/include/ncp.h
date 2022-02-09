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
 * @file
 * @brief Bluetooth Network Co-Processor.
 */

#ifndef NCP_H
#define NCP_H

#include "sl_status.h"
#include "sl_bt_api.h"

/**
 * @brief Register Bluetooth stack event handler callback.
 *
 * @param[in] bt_on_evt Event handler function pointer.
 */
void ncp_set_cb(void(*bt_on_event)(sl_bt_msg_t *evt));

/**
 * @brief Resets the NCP target. Use this instead of sl_bt_system_reset.
 */
void ncp_reset(void);

#endif // NCP_H
