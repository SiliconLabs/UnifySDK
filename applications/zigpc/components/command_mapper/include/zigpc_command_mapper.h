/*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file command_mapper.h
 * @defgroup command_mapper ZigPC Command Mapper
 * @ingroup zigpc_components
 * @brief The ZigPC Command Mapper is responsible for translating MQTT DotDot
 * commands to be sent to the Zigbee network.
 *
 * @{
 */

#ifndef ZIGPC_COMMAND_MAPPER_H
#define ZIGPC_COMMAND_MAPPER_H

#include "sl_status.h"

/**
 * @brief zigpc_command_mapper_setup - sets up the parts of the software used 
 * by the command mapper
 *
 * @return the status if the method succeeded and the component was set up
 * correctly
**/
sl_status_t zigpc_command_mapper_setup(void);

/**
 * @brief zigpc_command_mapper_shutdown - graceful shuts down all parts of the
 * command mapper, expected to be called before the program exits
 *
 * @return an integer representing the success of the operation. 0 for success
 * any other number represents a failure
 *
**/
int zigpc_command_mapper_shutdown(void);

#endif  //ZIGPC_COMMAND_MAPPER_H

/** @} end command_mapper */
