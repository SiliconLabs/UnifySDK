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

/**
 * @file zigbee_host_callbacks.h
 * @defgroup zigbee_host_callbacks ZigbeeHost Callbacks
 * @ingroup zigbee_host
 * @brief Callbacks used for listening to events from ZigbeeHost
 *
 * @{
 */
#ifndef ZIGBEE_HOST_CALLBACKS_H
#define ZIGBEE_HOST_CALLBACKS_H

#include <stdbool.h>

// zigbee stack includes
#include <app/util/ezsp/ezsp-enum.h>
#include <stack/include/ember-types.h>
#include <app/framework/include/af-types.h>

// slc autogen includes
#include <zap-type.h>

/**
 * @brief The following structure holds all the callbacks ZigbeeHost will use
 * to notify of events from ZigbeeHost. All of the following callbacks assume
 * the ZigbeeHost house-keeping APIs (@ref zigbeeHostInit and @ref zigbeeHostTick)
 * are called appropriately.
 */
struct zigbeeHostCallbacks {
  /**
   * @brief This callback is invoked when the Ember Application Framework has
   * initialized the stack layer (including any NCP init routines).
   */
  void (*onEmberAfStackInitalized)(void);

  /**
   * @brief This callback is invoked before the NCP is reset. This occurs after
   * the host process detects an error via the EZSP link from the NCP.
   *
   * @param resetStatus   EZSP error status received from the NCP.
   */
  void (*onEmberAfNcpPreReset)(EzspStatus resetStatus);

  /**
   * @brief This callback is invoked to configure the NCP after it is reset.
   * This occurs either at startup or after EZSP error-based reset.
   * 
   * NOTE: The function is always called twice when the NCP is reset. First
   * with ncpMemConfigureStage=TRUE and then with
   * ncpMemConfigureStage=FALSE.
   * 
   * @param ncpMemConfigureStage   TRUE if ONLY memory
   * configurable EZSP commands are accepted by the NCP, FALSE if only
   * non-memory configurable EZSP commands can be sent to the NCP.
   */
  void (*onEmberAfNcpPostReset)(bool ncpMemConfigureStage);

  /**
   * @brief This callback is invoked when the network has initialized on the
   * Gateway. If the network is down, the network-creator plugin is used to
   * form the Zigbee network and the complete callback of this plugin is used
   * to invoked this callback. If the network is already up, the network
   * information is retrieved from the Gateway and passed through this
   * callback. Users should wait for this callback before performing operations
   * on the Zigbee network. If this callback is not invoked within an appropriate
   * delay (TODO: Insert benchmarking values), it is safe to assume that network
   * initialization has failed.
   *
   * NOTE: This callback is invoked as a response to calling
   * @ref zigbeeHostTrustCenterInit.
   *
   * @param network   Current network parameters
   *
   */
  void (*onNetworkInitialized)(const EmberNetworkParameters *network);

  /**
   * @brief This callback is invoked when a device has started joining the
   * Gateway's network. The start of a join/association process for a Zigbee
   * device is marked it uses the initial joining Link Key to join the network.
   *
   * NOTE: This callback is invoked only when the network is open for joining.
   *
   * @param eui64     EUI of joining device
   *
   */
  void (*onNetworkDeviceJoin)(const EmberEUI64 eui64);

  /**
   * @brief This callback is invoked when a device has completed joining the
   * Gateway's network. The end of a join/association process for a Zigbee
   * device is marked when the replacement Link Key is generated and exchanged
   * with the joining device is verified by the Gateway. If this callback is
   * not invoked within an appropriate delay (TODO: Insert benchmarking
   * values), it is safe to assume that device join has failed.
   *
   * NOTE: This callback is invoked as a response to calling
   * @ref zigbeeHostTrustCenterAddDeviceInstallCode.
   *
   * @param eui64     EUI of joined device
   *
   */
  void (*onTrustCenterDeviceJoinComplete)(const EmberEUI64 eui64);

  /**
   * @brief This callback is invoked when the Gateway has successfully removed
   * the device by listening on trust center leave events. Users should
   * assume that the removed device was once part of the network. If this callback
   * is not invoked within an appropriate delay (TODO: Insert benchmarking
   * values), it is safe to assume that device removal process has failed.
   *
   * NOTE: This callback is invoked as a response to calling
   * @ref zigbeeHostNetworkDeviceLeaveRequest.
   *
   * @param eui64         EUI of discovered device in little-endian
   */
  void (*onNetworkDeviceLeaveResponse)(const EmberEUI64 eui64);

  /**
   * @brief This callback is invoked when the Gateway has received a ZDO Active
   * Endpoints Descriptor response from a device on the network. This callback
   * is invoked as a response to calling
   * @ref zigbeeHostZdoActiveEndpointDescRequest. If this callback is not
   * invoked within an reasonable delay (TODO: Insert benchmarking values),
   * it is safe to assume the ZDO Active Endpoints Descriptor Request failed
   * to be processed by the sending device.
   *
   * TODO: Discovery restart not supported yet. The device should leave and
   * rejoin the network in order for the device discovery to start again.
   *
   * @param eui64         Discovered device identifier.
   * @param endpointCount Total number of active endpoints discovered.
   * @param endpointCount Endpoint Identifier list.
   */
  void (*onZdoActiveEndpointsResponse)(const EmberEUI64 eui64,
                                       uint8_t endpointCount,
                                       const uint8_t *endpointList);

  /**
   * @brief This callback is invoked when the Gateway has received a ZDO simple
   * descriptor response from a device on the network. This callback is invoked
   * as a response to calling @ref zigbeeHostZdoSimpleDescriptorRequest. If this
   * callback is not invoked within an reasonable delay (TODO: Insert
   * benchmarking values), it is safe to assume the ZDO Simple Descriptor
   * Request failed to be processed by the sending device.
   *
   * TODO: Discovery restart not supported yet. The device should leave and
   * rejoin the network in order for the device discovery to start again.
   *
   *
   * @param eui64         Device identifier.
   * @param endpointInfo  Discovered endpoint information.
   */
  void (*onZdoSimpleDescriptorResponse)(const EmberEUI64 eui64,
                                        const EmberAfClusterList *endpointInfo);

  /**
   * @brief This callback is invoked when the Gateway receives a ZCL global
   * report attribute change message. The response will contain a message
   * buffer consisting of one or more attribute report records. Refer to
   * ZCL specification section "Attribute Report Response Command" for more
   * information.
   *
   * @param eui64                         EUI of device where attributes were read from.
   * @param endpointId                    Endpoint identifer of device.
   * @param clusterId                     ZCL Cluster ID of attributes.
   * @param attribute_report_records      Buffer of attribute report records.
   * @param attribute_report_records_size Size of report records buffer.
   */
  void (*onReportedAttributeChange)(const EmberEUI64 eui64,
                                    uint8_t endpoint,
                                    uint16_t clusterId,
                                    uint8_t *attribute_report_records,
                                    uint16_t attribute_report_records_size);

  /**
   * @brief This callback is invoked when the Gateway receives a ZCL global
   * read attribute response message. The response will contain a message
   * buffer consisting of one or more read attribute status records. Refer to
   * ZCL specification section "Read Attributes Response Command" for more
   * information.
   *
   * @param eui64                         EUI of device where attributes were read from.
   * @param endpointId                    Endpoint identifer of device.
   * @param clusterId                     ZCL Cluster ID of attributes.
   * @param attribute_status_records      Buffer of attribute status records.
   * @param attribute_status_records_size Size of status records buffer.
   * @return EmberStatus                  EMBER_SUCCESS if frame is being
   * handled by a supported cluster, EMBER_NOT_FOUND if the command frame
   * parsing is not supported.
   */
  void (*onReadAttributesResponse)(const EmberEUI64 eui64,
                                   uint8_t endpoint,
                                   uint16_t clusterId,
                                   uint8_t *attribute_status_records,
                                   uint16_t attribute_status_records_size);

  /**
   * @brief This callback is invoked when the Gateway receives a ZCL global
   * configure reporting response message. The response will contain a message
   * buffer consisting of one or more configure reporting status records. Refer to
   * ZCL specification section "Configure Reporting Response Command" for more
   * information.
   *
   * @param eui64                         EUI of source device of ZCL message
   * @param endpointId                    Endpoint identifer of device.
   * @param clusterId                     ZCL Cluster ID of attributes.
   * @param configure_status_records      Buffer of configure status records.
   * @param configure_status_records_size Size of status records buffer.
   *
   * @return EmberStatus                  EMBER_SUCCESS if frame is being
   * handled by a supported cluster, EMBER_NOT_FOUND if the command frame
   * parsing is not supported.
   */
  void (*onConfigureReportingResponse)(const EmberEUI64 eui64,
                                       uint8_t endpoint,
                                       uint16_t clusterId,
                                       uint8_t *configure_status_records,
                                       uint16_t configure_status_records_size);

  /**
   * @brief This callback is invoked when the Gateway receives a ZCL cluster
   * command from a node on the network. The response contains a message buffer
   * consisting of the command payload. Refer to the ZCL specification section
   * "General ZCL Frame Format" for more information.
   *
   * @param eui64                   EUI of source device.
   * @param endpointId              Endpoint identifer of source device.
   * @param clusterId               ZCL Cluster identifier.
   * @param commandId               ZCL Cluster command identifier.
   * @param buffer                  Buffer containing command frame.
   * @param bufferLength            Buffer containing command frame.
   * @param bufferPayloadStartIndex Index indicating start of command payload.
   * @return EmberAfStatus          Status of the command processing.
   * EMBER_ZCL_STATUS_SUCCESS if the command was handled properly,
   * EMBER_ZCL_STATUS_UNSUP_COMMAND if the command is not supported,
   * EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER if the cluster is not supported.
   */
  EmberAfStatus (*onClusterCommandReceived)(const EmberEUI64 eui64,
                                            uint8_t endpoint,
                                            uint16_t clusterId,
                                            uint8_t commandId,
                                            bool commandFromServerToClient,
                                            const uint8_t *buffer,
                                            uint16_t bufferLength,
                                            uint8_t bufferPayloadStartIndex);

  /**
   * @brief This callback is invoked when the ota-server plugin determines an
   * OTA update has started.
   *
   * NOTE: The ota-server invokes this callback assuming only one OTA update
   * is being performed. If ota-server services multiple OTA sessions (1 per
   * device) simultaneously, it will switch between each session and invoke
   * this callback at the when it switches to a new session. Therefore, users
   * of this function should be prepared to have this function called for a
   * device multiple times during an OTA trasaction.
   *
   * @param eui64           Target Device.
   * @param manufacturerId  Device Manufacturer ID.
   * @param imageTypeId     OTA Image type.
   * @param firmwareVersion OTA Image FW version.
   */
  void (*onOtaUpdateStarted)(const EmberEUI64 eui64,
                             uint16_t manufacturerId,
                             uint16_t imageTypeId,
                             uint32_t firmwareVersion);
  /**
   * @brief This callback is invoked when the ota-server plugin determines an
   * OTA update has completed.
   *
   * NOTE: This callback is also invoked when a OTA client requests an image
   * block and the server does not have an image available. The ZCL status
   * EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE is sent in this case.
   *
   *
   * @param eui64           Target Device.
   * @param manufacturerId  Device Manufacturer ID.
   * @param imageTypeId     OTA Image type.
   * @param firmwareVersion OTA Image FW version.
   * @param status          Status of the OTA update:
   * EMBER_ZCL_STATUS_SUCCESS if the update is performed successfully.
   * EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE if there is not image available for the
   * device requesting information.
   */
  void (*onOtaUpdateCompleted)(const EmberEUI64 eui64,
                               uint16_t manufacturerId,
                               uint16_t imageTypeId,
                               uint32_t firmwareVersion,
                               EmberAfStatus status);

  void (*onBindUnbindResponse)( const EmberEUI64 sourceEui64,
                                uint8_t sourceEndpoint,
                                uint16_t clusterId,
                                const EmberEUI64 destEui64,
                                uint8_t destEndpoint,
                                bool isBindResponse,
                                uint8_t zdoStatus);
};

#endif  //ZIGBEE_HOST_CALLBACKS_H

/** @} end zigbee_host_callbacks */
