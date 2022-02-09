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
#include "ncp_fixt.h"
#include "ncp.h"
#include "ncp_test_fixt.h"

extern void sl_bt_on_event(sl_bt_msg_t *evt);

int main(int argc, char **argv)
{
  // Setup fixtures
  uic_fixt_setup_step_t uic_fixt_setup_steps_list[] = {
    {ncp_fixt_setup, "NCP"},
    {ncp_test_setup, "NCP test"},
    {NULL, "Terminator"}
  };

  // Shutdown fixtures
  uic_fixt_shutdown_step_t uic_fixt_shutdown_steps_list[] = {
    {ncp_fixt_shutdown, "NCP"},
    {NULL, "Terminator"}
  };

  ncp_config_init();
  ncp_set_cb(sl_bt_on_event);
  ncp_test_config_init();

  // Run uic_main and return result
  return uic_main(uic_fixt_setup_steps_list,
                  uic_fixt_shutdown_steps_list,
                  argc,
                  argv,
                  CMAKE_PROJECT_VERSION);
}
