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
 * @file zigpc_protctrl_nwmgmt_write.h
 * @defgroup zigpc_protctrl_nwmgmt_write ZigPC NetworkManagement/Write Handler
 * @ingroup zigpc_protctrl_process
 * @brief The ZigPC Protocol Controller NetworkManagement/Write Handler is
 * responsible for managing network requests that come in (such as: add a node,
 * remove a node, etc.)
 *
 * The following topic is managed by the NetworkManagement/Write handler:
 *  - SUBCRIBE: ucl/by-unid/PC_UNID/ProtocolController/NetworkManagement/Write
 *
 *
 * @{
 */

#ifndef ZIGPC_PROTCTRL_NWMGMT_WRITE_H
#define ZIGPC_PROTCTRL_NWMGMT_WRITE_H

#include "sl_status.h"

#include "zigpc_common_zigbee.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize and subscribe to the MQTT topic for handling network
 * management writes.
 *
 * @param pc_unid_wo_prefix Character array storing the UNID of the protocol
 * controller without the prefix.
 * @return sl_status_t always returns SL_STATUS_OK
 */
sl_status_t zigpc_protctrl_nwmgmt_write_init(char *pc_unid_wo_prefix);

/**
 * @brief Handler for when the ZigPC Protctrl process receives a new message
 * from the network management write subscription. This function will parse
 * the incoming message (assuming a JSON payload) and send the extracted
 * state and it's associated state parameters to the following callback:
 * zigpc_protctrl_on_write_update_callback().
 *
 * NOTE: The data passed in is a reference to a c++ string that will be static
 * casted.
 *
 * @param update_message  Reference to network management write message
 * @return sl_status_t    SL_STATUS_OK if parsed and resulting action executed,
 * SL_STATUS_OBJECT_READ if the message fails parsing.
 */
sl_status_t zigpc_protctrl_nwmgmt_write_update(char *update_message);

#ifdef __cplusplus
}
#endif

#endif /* ZIGPC_PROTCTRL_NWMGMT_WRITE_H */

/** @} end zigpc_protctrl_nwmgmt_write */
