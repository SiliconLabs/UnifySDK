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
extern "C" {
#include "uic_mqtt_mock.h"
}
#include <string.h>

#include <map>
#include <string>
#include <queue>
#include <deque>
#include <string>
#include <algorithm>
#include <regex>

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

void mqtt_mock_helper_init()
{
  uic_mqtt_publish_Stub(mqtt_mock_helper_mqtt_publish_stub);
  uic_mqtt_subscribe_Stub(mqtt_mock_helper_mqtt_subscribe_stub);
  uic_mqtt_unsubscribe_Stub(mqtt_mock_helper_mqtt_unsubscribe_stub);
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

  try {
    auto &subscribers = mqtt_helper_map.at(wc_topic).subscribers;

    for (auto it = subscribers.begin(); it != subscribers.end(); ++it) {
      (*it)(topic, msg, msg_len);
    }
  } catch (...) {
    // do nothing
  }
}

void mqtt_mock_helper_publish_to_all_topics(const char *msg, size_t msg_len)
{
  for (auto &elem: mqtt_helper_map) {
    std::string topic = elem.first;
    topic.replace(topic.find('+'), 1, "zwDEADBEEF");
    topic.replace(topic.find('+'), 1, "ep0");
    mqtt_mock_helper_publish(topic.c_str(), msg, msg_len);
  }
}
