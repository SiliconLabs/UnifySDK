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
#include "zpc_attribute_resolver_fixt.h"
#include "attribute_store_fixt.h"
#include "attribute_store.h"
#include "unify_dotdot_attribute_store.h"
#include "attribute_transitions.h"
#include "attribute_timeouts.h"
#include "attribute_mapper.h"
#include "zpc_attribute_mapper.h"
#include "datastore_fixt.h"
#include "dotdot_mapper_fixt.h"
#include "dotdot_mqtt.h"
#include "network_monitor_fixt.h"
#include "uic_main.h"
#include "ucl_mqtt.h"
#include "zpc_config_fixt.h"
#include "zpc_config.h"
#include "zpc_stdin_fixt.h"
#include "zpc_attribute_store.h"
#include "zwave_controller.h"
#include "zwave_command_handler.h"
#include "zwave_network_management_fixt.h"
#include "zwave_rx_fixt.h"
#include "zwave_s2_fixt.h"
#include "zwave_smartstart_management_fixt.h"
#include "zwave_tx_fixt.h"
#include "zwave_transports_fixt.h"
#include "zcl_cluster_servers.h"
#include "zpc_application_monitoring.h"
#include "zpc_dotdot_mqtt.h"
#include "zpc_datastore_fixt.h"
#include "zwave_rust_handlers.h"
#include "request_poller.h"
#include "zpc_ncp_update_fixt.h"
#include "rust_contiki_fixt.h"
#include "rust_attribute_store.h"
#include "zwave_poll_manager.h"

// Z-Wave Command Classes
#include "zwave_command_classes_fixt.h"

static uic_fixt_setup_step_t uic_fixt_setup_steps_list[] = {
  /** Initialize zpc_config. NB: This shall be first in the setup fixtures list */
  {&zpc_config_fixt_setup, "ZPC Config"},
  {&zpc_ncp_update_fixt_setup, "ZPC NCP update"},
  /** Configures data for the ApplicationMonitoring cluster */
  {&zpc_application_monitoring_init, "ZPC ApplicationMonitoring"},
  /** Initialize data store.  The data-store component depends on the
   * configuration component for file locations. */
  {&zpc_datastore_fixt_setup, "Datastore"},
  /** Connect to the Z-Wave API interface and start up the RX part.
   * The Z-Wave API depends on the configuration component. */
  {&zwave_rx_fixt_setup, "Z-Wave RX"},
  /** Start Z-Wave TX */
  {&zwave_tx_fixt_setup, "Z-Wave TX"},
  /** Let the Network management initialize its state machine
      and cache our NodeID/ HomeID from the Z-Wave API */
  {&zwave_network_management_fixt_setup, "Z-Wave Network Management"},
  /** Start Z-Wave S2 */
  {&zwave_s2_fixt_setup, "Z-Wave S2"},

  {&initialize_request_poller_process, "Rust task executor"},

  /** Initializes all Z-Wave transports */
  {&zwave_transports_init, "Z-Wave Transports"},

  /**
   * Initialize the attribute store library.
   * Z-Wave RX, TX and the datastore shall be initialized first.
   */
  {&attribute_store_init, "Attribute store"},
  /**
   * Initialize the rust attribute store library. need to be called
   * post attribute_store_init
   */
  {&rust_attribute_store_init, "Rust attribute store"},

  /**
   * Initializes the attribute transitions functionality.
   * The attribute store must be initialized first
   */
  {&attribute_transitions_init, "Attribute transitions"},

  /**
   * Initializes the attribute timeouts functionality.
   * The attribute store must be initialized first
   */
  {&attribute_timeouts_init, "Attribute timeouts"},

  /**
   * Initialize the ZPC attribute resolver, and the Unify part of it
   * Keep that right before the network monitor,
   * in order to get proper paused/enabled resolution right after init.
   */
  {&zpc_attribute_resolver_init, "ZPC attribute resolver"},

  /**
   * Initialize the Attribute Poll Engine, this shall be initialized before
   * registering any attributes for polling.
   */
  {&zwave_poll_manager_init, "zwave attribute poller"},

  /**
   * Network monitor starts, sets our UNID to zwave_unid
   * and verify that everything that should be in the Attribute Store
   * is there.
   * Requires Network Management, Attribute Store and ZPC Attribute Resolver
   * to be initialized.
   * It pauses the resolution of other networks and activates our network
   * for the resolver.
   */
  {&network_monitor_setup_fixt, "Network Monitor"},

  /**
   * Initialize DotDot MQTT Topics handler
   */
  {&zpc_dotdot_mqtt_init, "ZPC DotDot MQTT Topics"},
  /**
   * Initialize OnOff cluster mapper.
   * The cluster_serializer and attribute store MUST be initialized first.
   */
  {&dotdot_mapper_init, "DotDot mapper"},

  /**
   * Initialize ZCL Cluster servers (simulated clusters
   * that the ZPC serves on behalf of the nodes)
   */
  {&zcl_cluster_servers_init, "ZCL Cluster servers"},

  /**
   * UCL MQTT initialize all ucl mqtt handlers.
   */
  {&ucl_mqtt_setup_fixt, "UCL MQTT"},

  /**
   * Initializes the ZCL/DotDot specialization of the Attribute Store.
   * MUST be initialized before uic_mqtt_dotdot_init
   */
  {&unify_dotdot_attribute_store_init, "Unify DotDot Attribute Store"},

  /**
   * Initialize DotDot MQTT handler that serializes and deserializes
   * dotdot messages.
   * All components registering callbacks to MQTT MUST be initialized before
   * this component.
   */
  {&uic_mqtt_dotdot_init, "DotDot MQTT"},

  /** Initializes all supported / controlled CCs */
  {&zwave_command_class_init_rust_handlers, "Z-Wave Rust Command Classes"},
  {&zwave_command_class_init_rust_handlers_legacy,
   "Z-Wave legacy Command Classes"},

  {&zwave_command_classes_init, "Z-Wave Command Classes"},

  /** Initialize the Z-Wave Command handler framework after the Command Classes */
  {&zwave_command_handler_init, "Z-Wave Command Handler framework"},

  {&zwave_smartstart_management_setup_fixt, "Z-Wave SmartStart Management"},

  {&zpc_stdin_setup_fixt, "ZPC Stdin"},
  {&zpc_attribute_mapper_init, "ZPC Attribute Mapper"},

  /**
   * Initialize attribute_store specifically for ZPC.
   * Requires Network Management, Attribute Store and Attribute Mapper to be initialized.
   * This component will trigger an update callback for all attributes under our current HomeID.
   * It should be initialized last.
   */
  {&zpc_attribute_store_init, "ZPC Attribute Store"},

  {NULL, "Terminator"}};

/** Final tear-down steps.
 *
 * @ingroup uic_fixt_int
 *
 * Functions to take down components and free allocated resources. The
 * function are executed in order, after all contiki processes have
 * been stopped.
 */
static uic_fixt_shutdown_step_t uic_fixt_shutdown_steps_list[]
  = {{&zcl_cluster_servers_teardown, "ZCL Cluster servers"},
     {&zpc_attribute_resolver_teardown, "ZPC attribute resolver"},
     {&zwave_command_handler_teardown, "Z-Wave Command Handler framework"},
     {&zwave_network_management_fixt_teardown, "Z-Wave Network Management"},
     {&zwave_rx_fixt_teardown, "Z-Wave RX"},
     {&attribute_transitions_teardown, "Attribute transitions"},
     {&attribute_timeouts_teardown, "Attribute timeouts"},
     {&rust_contiki_teardown, "Rust Mainloop"},
     {&attribute_store_teardown, "Attribute store"},
     {&datastore_fixt_teardown, "Datastore"},
     {&dotdot_mapper_teardown, "DotDot mapper"},
     {NULL, "Terminator"}};

int main(int argc, char **argv)
{
  attribute_mapper_config_init();  //TODO how do we want to do this
  if (zpc_config_init()) {
    return -1;
  }

  return uic_main(uic_fixt_setup_steps_list,
                  uic_fixt_shutdown_steps_list,
                  argc,
                  argv,
                  CMAKE_PROJECT_VERSION);
}
