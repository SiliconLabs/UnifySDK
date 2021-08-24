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

#ifndef Z3GATEWAY_COMMON_H
#define Z3GATEWAY_COMMON_H

#include "app/framework/include/af-types.h"
#include "z3gateway.h"

/**
 * @brief Size of buffer used to send ZCL frames
 */
#define ZCL_BUFFER_SIZE 255

/**
 * @brief Default endpoint that is sent with every APS message to the Zigbee
 * network.
 */
#define Z3GATEWAY_FRAME_SRC_ENDPOINT 1

#ifndef UNUSED_VAR

/**
 * @brief Useful macro for avoiding compiler warnings related to unused
 * function arguments or unused variables.
 */
#define UNUSED_VAR(x) (void)(x)
#endif

#define Z3GATEWAY_CALLBACK_EXISTS(callback_struct, callback_event) \
  ((callback_struct != NULL) && (callback_struct->callback_event != NULL))

/**
 * @brief Internal state
 *
 */
struct z3gatewayState {
  struct z3gatewayCallbacks *callbacks;
};

struct z3gatewayState z3gwState;

#endif  // Z3GATEWAY_COMMON_H
