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
 * @defgroup zigpc_datastore_accessor ZigPC datastore accessor templates.
 * @ingroup zigpc_datastore
 * @brief Templates and datastore accessor classes.
 *
 * @{
 */

#ifndef ZIGPC_DATASTORE_ACCESSOR_HPP
#define ZIGPC_DATASTORE_ACCESSOR_HPP

#include <sstream>

// Unify shared includes
#include <sl_status.h>
#include <sl_log.h>
#include <attribute_store.h>

// ZigPC includes
#include "zigpc_datastore.h"
#include "zigpc_datastore_id.h"

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
template<typename search_key_t, typename data_t> class StoreAccessor
{
  private:
  attribute_store_type_t node_type; /**< Entity type to be used */
  std::string label;                /**< Entity label to use in logs */

  public:
  explicit StoreAccessor(attribute_store_type_t node_type, std::string label) :
    node_type(node_type), label(label)
  {}

  virtual ~StoreAccessor() = default;

  inline std::string get_label(void) const
  {
    return label;
  }

  inline attribute_store_type_t get_node_type(void) const
  {
    return node_type;
  }

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
    auto found_node = ATTRIBUTE_STORE_INVALID_NODE;
    
    std::stringstream ss;
    ss << key;
    std::string key_str = ss.str();

    bool input_valid = attribute_store_node_exists(parent);
    if (input_valid == false) {
      sl_log_warning(LOG_TAG,
                     "%s: %s entity[NodeId:%u] Failed to find parent node",
                     this->label.c_str(),
                     key_str.c_str());
    } else {
      found_node = attribute_store_get_node_child_by_value(
        parent,
        this->node_type,
        DESIRED_ATTRIBUTE,
        reinterpret_cast<uint8_t *>(&key),
        sizeof(search_key_t),
        0);
    }

    return found_node;
  }

  /**
   * @brief Retrive the count of child entities by type.
   *
   * @param parent  Parent node ID to look under.
   * @param key     Search key to find entity node under parent.
   * @param type    Child node type to find under entity.
   * @return size_t 0 if no children are found, else the child count.
   */
  size_t child_count(attribute_store_node_t parent,
                     search_key_t key,
                     attribute_store_type_t child_type)
  {
    size_t child_count = 0U;

    attribute_store_node_t found_node = this->find(parent, key);
    if (found_node != ATTRIBUTE_STORE_INVALID_NODE) {
      attribute_store_node_t child_node
        = attribute_store_get_node_child_by_type(found_node,
                                                 child_type,
                                                 child_count);

      bool child_valid = !(child_node == ATTRIBUTE_STORE_INVALID_NODE);
      while (child_valid) {
        child_count++;
        child_node  = attribute_store_get_node_child_by_type(found_node,
                                                            child_type,
                                                            child_count);
        child_valid = !(child_node == ATTRIBUTE_STORE_INVALID_NODE);
      }
    }

    return child_count;
  }

  /**
   * @brief Find the search key of the child entity based on the index.
   *
   * @param parent
   * @param index
   * @param key
   * @return sl_status_t SL_STATUS_OK if found and search key is populated,
   * SL_STATUS_NOT_FOUND if the entity is not found, or SL_STATUS_FAIL if
   * reading search key failed.
   */
  sl_status_t find_by_index(attribute_store_node_t parent,
                            size_t index,
                            search_key_t *key)
  {
    sl_status_t status = SL_STATUS_OK;

    attribute_store_node_t found_child
      = attribute_store_get_node_child_by_type(parent,
                                               this->node_type,
                                               (uint32_t)index);
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
    sl_status_t status = SL_STATUS_OK;
    auto found_node    = ATTRIBUTE_STORE_INVALID_NODE;

    std::stringstream ss;
    ss << key;
    std::string key_str = ss.str();

    found_node = this->find(parent, key);
    if (found_node != ATTRIBUTE_STORE_INVALID_NODE) {
      status = SL_STATUS_ALREADY_EXISTS;
    } else {
      found_node = attribute_store_add_node(this->node_type, parent);
      if (found_node == ATTRIBUTE_STORE_INVALID_NODE) {
        status = SL_STATUS_FAIL;
        sl_log_error(LOG_TAG,
                     "%s: Failed to create entity %s",
                     this->label.c_str(),
                     key_str.c_str());
      }
    }

    if (status == SL_STATUS_OK) {
      uint8_t key_size = sizeof(search_key_t);

      status = attribute_store_set_node_attribute_value(
        found_node,
        DESIRED_ATTRIBUTE,
        reinterpret_cast<uint8_t *>(&key),
        key_size);
      if (status != SL_STATUS_OK) {
        sl_log_error(LOG_TAG,
                     "%s: %s entity[NodeId:%u]: Failed to populate entity data",
                     this->label.c_str(),
                     key_str.c_str(),
                     found_node);
      }
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
    sl_status_t status = SL_STATUS_OK;
    auto found_node    = ATTRIBUTE_STORE_INVALID_NODE;

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
      if (status != SL_STATUS_OK) {
        std::stringstream ss;
        ss << key;
        std::string key_str = ss.str();
        sl_log_error(LOG_TAG,
                     "%s: %s entity[NodeId:%u]: Failed to read entity data",
                     this->label.c_str(),
                     key_str.c_str(),
                     found_node);
      }
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
    sl_status_t status = SL_STATUS_OK;
    auto found_node    = ATTRIBUTE_STORE_INVALID_NODE;

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
      if (status != SL_STATUS_OK) {
        std::stringstream ss;
        ss << key;
        std::string key_str = ss.str();
        sl_log_error(LOG_TAG,
                     "%s: %s entity[NodeId:%u]: Failed to write entity data",
                     this->label.c_str(),
                     key_str.c_str(),
                     found_node);
      }
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

    std::stringstream ss;
    ss << key;
    std::string key_str = ss.str();

    attribute_store_node_t found_node = this->find(parent, key);

    if (found_node != ATTRIBUTE_STORE_INVALID_NODE) {
      status = attribute_store_delete_node(found_node);
      if (status != SL_STATUS_OK) {
        sl_log_error(LOG_TAG,
                     "%s: %s entity[NodeId:%u]: Failed to remove entity",
                     this->label.c_str(),
                     key_str.c_str(),
                     found_node);
      }
    } else {
      status = SL_STATUS_NOT_FOUND;
    }

    return status;
  }

  /**
   * @brief Remove all child entities from the store.
   *
   * @param parent
   * @param key
   * @return sl_status_t SL_STATUS_OK if child entities are deleted
   * successfully, SL_STATUS_NOT_FOUND if the entity is not found, or
   * SL_STATUS_FAIL if the delete process failed.
   */
  sl_status_t remove_children(attribute_store_node_t parent, search_key_t key)
  {
    sl_status_t status = SL_STATUS_OK;

    attribute_store_node_t found_node = this->find(parent, key);

    std::stringstream ss;
    ss << key;
    std::string key_str = ss.str();
    if (found_node == ATTRIBUTE_STORE_INVALID_NODE) {
      status = SL_STATUS_NOT_FOUND;
      sl_log_warning(LOG_TAG,
                     "%s: Failed to find entity %s to remove children under",
                     this->label.c_str(),
                     key_str.c_str());
    } else {
      size_t child_count = attribute_store_get_node_child_count(found_node);
      for (size_t i = 0; i < child_count; i++) {
        attribute_store_node_t child_node
          = attribute_store_get_node_child(found_node, i);
        if (child_node != ATTRIBUTE_STORE_INVALID_NODE) {
          status = attribute_store_delete_node(child_node);
          if (status != SL_STATUS_OK) {
            sl_log_error(LOG_TAG,
                         "%s: Failed to remove child[%u] remove entity",
                         this->label.c_str(),
                         i);
          }
        }
      }
    }

    return status;
  }

  /**
   * @brief Get string representation of entity in byte buffer representation.
   *
   * NOTE: Can be overriden.
   *
   * @param parent
   * @param key
   * @return std::string
   */
  virtual std::string to_str(attribute_store_node_t parent, search_key_t key)
  {
    std::ostringstream ss;
    data_t data;

    sl_status_t status = this->read(parent, key, &data);
    if (status == SL_STATUS_OK) {
      ss << this->label << ": ";
      uint8_t *buffer = reinterpret_cast<uint8_t *>(&data);
      for (size_t i = 0U; i < sizeof(data_t); i++) {
        ss << *buffer << " ";
        buffer++;
      }
    }

    return ss.str();
  }
};

template<typename list_entry_t> class StoreListAccessor
{
  private:
  StoreAccessor<list_entry_t, uint8_t> list_entry;

  public:
  explicit StoreListAccessor(attribute_store_type_t node_type,
                             std::string label) :
    list_entry(node_type, label)
  {}

  virtual ~StoreListAccessor() = default;

  /**
   * @brief Read the list of entries.
   *
   * @param parent        Parent node to read entries under.
   * @param list          Reference to list to write.
   * @param list_capacity Number of items in the list.
   * @return sl_status_t SL_STATUS_OK if the list entries are read
   * successfully, SL_STATUS_NOT_FOUND if the parent is not found, or
   * SL_STATUS_NULL_POINTER if invalid list reference is passed, or
   * SL_STATUS_FAIL if the read process failed.
   */
  sl_status_t read_list(attribute_store_node_t list_parent,
                        list_entry_t *const list,
                        size_t list_capacity)
  {
    sl_status_t status = SL_STATUS_OK;

    if (list == nullptr) {
      status = SL_STATUS_NULL_POINTER;
    }
    for (size_t i = 0; (status == SL_STATUS_OK) && (i < list_capacity); i++) {
      status = this->list_entry.find_by_index(list_parent, i, list + i);
    }
    return status;
  }

  /**
   * @brief Create the list into the store if it doesn't exist already.
   *
   * NOTE: if this function returns SL_STATUS_ALREADY_EXISTS, ensure that
   * the parent entity has it's children of this entity type removed.
   *
   * @param parent      Parent node to read entries under.
   * @param list        Reference to list to write.
   * @param list_count  Number of items in the list.
   * @return sl_status_t SL_STATUS_OK if the list entries are added
   * successfully, SL_STATUS_NOT_FOUND if the parent is not found, or
   * SL_STATUS_ALREADY_EXISTS if an array already exists, or
   * SL_STATUS_NULL_POINTER if invalid list reference is passed, or
   * SL_STATUS_FAIL if the create process failed.
   */
  sl_status_t create_list(attribute_store_node_t list_parent,
                          const list_entry_t *list,
                          size_t list_count)
  {
    sl_status_t status = SL_STATUS_OK;

    if (list == nullptr) {
      status = SL_STATUS_NULL_POINTER;
    } else {
      // Ensure there is no existing array under the parent by checking if
      // first element exists.
      attribute_store_node_t child_node
        = attribute_store_get_first_child_by_type(
          list_parent,
          this->list_entry.get_node_type());
      if (child_node != ATTRIBUTE_STORE_INVALID_NODE) {
        status = SL_STATUS_ALREADY_EXISTS;
      }
    }

    for (size_t i = 0; (status == SL_STATUS_OK) && (i < list_count); i++) {
      status = this->list_entry.create(list_parent, list[i]);
    }

    return status;
  }

  /**
   * @brief Remove a list under the node ID passed in.
   *
   * @param parent        Parent Node ID that has the list.
   * @return sl_status_t
   */
  sl_status_t remove_list(attribute_store_node_t list_parent)
  {
    sl_status_t status = SL_STATUS_OK;

    attribute_store_type_t entry_type = this->list_entry.get_node_type();

    if (list_parent != ATTRIBUTE_STORE_INVALID_NODE) {
      size_t list_index = 0U;

      attribute_store_node_t entry_node
        = attribute_store_get_node_child_by_type(list_parent,
                                                 entry_type,
                                                 list_index);
      while ((status == SL_STATUS_OK)
             && entry_node != ATTRIBUTE_STORE_INVALID_NODE) {
        status = attribute_store_delete_node(entry_node);
        list_index++;
        entry_node = attribute_store_get_node_child_by_type(list_parent,
                                                            entry_type,
                                                            list_index);
      }
    }

    return status;
  }

  /* Abstract functions that must be implemented */

  virtual std::string to_str(attribute_store_node_t parent, size_t list_count)
    = 0;
};

/**********************************
 **********************************
 *
 * Accessors used by component
 *
 **********************************
 **********************************/

class NetworkAccessor : public StoreAccessor<uint8_t, zigpc_network_data_t>
{
  public:
  explicit NetworkAccessor();

  /**
   * @brief Return string representation of network entity.
   *
   * @param parent  Parent of network entity (usually attribute tree root)
   * @param index   Network index to print.
   * @return std::string Network data in string form, else empty string if
   * entity is not stored.
   */
  std::string to_str(attribute_store_node_t parent, uint8_t index) override;
};

class DeviceAccessor :
  public StoreAccessor<zigbee_eui64_uint_t, zigpc_device_data_t>
{
  public:
  explicit DeviceAccessor();

  /**
   * @brief Return string representation of device entity.
   *
   * @param parent      Parent of device entity (network entity).
   * @param eui64       Device identifer to search.
   * @return std::string  Device data in string form, else empty string if
   * entity is not stored.
   */
  std::string to_str(attribute_store_node_t parent,
                     zigbee_eui64_uint_t eui64) override;
};

class EndpointAccessor :
  public StoreAccessor<zigbee_endpoint_id_t, zigpc_endpoint_data_t>
{
  public:
  explicit EndpointAccessor();

  /**
   * @brief Return string representation of endpoint entity.
   *
   * @param parent        Parent of endpoint entity (device entity).
   * @param endpoint_id   Endpoint identifer to search.
   * @return std::string  Endpoint data in string form, else empty string if
   * entity is not stored.
   */
  std::string to_str(attribute_store_node_t parent,
                     zigbee_endpoint_id_t endpoint_id) override;
};

class GroupAccessor :
    public StoreAccessor<zigbee_group_id_t, zigpc_group_data_t>
{
    private:
        bool is_reported;

    public:
        explicit GroupAccessor( bool is_reported); 
  
   /**
   * @brief Get the label type of the group.
   *
   * @param is_reported - the reported or desired direction of the group
   * @return std::string
   */
        static inline std::string get_label_type(bool is_reported)
        {
            return is_reported ? "ReportedGroup" : "DesiredGroup";
        }
        
  /**
   * @brief Retrieve the attribute store node type of the group based on the
   * reported/desired type received.
   *
   * @param is_reported
   * @return attribute_store_type_t ZIGPC_DS_TYPE_SERVER_CLUSTER for server side
   * or ZIGPC_DS_TYPE_CLIENT_CLUSTER otherwise.
   */
        static inline attribute_store_type_t
            get_entity_type(bool is_reported)
            {
                return is_reported ? ZIGPC_DS_TYPE_GROUP_REPORTED : ZIGPC_DS_TYPE_GROUP_DESIRED; 
            }
  
  /**
   * @brief Return string representation of group entity.
   *
   * @param parent        Parent of group entity (endpoint entity).
   * @param group_id    Group identifer to search.
   * @return std::string  Group data in string form, else empty string if
   * entity is not stored.
   */
        std::string to_str(attribute_store_node_t parent,
                     zigbee_group_id_t group_id) override;


};

class ClusterAccessor :
  public StoreAccessor<zcl_cluster_id_t, zigpc_cluster_data_t>
{
  private:
  zcl_cluster_side_t cluster_side;

  public:
  explicit ClusterAccessor(zcl_cluster_side_t cluster_side);

  /**
   * @brief Get the label based on the cluster side used.
   *
   * @param cluster_side
   * @return std::string
   */
  static inline std::string get_label_type(zcl_cluster_side_t cluster_side)
  {
    return (cluster_side == ZCL_CLUSTER_SERVER_SIDE) ? "ServerCluster"
                                                     : "ClientCluster";
  }

  /**
   * @brief Retrieve the attribute store node type of the cluster based on the
   * cluster side received.
   *
   * @param cluster_side
   * @return attribute_store_type_t ZIGPC_DS_TYPE_SERVER_CLUSTER for server side
   * or ZIGPC_DS_TYPE_CLIENT_CLUSTER otherwise.
   */
  static inline attribute_store_type_t
    get_entity_type(zcl_cluster_side_t cluster_side)
  {
    return (cluster_side == ZCL_CLUSTER_SERVER_SIDE)
             ? ZIGPC_DS_TYPE_SERVER_CLUSTER
             : ZIGPC_DS_TYPE_CLIENT_CLUSTER;
  }

  /**
   * @brief Return string representation of cluster entity.
   *
   * @param parent        Parent of cluster entity (endpoint entity).
   * @param cluster_id    Cluster identifer to search.
   * @return std::string  Cluster data in string form, else empty string if
   * entity is not stored.
   */
  std::string to_str(attribute_store_node_t parent,
                     zcl_cluster_id_t cluster_id) override;
};

class ClusterAttributeListAccessor :
  public StoreListAccessor<zcl_attribute_id_t>
{
  public:
  explicit ClusterAttributeListAccessor();

  /**
   * @brief Get the label type of the attribute list.
   *
   * @return std::string
   */
  static inline std::string get_label_type(void)
  {
    return "Attribute";
  }

  /**
   * @brief Return string representation of attribute list.
   *
   * @param parent        Parent of attribute list (cluster entity).
   * @param list_count    Count of attribute list entries to print.
   * @return std::string  Attribute list in string form, else empty string if
   * entity is not stored.
   */
  std::string to_str(attribute_store_node_t parent, size_t list_count) override;
};

class BindingListAccessor :
     public StoreListAccessor<std::string>
{
    public:
    explicit BindingListAccessor();
  
    static inline std::string get_label_type(void)
    {
        return "Binding";
    }
  
    std::string to_str(attribute_store_node_t parent, size_t list_count) override;
};

class ClusterCommandListAccessor : public StoreListAccessor<zcl_command_id_t>
{
  private:
  zcl_command_type_t command_type;

  public:
  explicit ClusterCommandListAccessor(zcl_command_type_t command_type);

  /**
   * @brief Get the label type based on the command type passed in.
   *
   * @param command_type
   * @return std::string
   */
  static inline std::string get_label_type(zcl_command_type_t command_type)
  {
    return (command_type == ZCL_CLUSTER_GENERATED_COMMAND) ? "GeneratedCommand"
                                                           : "RecievedCommand";
  }

  /**
   * @brief Retrieve the attribute store node type of the command based on the
   * command type received.
   *
   * @param command_type
   * @return attribute_store_type_t ZIGPC_DS_TYPE_CLUSTER_GENERATED_COMMAND or
   * or ZIGPC_DS_TYPE_CLUSTER_RECEIVED_COMMAND.
   */
  static inline attribute_store_type_t
    get_entity_type(zcl_command_type_t command_type)
  {
    return (command_type == ZCL_CLUSTER_GENERATED_COMMAND)
             ? ZIGPC_DS_TYPE_CLUSTER_GENERATED_COMMAND
             : ZIGPC_DS_TYPE_CLUSTER_RECEIVED_COMMAND;
  }

  /**
   * @brief Return string representation of command list.
   *
   * @param parent        Parent of command list (cluster entity).
   * @param list_count    Count of command list entries to print.
   * @return std::string  Command list in string form, else empty string if
   * entity is not stored.
   */
  std::string to_str(attribute_store_node_t parent, size_t list_count) override;
};

#endif  //ZIGPC_DATASTORE_ACCESSOR_HPP

/** @} end zigpc_datastore_accessor */
