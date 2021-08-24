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
 * @defgroup zwave_tx_intercept Z-Wave TX Intercept
 * @ingroup zwave_tx
 * @brief Intercept the Z-Wave Frame that can not be sent using zwapi_send_data
 *        command and send the frame using other zwapi transport commands i.e.,
 *        zwapi_request_node_info.
 *
 *
 * @{
 */

#ifndef ZWAVE_TX_INTERCEPT_PAYLOAD_H
#define ZWAVE_TX_INTERCEPT_PAYLOAD_H

#include "zwave_tx_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Send a frame using zwapi tranport APIs and retrun the status.
 *
 * @param data zwave_tx_queue element that contains the payload data.
 *
 * @return SL_STATUS_NOT_SUPPORTED if the frame shall be sent using
 * zwapi_send_data.
 * @return status of the sent frame if the frame is sent using other zwapi
 * tranport APIs.
 */
sl_status_t zwave_tx_intercept_payload(zwave_tx_queue_element_t const &data);

/**
 * @brief Check if an NIF have been sent.
 *
 * @param node_id
 * @return true if awaiting NIF.
 * @return false if not awaiting NIF.
 */
bool zwave_tx_intercept_waiting_for_nif();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_TX_INTERCEPT_PAYLOAD_H
/** @} end zwave_tx_intercept_payload */
