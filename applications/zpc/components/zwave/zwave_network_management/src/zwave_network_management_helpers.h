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
 * @defgroup zwave_network_management_helpers Z-Wave Network Management helpers
 * @ingroup zwave_network_management
 * @brief Internal helper functions used with Network Management
 *
 * Internal helper functions used with Network Management
 *
 * @{
 */

#ifndef ZWAVE_NETWORK_MANAGEMENT_HELPERS_H
#define ZWAVE_NETWORK_MANAGEMENT_HELPERS_H

#include <stdbool.h>
#include "zwave_controller_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Resets all the variables sets in the nms struct.
 *
 * Use this when returning to IDLE or when swapping between Network Management
 * Operations not to read data set by a previous operation.
 *
 */
void reset_nms_last_operation_data();

/**
 * @brief Checks if we are alone in our network and returns true if so.
 *
 * @returns true if we are alone in our networks
 * @returns false if at least another NodeID is part of the network
 */
bool we_are_alone_in_our_network();

/**
 * @brief Checks if all the conditions are fulfilled to start a new
 *        network management operation. (Set default operation excluded)
 *
 * We will check that we are not already doing something and that for example
 * the Z-Wave Controller is not resetting.
 *
 * @returns true if we can start a new operation, false otherwise
 */
bool network_management_is_ready_for_a_new_operation();

/**
 * @brief Asks the Z-Wave API to tell us our current HomeID and NodeID
 *
 * NOTE: Should be used only when necessary, as communicating with the Z-Wave API
 * is slow and blocking.
 */
void network_management_refresh_cached_ids();

/**
 * @brief Asks the Z-Wave API about the list of NodeIDs in our network
 *
 * NOTE: Should be used only when necessary, as communicating with the Z-Wave API
 * is slow and blocking.
 */
void network_management_refresh_cached_node_list();

/**
 * @brief Asks the Z-Wave API to tell us what are the controller's capabilities
 * in this network
 *
 * NOTE: Should be used only when necessary, as communicating with the Z-Wave API
 * is slow and blocking.
 */
void network_management_refresh_controller_capabilities_bitmask();

/**
 * @brief Quick helper function that will refresh all network information
 * (HomeID, NodeID, Node list, controller capabilities)
 *
 * NOTE: Should be used only when necessary, as communicating with the Z-Wave API
 * is slow and blocking.
 */
void network_management_refresh_network_information();

/**
 * @brief Makes sure that the Z-Wave API is not performing any network
 * management operation
 */
void network_management_stop_ongoing_operations();

/**
 * @brief Verifies if a Command Class is part of of NIF's command class list.
 *
 * @param nif             Pointer to a Z-Wave Command Class array
 * @param nif_length      The number of elements in the NIF pointer
 * @param command_class   The command class to search for.
 * @returns true if the Command Class is in the NIF, false otherwise.
 */
bool is_command_class_in_nif(const zwave_command_class_t *nif,
                             uint8_t nif_length,
                             zwave_command_class_t command_class);

/**
 * @brief Looks at the state of network S2 bootstrapping after being SmartStart
 * included and decides if we should reset and leave the network
 * @returns true if we should reset and leave the network, false if we are happy
 */
bool we_should_self_destruct();

/**
 * @brief Attempts to take the SUC/SIS Role, if there is no SUC in the network
 */
void network_management_take_sis_role_if_no_suc_in_network();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_NETWORK_MANAGEMENT_HELPERS_H
/** @} end zwave_network_management_helpers */
