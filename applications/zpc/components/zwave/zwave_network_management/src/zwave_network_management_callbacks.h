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
 * @defgroup zwave_network_management_callbacks Callbacks used by the Z-Wave Network management
 * @ingroup zwave_network_management
 * @brief Z-Wave Network Management callback functions
 *
 * Callback functions used by Z-Wave Network Management for external modules,
 * such as the Z-Wave API or Z-Wave Controller
 * @{
 */

#ifndef ZWAVE_NETWORK_MANAGEMENT_CALLBACKS_H
#define ZWAVE_NETWORK_MANAGEMENT_CALLBACKS_H

#include "zwapi_protocol_controller.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief callback function for @ref zwapi_set_learn_mode()
 */
void on_learn_mode_callback(LEARN_INFO *learn_mode_info);

/**
 * @brief callback function for @ref zwapi_set_suc_node_id()
 * @param set_suc_status the status of the Set SUC Operation
 */
void on_set_suc_node_id_completed(uint8_t set_suc_status);

/**
 * @brief callback function for @ref zwapi_set_default()
 */
void on_set_default_complete();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_NETWORK_MANAGEMENT_CALLBACKS_H
/** @} end zwave_network_management_callbacks */
