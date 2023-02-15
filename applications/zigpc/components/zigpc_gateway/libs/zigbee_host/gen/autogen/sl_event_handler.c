#include "sl_event_handler.h"

#include "zigbee_app_framework_common.h"
#include "sl_iostream_stdio.h"
#include "sl_cli_instances.h"
#include "sl_iostream_init_instances.h"
#include "sl_cli_threaded_host.h"

void sl_platform_init(void)
{
}

void sl_driver_init(void)
{
}

void sl_service_init(void)
{
  sl_iostream_init_instances();
  sl_cli_instances_init();
  // mpbreton 2022-09-03 : Enabling the threaded cli causes interference.
  // Do no re-enable those lines in future autogeneration.
  //sli_cli_threaded_host_init();
}

void sl_stack_init(void)
{
  sli_zigbee_app_framework_init_callback();
}

void sl_internal_app_init(void)
{
}

void sl_platform_process_action(void)
{
}

void sl_service_process_action(void)
{
  sl_cli_instances_tick();
}

void sl_stack_process_action(void)
{
  sli_zigbee_app_framework_tick_callback();
}

void sl_internal_app_process_action(void)
{
}

void sl_iostream_init_instances(void)
{
  sl_iostream_stdio_init();
}

