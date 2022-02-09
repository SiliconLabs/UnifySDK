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
 * @defgroup zwave_multi_channel_transport Multi Channel Transport
 * @ingroup zwave_transports
 * @brief Transport for Multi Channel
 *
 * @{
 */

#ifndef ZWAVE_MULTI_CHANNEL_TRANSPORT_H
#define ZWAVE_MULTI_CHANNEL_TRANSPORT_H

#include "sl_status.h"
#include "zwave_node_id_definitions.h"

// We allow to encapsulate the maximum minus our encapsulation command overhead
#define MULTI_CHANNEL_ENCAPSULATION_OVERHEAD 4
#define MULTI_CHANNEL_ENCAPSULATED_COMMAND_MAXIMUM_SIZE \
  (ZWAVE_MAX_FRAME_SIZE - MULTI_CHANNEL_ENCAPSULATION_OVERHEAD)

// Frame parsing define
#define BIT_ADDRESS 0x80

#define MULTI_CHANNEL_VERSION MULTI_CHANNEL_VERSION_V4
#define ENDPOINT_MASK         0x7F

typedef struct zwave_multi_channel_encapsulation_frame {
  uint8_t command_class; /* The command class */
  uint8_t command;       /* The command */
  uint8_t source_endpoint;
  uint8_t destination_endpoint;
  uint8_t encapsulated_command
    [MULTI_CHANNEL_ENCAPSULATED_COMMAND_MAXIMUM_SIZE]; /* Encapsulated command */
} zwave_multi_channel_encapsulation_frame_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Multi Channel Transport
 * *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_multi_channel_transport_init(void);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_MULTI_CHANNEL_TRANSPORT_H
/** @} end zwave_multi_channel_transport */
