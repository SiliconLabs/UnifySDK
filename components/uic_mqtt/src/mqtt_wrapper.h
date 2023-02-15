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
#ifndef MQTT_WRAPPER_H
#define MQTT_WRAPPER_H

// Standard library
#include <stdbool.h>

// Unify components
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

struct mqtt_message {
  int mid;            ///< Message-ID.
  const char *topic;  ///< MQTT-topic where message originated from.
  void *payload;      ///< Pointer to the payload.
  int payloadlen;     ///< Size of the payload in bytes.
  int qos;            ///< QoS value when message was published.
  bool retain;        ///< True if this was a retained message.
};

/**
 * @brief Initialize the underlying MQTT-client library
 *
 * This function is **not** thread-safe.
 *
 * @return \ref SL_STATUS_OK always.
 */
sl_status_t mqtt_wrapper_lib_init();

/**
 * @brief Initialize a new instance of the underlying client implementation.
 *
 * @param id The client-ID to present to the MQTT-broker.
 * @param clean_session True if the session should be clean.
 * @param obj A pointer to a user-object that will be passed along with callbacks.
 * @return A pointer to the client-instance.
 */
void *mqtt_wrapper_new(const char *id, bool clean_session, void *obj);

/**
 * @brief Do a cleanup of the underlying client-library.
 *
 * @return \ref SL_STATUS_OK always.
 */
sl_status_t mqtt_wrapper_lib_cleanup();

/**
 * @brief Destroy (free) a client-instance.
 *
 * @param instance Pointer to a client-instance to free.
 */
void mqtt_wrapper_destroy(void *instance);

/**
 * @brief Fetch the file-descriptor for the underlying network-socket.
 *
 * @param instance Pointer to a client-instance.
 * @return The file-descriptor.
 */
int mqtt_wrapper_socket(void *instance);
/**
 * @brief Configure the client for Certficate based TLS support
 *
 * Wrapper to mosquitto_tls_set()
 *
 * @param instance Pointer to a client-instance.
 * @param cafile   path to a file containing the PEM encoded trusted CA certificate.
 * @param certfile path to a file containing the PEM encoded certificate file for this client.
 * @param keyfile  path to a file containing the PEM encoded private key for this client.
 * @return \ref SL_STATUS_OK \copybrief SL_STATUS_OK
 * @return \ref SL_STATUS_INVALID_PARAMETER \copybrief SL_STATUS_INVALID_PARAMETER
 * @return \ref SL_STATUS_ERRNO \copybrief SL_STATUS_ERRNO
 */
sl_status_t mqtt_wrapper_tls_set(void *instance,
                                 const char *cafile,
                                 const char *certfile,
                                 const char *keyfile);
/**
 * @brief Configure the client for pre-shared-key based TLS support
 *
 * Wrapper to mosquitto_tls_psk_set()
 *
 * @param instance Pointer to a client-instance.
 * @param psk the pre-shared-key in hex format with no leading “0x”.
 * @param id the identity of this client.
 * @param ciphers a string describing the PSK ciphers available for use.
 *        See the “openssl ciphers” tool for more information.
 *        If NULL, the default ciphers will be used.
 * @return \ref SL_STATUS_OK \copybrief SL_STATUS_OK
 * @return \ref SL_STATUS_INVALID_PARAMETER \copybrief SL_STATUS_INVALID_PARAMETER
 * @return \ref SL_STATUS_ERRNO \copybrief SL_STATUS_ERRNO
 */
sl_status_t mqtt_wrapper_tls_psk_set(void *instance,
                                     const char *psk,
                                     const char *id,
                                     const char *ciphers);
/**
 * @brief Connect to an MQTT-broker
 *
 * @param instance Pointer to a client-instance.
 * @param host MQTT-broker hostname/IP-address
 * @param port TCP port to connect to
 * @param keepalive The keep-alive interval for this connection.
 * @return \ref SL_STATUS_OK \copybrief SL_STATUS_OK
 * @return \ref SL_STATUS_INVALID_PARAMETER \copybrief SL_STATUS_INVALID_PARAMETER
 * @return \ref SL_STATUS_ERRNO \copybrief SL_STATUS_ERRNO
 */
sl_status_t mqtt_wrapper_connect(void *instance,
                                 const char *host,
                                 int port,
                                 int keepalive);

/**
 * @brief Close the MQTT-broker connection.
 *
 * @param instance Pointer to a client-instance.
 * @return \ref SL_STATUS_OK \copybrief SL_STATUS_OK
 * @return \ref SL_STATUS_INVALID_PARAMETER \copybrief SL_STATUS_INVALID_PARAMETER
 * @return \ref SL_STATUS_NET_MQTT_NO_CONN \copybrief SL_STATUS_NET_MQTT_NO_CONN
 */
sl_status_t mqtt_wrapper_disconnect(void *instance);

/**
 * @brief Run an iteration of the network-loop of the underlying client-library.
 *
 * @param instance Pointer to a client-instance.
 * @param timeout Maximum number of **milliseconds** to wait on the
 *                underlying select() call.
 * @return \ref SL_STATUS_OK \copybrief SL_STATUS_OK
 * @return \ref SL_STATUS_INVALID_PARAMETER \copybrief SL_STATUS_INVALID_PARAMETER
 * @return \ref SL_STATUS_ALLOCATION_FAILED \copybrief SL_STATUS_ALLOCATION_FAILED
 * @return \ref SL_STATUS_NET_MQTT_NO_CONN \copybrief SL_STATUS_NET_MQTT_NO_CONN
 * @return \ref SL_STATUS_NET_MQTT_LOST_CONN \copybrief SL_STATUS_NET_MQTT_LOST_CONN
 * @return \ref SL_STATUS_NET_MQTT_PROTOCOL \copybrief SL_STATUS_NET_MQTT_PROTOCOL
 * @return \ref SL_STATUS_ERRNO \copybrief SL_STATUS_ERRNO
 */
sl_status_t mqtt_wrapper_loop(void *instance, int timeout);

/**
 * @brief Carry out read-operations on the network-socket connected to the MQTT-broker
 *
 * @param instance Pointer to a client-instance.
 * @return \ref SL_STATUS_OK \copybrief SL_STATUS_OK
 * @return \ref SL_STATUS_INVALID_PARAMETER \copybrief SL_STATUS_INVALID_PARAMETER
 * @return \ref SL_STATUS_ALLOCATION_FAILED \copybrief SL_STATUS_ALLOCATION_FAILED
 * @return \ref SL_STATUS_NET_MQTT_NO_CONN \copybrief SL_STATUS_NET_MQTT_NO_CONN
 * @return \ref SL_STATUS_NET_MQTT_LOST_CONN \copybrief SL_STATUS_NET_MQTT_LOST_CONN
 * @return \ref SL_STATUS_NET_MQTT_PROTOCOL \copybrief SL_STATUS_NET_MQTT_PROTOCOL
 * @return \ref SL_STATUS_ERRNO \copybrief SL_STATUS_ERRNO
 */
sl_status_t mqtt_wrapper_loop_read(void *instance);

/**
 * @brief Carry out write-opeartions on the network-socket connected to the MQTT-broker.
 *
 * @param instance Pointer to a client-instance.
 * @return \ref SL_STATUS_OK \copybrief SL_STATUS_OK
 * @return \ref SL_STATUS_INVALID_PARAMETER \copybrief SL_STATUS_INVALID_PARAMETER
 * @return \ref SL_STATUS_ALLOCATION_FAILED \copybrief SL_STATUS_ALLOCATION_FAILED
 * @return \ref SL_STATUS_NET_MQTT_NO_CONN \copybrief SL_STATUS_NET_MQTT_NO_CONN
 * @return \ref SL_STATUS_NET_MQTT_LOST_CONN \copybrief SL_STATUS_NET_MQTT_LOST_CONN
 * @return \ref SL_STATUS_NET_MQTT_PROTOCOL \copybrief SL_STATUS_NET_MQTT_PROTOCOL
 * @return \ref SL_STATUS_ERRNO \copybrief SL_STATUS_ERRNO
 */
sl_status_t mqtt_wrapper_loop_write(void *instance);

/**
 * @brief Carry out miscellaneous operations required as part of the nwtwork-loop (e.g. PINGs).
 *
 * @param instance Pointer to a client-instance.
 * @return \ref SL_STATUS_OK \copybrief SL_STATUS_OK
 * @return \ref SL_STATUS_INVALID_PARAMETER \copybrief SL_STATUS_INVALID_PARAMETER
 * @return \ref SL_STATUS_NET_MQTT_NO_CONN \copybrief SL_STATUS_NET_MQTT_NO_CONN
 */
sl_status_t mqtt_wrapper_loop_misc(void *instance);

/**
 * @brief Configures the MQTT Will message using the Client ID.
 *
 * @param instance          Pointer to a client-instance.
 * @param mqtt_client_id    String of the Client ID
 * @param qos               MQTT QoS for the Will message.
 *
 * @returns SL_STATUS_OK if successful, any other codes in case of error.
 */
sl_status_t mqtt_wrapper_set_will_message(void *instance,
                                          const char *mqtt_client_id,
                                          int qos);

/**
 * @brief Publish a message to a topic.
 *
 * @param instance Pointer to a client-instance.
 * @param mid Message-ID. Can be set to NULL.
 * @param topic Topic name to publish to.
 * @param payloadlen Size of payload in bytes.
 * @param payload Pointer to payload to send.
 * @param qos QoS required for this message.
 * @param retain Set to true if message should be retained on the broker.
 * @return \ref SL_STATUS_OK \copybrief SL_STATUS_OK
 * @return \ref SL_STATUS_INVALID_PARAMETER \copybrief SL_STATUS_INVALID_PARAMETER
 * @return \ref SL_STATUS_ALLOCATION_FAILED \copybrief SL_STATUS_ALLOCATION_FAILED
 * @return \ref SL_STATUS_NET_MQTT_NO_CONN \copybrief SL_STATUS_NET_MQTT_NO_CONN
 * @return \ref SL_STATUS_NET_MQTT_PROTOCOL \copybrief SL_STATUS_NET_MQTT_PROTOCOL
 * @return \ref SL_STATUS_NET_MQTT_PAYLOAD_SIZE \copybrief SL_STATUS_NET_MQTT_PAYLOAD_SIZE
 * @return \ref SL_STATUS_NET_MQTT_MALFORMED_UTF8 \copybrief SL_STATUS_NET_MQTT_MALFORMED_UTF8
 * @return \ref SL_STATUS_NET_MQTT_QOS_NOT_SUPPORTED \copybrief SL_STATUS_NET_MQTT_QOS_NOT_SUPPORTED
 * @return \ref SL_STATUS_NET_MQTT_OVERSIZE_PACKET \copybrief SL_STATUS_NET_MQTT_OVERSIZE_PACKET
 */
sl_status_t mqtt_wrapper_publish(void *instance,
                                 int *mid,
                                 const char *topic,
                                 int payloadlen,
                                 const void *payload,
                                 int qos,
                                 bool retain);

/**
 * @brief Subscribe to a topic
 *
 * @param instance Pointer to a client-instance.
 * @param mid Message-ID. Can be set to NULL.
 * @param sub Topic/pattern to subscribe to.
 * @param qos Requested QoS for this subscription.
 * @return \ref SL_STATUS_OK \copybrief SL_STATUS_OK
 * @return \ref SL_STATUS_INVALID_PARAMETER \copybrief SL_STATUS_INVALID_PARAMETER
 * @return \ref SL_STATUS_ALLOCATION_FAILED \copybrief SL_STATUS_ALLOCATION_FAILED
 * @return \ref SL_STATUS_NET_MQTT_NO_CONN \copybrief SL_STATUS_NET_MQTT_NO_CONN
 * @return \ref SL_STATUS_NET_MQTT_MALFORMED_UTF8 \copybrief SL_STATUS_NET_MQTT_MALFORMED_UTF8
 * @return \ref SL_STATUS_NET_MQTT_OVERSIZE_PACKET \copybrief SL_STATUS_NET_MQTT_OVERSIZE_PACKET
 */
sl_status_t
  mqtt_wrapper_subscribe(void *instance, int *mid, const char *sub, int qos);

/**
 * @brief Unsubscribe from a topic.
 *
 * @param instance Pointer to a client-instance.
 * @param mid Message-ID. Can be set to NULL.
 * @param sub Topic/pattern to unsubscribe from.
 * @return \ref SL_STATUS_OK \copybrief SL_STATUS_OK
 * @return \ref SL_STATUS_INVALID_PARAMETER \copybrief SL_STATUS_INVALID_PARAMETER
 * @return \ref SL_STATUS_ALLOCATION_FAILED \copybrief SL_STATUS_ALLOCATION_FAILED
 * @return \ref SL_STATUS_NET_MQTT_NO_CONN \copybrief SL_STATUS_NET_MQTT_NO_CONN
 * @return \ref SL_STATUS_NET_MQTT_MALFORMED_UTF8 \copybrief SL_STATUS_NET_MQTT_MALFORMED_UTF8
 * @return \ref SL_STATUS_NET_MQTT_OVERSIZE_PACKET \copybrief SL_STATUS_NET_MQTT_OVERSIZE_PACKET
 */
sl_status_t mqtt_wrapper_unsubscribe(void *instance, int *mid, const char *sub);

/**
 * @brief Check whether a given topic matches a subscription-pattern.
 *
 * @param sub Subscription string to match topic against.
 * @param topic Topic to check
 * @param result Pointer to a bool to hold the result.
 * @return \ref SL_STATUS_OK \copybrief SL_STATUS_OK
 * @return \ref SL_STATUS_INVALID_PARAMETER \copybrief SL_STATUS_INVALID_PARAMETER
 * @return \ref SL_STATUS_ALLOCATION_FAILED \copybrief SL_STATUS_ALLOCATION_FAILED
 */
sl_status_t mqtt_wrapper_topic_matches_sub(const char *sub,
                                           const char *topic,
                                           bool *result);

/**
 * @brief Set the connect callback.
 *
 * This is called when the broker sends a CONNACK message in response to a connection.
 *
 * @param instance Pointer to a client-instance.
 * @param on_connect Function-pointer to a well-formed callback function: void callback(void *instance, void *obj, int result_code)
 *
 * \par Callback result codes
 * * 0 - success
 * * 1 - connection refused (unacceptable protocol version)
 * * 2 - connection refused (identifier rejected)
 * * 3 - connection refused (broker unavailable)
 * * 4-255 - reserved for future use
 */
void mqtt_wrapper_connect_callback_set(
  void *instance, void (*on_connect)(void *inst, void *obj, int result_code));

/**
 * @brief Set the disconnect callback.
 *
 * This is called when the broker has received the DISCONNECT command and has disconnected the client.
 *
 * @param instance Pointer to a client-instance
 * @param on_disconnect Function-pointer to a well-formed callback function: void callback(void *instance, void *obj, int result_code)
 *
 * \par Callback result codes
 * * 0 - Success, i.e. the client has called its disconnect function.
 * * Any other value means that an unexpected disconnect has occurred.
 */
void mqtt_wrapper_disconnect_callback_set(
  void *instance, void (*on_disconnect)(void *inst, void *obj, int rc));

/**
 * @brief Set the message callback.
 *
 * This is called every time a new (published) message is received from the broker.
 *
 * @param instance Pointer to a client-instance
 * @param on_message Function-pointer to a well-formed callback function: void callback(void *instance, void *obj, const struct \ref mqtt_message *msg)
 */
void mqtt_wrapper_message_callback_set(
  void *instance,
  void (*on_message)(void *inst, void *obj, const struct mqtt_message *msg));

#ifdef __cplusplus
}
#endif

#endif
