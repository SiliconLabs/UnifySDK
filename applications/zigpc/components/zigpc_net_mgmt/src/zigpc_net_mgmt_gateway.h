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

#ifndef ZIGPC_NET_MGMT_GATEWAY_H
#define ZIGPC_NET_MGMT_GATEWAY_H

/**
 * @brief Setup listeners to receive ZigPC Gateway events.
 *
 * @return sl_status_t SL_STATUS_OK on success,
 * if not there is an error in initializing observables
 */
sl_status_t zigpc_net_mgmt_gateway_init_observer(void);

#endif /* ZIGPC_NET_MGMT_GATEWAY_H */
