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
 * @file z3gateway.h
 * @defgroup z3gateway Z3Gateway Component
 * @ingroup zigpc_components
 * @brief Wrapper command and callback functions setup for Z3Gateway
 *
 * @{
 */

#ifndef Z3GATEWAY_H
#define Z3GATEWAY_H

#include "z3gateway_callbacks.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Options setup to establish communications with Z3Gateway
 */
struct z3gatewayOpts {
  char *serialPort;
  struct z3gatewayCallbacks *callbacks;
};

/**
 * @brief Structure used to import/export the Z3Gateway device-table plugin
 * state.
 *
 * TODO: Keep this structure maintained with the device-table plugin state
 * information normally persisted to a file.
 */
struct z3gatewayDeviceTableStorageEntry {
  EmberEUI64 eui64;      /**< EUI64 */
  EmberNodeId nodeId;    /**< Short ID */
  uint8_t endpoint;      /**< Endpoint */
  uint16_t deviceId;     /**< device ID */
  size_t clusterIdCount; /**< Element count of cluster ID list */
  uint8_t
    clusterIdClientStartIndex; /**< Index marking end of server clusters */
  /**
   * @brief Pointer to list of cluster IDs (server clusters then client
   * clusters)
   */
  uint16_t *clusterIdList;
};

/**********************************/
/* Housekeeping API               */
/**********************************/

/**
 * @brief Initialize the Z3Gateway framework.
 *
 * @param opts  options to be passed
 *
 * @return true if initialization is successful, false otherwise.
 */
int z3gatewayInit(struct z3gatewayOpts *opts);

/**
 * @brief Call the Z3Gateway tick function.
 *
 */
void z3gatewayTick(void);

/**
 * @brief Perform cleanup routines for Z3Gateway.
 *
 */
void z3gatewayShutdown(void);

/**********************************/
/* Command API                    */
/**********************************/

/**
 * @brief Invoke the emberAf info command. This command will print network
 * and device level information to the console.
 *
 */
void z3gatewayCommandPrintInfo(void);

/**
 * @brief Populate the list of counters plugin entries into the list paseed
 * in.
 *
 * @param list          Destination list to copy items to
 * @return EmberStatus  Always returns EMBER_SUCCESS.
 */
EmberStatus z3gatewayGetCountersList(uint16_t list[EMBER_COUNTER_TYPE_COUNT]);

/**
 * @brief Clears the counters of counter plugin
 * 
 */
void z3gatewayClearCounters();

/**
 * @brief Initialize the Trust Center network
 *
 * @return EmberStatus EMBER_SUCCESS or error
 */
EmberStatus z3gatewayTrustCenterInit(void);

/**
 * @brief Clear the existing device-table state.
 *
 * @return EmberStatus Always returns EMBER_SUCCESS.
 */
EmberStatus z3gatewayDeviceTableClear(void);

/**
 * @brief Get the number of z3gatewayDeviceTableStorageEntry needed to store
 * the device-table state.
 *
 * NOTE: Currently this returned value is fixed based on
 * EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE.
 *
 * @return uint16_t     Number of device-table entries.
 */
uint16_t z3gatewayDeviceTableCount(void);

/**
 * @brief Get the current count of cluster list used by the the device-table
 * entry based on index.
 *
 * NOTE: Currently this returned value is fixed based on
 * EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE.
 *
 * @param entryIndex  index of the device-table entry
 *
 * @return size_t     Size of device-table cluster list for the index.
 */
size_t z3gatewayDeviceTableEntryClusterListCount(uint16_t entryIndex);

/**
 * @brief Clear and populate the device-table plugin state used by Z3Gateway.
 *
 * NOTE: This operation should be performed before calling z3gatewayTick()
 * as the behavior is not predictable if called after.
 *
 * @param entryList     Pointer to list of device-table entries to populate with.
 * @param entryIndex    Index of the device-table to import to.
 * @return EmberStatus  EMBER_SUCCESS if successfully imported,
 * EMBER_BAD_ARGUMENT if invalid parameters passed in, or
 * EMBER_INDEX_OUT_OF_RANGE if the entry passed in is too large to store in
 * plugin table.
 */
EmberStatus z3gatewayDeviceTableImportEntry(
  const struct z3gatewayDeviceTableStorageEntry *const entry,
  const uint16_t entryIndex);

/**
 * @brief Export the device-table plugin state used by Z3Gateway into a
 * destination list provided.
 *
 * @param entryList     Pointer to list of device-table entries to copy to.
 * @param entryIndex    Index of the device-table to export.
 * @return EmberStatus  EMBER_SUCCESS if successfully exported, or
 * EMBER_BAD_ARGUMENT if invalid pointer is passed in, or
 * EMBER_INDEX_OUT_OF_RANGE if the entry passed in is insufficent to exprot to.
 */
EmberStatus z3gatewayDeviceTableExportEntry(
  struct z3gatewayDeviceTableStorageEntry *const entry,
  const uint16_t entryIndex);

/**
 * @brief Allow devices to join the gateway network
 *
 * @param openForever  Indicate whether to open the network indefinitely.
 * @return EmberStatus  EMBER_SUCCESS on opening the network.
 *                      EMBER_NETWORK_DOWN if the network is not initialized.
 */
EmberStatus z3gatewayTrustCenterJoinOpen(bool openForever);

/**
 * @brief Stop devices from joining the gateway network
 *
 * @return EmberStatus  EMBER_SUCCESS on successfully closing the network.
 *                      EMBER_NETWORK_DOWN if the network is not initialized.
 */
EmberStatus z3gatewayTrustCenterJoinClose(void);

/**
 * @brief Add a device to the network using the Z3 Install Code method.
 *
 * @param eui64               Identifier of an end device
 * @param installCode         Install code from the end device
 * @param installCodeLength   Length of the passed install code
 * @return EmberStatus  EMBER_SUCCESS or error
 */
EmberStatus z3gatewayTrustCenterAddDeviceInstallCode(const EmberEUI64 eui64,
                                                     uint8_t *installCode,
                                                     uint8_t installCodeLength);

/**
 * @brief Remove a device that already exists on the network. This call uses
 * the device-table plugin to initiate the leave request. if the device is not
 * currently tracked by the plugin, EMBER_NOT_FOUND will be returned.
 *
 * @param eui64         End device identifier
 * @return EmberStatus  EMBER_SUCCESS when the Leave Request is sent successfully,
 * EMBER_NOT_FOUND if the device is not found in the device-table plugin.
 */
EmberStatus z3gatewayTrustCenterRemoveDevice(const EmberEUI64 eui64);

/**
 * @brief Start Discovery device capabilities (Endpoint information).
 *
 * @param eui64         End device identifier
 * @return EmberStatus  EMBER_SUCCESS or error
 */
EmberStatus z3gatewayTrustCenterStartDiscovery(const EmberEUI64 eui64);

/**
 * @brief Populate Z3Gateway ZCL frame to be sent on the network. This function
 * will copy the supplied buffer to an internal buffer. Along with the buffer,
 * an offset to the sequence ID should be supplied as this function will fill
 * in the correct Sequence ID at the index supplied.
 *
 * @param buffer              ZCL frame buffer containing additional data.
 * @param bufferSize          Size of the ZCL frame buffer.
 * @param bufferSequenceIndex Offset of sequence ID in the frame buffer to be filled.
 *
 * @return the status of the operation: EMBER_SUCCESS if ZCL Frame is populated
 * successfully, EMBER_BAD_ARGUMENT if any inputs are invalid,
 * EMBER_INDEX_OUT_OF_RANGE if the buffer size does not meet the maximum
 * and minimum limits.
**/
EmberStatus z3gatewayFillZclFrame(const uint8_t *buffer,
                                  size_t bufferSize,
                                  size_t bufferSequenceIndex);

/**
 * @brief Send the internal Z3Gateway ZCL frame to an end device on the network
 * via unicast message to the device that has the matching
 * EUI64/endpoint/cluster identifiers.
 *
 * NOTE: This function assumes that \ref z3gatewayFillZclFrame has been called
 * prior to fill in a ZCL message/frame.
 *
 * @param eui64     Identifier of the Zigbee end device.
 * @param endpoint  Identifier of the endpoint on the Zigbee end device.
 * @param clusterId Identifier corresponding to the ZCL cluster
 *                  (ex. 0x0006 <--> OnOff).
 *
 * @return the status of the operation: EMBER_SUCCESS if message is sent
 * successfully, EMBER_BAD_ARGUMENT if any inputs are invalid,
 * or EMBER_NOT_FOUND if the end device EUI64 is not found.
**/
EmberStatus z3gatewaySendZclFrameUnicast(const EmberEUI64 eui64,
                                         uint8_t endpoint,
                                         uint16_t clusterId);

/**
 * @brief Send the internal Z3Gateway ZCL frame via a multicast message to the
 * devices that has the matching multicastId in their ZCL Groups table.
 *
 * NOTE: This function assumes that \ref z3gatewayFillZclFrame has been called
 * prior to fill in a ZCL message/frame.
 *
 * @param multicastId Identifier of Multicast ID used for the message.
 * @param clusterId   Identifier corresponding to the ZCL cluster
 *                    (ex. 0x0006 <--> OnOff).
 *
 * @return the status of the operation: EMBER_SUCCESS if message is sent
 * successfully, EMBER_BAD_ARGUMENT if any inputs are invalid.
**/
EmberStatus z3gatewaySendZclFrameMulticast(EmberMulticastId multicastId,
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
 * @return EmberStatus        EMBER_SUCCESS if message is sent successfully,
 * EMBER_NOT_FOUND if the end device EUI64 is not found.
 */
EmberStatus z3gatewayInitReporting(const EmberEUI64 eui64,
                                   uint8_t endpoint,
                                   uint16_t clusterId,
                                   const uint8_t *reportRecord,
                                   size_t recordSize);
/**
 * @brief Send a Zigbee Bind Request message. Required for reports and other
 * zigbee-specific management options. Can be used to configure unicast or multicast
 * bindings. If a unicast binding, groupId should be set to 0
 *
 * @param eui64               Identifier of the Zigbee end device
 * @param endpoint            Identifier of the endpoint on the Zigbee end device.
 * @param clusterId           Identifier corresponding to the ZCL cluster.
 * @param groupId             Identifier for the groupId (if multicast binding)
 * @return EmberStatus        EMBER_SUCCESS if message is sent successfully,
 * EMBER_NOT_FOUND if the end device EUI64 is not found.
 */
EmberStatus z3gatewayInitBinding(const EmberEUI64 eui64,
                                 uint8_t endpoint,
                                 uint16_t clusterId,
                                 uint16_t groupId);

/**
 * @brief z3gatewaySendPollingCheckInResponse
 * Send a CheckInResponse to a given poll control server
 * By default, sets it to fast poll with the default timeout
 *
 * @return EMBER_SUCCESS if the response was sent
**/
EmberStatus z3gatewaySendPollingCheckInResponse(void);

/**
 * @brief z3gatewayAddOtaImage 
 * Manually adds an OtaImage to the ota-server plugin
 *
 * @param filename - the filename of the OTA Image
 *
 * @return EMBER_SUCCESS if Image was added successfully.
 * Fails if the ota-storage plugin was unable to parse
 * the filename correctly with EMBER_ERR_FATAL
**/
EmberStatus z3gatewayAddOtaImage(const char* filename);

#ifdef __cplusplus
}
#endif

#endif  //Z3GATEWAY_H

/** @} end z3gateway */
