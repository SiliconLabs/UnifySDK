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

/**
 * @defgroup multicast_follow_up_transport Z-Wave Multicast follow-ups Transport
 * @ingroup zwave_transports
 * @brief Transport for automatic sending of Singlecast follow-ups after a
 *        Multicast frame
 *
 * @{
 */

#ifndef MULTICAST_FOLLOW_UPS_TRANSPORT_H
#define MULTICAST_FOLLOW_UPS_TRANSPORT_H

// Unify includes
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Multicast Follow-Ups Transport
 * *
 * @returns SL_STATUS_OK if successful, SL_STATUS_FAIL otherwise
 */
sl_status_t multicast_follow_ups_transport_init(void);

#ifdef __cplusplus
}
#endif

#endif  //MULTICAST_FOLLOW_UPS_TRANSPORT_H
/** @} end multicast_follow_up_transport */
