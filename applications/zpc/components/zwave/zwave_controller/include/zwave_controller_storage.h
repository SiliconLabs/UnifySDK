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
 * @defgroup zwave_controller_storage Z-Wave Controller Storage
 * @ingroup zwave_controller_component
 * @brief Callbacks registration system that enables an application to provide
 * some functions performing look-ups in the persistent storage system.
 *
 * @{
 */

#ifndef ZWAVE_CONTROLLER_STORAGE_H
#define ZWAVE_CONTROLLER_STORAGE_H

// Generic includes
#include <stdbool.h>

// Z-Wave definitions
#include "zwave_node_id_definitions.h"
#include "zwave_controller_types.h"
#include "zwave_keyset_definitions.h"
#include "zwave_generic_types.h"
#include "zwave_command_class_version_types.h"

// Unify includes
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  /// This function is used to set node as S2 capable
  sl_status_t (*set_node_as_s2_capable)(zwave_node_id_t node_id);
  /// This function is used to verfy if a nod is S2 capable or not
  bool (*is_node_S2_capable)(zwave_node_id_t node_id);
  /// This function is used to get the node granted key
  sl_status_t (*get_node_granted_keys)(zwave_node_id_t node_id,
                                       zwave_keyset_t *keys);
  /// This function is used to get the current zwave protocol running on a node
  zwave_protocol_t (*get_inclusion_protocol)(zwave_node_id_t node_id);
  /// This function is used the cc version of a node
  zwave_cc_version_t (*zwave_controller_storage_cc_version)(
    zwave_command_class_t command_class,
    zwave_node_id_t node_id,
    zwave_endpoint_id_t endpoint_id);
} zwave_controller_storage_callback_t;


/**
 * @brief zwave_controller_storage callbacks register API
 *
 * @param callbacks \ref callbacks pointer for the zwave_controller component.
 *
 * @returns  \ref SL_STATUS_OK if the callback pointer has been registered.
 * @returns  \ref SL_STATUS_FAIL if the callback pointer is invalide
 *
 */
sl_status_t zwave_controller_storage_callback_register(
  const zwave_controller_storage_callback_t *callbacks);

/**
 * @brief Set the Z-Wave node as s2 capable when it supports S2.
 *
 * @param node_id             The NodeID that supports S2.
 */
sl_status_t
  zwave_controller_storage_as_set_node_s2_capable(zwave_node_id_t node_id);

/**
 * @brief Verify if the registered node is supporting S2.
 *
 * @param node_id             The NodeID that supports S2.
 * @returns True if the node has been registered to support S2, false otherwise
 */
bool zwave_controller_storage_is_node_s2_capable(zwave_node_id_t node_id);

/**
 * @brief Return the version of a Command Class implemented by a node.
 *
 * @param command_class The Command Class identifier for which the version
 *                      is requested.
 * @param node_id       The NodeID for which the version is requested.
 * @param endpoint_id   The Endpoint for which the version is requested.

 * @returns the version number of the Command Class. 0 if it is
 *          neither supported nor controlled.
 */
zwave_cc_version_t
  zwave_controller_storage_get_cc_version(zwave_command_class_t command_class,
                                          zwave_node_id_t node_id,
                                          zwave_endpoint_id_t endpoint_id);
/**
 * @brief Return the granted keys for a node.
 *
 * @param [in] node_id Z-Wave NodeID of the node
 * @param [out] keys Pointer where to write granted keys.
 */
sl_status_t
  zwave_controller_storage_get_node_granted_keys(zwave_node_id_t node_id,
                                                 zwave_keyset_t *keys);

/**
 * @brief Returns the protocol that the node is running in this network
 *
 * This function will return the protocol that has been used for including
 * a node in our network.
 *
 * @param node_id The NodeID for which the operating protocol is requested
 */
zwave_protocol_t
  zwave_controller_storage_inclusion_protocol(zwave_node_id_t node_id);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_CONTROLLER_STORAGE_H
/** @} end zwave_controller_storage */
