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
#include <cstring>
#include <unistd.h>

// Import command handler class and project libraries
#include "dev_cli_config.h"
#include "dev_cli_command_runner.h"
#include "dev_cli_command_handler.hpp"
#include "command_smartstartlist_lib.hpp"

// Process events for a command
#include "dev_cli_run.h"

// UIC component
#include "sys/process.h"
#include "sl_log.h"
#include "uic_mqtt.h"

#define LOG_TAG "add_smartstart_entry"

// Pre decleration of the main function for command
int add_smart_start_entry_main();

// Pre declare functions used in main function
int update_smart_start_list(const char *dsk,
                            const char *include,
                            const char *protocolunid,
                            const char *unid);

/**
 * @brief This command class is used for adding smart start entry
 *
 * The command class can implement a smart start entry if user inputs DSK key,
 * Include, ProtocolControllerUnid and UNID.
 *
 * @ingroup dev_cli_commands Developer CLI commands
 *
 */
CommandTemplate AddSmartStartEntry(
  "addsmartstartentry",
  "Adding a smart start entry",
  "usage: [dev_clu.host *.port *.dsk *.include *.protocolunid *.unid]",
  add_smart_start_entry_main);

/**
 * @brief This command will add a smart start entry to the smart start list
 *
 * \ingroup DEV CLI COMMAND
 * @returns int
 */
int add_smart_start_entry_main()
{
  sl_log_debug(LOG_TAG, "Adding smart start entry to smart start list\n");
  sl_log_set_level(SL_LOG_ERROR);

  const dev_cli_config_t *config = dev_cli_get_config();

  // From the config input arguments are formatted to an update for MQTT broker
  int state = update_smart_start_list(config->dsk,
                                      config->include,
                                      config->protocolunid,
                                      config->unid);

  // User information
  if (state == 0) {
    printf("Added dsk key to smart start list DSK: %s\n", config->dsk);
  } else {
    printf("Were unable to add dsk key to smart start list DSK: %s\n",
           config->dsk);
  }

  // End process
  process_post(PROCESS_BROADCAST, PROCESS_COMMAND_DONE, 0);
  return 0;
}

// Update smart start list command
int update_smart_start_list(const char *dsk,
                            const char *include,
                            const char *protocolunid,
                            const char *unid)
{
  int size_mesg = 1024;
  char update[size_mesg];
  if (std::strcmp(dsk, "") != 0) {
    snprintf(update,
             size_mesg,
             "{\"DSK\": \"%s\", \"Include\": %s, \"ProtocolControllerUnid\": "
             "\"%s\", \"Unid\": \"%s\"}",
             dsk,
             include,
             protocolunid,
             unid);

    uic_mqtt_publish("ucl/SmartStart/List/Update",
                     update,
                     strlen(update),
                     false);

    sl_log_debug(LOG_TAG, "Updated smart start list on DSK key: %s\n", dsk);
    return 0;
  } else {
    const char *error_msg = "No user input of DSK key ignoring update";
    sl_log_error(LOG_TAG, "%s\n", error_msg);
    return 1;
  }
}