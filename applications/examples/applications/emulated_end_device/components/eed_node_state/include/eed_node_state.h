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

/**
 * @defgroup eed_node_state EED Node state handler
 * @brief Handles the node state topic for simulated MQTT Devices
 *
 * @{
 */

#ifndef EED_NODE_STATE_H
#define EED_NODE_STATE_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief initializes the EED Node state
 *
 * @returns sl_status_t value
*/
sl_status_t eed_node_state_init();

#ifdef __cplusplus
}
#endif

#endif  //EED_NODE_STATE_H
/** @} end eed_node_state */
