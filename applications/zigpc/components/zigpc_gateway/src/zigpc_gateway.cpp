/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

// Shared UIC includes
#include "sl_log.h"

// ZigPC includes
#include <zcl_definitions.h>
#include <zigpc_datastore.h>

// Component includes
#include "zigpc_gateway.h"
#include "gateway_sleepy_queue.h"
#include "zigpc_gateway_request_queue.hpp"

namespace gw = zigpc::gateway;

sl_status_t
  zigpc_gateway_add_node_install_code(const zigbee_eui64_t node_eui64,
                                      const zigbee_install_code_t install_code,
                                      const uint8_t install_code_length)
{
  sl_status_t status = SL_STATUS_OK;

  if ((node_eui64 == nullptr) || (install_code == nullptr)) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    auto call
      = std::make_shared<gw::AddInstallCodeRequest>(node_eui64,
                                                    install_code,
                                                    install_code_length);

    gw::RequestQueue::getInstance().enqueue(call);
  }

  return status;
}

sl_status_t zigpc_gateway_network_init(void)
{
  sl_status_t status = SL_STATUS_OK;

  auto call = std::make_shared<gw::NetworkInitRequest>();

  gw::RequestQueue::getInstance().enqueue(call);

  return status;
}

sl_status_t zigpc_gateway_network_permit_joins(bool enable)
{
  sl_status_t status = SL_STATUS_OK;

  auto call = std::make_shared<gw::PermitJoinRequest>(enable);

  gw::RequestQueue::getInstance().enqueue(call);

  return status;
}

sl_status_t
  zigpc_gateway_send_zcl_command_frame(const zigbee_eui64_t eui64,
                                       zigbee_endpoint_id_t endpoint_id,
                                       zcl_cluster_id_t cluster_id,
                                       const zcl_frame_t *frame)
{
  sl_status_t status = SL_STATUS_OK;

  if ((eui64 == nullptr) || (frame == nullptr)) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    bool sleepy_cluster_found
      = zigpc_datastore_contains_cluster(eui64,
                                         endpoint_id,
                                         ZCL_CLUSTER_SERVER_SIDE,
                                         ZIGPC_ZCL_CLUSTER_POLL_CONTROL);

    if (!sleepy_cluster_found) {
      auto call = std::make_shared<gw::ZCLFrameUnicastRequest>(eui64,
                                                               endpoint_id,
                                                               cluster_id,
                                                               *frame);

      gw::RequestQueue::getInstance().enqueue(call);
    } else {
      gateway_frame_t frame_data;

      std::copy(eui64, eui64 + sizeof(zigbee_eui64_t), frame_data.eui64);
      frame_data.endpoint_id = endpoint_id;

      frame_data.cluster_id               = cluster_id;
      frame_data.frame.size               = frame->size;
      frame_data.frame.offset_sequence_id = frame->offset_sequence_id;
      std::copy(frame->buffer,
                frame->buffer + frame->size,
                frame_data.frame.buffer);

      status = zigpc_gateway_store_sleepy_frame(frame_data);
    }
  }

  return status;
}

sl_status_t zigpc_gateway_unload_sleepy_messages(const zigbee_eui64_t eui64)
{
  sl_status_t status       = SL_STATUS_OK;
  unsigned int num_retries = 0;

  //send a check-in response only if there are messages stored
  if (zigpc_gateway_retrieve_num_stored(eui64) > 0) {
    zigbeeHostSendPollingCheckInResponse();
  } else {
    status = SL_STATUS_EMPTY;
  }

  while ((status != SL_STATUS_EMPTY) && (status != SL_STATUS_FAIL)
         && (num_retries < MAX_SLEEPY_RETRIES)) {
    gateway_frame_t frame_data;

    status = zigpc_gateway_retrieve_sleepy_frame(eui64, &frame_data);

    if (status == SL_STATUS_OK) {
      auto call
        = std::make_shared<gw::ZCLFrameUnicastRequest>(frame_data.eui64,
                                                       frame_data.endpoint_id,
                                                       frame_data.cluster_id,
                                                       frame_data.frame);

      gw::RequestQueue::getInstance().enqueue(call);
    }

    num_retries = num_retries + 1;
  }

  return status;
}

sl_status_t zigpc_gateway_send_zcl_frame_multicast(zigbee_group_id_t group_id,
                                                   zcl_cluster_id_t cluster_id,
                                                   const zcl_frame_t *frame)
{
  sl_status_t status = SL_STATUS_OK;

  if (frame == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    auto call = std::make_shared<gw::ZCLFrameMulticastRequest>(group_id,
                                                               cluster_id,
                                                               *frame);

    gw::RequestQueue::getInstance().enqueue(call);
  }

  return status;
}

sl_status_t zigpc_gateway_request_binding(const zigbee_eui64_t eui64,
                                          uint8_t endpoint_id,
                                          zcl_cluster_id_t cluster_id)
{
  sl_status_t status = SL_STATUS_OK;

  if (eui64 == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    auto call = std::make_shared<gw::BindingRequestRequest>(eui64,
                                                            endpoint_id,
                                                            cluster_id);

    gw::RequestQueue::getInstance().enqueue(call);
  }

  return status;
}

sl_status_t
  zigpc_gateway_request_binding_endpoint(const zigbee_eui64_t eui64,
                                         const zigbee_endpoint_t endpoint)
{
  const unsigned int num_clusters = endpoint.cluster_count;
  for (unsigned int i = 0; i < num_clusters; i++) {
    sl_status_t status
      = zigpc_gateway_request_binding(eui64,
                                      endpoint.endpoint_id,
                                      endpoint.cluster_list[i].cluster_id);

    if (SL_STATUS_OK != status) {
      sl_log_warning(LOG_TAG,
                     "Error sending binding request for eui64 %llx - endpoint "
                     "%u - cluster %u",
                     eui64,
                     endpoint.endpoint_id,
                     endpoint.cluster_list[i].cluster_id);
    }
  }
  return SL_STATUS_OK;
}

sl_status_t zigpc_gateway_configure_reports(const zigbee_eui64_t eui64,
                                            uint8_t endpoint_id,
                                            zcl_cluster_id_t cluster_id,
                                            const uint8_t *report_record,
                                            unsigned int record_size)

{
  sl_status_t status = SL_STATUS_OK;

  if ((eui64 == nullptr) || (report_record == nullptr)) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    auto call
      = std::make_shared<gw::ZCLConfigureReportingRequest>(eui64,
                                                           endpoint_id,
                                                           cluster_id,
                                                           report_record,
                                                           record_size);

    gw::RequestQueue::getInstance().enqueue(call);
  }

  return status;
}

sl_status_t zigpc_gateway_remove_node(const zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;

  if (eui64 == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    auto call = std::make_shared<gw::DeviceRemoveRequest>(eui64);

    gw::RequestQueue::getInstance().enqueue(call);
  }

  return status;
}

sl_status_t zigpc_gateway_discover_device(const zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;

  if (eui64 == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    auto disc_dev_request = std::make_shared<gw::DiscoverDeviceRequest>(eui64);

    gw::RequestQueue::getInstance().enqueue(disc_dev_request);
  }

  return status;
}

sl_status_t zigpc_gateway_discover_endpoint(const zigbee_eui64_t eui64,
                                            zigbee_endpoint_id_t endpoint_id)
{
  sl_status_t status = SL_STATUS_OK;

  if (eui64 == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    auto disc_ep_request
      = std::make_shared<gw::DiscoverEndpointRequest>(eui64, endpoint_id);

    gw::RequestQueue::getInstance().enqueue(disc_ep_request);
  }

  return status;
}

sl_status_t zigpc_gateway_add_ota_image(const char *filename,
                                        unsigned int filename_size)
{
  sl_status_t status = SL_STATUS_OK;

  if ((filename != nullptr) && (filename_size > 0)) {
    std::string path = std::string(filename);
    auto call        = std::make_shared<gw::AddOTAImageRequest>(path);

    gw::RequestQueue::getInstance().enqueue(call);
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

void zigpc_gateway_command_print_info(void)
{
  zigbeeHostCommandPrintInfo();
}

void zigpc_gateway_command_print_nwk_key(void)
{
  EmberKeyStruct nwk_key;

  EmberStatus status
    = zigbeeHostGetEmberKey(EMBER_CURRENT_NETWORK_KEY, &nwk_key);
  if (status != EMBER_SUCCESS) {
    sl_log_error(LOG_TAG, "Failed to read NWK key: EmberStatus(0x%X)", status);
  } else {
    char nwk_key_str[EMBER_ENCRYPTION_KEY_SIZE * 3 + 1];
    size_t nwk_key_str_size = 0;
    for (uint8_t i = 0; i < EMBER_ENCRYPTION_KEY_SIZE; i++) {
      nwk_key_str_size += snprintf(nwk_key_str + nwk_key_str_size,
                                   4,
                                   " %02X",
                                   emberKeyContents(&(nwk_key.key))[i]);
    }
    sl_log_info(LOG_TAG, "NWK Key:%s", nwk_key_str);
  }
}

bool zigpc_gateway_install_code_is_valid(const uint8_t *install_code,
                                         uint8_t install_code_length)
{
  bool is_valid
    = zigbeeHostTrustCenterInstallCodeValid(install_code, install_code_length);
  if (!is_valid) {
    sl_log_warning(LOG_TAG, "Install code verification failed");
  }

  return is_valid;
}

sl_status_t zigpc_gateway_bootload_restart()
{
    EmberStatus status = 
        zigbeeHostLaunchBootloader();
    
    return (status == EMBER_SUCCESS) ? 
                SL_STATUS_OK : SL_STATUS_FAIL;
}
