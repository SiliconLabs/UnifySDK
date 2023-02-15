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

#include "uic_main.h"
#include "aoxpc_ncp_core_fixt.h"
#include "aoxpc_ncp_process_fixt.h"
#include "aoxpc_ncp_update_fixt.h"
#include "aoxpc.h"
#include "aoxpc_correction.h"
#include "aox_locator_configuration.h"
#include "aoxpc_application_monitoring.h"
#include "aoxpc_datastore_fixt.h"
#include "aoxpc_config.h"
#include "aoxpc_attribute_store_attr_type_name_helper.h"

// Unify shared components
#include "dotdot_mqtt.h"
#include "attribute_store_fixt.h"
#include "datastore_fixt.h"

// Generic incldues
#include <stdlib.h>

int main(int argc, char **argv)
{
  // Setup fixtures
  uic_fixt_setup_step_t uic_fixt_setup_steps_list[]
    = {/*
        * Establish connection with the Bluetooth Network Co-Processor.
        */
       {&aoxpc_ncp_core_fixt_setup, "NCP Core"},
       /*
        * Check for firmware update request and execute it.
        */
       {&aoxpc_ncp_update_fixt_setup, "NCP Update"},
       /*
        * Start receiving NCP events in its own process.
        */
       {&aoxpc_ncp_process_fixt_setup, "NCP Process"},
       {&aoxpc_application_monitoring_init, "AoXPC ApplicationMonitoring"},
       /*
        * Initialize AoXPC datastore for persistent storage
        */
       {&aoxpc_datastore_fixt_setup, "AoXPC Datastore"},
       /*
        * Initialize the attribute store library.
        * Datastore MUST be initialized first.
        */
       {&attribute_store_init, "Attribute store"},
       /**
        * Initialize the module to register all attribute used by the AoXPC and
        * enable the APIs to convert the attribute type name to attribute type id.
        */
       {&aoxpc_attribute_store_attr_type_name_helper_init,
        "AoXPC Attribute type name to id convertor"},
       /*
       * Initialize AoXLocator cluster resources.
       * Attribute Store MUST be initialized before.
       */
       {&aox_locator_configuration_init, "AoX Locator Cluster configuration"},
       /*
       * Subscribe for and interpret ApplyCorrection commands.
       */
       {&aoxpc_correction_init, "AoX Protocol Controller Correction"},
       /*
       * Initialize DotDot MQTT handler that serializes and deserializes
       * dotdot messages.
       * All components registering callbacks to MQTT MUST be initialized before
       * this component.
       */
       {&uic_mqtt_dotdot_init, "DotDot MQTT"},
       /*
       * Main application init.
       */
       {&aoxpc_fixt_setup, "AoX Protocol Controller"},
       {NULL, "Terminator"}};

  // Shutdown fixtures
  uic_fixt_shutdown_step_t uic_fixt_shutdown_steps_list[]
    = {{&attribute_store_teardown, "Attribute store"},
       {&datastore_fixt_teardown, "Datastore"},
       {NULL, "Terminator"}};

  // Initialize all configurations
  if (SL_STATUS_OK != aoxpc_config_init()) {
    return EXIT_FAILURE;
  }

  // Run uic_main and return result
  return uic_main(uic_fixt_setup_steps_list,
                  uic_fixt_shutdown_steps_list,
                  argc,
                  argv,
                  CMAKE_PROJECT_VERSION);
}
