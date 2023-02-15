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
 * @defgroup zwave_controller Z-Wave Controller
 * @ingroup zpc_components
 * @brief The @ref zwave_controller super is a layer providing a set of components
 * that allow to build a Z-Wave Controller application.
 *
 * The @ref zwave_controller super is a layer on top of the zwave_api which
 * implements low level Z-Wave functionality which could be argued should have
 * been implemented in the Z-Wave controller firmware. The zwave_controller
 * super component has not need for a persistent storage. It performs transport
 * encapsulations and has a FSM for network operations. All in all it simplifies
 * the more low-level zwave_api
 */

/**
 * @defgroup zwave_controller_component Z-Wave Controller Component
 * @ingroup zwave_controller
 * @brief Z-Wave Controller component, glueing all the subcomponent of the
 * @ref zwave_controller super component
 *
 * @{
 */
#ifndef ZWAVE_CONTROLLER_H
#define ZWAVE_CONTROLLER_H

#include "zwave_network_management.h"
#include "zwave_controller_callbacks.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set the application part of the node information frame (NIF).
 *
 * This sets the application part of the node information frame. The full
 * node information frame may include additional command classes such as
 * - COMMAND_CLASS_SECURITY
 * - COMMAND_CLASS_SECURITY2
 * - COMMAND_CLASS_TRANSPORT_SERVICE
 * - COMMAND_CLASS_CRC16
 * which will added to the actual NIF.
 *
 * @param command_classes List of command classes
 * @param command_classes_length Length of command class list
 */
void zwave_controller_set_application_nif(const uint8_t *command_classes,
                                          uint8_t command_classes_length);

/**
 * @brief Set the secure node information.
 *
 * This sets the secure node information command class list. This list is
 * the contents of the SECURE_COMMANDS_SUPPORTED frames for S0 and S2.
 *
 * @param command_classes List of command classes
 * @param command_classes_length Length of command class list
 */
void zwave_controller_set_secure_application_nif(
  const uint8_t *command_classes, uint8_t command_classes_length);

/**
 * @brief Retrieve the DSK of the controller.
 *
 * @param[out] dsk The DSK
 */
void zwave_controller_get_dsk(zwave_dsk_t *dsk);

/**
 * @brief Instructs the Z-Wave Controller to initiate a Set Default / Reset
 * operation
 *
 */
void zwave_controller_reset();

/**
 * @brief Tells if we are currently reseting the ZPC.
 *
 * @returns true if we are in the middle of a reset operation, false otherwise.
 */
bool zwave_controller_is_reset_ongoing();

#ifdef __cplusplus
}
#endif
/** @} end zwave_controller */

#endif  //ZWAVE_CONTROLLER_H
