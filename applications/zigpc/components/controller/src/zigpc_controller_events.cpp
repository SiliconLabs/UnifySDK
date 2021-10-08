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

#include <iomanip>
#include <string>
#include <vector>

// Shared UIC includes
#include <sl_log.h>
#include <uic_mqtt.h>
#include <dotdot_mqtt.h>

// ZigPC includes
#include <attribute_management.h>
#include <zcl_definitions.h>
#include <zigpc_common_zigbee.h>
#include <zigpc_common_unid.h>
#include <zigpc_datastore.h>
#include <zigpc_datastore.hpp>
#include <zigpc_gateway.h>
#include <zigpc_net_mgmt.h>
#include <zigpc_ota_zigbee.h>
#include <zigpc_ucl.hpp>

#include "zigpc_controller_int.hpp"

namespace zigpc_ctrl
{
sl_status_t on_device_announced(const zigbee_eui64_t eui64)
{
  sl_status_t status = zigpc_net_mgmt_interview_node(eui64);

  return status;
}

sl_status_t on_device_interviewed(const zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;

  for (const zigbee_endpoint_id_t &endpoint_id:
       zigpc_datastore::endpoint::get_id_list(eui64)) {
    status = on_endpoint_interviewed(eui64, endpoint_id);
  }

  return status;
}

sl_status_t on_endpoint_interviewed(const zigbee_eui64_t eui64,
                                    zigbee_endpoint_id_t endpoint_id)
{
  zigbee_endpoint_t ep_data;

  std::vector<zcl_cluster_id_t> server_cluster_list
    = zigpc_datastore::cluster::get_id_list(eui64,
                                            endpoint_id,
                                            ZCL_CLUSTER_SERVER_SIDE);

  ep_data.endpoint_id   = endpoint_id;
  ep_data.cluster_count = server_cluster_list.size();
  for (size_t i = 0; i < ep_data.cluster_count; i++) {
    ep_data.cluster_list[i].cluster_id = server_cluster_list[i];
  }

  sl_status_t status = zigpc_gateway_request_binding_endpoint(eui64, ep_data);

  if (status == SL_STATUS_OK) {
    status = configure_attributes_endpoint(eui64, ep_data);
  }

  // Setup supported commands for endpoint
  if (status == SL_STATUS_OK) {
    std::string unid = zigpc_ucl::mqtt::build_unid(zigbee_eui64_to_uint(eui64));
    uic_mqtt_dotdot_publish_supported_commands(unid.c_str(), endpoint_id);
  }

  // Setup OTA for capable devices
  if (status == SL_STATUS_OK) {
    bool contains_ota_cluster
      = zigpc_datastore_contains_cluster(eui64,
                                         endpoint_id,
                                         ZCL_CLUSTER_CLIENT_SIDE,
                                         ZIGPC_ZCL_CLUSTER_OTA_UPGRADE);

    if (contains_ota_cluster == true) {
      status = zigpc_ota_configure_node(eui64);
      if (status != SL_STATUS_OK) {
        sl_log_warning(LOG_TAG,
                       "Failed to configure OTA for device: 0x%X",
                       status);
      }
    }
  }

  return status;
}

void on_shutdown(void)
{
  std::vector<zigbee_eui64_uint_t> dev_list
    = zigpc_datastore::device::get_id_list();

  // get Protocol controller UNID to unretain the Protocol controller messages
  zigpc_network_data_t nwk;
  sl_status_t status = zigpc_datastore_read_network(&nwk);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Failed to get Gateway UNID to unretain: 0x%X",
                   status);
  } else {
    dev_list.push_back(zigbee_eui64_to_uint(nwk.gateway_eui64));
  }

  for (auto const &eui64: dev_list) {
    std::string topic = "ucl/by-unid/" + zigpc_ucl::mqtt::build_unid(eui64);
    uic_mqtt_unretain(topic.c_str());
  }
}

}  // namespace zigpc_ctrl
