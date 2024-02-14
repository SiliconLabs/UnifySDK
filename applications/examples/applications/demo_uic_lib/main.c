#include <stddef.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "uic_main.h"
#include "uic_main_externals.h"  // for registering fd
#include "process.h"
#include "config.h"
#include "sl_log.h"
#include "sl_status.h"
#include "uic_mqtt.h"
#include "datastore_fixt.h"
#include "datastore.h"

// Application includes
#include "demo_uic_cli.h"

// Log tag used in this file
#define LOG_TAG "demo process"

typedef enum { DEMO_EVENT_MQTT } demo_process_event_t;

struct demo_mqtt_payload {
  unsigned int size;
  char *topic;
  char *payload;
};

static int udp_sock_fd = -1;

// Demo process definition
PROCESS(demo_process, "Demo process");

// MQTT Callback, called when an MQTT message is received on the subscribed topic
void demo_mqtt_callback(const char *topic,
                        const char *payload,
                        size_t payload_size)
{
  // Allocate a copy of the topic, payload and payload size,
  // and send it to the demo process for decoupling
  struct demo_mqtt_payload *demo_mqtt_payload
    = malloc(sizeof(demo_mqtt_payload));
// TODO, may be reworked to avoid build errors
#if defined(__GNUC__) && (__GNUC__ == 12)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
  demo_mqtt_payload->topic = malloc(strnlen(topic, 100));
  memcpy(demo_mqtt_payload->topic, topic, strnlen(topic, 100));
  demo_mqtt_payload->payload = malloc(payload_size);
  memcpy(demo_mqtt_payload->payload, payload, payload_size);
  demo_mqtt_payload->size = payload_size;
#if defined(__GNUC__) && (__GNUC__ == 12)
#pragma GCC diagnostic pop
#endif
  process_post(&demo_process, DEMO_EVENT_MQTT, demo_mqtt_payload);
}

// Setup fixture, called on start-up
sl_status_t demo_setup(void)
{
  // Setup cli
  demo_uic_cli_setup();

  // Start the demo process
  process_start(&demo_process, NULL);

  // Subscribe to an MQTT-topic
  uic_mqtt_subscribe("Demo/DemoTopic", demo_mqtt_callback);

  //////////////////////////////////////////////////////////////////////////////
  // Setup UDP socket for listening (demo on fd)
  //////////////////////////////////////////////////////////////////////////////
  // Read port from config
  int port;
  if (CONFIG_STATUS_OK != config_get_as_int("demo.port", &port)) {
    sl_log_error(LOG_TAG, "Failed to read 'demo.port' from config");
    return SL_STATUS_FAIL;
  }

  // Setup socket
  struct sockaddr_in servaddr;
  if ((udp_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    sl_log_error(LOG_TAG, "Failed to setup socket");
    return SL_STATUS_FAIL;
  }
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;  // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port        = htons(port);
  if (bind(udp_sock_fd, (const struct sockaddr *)&servaddr, sizeof(servaddr))
      < 0) {
    sl_log_error(LOG_TAG, "Failed to bind socket");
    close(udp_sock_fd);
    return SL_STATUS_FAIL;
  }
  sl_log_debug(LOG_TAG,
               "Successfully opened socket (fd: %d) on port: %d",
               udp_sock_fd,
               port);
  // Register FD in uic_main,
  // causing uic_main to do a PROCESS_POLL on demo_process.
  uic_main_ext_register_rfd(udp_sock_fd, NULL, &demo_process);
  return SL_STATUS_OK;
}

// Shutdown fixture, called on shut-down
int demo_shutdown(void)
{
  sl_log_debug(LOG_TAG, "Cleaning up resources");
  close(udp_sock_fd);
  return 0;
}

// Poll handler (called when uic_main signals something on udp_sock_fd)
void demo_process_poll_handler()
{
  // Read from socket and print to log
  char buf[1001] = {0};
  recv(udp_sock_fd,
       buf,
       sizeof(buf) - 1,
       0);  // save room in buf for 0 termination
  sl_log_debug(LOG_TAG, "Received: %s", buf);
}

// Demo process implementation
PROCESS_THREAD(demo_process, ev, data)
{
  const char *demo_string;
  // Handle process poll
  PROCESS_POLLHANDLER(demo_process_poll_handler());

  PROCESS_BEGIN()
  while (1) {
    switch (ev) {
      case PROCESS_EVENT_INIT:
        if (CONFIG_STATUS_OK
            != config_get_as_string("demo.string", &demo_string)) {
          // failed to get demo.string configuration
          return -1;
        }
        sl_log_debug(LOG_TAG, "Init process");
        sl_log_debug(LOG_TAG, "config demo_string %s", demo_string);
        const char *demo_message = "This is a demo message.";
        uic_mqtt_publish("Demo/DemoTopic",
                         demo_message,
                         strlen(demo_message),
                         false);
        int64_t answer                 = 0;
        datastore_key_t the_answer_key = "the answer to everything";
        sl_log_debug(LOG_TAG, "Reading \"%s\"", the_answer_key);

        if (!datastore_contains_int(the_answer_key)) {
          sl_log_debug(LOG_TAG,
                       "\"%s\" is now being set to: %d",
                       the_answer_key,
                       42);
          datastore_store_int(the_answer_key, 42);
        }
        sl_status_t datastore_status
          = datastore_fetch_int(the_answer_key, &answer);
        assert(datastore_status == SL_STATUS_OK);
        sl_log_debug(LOG_TAG,
                     "\"%s\" is %d, status:%d",
                     the_answer_key,
                     answer,
                     datastore_status);
        break;
      case PROCESS_EVENT_EXIT:
        sl_log_debug(LOG_TAG, "Exiting process");
        break;
      case DEMO_EVENT_MQTT: {
        // use the payload from data
        struct demo_mqtt_payload *payload = (struct demo_mqtt_payload *)data;
        sl_log_debug(LOG_TAG, "[%s]: %s\n", payload->topic, payload->payload);
        // remember to free allocated memory
        free(payload->topic);
        free(payload->payload);
        free(payload);
      }
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}

/**
 * @brief Wrapper for the datastore initialization function
 *
 */
sl_status_t uic_demo_datastore_fixt_setup()
{
  // Here the application determines which file they want to use for the datastore
  const char *datastore_file = "/tmp/uic_demo.db";

  // Then initialize the datastore with this file
  return datastore_fixt_setup(datastore_file);
}

/**
 * @brief Main function
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char **argv)
{
  // Initialize configuration
  config_add_string("demo.string", "Demo variable", "");
  config_add_int("demo.port", "Port number for UDP server", 4242);

  // Setup fixtures
  uic_fixt_setup_step_t uic_fixt_setup_steps_list[]
    = {{&uic_demo_datastore_fixt_setup, "Datastore"},
       {&demo_setup, "Demo Setup"},
       {NULL, "Terminator"}};

  // Shutdown fixtures
  uic_fixt_shutdown_step_t uic_fixt_shutdown_steps_list[]
    = {{demo_shutdown, "Demo Shutdown"},
       {datastore_fixt_teardown, "Datastore Shutdown"},
       {NULL, "Terminator"}};

  // Run uic_main and return result
  return uic_main(uic_fixt_setup_steps_list,
                  uic_fixt_shutdown_steps_list,
                  argc,
                  argv,
                  CMAKE_PROJECT_VERSION);
}
