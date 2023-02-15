/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include <stddef.h>
#include "uic_main.h"
#include "config.h"
#include "datastore_fixt.h"
#include "zigpc_uic_datastore_fixt.h"
#include "attribute_store_fixt.h"
#include "dotdot_mqtt.h"

#include "zigpc_config.h"
#include "zigpc_config_fixt.h"
#include "zigpc_application_monitoring.h"
#include "zigpc_discovery.h"
#include "zigpc_net_mgmt_process.h"
#include "zigpc_command_mapper.h"
#include "zigpc_diagnostics.h"
#include "zigpc_smartstart_fixt.h"
#include "zigpc_ucl_fixt.h"
#include "attribute_management_process.h"
#include "zigpc_controller.h"
#include "zigpc_group_mgmt.h"
#include "zigpc_stdin_fixt.h"
#include "zcl_command_parser.h"
#include "zcl_attribute_info.h"
#include "zigpc_ota_zigbee.h"
#include "zigpc_ncp_updater.h"
#include "zigpc_binding_setup.h"

#include "zigpc_gateway_process.h"

/**
 * @brief ZigPC Pre-contiki set-up steps.
 *
 * The set-up steps are executed in order, before launching contiki
 * and starting the contiki processes.
 */
static uic_fixt_setup_step_t uic_fixt_setup_steps_list[] = {
  {zigpc_config_fixt_setup, "ZigPC Config"},
  /**
   * The data-store component depends on the config component for
   * database location.
   */
  {&zigpc_application_monitoring_init, "ZigPC ApplicationMonitoring"},
  {&zigpc_uic_datastore_fixt_setup, "Datastore"},
  {&attribute_store_init, "Attribute store"},
  {&zigpc_gateway_process_setup, "ZigPC Gateway"},
  {&zigpc_zclcmdparse_setup, "ZigPC ZCL Command Parser"},
  {&zigpc_zcl_profiles_init, "ZigPC ZCL Profiles"},
  {&zigpc_binding_setup, "ZigPC Binding"},
  {&zigpc_discovery_fixt_setup, "ZigPC Discovery"},
  {&zigpc_net_mgmt_process_setup, "ZigPC Network Management"},
  {&attribute_management_setup, "ZigPC Attribute Management"},
  {&zigpc_command_mapper_setup, "ZigPC Command Mapper"},
  {&zigpc_smartstart_setup, "ZigPC SmartStart"},
  {&zigpc_ota_init, "ZigPC OTA"},
  {&zigpc_ctrl_fixt_setup, "ZigPC Controller"},
  {&zigpc_diagnostics_init_fixt, " ZigPC Diagnostics"},
  {&zigpc_ucl_fixt_setup, "ZigPC UCL Util"},
  {&uic_mqtt_dotdot_init, " DotDot MQTT Translator"},
  {&zigpc_group_init, "ZigPC Groups Management"},
  {&zigpc_stdin_setup_fixt, "ZigPC STDIN"},
  {NULL, "Terminator"},
};

/**
 * @brief ZigPC Final tear-down steps.
 *
 * Functions to take down components and free allocated resources. The
 * function are executed in order, after all contiki processes have
 * been stopped.
 */
static uic_fixt_shutdown_step_t uic_fixt_shutdown_steps_list[] = {
  {zigpc_ucl_fixt_shutdown, "ZigPC UCL Util"},
  {zigpc_ctrl_fixt_shutdown, "ZigPC Controller"},
  {zigpc_smartstart_shutdown, "ZigPC SmartStart"},
  {zigpc_command_mapper_shutdown, "ZigPC Command Mapper"},
  {attribute_management_shutdown, "ZigPC Attribute Management"},
  {zigpc_net_mgmt_process_shutdown, "ZigPC Network Management"},
  {zigpc_discovery_fixt_shutdown, "ZigPC Discovery"},
  {zigpc_binding_shutdown, "ZigPC Binding"},
  {zigpc_gateway_process_shutdown, "ZigPC Gateway"},
  {attribute_store_teardown, "Attribute store"},
  {datastore_fixt_teardown, "Datastore"},
  {NULL, "Terminator"},
};

static uic_fixt_setup_step_t zigpc_ncp_updater_setup_list[]
  = {{zigpc_gateway_process_setup, "ZigPC Gateway"},
     {zigpc_ncp_updater_setup, "ZigPC NCP Updater"},
     {NULL, "TERMINATOR"}};

static uic_fixt_shutdown_step_t zigpc_ncp_updater_shutdown_list[]
  = {{zigpc_gateway_process_shutdown, "ZigPC Gateway"},
     {zigpc_ncp_updater_shutdown, "ZigPC NCP Updater"},
     {NULL, "TERMINATOR"}};

int main(int argc, char **argv)
{
  // TODO: Investigate possibilities of integrating config_init into the uic_main
  if (zigpc_config_init()) {
    return -1;
  }

  config_status_t config_status
    = config_parse(argc, argv, CMAKE_PROJECT_VERSION);

  if (config_status != CONFIG_STATUS_OK) {
    return -1;
  }

  int main_status = 0;

  config_status_t has_ncp_update_flag = config_has_flag("zigpc.ncp_update");

  if (CONFIG_STATUS_OK == has_ncp_update_flag) {
    main_status = uic_main(zigpc_ncp_updater_setup_list,
                           zigpc_ncp_updater_shutdown_list,
                           argc,
                           argv,
                           CMAKE_PROJECT_VERSION);
  } else {
    main_status = uic_main(uic_fixt_setup_steps_list,
                           uic_fixt_shutdown_steps_list,
                           argc,
                           argv,
                           CMAKE_PROJECT_VERSION);
  }

  return main_status;
}
