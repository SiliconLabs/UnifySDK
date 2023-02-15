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
#include "mqtt_test_helper.h"

// Unify includes
#include "sl_log.h"

#include "mosquitto.h"

extern "C" {
#include "uic_mqtt_mock.h"
}

// Generic includes
#include <string>
#include <unordered_map>
#include <string>
#include <queue>
#include <deque>
#include <string>
#include <algorithm>
#include <regex>
#include <cstring>

constexpr const char *LOG_TAG = "mqtt_test_helper";

typedef void (*mqtt_subscribe_callback)(const char *topic,
                                        const char *message,
                                        const size_t message_length);

class mqtt_test_helper
{
  public:
  std::deque<mqtt_subscribe_callback> subscribers;
  std::queue<std::string> publishes;
  unsigned int number_of_publishes = 0;
  mqtt_test_helper();
  ~mqtt_test_helper();
};

mqtt_test_helper::mqtt_test_helper() {}

mqtt_test_helper::~mqtt_test_helper() = default;

static std::unordered_map<std::string, mqtt_test_helper> mqtt_helper_map;

static void mqtt_test_helper_mqtt_publish_stub(const char *topic,
                                               const char *message,
                                               const size_t message_length,
                                               bool retain,
                                               int ncalls)
{
  std::string topic_str(topic);
  if (0 == mqtt_helper_map.count(topic_str)) {
    mqtt_helper_map.try_emplace(topic_str, mqtt_test_helper());
  }
  try {
    mqtt_test_helper &helper = mqtt_helper_map.at(topic_str);
    helper.publishes.push(std::string(message));
    helper.number_of_publishes += 1;
  } catch (const std::out_of_range &e) {
    // do nothing
  }
}

static void mqtt_test_helper_mqtt_subscribe_stub(const char *topic,
                                                 mqtt_subscribe_callback cb,
                                                 int ncalls)
{
  sl_log_info(LOG_TAG, "Incoming subscription to %s, function %p", topic, cb);
  std::string topic_str(topic);
  if (0 == mqtt_helper_map.count(topic_str)) {
    mqtt_helper_map.try_emplace(topic_str, mqtt_test_helper());
  }
  auto &subscribers = mqtt_helper_map.at(topic_str).subscribers;
  if (std::find(subscribers.begin(), subscribers.end(), cb)
      == subscribers.end()) {
    subscribers.push_back(cb);
  }
}

static void mqtt_test_helper_mqtt_unsubscribe_stub(const char *topic,
                                                   mqtt_subscribe_callback cb,
                                                   int ncalls)
{
  try {
    auto &subscribers = mqtt_helper_map.at(std::string(topic)).subscribers;
    auto subscriber   = std::find(subscribers.begin(), subscribers.end(), cb);
    if (subscriber != subscribers.end()) {
      subscribers.erase(subscriber);
    }
  } catch (const std::out_of_range &) {
    // do nothing
  }
}

static int uic_mqtt_count_topics_stub(const char *prefix_pattern, int num_calls)
{
  int number_of_publications = 0;
  std::string prefix(prefix_pattern);

  for (const auto &[key, value]: mqtt_helper_map) {
    if (key.find(prefix) != std::string::npos) {
      number_of_publications += value.number_of_publishes;
    }
  }

  return number_of_publications;
}

void mqtt_test_helper_init()
{
  sl_log_info(LOG_TAG, "MQTT Mock helper initialization");
  uic_mqtt_publish_Stub(mqtt_test_helper_mqtt_publish_stub);
  uic_mqtt_subscribe_Stub(mqtt_test_helper_mqtt_subscribe_stub);
  uic_mqtt_unsubscribe_Stub(mqtt_test_helper_mqtt_unsubscribe_stub);
  uic_mqtt_count_topics_Stub(uic_mqtt_count_topics_stub);
  for (auto &[key, value]: mqtt_helper_map) {
    value.number_of_publishes = 0;
  }
}

unsigned int mqtt_test_helper_get_num_subscribers(const char *topic)
{
  try {
    const mqtt_test_helper &helper = mqtt_helper_map.at(std::string(topic));
    return helper.subscribers.size();
  } catch (...) {
    // do nothing
  }
  return 0;
}

unsigned int mqtt_test_helper_get_total_num_subscribers()
{
  unsigned int total_number = 0;
  for (const auto &[key, value]: mqtt_helper_map) {
    total_number += value.subscribers.size();
  }
  return total_number;
}

unsigned int mqtt_test_helper_get_num_publish(const char *topic)
{
  try {
    const mqtt_test_helper &helper = mqtt_helper_map.at(std::string(topic));
    return helper.number_of_publishes;
  } catch (...) {
    // do nothing;
  }
  return 0;
}

char *mqtt_test_helper_pop_publish(const char *topic, char *result)
{
  try {
    mqtt_test_helper &helper = mqtt_helper_map.at(std::string(topic));
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

void mqtt_test_helper_publish(const char *topic,
                              const char *msg,
                              size_t msg_len)
{
  for (auto &[key, value]: mqtt_helper_map) {
    bool topic_match = false;
    mosquitto_topic_matches_sub(key.c_str(), topic, &topic_match);
    if (topic_match) {
      // Call all subscribers callback function
      std::for_each(std::begin(value.subscribers),
                    std::end(value.subscribers),
                    [topic, msg, msg_len](const mqtt_subscribe_callback cb) {
                      cb(topic, msg, msg_len);
                    });
    }
  }
}

void mqtt_test_helper_publish_to_all_topics(const char *msg, size_t msg_len)
{
  for (const auto &[key, value]: mqtt_helper_map) {
    std::string topic = key;
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
                  key.c_str(),
                  e.what());
    }

    mqtt_test_helper_publish(topic.c_str(), msg, msg_len);
  }
}
