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
 * @file zigpc_datastore.h
 * @defgroup zigpc_datastore ZigPC Data Storage API
 * @ingroup zigpc_components
 * @brief API for the creating, updating, and deleting persistent information.
 *
 * The ZigPC datastore component allows components to persist discovery
 * information using the tree-based storage API provided by the shared Unify
 * attribute store component. The desired fields in the attribute store are
 * used as a container to store the search keys for network, device, endpoint,
 * and cluster entity searching. The following APIs are available for each of
 * these entities: create, read, write, remove, get_count, and find_by_index.
 *
 * In order to perist an entity, it should first be created before any data
 * can be read/written from it.
 *
 * The get_count and find_by_index operations can be used to iterate
 * through sub-entities (i.e. iterating through endpoints in a device).
 *
 * The following diagram shows the relationship between each of these entities:
 * @startuml
   class AttributeStoreRoot
   entity Network <<instance>> {
     uint8_t search_key
     ...network data...
   }
   entity Device_1 <<instance>> {
     zigbee_eui64_t search_key
     ...device data...
   }
   entity Device_2 <<instance>> {
     zigbee_eui64_t search_key
     ...device data...
   }
   entity Device_3 <<instance>> {
     zigbee_eui64_t search_key
     ...device data...
   }
   entity Endpoint_1_1 <<instance>> {
     zigbee_endpoint_id_t search_key
     ...endpoint data...
   }
   entity Endpoint_1_2 <<instance>> {
     zigbee_endpoint_id_t search_key
     ...endpoint data...
   }
   entity Endpoint_2_1 <<instance>> {
     zigbee_endpoint_id_t search_key
     ...endpoint data...
   }
   entity Endpoint_3_1 <<instance>> {
     zigbee_endpoint_id_t search_key
     ...endpoint data...
   }
   entity Endpoint_3_2 <<instance>> {
     zigbee_endpoint_id_t search_key
     ...endpoint data...
   }
   entity Endpoint_3_3 <<instance>> {
     zigbee_endpoint_id_t search_key
     ...endpoint data...
   }
   entity Cluster_1_1_x <<instance>> {
     zcl_cluster_id_t search_key
     ...cluster data...
   }
   entity Cluster_1_2_x <<instance>> {
     zcl_cluster_id_t search_key
     ...cluster data...
   }
   entity Cluster_2_1_x <<instance>> {
     zcl_cluster_id_t search_key
     ...cluster data...
   }
   entity Cluster_3_1_x <<instance>> {
     zcl_cluster_id_t search_key
     ...cluster data...
   }
   entity Cluster_3_2_x <<instance>> {
     zcl_cluster_id_t search_key
     ...cluster data...
   }
   entity Cluster_3_3_x <<instance>> {
     zcl_cluster_id_t search_key
     ...cluster data...
   }

   AttributeStoreRoot o-down- Network: 1..1
   Network o-down- Device_1
   Network o-down- Device_2
   Network o-down- Device_3
   Device_1 o-down- Endpoint_1_1
   Device_1 o-down- Endpoint_1_2
   Device_2 o-down- Endpoint_2_1
   Device_3 o-down- Endpoint_3_1
   Device_3 o-down- Endpoint_3_2
   Device_3 o-down- Endpoint_3_3
   Endpoint_1_1 o-down- Cluster_1_1_x
   Endpoint_1_2 o-down- Cluster_1_2_x
   Endpoint_2_1 o-down- Cluster_2_1_x
   Endpoint_3_1 o-down- Cluster_3_1_x
   Endpoint_3_2 o-down- Cluster_3_2_x
   Endpoint_3_3 o-down- Cluster_3_3_x
   @enduml
 * @{
 *
**/

#ifndef ZIGPC_DATASTORE_H
#define ZIGPC_DATASTORE_H

#include "sl_status.h"
#include "zigpc_common_zigbee.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Network information.
 *
 */
typedef struct {
  zigbee_eui64_t gateway_eui64;
  zigbee_panid_t panid;
  zigbee_ext_panid_t ext_panid;
  zigbee_radio_channel_t radio_channel;
} zigpc_network_data_t;

/**
 * @brief Device information.
 *
 */
typedef struct {
  enum zigbee_node_network_status network_status;
  uint32_t max_cmd_delay;
  uint8_t endpoint_total_count;
  uint8_t endpoint_discovered_count;
} zigpc_device_data_t;

/**
 * @brief Device endpoint information.
 *
 */
typedef struct {
  uint8_t zcl_version;
} zigpc_endpoint_data_t;

/**
 * @brief Device endpoint cluster information.
 *
 */
typedef struct {
  uint8_t dummy; /**< placeholder */
} zigpc_cluster_data_t;

/**********************************
 **********************************
 *
 * Network Entity APIs
 *
 **********************************
 **********************************/

/**
 * @brief Create network entity to be persisted. The read/write APIs can be
 * used to manipulate the data associated with the network.
 *
 * @return sl_status_t SL_STATUS_OK on success, SL_STATUS_ALREADY_EXISTS if
 * the network node already exists, SL_STATUS_NOT_INITIALIZED if attribute
 * store is not initialized or SL_STATUS_FAIL if the creation process fails.
 */
sl_status_t zigpc_datastore_create_network(void);

/**
 * @brief Read data from persisted network entity.
 *
 * @param data           Network information to read.
 * @return sl_status_t   SL_STATUS_OK on success, SL_STATUS_NULL_POINTER
 * if invalid data object provided, SL_STATUS_NOT_FOUND if the network node
 * does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is not
 * initialized, or SL_STATUS_FAIL if the read process fails.
 */
sl_status_t zigpc_datastore_read_network(zigpc_network_data_t *const data);

/**
 * @brief Write data to persisted network entity.
 *
 * @param data           Network information to write.
 * @return sl_status_t   SL_STATUS_OK on success, SL_STATUS_NULL_POINTER
 * if invalid data object provided, SL_STATUS_NOT_FOUND if the network node
 * does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is not
 * initialized, or SL_STATUS_FAIL if the write process fails.
 */
sl_status_t zigpc_datastore_write_network(const zigpc_network_data_t *data);

/**
 * @brief Remove persisted network entity.
 *
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NOT_FOUND if the
 * network node does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is
 * not initialized, or SL_STATUS_FAIL if the delete process fails.
 */
sl_status_t zigpc_datastore_remove_network(void);

/**********************************
 **********************************
 *
 * Device Entity APIs
 *
 **********************************
 **********************************/

/**
 * @brief Get count of devices persisted under the network.
 *
 * @return size_t Number of devices persisted.
 */
size_t zigpc_datastore_get_device_count(void);

/**
 * @brief Find a particular device entity and retrieve its search key (EUI64)
 * based on an index. See \ref zigpc_datastore_get_device_count on getting the
 * total number of devices persisted.
 *
 * @param index         Index to retrieve.
 * @param eui64         Reference to device identifier search key to populate.
 * @return sl_status_t  SL_STATUS_OK if the device under the index is found,
 * SL_STATUS_NOT_FOUND if the device is not found, SL_STATUS_NULL_POINTER if
 * the provided EUI64 is invalid, or SL_STATUS_FAIL if the find process fails.
 */
sl_status_t zigpc_datastore_find_device_by_index(size_t index,
                                                 zigbee_eui64_t *const eui64);

/**
 * @brief Create device entity to be persisted under the network. The
 * read/write APIs can be used to manipulate the data associated with this
 * device entity.
 *
 * @param eui64         Device identifier.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_ALREADY_EXISTS if the
 * device node already exists, SL_STATUS_NOT_INITIALIZED if attribute store is
 * not initialized or SL_STATUS_FAIL if the creation process fails.
 */
sl_status_t zigpc_datastore_create_device(const zigbee_eui64_t eui64);

/**
 * @brief Read data from a persisted device entity.
 *
 * @param eui64         Device identifier.
 * @param data          Device information to read.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER
 * if invalid data object provided, SL_STATUS_NOT_FOUND if the device node
 * does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is not
 * initialized, or SL_STATUS_FAIL if the read process fails.
 */
sl_status_t zigpc_datastore_read_device(const zigbee_eui64_t eui64,
                                        zigpc_device_data_t *const data);

/**
 * @brief Write data to a persisted device entity.
 *
 *
 * @param eui64         Device identifier.
 * @param data          Device information to write.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER
 * if invalid data object provided, SL_STATUS_NOT_FOUND if the device node
 * does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is not
 * initialized, or SL_STATUS_FAIL if the write process fails.
 */
sl_status_t zigpc_datastore_write_device(const zigbee_eui64_t eui64,
                                         const zigpc_device_data_t *data);

/**
 * @brief Remove persisted device entity.
 *
 * @param eui64         Device identifier.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NOT_FOUND if the
 * device node does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is
 * not initialized, or SL_STATUS_FAIL if the delete process fails.
 */
sl_status_t zigpc_datastore_remove_device(const zigbee_eui64_t eui64);

/**********************************
 **********************************
 *
 * Endpoint Entity APIs
 *
 **********************************
 **********************************/

/**
 * @brief Get count of endpoints persisted under a device.
 *
 * @param eui64         Device identifier.
 * @return size_t       Number of endpoints persisted.
 */
size_t zigpc_datastore_get_endpoint_count(const zigbee_eui64_t eui64);

/**
 * @brief Find a particular endpoint entity persisted under a device and retrieve
 * its search key (endpoint_id) based on an index. See \ref
 * zigpc_datastore_get_endpoint_count on getting the total number of endpoints
 * persisted under a device.
 *
 * @param eui64         Device identifier.
 * @param index         Index to retrieve.
 * @param endpoint_id   Reference to endpoint identifier search key to populate.
 * @return sl_status_t  SL_STATUS_OK if the endpoint is found,
 * SL_STATUS_NOT_FOUND if the endpoint is not found, SL_STATUS_NULL_POINTER if
 * the provided EUI64 or endpoint_id is invalid, or SL_STATUS_FAIL if the find
 * process fails.
 */
sl_status_t zigpc_datastore_find_endpoint_by_index(
  const zigbee_eui64_t eui64, size_t index, zigbee_endpoint_id_t *endpoint_id);

/**
 * @brief Create endpoint entity to be persisted under a device. The read/write
 * APIs can be used to manipulate the data associated with this endpoint entity.
 *
 * @param eui64         Device identifier to persist under.
 * @param endpoint_id   Endpoint identifier.
 *
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_ALREADY_EXISTS if
 * the device node already exists, SL_STATUS_NOT_INITIALIZED if attribute store
 * is not initialized or SL_STATUS_FAIL if the creation process fails.
 */
sl_status_t zigpc_datastore_create_endpoint(const zigbee_eui64_t eui64,
                                            zigbee_endpoint_id_t endpoint_id);

/**
 * @brief Read data from a persisted endpoint entity.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param data          Endpoint data to be read.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER
 * if invalid data object provided, SL_STATUS_NOT_FOUND if the endpoint node
 * does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is not
 * initialized, or SL_STATUS_FAIL if the read process fails.
 */
sl_status_t zigpc_datastore_read_endpoint(const zigbee_eui64_t eui64,
                                          zigbee_endpoint_id_t endpoint_id,
                                          zigpc_endpoint_data_t *const data);

/**
 * @brief Write data to a persisted device entity.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param data          Endpoint data to be written.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER
 * if invalid data object provided, SL_STATUS_NOT_FOUND if the endpoint node
 * does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is not
 * initialized, or SL_STATUS_FAIL if the write process fails.
 */
sl_status_t zigpc_datastore_write_endpoint(const zigbee_eui64_t eui64,
                                           zigbee_endpoint_id_t endpoint_id,
                                           const zigpc_endpoint_data_t *data);

/**
 * @brief Remove persisted endpoint entity.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NOT_FOUND if the
 * endpoint node does not exist, SL_STATUS_NOT_INITIALIZED if attribute store
 * is not initialized, or SL_STATUS_FAIL if the delete process fails.
 */
sl_status_t zigpc_datastore_remove_endpoint(const zigbee_eui64_t eui64,
                                            zigbee_endpoint_id_t endpoint_id);

/**********************************
 **********************************
 *
 * Cluster Entity APIs
 *
 **********************************
 **********************************/

/**
 * @brief Get count of clusters persisted under a device endpoint.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @return size_t Number of endpoints persisted.
 */

size_t zigpc_datastore_get_cluster_count(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id);

/**
 * @brief Find a particular cluster entity persisted under a device endpoint and
 * retrieve its search key (cluster_id) based on an index. See \ref
 * zigpc_datastore_get_cluster_count on getting the total number of clusters
 * persisted under a device endpoint.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param index         Index to retrieve.
 * @param cluster_id    Reference to cluster identifier search key to populate.
 * @return sl_status_t  SL_STATUS_OK if the endpoint is found,
 * SL_STATUS_NOT_FOUND if the cluster node is not found, SL_STATUS_NULL_POINTER
 * if the provided EUI64 or endpoint_id is invalid, or SL_STATUS_FAIL if the
 * find process fails.
 */
sl_status_t
  zigpc_datastore_find_cluster_by_index(const zigbee_eui64_t eui64,
                                        zigbee_endpoint_id_t endpoint_id,
                                        size_t index,
                                        zcl_cluster_id_t *cluster_id);

/**
 * @brief Create cluster entity to be persisted under a device endpoint. The
 * read/write APIs can be used to manipulate the data associated with this
 * cluster entity.
 *
 * @param eui64         Device identifier to persist under.
 * @param endpoint_id   Endpoint identifier to persist under.
 * @param cluster_id    Cluster identifier.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_ALREADY_EXISTS if
 * the cluster node already exists, SL_STATUS_NOT_INITIALIZED if attribute store
 * is not initialized or SL_STATUS_FAIL if the creation process fails.
 */
sl_status_t zigpc_datastore_create_cluster(const zigbee_eui64_t eui64,
                                           zigbee_endpoint_id_t endpoint_id,
                                           zcl_cluster_id_t cluster_id);

/**
 * @brief Read data from a persisted cluster entity.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param cluster_id    Cluster identifier.
 * @param data          Cluster data to be read.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER
 * if invalid data object provided, SL_STATUS_NOT_FOUND if the cluster node
 * does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is not
 * initialized, or SL_STATUS_FAIL if the read process fails.
 */
sl_status_t zigpc_datastore_read_cluster(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id,
                                         zcl_cluster_id_t cluster_id,
                                         zigpc_cluster_data_t *const data);

/**
 * @brief Write data to a persisted cluster entity.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param cluster_id    Cluster identifier.
 * @param data          Cluster data to be written.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER
 * if invalid data object provided, SL_STATUS_NOT_FOUND if the cluster node
 * does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is not
 * initialized, or SL_STATUS_FAIL if the write process fails.
 */
sl_status_t zigpc_datastore_write_cluster(const zigbee_eui64_t eui64,
                                          zigbee_endpoint_id_t endpoint_id,
                                          zcl_cluster_id_t cluster_id,
                                          const zigpc_cluster_data_t *data);

/**
 * @brief Remove persisted cluster entity.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param cluster_id    Cluster identifier.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NOT_FOUND if the
 * cluster node does not exist, SL_STATUS_NOT_INITIALIZED if attribute store
 * is not initialized, or SL_STATUS_FAIL if the delete process fails.
 */
sl_status_t zigpc_datastore_remove_cluster(const zigbee_eui64_t eui64,
                                           zigbee_endpoint_id_t endpoint_id,
                                           zcl_cluster_id_t cluster_id);

#ifdef __cplusplus
}
#endif

#endif  // ZIGPC_DATASTORE_H

/** @} end zigpc_datastore **/
