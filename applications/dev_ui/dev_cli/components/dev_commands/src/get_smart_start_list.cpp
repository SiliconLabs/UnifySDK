/* # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

// Standard library
#include <unistd.h>

// Third party libraries
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

// Import command handler class and project libraries
#include "dev_cli_config.h"
#include "dev_cli_command_handler.hpp"
#include "command_smartstartlist_lib.hpp"

// Process events for a command
#include "dev_cli_run.h"

// UIC component
#include "uic_mqtt.h"
#include "sys/process.h"
#include "sl_log.h"

// Setup Log ID
#define LOG_TAG "get_smart_start_list"

// Pre decleration of the main function for command
int get_smart_start_list_main();

// Pre declaration of functions used in main
void print_smart_start_list();

/**
 * @brief Command retrieves the SmartStart list and displays it
 *
 * Connects to a MQTT broker, retrieves the SmartStart list and displays it
 *
 * @ingroup dev_cli_commands Developer CLI commands
 *
 */
CommandTemplate GetSmartStartList("getsmartstartlist",
                                  "Get the SmartStart list",
                                  "usage: [dev_cli.host *.port]",
                                  get_smart_start_list_main);

int get_smart_start_list_main()
{
  // TODO: Timeout the callback function if there is no retained message in
  // ucl/SmartStart/List CURRENT STATE: Subscribes to ucl/SmartStart/list and
  // waits for the callback to broadcast commands done.
  print_smart_start_list();

  // Process broadcast command done in callback function ->
  // command_smartstartlist_lib.cpp
  return 0;
}

// Short alias for this namespace
namespace bpt = boost::property_tree;

// SmartStart List json object
static bpt::ptree smart_start_list_json;
static std::vector<std::string> dsk_keys;

// Print specifically the SmartStart list in a json object
static void
  print_whole_smart_start_list(boost::property_tree::ptree const &smart_json)
{
  printf("\n\n");
  int entry_num = 0;
  printf("SmartStart List:\n\n");
  dsk_keys.clear();
  for (const bpt::ptree::value_type &entry: smart_json) {
    printf("Entry: %d\n", entry_num);
    for (auto iter = entry.second.begin(); iter != entry.second.end(); iter++) {
      printf("\t%s: %s\n", iter->first.c_str(), iter->second.data().c_str());

      if (strcmp(iter->first.c_str(), "DSK") == 0) {
        dsk_keys.push_back(iter->second.data());
      }
    }
    printf("\n");
    entry_num++;
  }
  printf("\n");
}

int parse_json_smart_start_list(const char *ss_list)
{
  std::stringstream ss(ss_list);
  try {
    // Try reading the string as json expecting a string formatted as json
    printf("Parsing SmartStart list\n");
    bpt::read_json(ss, smart_start_list_json);
    print_whole_smart_start_list(smart_start_list_json);
    return 0;
  } catch (const boost::property_tree::json_parser::json_parser_error &e) {
    // Error handling if the output of ucl/SmartStart/List is unable to be
    // parsed
    printf(
      "Error: Could not parse the JSON data for the SmartStart list.\n");
    return 1;
  }
}

// Saving SmartStart list on callback for subscrigin to ucl/SmartStart/List
static void print_smart_start_list_cb(const char *topic,
                                      const char *payload,
                                      size_t payload_size)
{
  sl_log_debug(LOG_TAG, "[%s]: %s\n", topic, payload);
  parse_json_smart_start_list(payload);
  process_post(PROCESS_BROADCAST, PROCESS_COMMAND_DONE, 0);
}

// Subscribe to the topic for the SmartStart list for printing it with callback
// function
void print_smart_start_list()
{
  uic_mqtt_subscribe("ucl/SmartStart/List", print_smart_start_list_cb);
}