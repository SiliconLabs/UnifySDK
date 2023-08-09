/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup unify_dotdot_attribute_store_group_cluster C++ Helpers for Unify DotDot Attribute Store Group Cluster
 *
 * @{
 */

#ifndef UNIFY_DOTDOT_ATTRIBUTE_STORE_GROUP_CLUSTER_HPP
#define UNIFY_DOTDOT_ATTRIBUTE_STORE_GROUP_CLUSTER_HPP

#include "attribute_store.h"
#include "zap-types.h"
#include <string>

/**
 * @brief Publishes the list of groups for an unid/endpoint
 * ucl/by-unid/&lt;unid&gt;/ep&lt;id&gt;/Groups/Attributes/GroupList/Reported
 * ucl/by-unid/&lt;unid&gt;/ep&lt;id&gt;/Groups/Attributes/GroupList/Desired
 *
 * @param endpoint_id_node   Attribute Store node of the endpoint
 */
void publish_group_list_cluster_attribute(
  attribute_store_node_t endpoint_id_node);

/**
 * @brief Publishes the Groups NameSupport attribute for an unid/endpoint
 * ucl/by-unid/&lt;unid&gt;/ep&lt;id&gt;/Groups/Attributes/NameSupport/Reported
 * ucl/by-unid/&lt;unid&gt;/ep&lt;id&gt;/Groups/Attributes/NameSupport/Desired
 *
 * @param [in] endpoint_id_node   Attribute Store node of the endpoint
 */
void publish_name_support_cluster_attribute(
  attribute_store_node_t endpoint_id_node);

/**
 * @brief Publishes the Groups SupportedCommands attribute for an unid/endpoint
 * ucl/by-unid/&lt;unid&gt;/ep&lt;id&gt;/Groups/SupportedCommands
 *
 * If the passed UNID is the Protocol Controller's unid, no publications will be made.
 *
 * The list of commands is static and identical for all unid/endpoints, so no
 * verification will be made and it will just be published as such.
 *
 * @param [in] unid          UNID to use in the topic
 * @param [in] endpoint_id   Endpoint ID to use in the topic
 *
 */
void publish_group_cluster_supported_commands(const std::string &unid,
                                              dotdot_endpoint_id_t endpoint_id);

/**
 * @brief Publishes the Groups Cluster Revision for an unid/endpoint
 * ucl/by-unid/&lt;unid&gt;/ep&lt;id&gt;/Groups/Attributes/ClusterRevision
 *
 * If the passed UNID is the Protocol Controller's unid, no publications will be made.
 *
 * @param [in] unid          UNID to use in the topic
 * @param [in] endpoint_id   Endpoint ID to use in the topic
 *
 */
void publish_group_cluster_cluster_revision(const std::string &unid,
                                            dotdot_endpoint_id_t endpoint_id);

/**
 * @brief Publishes the Groups Name attribute for an unid/endpoint and Group ID
 * ucl/by-unid/&lt;unid&gt;/ep&lt;id&gt;/Groups/&lt;GroupID&gt;/Name/Reported
 * ucl/by-unid/&lt;unid&gt;/ep&lt;id&gt;/Groups/&lt;GroupID&gt;/Name/Desired
 * *
 * The list of commands is static and identical for all unid/endpoints, so no
 * verification will be made and it will just be published as such.
 *
 * @param [in] group_name_node    Attribute Store Node containing the group name string
 */
void publish_group_name_cluster_attribute(
  attribute_store_node_t group_name_node);

/**
 * @brief Removes the Groups Name attribute publications for an unid/endpoint
 * and Group ID
 * ucl/by-unid/&lt;unid&gt;/ep&lt;id&gt;/Groups/&lt;GroupID&gt;/Name/Reported
 * ucl/by-unid/&lt;unid&gt;/ep&lt;id&gt;/Groups/&lt;GroupID&gt;/Name/Desired
 *
 * If the passed UNID is the ZPC's unid, no publications will be made.
 *
 * The list of commands is static and identical for all unid/endpoints, so no
 * verification will be made and it will just be published as such.
 *
 * @param [in] unid          UNID to use in the topic
 * @param [in] endpoint_id   Endpoint ID to use in the topic
 * @param [in] group_id      Group ID to use in the topic
 */
void unretain_group_name_publications(const std::string &unid,
                                      dotdot_endpoint_id_t endpoint_id,
                                      uint16_t group_id);

#endif  //UNIFY_DOTDOT_ATTRIBUTE_STORE_GROUP_CLUSTER_HPP
/** @} end unify_dotdot_attribute_store_group_cluster */
