/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup unify_node_state_monitor
 * @brief Monitors when Unify nodes are added, removed, or failing
 *
 * Monitor Unify nodes on MQTT and support following:
 * * Unify nodes are added
 * * Unify nodes are removed
 * * Unify nodes are failing
 *
 * @{
 */

#ifndef UNIFY_NODE_STATE_MONITOR_H
#define UNIFY_NODE_STATE_MONITOR_H
#include "sys/clock.h"
#include "ctimer.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <nlohmann/json.hpp>
namespace unify::node_state_monitor
{
/** Endpoint type. */
using endpoint_id_t = uint16_t;

/** Default threshold for how long time a node shall have been without change
 * before on_unify_node_added is called. */
static constexpr clock_time_t DEFAULT_NODE_STATE_FUNCTIONAL_THRESHOLD
  = 1 * CLOCK_SECOND;
/** Default timeout for the timer that checks if on_unify_node_added shall be
 * called. */
static constexpr clock_time_t DEFAULT_NODE_STATE_FUNCTIONAL_TIMEOUT
  = 1 * CLOCK_SECOND;

/**
 * @brief Unify Cluster.
 */
class cluster
{
  public:
  /** ZCL Cluster ID. */
  std::string cluster_name;
  /** List of attributes provided by the Cluster. */
  std::unordered_set<std::string> attributes;
  /** List of supported commands provided by the Cluster. */
  std::unordered_set<std::string> supported_commands;
  /** List of supported generated commands provided by the Cluster. */
  std::unordered_set<std::string> generated_commands;
  /**
   * @brief Construct a new cluster object.
   *
   * @param cluster_name ZCL Cluster ID.
   */
  explicit cluster(const std::string &cluster_name) :
    cluster_name(cluster_name) {};

  /**
   * @brief Get a string representation of the cluster.
   *
   * @return std::string.
   */
  std::string to_string() const;
};

/**
 * @brief Unify Endpoint.
 */
class endpoint
{
  public:
  /** Endpoint identifier. */
  endpoint_id_t endpoint_id;
  /** Map of all clusters under the endpoint. The key to the map is the Cluster
   * ID. */
  std::unordered_map<std::string, cluster> clusters;
  /**
   * @brief Construct a new endpoint object.
   *
   * @param id Endpoint identifier.
   */
  explicit endpoint(endpoint_id_t id) : endpoint_id(id) {};
  /**
   * @brief Create a new cluster in the cluster map and return a reference to
   * it. If the cluster_name already exist in the map, then return a reference to
   * that instead.
   *
   * @param cluster_name ZCL Cluster ID.
   * @return cluster& Reference to the Cluster.
   */
  cluster &emplace_cluster(const std::string &cluster_name);

  /**
   * @brief Get a string representation of the endpoint
   *
   * @return std::string
   */
  std::string to_string() const;
};

/**
 * @brief Unify Node.
 */
class node
{
  public:
  /** Node UNID. */
  std::string unid;
  /** Node State. */
  uint32_t state = 4;
  /** Node Security. */
  uint32_t security_info;
  /** Map of all endpoints under the node, the key to the map is the endpoint
   * identifier. */
  std::unordered_map<endpoint_id_t, endpoint> endpoints;
  /** Time for last update of the node. */
  clock_time_t last_update;
  /**
   * @brief Construct a new node object.
   *
   * When creating a new node, the state is set to
   * ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE.
   *
   * @param unid UNID of the node.
   */
  explicit node(const std::string &unid) : unid(unid)
  {
    last_update = clock_time();
  };

  /**
   * @brief Create a new endpoint in the endpoint map and return a reference to
   * it. If the endpoint_id alredy exist in the map return a reference to that
   * instead.
   *
   * @param endpoint_id
   * @return endpoint&
   */
  endpoint &emplace_endpoint(endpoint_id_t endpoint_id);

  /**
   * @brief Get a string representation of the node.
   *
   * @return std::string.
   */
  std::string to_string() const;
};

/**
 * @brief Interface class for the \ref node_state_monitor.
 *
 * This class is to be implemented by users of the \ref node_state_monitor, and
 * should be set in the \ref node_state_monitor::set_interface.
 */
class node_state_monitor_interface
{
  public:
  /**
   * @brief Unify Node is added.
   *
   * Called when a Unify Node is added and resolved, i.e. the node object should
   * be complete with all andpoints and clusters.
   *
   * @param node Node that is added.
   */
  virtual void on_unify_node_added(const node &node) = 0;

  /**
   * @brief Unify Node is removed.
   *
   * Called when a Unify Node is deleted.
   * @param unid UNID of node that is deleted.
   */
  virtual void on_unify_node_removed(const std::string &unid) = 0;

  /**
   * @brief Unify Node state changed.
   *
   * Called when the state changes on a Unify Node. This will likely be called
   * before the \ref on_unify_node_added, as there are mutiple states a goes
   * through before it is complete with endpoints and clusters.
   * @param node
   */
  virtual void on_unify_node_state_changed(const node &node) = 0;

  virtual ~node_state_monitor_interface() = default;
};

/**
 * The role of the node state monitor is to subscribe to the
 * ucl/by-unid/<unid>/State topic. It will check when nodes a are changing
 * state, and create \ref node object, populated with \ref endpoint, state and
 * security information. When a new node is found / deleted or updated a
 * callback is fired containing the node object. The node state monitor should
 * only fire a callback once all information has been collected.
 *
 * Because it is not possible to determine if all MQTT topics for a given node
 * is received, when a NetworkStatus: "Online Functional" is received, the
 * node_state_monitor delays the \ref
 * node_state_monitor_interface::on_unify_node_added callback for \ref
 * node_state_monitor::node_state_functional_threshold time, assuming that all
 * MQTT topics related to the node_state is received within this time. The time
 * for this may be configured using \ref
 * node_state_monitor::set_node_state_functional_threshold.
 */
class node_state_monitor
{
  public:
  /**
   * @brief Get the instance object.
   *
   * @return node_state_monitor&.
   */
  static node_state_monitor &get_instance()
  {
    // Singleton instance
    static node_state_monitor instance;
    return instance;
  };

  /**
   * @brief Get a reference to the nodes object containing all the nodes.
   * 
   * @return const std::unordered_map<std::string, node>& map of all nodes.
   */
  const std::unordered_map<std::string, node> &get_nodes() const
  {
    return nodes;
  };

  /**
   * @brief Set the node state functional threshold.
   *
   * The threshold is the time a node shall have been without change after
   * NetworkStatus "Online Functional" is received, before the
   * on_unify_node_added is called.
   *
   * Default value is \ref DEFAULT_NODE_STATE_FUNCTIONAL_THRESHOLD.
   *
   * @param t Threshold
   */
  void set_node_state_functional_threshold(clock_time_t t);

  /**
   * @brief Set the node state functional timeout time
   *
   * This is the timeout for monitoring the threshold \ref
   * set_node_state_functional_threshold.
   *
   * Default value is \ref DEFAULT_NODE_STATE_FUNCTIONAL_TIMEOUT.
   * @param t Timeout
   */
  void set_node_state_functional_timeout_time(clock_time_t t);

  /**
   * @brief Set the interface object.
   *
   * @param _interface Interface object to set.
   */
  void set_interface(node_state_monitor_interface *_interface)
  {
    interface = _interface;
  }

  private:
  // Map of all nodes <UNID, node>
  std::unordered_map<std::string, node> nodes;
  // Set of nodes pending a on_unify_node_added callback.
  std::unordered_set<std::string> nodes_pending_node_added;
  // Timer used for delaying on_unify_node_added callback.
  struct ctimer node_state_functional_timer;
  // Threshold for delaying on_unify_node_added, the time a node shall have been
  // unchanged before a on_unify_node_added callback is issued.
  clock_time_t node_state_functional_threshold
    = DEFAULT_NODE_STATE_FUNCTIONAL_THRESHOLD;
  // Timeout for the timer that monitors node change thresholds.
  clock_time_t node_state_functional_timeout_time
    = DEFAULT_NODE_STATE_FUNCTIONAL_TIMEOUT;

  node_state_monitor_interface *interface;
  /**
   * @brief Callback function for when state messages are received from MQTT.
   *
   * @param topic MQTT Topic
   * @param message MQTT Message
   * @param message_length MQTT Message Length
   */
  static void on_mqtt_node_state_message(const char *topic,
                                         const char *message,
                                         const size_t message_length);

  /**
   * @brief Callback function for when cluster supported commands message is
   * received from MQTT.
   *
   * @param topic MQTT Topic
   * @param message MQTT Message
   * @param message_length MQTT Message Length.
   */
  static void on_mqtt_cluster_supported_commands_message(
    const char *topic, const char *message, const size_t message_length);

  /**
   * @brief Callback function for when cluster supported generated commands
   * message is received from MQTT.
   *
   * @param topic MQTT Topic
   * @param message MQTT Message
   * @param message_length MQTT Message Length.
   */
  static void on_mqtt_cluster_supported_generated_commands_message(
    const char *topic, const char *message, const size_t message_length);

  /**
   * @brief Callback function for when cluster attributes message is received
   * from MQTT.
   *
   * @param topic MQTT Topic
   * @param message MQTT Message
   * @param message_length MQTT Message Length.
   */
  static void on_mqtt_cluster_attributes(const char *topic,
                                         const char *message,
                                         const size_t message_length);

  /**
   * @brief Create a new node in the node map and return a reference to it. If
   * the unid alredy exist in the map return a reference to that instead.
   */
  node &emplace_node(const std::string &unid);

  /**
   * @brief Called from \ref node_state_monitor::on_mqtt_node_state_message when
   * a node state changes.
   *
   * @param unid UNID of node.
   * @param jsn JSON payload from MQTT.
   */
  void on_node_state_changed(const std::string &unid,
                             const nlohmann::json &jsn);
  /**
   * @brief Called from \ref node_state_monitor::on_mqtt_node_state_message when a
   * node is removed.
   *
   * @param unid UNID of node removed
   */
  void on_node_removed(const std::string &unid);

  /**
   * @brief Called from \ref node_state_monitor::on_mqtt_cluster_attributes when
   * an attribute is added to the cluster.
   *
   * @param unid UNID of the node.
   * @param endpoint_id Endpoint ID.
   * @param cluster_name Cluster ID.
   * @param attribute Attribute that is added.
   */
  void on_cluster_attribute_added(const std::string &unid,
                                  endpoint_id_t endpoint_id,
                                  const std::string &cluster_name,
                                  const std::string &attribute);

  /**
   * @brief Called from \ref node_state_monitor::on_mqtt_cluster_attributes when
   * an attribute is removed from the cluster.
   *
   * @param unid UNID of the node.
   * @param endpoint_id Endpoint ID.
   * @param cluster_name Cluster ID.
   * @param attribute Attribute that is removed.
   */
  void on_cluster_attribute_removed(const std::string &unid,
                                    endpoint_id_t endpoint_id,
                                    const std::string &cluster_name,
                                    const std::string &attribute);

  /**
   * @brief Called from \ref
   * node_state_monitor::on_mqtt_cluster_supported_commands_message when
   * supported commands are added.
   *
   * @param unid UNID of the node.
   * @param endpoint_id Endpoint ID.
   * @param cluster_name Cluster ID.
   * @param jsn JSON object received (payload).
   */
  void on_cluster_supported_commands_added(const std::string &unid,
                                           endpoint_id_t endpoint_id,
                                           const std::string &cluster_name,
                                           const nlohmann::json &jsn);

  /**
   * @brief Called from \ref
   * node_state_monitor::on_mqtt_cluster_supported_commands_message when
   * supported commands are removed.
   *
   * @param unid UNID of the node.
   * @param endpoint_id Endpoint ID.
   * @param cluster_name Cluster ID.
   */
  void on_cluster_supported_commands_removed(const std::string &unid,
                                             endpoint_id_t endpoint_id,
                                             const std::string &cluster_name);

  /**
   * @brief Called from \ref
   * node_state_monitor::on_mqtt_cluster_supported_generated_commands_message
   * when supported generated commands are added.
   *
   * @param unid UNID of the node.
   * @param endpoint_id Endpoint ID.
   * @param cluster_name Cluster ID.
   * @param jsn JSON object received (payload).
   */
  void on_cluster_generated_commands_added(const std::string &unid,
                                           endpoint_id_t endpoint_id,
                                           const std::string &cluster_name,
                                           const nlohmann::json &jsn);

  /**
   * @brief Called from \ref
   * node_state_monitor::on_mqtt_cluster_supported_generated_commands_message
   * when supported generated commands are removed.
   *
   * @param unid UNID of the node.
   * @param endpoint_id Endpoint ID.
   * @param cluster_name Cluster ID.
   */
  void on_cluster_generated_commands_removed(const std::string &unid,
                                             endpoint_id_t endpoint_id,
                                             const std::string &cluster_name);

  /** Called when timeout occurs for node_state_functional_timer. */
  static void on_node_state_functional_timeout(void *context);

  // Delete copy constructor
  node_state_monitor(const node_state_monitor &) = delete;
  node_state_monitor const &operator=(const node_state_monitor &) = delete;
  /** Private Constructor as it is a singleton. */
  node_state_monitor();
  /** Private Destructor. */
  ~node_state_monitor();
};

}  // namespace unify::node_state_monitor

#endif  //UNIFY_NODE_STATE_MONITOR_H
/** @} end unify_node_state_monitor */
