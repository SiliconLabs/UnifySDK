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
 * @defgroup zwave_command_class_association_helper Association Command Classes helpers
 * @ingroup command_classes
 * @brief Utils/Helpers for Association Command Classes
 *
 * This module provides some utilities helping with
 * functionalities relating to the Association command classes
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_ASSOCIATION_HELPER_H
#define ZWAVE_COMMAND_CLASS_ASSOCIATION_HELPER_H

#include "attribute_store.h"
#include "zwave_command_class_association_types.h"
#include "zwave_node_id_definitions.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the Attribute store node of a certain type under an
 * association Group ID
 *
 * @param node_id           NodeID to find in the attribute store
 * @param endpoint_id       Endpoint ID to find under the NodeID
 * @param group_id          Group ID to find under the Endpoint ID
 * @param type              Attribute Store node type requested under the
 *
 * @return attribute_store_node_t
 */
attribute_store_node_t get_group_child_node(zwave_node_id_t node_id,
                                            zwave_endpoint_id_t endpoint_id,
                                            association_group_id_t group_id,
                                            attribute_store_type_t type);

/**
 * @brief Finds the Attribute Store Node for the group content
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group identifier
 *
 * @returns attribute_store_node_t for the group content.
 *          ATTRIBUTE_STORE_INVALID_NODE if it was not found.
 */
attribute_store_node_t get_group_content_node(zwave_node_id_t node_id,
                                              zwave_endpoint_id_t endpoint_id,
                                              association_group_id_t group_id);

/**
 * @brief Checks if an association is legal or if we will get in trouble
 * with cert if we establish it :-)
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 * @param association       The Association object (i.e. destination)
 *
 * @returns true if we may establish the association, false otherwise
 */
bool is_association_legal(zwave_node_id_t node_id,
                          zwave_endpoint_id_t endpoint_id,
                          association_group_id_t group_id,
                          association_t association);

/**
 * @brief Returns how many groups are supported by an endpoint
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 *
 * @returns The number of supported association groups for the endpoint.
 *          0 in case of error.
 */
association_group_count_t get_number_of_groups(zwave_node_id_t node_id,
                                               zwave_endpoint_id_t endpoint_id);

/**
 * @brief Checks in the attribute store the maximum number of associations for
 *        a group.
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group identifier
 *
 * @returns 0 in case of error.
 *          Else the maximum number of associations for the group.
 */
association_group_capacity_t
  get_group_capacity(zwave_node_id_t node_id,
                     zwave_endpoint_id_t endpoint_id,
                     association_group_id_t group_id);

/**
 * @brief Adds a desired association in a group in the Attribute Store
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 * @param association       The Association object to add
 *
 * @returns SL_STATUS_OK if accepted. SL_STATUS_FAIL in case of error
 */
sl_status_t add_desired_association(zwave_node_id_t node_id,
                                    zwave_endpoint_id_t endpoint_id,
                                    association_group_id_t group_id,
                                    association_t association);

/**
 * @brief Adds a reported association in a group in the Attribute Store
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 * @param association       The Association object to add
 *
 * @returns SL_STATUS_OK if accepted. SL_STATUS_FAIL in case of error
 */
sl_status_t add_reported_association(zwave_node_id_t node_id,
                                     zwave_endpoint_id_t endpoint_id,
                                     association_group_id_t group_id,
                                     association_t association);

/**
 * @brief Adds a desired association in a group in the Attribute Store
 * and remove an existing association if needed.
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 * @param new_association   The Association object to add
 *
 * @returns SL_STATUS_OK if accepted. SL_STATUS_FAIL in case of error
 */
sl_status_t force_add_desired_association(zwave_node_id_t node_id,
                                          zwave_endpoint_id_t endpoint_id,
                                          association_group_id_t group_id,
                                          association_t new_association);

/**
 * @brief Removes a desired association in a group in the Attribute Store
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 * @param association       The Association object to remove
 *
 * @returns SL_STATUS_OK if accepted.
 *          SL_STATUS_NOT_FOUND if the association was not set
 *          for the destination.
 */
sl_status_t remove_desired_association(zwave_node_id_t node_id,
                                       zwave_endpoint_id_t endpoint_id,
                                       association_group_id_t group_id,
                                       association_t association);

/**
 * @brief Removes a reported association in a group in the Attribute Store
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 * @param association       The Association object to remove
 *
 * @returns SL_STATUS_OK if accepted.
 *          SL_STATUS_NOT_FOUND if the association was not set
 *          for the destination.
 */
sl_status_t remove_reported_association(zwave_node_id_t node_id,
                                        zwave_endpoint_id_t endpoint_id,
                                        association_group_id_t group_id,
                                        association_t association);

/**
 * @brief Removes all desired association in a group in the Attribute Store.
 * It will set the attribute to an ASSOCIATION_MARKER byte.
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 *
 * @returns SL_STATUS_OK if accepted. SL_STATUS_FAIL in case of error
 */
sl_status_t remove_all_desired_associations(zwave_node_id_t node_id,
                                            zwave_endpoint_id_t endpoint_id,
                                            association_group_id_t group_id);

/**
 * @brief Removes all reported association in a group in the Attribute Store.
 * It will set the attribute to an ASSOCIATION_MARKER byte.
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 *
 * @returns SL_STATUS_OK if accepted. SL_STATUS_FAIL in case of error
 */
sl_status_t remove_all_reported_associations(zwave_node_id_t node_id,
                                             zwave_endpoint_id_t endpoint_id,
                                             association_group_id_t group_id);

/**
 * @brief Removes all reported NodeID associations in a group in the
 * Attribute Store.
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 *
 * @returns SL_STATUS_OK if accepted. SL_STATUS_FAIL in case of error
 */
sl_status_t
  remove_all_reported_node_id_associations(zwave_node_id_t node_id,
                                           zwave_endpoint_id_t endpoint_id,
                                           association_group_id_t group_id);

/**
 * @brief Checks if the reported value of an association group is full
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 *
 * @returns True if no more association can be established
 */
bool is_reported_group_full(zwave_node_id_t node_id,
                            zwave_endpoint_id_t endpoint_id,
                            association_group_id_t group_id);

/**
 * @brief Checks if the desired value of an association group is full
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 *
 * @returns True if no more association can be established
 */
bool is_desired_group_full(zwave_node_id_t node_id,
                           zwave_endpoint_id_t endpoint_id,
                           association_group_id_t group_id);

/**
 * @brief Checks if an association is already part of a group
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 * @param association   The Association object to look for
 *
 * @returns true if the association is already in the group, false otherwise
 */
bool is_association_in_group(zwave_node_id_t node_id,
                             zwave_endpoint_id_t endpoint_id,
                             association_group_id_t group_id,
                             association_t association);

/**
 * @brief Attempts to remove a NodeID for all associations groups in the
 * current network.
 *
 * @param node_id   Z-Wave Node ID to remove from
 *                  any and all association groups
 *
 */
void remove_desired_node_id_from_all_associations_in_network(
  zwave_node_id_t node_id);

/**
 * @brief Attempts to add an Association for all associations groups for a given
 * NodeID/Ep.
 *
 * @param node_id             Z-Wave Node ID for which the
 *                            association groups get added an association
 * @param endpoint_id         Endpoint for which the
 *                            association groups get added an association
 * @param association         The Association object to add
 */
void add_desired_association_in_all_groups(zwave_node_id_t node_id,
                                           zwave_endpoint_id_t endpoint_id,
                                           association_t association);

/**
 * @brief Attempts to remove an Association for all associations groups for a given
 * NodeID/Ep.
 *
 * @param node_id             Z-Wave Node ID for which the
 *                            association groups get added an association
 * @param endpoint_id         Endpoint for which the
 *                            association groups get added an association
 * @param association         The Association object to add
 */
void remove_desired_association_in_all_groups(zwave_node_id_t node_id,
                                              zwave_endpoint_id_t endpoint_id,
                                              association_t association);

/**
 * @brief Attempts to remove a NodeID for all associations groups for a given
 * NodeID/Ep.
 *
 * @param node_id_to_remove   Z-Wave Node ID to remove from a given NodeID/Endpoint
 * @param node_id             Z-Wave Node ID for which the
 *                            association groups get cleaned up
 * @param endpoint_id         Endpoint for which the
 *                            association groups get cleaned up
 */
void remove_desired_node_id_from_all_associations(
  zwave_node_id_t node_id_to_remove,
  zwave_node_id_t node_id,
  zwave_endpoint_id_t endpoint_id);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_ASSOCIATION_HELPER_H
/** @} end zwave_command_class_association_helper */
