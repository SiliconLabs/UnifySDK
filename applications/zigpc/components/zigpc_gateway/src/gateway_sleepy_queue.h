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
/**
 * @file zigpc_sleepy_queue.h
 * @defgroup zigpc_sleepy_queue ZigPC Gateway Component
 * @ingroup zigpc_gateway
 * @brief The interface for interacting with sleepy messages 
 *
 * @{
 */

#ifndef ZIGPC_GATEWAY_SLEEPY_QUEUE_H
#define ZIGPC_GATEWAY_SLEEPY_QUEUE_H

#include "zigpc_common_zigbee.h"
#include "zigpc_gateway_int.h"

#ifdef __cplusplus
extern "C" {
#endif

//Max retries when attempting to send a sleepy messages
const unsigned int MAX_SLEEPY_RETRIES = 64;

typedef struct {
  zigbee_eui64_t eui64;
  zigbee_endpoint_id_t endpoint_id;
  zcl_cluster_id_t cluster_id;
  zcl_frame_t frame;
} gateway_frame_t;

/**
 * @brief zigpc_gateway_store_sleepy_frame
 * Stores a new ZCL frame destined to a sleepy end device
 *
 * @param frame - the ZCL frame, including EUI64 and endpoint info
 *
 * @return SL_STATUS_OK if the frame could be stored
 */
sl_status_t zigpc_gateway_store_sleepy_frame(gateway_frame_t frame);

/**
 * @brief zigpc_gateway_retrieve_sleepy_frame
 * Retrieve the next frame destined for a sleepy device
 * Frames are retrieved in the order they are stored (FIFO)
 *
 * @param eui64 - the eui64 of the sleepy end device
 * @param frame - the destination where to copy the ZCL frame
 *
 * @return the status of the operation. Returns SL_STATUS_OK
 * if the next frame could be retrieved and copied. Returns
 * SL_STATUS_EMPTY when no more frames are available. Returns
 * SL_STATUS_FAIL if something went wrong.
 */
sl_status_t zigpc_gateway_retrieve_sleepy_frame(const zigbee_eui64_t eui64,
                                                gateway_frame_t *frame);

/**
 * @brief zigpc_gateway_retrieve_num_stored
 * Retrieves the number of stored messages for a given sleepy
 * device
 *
 * @param eui64 - the eui64 of the device to check
 *
 * @return the number of frames available. Returns 0 if no
 * frames or stored or if the EUI64 could not be found
 */
unsigned int zigpc_gateway_retrieve_num_stored(const zigbee_eui64_t eui64);

#ifdef __cplusplus
}
#endif

#endif  //ZIGPC_GATEWAY_SLEEPY_QUEUE_H

/** @} end zigpc_sleepy_queue */
