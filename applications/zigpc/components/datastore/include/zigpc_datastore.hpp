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
#include "zigpc_datastore.h"

namespace zigpc_datastore
{
namespace device
{
/**
 * @brief Get list of devices on the network.
 *
 * @return std::vector<zigbee_eui64_uint_t> List of device IDs.
 */
inline std::vector<zigbee_eui64_uint_t> get_id_list(void)
{
  sl_status_t status = SL_STATUS_OK;

  const size_t dev_count = zigpc_datastore_get_device_count();

  std::vector<zigbee_eui64_uint_t> dev_list(dev_count);
  for (size_t i = 0; (status == SL_STATUS_OK) && (i < dev_count); i++) {
    zigbee_eui64_t eui64;
    status = zigpc_datastore_find_device_by_index(i, &eui64);
    if (status == SL_STATUS_OK) {
      dev_list[i] = zigbee_eui64_to_uint(eui64);
    }
  }

  return dev_list;
}

}  // namespace device

namespace endpoint
{
/**
 * @brief Get list of endpoint IDs under a device.
 *
 * @param eui64 Device identifier.
 * @return std::vector<zigbee_endpoint_id_t> List of endpoint IDs.
 */
inline std::vector<zigbee_endpoint_id_t> get_id_list(const zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;
  size_t ep_count    = 0;

  if (eui64 == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    ep_count = zigpc_datastore_get_endpoint_count(eui64);
  }

  std::vector<zigbee_endpoint_id_t> ep_list(ep_count);
  for (size_t i = 0; (status == SL_STATUS_OK) && (i < ep_count); i++) {
    status
      = zigpc_datastore_find_endpoint_by_index(eui64, i, ep_list.data() + i);
  }

  return ep_list;
}

}  // namespace endpoint

namespace cluster
{
/**
 * @brief Get list of cluster IDs under an endpoint based on the cluser side.
 *
 * @param eui64         Device identifier.
 * @param ep_id         Endpoint identifier.
 * @param cluster_side  Cluster side (server/client)
 * @return std::vector<zcl_cluster_id_t>  List of cluster IDs.
 */
inline std::vector<zcl_cluster_id_t>
  get_id_list(const zigbee_eui64_t eui64,
              zigbee_endpoint_id_t ep_id,
              zcl_cluster_side_t cluster_side)
{
  sl_status_t status   = SL_STATUS_OK;
  size_t cluster_count = 0;

  if (eui64 == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    cluster_count
      = zigpc_datastore_get_cluster_count(eui64, ep_id, cluster_side);
  }

  std::vector<zcl_cluster_id_t> cluster_list(cluster_count);
  for (size_t i = 0; (status == SL_STATUS_OK) && (i < cluster_count); i++) {
    status = zigpc_datastore_find_cluster_by_index(eui64,
                                                   ep_id,
                                                   cluster_side,
                                                   i,
                                                   cluster_list.data() + i);
  }

  return cluster_list;
}

}  // namespace cluster

namespace group
{
/**
 * @brief Get list of group IDs under an endpoint based on the report/desired
 * state.
 *
 * @param eui64         Device identifier.
 * @param ep_id         Endpoint identifier.
 * @param is_reported   Look in stored reported/desired state.
 * @return std::vector<zcl_cluster_id_t>  List of group IDs.
 */
inline std::vector<zigbee_group_id_t> get_group_list(const zigbee_eui64_t eui64,
                                                     zigbee_endpoint_id_t ep_id,
                                                     bool is_reported)
{
  sl_status_t status = SL_STATUS_OK;
  size_t group_count = 0;

  if (eui64 == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    group_count = zigpc_datastore_get_group_count(eui64, ep_id, is_reported);
  }

  std::vector<zigbee_group_id_t> group_list(group_count);
  for (size_t i = 0; (status == SL_STATUS_OK) && (i < group_count); i++) {
    status = zigpc_datastore_find_group_by_index(eui64,
                                                 ep_id,
                                                 is_reported,
                                                 i,
                                                 group_list.data() + i);
  }

  return group_list;
}
}  // namespace group

}  // namespace zigpc_datastore

#endif  // ZIGPC_DATASTORE_HPP

/** @} end zigpc_datastore_hpp **/
