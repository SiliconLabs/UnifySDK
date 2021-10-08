/*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/**
 * @file zigpc_datastore.hpp
 * @defgroup zigpc_datastore_hpp ZigPC Datastore C++ API
 * @ingroup zigpc_components
 * @brief C++ API for the ZigPC datstore component.
 *
 * See zigpc_datastore.h for more information on the component responsibilities.
 *
 * @{
 *
**/

#ifndef ZIGPC_DATASTORE_HPP
#define ZIGPC_DATASTORE_HPP

#include <vector>

// ZigPC includes
#include <zigpc_common_zigbee.h>

namespace zigpc_datastore
{
namespace device
{
/**
 * @brief Get list of devices on the network.
 *
 * @return std::vector<zigbee_eui64_uint_t> List of device IDs.
 */
std::vector<zigbee_eui64_uint_t> get_id_list(void);

}  // namespace device

namespace endpoint
{
/**
 * @brief Get list of endpoint IDs under a device.
 *
 * @param eui64 Device identifier.
 * @return std::vector<zigbee_endpoint_id_t> List of endpoint IDs.
 */
std::vector<zigbee_endpoint_id_t> get_id_list(const zigbee_eui64_t eui64);

}  // namespace endpoint

namespace cluster
{
/**
 * @brief Get list of cluster IDs under an endpoint.
 *
 * @param eui64         Device identifier.
 * @param ep_id         Endpoint identifer.
 * @param cluster_side  Cluster side (server/client)
 * @return std::vector<zcl_cluster_id_t>  List of cluster IDs.
 */
std::vector<zcl_cluster_id_t> get_id_list(const zigbee_eui64_t eui64,
                                          zigbee_endpoint_id_t ep_id,
                                          zcl_cluster_side_t cluster_side);
}  // namespace cluster

}  // namespace zigpc_datastore

#endif  // ZIGPC_DATASTORE_HPP

/** @} end zigpc_datastore_hpp **/