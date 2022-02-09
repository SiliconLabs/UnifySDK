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

#ifndef ZWAVE_TX_FOLLOW_UPS_H
#define ZWAVE_TX_FOLLOW_UPS_H

#include "zwave_tx.h"

/**
 * @defgroup zwave_tx_follow_ups Z-Wave TX Follow-ups
 * @ingroup zwave_tx
 * @brief Z-Wave TX component managing singlecast Follow Up frames
 *
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enqueues follow-up frames for a given multicast frame.
 *
 * This function should be called after the Multicast frame has been successfully
 * transmitted.
 *
 * @param multicast_frame_tx_id the Tx session ID of the multicast frame.
 *
 * @returns SL_STATUS_OK if frames were enqueued. SL_STATUS_FAIL in case of
 *          error and the follows up were not queued correctly.
 * queued
 */
sl_status_t
  zwave_tx_enqueue_follow_ups(zwave_tx_session_id_t multicast_frame_tx_id);

#ifdef __cplusplus
}
#endif
/** @} end of zwave_tx_follow_ups */

#endif  // ZWAVE_TX_FOLLOW_UPS_H
