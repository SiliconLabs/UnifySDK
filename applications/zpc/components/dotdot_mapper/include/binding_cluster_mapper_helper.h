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
 * @defgroup binding_cluster_mapper_helper Binding Cluster Mapping Helper
 * @ingroup dotdot_mapper Dotdot Mapper
 * @brief Module mapping bindable cluster to association group properties
 *
 * This module helps to translates before association groups data and bindings
 *
 * @{
 */

#ifndef BINDING_CLUSTER_MAPPER_HELPER_H
#define BINDING_CLUSTER_MAPPER_HELPER_H

#include <stdbool.h>
#include "uic_typedefs.h"
#include "zwave_generic_types.h"
#include "zwave_command_class_association_types.h"

// Maximum number of bindable clusters that we will advertise in the return values.
#define MAXIMUM_NUMBER_OF_BINDABLE_CLUSTERS 30

// Maximum number of bindable clusters that we will advertise in the return values.
#define MAXIMUM_NUMBER_OF_ASSOCIATION_GROUPS 255

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Checks if the Command Class / Command combination is mapped
 * to a cluster binding.
 *
 * @param command_class     The Command Class
 * @param command           The Command
 * @returns true if this combination is mapped to a cluster in the bindings, false otherwise
 */
bool is_command_class_and_command_mapped_to_bindings(
  zwave_command_class_t command_class, zwave_command_t command);
/**
 * @brief Finds the list of Association Groups to associate under a UNID/Endpoint
 * that sends command that belong to the Cluster ID
 *
 * @param unid                    Node UNID
 * @param endpoint                Node's EndpointID
 * @param cluster_id              The Cluster ID to match association groups from
 * @param [out] group_id_list     The list of Groups to associate. MUST be able to
 *                                contain up to MAXIMUM_NUMBER_OF_ASSOCIATION_GROUPS
 *                                elements
 * @param [out] number_of_groups  Number of Group IDs to association (number of
 *                                elements to read from the group_id_list)
 */
void binding_mapper_helper_get_association_group_list_from_cluster(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  dotdot_cluster_id_t cluster_id,
  association_group_id_t *group_id_list,
  uint8_t *number_of_groups);

/**
 * @brief Finds the list of Clusters that an association group id covers with the
 * commands it sends
 *
 * @param unid                      Node UNID
 * @param endpoint                  Node's EndpointID
 * @param group_id                  The Group ID to check for matching Cluster IDs
 * @param [out] cluster_id_list     The list of bindable clusters. MUST be able to
 *                                  contain up to MAXIMUM_NUMBER_OF_BINDABLE_CLUSTERS
 *                                  elements
 * @param [out] number_of_clusters  Number of bindable clusters
 */
void binding_mapper_helper_get_cluster_list_from_association_group(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  association_group_id_t group_id,
  dotdot_cluster_id_t *cluster_id_list,
  uint8_t *number_of_clusters);

/**
 * @brief Finds the list of bindable Clusters that the association groups
 * of a node covers with commands it sends
 *
 * @param unid                        Node UNID
 * @param endpoint                    Node's EndpointID
 * @param [out] cluster_id_list       The list of bindable clusters. MUST be able to
 *                                    contain up to MAXIMUM_NUMBER_OF_BINDABLE_CLUSTERS
 *                                    elements
 * @param [out] number_of_clusters    Number of bindable clusters
 */
void binding_mapper_helper_get_bindable_cluster_list_from_node(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  dotdot_cluster_id_t *cluster_id_list,
  uint8_t *number_of_clusters);

#ifdef __cplusplus
}
#endif

#endif  //BINDING_CLUSTER_MAPPER_HELPER_H
/** @} end binding_cluster_mapper_helper */
