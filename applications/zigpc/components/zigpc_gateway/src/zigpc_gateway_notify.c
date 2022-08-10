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

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

// Shared Unify includes
#include <sl_log.h>
#include <sl_status.h>

// ZigPC includes
#include <zigpc_common_observable.h>

// Component includes
#include "zigpc_gateway_notify.h"
#include "zigpc_gateway_int.h"

struct zigpc_observable zigpc_gateway_observable;

sl_status_t
  zigpc_gateway_register_observer(enum zigpc_gateway_notify_event event,
                                  zigpc_observer_callback_t callback)
{
  return zigpc_observable_register(&zigpc_gateway_observable, event, callback);
}
sl_status_t
  zigpc_gateway_unregister_observer(enum zigpc_gateway_notify_event event,
                                    zigpc_observer_callback_t callback)
{
  return zigpc_observable_unregister(&zigpc_gateway_observable,
                                     event,
                                     callback);
}

sl_status_t zigpc_gateway_reset_observers(void)
{
  sl_status_t result = SL_STATUS_OK;

  static const zigpc_observer_event_t observer_events[ZIGPC_GATEWAY_NOTIFY_MAX]
    = {
      ZIGPC_GATEWAY_NOTIFY_NETWORK_INIT,
      ZIGPC_GATEWAY_NOTIFY_NODE_ADD_START,
      ZIGPC_GATEWAY_NOTIFY_NODE_ADD_COMPLETE,
      ZIGPC_GATEWAY_NOTIFY_NODE_REMOVED,
      ZIGPC_GATEWAY_NOTIFY_NODE_DISCOVERED,
      ZIGPC_GATEWAY_NOTIFY_NODE_ENDPOINT_DISCOVERED,
      ZIGPC_GATEWAY_NOTIFY_REPORTED_ATTRIBUTE,
      ZIGPC_GATEWAY_NOTIFY_READ_ATTRIBUTE_RESPONSE,
      ZIGPC_GATEWAY_NOTIFY_CONFIGURE_REPORTING_RESPONSE,
      ZIGPC_GATEWAY_NOTIFY_ZCL_COMMAND_RECEIVED,
      ZIGPC_GATEWAY_NOTIFY_OTA_UPDATE_STARTED,
      ZIGPC_GATEWAY_NOTIFY_OTA_UPDATE_COMPLETED,
    };

  zigpc_observable_clear(&zigpc_gateway_observable);

  result = zigpc_observable_init(&zigpc_gateway_observable,
                                 observer_events,
                                 ZIGPC_GATEWAY_NOTIFY_MAX);
  if (result != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Observer init failed: 0x%X", result);
  }

  return result;
}

/**
 * @brief ZigbeeHost handler for network initialization
 *
 * @param network     Network properties.
 */
void zigpc_gateway_hdl_on_network_initialized(
  const EmberNetworkParameters *network)
{
  static struct zigpc_gateway_on_network_init network_init = {0};

  EmberEUI64 eui64_le;
  zigbeeHostGetEui64(eui64_le);
  zigbee_eui64_copy_switch_endian(network_init.zigpc_eui64, eui64_le);

  network_init.zigpc_endpoint_id = zigbeeHostGetPrimaryEndpointId();

  network_init.zigpc_panid = network->panId;
  memcpy(network_init.zigpc_ext_panid,
         network->extendedPanId,
         ZIGBEE_EXTENDED_PAN_ID_SIZE);
  network_init.zigpc_radio_channel = network->radioChannel;

  sl_status_t result
    = zigpc_observable_notify(&zigpc_gateway_observable,
                              ZIGPC_GATEWAY_NOTIFY_NETWORK_INIT,
                              (void *)&network_init);
  if (result != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "NOTIFY_NETWORK_INIT failed: 0x%X", result);
  }
}

/**
 * @brief ZigbeeHost callback handler when a node has started joining
 * the network. This handler will notify any listening components of
 * the node that has started the join process.
 *
 * @param eui64_le  Joining node identifier in little endian
 */
void zigpc_gateway_hdl_on_node_add_start(const EmberEUI64 eui64_le)
{
  sl_status_t status;
  static struct zigpc_gateway_on_node_add node_add = {0};

  zigbee_eui64_copy_switch_endian(node_add.eui64, eui64_le);

  sl_log_debug(LOG_TAG,
               "EUI64 %016" PRIX64 ": Device announced",
               zigbee_eui64_to_uint(node_add.eui64));

  status = zigpc_observable_notify(&zigpc_gateway_observable,
                                   ZIGPC_GATEWAY_NOTIFY_NODE_ADD_START,
                                   (void *)&node_add);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "NOTIFY_NODE_ADD_START failed: 0x%X", status);
  }
}

/**
 * @brief ZigbeeHost callback handler when a node has finished joining
 * the network. This handler will notify any listening components of
 * the node that has completed the join process.
 *
 * @param eui64_le  Joined node identifier in little endian
 */
void zigpc_gateway_hdl_on_node_add_complete(const EmberEUI64 eui64_le)
{
  sl_status_t status;
  static struct zigpc_gateway_on_node_add node_add = {0};

  zigbee_eui64_copy_switch_endian(node_add.eui64, eui64_le);

  sl_log_debug(LOG_TAG,
               "EUI64 %016" PRIX64 ": Key replacement verified",
               zigbee_eui64_to_uint(node_add.eui64));

  status = zigpc_observable_notify(&zigpc_gateway_observable,
                                   ZIGPC_GATEWAY_NOTIFY_NODE_ADD_COMPLETE,
                                   (void *)&node_add);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "NOTIFY_NODE_ADD_COMPLETE failed: 0x%X", status);
  }
}
/**
 * @brief Notify handler for device removal complete callback.
 *
 * @param eui64_le      Device identifier
 */
void zigpc_gateway_hdl_on_node_removed(const EmberEUI64 eui64_le)
{
  sl_status_t status;
  static zigpc_gateway_on_node_removed_t node_removed = {0};

  zigbee_eui64_copy_switch_endian(node_removed.eui64, eui64_le);

  sl_log_debug(LOG_TAG,
               "EUI64 %016" PRIX64 ": Removed",
               zigbee_eui64_to_uint(node_removed.eui64));

  status = zigpc_observable_notify(&zigpc_gateway_observable,
                                   ZIGPC_GATEWAY_NOTIFY_NODE_REMOVED,
                                   (void *)&node_removed);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "NOTIFY_NODE_REMOVED failed: 0x%X", status);
  }
}

/**
 * @brief Notify handler for device discovery containing endpoint count
 *
 * @param eui64_le      Device identifier.
 * @param endpointCount Number of endpoints discovery events to expect.
 * @param endpointList  List of endpoints discovered.
 */
void zigpc_gateway_hdl_on_device_endpoints_discovered(
  const EmberEUI64 eui64_le, uint8_t endpointCount, const uint8_t *endpointList)
{
  if ((eui64_le == NULL) || ((endpointCount > 0U) && (endpointList == NULL))) {
    return;
  }

  zigpc_gateway_on_node_discovered_t node_disc = {0};

  zigbee_eui64_copy_switch_endian(node_disc.eui64, eui64_le);
  node_disc.endpoint_count = endpointCount;

  if (endpointCount == 0U) {
    // Notify and return early if no endpoints are active
    node_disc.endpoint_list = NULL;

    sl_status_t status
      = zigpc_observable_notify(&zigpc_gateway_observable,
                                ZIGPC_GATEWAY_NOTIFY_NODE_DISCOVERED,
                                (void *)&node_disc);
    if (status != SL_STATUS_OK) {
      sl_log_warning(LOG_TAG, "NOTIFY_NODE_DISCOVER failed: 0x%X", status);
    }

    return;
  }

  zigbee_endpoint_id_t endpoint_list[endpointCount];
  memcpy(endpoint_list,
         endpointList,
         sizeof(zigbee_endpoint_id_t) * endpointCount);
  node_disc.endpoint_list = endpoint_list;

  sl_log_debug(LOG_TAG,
               "EUI64 %016" PRIX64 ": %u endpoints discovered",
               zigbee_eui64_to_uint(node_disc.eui64),
               endpointCount);

  sl_status_t status
    = zigpc_observable_notify(&zigpc_gateway_observable,
                              ZIGPC_GATEWAY_NOTIFY_NODE_DISCOVERED,
                              (void *)&node_disc);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "NOTIFY_NODE_DISCOVER failed: 0x%X", status);
  }
}

/**
 * @brief Notify handler for ZDO simple descriptor response containing
 * endpoint discovery information.
 *
 * @param eui64_le      Device identifier
 * @param endpointInfo  Discovered endpoint information
 */
void zigpc_gateway_hdl_on_endpoint_clusters_discovered(
  const EmberEUI64 eui64_le, const EmberAfClusterList *endpointInfo)
{
  if ((eui64_le == NULL) || (endpointInfo == NULL)) {
    return;
  }

  zigpc_gateway_on_node_endpoint_discovered_t endpoint_discovered = {0};

  zigbee_eui64_copy_switch_endian(endpoint_discovered.eui64, eui64_le);
  zigbee_eui64_uint_t eui64_uint
    = zigbee_eui64_to_uint(endpoint_discovered.eui64);
  sl_log_debug(LOG_TAG,
               "Discovered EUI64:%016" PRIX64
               " endpoint %u: deviceId: 0x%04X cluster count: %u",
               eui64_uint,
               endpointInfo->endpoint,
               endpointInfo->deviceId,
               endpointInfo->inClusterCount);
  for (int i = 0; i < endpointInfo->inClusterCount; i++) {
    sl_log_debug(LOG_TAG,
                 "Discovered EUI64 %016" PRIX64
                 ": endpoint %u: cluster %u = 0x%04X",
                 eui64_uint,
                 endpointInfo->endpoint,
                 i,
                 endpointInfo->inClusterList[i]);
  }

  endpoint_discovered.endpoint.endpoint_id   = endpointInfo->endpoint;
  endpoint_discovered.endpoint.cluster_count = endpointInfo->inClusterCount;

  for (size_t cluster_index = 0; cluster_index < endpointInfo->inClusterCount;
       cluster_index++) {
    endpoint_discovered.endpoint.cluster_list[cluster_index].cluster_id
      = endpointInfo->inClusterList[cluster_index];
  }

  endpoint_discovered.endpoint.client_cluster_count
    = endpointInfo->outClusterCount;
  for (size_t cluster_index = 0; cluster_index < endpointInfo->outClusterCount;
       cluster_index++) {
    endpoint_discovered.endpoint.client_cluster_list[cluster_index].cluster_id
      = endpointInfo->outClusterList[cluster_index];

    if (endpointInfo->outClusterList[cluster_index]
        == ZIGPC_ZCL_CLUSTER_OTA_UPGRADE) {
      endpoint_discovered.endpoint.cluster_list[endpointInfo->inClusterCount]
        .cluster_id
        = endpointInfo->outClusterList[cluster_index];

      endpoint_discovered.endpoint.cluster_count++;
    }
  }

  sl_status_t status
    = zigpc_observable_notify(&zigpc_gateway_observable,
                              ZIGPC_GATEWAY_NOTIFY_NODE_ENDPOINT_DISCOVERED,
                              (void *)&endpoint_discovered);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Notify NODE_ENDPOINT_DISCOVERED failed: 0x%X",
                   status);
  }
}

void zigpc_gateway_hdl_on_reported_attribute(
  const EmberEUI64 eui64_le,
  uint8_t endpoint,
  uint16_t clusterId,
  uint8_t *attribute_status_records,
  uint16_t attribute_status_records_size)
{
  sl_status_t status = SL_STATUS_OK;
  zigpc_gateway_on_attribute_reported_t reported_data;

  if (attribute_status_records_size > ZCL_FRAME_BUFFER_SIZE_MAX) {
    status = SL_STATUS_WOULD_OVERFLOW;
    sl_log_error(LOG_TAG,
                 "Cannot copy over report that is bigger than ZCL frame"
                 " size allocated: 0x%04X",
                 status);
  }

  if (status == SL_STATUS_OK) {
    zigbee_eui64_copy_switch_endian(reported_data.eui64, eui64_le);
    reported_data.endpoint_id = endpoint;
    reported_data.cluster_id  = clusterId;
    reported_data.frame.size  = attribute_status_records_size;

    memcpy(reported_data.frame.buffer,
           attribute_status_records,
           sizeof(uint8_t) * reported_data.frame.size);

    sl_log_debug(LOG_TAG,
                 "Attribute report from EUI64:%016" PRIX64
                 " endpoint:%d, cluster:%d, frame_size:%d",
                 zigbee_eui64_to_uint(reported_data.eui64),
                 reported_data.endpoint_id,
                 reported_data.cluster_id,
                 reported_data.frame.size);

    status = zigpc_observable_notify(&zigpc_gateway_observable,
                                     ZIGPC_GATEWAY_NOTIFY_REPORTED_ATTRIBUTE,
                                     (void *)&reported_data);
  }

  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "ATTRIBUTE REPORTING FAILED: 0x%X", status);
  }
}

void zigpc_gateway_hdl_on_read_attributes(
  const EmberEUI64 eui64_le,
  uint8_t endpoint,
  uint16_t clusterId,
  uint8_t *attribute_status_records,
  uint16_t attribute_status_records_size)
{
  sl_status_t status = SL_STATUS_OK;
  static zigpc_gateway_on_read_attribute_response_t read_response = {0};

  if (attribute_status_records_size > ZCL_FRAME_BUFFER_SIZE_MAX) {
    status = SL_STATUS_WOULD_OVERFLOW;
    sl_log_error(LOG_TAG,
                 "Cannot copy over read response that is bigger than ZCL frame"
                 " size allocated: 0x%04X",
                 status);
  }

  if (status == SL_STATUS_OK) {
    zigbee_eui64_copy_switch_endian(read_response.eui64, eui64_le);

    sl_log_debug(LOG_TAG,
                 "Read Attribute Response from EUI64:%016" PRIX64
                 " endpoint: %u cluster: 0x%04X",
                 zigbee_eui64_to_uint(read_response.eui64),
                 endpoint,
                 clusterId);

    read_response.endpoint_id = endpoint;
    read_response.cluster_id  = clusterId;
    read_response.frame.size  = attribute_status_records_size;
    memcpy(read_response.frame.buffer,
           attribute_status_records,
           sizeof(uint8_t) * read_response.frame.size);

    status
      = zigpc_observable_notify(&zigpc_gateway_observable,
                                ZIGPC_GATEWAY_NOTIFY_READ_ATTRIBUTE_RESPONSE,
                                (void *)&read_response);
    if (status != SL_STATUS_OK) {
      sl_log_warning(LOG_TAG, "READ_ATTRIBUTE_RESPONSE failed: 0x%X", status);
    }
  }
}

void zigpc_gateway_hdl_on_configure_response(
  const EmberEUI64 eui64_le,
  uint8_t endpoint,
  uint16_t clusterId,
  uint8_t *attribute_status_records,
  uint16_t attribute_status_records_size)
{
  sl_status_t status                                            = SL_STATUS_OK;
  zigpc_gateway_on_read_attribute_response_t configure_response = {0};

  if (attribute_status_records_size > ZCL_FRAME_BUFFER_SIZE_MAX) {
    status = SL_STATUS_WOULD_OVERFLOW;
    sl_log_error(LOG_TAG,
                 "Cannot copy over read response that is bigger than ZCL frame"
                 " size allocated: 0x%04X",
                 status);

    return;
  }

  if (status == SL_STATUS_OK) {
    zigbee_eui64_copy_switch_endian(configure_response.eui64, eui64_le);
    configure_response.endpoint_id = endpoint;
    configure_response.cluster_id  = clusterId;
    configure_response.frame.size  = attribute_status_records_size;

    sl_log_debug(LOG_TAG,
                 "Read Attribute Response from EUI64 %016:" PRIX64
                 " endpoint: %u cluster: 0x%04X",
                 zigbee_eui64_to_uint(configure_response.eui64),
                 endpoint,
                 clusterId);

    memcpy(configure_response.frame.buffer,
           attribute_status_records,
           sizeof(uint8_t) * configure_response.frame.size);

    status = zigpc_observable_notify(
      &zigpc_gateway_observable,
      ZIGPC_GATEWAY_NOTIFY_CONFIGURE_REPORTING_RESPONSE,
      (void *)&configure_response);
    if (status != SL_STATUS_OK) {
      sl_log_warning(LOG_TAG, "CONFIGURE_RESPONSE failed: 0x%X", status);
    }
  }
}

EmberAfStatus zigpc_gateway_hdl_on_cmd_received(const EmberEUI64 eui64_le,
                                                uint8_t endpoint,
                                                uint16_t clusterId,
                                                uint8_t commandId,
                                                bool commandFromServerToClient,
                                                const uint8_t *buffer,
                                                uint16_t bufferLength,
                                                uint8_t bufferPayloadStartIndex)
{
  sl_status_t status                                          = SL_STATUS_OK;
  static zigpc_gateway_on_command_received_t command_received = {0};

  // Set default ZCL status in case there are no listeners
  command_received.return_status = ZIGPC_ZCL_STATUS_UNSUPPORTED_CLUSTER;

  if (bufferLength > ZCL_FRAME_BUFFER_SIZE_MAX) {
    status = SL_STATUS_WOULD_OVERFLOW;
    sl_log_error(LOG_TAG,
                 "Cannot copy over command that is bigger than ZCL frame"
                 " size allocated: 0x%04X",
                 status);
  }

  if (status == SL_STATUS_OK) {
    zigbee_eui64_copy_switch_endian(command_received.eui64, eui64_le);
    sl_log_debug(LOG_TAG,
                 "Received command from EUI64:%016" PRIX64
                 " endpoint: %u cluster: 0x%04X "
                 "command: 0x02X",
                 zigbee_eui64_to_uint(command_received.eui64),
                 endpoint,
                 clusterId,
                 commandId);

    command_received.endpoint_id           = endpoint;
    command_received.cluster_id            = clusterId;
    command_received.command_id            = commandId;
    command_received.from_server_to_client = commandFromServerToClient;
    command_received.frame.size            = bufferLength;
    command_received.frame_payload_offset  = bufferPayloadStartIndex;
    memcpy(command_received.frame.buffer,
           buffer,
           sizeof(uint8_t) * bufferLength);

    //TODO add in poll control server to zcl_profiles to read data
    if ((command_received.cluster_id == 0x0020)
        && (command_received.command_id == 0x00)) {
      zigpc_gateway_unload_sleepy_messages(command_received.eui64);
    }

    status = zigpc_observable_notify(&zigpc_gateway_observable,
                                     ZIGPC_GATEWAY_NOTIFY_ZCL_COMMAND_RECEIVED,
                                     (void *)&command_received);
    if (status != SL_STATUS_OK) {
      sl_log_warning(
        LOG_TAG,
        "Sending ZIGPC_GATEWAY_NOTIFY_ZCL_COMMAND_RECEIVED failed: 0x%X",
        status);
    }
  }

  return (EmberAfStatus)command_received.return_status;
}

void zigpc_gateway_hdl_on_ota_update_started(const EmberEUI64 eui64,
                                             uint16_t manufacturerId,
                                             uint16_t imageTypeId,
                                             uint32_t firmwareVersion)
{
  sl_status_t status;
  static zigpc_gateway_on_ota_started_t ota_event;

  zigbee_eui64_copy_switch_endian(ota_event.eui64, eui64);
  ota_event.manufacturer_id  = manufacturerId;
  ota_event.image_type_id    = imageTypeId;
  ota_event.firmware_version = firmwareVersion;

  sl_log_debug(LOG_TAG,
               "Received OTA update start event from EUI64 %016" PRIX64,
               zigbee_eui64_to_uint(ota_event.eui64));

  status = zigpc_observable_notify(&zigpc_gateway_observable,
                                   ZIGPC_GATEWAY_NOTIFY_OTA_UPDATE_STARTED,
                                   (void *)&ota_event);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "OTA_UPDATE_STARTED notify failed: 0x%X", status);
  }
}

void zigpc_gateway_hdl_on_ota_update_completed(const EmberEUI64 eui64,
                                               uint16_t manufacturerId,
                                               uint16_t imageTypeId,
                                               uint32_t firmwareVersion,
                                               EmberAfStatus otaStatus)
{
  sl_status_t status;
  static zigpc_gateway_on_ota_completed_t ota_event;

  zigbee_eui64_copy_switch_endian(ota_event.eui64, eui64);
  ota_event.manufacturer_id  = manufacturerId;
  ota_event.image_type_id    = imageTypeId;
  ota_event.firmware_version = firmwareVersion;
  ota_event.status           = otaStatus;

  sl_log_debug(LOG_TAG,
               "Received OTA update complete event from EUI64 %016" PRIX64,
               zigbee_eui64_to_uint(ota_event.eui64));

  status = zigpc_observable_notify(&zigpc_gateway_observable,
                                   ZIGPC_GATEWAY_NOTIFY_OTA_UPDATE_COMPLETED,
                                   (void *)&ota_event);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "OTA_UPDATE_COMPLETED notify failed: 0x%X", status);
  }
}

struct zigbeeHostCallbacks zigpc_gateway_zigbee_host_callbacks = {
  .onEmberAfStackInitalized        = NULL,
  .onEmberAfNcpPreReset            = zigpc_gateway_on_ncp_pre_reset,
  .onEmberAfNcpPostReset           = zigpc_gateway_on_ncp_post_reset,
  .onNetworkInitialized            = zigpc_gateway_hdl_on_network_initialized,
  .onNetworkDeviceJoin             = zigpc_gateway_hdl_on_node_add_start,
  .onTrustCenterDeviceJoinComplete = zigpc_gateway_hdl_on_node_add_complete,
  .onNetworkDeviceLeaveResponse    = zigpc_gateway_hdl_on_node_removed,
  .onZdoActiveEndpointsResponse
  = zigpc_gateway_hdl_on_device_endpoints_discovered,
  .onZdoSimpleDescriptorResponse
  = zigpc_gateway_hdl_on_endpoint_clusters_discovered,
  .onReportedAttributeChange    = zigpc_gateway_hdl_on_reported_attribute,
  .onReadAttributesResponse     = zigpc_gateway_hdl_on_read_attributes,
  .onConfigureReportingResponse = zigpc_gateway_hdl_on_configure_response,
  .onClusterCommandReceived     = zigpc_gateway_hdl_on_cmd_received,
  .onOtaUpdateStarted           = zigpc_gateway_hdl_on_ota_update_started,
  .onOtaUpdateCompleted         = zigpc_gateway_hdl_on_ota_update_completed,
};
