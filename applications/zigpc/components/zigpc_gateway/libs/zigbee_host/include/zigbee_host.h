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
 * @file zigbee_host.h
 * @defgroup zigbee_host ZigbeeHost Component
 * @ingroup zigpc_components
 * @brief Wrapper command and callback functions setup for ZigbeeHost
 *
 * @{
 */

#ifndef ZIGBEE_HOST_H
#define ZIGBEE_HOST_H

#include <stdbool.h>

#include "zigbee_host_callbacks.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Options setup to establish communications with ZigbeeHost
 */
struct zigbeeHostOpts {
  char *serialPort;
  char *otaPath;
  struct zigbeeHostCallbacks *callbacks;
  const uint16_t *supportedClusterList;
  unsigned int supportedClusterListSize;
};

/**********************************/
/* Housekeeping API               */
/**********************************/

/**
 * @brief Initialize the ZigbeeHost framework.
 *
 * @param opts  options to be passed
 *
 * @return true if initialization is successful, false otherwise.
 */
int zigbeeHostInit(struct zigbeeHostOpts *opts);

/**
 * @brief Call the ZigbeeHost tick function.
 *
 */
void zigbeeHostTick(void);

/**
 * @brief Perform cleanup routines for ZigbeeHost.
 *
 */
void zigbeeHostShutdown(void);

/**********************************/
/* Command API                    */
/**********************************/

/**
 * @brief Retrieve EUI64 of the gateway.
 *
 * @param eui64 Reference to EUI64 to populate
 */
void zigbeeHostGetEui64(sl_802154_long_addr_t eui64);

/**
 * @brief Retrieve the primary endpointID used by the gateway.
 *
 * @return uint8_t Endpoint identifiter.
 */
uint8_t zigbeeHostGetPrimaryEndpointId(void);

/**
 * @brief Invoke the emberAf info command. This command will print network
 * and device level information to the console.
 *
 */
void zigbeeHostCommandPrintInfo(void);

/**
 * @brief Sends a command to the geck-sdk
 *
 * @param array Null terminated array containing the command
 */
void zigbeeHostSendSlCliCommand(char* array);

/**
 * @brief Populate the list of counters plugin entries into the list paseed
 * in.
 *
 * @param list          Destination list to copy counters data to.
 * @param count         Length of destination list provided.
 * @return sl_status_t  Always returns SL_STATUS_OK.
 */
sl_status_t zigbeeHostGetCountersList(uint16_t *const list, size_t count);

/**
 * @brief Clears the counters of counter plugin
 *
 */
void zigbeeHostClearCounters(void);

/**
 * @brief get count of active neighbors
 *
 * @return uint8_t number of active neighbors in the neighbors adresse table
 */
uint8_t zigbeeHostGetNeighborCount();

/**
 * @brief Get a specific neighbor from the neighbor address table
 *
 * @param index index of the neighbor
 * @param eui64 Destination eui64 to copy data to
 * @return sl_status_t
 */
sl_status_t zigbeeHostGetNeighborEUI64(uint8_t index,sl_802154_long_addr_t eui64);

/**
 * @brief Initialize the Trust Center network
 *
 * @return sl_status_t SL_STATUS_OK or error
 */
sl_status_t zigbeeHostTrustCenterInit(void);

sl_status_t zigbeeHostTrustCenterInitWithArgs(
        uint16_t pan_id,
        int8_t radio_tx_power,
        uint8_t channel);

/**
 * @brief Add new EUI64 to NodeId mapping to the EmberAf address-table plugin.
 *
 * NOTE: The address-table is stored on RAM in the NCP (where the NodeId is
 * retrieved/set). Therefore on NCP reset, the address-table on the NCP will
 * be cleared.
 *
 * @param eui64         Device identifier
 * @param nodeId        Device network-based identifier
 * @return sl_status_t SL_STATUS_OK on successfully adding mapping,
 * SL_STATUS_INVALID_PARAMETER on any invalid arguments provided,
 * SL_STATUS_INVALID_STATE if the EUI64 is not tracked by the address-table plugin.
 */
sl_status_t zigbeeHostAddAddressTableEntry(const sl_802154_long_addr_t eui64,
                                           const sl_802154_short_addr_t nodeId);

/**
 * @brief Retrieve the NodeId for the EUI64 provided from the address-table
 * plugin.
 *
 * @param eui64         Device identifier to lookup.
 * @param nodeId        Reference to network-based identifier to populate,
 *                      SL_ZIGBEE_UNKNOWN_NODE_ID populated if not found.
 * @return sl_status_t  SL_STATUS_OK on successfully retrieving NodeId,
 * SL_STATUS_INVALID_PARAMETER on any invalid arguments provided,
 * SL_STATUS_INVALID_STATE if the EUI64 is not tracked by the address-table plugin.
 */
sl_status_t zigbeeHostGetAddressTableEntry(const sl_802154_long_addr_t eui64,
                                           sl_802154_short_addr_t *const nodeId);

/**
 * @brief Allow devices to join the gateway network
 *
 * @param openForever  Indicate whether to open the network indefinitely.
 * @return sl_status_t  SL_STATUS_OK on opening the network.
 *                      SL_STATUS_NETWORK_DOWN if the network is not initialized.
 */
sl_status_t zigbeeHostTrustCenterJoinOpen(bool openForever);

/**
 * @brief Stop devices from joining the gateway network
 *
 * @return sl_status_t  SL_STATUS_OK on successfully closing the network.
 *                      SL_STATUS_NETWORK_DOWN if the network is not initialized.
 */
sl_status_t zigbeeHostTrustCenterJoinClose(void);

/**
 * @brief Retrieve the Trust Center well-known link key.
 *
 * @return sl_zigbee_key_data_t
 */
sl_zigbee_key_data_t zigbeeHostGetTrustCenterWellKownKey(void);

/**
 * @brief Add link key to either to the regular or transient key table.
 *
 * @param eui64             Device that will be using the key (or wildcard).
 * @param key               Link key data
 * @param inTransientTable  Store key in either the regular or transient table.
 * @return sl_status_t      SL_STATUS_OK on success, or EZSP-related error
 * occured when requesting key to be added.
 */
sl_status_t zigbeeHostTrustCenterAddLinkKey(const sl_802154_long_addr_t eui64,
                                            const sl_zigbee_key_data_t *key,
                                            bool inTransientTable);

/**
 * @brief Validate if install code passed is valid and can be converted
 * into a link key.
 *
 * @param installCode         Install code data.
 * @param installCodeLength   Install code length.
 * @return true               Valid and can be converted to Link key.
 * @return false              Cannot be converted to Link key.
 */
bool zigbeeHostTrustCenterInstallCodeValid(const uint8_t *installCode,
                                           uint8_t installCodeLength);

/**
 * @brief Add a device to the network using the Z3 Install Code method.
 *
 * @param eui64               Identifier of an end device
 * @param installCode         Install code from the end device
 * @param installCodeLength   Length of the passed install code
 * @return sl_status_t  SL_STATUS_OK or error
 */
sl_status_t zigbeeHostTrustCenterAddDeviceInstallCode(
  const sl_802154_long_addr_t eui64, uint8_t *installCode, uint8_t installCodeLength);

/**
 * @brief Remove a device that already exists on the network. This call uses
 * the EmberAf network leave request API. if the device is not
 * currently tracked by the address-table, SL_STATUS_NOT_FOUND  will be returned.
 *
 * @param eui64         End device identifier
 * @return sl_status_t  SL_STATUS_OK when the Leave Request is sent successfully,
 * SL_STATUS_NOT_FOUND  if the device is not found via the address-table plugin.
 */
sl_status_t zigbeeHostNetworkDeviceLeaveRequest(const sl_802154_long_addr_t eui64);

/**
 * @brief Send ZDO Active Endpoints Descriptor request to device on the network.
 * This function will send a ZDO message to the target device. The callback
 * @ref onZdoActiveEndpointsResponse is trigged for successful responses
 * to this request.
 *
 * @param eui64         Device identifier to probe.
 * @return sl_status_t  SL_STATUS_OK on successfully sending request, error
 * otherwise.
 */
sl_status_t zigbeeHostZdoActiveEndpointsRequest(const sl_802154_long_addr_t eui64);

/**
 * @brief Send ZDO Simple Descriptor request to device on the network.
 * This function will send a ZDO message to the target device. The callback
 * @ref onZdoSimpleDescriptorResponse is trigged for successful responses
 * to this request.
 *
 * @param eui64         Device identifier to probe.
 * @param endpointId    Endpoint identifier to probe.
 * @return sl_status_t  SL_STATUS_OK on successfully sending request, error
 * otherwise.
 */
sl_status_t zigbeeHostZdoSimpleDescriptorRequest(const sl_802154_long_addr_t eui64,
                                                 uint8_t endpointId);

/**
 * @brief Populate ZigbeeHost ZCL frame to be sent on the network. This function
 * will copy the supplied buffer to an internal buffer. Along with the buffer,
 * an offset to the sequence ID should be supplied as this function will fill
 * in the correct Sequence ID at the index supplied.
 *
 * @param buffer              ZCL frame buffer containing additional data.
 * @param bufferSize          Size of the ZCL frame buffer.
 * @param bufferSequenceIndex Offset of sequence ID in the frame buffer to be filled.
 *
 * @return the status of the operation: SL_STATUS_OK if ZCL Frame is populated
 * successfully, SL_STATUS_INVALID_PARAMETER if any inputs are invalid,
 * SL_STATUS_INVALID_INDEX  if the buffer size does not meet the maximum
 * and minimum limits.
**/
sl_status_t zigbeeHostFillZclFrame(const uint8_t *buffer,
                                   size_t bufferSize,
                                   size_t bufferSequenceIndex);

/**
 * @brief Send the internal ZigbeeHost ZCL frame to an end device on the network
 * via unicast message to the device that has the matching
 * EUI64/endpoint/cluster identifiers.
 *
 * NOTE: This function assumes that \ref zigbeeHostFillZclFrame has been called
 * prior to fill in a ZCL message/frame.
 *
 * @param eui64     Identifier of the Zigbee end device.
 * @param endpoint  Identifier of the endpoint on the Zigbee end device.
 * @param clusterId Identifier corresponding to the ZCL cluster
 *                  (ex. 0x0006 <--> OnOff).
 *
 * @return the status of the operation: SL_STATUS_OK if message is sent
 * successfully, SL_STATUS_INVALID_PARAMETER if any inputs are invalid,
 * or SL_STATUS_NOT_FOUND  if the end device EUI64 is not found.
**/
sl_status_t zigbeeHostSendZclFrameUnicast(const sl_802154_long_addr_t eui64,
                                          uint8_t endpoint,
                                          uint16_t clusterId);

/**
 * @brief Send the internal ZigbeeHost ZCL frame via a multicast message to the
 * devices that has the matching multicastId in their ZCL Groups table.
 *
 * NOTE: This function assumes that \ref zigbeeHostFillZclFrame has been called
 * prior to fill in a ZCL message/frame.
 *
 * @param multicastId Identifier of Multicast ID used for the message.
 * @param clusterId   Identifier corresponding to the ZCL cluster
 *                    (ex. 0x0006 <--> OnOff).
 *
 * @return the status of the operation: SL_STATUS_OK if message is sent
 * successfully, SL_STATUS_INVALID_PARAMETER if any inputs are invalid.
**/
sl_status_t zigbeeHostSendZclFrameMulticast(sl_zigbee_multicast_id_t multicastId,
                                            uint16_t clusterId);
/**
 * @brief Send a ZCL Configure Reporting Message to an end device on the
 * network. This function will copy the supplied report record buffer to an
 * internal buffer before sending a unicast message to the device that has the
 * matching EUI64/endpoint/cluster identifiers.
 *
 * @param eui64               Identifier of the Zigbee end device
 * @param endpoint            Identifier of the endpoint on the Zigbee end device.
 * @param clusterId           Identifier corresponding to the ZCL cluster.
 * @param reportRecord        ZCL Configure Reporting Record buffer.
 * @param recordSize          Size of the ZCL Configure Reporting Record.
 * @return sl_status_t        SL_STATUS_OK if message is sent successfully,
 * SL_STATUS_NOT_FOUND  if the end device EUI64 is not found.
 */
sl_status_t zigbeeHostInitReporting(const sl_802154_long_addr_t eui64,
                                    uint8_t endpoint,
                                    uint16_t clusterId,
                                    const uint8_t *reportRecord,
                                    size_t recordSize);
/**
 * @brief Send a Zigbee Bind Request message. Required for reports and other
 * zigbee-specific management options. Can be used to configure unicast or multicast
 * bindings. If a unicast binding, groupId should be set to 0
 *
 * @param sourceEui64         Identifier of the Zigbee end device
 * @param sourceEndpoint      Identifier of the endpoint on the Zigbee end device.
 * @param clusterId           Identifier corresponding to the ZCL cluster.
 * @param groupId             Identifier for the groupId (if multicast binding)
 * @param destEui64           Destination address for the binding
 * @param destEndpoint        Destination endpoint for the binding
 * @param isBindRequest       True if a binding request, otherwise an unbinding request
 * @return sl_status_t        SL_STATUS_OK if message is sent successfully,
 * SL_STATUS_NOT_FOUND  if the end device EUI64 is not found.
 */
sl_status_t zigbeeHostInitBinding(const sl_802154_long_addr_t sourceEui64,
                                  uint8_t sourceEndpoint,
                                  uint16_t clusterId,
                                  uint16_t groupId,
                                  sl_802154_long_addr_t destEui64,
                                  uint8_t destEndpoint,
                                  bool isBindRequest);

/**
 * @brief zigbeeHostSendPollingCheckInResponse
 * Send a CheckInResponse to a given poll control server
 * By default, sets it to fast poll with the default timeout
 *
 * @return SL_STATUS_OK if the response was sent
**/
sl_status_t zigbeeHostSendPollingCheckInResponse(bool startFastPolling);

/**
 * @brief zigbeeHostAddOtaImage
 * Manually adds an OtaImage to the ota-server plugin
 *
 * @param filename - the filename of the OTA Image
 *
 * @return SL_STATUS_OK if Image was added successfully.
 * Fails if the ota-storage plugin was unable to parse
 * the filename correctly with SL_STATUS_FAIL
**/
sl_status_t zigbeeHostAddOtaImage(const char *filename);

/**
 * @brief Populate Ember key data specified by the type.
 *
 * @param type          Ember key type to get.
 * @param key           reference where data will be populated.
 * @return sl_status_t  SL_STATUS_OK on success, or error otherwise.
 */
sl_status_t zigbeeHostGetEmberKey(sl_zigbee_key_type_t type, sl_zigbee_key_struct_t *const key);

/**
 * @brief Set EZSP policy on the ZigPC NCP.
 *
 * @param policyId      Type of policy to apply.
 * @param decisionId    Policy variant to apply.
 * @param policyName    Descriptive name of policy.
 * @param decisionName  Descruptive label of decision to be set.
 * @return sl_zigbee_ezsp_status_t   SL_STATUS_OK on success, or EZSP-related error
 * otherwise.
 */
sl_zigbee_ezsp_status_t zigbeeHostSetEzspPolicy(sl_zigbee_ezsp_policy_id_t policyId,
                                   sl_zigbee_ezsp_decision_id_t decisionId,
                                   const char *policyName,
                                   const char *decisionName);

sl_status_t zigbeeHostLaunchBootloader();
#ifdef __cplusplus
}
#endif

#endif  //ZIGBEE_HOST_H

/** @} end zigbee_host */
