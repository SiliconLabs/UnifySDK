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
 * @defgroup mqtt_client MQTT Client
 * @ingroup uic_mqtt
 * @brief This is the implementation of all of the logic for our MQTT client.
 *
 * The MQTT-Client is client-library-agnostic and depends on having the
 * appropriate wrapper, e.g. mqtt_wrapper_mosquitto.cpp, to interface with
 * the underlying client-library. All functionality is asynchronous where possible.
 *
 * @{
 */

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

// Standard library
#include <stdbool.h>
#include <unistd.h>
#include <inttypes.h>

// Unify components
#include "sl_status.h"

#ifndef MQTT_CLIENT_POLL_TIMER_MILLISECONDS
#define MQTT_CLIENT_POLL_TIMER_MILLISECONDS 2000
#endif

#ifndef MQTT_CLIENT_KEEP_ALIVE_INTERVAL_MILLISECONDS
#define MQTT_CLIENT_KEEP_ALIVE_INTERVAL_MILLISECONDS 60000
#endif

#ifndef MQTT_CLIENT_MINIMUM_RECONNECT_BACKOFF_MILLISECONDS
#define MQTT_CLIENT_MINIMUM_RECONNECT_BACKOFF_MILLISECONDS 1000
#endif

#ifndef MQTT_CLIENT_MAXIMUM_RECONNECT_BACKOFF_MILLISECONDS
#define MQTT_CLIENT_MAXIMUM_RECONNECT_BACKOFF_MILLISECONDS 60000
#endif

#ifndef MQTT_CLIENT_QoS
#define MQTT_CLIENT_QoS 0
#endif

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Internal events for the mqtt_client:
 *  - MQTT_EVENT_*
 *      - Used for implementing the asynchronous architecture of the client.
 *        A function-call will enqueue some data, and then emit one of those events
 *        for later processing.
 *  - MQTT_TRANSITION_*
 *      - Events for the FSM. The FSM should ONLY change states as a response to
 *        those events being emitted.
 */
typedef enum {
  MQTT_EVENT_CONNECT,      ///< Request the client to connect to its broker.
  MQTT_EVENT_DISCONNECT,   ///< Disconnect (from broker) request.
  MQTT_EVENT_PUBLISH,      ///< Publish one queued up message.
  MQTT_EVENT_SUBSCRIBE,    ///< Subscribe to one queued up topic.
  MQTT_EVENT_UNSUBSCRIBE,  ///< Unsubscribe from one queued up topic.
  MQTT_TRANSITION_DISCONNECTED,  ///< Request FSM to transition to the *disconnected* state.
  MQTT_TRANSITION_CONNECTING,  ///< Request FSM to transition to the *connecting* state.
  MQTT_TRANSITION_CONNECTED,  ///< Request FSM to transition to the *connected* state.
} mqtt_event_t;
// This is really hackish, but I couldn't figure out a way to do this with
// preprocessor-magic but still be able to properly Doxygen-document the
// members of the enum.
__attribute__((unused)) static const char *event_names[]
  = {"MQTT_EVENT_CONNECT",
     "MQTT_EVENT_DISCONNECT",
     "MQTT_EVENT_PUBLISH",
     "MQTT_EVENT_SUBSCRIBE",
     "MQTT_EVENT_UNSUBSCRIBE",
     "MQTT_TRANSITION_DISCONNECTED",
     "MQTT_TRANSITION_CONNECTING",
     "MQTT_TRANSITION_CONNECTED"};

struct mqtt_client;

typedef struct mqtt_client
  *mqtt_client_t;  ///< Type for internal consistency and to save typing.

/**
 * @brief A callback type for pushing incoming messages.
 *
 * @param topic The topic the message came from
 * @param message The message payload
 * @param message_length The size of the message payload in bytes.
 */
typedef void (*message_callback_t)(const char *topic,
                                   const char *message,
                                   const size_t message_length);

/**
 * @brief A callback type for pushing incoming messages.
 *
 * @param topic The topic the message came from
 * @param message The message payload
 * @param message_length The size of the message payload in bytes.
 * @param user User defined pointer provided when subscribing
 */
typedef void (*message_callback_ex_t)(const char *topic,
                                   const char *message,
                                   const size_t message_length,void* user);

// C-wrappers. All of those functions have a C++ counterpart within mqtt_client
// or a C++ specific functionality that needs to be bridged to C-land (e.g. new).

/**
 * @brief Initialize whatever resources the client needs.
 *
 * @param instance Pointer to an mqtt_client instance.
 * @return true if initialization was successful. False otherwise.
 */
sl_status_t mqtt_client_setup(mqtt_client_t instance);

/**
 * @brief Close open connections and free up resources acquired by the client.
 *
 * @param instance Pointer to an mqtt_client instance.
 * @return \ref SL_STATUS_OK on success.
 * @return \ref SL_STATUS_FAIL on failure.
 */
sl_status_t mqtt_client_teardown(mqtt_client_t instance);

/**
 * @brief Get the file-descriptor for the underlying MQTT TCP socket.
 *
 * @param instance Pointer to an mqtt_client instance.
 * @return The underlying TCP-socket file-descriptor.
 */
int mqtt_client_file_descriptor(mqtt_client_t instance);

/**
 * @brief Process any data that needs processed.
 *
 * Also executes the network-loop of the underlying MQTT-client library, which means
 * that this function should be called at least at the same interval as the configured
 * keep-alive for the connection: \ref MQTT_CLIENT_KEEP_ALIVE_INTERVAL_MILLISECONDS
 *
 * @param instance Pointer to an mqtt_client instance.
 */
void mqtt_client_poll(mqtt_client_t instance);

/**
 * @brief Do a synchronous disconnect from the MQTT-broker. This can be used when
 *        a system is being shut down and the connection needs to be closed gracefully.
 * @param instance Pointer to an mqtt_client instance.
 */
void mqtt_client_disconnect(mqtt_client_t instance);

/**
 * @brief Push events to the mqtt_client.
 *
 * This is the entry-point for all events. The mqtt_client proxies all events to
 * its subsystems, including the FSM.
 *
 * @param instance Pointer to an mqtt_client instance.
 * @param incoming_event An event defined by \ref mqtt_event_t
 */
void mqtt_client_event(mqtt_client_t instance, const int incoming_event);

/**
 * @brief Publish a message to the MQTT-broker.
 *
 * This, like (almost) all other functionality of the mqtt_client, is an asynchronous
 * (non-blocking) call. The outgoing message is queued and then sent if/when the
 * connection-state allows for it.
 *
 * @param instance Pointer to an mqtt_client instance.
 * @param topic The MQTT topic to publish to.
 * @param message The message payload itself.
 * @param message_length The size of the message.
 * @param retain Set to true if the message shall be retained by the MQTT-broker.
 */
void mqtt_client_publish(mqtt_client_t instance,
                         const char *topic,
                         const char *message,
                         const size_t message_length,
                         bool retain);

/**
 * @brief Un-retain a previously retained message.

 *
 * This will send an empty retained message for all previously
 * published retained topics which has the given prefix.
 *
 * According to the mqtt specification it will cause the broker to
 * delete the retained messages.
 *
 * @param instance Pointer to an mqtt_client instance.
 * @param prefix_pattern prefix to un-retain
 */
void mqtt_client_unretain(mqtt_client_t instance, const char *prefix_pattern);

/**
   * @brief Count the number of topics published
   *
   * This function counts the number of topics which has been published
   * retained matching a pattern prefix.
   * @param instance Pointer to an mqtt_client instance.
   * @param prefix_pattern
   * @return int
   */
int mqtt_client_count_topics(mqtt_client_t instance,
                             const char *prefix_pattern);

/**
 * @brief Un-retain a previously retained message.
 *
 * Any topic matches the regex pattern
 *
 * @param instance Pointer to an mqtt_client instance.
 * @param regex A regular expression to match topics against
 */
void mqtt_client_unretain_by_regex(mqtt_client_t instance, const char *regex);

/**
 * @brief Subscribe to an MQTT-topic.
 *
 * This, like (almost) all other functionality of the mqtt_client, is an asynchronous
 * (non-blocking) call. The subscription request is queued and then sent if/when the
 * connection-state allows for it.
 *
 * @param instance Pointer to an mqtt_client instance.
 * @param topic The MQTT-topic name/pattern to subscribe to.
 * @param callback Function-pointer to a callback-function that receives the incoming
 *                 messages and has the \ref message_callback_t function-signature.
 */
void mqtt_client_subscribe(mqtt_client_t instance,
                           const char *topic,
                           message_callback_t callback);


/**
 * @brief Subscribe to an MQTT-topic.
 *
 * This, like (almost) all other functionality of the mqtt_client, is an asynchronous
 * (non-blocking) call. The subscription request is queued and then sent if/when the
 * connection-state allows for it.
 *
 * @param instance Pointer to an mqtt_client instance.
 * @param topic The MQTT-topic name/pattern to subscribe to.
 * @param callback Function-pointer to a callback-function that receives the incoming
 *                 messages and has the \ref message_callback_t function-signature.
 * @param user  User defined pointer which will be returned in the callback
 */
void mqtt_client_subscribe_ex(mqtt_client_t instance,
                           const char *topic,
                           message_callback_ex_t callback,void* user);

/**
 * @brief Unsubscribe from an MQTT-topic
 *
 * This, like (almost) all other functionality of the mqtt_client, is an asynchronous
 * (non-blocking) call. The unsubscription request is queued and then sent if/when the
 * connection-state allows for it, with one caveat: The callback-registration happens
 * immediately, but sending the unsubscription-command to the broker is asynchronous.
 *
 * @param instance Pointer to an mqtt_client instance.
 * @param topic The MQTT-topic name/pattern to unsubscribe from.
 * @param callback Function-pointer to a callback-function that receives the incoming
 *                 messages and has the \ref message_callback_t function-signature.
 */
void mqtt_client_unsubscribe(mqtt_client_t instance,
                             const char *topic,
                             message_callback_t callback);


/**
 * @brief Unsubscribe from an MQTT-topic
 *
 * This, like (almost) all other functionality of the mqtt_client, is an asynchronous
 * (non-blocking) call. The unsubscription request is queued and then sent if/when the
 * connection-state allows for it, with one caveat: The callback-registration happens
 * immediately, but sending the unsubscription-command to the broker is asynchronous.
 *
 * @param instance Pointer to an mqtt_client instance.
 * @param topic The MQTT-topic name/pattern to unsubscribe from.
 * @param callback Function-pointer to a callback-function that receives the incoming
 *                 messages and has the \ref message_callback_t function-signature.
 * @param user    Argument used when registering
 */
void mqtt_client_unsubscribe_ex(mqtt_client_t instance,
                             const char *topic,
                             message_callback_ex_t callback,
                             void* user);


/**
 * @brief Allocates a new mqtt_client instance.
 *
 * @param hostname The hostname/IP-address of the MQTT-broker this client should
 *                 connect to.
 * @param port The TCP-port the broker is listening on.
 * @param client_id The client-id to use when identifying to the broker. This can
 *                  be set to NULL.
 * @param cafile   path to a file containing the PEM encoded trusted CA certificate file.
 * @param certfile path to a file containing the PEM encoded certificate file for this client.
 * @param keyfile  path to a file containing the PEM encoded private key for this client.
 * @param event_sender A function-pointer to the external-function that the client
 *                     shall use for emitting events.
 * @param delayed_event_sender A function-pointer to an external-function that the
 *                             client can use for emitting delayed events.
 * @param event_counter A function-pointer to the external function that the client
 *                      shall use for counting the number of events on the event-queue.
 * @return A pointer to the mqtt_client instance.
 */
mqtt_client_t mqtt_client_new(
  const char *hostname,
  const uint32_t port,
  const char *client_id,
  const char *cafile,
  const char *certfile,
  const char *keyfile,
  int (*event_sender)(const int event, void *procdata),
  void (*delayed_event_sender)(const int event,
                               const unsigned long milliseconds),
  int (*event_counter)(const int event, void *procdata));

/**
 * @brief Deletes/frees the mqtt_client instance.
 *
 * @param instance A pointer to an mqtt_client instance.
 */
void mqtt_client_delete(mqtt_client_t instance);

void mqtt_client_on_connect_callback_set(
  mqtt_client_t instance,
  void (*on_connect)(mqtt_client_t inst, const int file_descriptor));

void mqtt_client_on_disconnect_callback_set(
  mqtt_client_t instance,
  void (*on_disconnect)(mqtt_client_t inst, const int file_descriptor));

/**
 * @brief Sets a callback for calling before calling disconnect.
 *
 * @param instance A pointer to an mqtt_client instance.
 * @param before_disconnect A function-pointer to the external-function 
 * before disconnect callback
 */
void mqtt_client_before_disconnect_callback_set(mqtt_client_t instance,
                                                void (*before_disconnect)());

/**
 * @brief Deletes/frees the mqtt_client instance.
 *
 * @param instance A pointer to an mqtt_client instance.
 * @param after_connect function-pointer to the external-function 
 * after connect callback
 */
void mqtt_client_after_connect_callback_set(mqtt_client_t instance,
                                            void (*after_connect)());

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // MQTT_CLIENT_H

/** @} end mqtt_client */
