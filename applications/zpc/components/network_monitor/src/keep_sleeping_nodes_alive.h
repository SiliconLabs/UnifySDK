/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "attribute_store.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup network_monitor_keep_alive Network Monitor Keep alive
 * @ingroup network_monitor
 * @brief This module implements a safety mechanism for wake up nodes. It
 * attempts to keep these nodes awake while an interview and/or inclusion is
 * ongoing. Wake up nodes go into sleep mode after a interval of roughly 10
 * seconds. This means that if the ZPC is busy handling other nodes we might not
 * be in time to talk to the wake up node before it goes into sleep mode.
 *
 * By sending NOP frames to this node we try to prevent it from going to sleep.
 * When the interview of this node is done we stop sending NOP frames.
 *
 * If the device did not responded for 15 seconds, we give up trying to send NOP
 * frames
 */
void initialize_keep_alive_for_sleeping_nodes();

#ifdef __cplusplus
}
#endif