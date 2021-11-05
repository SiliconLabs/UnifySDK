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

#include <algorithm>
#include <cinttypes>
#include <iterator>
#include <memory>
#include <queue>

// Shared UIC includes
#include <sl_log.h>
#include <sl_status.h>

// Component includes
#include "zigpc_gateway.h"
#include "zigpc_gateway_int.h"
#include "zigpc_gateway_request_queue.hpp"

#define LOG_PREFIX_GATEWAY          "Gateway "
#define LOG_PREFIX_EUI64            "Eui64:%016" PRIX64 " "
#define LOG_PREFIX_EUI64_EP_CLUSTER "Eui64:%016" PRIX64 ",Ep:%u,Cluster:0x%04X "
#define LOG_PREFIX_GROUP_CLUSTER    "Group:0x%04X,Cluster:0x%04X "

namespace zigpc::gateway
{
RequestQueue::Entry::Entry(const char *label) : label(label) {}

size_t RequestQueue::Entry::getSendAttempts(void) const
{
  return send_attempts;
}

void RequestQueue::Entry::incrementSendAttempt()
{
  this->send_attempts++;
}

const char *RequestQueue::Entry::getLabel(void) const
{
  return label;
}

RequestQueue::RequestQueue() {}

RequestQueue &RequestQueue::getInstance(void)
{
  static RequestQueue instance;
  return instance;
}
void RequestQueue::stopDispatching()
{
  this->dispatching_allowed = false;
  sl_log_info(LOG_TAG, "RequestQueue dispatching suspended");
}
void RequestQueue::startDispatching()
{
  sl_log_info(LOG_TAG, "RequestQueue dispatching resumed");
  this->dispatching_allowed = true;
  this->defer_cycles        = DEFER_CYCLES_START_BACKOFF;
}

void RequestQueue::enqueue(std::shared_ptr<Entry> call)
{
  this->queue.push(call);
}

sl_status_t RequestQueue::dispatch(void)
{
  sl_status_t status = SL_STATUS_OK;

  if (this->queue.empty()) {
    status = SL_STATUS_EMPTY;
  } else if (this->defer_cycles > 0) {
    // Skip sending any messages, only call tick loop
    this->defer_cycles--;
  } else if (this->dispatching_allowed == true) {
    const std::shared_ptr<Entry> &call = this->queue.front();
    sl_log_debug(LOG_TAG, "Request[%s] dequeued", call->getLabel());

    // Increase attempt count
    call->incrementSendAttempt();

    EmberStatus ember_status = call->invoke();

    sl_log_level_t log_level = sl_log_level_t::SL_LOG_DEBUG;
    if (ember_status == EMBER_MAX_MESSAGE_LIMIT_REACHED) {
      // Handle case of too many in-flight messages
      log_level = sl_log_level_t::SL_LOG_WARNING;
      status    = SL_STATUS_TRANSMIT_BUSY;

      // Do not send any messages for a few cycles
      this->defer_cycles += DEFER_CYCLES_MESSAGE_LIMIT_REACHED;

    } else if (ember_status == EMBER_SUCCESS) {
      log_level = sl_log_level_t::SL_LOG_INFO;
      status    = SL_STATUS_OK;
      this->defer_cycles += DEFER_CYCLES_DEFAULT;

    } else if ((ember_status == EMBER_NETWORK_DOWN)
               || (ember_status == EZSP_NOT_CONNECTED)
               || (ember_status == EZSP_ERROR_VERSION_NOT_SET)) {
      // Do not send any messages for some cycles to see if an NCP reset is pending
      this->defer_cycles = DEFER_CYCLES_LOST_CONNECTION;
      log_level          = sl_log_level_t::SL_LOG_WARNING;
      status             = SL_STATUS_TRANSMIT_BUSY;

    } else {
      log_level = sl_log_level_t::SL_LOG_ERROR;
      status    = SL_STATUS_FAIL;
    }

    sl_log(LOG_TAG,
           log_level,
           "Request[%s] dispatch attempt[%u] status: 0x%X",
           call->getLabel(),
           call->getSendAttempts(),
           status);

    // Remove call if there was no problem sending it
    if (status != SL_STATUS_TRANSMIT_BUSY) {
      this->queue.pop();
    }
  }
  // Request EmberAf Tick loop even if there is no call to dispatch
  z3gatewayTick();

  return status;
}

NetworkInitRequest::NetworkInitRequest(void) :
  RequestQueue::Entry("Network Init")
{}

EmberStatus NetworkInitRequest::invoke(void)
{
  EmberStatus status = z3gatewayTrustCenterInit();

  sl_log(LOG_TAG,
         (status == EMBER_SUCCESS) ? SL_LOG_INFO : SL_LOG_ERROR,
         LOG_PREFIX_GATEWAY "Network Init EmberAfStatus: 0x%X",
         status);

  return status;
}

PermitJoinRequest::PermitJoinRequest(bool enable_joins) :
  RequestQueue::Entry("Permit Join Control"), enable_joins(enable_joins)
{}

EmberStatus PermitJoinRequest::invoke(void)
{
  EmberStatus status = EMBER_SUCCESS;

  if (this->enable_joins) {
    const zigpc_config_t *conf = zigpc_get_config();
    if (conf->tc_use_well_known_key == true) {
      EmberEUI64 wildcardEui64
        = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
      EmberKeyData centralizedKey = z3gatewayGetTrustCenterWellKownKey();
      status
        = z3gatewayTrustCenterAddLinkKey(wildcardEui64, &centralizedKey, true);
      sl_log(LOG_TAG,
             (status == EMBER_SUCCESS) ? SL_LOG_INFO : SL_LOG_ERROR,
             LOG_PREFIX_GATEWAY "Well-known link key add EmberAfStatus: 0x%X",
             status);
    }
    status = z3gatewayTrustCenterJoinOpen(true);
    zigpc_gateway_command_print_nwk_key();
  } else {
    status = z3gatewayTrustCenterJoinClose();
  }

  sl_log(LOG_TAG,
         (status == EMBER_SUCCESS) ? SL_LOG_INFO : SL_LOG_ERROR,
         LOG_PREFIX_GATEWAY "%sable permit-join EmberAfStatus: 0x%X",
         (this->enable_joins) ? "En" : "Dis",
         status);

  return status;
}

AddInstallCodeRequest::AddInstallCodeRequest(
  const zigbee_eui64_t eui64,
  const zigbee_install_code_t install_code,
  uint8_t install_code_length) :
  RequestQueue::Entry("Add Install Code"),
  install_code_length(install_code_length)
{
  std::copy(eui64, eui64 + sizeof(zigbee_eui64_t), this->eui64);
  std::copy(install_code,
            install_code + sizeof(zigbee_install_code_t),
            this->install_code);
}

EmberStatus AddInstallCodeRequest::invoke(void)
{
  zigbee_eui64_t eui64_le;
  zigbee_eui64_copy_switch_endian(eui64_le, this->eui64);
  EmberStatus status
    = z3gatewayTrustCenterAddDeviceInstallCode(eui64_le,
                                               this->install_code,
                                               this->install_code_length);

  sl_log(LOG_TAG,
         (status == EMBER_SUCCESS) ? SL_LOG_INFO : SL_LOG_ERROR,
         LOG_PREFIX_EUI64 "Add install-code EmberAfStatus: 0x%X",
         zigbee_eui64_to_uint(this->eui64),
         status);

  return status;
}

DeviceInterviewRequest::DeviceInterviewRequest(const zigbee_eui64_t eui64) :
  RequestQueue::Entry("Device Interview")
{
  std::copy(eui64, eui64 + sizeof(zigbee_eui64_t), this->eui64);
}

EmberStatus DeviceInterviewRequest::invoke(void)
{
  zigbee_eui64_t eui64_le;
  zigbee_eui64_copy_switch_endian(eui64_le, this->eui64);
  EmberStatus status = z3gatewayTrustCenterStartDiscovery(eui64_le);

  sl_log(LOG_TAG,
         (status == EMBER_SUCCESS) ? SL_LOG_INFO : SL_LOG_ERROR,
         LOG_PREFIX_EUI64 "Interview EmberAfStatus: 0x%X",
         zigbee_eui64_to_uint(this->eui64),
         status);
  return status;
}

DeviceRemoveRequest::DeviceRemoveRequest(const zigbee_eui64_t eui64) :
  RequestQueue::Entry("Device Remove")
{
  std::copy(eui64, eui64 + sizeof(zigbee_eui64_t), this->eui64);
}

EmberStatus DeviceRemoveRequest::invoke(void)
{
  zigbee_eui64_t eui64_le;
  zigbee_eui64_copy_switch_endian(eui64_le, this->eui64);
  EmberStatus status = z3gatewayTrustCenterRemoveDevice(eui64_le);

  sl_log(LOG_TAG,
         (status == EMBER_SUCCESS) ? SL_LOG_INFO : SL_LOG_ERROR,
         LOG_PREFIX_EUI64 "Remove EmberAfStatus: 0x%X",
         zigbee_eui64_to_uint(this->eui64),
         status);

  return status;
}

BindingRequestRequest::BindingRequestRequest(const zigbee_eui64_t eui64,
                                             zigbee_endpoint_id_t endpoint_id,
                                             zcl_cluster_id_t cluster_id) :
  RequestQueue::Entry("Binding Request"),
  endpoint_id(endpoint_id),
  cluster_id(cluster_id)
{
  std::copy(eui64, eui64 + sizeof(zigbee_eui64_t), this->eui64);
}

EmberStatus BindingRequestRequest::invoke(void)
{
  zigbee_eui64_t eui64_le;
  zigbee_eui64_copy_switch_endian(eui64_le, this->eui64);
  EmberStatus status = z3gatewayInitBinding(eui64_le,
                                            this->endpoint_id,
                                            this->cluster_id,
                                            0);  //only unicast bindings for now

  sl_log(LOG_TAG,
         (status == EMBER_SUCCESS) ? SL_LOG_INFO : SL_LOG_ERROR,
         LOG_PREFIX_EUI64_EP_CLUSTER "Binding Request EmberAfStatus: 0x%X",
         zigbee_eui64_to_uint(this->eui64),
         this->endpoint_id,
         this->cluster_id,
         status);
  return status;
}

ZCLConfigureReportingRequest::ZCLConfigureReportingRequest(
  const zigbee_eui64_t eui64,
  zigbee_endpoint_id_t endpoint_id,
  zcl_cluster_id_t cluster_id,
  const uint8_t *report_record,
  unsigned int report_size) :
  RequestQueue::Entry("ZCL Configure Reporting"),
  endpoint_id(endpoint_id),
  cluster_id(cluster_id)
{
  std::copy(eui64, eui64 + sizeof(zigbee_eui64_t), this->eui64);

  // Copy report record into ZCL frame data
  this->frame.size = report_size;
  std::copy(report_record,
            report_record + this->frame.size,
            this->frame.buffer);
}

EmberStatus ZCLConfigureReportingRequest::invoke(void)
{
  zigbee_eui64_t eui64_le;
  zigbee_eui64_copy_switch_endian(eui64_le, this->eui64);

  unsigned int report_size = static_cast<unsigned int>(this->frame.size);
  EmberStatus status       = z3gatewayInitReporting(eui64_le,
                                              this->endpoint_id,
                                              this->cluster_id,
                                              this->frame.buffer,
                                              report_size);

  sl_log(LOG_TAG,
         (status == EMBER_SUCCESS) ? SL_LOG_INFO : SL_LOG_ERROR,
         LOG_PREFIX_EUI64_EP_CLUSTER
         "ZCL Configure Reporting EmberAfStatus: 0x%X",
         zigbee_eui64_to_uint(this->eui64),
         this->endpoint_id,
         this->cluster_id,
         status);

  return status;
}

ZCLFrameUnicastRequest::ZCLFrameUnicastRequest(const zigbee_eui64_t eui64,
                                               zigbee_endpoint_id_t endpoint_id,
                                               zcl_cluster_id_t cluster_id,
                                               const zcl_frame_t &frame) :

  RequestQueue::Entry("ZCL Frame Unicast"),
  endpoint_id(endpoint_id),
  cluster_id(cluster_id),
  frame(frame)
{
  std::copy(eui64, eui64 + sizeof(zigbee_eui64_t), this->eui64);
}

EmberStatus ZCLFrameUnicastRequest::invoke(void)
{
  zigbee_eui64_t eui64_le;
  zigbee_eui64_copy_switch_endian(eui64_le, this->eui64);
  EmberStatus status = z3gatewayFillZclFrame(this->frame.buffer,
                                             this->frame.size,
                                             this->frame.offset_sequence_id);

  if (status == EMBER_SUCCESS) {
    status = z3gatewaySendZclFrameUnicast(eui64_le,
                                          this->endpoint_id,
                                          this->cluster_id);

    sl_log(LOG_TAG,
           (status == EMBER_SUCCESS) ? SL_LOG_INFO : SL_LOG_WARNING,
           LOG_PREFIX_EUI64_EP_CLUSTER "ZCL frame send EmberAfStatus: 0x%X",
           zigbee_eui64_to_uint(this->eui64),
           this->endpoint_id,
           this->cluster_id,
           status);
  } else {
    sl_log_error(LOG_TAG,
                 "Failed to fill ZCL frame to send unicast EmberAfStatus: 0x%X",
                 status);
  }

  return status;
}

ZCLFrameMulticastRequest::ZCLFrameMulticastRequest(zigbee_group_id_t group_id,
                                                   zcl_cluster_id_t cluster_id,
                                                   const zcl_frame_t &frame) :
  RequestQueue::Entry("ZCL Frame Multicast"),
  group_id(group_id),
  cluster_id(cluster_id),
  frame(frame)
{}

EmberStatus ZCLFrameMulticastRequest::invoke(void)
{
  EmberStatus status = z3gatewayFillZclFrame(this->frame.buffer,
                                             this->frame.size,
                                             this->frame.offset_sequence_id);

  if (status == EMBER_SUCCESS) {
    status = z3gatewaySendZclFrameMulticast(this->group_id, this->cluster_id);

    sl_log(LOG_TAG,
           (status == EMBER_SUCCESS) ? SL_LOG_INFO : SL_LOG_WARNING,
           LOG_PREFIX_GROUP_CLUSTER "ZCL frame send EmberAfStatus: 0x%X",
           this->group_id,
           this->cluster_id,
           status);
  } else {
    sl_log_error(
      LOG_TAG,
      "Failed to fill ZCL frame to send as multicast EmberAfStatus: 0x%X",
      status);
  }

  return status;
}

AddOTAImageRequest::AddOTAImageRequest(std::string &filename) :
  RequestQueue::Entry("Add OTA Image"), filename(filename)
{}

EmberStatus AddOTAImageRequest::invoke(void)
{
  EmberStatus status = z3gatewayAddOtaImage(this->filename.c_str());

  sl_log(LOG_TAG,
         (status == EMBER_SUCCESS) ? SL_LOG_INFO : SL_LOG_ERROR,
         LOG_PREFIX_GATEWAY "Add OTA Image [%s] EmberAfStatus: 0x%X",
         this->filename.c_str(),
         status);

  return status;
}

}  // namespace zigpc::gateway
