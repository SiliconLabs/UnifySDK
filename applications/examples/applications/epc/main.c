/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
// EPC Components
#include "epc_datastore_fixt.h"
#include "epc_config_fixt.h"
#include "epc_application_monitoring.h"
#include "epc_config.h"

// Unify components
#include "uic_main.h"
#include "datastore_fixt.h"
#include "attribute_store_fixt.h"
#include "dotdot_mqtt.h"
#include "unify_dotdot_attribute_store.h"
#include "sl_log.h"

// Generic incldues
#include <stdlib.h>

// Log tag
#define LOG_TAG "main"

// List of init functions.
static uic_fixt_setup_step_t uic_fixt_setup_steps_list[]
  = {{&epc_config_fixt_setup, "EPC Config"},
     {&epc_application_monitoring_init, "EPC ApplicationMonitoring"},
     {&epc_datastore_fixt_setup, "Datastore"},
     {&attribute_store_init, "Attribute store"},
     /**
      * Initializes the ZCL/DotDot specialization of the Attribute Store.
      * MUST be initialized before uic_mqtt_dotdot_init
      */
     {&unify_dotdot_attribute_store_init, "Unify DotDot Attribute Store"},
     {&uic_mqtt_dotdot_init, "DotDot MQTT"},
     {NULL, "Terminator"}};

// List of shutdown functions
static uic_fixt_shutdown_step_t uic_fixt_shutdown_steps_list[]
  = {{&attribute_store_teardown, "Attribute store"},
     {&datastore_fixt_teardown, "Datastore"},
     {NULL, "Terminator"}};

/**
 * @brief Main function for the Example Protocol Controller (EPC)
 *
 * @param argc
 * @param argv
 * @return 0 on success (EXIT_SUCCESS), any other code in case of error
 */
int main(int argc, char **argv)
{
  // Initialize the EPC Configuration, add new keys/default values as needed.
  if (epc_config_init()) {
    return EXIT_FAILURE;
  }

  sl_log_info(LOG_TAG, "Starting the EPC main loop");

  // Run uic_main and return result
  return uic_main(uic_fixt_setup_steps_list,
                  uic_fixt_shutdown_steps_list,
                  argc,
                  argv,
                  CMAKE_PROJECT_VERSION);
}
