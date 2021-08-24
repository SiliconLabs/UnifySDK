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
 * @file zigpc_smartstart_fixt.h
 * @defgroup zigpc_smartstart ZigPC SmartStart
 * @ingroup zigpc_components
 * @brief ZigPC SmartStart Functionality
 *
 * The Zigbee Protocol Controller uses the shared SmartStart component to
 * listen for Unify Smartstart List inclusions and configure the gateway to
 * start including Zigbee nodes using SmartStart entry information. Currently
 * this component accepts the following information from the SmartStart list
 * entries:
 * - Z3 Install Code information encoded in known SmartStart DSK format
 *
 * Once a compatible SmartStart entry is found, the entry information is used
 * to allow the corresponding Zigbee node to join the Zigbee network. When a
 * Zigbee node has successfully joined the network, the UNID field of the
 * corresponding SmartStart List entry will be updated to inform the identifier
 * to be used to control/observe the added device.
 *
 * If a device is removed from the Zigbee network and the SmartStart list
 * contains an entry corresponding to the device UNID, the UNID field will be
 * cleared from the SmartStart entry.
 *
 * See the SmartStart functionality in the Unify spec for more details.
 *
 * @{
 */

#ifndef ZIGPC_SMARTSTART_FIXT_H
#define ZIGPC_SMARTSTART_FIXT_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Set up function for the ZigPC SmartStart Component.
 *
 * @return SL_STATUS_OK on success, any other value on error.
 */
sl_status_t zigpc_smartstart_setup(void);

/** @brief Shut down function for the ZigPC SmartStart Component.
 *
 * @return Return 0 on success.
 */
int zigpc_smartstart_shutdown(void);

#ifdef __cplusplus
}
#endif

/** @} end zigpc_smartstart */

#endif /* ZIGPC_SMARTSTART_FIXT_H */
