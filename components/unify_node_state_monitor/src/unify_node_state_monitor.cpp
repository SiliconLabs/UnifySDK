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
#include "unify_node_state_monitor.hpp"
#include "uic_mqtt.h"
#include "sl_log.h"
#include "dotdot_mqtt_helpers.hpp"
#include "ctimer.h"
#include "zap-types.h"
#include <nlohmann/json.hpp>
#include <regex>
#include <experimental/iterator>
#include <sstream>
static constexpr const char *LOG_TAG         = "node_state_monitor";
static constexpr const char *UCL_STATE_TOPIC = "ucl/by-unid/+/State";
static constexpr const char *UCL_SUPPORTED_COMMANDS_TOPIC
  = "ucl/by-unid/+/+/+/SupportedCommands";
static constexpr const char *UCL_SUPPORTED_GENERATED_COMMANDS_TOPIC
  = "ucl/by-unid/+/+/+/SupportedGeneratedCommands";
static constexpr const char *UCL_ATTRIBUTES_TOPIC
  = "ucl/by-unid/+/+/+/Attributes/+/Reported";

namespace unify::node_state_monitor
{
node &node_state_monitor::emplace_node(const std::string &unid)
{
  return (*nodes.try_emplace(unid, node(unid)).first).second;
}

endpoint &node::emplace_endpoint(endpoint_id_t endpoint_id)
{
  return (*endpoints.try_emplace(endpoint_id, endpoint(endpoint_id)).first)
    .second;
}
std::string node::to_string() const
{
  std::stringstream ss;
  ss << "UNID: " << unid << " State: " << state
     << " Security: " << security_info << std::endl;
  for (auto const &[id, endpoint]: endpoints) {
    ss << endpoint.to_string();
  }
  return ss.str();
}

cluster &endpoint::emplace_cluster(const std::string &cluster_name)
{
  return (*clusters.try_emplace(cluster_name, cluster(cluster_name)).first)
    .second;
}

std::string endpoint::to_string() const
{
  std::stringstream ss;
  ss << "Endpoint: " << endpoint_id << std::endl;
  for (auto const &[id, cluster]: clusters) {
    ss << "  " << cluster.to_string();
  }
  return ss.str();
}

std::string cluster::to_string() const
{
  std::stringstream ss;
  ss << "<" << cluster_name << ">"
     << " Attributes: [";
  std::copy(attributes.begin(),
            attributes.end(),
            std::experimental::make_ostream_joiner(ss, ", "));
  ss << "] SupportedCommands: [";
  std::copy(supported_commands.begin(),
            supported_commands.end(),
            std::experimental::make_ostream_joiner(ss, ", "));

  ss << "] SupportedGeneratedCommands: [";
  std::copy(generated_commands.begin(),
            generated_commands.end(),
            std::experimental::make_ostream_joiner(ss, ", "));
  ss << "]" << std::endl;
  return ss.str();
}

void node_state_monitor::on_mqtt_node_state_message(
  const char *topic, const char *c_message, const size_t c_message_length)
{
  auto unid_opt = unid_from_mqtt_topic(topic);
  if (!unid_opt.has_value()) {
    sl_log_warning(LOG_TAG, "Received invalid UNID in topic: %s", topic);
    return;
  }
  std::string unid = unid_opt.value();
  if (c_message && c_message_length > 0) {
    try {
      nlohmann::json jsn;
      std::string message = std::string(c_message);
      jsn                 = nlohmann::json::parse(message);
      node_state_monitor::get_instance().on_node_state_changed(unid, jsn);
    } catch (const nlohmann::json::exception &err) {
      sl_log_warning(LOG_TAG,
                     "Failed to parse JSON message: '%s', error: %s",
                     c_message,
                     err.what());
    }
  } else {
    node_state_monitor::get_instance().on_node_removed(unid);
    return;
  }
}

void node_state_monitor::on_mqtt_cluster_supported_commands_message(
  const char *topic, const char *c_message, const size_t c_message_length)
{
  std::regex rgx("ucl/by-unid"
                 "/([^/]*)"            // UNID
                 "/ep([0-9]+)"         // Endpoint
                 "/([^/]*)"            // Cluster
                 "/SupportedCommands"  // SupportedCommands
  );
  std::smatch match;
  std::string topic_str(topic);
  if (std::regex_search(topic_str, match, rgx)) {
    const std::string &unid = match[1];
    endpoint_id_t endpoint  = static_cast<endpoint_id_t>(std::stoul(match[2]));
    const std::string &cluster_name = match[3];
    if (c_message && c_message_length > 0) {
      try {
        nlohmann::json jsn;
        std::string message = std::string(c_message);
        jsn                 = nlohmann::json::parse(message);
        node_state_monitor::get_instance().on_cluster_supported_commands_added(
          unid,
          endpoint,
          cluster_name,
          jsn);
      } catch (const nlohmann::json::exception &err) {
        sl_log_warning(LOG_TAG,
                       "Failed to parse JSON message: '%s', error: %s",
                       c_message,
                       err.what());
      }
    } else {
      node_state_monitor::get_instance().on_cluster_supported_commands_removed(
        unid,
        endpoint,
        cluster_name);
    }
  }
}

void node_state_monitor::on_mqtt_cluster_supported_generated_commands_message(
  const char *topic, const char *c_message, const size_t c_message_length)
{
  std::regex rgx("ucl/by-unid"
                 "/([^/]*)"                     // UNID
                 "/ep([0-9]+)"                  // Endpoint
                 "/([^/]*)"                     // Cluster
                 "/SupportedGeneratedCommands"  // SupportedGeneratedCommands
  );
  std::smatch match;
  std::string topic_str(topic);
  if (std::regex_search(topic_str, match, rgx)) {
    const std::string &unid = match[1];
    endpoint_id_t endpoint  = static_cast<endpoint_id_t>(std::stoul(match[2]));
    const std::string &cluster_name = match[3];
    if (c_message && c_message_length > 0) {
      try {
        nlohmann::json jsn;
        std::string message = std::string(c_message);
        jsn                 = nlohmann::json::parse(message);
        node_state_monitor::get_instance().on_cluster_generated_commands_added(
          unid,
          endpoint,
          cluster_name,
          jsn);
      } catch (const nlohmann::json::exception &err) {
        sl_log_warning(LOG_TAG,
                       "Failed to parse JSON message: '%s', error: %s",
                       c_message,
                       err.what());
      }

    } else {
      node_state_monitor::get_instance().on_cluster_generated_commands_removed(
        unid,
        endpoint,
        cluster_name);
    }
  }
}

void node_state_monitor::on_mqtt_cluster_attributes(const char *topic,
                                                    const char *message,
                                                    const size_t message_length)
{
  std::regex rgx("ucl/by-unid"
                 "/([^/]*)"     // UNID
                 "/ep([0-9]+)"  // Endpoint
                 "/([^/]*)"     // Cluster
                 "/Attributes"  // Attributes
                 "/([^/]*)"     // Attribute
                 ".*");
  std::smatch match;
  std::string topic_str(topic);
  if (std::regex_search(topic_str, match, rgx)) {
    const std::string &unid = match[1];
    endpoint_id_t endpoint  = static_cast<endpoint_id_t>(std::stoul(match[2]));
    const std::string &cluster_name = match[3];
    const std::string &attribute    = match[4];
    if (message && message_length > 0) {
      node_state_monitor::get_instance()
        .on_cluster_attribute_added(unid, endpoint, cluster_name, attribute);
    } else {
      node_state_monitor::get_instance()
        .on_cluster_attribute_removed(unid, endpoint, cluster_name, attribute);
    }
  }
}

void node_state_monitor::on_node_state_changed(const std::string &unid,
                                               const nlohmann::json &jsn)
{
  node &n       = emplace_node(unid);
  n.last_update = clock_time();
  // Store old state for use later to check if state has changed
  auto old_state = n.state;
  std::string state_str;
  jsn.at("NetworkStatus").get_to(state_str);
  auto state_value = node_state_network_status_get_enum_value_number(state_str);
  if (state_value == std::numeric_limits<uint32_t>::max()) {
    sl_log_warning(LOG_TAG,
                   "Received invalid NetworkStatus: %s",
                   state_str.c_str());
  } else {
    n.state = state_value;
  }
  std::string security_str;
  jsn.at("Security").get_to(security_str);
  auto security_value = node_state_security_get_enum_value_number(security_str);
  if (security_value == std::numeric_limits<uint32_t>::max()) {
    sl_log_warning(LOG_TAG,
                   "Received invalid Security: %s",
                   security_str.c_str());
  } else {
    n.security_info = security_value;
  }
  if (interface && (n.state != old_state)) {
    // If state has changed report it.
    interface->on_unify_node_state_changed(n);
    if (ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL == n.state) {
      nodes_pending_node_added.emplace(n.unid);
    }
  }
}

void node_state_monitor::on_cluster_attribute_added(
  const std::string &unid,
  endpoint_id_t endpoint_id,
  const std::string &cluster_name,
  const std::string &attribute)
{
  auto &n = emplace_node(unid);
  if (n.emplace_endpoint(endpoint_id)
        .emplace_cluster(cluster_name)
        .attributes.emplace(attribute)
        .second) {
    // Only update last_update if attribute didn't exist.
    // Emplace returns true if the item was inserted.
    n.last_update = clock_time();
  }
}

void node_state_monitor::on_cluster_attribute_removed(
  const std::string &unid,
  endpoint_id_t endpoint_id,
  const std::string &cluster_name,
  const std::string &attribute)
{
  try {
    auto &n       = nodes.at(unid);
    n.last_update = clock_time();
    n.endpoints.at(endpoint_id)
      .clusters.at(cluster_name)
      .attributes.erase(attribute);

  } catch (const std::out_of_range &) {
    // Failed to lookup one of the parents, which is ok, then there is no reason
    // the erase the attribute. It is likely that one of the paresnts have been
    // removed before the attribute.
  }
}

void node_state_monitor::on_cluster_supported_commands_added(
  const std::string &unid,
  endpoint_id_t endpoint_id,
  const std::string &cluster_name,
  const nlohmann::json &jsn)
{
  auto &n       = emplace_node(unid);
  n.last_update = clock_time();
  auto &cluster = n.emplace_endpoint(endpoint_id).emplace_cluster(cluster_name);
  try {
    for (const auto &value: jsn.at("value")) {
      std::string cmd;
      value.get_to(cmd);
      cluster.supported_commands.emplace(cmd);
    }
  } catch (const std::out_of_range &e) {
    sl_log_warning(LOG_TAG,
                   "Failed to parse supported generated commands: %s",
                   e.what());
  }
}

void node_state_monitor::on_cluster_supported_commands_removed(
  const std::string &unid,
  endpoint_id_t endpoint_id,
  const std::string &cluster_name)
{
  try {
    auto &n       = nodes.at(unid);
    n.last_update = clock_time();
    n.endpoints.at(endpoint_id)
      .clusters.at(cluster_name)
      .supported_commands.clear();
  } catch (const std::out_of_range &) {
    // Failed to lookup one of the parents, which is ok, then there is no reason
    // the erase the command. It is likely that one of the paresnts have been
    // removed before the command.
  }
}

void node_state_monitor::on_cluster_generated_commands_added(
  const std::string &unid,
  endpoint_id_t endpoint_id,
  const std::string &cluster_name,
  const nlohmann::json &jsn)
{
  auto &n       = emplace_node(unid);
  n.last_update = clock_time();
  auto &cluster = n.emplace_endpoint(endpoint_id).emplace_cluster(cluster_name);
  try {
    for (const auto &value: jsn.at("value")) {
      std::string cmd;
      value.get_to(cmd);
      cluster.generated_commands.emplace(cmd);
    }
  } catch (const std::out_of_range &e) {
    sl_log_warning(LOG_TAG,
                   "Failed to parse supported generated commands: %s",
                   e.what());
  }
}

void node_state_monitor::on_cluster_generated_commands_removed(
  const std::string &unid,
  endpoint_id_t endpoint_id,
  const std::string &cluster_name)
{
  try {
    auto &n       = nodes.at(unid);
    n.last_update = clock_time();
    n.endpoints.at(endpoint_id)
      .clusters.at(cluster_name)
      .generated_commands.clear();
  } catch (const std::out_of_range &) {
    // Failed to lookup one of the parents, which is ok, then there is no reason
    // the erase the command. It is likely that one of the paresnts have been
    // removed before the command.
  }
}

void node_state_monitor::on_node_removed(const std::string &unid)
{
  bool removed = nodes.erase(unid) > 0;
  if (interface && removed) {
    interface->on_unify_node_removed(unid);
  }
}

void node_state_monitor::on_node_state_functional_timeout(void *context)
{
  auto *self = reinterpret_cast<node_state_monitor *>(context);

  for (auto it = self->nodes_pending_node_added.begin();
       it != self->nodes_pending_node_added.end();) {
    try {
      const auto &n = self->nodes.at(*it);
      if (n.last_update + self->node_state_functional_threshold
          < clock_time()) {
        if (self->interface) {
          self->interface->on_unify_node_added(n);
        }
        it = self->nodes_pending_node_added.erase(it);
        continue;
      }
    } catch (const std::out_of_range &e) {
      sl_log_warning(LOG_TAG, "Failed to look up node %s", e.what());
    }
    ++it;
  }
  ctimer_restart(&self->node_state_functional_timer);
}
void node_state_monitor::set_node_state_functional_threshold(clock_time_t t)
{
  node_state_functional_threshold = t;
}
void node_state_monitor::set_node_state_functional_timeout_time(clock_time_t t)
{
  node_state_functional_timeout_time = t;
  ctimer_stop(&node_state_functional_timer);
  ctimer_set(&node_state_functional_timer,
             node_state_functional_timeout_time,
             on_node_state_functional_timeout,
             this);
}

node_state_monitor::node_state_monitor()
{
  uic_mqtt_subscribe(
    UCL_SUPPORTED_COMMANDS_TOPIC,
    node_state_monitor::on_mqtt_cluster_supported_commands_message);

  uic_mqtt_subscribe(
    UCL_SUPPORTED_GENERATED_COMMANDS_TOPIC,
    node_state_monitor::on_mqtt_cluster_supported_generated_commands_message);

  uic_mqtt_subscribe(UCL_ATTRIBUTES_TOPIC,
                     node_state_monitor::on_mqtt_cluster_attributes);

  uic_mqtt_subscribe(UCL_STATE_TOPIC,
                     node_state_monitor::on_mqtt_node_state_message);

  ctimer_set(&node_state_functional_timer,
             node_state_functional_timeout_time,
             on_node_state_functional_timeout,
             this);
}

node_state_monitor::~node_state_monitor()
{
  uic_mqtt_unsubscribe(UCL_STATE_TOPIC,
                       node_state_monitor::on_mqtt_node_state_message);
  uic_mqtt_unsubscribe(
    UCL_SUPPORTED_COMMANDS_TOPIC,
    node_state_monitor::on_mqtt_cluster_supported_commands_message);
  uic_mqtt_unsubscribe(
    UCL_SUPPORTED_GENERATED_COMMANDS_TOPIC,
    node_state_monitor::on_mqtt_cluster_supported_generated_commands_message);
  uic_mqtt_unsubscribe(UCL_ATTRIBUTES_TOPIC,
                       node_state_monitor::on_mqtt_cluster_attributes);
}

}  // namespace unify::node_state_monitor
