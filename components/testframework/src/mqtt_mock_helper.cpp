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
#include "mqtt_mock_helper.h"

// UIC includes
#include "sl_log.h"

extern "C" {
#include "uic_mqtt_mock.h"
}

// Generic includes
#include <string>
#include <map>
#include <string>
#include <queue>
#include <deque>
#include <string>
#include <algorithm>
#include <regex>

constexpr const char *LOG_TAG = "mqtt_mock_helper";

typedef void (*mqtt_subscribe_callback)(const char *topic,
                                        const char *message,
                                        const size_t message_length);

class mqtt_mock_helper
{
  public:
  std::deque<mqtt_subscribe_callback> subscribers;
  std::queue<std::string> publishes;
  unsigned int number_of_publishes;
  mqtt_mock_helper();
  ~mqtt_mock_helper();
};

mqtt_mock_helper::mqtt_mock_helper() : number_of_publishes(0) {}

mqtt_mock_helper::~mqtt_mock_helper() = default;

static std::map<std::string, mqtt_mock_helper> mqtt_helper_map;

static void mqtt_mock_helper_mqtt_publish_stub(const char *topic,
                                               const char *message,
                                               const size_t message_length,
                                               bool retain,
                                               int ncalls)
{
  std::string topic_str(topic);
  if (0 == mqtt_helper_map.count(topic_str)) {
    mqtt_helper_map.insert(std::make_pair(topic_str, mqtt_mock_helper()));
  }
  try {
    mqtt_mock_helper &helper = mqtt_helper_map.at(topic_str);
    helper.publishes.push(std::string(message));
    helper.number_of_publishes += 1;
  } catch (const std::out_of_range &e) {
    // do nothing
  }
}

static void mqtt_mock_helper_mqtt_subscribe_stub(const char *topic,
                                                 mqtt_subscribe_callback cb,
                                                 int ncalls)
{
  sl_log_info(LOG_TAG, "Incoming subscription to %s, function %p", topic, cb);
  std::string topic_str(topic);
  if (0 == mqtt_helper_map.count(topic_str)) {
    mqtt_helper_map.insert(std::make_pair(topic_str, mqtt_mock_helper()));
  }
  auto &subscribers = mqtt_helper_map.at(topic_str).subscribers;
  if (std::find(subscribers.begin(), subscribers.end(), cb)
      == subscribers.end()) {
    subscribers.push_back(cb);
  }
}

static void mqtt_mock_helper_mqtt_unsubscribe_stub(const char *topic,
                                                   mqtt_subscribe_callback cb,
                                                   int ncalls)
{
  try {
    auto &subscribers = mqtt_helper_map.at(std::string(topic)).subscribers;
    auto subscriber   = std::find(subscribers.begin(), subscribers.end(), cb);
    if (subscriber != subscribers.end()) {
      subscribers.erase(subscriber);
    }
  } catch (const std::out_of_range &e) {
    // do nothing
  }
}

static int uic_mqtt_count_topics_stub(const char *prefix_pattern, int num_calls)
{
  int number_of_publications = 0;
  std::string prefix(prefix_pattern);

  for (auto item: mqtt_helper_map) {
    if (item.first.find(prefix) != std::string::npos) {
      number_of_publications += item.second.number_of_publishes;
    }
  }

  return number_of_publications;
}

void mqtt_mock_helper_init()
{
  sl_log_info(LOG_TAG, "MQTT Mock helper initialization");
  uic_mqtt_publish_Stub(mqtt_mock_helper_mqtt_publish_stub);
  uic_mqtt_subscribe_Stub(mqtt_mock_helper_mqtt_subscribe_stub);
  uic_mqtt_unsubscribe_Stub(mqtt_mock_helper_mqtt_unsubscribe_stub);
  uic_mqtt_count_topics_Stub(uic_mqtt_count_topics_stub);
  for (auto &elem: mqtt_helper_map) {
    elem.second.number_of_publishes = 0;
  }
}

unsigned int mqtt_mock_helper_get_num_subscribers(const char *topic)
{
  try {
    mqtt_mock_helper &helper = mqtt_helper_map.at(std::string(topic));
    return helper.subscribers.size();
  } catch (...) {
    // do nothing;
  }
  return 0;
}

unsigned int mqtt_mock_helper_get_total_num_subscribers()
{
  unsigned int total_number = 0;
  for (auto &elem: mqtt_helper_map) {
    total_number += elem.second.subscribers.size();
  }
  return total_number;
}

unsigned int mqtt_mock_helper_get_num_publish(const char *topic)
{
  try {
    mqtt_mock_helper &helper = mqtt_helper_map.at(std::string(topic));
    return helper.number_of_publishes;
  } catch (...) {
    // do nothing;
  }
  return 0;
}

unsigned int mqtt_mock_helper_get_total_num_publish()
{
  unsigned int total_number = 0;
  for (auto &elem: mqtt_helper_map) {
    total_number += elem.second.number_of_publishes;
  }
  return total_number;
}

char *mqtt_mock_helper_pop_publish(const char *topic, char *result)
{
  try {
    mqtt_mock_helper &helper = mqtt_helper_map.at(std::string(topic));
    if (!helper.publishes.empty()) {
      strcpy(result, helper.publishes.front().c_str());
      helper.publishes.pop();
      return result;
    }
  } catch (...) {
    // do nothing;
  }
  return nullptr;
}

void mqtt_mock_helper_publish(const char *topic,
                              const char *msg,
                              size_t msg_len)
{
  // replace /zwDEADBEEF/ep43/ with /+/+/
  const std::regex unid_ep_re("/\\w+/ep\\d+/");
  std::string wc_topic = std::regex_replace(topic, unid_ep_re, "/+/+/");

  // Replace /Reported or /Desired at the end with /#
  const std::regex reported_re("/Reported$");
  wc_topic = std::regex_replace(wc_topic, reported_re, "/#");
  const std::regex desired_re("/Desired$");
  wc_topic = std::regex_replace(wc_topic, desired_re, "/#");
  sl_log_debug(LOG_TAG, "Replaced topic: %s", wc_topic.c_str());

  try {
    auto &subscribers = mqtt_helper_map.at(wc_topic).subscribers;

    for (auto it = subscribers.begin(); it != subscribers.end(); ++it) {
      (*it)(topic, msg, msg_len);
    }
  } catch (...) {
    // do nothing
    sl_log_warning(LOG_TAG,
                   "Mock helper publish received with "
                   "no subscription for topic %s",
                   topic);
  }
}

void mqtt_mock_helper_group_publish(const char *topic,
                                    const char *msg,
                                    size_t msg_len)
{
  // replace /1234/ with /+/
  const std::regex groupid_re("/by-group/\\d+/");
  std::string wc_topic = std::regex_replace(topic, groupid_re, "/by-group/+/");

  try {
    auto &subscribers = mqtt_helper_map.at(wc_topic).subscribers;

    for (auto it = subscribers.begin(); it != subscribers.end(); ++it) {
      try {
        (*it)(topic, msg, msg_len);
      } catch (std::exception &e) {
        sl_log_warning(LOG_TAG,
                       "Unhandled by-group handler exception for topic %s: %s",
                       topic,
                       e.what());
      }
    }
  } catch (std::exception &e) {
    // do nothing
    sl_log_warning(LOG_TAG,
                   "Mock helper by-group publish received with "
                   "no subscription for topic %s: %s",
                   topic,
                   e.what());
  }
}

void mqtt_mock_helper_publish_to_all_topics(const char *msg, size_t msg_len)
{
  for (auto &elem: mqtt_helper_map) {
    std::string topic = elem.first;
    // Try to take the topic filter and replace + signs with probable publication
    // values.
    try {
      topic.replace(topic.find('+'), 1, "zwDEADBEEF");
      topic.replace(topic.find('+'), 1, "ep0");
    } catch (std::out_of_range const &e) {
      // Could not replace the endpoint value. Don't do anything about it.
      sl_log_info(LOG_TAG,
                  "Could not replace UNID / EP in the "
                  "topic filter %s. Exception: %s",
                  elem.first.c_str(),
                  e.what());
    }

    mqtt_mock_helper_publish(topic.c_str(), msg, msg_len);
  }
}
