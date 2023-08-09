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
 * @defgroup uic_mqtt Unify MQTT Client
 * @ingroup unify_components
 * @brief MQTT client interface
 *
 * This component implement an mqtt client interface. The mqtt client
 * features connection state management, with re-connect on disconnects and
 * cashing of publish messages in case the a publish message cannot be delivered
 * to the broker.
 *
 * Other components can use the MQTT component to subscribe (@ref
 * uic_mqtt_subscribe) to MQTT topics and register callbacks when new messages
 * arrives.
 *
 * This api also features publishing messages(@ref uic_mqtt_publish) and keeping
 * track on which messages has been published with the retained flag such that
 * they mey be un-retaind again if desired (@ref uic_mqtt_unretain  @ref
 * uic_mqtt_unretain_by_regex).
 *
 * @{
 */

#ifndef UIC_MQTT_H
#define UIC_MQTT_H

#define MQTT_TOPIC_MAX_LENGTH   512
#define MQTT_MESSAGE_MAX_LENGTH 512

#include <stdbool.h>
#include <stddef.h>
#include "sl_status.h"

// Helper defines for applications to "unretain_by_regex"
// clang-format off
#define REGEX_NOT_STATE_OR_MQTT_CLIENT_TOPICS   "^(?!((ucl\\/by-unid\\/.*\\/State$)|(ucl\\/by-mqtt-client\\/.*))).*"
#define REGEX_NOT_STATE_TOPIC                   "^(?!ucl\\/by-unid\\/.*\\/State$).*"
#define REGEX_ALL_TOPICS                        "^.+"
// clang-format on

/**
 * @brief A callback type for pushing incoming messages.
 *
 * @param topic The topic the message came from
 * @param message The message payload
 * @param message_length The size of the message payload in bytes.
 */
typedef void (*mqtt_message_callback_t)(const char *topic,
                                        const char *message,
                                        const size_t message_length);

typedef void (*mqtt_message_callback_ex_t)(const char *topic,
                                           const char *message,
                                           const size_t message_length,
                                           void *user);

/**
 * @brief A callback type for a before disconnect and after connect.
 *
 */
typedef void (*mqtt_connection_callbacks_t)();

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the MQTT client instance.
 *
 * @returns true if the library is initialized, and created an MQTT client
 * instance.
 * @returns true if initialization was successful and client_instance was created.
 */
sl_status_t uic_mqtt_setup();

/**
 * Tear-down the MQTT library and client instances.
 *
 * @returns success
 */
int uic_mqtt_teardown();

/**
 * @brief Add the message and topic to publisher queue system.
 *
 * @param topic Represent the topic to publish to.
 * @param message A message that will be published to a given topic.
 *                It shall have a zero-terminated character.
 * @param message_length A size of the message in bytes.
 * @param retain Describes if the broker has to retain or not the message.
 *
 */
void uic_mqtt_publish(const char *topic,
                      const char *message,
                      const size_t message_length,
                      bool retain);

/**
 * @brief Un-retain a previously retained message.

 *
 * This will send an empty retained message for all previously
 * published retained topics which has the given prefix.
 *
 * Example: If retained topics are ["a/b", "a/b/c", "b"] and prefix_pattern is "a",
 * then the following topics will be unretained: ["a/b", "a/b/c"]
 *
 * According to the MQTT specification it will cause the broker to
 * delete the retained messages.
 *
 * @param prefix_pattern prefix to un-retain
 */
void uic_mqtt_unretain(const char *prefix_pattern);

/**
   * @brief Count the number of topics published
   *
   * This function counts the number of topics which has been published
   * retained matching a pattern prefix.
   * @param prefix_pattern
   * @return int
   */
int uic_mqtt_count_topics(const char *prefix_pattern);

/**
 * @brief Un-retain a previously retained message.
 *
 * Any topic matches the regex pattern
 *
 * @param regex regular expression
 */
void uic_mqtt_unretain_by_regex(const char *regex);

/**
 * @brief Subscribe to a topic.
 *
 * @param topic Describes the subscription topic pattern.
 * @param callback A callback function,
 *                 that will be called when a message is received from the broker.
 *
 */
void uic_mqtt_subscribe(const char *topic, mqtt_message_callback_t callback);

/**
 * @brief Subscribe to a topic.
 *
 * @param topic     Describes the subscription topic pattern.
 * @param callback  A callback function,
 *                  that will be called when a message is received from the broker.
 * @param user      A pointer value that will be passed to the callabck
 *                  function when invoked
 */
void uic_mqtt_subscribe_ex(const char *topic,
                           mqtt_message_callback_ex_t callback,
                           void *user);

/**
 * @brief Unsubscribe from a topic.
 *
 * @param topic Describes the subscription pattern.
 * @param callback A callback function corresponds to unsubscribe topic
 *
 */
void uic_mqtt_unsubscribe(const char *topic, mqtt_message_callback_t callback);

/**
 * @brief Unsubscribe from a topic.
 *
 * @param topic     Describes the subscription pattern.
 * @param callback  A callback function corresponds to unsubscribe topic
 * @param user      User pointer value to pass to the MQTT message callback
 *
 */
void uic_mqtt_unsubscribe_ex(const char *topic,
                             mqtt_message_callback_ex_t callback,
                             void *user);

/**
 * @brief Set a callback which will be executed before calling a disconnect.
 *
 * @param callback A callback function to be executed before calling disconnect.
 *
 */
void uic_mqtt_set_before_disconnect_callback(
  mqtt_connection_callbacks_t callback);

/**
 * @brief Set a callback which will be executed after having connected.
 *
 * @param callback A callback function to be executed after having connected.
 */
void uic_mqtt_set_after_connect_callback(mqtt_connection_callbacks_t callback);

/**
 * @brief Gets the MQTT Client ID in used by the MQTT Client.
 *
 * @returns NULL if no Client ID is in use, else a C string containing the Client ID.
 */
const char *uic_mqtt_get_client_id();

/**
 * @brief Checks if we are currently connected to an MQTT Broker.
 *
 * @returns true if connected to a broker
 */
bool uic_mqtt_is_connected_to_broker();

#ifdef __cplusplus
}
#endif

#endif  // UIC_MQTT
/** @} end mqtt */
