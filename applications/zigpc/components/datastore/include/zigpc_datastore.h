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

// Shared Unify includes
#include <sl_status.h>
#include <sl_log.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>

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
} __attribute__((__packed__)) zigpc_network_data_t;

/**
 * @brief Device information.
 *
 */
typedef struct {
  enum zigbee_node_network_status network_status;
  uint32_t max_cmd_delay;
  uint8_t endpoint_total_count;
  uint8_t endpoint_discovered_count;
} __attribute__((__packed__)) zigpc_device_data_t;

/**
 * @brief Device endpoint information.
 *
 */
typedef struct {
  uint8_t zcl_version;
} __attribute__((__packed__)) zigpc_endpoint_data_t;

/**
 * @brief Device endpoint cluster information.
 *
 */
typedef struct {
  uint8_t dummy; /**< placeholder */
} __attribute__((__packed__)) zigpc_cluster_data_t;

/**
 * @brief Device endpoint group information.
 *
 */
typedef struct {
  char group_name[ZCL_GROUP_NAME_LENGTH];
} __attribute__((__packed__)) zigpc_group_data_t;

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

/**
 * @brief Remove persisted children under device entity.
 *
 * @param eui64         Device identifier.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NOT_FOUND if the
 * device node does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is
 * not initialized, or SL_STATUS_FAIL if the child delete process fails.
 */
sl_status_t zigpc_datastore_remove_device_children(const zigbee_eui64_t eui64);

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
 * Group Entity APIs
 *
 **********************************
 **********************************/

/**
 * @brief Get count of groups persisted under an endpoint.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param is_reported   Look in stored reported/desired state.
 * @return size_t       Number of groups persisted.
 */
size_t zigpc_datastore_get_group_count(const zigbee_eui64_t eui64,
                                       zigbee_endpoint_id_t endpoint_id,
                                       bool is_reported);

/**
 * @brief Find a particular group entity persisted under a device endpoint and
 * retrieve its search key (group_id) based on an index. See \ref
 * zigpc_datastore_get_group_count on getting the total number of groupss
 * persisted under a device endpoint.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param is_reported   Look in stored reported/desired state.
 * @param index         Index to retrieve.
 *
 * @return sl_status_t  SL_STATUS_OK if the endpoint is found,
 * SL_STATUS_NOT_FOUND if the group node is not found, SL_STATUS_NULL_POINTER
 * if the provided EUI64 or endpoint_id is invalid, or SL_STATUS_FAIL if the
 * find process fails.
 */
sl_status_t
  zigpc_datastore_find_group_by_index(const zigbee_eui64_t eui64,
                                      zigbee_endpoint_id_t endpoint_id,
                                      bool is_reported,
                                      size_t index,
                                      zigbee_group_id_t *group_id);

/**
 * @brief Determine if a particular group ID is supported under a device
 * endpoint.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param is_reported   Look in stored reported/desired state.
 * @param group_id      Reference to group identifier search key to populate.

 * @return bool         TRUE if the group identifier exists under the device
 * endpoint, FALSE if not.
 */
bool zigpc_datastore_contains_group(const zigbee_eui64_t eui64,
                                    zigbee_endpoint_id_t endpoint_id,
                                    bool is_reported,
                                    zigbee_group_id_t group_id);

/**
 * @brief Create group entity to be persisted under a device endpoint. The
 * read/write APIs can be used to manipulate the data associated with this
 * group entity.
 *
 * @param eui64         Device identifier to persist under.
 * @param endpoint_id   Endpoint identifier to persist under.
 * @param is_reported   Look in stored reported/desired state.
 * @param group_id      Reference to group identifier search key to populate.
 *
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_ALREADY_EXISTS if
 * the group node already exists, SL_STATUS_NOT_INITIALIZED if attribute store
 * is not initialized or SL_STATUS_FAIL if the creation process fails.
 */
sl_status_t zigpc_datastore_create_group(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id,
                                         bool is_reported,
                                         zigbee_group_id_t group_id);

/**
 * @brief Read data from a persisted group entity.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param is_reported   Look in stored reported/desired state.
 * @param group_id      Reference to group identifier search key to populate.
 * @param data          Group data to be read.
 *
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER
 * if invalid data object provided, SL_STATUS_NOT_FOUND if the group node
 * does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is not
 * initialized, or SL_STATUS_FAIL if the read process fails.
 */
sl_status_t zigpc_datastore_read_group(const zigbee_eui64_t eui64,
                                       zigbee_endpoint_id_t endpoint_id,
                                       bool is_reported,
                                       zigbee_group_id_t group_id,
                                       zigpc_group_data_t *const data);

/**
 * @brief Write data to a persisted group entity.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param is_reported   Look in stored reported/desired state.
 * @param group_id      Reference to group identifier search key to populate.
 * @param data          Group data to be read.
 *
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER
 * if invalid data object provided, SL_STATUS_NOT_FOUND if the group node
 * does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is not
 * initialized, or SL_STATUS_FAIL if the write process fails.
 */
sl_status_t zigpc_datastore_write_group(const zigbee_eui64_t eui64,
                                        zigbee_endpoint_id_t endpoint_id,
                                        bool is_reported,
                                        zigbee_group_id_t group_id,
                                        const zigpc_group_data_t *data);

/**
 * @brief Remove persisted group entity.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param is_reported   Look in stored reported/desired state.
 * @param group_id      Reference to group identifier search key to populate.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NOT_FOUND if the
 * group node does not exist, SL_STATUS_NOT_INITIALIZED if attribute store
 * is not initialized, or SL_STATUS_FAIL if the delete process fails.
 */
sl_status_t zigpc_datastore_remove_group(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id,
                                         bool is_reported,
                                         zigbee_group_id_t group_id);
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
 * @param cluster_side  Cluster side (server/client).
 * @return size_t Number of endpoints persisted.
 */
size_t zigpc_datastore_get_cluster_count(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id,
                                         zcl_cluster_side_t cluster_side);

/**
 * @brief Find a particular cluster entity persisted under a device endpoint and
 * retrieve its search key (cluster_id) based on an index. See \ref
 * zigpc_datastore_get_cluster_count on getting the total number of clusters
 * persisted under a device endpoint.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param cluster_side  Cluster side (server/client).
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
                                        zcl_cluster_side_t cluster_side,
                                        size_t index,
                                        zcl_cluster_id_t *cluster_id);

/**
 * @brief Determine if a particular cluster ID is supported under a device
 * endpoint.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param cluster_side  Cluster side (server/client).
 * @param cluster_id    Cluster identifier to find.
 * @return bool         TRUE if the cluster identifier exists under the device
 * endpoint, FALSE if not.
 */
bool zigpc_datastore_contains_cluster(const zigbee_eui64_t eui64,
                                      zigbee_endpoint_id_t endpoint_id,
                                      zcl_cluster_side_t cluster_side,
                                      zcl_cluster_id_t cluster_id);

/**
 * @brief Create cluster entity to be persisted under a device endpoint. The
 * read/write APIs can be used to manipulate the data associated with this
 * cluster entity.
 *
 * @param eui64         Device identifier to persist under.
 * @param endpoint_id   Endpoint identifier to persist under.
 * @param cluster_id    Cluster identifier.
 * @param cluster_side  Cluster side (server/client).
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_ALREADY_EXISTS if
 * the cluster node already exists, SL_STATUS_NOT_INITIALIZED if attribute store
 * is not initialized or SL_STATUS_FAIL if the creation process fails.
 */
sl_status_t zigpc_datastore_create_cluster(const zigbee_eui64_t eui64,
                                           zigbee_endpoint_id_t endpoint_id,
                                           zcl_cluster_side_t cluster_side,
                                           zcl_cluster_id_t cluster_id);

/**
 * @brief Read data from a persisted cluster entity.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param cluster_id    Cluster identifier.
 * @param cluster_side  Cluster side (server/client).
 * @param data          Cluster data to be read.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER
 * if invalid data object provided, SL_STATUS_NOT_FOUND if the cluster node
 * does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is not
 * initialized, or SL_STATUS_FAIL if the read process fails.
 */
sl_status_t zigpc_datastore_read_cluster(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id,
                                         zcl_cluster_side_t cluster_side,
                                         zcl_cluster_id_t cluster_id,
                                         zigpc_cluster_data_t *const data);

/**
 * @brief Write data to a persisted cluster entity.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param cluster_id    Cluster identifier.
 * @param cluster_side  Cluster side (server/client).
 * @param data          Cluster data to be written.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER
 * if invalid data object provided, SL_STATUS_NOT_FOUND if the cluster node
 * does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is not
 * initialized, or SL_STATUS_FAIL if the write process fails.
 */
sl_status_t zigpc_datastore_write_cluster(const zigbee_eui64_t eui64,
                                          zigbee_endpoint_id_t endpoint_id,
                                          zcl_cluster_side_t cluster_side,
                                          zcl_cluster_id_t cluster_id,
                                          const zigpc_cluster_data_t *data);

/**
 * @brief Remove persisted cluster entity.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param cluster_id    Cluster identifier.
 * @param cluster_side  Cluster side (server/client).
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NOT_FOUND if the
 * cluster node does not exist, SL_STATUS_NOT_INITIALIZED if attribute store
 * is not initialized, or SL_STATUS_FAIL if the delete process fails.
 */
sl_status_t zigpc_datastore_remove_cluster(const zigbee_eui64_t eui64,
                                           zigbee_endpoint_id_t endpoint_id,
                                           zcl_cluster_side_t cluster_side,
                                           zcl_cluster_id_t cluster_id);

/**********************************
 **********************************
 *
 * Cluster Attribute List API
 *
 **********************************
 **********************************/

/**
 * @brief Get count of attributes persisted under a cluster.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param cluster_side  Cluster side (server/client).
 * @param cluster_id    Cluster identifier.
 * @return size_t Number of attributes persisted.
 */
size_t zigpc_datastore_get_attribute_count(const zigbee_eui64_t eui64,
                                           zigbee_endpoint_id_t endpoint_id,
                                           zcl_cluster_side_t cluster_side,
                                           zcl_cluster_id_t cluster_id);

/**
 * @brief Read the attribute list under a given cluster.
 *
 * @param eui64           Device identifier.
 * @param endpoint_id     Endpoint identifier.
 * @param cluster_side    Cluster side (server/client).
 * @param cluster_id      Cluster identifier.
 * @param attribute_list  Attribute list to populate.
 * @param attribute_count Reference to attribute list count; IN: Provides the
 *                        capacity of list passed in; OUT: updates with number
 *                        of items populated into the list.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER
 * if invalid list provided, SL_STATUS_NOT_FOUND if the cluster node
 * does not exist, SL_STATUS_INVALID_COUNT if the attribute list passed
 * in has insufficient capacity, SL_STATUS_NOT_INITIALIZED if attribute store is not
 * initialized, or SL_STATUS_FAIL if the read process fails.
 */
sl_status_t
  zigpc_datastore_read_attribute_list(const zigbee_eui64_t eui64,
                                      zigbee_endpoint_id_t endpoint_id,
                                      zcl_cluster_side_t cluster_side,
                                      zcl_cluster_id_t cluster_id,
                                      zcl_attribute_id_t *const attribute_list,
                                      size_t *attribute_count);

/**
 * @brief Write attribute list under a given cluster. This function will
 * overwrite any existing list by removing any pre-existing list.
 *
 * @param eui64           Device identifier.
 * @param endpoint_id     Endpoint identifier.
 * @param cluster_side    Cluster side (server/client).
 * @param cluster_id      Cluster identifier.
 * @param attribute_list  Attribute list to persist.
 * @param attribute_count Number of list items to persist.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER
 * if invalid list provided, SL_STATUS_NOT_FOUND if the cluster node
 * does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is not
 * initialized, or SL_STATUS_FAIL if the write process fails.
 */
sl_status_t
  zigpc_datastore_write_attribute_list(const zigbee_eui64_t eui64,
                                       zigbee_endpoint_id_t endpoint_id,
                                       zcl_cluster_side_t cluster_side,
                                       zcl_cluster_id_t cluster_id,
                                       const zcl_attribute_id_t *attribute_list,
                                       size_t attribute_count);

/**********************************
 **********************************
 *
 * Cluster Command List API
 *
 **********************************
 **********************************/

/**
 * @brief Get count of commands persisted under a cluster based on the command
 * type passed in.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param cluster_side  Cluster side (server/client).
 * @param cluster_id    Cluster identifier.
 * @param command_type  Type of commands under the cluster (out/in).
 * @return size_t Number of commands persisted.
 */
size_t zigpc_datastore_get_command_count(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id,
                                         zcl_cluster_side_t cluster_side,
                                         zcl_cluster_id_t cluster_id,
                                         zcl_command_type_t command_type);

/**
 * @brief Read command list under a given cluster based on the command type
 * passed in.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param cluster_side  Cluster side (server/client).
 * @param cluster_id    Cluster identifier.
 * @param command_type  Type of commands under the cluster (out/in).
 * @param command_list  Command list to populate.
 * @param command_count Reference to command list count; IN: Provides the
 *                      capacity of list passed in; OUT: updates with number
 *                      of items populated into the list.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER
 * if invalid list provided, SL_STATUS_NOT_FOUND if the cluster node
 * does not exist, SL_STATUS_INVALID_COUNT if the command list passed
 * in has insufficient capacity, SL_STATUS_NOT_INITIALIZED if attribute store is not
 * initialized, or SL_STATUS_FAIL if the read process fails.
 */
sl_status_t
  zigpc_datastore_read_command_list(const zigbee_eui64_t eui64,
                                    zigbee_endpoint_id_t endpoint_id,
                                    zcl_cluster_side_t cluster_side,
                                    zcl_cluster_id_t cluster_id,
                                    zcl_command_type_t command_type,
                                    zcl_command_id_t *const command_list,
                                    size_t *command_count);

/**
 * @brief Write command list under a given cluster based on the command type
 * passed in. This function will overwrite any existing list by removing any
 * pre-existing list.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Endpoint identifier.
 * @param cluster_side  Cluster side (server/client).
 * @param cluster_id    Cluster identifier.
 * @param command_type  Type of commands under the cluster (out/in).
 * @param command_list  Command list to persist.
 * @param command_count Number of list items to persist.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER
 * if invalid list provided, SL_STATUS_NOT_FOUND if the cluster node
 * does not exist, SL_STATUS_NOT_INITIALIZED if attribute store is not
 * initialized, or SL_STATUS_FAIL if the write process fails.
 */
sl_status_t
  zigpc_datastore_write_command_list(const zigbee_eui64_t eui64,
                                     zigbee_endpoint_id_t endpoint_id,
                                     zcl_cluster_side_t cluster_side,
                                     zcl_cluster_id_t cluster_id,
                                     zcl_command_type_t command_type,
                                     const zcl_command_id_t *command_list,
                                     size_t command_count);

/**********************************
 **********************************
 *
 * Utility Functions
 *
 **********************************
 **********************************/

/**
 * @brief Find the endpoint identifier under a device containing a particular
 * cluster identifier.
 *
 * @param eui64         Device identifier.
 * @param cluster_side  Cluster side (server/client).
 * @param cluster_id    Cluster identifier.
 * @param endpoint_id   Reference to endpoint identifier search key to populate.
 * @return sl_status_t  SL_STATUS_OK if the endpoint is found and populated,
 * SL_STATUS_NOT_FOUND if the endpoint is not found under the device,
 * SL_STATUS_NULL_POINTER if the provided EUI64 or endpoint_id is invalid, or
 * SL_STATUS_FAIL if the find process fails.
 */
sl_status_t zigpc_datastore_find_endpoint_containing_cluster(
  const zigbee_eui64_t eui64,
  zcl_cluster_side_t cluster_side,
  zcl_cluster_id_t cluster_id,
  zigbee_endpoint_id_t *const endpoint_id);

/**********************************
 **********************************
 *
 * Utility Logging API
 *
 **********************************
 **********************************/

/**
 * @brief Log the stored discovery information under a device which includes:
 * endpoints, clusters, attributes, and command support information.
 *
 * @param eui64 Device identifier.
 */
void zigpc_datastore_log_device_disc_state(const zigbee_eui64_t eui64);

/**
 * @brief Log the network information.
 *
 * @param log_tag       Tag to log the message under.
 * @param log_level     Level to log the message under.
 * @param log_prefix    String to prefix log message.
 * @return sl_status_t  SL_STATUS_OK if the network is successfully logged,
 * SL_STATUS_NULL_POINTER if the log_tag provided is invalid, or
 * SL_STATUS_NOT_FOUND if the network entity is not found.
 */
sl_status_t zigpc_datastore_log_network(const char *log_tag,
                                        sl_log_level_t log_level,
                                        const char *log_prefix);

/**
 * @brief Log single device entity.
 *
 * @param log_tag       Tag to log the message under.
 * @param log_level     Level to log the message under.
 * @param log_prefix    String to prefix log message.
 * @param eui64         Device identifier.
 * @return sl_status_t  SL_STATUS_OK if the device is successfully logged,
 * SL_STATUS_NULL_POINTER if the log_tag provided is invalid, or
 * SL_STATUS_NOT_FOUND if the device entity is not found.
 */
sl_status_t zigpc_datastore_log_device(const char *log_tag,
                                       sl_log_level_t log_level,
                                       const char *log_prefix,
                                       const zigbee_eui64_t eui64);

#ifdef __cplusplus
}
#endif

#endif  // ZIGPC_DATASTORE_H

/** @} end zigpc_datastore **/
