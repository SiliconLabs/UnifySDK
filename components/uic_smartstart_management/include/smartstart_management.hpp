/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file smartstart_management.hpp
 * @defgroup smartstart SmartStart
 * @ingroup uic_components
 * @brief SmartStart management component
 *
 * This component is responsible for reacting on SmartStart network updates
 * and keeping a local SmartStart cache updated as well as ucl/SmartStart/List
 * MQTT topic.
 * @{
 *
 * @startuml
   participant MQTT
   participant SmartStartManagement as SSM
   participant ProtocolControllerSmartStartManagement as PCSSM

   note over SSM: Init
   SSM         -> MQTT : Subscribe to <i>ucl/SmartStart/List</i>
   MQTT        -> SSM  : onMqttPublish(<i>ucl/SmartStart/List</i>)
   note over SSM       : Update smartstart_cache
   SSM         -> PCSSM: on_smartstart_state_changed()
   note over PCSSM : Enable/Disable Inclusion of SmartStart Devices
   ... Time goes by ...
   group Node Added from ProtocolController
     note over PCSSM   : Node added with SmartStart
     PCSSM     -> SSM  : get_cache_entry(unid)
     note over PCSSM   : If node is found in cache continue to add the node
     PCSSM     -> SSM  : notify_node_added(key, unid)
     SSM       -> MQTT : Publish(<i>ucl/SmartStart/List/Update</i>, data)\ndata contains the updated    SmartStart entry,\nthat has been added
     MQTT        -> SSM  : onMqttPublish(<i>ucl/SmartStart/List</i>)
     note over SSM       : Update smartstart_cache
   end
   group Node Removed from ProtocolController
     note over PCSSM   : Node Removed from Network
     PCSSM     -> SSM  : notify_node_removed(unid)
     SSM       -> MQTT : Publish(<i>ucl/SmartStart/List/Update</i>, data)\ndata contains the updated    SmartStart entry,\nthat has been removed
     MQTT        -> SSM  : onMqttPublish(<i>ucl/SmartStart/List</i>)
     note over SSM       : Update smartstart_cache
   end

   group Node Added in SmartStartList
     MQTT      -> SSM  : onMqttPublish(<i>ucl/SmartStart/List</i>)
     note over SSM     : Update smartstart_cache
     SSM       -> PCSSM: on_smartstart_state_changed()
   end
   group Node Removed from ProtocolController
     MQTT      -> SSM  : onMqttPublish(<i>ucl/SmartStart/List</i>)
     note over SSM     : Update smartstart_cache
     SSM       -> PCSSM: on_smartstart_state_changed()
   end
   @enduml
 */

#ifndef SMARTSTART_MANAGEMENT_HPP
#define SMARTSTART_MANAGEMENT_HPP

#include <unordered_map>
#include <string>
#include <variant>
#include <functional>
#include <vector>

#include "sl_log.h"

namespace smartstart
{
/**
 * @brief Enum class representing keys for querying SmartStart List.
 */
enum class QueryKey { dsk, include, device_unid };
/**
 * @brief Enum class representing types for querying SmartStart List.
 */
enum class QueryType { exact, partial };

using QueryValue              = std::variant<bool, std::string>;
using notification_function_t = std::function<void(bool)>;
/**
 * @class smartstart::Query
 * @brief Class that contains key, type, an value for querying SmartStart List.
 */
class Query
{
  public:
  QueryType query_type;
  QueryKey query_key;
  QueryValue query_value;
  /**
   * @brief smartstart::Query constructor.
   * @param type Types of fields to query. Check \ref smartstart::QueryType for available options.
   * @param key Types of keys to query. Check \ref smartstart::QueryKey for available options.
   * @param value Types of value to be searched. Currently only string and bool is supported.
   */
  Query(QueryType type, QueryKey key, const QueryValue &value) :
    query_type {type}, query_key {key}, query_value {value}
  {}
};

/**
 * @class smartstart::Entry
 * @brief The unit data type stored in SmartStart List as value.
 */
class Entry
{
  public:
  std::string dsk;
  bool include;
  std::string protocol_controller_unid;
  std::string device_unid;
  std::vector<std::string> preferred_protocols;
  /**
   * @brief smartstart::Entry constructor.
   * @param dsk DSK as string, which is used as a key in the SmartStart cache.
   * @param include Boolean. Whether or not this device is ready to join a network.
   * @param protocol_controller_unid Unid as string. Which protocol controller this device is supposed to be included.
   * @param device_unid Device Unid as string. Empty by default and is updated with a successful inclusion.
   */
  Entry() = default;
  Entry(const std::string &dsk,
        bool include,
        const std::string &protocol_controller_unid,
        const std::string &device_unid) :
    dsk {dsk},
    include {include},
    protocol_controller_unid {protocol_controller_unid},
    device_unid {device_unid}
  {}
};

/**
 * @class smartstart::Management
 * @brief Class that provides APIs to add, update, and access SmartStart List. Also it offers
 *        callback for notifying that entries await inclusion.
 */
class Management
{
  private:
  /**
   * @brief SmartStart List cache map, key is DSK as a string.
   */
  std::unordered_map<std::string, Entry> _smartstart_cache;

  std::string _protocol_controller_unid;

  /**
   * @brief Function called when there are entries waiting for inclusion in the SmartStart list.
   */
  notification_function_t _notify_has_entries_awaiting_inclusion;

  static Management *_instance;

  Management() = default;

  public:
  /**
   * @brief Disable assign and constructor.
   */
  Management(Management &other) = delete;
  void operator=(const Management &) = delete;
  /**
   * @brief Get the singleton instance.
   */
  static Management *get_instance();

  /**
   * @brief Initialize function that takes protocol controller unid and a
   *         callback function signaling when entries awaits inclusion.
   * @param protocol_controller_unid Protocol Controller Unid as string in which this SmartStart Management belongs to.
   * @param has_entries_awaiting_inclusion_callback Function callback regsitered and called when there are entries awaiting inclusion in the SmartStart List.
   */
  sl_status_t init(
    const std::string &protocol_controller_unid,
    notification_function_t const &has_entries_awaiting_inclusion_callback);
  sl_status_t teardown();
  /**
   * @brief Get the cache entries with given query conditions.
   *
   * @param query Query criterias defined for searching.
   */
  std::vector<Entry> get_cache_entries(const Query &query);
  /**
   * @brief Update the internal cache of SmartStartList.
   *
   * This function parses the SmartStartList to internal representation,
   * stores the data in smartstart_cache, and signals other components
   * if there are entries waiting for inclusion for the protocol controller
   * by calling notify_has_entries_for_inclusion.
   *
   * @param smartstart_list String with JSON representation of SmartStart list.
   */
  sl_status_t update_smartstart_cache(const std::string &smartstart_list);

  /**
   * @brief Polling function which returns True if there is any entries with
   * Include == true. This could be useful e.g. when just boot.
   */
  bool has_entries_awaiting_inclusion();

  /**
   * @brief Called when a node has been added and SSM will then update the cache map.
   * @param dsk DSK as string to be added.
   * @param device_unid Device Unid as string to be updated in the corresponding entry.
   */
  sl_status_t notify_node_added(const std::string &dsk,
                                const std::string &device_unid);
  /**
   * @brief Called when a node has been removed and SSM will then update the cache map.
   * @param unid Unid as string to be removed.
   */
  sl_status_t notify_node_removed(const std::string &unid);

  const std::unordered_map<std::string, Entry> &get_cache() const;
};

} /* namespace smartstart */

/** @} end smartstart management */

#endif  //SMARTSTART_MANAGEMENT_H