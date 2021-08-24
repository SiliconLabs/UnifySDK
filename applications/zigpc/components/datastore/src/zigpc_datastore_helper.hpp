/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA);; available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @defgroup zigpc_datastore_helper ZigPC datastore accessor templates.
 * @ingroup zigpc_datastore
 * @brief Templates used for creating store accessors.
 *
 * @{
 */

#ifndef ZIGPC_DATASTORE_HELPER_HPP
#define ZIGPC_DATASTORE_HELPER_HPP

#include "attribute_store.h"
#ifdef __cplusplus

static constexpr char LOG_TAG[] = "zigpc_datastore";

/**
 * @brief Template class for accessing entities using the attribute store API.
 *
 * NOTE: The serach_key_t for every entity type uses the desired field of an
 * attribute store node while the data_t is stored using the reported field of
 * an attribute store node. Having the search_key_t contained in the desired
 * field allows the component to search for attribute store nodes by the
 * desired key value.
 *
 * @tparam search_key_t   key type to be stored in the attribute store
 *                        node's desired field and used for searching.
 * @tparam data_t         data to be stored in the attribute store node's
 *                        reported field.
 */
template<typename search_key_t, typename data_t> class AttrStoreHelper
{
  private:
  attribute_store_type_t node_type; /**< Entity type to be used */

  public:
  AttrStoreHelper(attribute_store_type_t node_type) : node_type(node_type) {}

  /**
   * @brief Find an entity based based on the key.
   *
   * @param parent
   * @param key
   * @return attribute_store_node_t ATTRIBUTE_STORE_INVALID_NODE if not found,
   * else the attribute node ID is returned.
   */
  attribute_store_node_t find(attribute_store_node_t parent, search_key_t key)
  {
    attribute_store_node_t found_node = ATTRIBUTE_STORE_INVALID_NODE;

    bool input_valid = attribute_store_node_exists(parent);
    if (input_valid == false) {
      sl_log_debug(LOG_TAG, "parent node not found");
    } else {
      found_node = attribute_store_get_node_child_by_value(
        parent,
        this->node_type,
        DESIRED_ATTRIBUTE,
        reinterpret_cast<uint8_t *>(&key),
        sizeof(search_key_t),
        0);
      if (found_node == ATTRIBUTE_STORE_INVALID_NODE) {
        sl_log_debug(LOG_TAG, "node not found");
      }
    }

    return found_node;
  }

  /**
   * @brief Retrive the count of child entities.
   *
   * @param parent
   * @param key
   * @return size_t 0 if no children are found, else the child count.
   */
  size_t child_count(attribute_store_node_t parent, search_key_t key)
  {
    size_t count = 0U;

    attribute_store_node_t found_node = this->find(parent, key);

    if (found_node != ATTRIBUTE_STORE_INVALID_NODE) {
      count = attribute_store_get_node_child_count(found_node);
    }

    return count;
  }

  /**
   * @brief Find the search key of the child entity based on the index.
   *
   * @param parent
   * @param child_index
   * @param key
   * @return sl_status_t SL_STATUS_OK if found and search key is populated,
   * SL_STATUS_NOT_FOUND if the entity is not found, or SL_STATUS_FAIL if
   * reading search key failed.
   */
  sl_status_t find_by_index(attribute_store_node_t parent,
                            size_t child_index,
                            search_key_t *key)
  {
    sl_status_t status = SL_STATUS_OK;

    attribute_store_node_t found_child
      = attribute_store_get_node_child_by_type(parent,
                                               this->node_type,
                                               (uint32_t)child_index);
    if (found_child == ATTRIBUTE_STORE_INVALID_NODE) {
      status = SL_STATUS_NOT_FOUND;
    } else {
      uint8_t child_size(sizeof(search_key_t));
      status = attribute_store_get_node_attribute_value(
        found_child,
        DESIRED_ATTRIBUTE,
        reinterpret_cast<uint8_t *>(key),
        &child_size);
    }

    return status;
  }

  /**
   * @brief Create a new entity based on the search key.
   *
   * @param parent
   * @param key
   * @return sl_status_t SL_STATUS_OK if creation process succeeded,
   * SL_STATUS_ALREADY_EXISTS if the entity already exists, or SL_STATUS_FAIL
   * if the create process failed.
   */
  sl_status_t create(attribute_store_node_t parent, search_key_t key)
  {
    sl_status_t status                = SL_STATUS_OK;
    attribute_store_node_t found_node = ATTRIBUTE_STORE_INVALID_NODE;

    found_node = this->find(parent, key);
    if (found_node != ATTRIBUTE_STORE_INVALID_NODE) {
      status = SL_STATUS_ALREADY_EXISTS;
    } else {
      found_node = attribute_store_add_node(this->node_type, parent);
      if (found_node == ATTRIBUTE_STORE_INVALID_NODE) {
        status = SL_STATUS_FAIL;
      }
    }

    if (status == SL_STATUS_OK) {
      uint8_t key_size = sizeof(search_key_t);

      status = attribute_store_set_node_attribute_value(
        found_node,
        DESIRED_ATTRIBUTE,
        reinterpret_cast<uint8_t *>(&key),
        key_size);
    }

    return status;
  }

  /**
   * @brief Read information of the entity.
   *
   * @param parent
   * @param key
   * @param data
   * @return sl_status_t SL_STATUS_OK if entity is read successfully,
   * SL_STATUS_NULL_POINTER if invalid data parameter to populate is provided,
   * SL_STATUS_NOT_FOUND if the entity is not found, or SL_STATUS_FAIL
   * if the read process failed.
   */
  sl_status_t
    read(attribute_store_node_t parent, search_key_t key, data_t *const data)
  {
    sl_status_t status                = SL_STATUS_OK;
    attribute_store_node_t found_node = ATTRIBUTE_STORE_INVALID_NODE;

    if (data == nullptr) {
      status = SL_STATUS_NULL_POINTER;
    } else {
      found_node = this->find(parent, key);
      if (found_node == ATTRIBUTE_STORE_INVALID_NODE) {
        status = SL_STATUS_NOT_FOUND;
      }
    }

    if (status == SL_STATUS_OK) {
      uint8_t data_size = sizeof(data_t);
      status            = attribute_store_get_node_attribute_value(
        found_node,
        REPORTED_ATTRIBUTE,
        reinterpret_cast<uint8_t *>(data),
        &data_size);
    }

    return status;
  }

  /**
   * @brief Write information to the entity.
   *
   * @param parent
   * @param key
   * @param data
   * @return sl_status_t SL_STATUS_OK if entity is written successfully,
   * SL_STATUS_NULL_POINTER if invalid data parameter to write is provided,
   * SL_STATUS_NOT_FOUND if the entity is not found, or SL_STATUS_FAIL
   * if the write process failed.
   */
  sl_status_t
    write(attribute_store_node_t parent, search_key_t key, const data_t *data)
  {
    sl_status_t status                = SL_STATUS_OK;
    attribute_store_node_t found_node = ATTRIBUTE_STORE_INVALID_NODE;

    if (data == nullptr) {
      status = SL_STATUS_NULL_POINTER;
    } else {
      found_node = this->find(parent, key);
      if (found_node == ATTRIBUTE_STORE_INVALID_NODE) {
        status = SL_STATUS_NOT_FOUND;
      }
    }

    if (status == SL_STATUS_OK) {
      uint8_t data_size = sizeof(data_t);
      status            = attribute_store_set_node_attribute_value(
        found_node,
        REPORTED_ATTRIBUTE,
        reinterpret_cast<const uint8_t *>(data),
        data_size);
    }

    return status;
  }

  /**
   * @brief Remove the entity from the store.
   *
   * @param parent
   * @param key
   * @return sl_status_t SL_STATUS_OK if entity is deleted successfully,
   * SL_STATUS_NOT_FOUND if the entity is not found, or SL_STATUS_FAIL
   * if the delete process failed.
   */
  sl_status_t remove(attribute_store_node_t parent, search_key_t key)
  {
    sl_status_t status = SL_STATUS_OK;

    attribute_store_node_t found_node = this->find(parent, key);

    if (found_node != ATTRIBUTE_STORE_INVALID_NODE) {
      status = attribute_store_delete_node(found_node);
    } else {
      status = SL_STATUS_NOT_FOUND;
      sl_log_debug(LOG_TAG, "node to delete not found");
    }

    return status;
  }
};

class NetworkStoreHelper : public AttrStoreHelper<uint8_t, zigpc_network_data_t>
{
  public:
  NetworkStoreHelper() :
    AttrStoreHelper<uint8_t, zigpc_network_data_t>(ZIGPC_DS_TYPE_NETWORK)
  {}
};

class DeviceStoreHelper :
  public AttrStoreHelper<zigbee_eui64_uint_t, zigpc_device_data_t>
{
  public:
  DeviceStoreHelper() :
    AttrStoreHelper<zigbee_eui64_uint_t, zigpc_device_data_t>(
      ZIGPC_DS_TYPE_DEVICE)
  {}
};

class EndpointStoreHelper :
  public AttrStoreHelper<zigbee_endpoint_id_t, zigpc_endpoint_data_t>
{
  public:
  EndpointStoreHelper() :
    AttrStoreHelper<zigbee_endpoint_id_t, zigpc_endpoint_data_t>(
      ZIGPC_DS_TYPE_ENDPOINT)
  {}
};

class ClusterStoreHelper :
  public AttrStoreHelper<zcl_cluster_id_t, zigpc_cluster_data_t>
{
  public:
  ClusterStoreHelper() :
    AttrStoreHelper<zcl_cluster_id_t, zigpc_cluster_data_t>(
      ZIGPC_DS_TYPE_CLUSTER)
  {}
};

#endif  // __cplusplus

#endif  //ZIGPC_DATASTORE_HELPER_HPP

/** @} end zigpc_datastore_helper */
