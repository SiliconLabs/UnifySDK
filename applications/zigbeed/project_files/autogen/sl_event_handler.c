#include "sl_event_handler.h"

#include "zigbee_app_framework_common.h"

void sl_platform_init(void)
{
}

void sl_driver_init(void)
{
}

void sl_service_init(void)
{
}

void sl_stack_init(void)
{
  sli_zigbee_stack_init_callback();
  sli_zigbee_app_framework_init_callback();
  sli_zigbee_app_framework_sleep_init();
  sli_zigbee_ncp_init_callback();
}

void sl_internal_app_init(void)
{
}

void sl_platform_process_action(void)
{
}

void sl_service_process_action(void)
{
}

void sl_stack_process_action(void)
{
  sli_zigbee_stack_tick_callback();
  sli_zigbee_app_framework_tick_callback();
  sli_zigbee_ncp_tick_callback();
}

void sl_internal_app_process_action(void)
{
}

