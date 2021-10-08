/*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/**
 * @file zigpc_controller.h
 * @defgroup zigpc_controller Centralized Logic for ZigPC Actions
 * @ingroup zigpc_components
 * @brief This component serves as a mediator to invoke behaviors for Zigbee devices
 * based on its network state (device announced, device interviewed, etc.)
 *
 *
 * @{
 *
 **/

#ifndef ZIGPC_CONTROLLER_H
#define ZIGPC_CONTROLLER_H

// Shared UIC includes
#include <sl_status.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Setup routines as a part of ZigPC Controller
 *
 * @return the status if the method succeeded and the component was set up
 * correctly.
**/
sl_status_t zigpc_ctrl_fixt_setup(void);

/**
 * @brief Graceful shutdown of ZigPC Controller modules, expected to be
 * called before the program exits.
 *
 * @return an integer representing the success of the operation. 0 for success
 * any other number represents a failure.
 *
**/
int zigpc_ctrl_fixt_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif  //ZIGPC_CONTROLLER_H
/**@} end zigpc_controller **/
