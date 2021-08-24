/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
// Standard library
#include <map>
#include <functional>
#include <string>

// System-wide libraries
#include <mosquitto.h>

// UIC components
#include "sl_status.h"
#include "sl_log.h"

// This component
#include "mqtt_wrapper.h"

// Some private typedefs to save typing.
typedef std::function<void(void *, void *, int)> on_connect_callback_t;
typedef std::function<void(void *, void *, int)> on_disconnect_callback_t;
typedef std::function<void(void *, void *, const struct mqtt_message *)>
  on_message_callback_t;

// Callback maps. Mosquitto itself has only one of each callback type for each
// instance respectively.
// In order to allow for multiple instances of Mosquitto (and multiple connections)
// we need to map our callbacks to whatever Mosquitto instance they belong to.
static std::map<struct mosquitto *, on_connect_callback_t> on_connect_map;
static std::map<struct mosquitto *, on_disconnect_callback_t> on_disconnect_map;
static std::map<struct mosquitto *, on_message_callback_t> on_message_map;
#define LOG_TAG "mqtt_wrapper_mosquitto"


// Private declaration of a function that converts between mosq_err_t and sl_status_t.
sl_status_t mqtt_wrapper_err_lut(int err);

sl_status_t mqtt_wrapper_lib_init()
{
  int major, minor, revision = 0;
  mosquitto_lib_version(&major, &minor, &revision);
  sl_log_debug(LOG_TAG,
               "Initializing Mosquitto library: %d.%d.%d\n",
               major,
               minor,
               revision);
  return mqtt_wrapper_err_lut(mosquitto_lib_init());
}

void *mqtt_wrapper_new(const char *id, bool clean_session, void *obj)
{
  return (void *)mosquitto_new(id, clean_session, obj);
}

static int pw_callback(char *buf, int size, int rwflag, void *userdata)
{
    std::string passphrase = "";
    buf = (char *)passphrase.c_str();
    size = passphrase.size();
    return size;
}

sl_status_t mqtt_wrapper_tls_set(void *instance,
                                 const char *cafile,
                                 const char *certfile,
                                 const char *keyfile)
{
    sl_log_debug(LOG_TAG, "mqtt_wrapper_tls_set\n");
  return mqtt_wrapper_err_lut(
    mosquitto_tls_set(static_cast<struct mosquitto *>(instance),
                      cafile,
                      nullptr,
                      certfile,
                      keyfile,
                      pw_callback));
}

sl_status_t mqtt_wrapper_tls_psk_set(void *instance,
                                     const char *psk,
                                     const char *id,
                                     const char *ciphers)
{
    sl_log_debug(LOG_TAG, "id: %s psk: %s\n", id, psk);
  return mqtt_wrapper_err_lut(
    mosquitto_tls_psk_set(static_cast<struct mosquitto *>(instance),
                          psk,
                          id,
                          ciphers));
}

sl_status_t mqtt_wrapper_lib_cleanup()
{
  return mqtt_wrapper_err_lut(mosquitto_lib_cleanup());
}

void mqtt_wrapper_destroy(void *instance)
{
  mosquitto_destroy(static_cast<struct mosquitto *>(instance));
}

int mqtt_wrapper_socket(void *instance)
{
  return mosquitto_socket(static_cast<struct mosquitto *>(instance));
}

sl_status_t mqtt_wrapper_connect(void *instance,
                                 const char *host,
                                 int port,
                                 int keepalive)
{
  return mqtt_wrapper_err_lut(
    mosquitto_connect(static_cast<struct mosquitto *>(instance),
                      host,
                      port,
                      keepalive));
}

sl_status_t mqtt_wrapper_disconnect(void *instance)
{
  return mqtt_wrapper_err_lut(
    mosquitto_disconnect(static_cast<struct mosquitto *>(instance)));
}

sl_status_t mqtt_wrapper_loop(void *instance, int timeout)
{
  return mqtt_wrapper_err_lut(
    mosquitto_loop(static_cast<struct mosquitto *>(instance), timeout, 1));
}

sl_status_t mqtt_wrapper_loop_read(void *instance) {
  return mqtt_wrapper_err_lut(
    mosquitto_loop_read(static_cast<struct mosquitto *>(instance), 1));
}

sl_status_t mqtt_wrapper_loop_write(void *instance)
{
  return mqtt_wrapper_err_lut(
    mosquitto_loop_write(static_cast<struct mosquitto *>(instance), 1));
}

sl_status_t mqtt_wrapper_loop_misc(void *instance)
{
  return mqtt_wrapper_err_lut(
    mosquitto_loop_misc(static_cast<struct mosquitto *>(instance)));
}

sl_status_t mqtt_wrapper_publish(void *instance,
                                 int *mid,
                                 const char *topic,
                                 int payloadlen,
                                 const void *payload,
                                 int qos,
                                 bool retain)
{
  return mqtt_wrapper_err_lut(
    mosquitto_publish(static_cast<struct mosquitto *>(instance),
                      mid,
                      topic,
                      payloadlen,
                      payload,
                      qos,
                      retain));
}

sl_status_t
  mqtt_wrapper_subscribe(void *instance, int *mid, const char *sub, int qos)
{
  return mqtt_wrapper_err_lut(
    mosquitto_subscribe(static_cast<struct mosquitto *>(instance),
                        mid,
                        sub,
                        qos));
}

sl_status_t mqtt_wrapper_unsubscribe(void *instance, int *mid, const char *sub)
{
  return mqtt_wrapper_err_lut(
    mosquitto_unsubscribe(static_cast<struct mosquitto *>(instance), mid, sub));
}

// On-connect wrappers
void mqtt_wrapper_on_connect_mosquitto(struct mosquitto *instance,
                                       void *obj,
                                       int rc)
{
  on_connect_map[instance]((void *)instance, obj, rc);
}

void mqtt_wrapper_connect_callback_set(void *instance,
                                       void (*on_connect)(void *, void *, int))
{
  on_connect_map[static_cast<struct mosquitto *>(instance)] = on_connect;
  mosquitto_connect_callback_set(static_cast<struct mosquitto *>(instance),
                                 mqtt_wrapper_on_connect_mosquitto);
}

// On-disconnect wrappers
void mqtt_wrapper_on_disconnect_mosquitto(struct mosquitto *instance,
                                          void *obj,
                                          int rc)
{
  on_disconnect_map[instance]((void *)instance, obj, rc);
}

void mqtt_wrapper_disconnect_callback_set(
  void *instance, void (*on_disconnect)(void *, void *, int))
{
  on_disconnect_map[static_cast<struct mosquitto *>(instance)] = on_disconnect;
  mosquitto_disconnect_callback_set(static_cast<struct mosquitto *>(instance),
                                    mqtt_wrapper_on_disconnect_mosquitto);
}

// On_message wrappers
void mqtt_wrapper_on_message_mosquitto(struct mosquitto *instance,
                                       void *obj,
                                       const struct mosquitto_message *msg)
{
  struct mqtt_message new_message;
  new_message.mid        = msg->mid;
  new_message.topic      = msg->topic;
  new_message.payload    = msg->payload;
  new_message.payloadlen = msg->payloadlen;
  new_message.qos        = msg->qos;
  new_message.retain     = msg->retain;
  on_message_map[instance]((void *)instance, obj, &new_message);
  // Mosquitto will free underlying data.
}

void mqtt_wrapper_message_callback_set(
  void *instance, void (*on_message)(void *, void *, const mqtt_message *))
{
  on_message_map[static_cast<struct mosquitto *>(instance)] = on_message;
  mosquitto_message_callback_set(static_cast<struct mosquitto *>(instance),
                                 mqtt_wrapper_on_message_mosquitto);
}

sl_status_t mqtt_wrapper_topic_matches_sub(const char *sub,
                                           const char *topic,
                                           bool *result)
{
  return mqtt_wrapper_err_lut(mosquitto_topic_matches_sub(sub, topic, result));
}

const sl_status_t sl_status_lut[] = {
  SL_STATUS_FAIL,                         // MOSQ_ERR_AUTH_CONTINUE = -4,
  SL_STATUS_FAIL,                         // MOSQ_ERR_NO_SUBSCRIBERS = -3,
  SL_STATUS_FAIL,                         // MOSQ_ERR_SUB_EXISTS = -2,
  SL_STATUS_FAIL,                         // MOSQ_ERR_CONN_PENDING = -1,
  SL_STATUS_OK,                           // MOSQ_ERR_SUCCESS = 0,
  SL_STATUS_ALLOCATION_FAILED,            // MOSQ_ERR_NOMEM = 1,
  SL_STATUS_NET_MQTT_PROTOCOL,            // MOSQ_ERR_PROTOCOL = 2,
  SL_STATUS_INVALID_PARAMETER,            // MOSQ_ERR_INVAL = 3,
  SL_STATUS_NET_MQTT_NO_CONN,             // MOSQ_ERR_NO_CONN = 4,
  SL_STATUS_FAIL,                         // MOSQ_ERR_CONN_REFUSED = 5,
  SL_STATUS_FAIL,                         // MOSQ_ERR_NOT_FOUND = 6,
  SL_STATUS_NET_MQTT_LOST_CONN,           // MOSQ_ERR_CONN_LOST = 7,
  SL_STATUS_NET_MQTT_TLS_HANDSHAKE,       // MOSQ_ERR_TLS = 8,
  SL_STATUS_NET_MQTT_PAYLOAD_SIZE,        // MOSQ_ERR_PAYLOAD_SIZE = 9,
  SL_STATUS_NET_MQTT_NOT_SUPPORTED,       // MOSQ_ERR_NOT_SUPPORTED = 10,
  SL_STATUS_NET_MQTT_AUTH,                // MOSQ_ERR_AUTH = 11,
  SL_STATUS_NET_MQTT_ACL_DENIED,          // MOSQ_ERR_ACL_DENIED = 12,
  SL_STATUS_FAIL,                         // MOSQ_ERR_UNKNOWN = 13,
  SL_STATUS_ERRNO,                        // MOSQ_ERR_ERRNO = 14,
  SL_STATUS_FAIL,                         // MOSQ_ERR_EAI = 15,
  SL_STATUS_FAIL,                         // MOSQ_ERR_PROXY = 16,
  SL_STATUS_FAIL,                         // MOSQ_ERR_PLUGIN_DEFER = 17,
  SL_STATUS_NET_MQTT_MALFORMED_UTF8,      // MOSQ_ERR_MALFORMED_UTF8 = 18,
  SL_STATUS_FAIL,                         // MOSQ_ERR_KEEPALIVE = 19,
  SL_STATUS_FAIL,                         // MOSQ_ERR_LOOKUP = 20,
  SL_STATUS_FAIL,                         // MOSQ_ERR_MALFORMED_PACKET = 21,
  SL_STATUS_NET_MQTT_DUPLICATE_PROPERTY,  // MOSQ_ERR_DUPLICATE_PROPERTY = 22,
  SL_STATUS_NET_MQTT_TLS_HANDSHAKE,       // MOSQ_ERR_TLS_HANDSHAKE = 23,
  SL_STATUS_NET_MQTT_QOS_NOT_SUPPORTED,   // MOSQ_ERR_QOS_NOT_SUPPORTED = 24,
  SL_STATUS_NET_MQTT_OVERSIZE_PACKET,     // MOSQ_ERR_OVERSIZE_PACKET = 25,
  SL_STATUS_FAIL                          // MOSQ_ERR_OCSP = 26,
};

// Even with all compiler optimizations turned off this is still pretty optimal.
sl_status_t mqtt_wrapper_err_lut(int err)
{
  constexpr int lut_size = sizeof(sl_status_lut) / sizeof(sl_status_t);
  if (err + 4 >= 0 && err < lut_size) {
    return sl_status_lut[err + 4];
  }
  return SL_STATUS_FAIL;
}
