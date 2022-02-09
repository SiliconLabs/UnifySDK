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

#include <stdlib.h>
#include "uic_main.h"
#include "dotdot_mqtt.h"
#include "positioning.h"

int main(int argc, char **argv)
{
  // Setup fixtures
  uic_fixt_setup_step_t uic_fixt_setup_steps_list[] = {
    /**
     * Main application init.
     */
    {&positioning_fixt_setup, "Positioning"},

    /**
     * Initialize DotDot MQTT handler that serializes and deserializes
     * dotdot messages.
     * All components registering callbacks to MQTT MUST be initialized before
     * this component.
     */
    {&uic_mqtt_dotdot_init, "DotDot MQTT"},
    {NULL, "Terminator"}
  };

  // Shutdown fixtures
  uic_fixt_shutdown_step_t uic_fixt_shutdown_steps_list[] = {
    {NULL, "Terminator"}
  };

  positioning_config_init();

  // Run uic_main and return result
  return uic_main(uic_fixt_setup_steps_list,
                  uic_fixt_shutdown_steps_list,
                  argc,
                  argv,
                  CMAKE_PROJECT_VERSION);
}
