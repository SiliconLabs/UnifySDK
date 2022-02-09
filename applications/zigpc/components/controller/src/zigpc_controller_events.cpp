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

#include <cinttypes>
#include <iomanip>
#include <string>
#include <vector>

// Shared UIC includes
#include <sl_log.h>
#include <dotdot_mqtt.h>

// ZigPC includes
#include <attribute_management.h>
#include <zcl_definitions.h>
#include <zigpc_common_zigbee.h>
#include <zigpc_common_unid.h>
#include <zigpc_datastore.h>
#include <zigpc_datastore.hpp>
#include <zigpc_discovery.h>
#include <zigpc_gateway.h>
#include <zigpc_group_mgmt.h>
#include <zigpc_net_mgmt.h>
#include <zigpc_ota_zigbee.h>
#include <zigpc_ucl.hpp>

#include "zigpc_controller_int.hpp"

namespace zigpc_ctrl
{
void zigpc_discovery_update_callback(zigbee_eui64_uint_t eui64,
                                     zigpc_discovery_status discovery_status)
{
  zigbee_eui64_t eui64_a;
  zigbee_uint_to_eui64(eui64, eui64_a);

  if (discovery_status == zigpc_discovery_status::DISCOVERY_START) {
    // IGNORE: discovery start updates
  } else if (discovery_status == zigpc_discovery_status::DISCOVERY_SUCCESS) {
    sl_status_t status = zigpc_ctrl::on_device_interviewed(eui64_a, true);

    if (status != SL_STATUS_OK) {
      sl_log_warning(zigpc_ctrl::LOG_TAG,
                     "Device interview success event handler status: 0x%X",
                     status);
    }

  } else {
    sl_status_t status = zigpc_ctrl::on_device_interview_failed(eui64_a);

    if (status != SL_STATUS_OK) {
      sl_log_warning(zigpc_ctrl::LOG_TAG,
                     "Device interview fail event handler status: 0x%X",
                     status);
    }
  }
}

sl_status_t on_device_announce(const zigbee_eui64_t eui64)
{
  zigpc_device_data_t device_data;

  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);

  sl_status_t status = zigpc_datastore_read_device(eui64, &device_data);
  if (status == SL_STATUS_OK) {
    status = zigpc_ucl::node_state::publish_state(eui64_i,
                                                  device_data.network_status,
                                                  device_data.max_cmd_delay);
  }

  if (status == SL_STATUS_OK) {
    status = zigpc_discovery_interview_device(eui64_i,
                                              zigpc_discovery_update_callback);
  }

  return status;
}

sl_status_t on_device_leave(const zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;

  for (const zigbee_endpoint_id_t &endpoint_id:
       zigpc_datastore::endpoint::get_id_list(eui64)) {
    status = zigpc_group_remove_all(eui64, endpoint_id);
    if (status != SL_STATUS_OK) {
      sl_log_warning(LOG_TAG,
                     "Failed to remove all groups for device %016" PRIX64
                     " endpoint %u: 0x%X (skipping to next "
                     "endpoint)",
                     zigbee_eui64_to_uint(eui64),
                     endpoint_id,
                     status);
    }
  }

  // TODO: Unsubscribe listening to OTA images for this device

  status
    = zigpc_ucl::node_state::remove_node_topics(zigbee_eui64_to_uint(eui64));

  return status;
}

sl_status_t on_device_interview_failed(const zigbee_eui64_t eui64)
{
  zigpc_device_data_t device_data;

  sl_status_t status = zigpc_datastore_read_device(eui64, &device_data);
  if (status == SL_STATUS_OK) {
    status = zigpc_ucl::node_state::publish_state(zigbee_eui64_to_uint(eui64),
                                                  device_data.network_status,
                                                  device_data.max_cmd_delay);
  }

  return status;
}

sl_status_t on_device_interviewed(const zigbee_eui64_t eui64,
                                  bool configure_endpoint)
{
  zigpc_device_data_t device_data;
  sl_status_t status = zigpc_datastore_read_device(eui64, &device_data);
  if (status == SL_STATUS_OK) {
    status = zigpc_ucl::node_state::publish_state(zigbee_eui64_to_uint(eui64),
                                                  device_data.network_status,
                                                  device_data.max_cmd_delay);
  }

  std::vector<zigbee_endpoint_id_t> endpoint_list
    = zigpc_datastore::endpoint::get_id_list(eui64);

  for (const zigbee_endpoint_id_t &endpoint_id: endpoint_list) {
    if (configure_endpoint == true) {
      status = perform_endpoint_configuration(eui64, endpoint_id);
      if (status != SL_STATUS_OK) {
        sl_log_warning(LOG_TAG,
                       "Failed to configure endpoint %d: 0x%X",
                       endpoint_id,
                       status);
      }
    }
    status = update_endpoint_capabilities(eui64, endpoint_id);
    if (status != SL_STATUS_OK) {
      sl_log_warning(
        LOG_TAG,
        "Failed to update endpoint %d capabilities: 0x%X (skipping to next "
        "endpoint configuration)",
        endpoint_id,
        status);
    }
  }

  status = update_device_capabilities(eui64, endpoint_list);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "Failed to update device capabilities");
  }

  return status;
}

sl_status_t perform_endpoint_configuration(const zigbee_eui64_t eui64,
                                           zigbee_endpoint_id_t endpoint_id)
{
  sl_status_t status = SL_STATUS_OK;

  zigbee_endpoint_t ep_data = {};

  std::vector<zcl_cluster_id_t> server_cluster_list
    = zigpc_datastore::cluster::get_id_list(eui64,
                                            endpoint_id,
                                            ZCL_CLUSTER_SERVER_SIDE);

  ep_data.endpoint_id   = endpoint_id;
  ep_data.cluster_count = server_cluster_list.size();
  for (size_t i = 0; i < ep_data.cluster_count; i++) {
    ep_data.cluster_list[i].cluster_id = server_cluster_list[i];
  }

  status = zigpc_gateway_request_binding_endpoint(eui64, ep_data);

  if (status == SL_STATUS_OK) {
    status = configure_attributes_endpoint(eui64, ep_data);
  }

  return status;
}

sl_status_t update_device_capabilities(
  const zigbee_eui64_t eui64,
  const std::vector<zigbee_endpoint_id_t> &endpoint_list)
{
  std::string topic_prefix
    = "ucl/by-unid/" + zigpc_ucl::mqtt::build_unid(zigbee_eui64_to_uint(eui64));

  sl_status_t status
    = uic_mqtt_dotdot_state_endpoint_id_list_publish(topic_prefix.c_str(),
                                                     endpoint_list.size(),
                                                     endpoint_list.data(),
                                                     UCL_MQTT_PUBLISH_TYPE_ALL);

  return status;
}

sl_status_t update_endpoint_capabilities(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id)
{
  sl_status_t status = SL_STATUS_OK;

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
void on_startup(void)
{
  // Act as in every device discovered has finished interviewing
  for (auto const &eui64_i: zigpc_datastore::device::get_id_list()) {
    zigbee_eui64_t eui64;
    zigpc_device_data_t eui64_data;
    sl_status_t status = zigbee_uint_to_eui64(eui64_i, eui64);
    if (status == SL_STATUS_OK) {
      status = zigpc_datastore_read_device(eui64, &eui64_data);
    }
    if (status == SL_STATUS_OK) {
      eui64_data.network_status = ZIGBEE_NODE_STATUS_INCLUDED;

      status = zigpc_datastore_write_device(eui64, &eui64_data);
    }
    if (status == SL_STATUS_OK) {
      status = on_device_interviewed(eui64, false);
    }
    if (status != SL_STATUS_OK) {
      sl_log_warning(zigpc_ctrl::LOG_TAG,
                     "Startup device EUI64:%016" PRIX64
                     " update event handler status: 0x%X",
                     eui64_i,
                     status);
    }
  }
}

void on_shutdown(void)
{
  // Update each device status as not servicable by zigpc
  for (auto const &eui64_i: zigpc_datastore::device::get_id_list()) {
    zigbee_eui64_t eui64_array;
    zigpc_device_data_t data;
    sl_status_t status = zigbee_uint_to_eui64(eui64_i, eui64_array);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Failed to convert EUI64:%016" PRIX64 ": 0x%X",
                   eui64_i,
                   status);
      continue;
    }

    status = zigpc_datastore_read_device(eui64_array, &data);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Failed to read EUI64:%016" PRIX64 " data: 0x%X",
                   eui64_i,
                   status);
      continue;
    }

    data.network_status = ZIGBEE_NODE_STATUS_UNAVAILABLE;

    status = zigpc_datastore_write_device(eui64_array, &data);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Failed to write EUI64:%016" PRIX64 " data: 0x%X",
                   eui64_i,
                   status);
      continue;
    }

    status = zigpc_ucl::node_state::publish_state(eui64_i,
                                                  data.network_status,
                                                  data.max_cmd_delay);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Failed to publish EUI64:%016" PRIX64
                   " status as unavailable: 0x%X",
                   eui64_i,
                   status);
    }
  }

  zigpc_ucl::node_state::cleanup_all_node_topics();
}

}  // namespace zigpc_ctrl
