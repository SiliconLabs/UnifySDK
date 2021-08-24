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
#include "datastore_fixt.h"
#include "attribute_store_fixt.h"
#include "dotdot_mqtt.h"

#include "zigpc_config.h"
#include "zigpc_config_fixt.h"
#include "zigpc_net_mgmt_process.h"
#include "zigpc_command_mapper.h"
#include "zigpc_diagnostics.h"
#include "zigpc_smartstart_fixt.h"
#include "zigpc_protctrl_process.h"
#include "zigpc_node_state.h"
#include "attribute_management_process.h"
#include "zigpc_node_mgmt_process.h"
#include "zigpc_group_mgmt.h"
#include "zigpc_stdin_fixt.h"
#include "zcl_command_parser.h"
#include "zcl_attribute_info.h"
#include "zigpc_ota_zigbee.h"

#include "zigpc_gateway_process.h"

/**
 * @brief ZigPC Pre-contiki set-up steps.
 *
 * The set-up steps are executed in order, before launching contiki
 * and starting the contiki processes.
 */
static uic_fixt_setup_step_t uic_fixt_setup_steps_list[] = {
  {zigpc_config_fixt_setup, "ZIGPC Config"},
  /**
   * The data-store component depends on the config component for
   * database location.
   */
  {datastore_fixt_setup, "Datastore"},
  {attribute_store_init, "Attribute store"},
  {zigpc_gateway_process_setup, "ZIGPC Gateway"},
  {zigpc_zclcmdparse_setup, "ZigPC ZCL Command Parser"},
  {zigpc_zcl_profiles_init, "ZigPC ZCL Profiles"},
  {zigpc_net_mgmt_process_setup, "ZIGPC Network Management"},
  {zigpc_command_mapper_setup, "ZIGPC Command Mapper"},
  {zigpc_node_state_setup, "ZIGPC Node State Monitor"},
  {zigpc_smartstart_setup, "ZIGPC SmartStart"},
  {attribute_management_setup, "ZIGPC Attribute Management"},
  {zigpc_node_mgmt_setup, "Zigpc Node Management"},
  {zigpc_diagnostics_init_fixt, " ZigPC Diagnostics"},
  {zigpc_protctrl_process_setup, "ZIGPC Protocol Controller Monitor"},
  {uic_mqtt_dotdot_init, " DotDot MQTT Translator"},
  {zigpc_group_init, "Zigpc Groups Management"},
  {zigpc_stdin_setup_fixt, "ZigPC STDIN"},
  {zigpc_ota_init, "Zigpc OTA"},
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
  {zigpc_protctrl_process_shutdown, "ZIGPC Protocol Controller Monitor"},
  {zigpc_smartstart_shutdown, "ZIGPC SmartStart"},
  {zigpc_node_state_shutdown, "ZIGPC Node State Monitor"},
  {zigpc_net_mgmt_process_shutdown, "ZIGPC Network Management"},
  {zigpc_gateway_process_shutdown, "ZIGPC Gateway"},
  {zigpc_command_mapper_shutdown, "ZIGPC Command Mapper"},
  {attribute_management_shutdown, "ZIGPC Attribute Management"},
  {zigpc_node_mgmt_shutdown, "Zigpc Node Management"},
  {attribute_store_teardown, "Attribute store"},
  {datastore_fixt_teardown, "Datastore"},
  {NULL, "Terminator"},
};

int main(int argc, char **argv)
{
  // TODO: Investigate possibilities of integrating config_init into the uic_main
  if (zigpc_config_init()) {
    return -1;
  }

  return uic_main(uic_fixt_setup_steps_list,
                  uic_fixt_shutdown_steps_list,
                  argc,
                  argv,
                  CMAKE_PROJECT_VERSION);
}
