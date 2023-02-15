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
 * @file mqtt_test_helper.h
 * @brief Helper module for testing MQTT
 *
 * Module that helps testing Unify MQTT component, and provides an easy to use
 * interface to get number of publishes to a given topic, getting the messages
 * that have been published to a specific topic, etc.
 * see function descriptions for further details
 *
 * @{
 */

#ifndef mqtt_test_helper_HPP
#define mqtt_test_helper_HPP
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Initialize the mqtt_test_helper
 */
void mqtt_test_helper_init();

/**
 * @brief Get number of subscribers for topic
 *
 * @param topic MQTT topic to get subscribers for
 * @return unsigned int Number of subscribers
 */
unsigned int mqtt_test_helper_get_num_subscribers(const char *topic);

/**
 * @brief Get total number of subscribers for any topic
 *
 * @return unsigned int Total number of subscribers
 */
unsigned int mqtt_test_helper_get_total_num_subscribers();

/**
 * @brief Get number of publishes to a given topic
 *
 * @param topic MQTT Topic to get number of publishes for
 * @return unsigned int Number of publishes to the topic
 */
unsigned int mqtt_test_helper_get_num_publish(const char *topic);

/**
 * @brief Pops an entry in a FIFO where the messages for a topic is stored
 *
 * @param topic Topic to pop an message for
 * @param result Buffer to store message in
 * @return char* Pointer to result buffer, or NULL for no result
 */
char *mqtt_test_helper_pop_publish(const char *topic, char *result);

/**
 * @brief Publish a message to a given MQTT topic
 *
 * @param topic Topic to publish on
 * @param msg Message to publish
 * @param msg_len Length of message
 */
void mqtt_test_helper_publish(const char *topic,
                              const char *msg,
                              size_t msg_len);
#define mqtt_test_helper_publish_str(topic, msg) \
  mqtt_test_helper_publish(topic, msg, strlen(msg));
/**
 * @brief Publish a message to all (subscribed) MQTT topic
 *
 * @param msg Message to publish
 * @param msg_len Length of message
 */
void mqtt_test_helper_publish_to_all_topics(const char *msg, size_t msg_len);

#ifdef __cplusplus
}
#endif

#endif  //mqtt_test_helper_HPP
/** @} end mqtt_test_helper */
