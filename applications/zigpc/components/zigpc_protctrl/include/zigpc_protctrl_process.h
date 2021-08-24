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
 * @file zigpc_protctrl_process.h
 * @defgroup zigpc_protctrl_process ZigPC ProtocolController Monitor
 * @ingroup zigpc_components
 * @brief The ZigPC Protocol Controller monitor is responsible for managing the
 * following MQTT topics:
 *  - ucl/by-unid/PC_UNID/ProtocolController/NetworkManagement
 *  - ucl/by-unid/PC_UNID/ProtocolController/Diagnostics (Not supported yet)
 *  - ucl/by-unid/PC_UNID/ProtocolController/Configuration (Not supported yet)
 *
 * The NetworkManagement topic is specified in the Unify Spec section 3.5 "Network
 * Management". This component currently handles the following functionality:
 *
 * - NetworkManagement State publishing: This functionality update the MQTT topic
 *   when the Network Management FSM changes its state. For example, if the network
 *   management FSM is in the idle state, the MQTT topic
 *   ...ProtocolController/NetworkManagement will be published with {state: "idle"}
 *   as well as other JSON fields that notify of next possible actions that the FSM
 *   can take
 *
 * @{
 */

#ifndef ZIGPC_PROTCTRL_PROCESS_H
#define ZIGPC_PROTCTRL_PROCESS_H

#include "sl_status.h"

/** @brief Setup fixture for ZigPC ProtocolController Monitor
 *
 * @return SL_SATUS_OK if initialization is successful, error otherwise
 */
sl_status_t zigpc_protctrl_process_setup(void);

/** @brief Shutdown fixture for ZigPC ProtocolController Monitor
 *
 * @return Return Always return 0.
 */
int zigpc_protctrl_process_shutdown(void);

#endif /* ZIGPC_PROTCTRL_PROCESS_H */

/** @} end zigpc_protctrl_process */
