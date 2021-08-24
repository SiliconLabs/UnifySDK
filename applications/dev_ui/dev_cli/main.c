#include "uic_main.h"
#include "dev_cli_run_fixt.h"
#include "dev_cli_config_fixt.h"
#include "dev_cli_config.h"

// NULL definition
#include <stddef.h>

static uic_fixt_setup_step_t uic_fixt_setup_steps_list[] = {

  {dev_cli_config_fixt_setup, "CLI UI Config"},

  {dev_cli_run_setup_fixt, "CLI UI Run"},

  {NULL, "Terminator"}};

/** Final tear-down steps.
 *
 * @ingroup uic_fixt_int
 *
 * Functions to take down components and free allocated resources. The
 * function are executed in order, after all contiki processes have
 * been stopped.
 */
static uic_fixt_shutdown_step_t uic_fixt_shutdown_steps_list[] = {

  {NULL, "Terminator"}};

int main(int argc, char **argv)
{
  if (dev_cli_config_init()) {
    return -1;
  }

  return uic_main(uic_fixt_setup_steps_list,
                  uic_fixt_shutdown_steps_list,
                  argc,
                  argv,
                  CMAKE_PROJECT_VERSION);
}
