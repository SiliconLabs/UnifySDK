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
 * @defgroup ZWAVE_API_DEMO Z-Wave API Demo
 * @ingroup zpc_applications
 * @brief Small application showcasing how to use the Z-Wave API.
 *
 * This small application connects to the Z-Wave Module, initializes it and
 * offers the possibility for a user to enter commands that will invoke
 * well-known Z-Wave API calls, such as add nodes, remove nodes, etc.
 */

/**
 * @file zwave_api_demo.h
 * @ingroup ZWAVE_API_DEMO
 * @brief Main include file for the Z-Wave API demo
 */

#ifndef ZWAVE_API_DEMO_H
#define ZWAVE_API_DEMO_H

#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include "zwave_api.h"
#include "command_line_input.h"
#include "zwave_api_demo_callbacks.h"
#include "zwave_api_demo_commands.h"
#include "sl_status.h"
#include "sl_log.h"

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder for future string functions.

#ifdef __cplusplus
}
#endif

#endif  // ZWAVE_API_DEMO_H
