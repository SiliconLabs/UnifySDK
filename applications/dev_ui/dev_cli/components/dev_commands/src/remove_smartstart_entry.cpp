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
#include <cstring>

// Import command handler class and project libraries
#include "dev_cli_config.h"
#include "dev_cli_command_runner.h"
#include "dev_cli_command_handler.hpp"
#include "command_smartstartlist_lib.hpp"

// Process events for a command
#include "dev_cli_run.h"

// UIC component
#include "sys/process.h"
#include "uic_mqtt.h"
#include "sl_log.h"

#define LOG_TAG "remove_smartstart_entry"

// Pre decleration of the main function for command
int remove_smart_start_entry_main();

// Pre decleration of functions used in main function
int remove_entry_smart_start_list(const char *dsk_key);

/**
 * @brief Removes a SmartStart entry
 *
 * The given DSK key provided by user will be removed from the SmartStart list
 * owned by the MQTT broker one are connected to.
 *
 * @ingroup dev_cli_commands Developer CLI commands
 *
 */
CommandTemplate
  RemoveSmartStartEntry("removesmartstartentry",
                        "Removing SmartStart entry based on dsk key",
                        "usage: [dev_cli.host *.port *.dsk]",
                        remove_smart_start_entry_main);

int remove_smart_start_entry_main()
{
  sl_log_debug(LOG_TAG, "Removing SmartStart entry from SmartStart list\n");

  const char *dsk_key = dev_cli_get_config()->dsk;
  // Publishing the dsk key to remove
  int state = remove_entry_smart_start_list(dsk_key);

  // User information
  if (state == 0) {
    printf("Removed SmartStart entry: %s\n", dsk_key);
  } else {
    printf("Were unable to add dsk key to SmartStart list DSK: %s\n", dsk_key);
  }

  // End process
  process_post(PROCESS_BROADCAST, PROCESS_COMMAND_DONE, 0);
  return 0;
}

// Publish a DSK for removing an entry from the provisioning list
int remove_entry_smart_start_list(const char *dsk_key)
{
  int size_mesg = 1024;
  char remove[size_mesg];

  if (std::strcmp(dsk_key, "") != 0) {
    snprintf(remove, size_mesg, "{\"DSK\" : \"%s\"}", dsk_key);
    uic_mqtt_publish("ucl/SmartStart/List/Remove",
                     remove,
                     strlen(remove),
                     false);
    sl_log_debug(LOG_TAG, "Published command to remove DSK: %s\n", dsk_key);
    return 0;
  } else {
    const char *error_msg = "No user input of DSK key ignoring update";
    sl_log_error(LOG_TAG, "%s\n", error_msg);
    return 1;
  }
};