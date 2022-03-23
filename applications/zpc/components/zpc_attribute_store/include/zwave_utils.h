/**
 * @file zwave_utils.h
 * @brief Various Z-Wave related utilities.
 *
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 */

#ifndef ZWAVE_UTILS_H
#define ZWAVE_UTILS_H

// Generic includes
#include <stdint.h>
#include <stdbool.h>

// Interfaces
#include "sl_status.h"
#include "zwave_generic_types.h"
#include "zwave_node_id_definitions.h"
#include "zwave_controller_keyset.h"
#include "zwave_keyset_definitions.h"
#include "zwave_command_class_version_types.h"
#include "zwave_command_class_wake_up_types.h"

/**
 * @ingroup zpc_utils
 * @{
 */

/**
 * @addtogroup zwave_utils Z-Wave Utilities
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns the operating mode of a node
 *
 * Refer to @ref zwave_nwk_spec for Operating Modes.
 * It can be Always listening (AL), Frequently listening (FL) or
 * Non-Listening (NL).
 *
 * It will search for the node in the Attribute store and deduct its operating
 * mode from the ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING
 * and ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL
 * attributes stored under the NodeID.
 *
 * @param node_id The NodeID for which the operating mode is requested
 * @returns zwave_operating_mode_t value.
 */
zwave_operating_mode_t zwave_get_operating_mode(zwave_node_id_t node_id);

/**
 * @brief Returns the protocol that the node is running in this network
 *
 * This function will return the protocol that has been used for including
 * a node in our network. After inclusion, nodes run only 1 protocol.
 *
 * It will search for the node in the Attribute store and deduct its protocol
 * from the ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL
 * attribute stored under the NodeID.
 *
 * @param node_id The NodeID for which the operating protocol is requested
 * @returns zwave_protocol_t value.
 */
zwave_protocol_t zwave_get_inclusion_protocol(zwave_node_id_t node_id);

/**
 * @brief Returns the name of a protocol value
 *
 * @param protocol zwave_protocol_t value to print out
 *
 * @returns A string pointer to the name of the Z-Wave Protocol.
 */
const char *zwave_get_protocol_name(zwave_protocol_t protocol);

/**
 * @brief Stores a protocol value for a NodeID in the attribute store.
 *
 * @param node_id                 The NodeID for which the operating protocol
 *                                must be saved in the attribute store
 * @param inclusion_protocol      zwave_protocol_t value to save
 * @returns SL_STATUS_OK on success, any other code if not successful.
 */
sl_status_t zwave_store_inclusion_protocol(zwave_node_id_t node_id,
                                           zwave_protocol_t inclusion_protocol);

/**
 * @brief Get the node granted keys object
 *
 * @param node_id
 * @param keys
 */
sl_status_t zwave_get_node_granted_keys(zwave_node_id_t node_id,
                                        zwave_keyset_t *keys);

/**
 * @brief Verify whether a node/endpoint supports a Command Class
 * using the attribute Store.
 *
 * @param command_class The Command Class identifier to look for.
 * @param node_id       The NodeID to verify for support.
 * @param endpoint_id   The Endpoint to verify for support.

 * @returns
 * - true if the node/endpoint supports the Command Class
 * - false if the node/endpoint does not support the Command Class
 */
bool zwave_node_supports_command_class(zwave_command_class_t command_class,
                                       zwave_node_id_t node_id,
                                       zwave_endpoint_id_t endpoint_id);

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
  zwave_node_get_command_class_version(zwave_command_class_t command_class,
                                       zwave_node_id_t node_id,
                                       zwave_endpoint_id_t endpoint_id);

/**
 * @brief Verify if we registered a node as supporting S2.
 *
 * @param node_id             The NodeID that supports S2.
 * @returns True if the node has been registered to support S2, false otherwise
 */
bool zwave_security_validation_is_node_s2_capable(zwave_node_id_t node_id);

/**
 * @brief Sets a node as S2 capable, meaning that we know it supports S2,
 *        and it will forever do.
 *
 * @param node_id             The NodeID that supports S2.
 */
sl_status_t
  zwave_security_validation_set_node_as_s2_capable(zwave_node_id_t node_id);

/**
 * @brief Navigates the attribute store to find out what is the Wake Up Interval
 *  of a node.
 *
 * It will search under all endpoints, (despite that the setting should be under
 * endpoint 0) and return the first found value.
 *
 * @param node_id       The Z-Wave NodeID for which we want to find out the
 *                      Wake Up Interval.
 * @returns The value of the Wake Up interval. 0 is returned if there is no
 *          Wake up interval of if it is unknown.
 */
wake_up_interval_t zwave_get_wake_up_interval(zwave_node_id_t node_id);

/** @} (end addtogroup zwave_utils) */
/** @} (end addtogroup zpc_utils) */
#ifdef __cplusplus
}
#endif

#endif /* ZWAVE_UTILS_H */
