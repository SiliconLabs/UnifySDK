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
#include "attribute_transitions.h"
#include "attribute_timeouts.h"
#include "attribute_mapper.h"
#include "datastore_fixt.h"
#include "dotdot_mqtt_topics_handler.h"
#include "dotdot_mapper_fixt.h"
#include "dotdot_mqtt.h"
#include "network_monitor_fixt.h"
#include "uic_main.h"
#include "ucl_mqtt.h"
#include "zpc_config_fixt.h"
#include "zpc_config.h"
#include "zpc_stdin_fixt.h"
#include "zpc_attribute_store.h"
#include "zwave_command_handler.h"
#include "zwave_network_management_fixt.h"
#include "zwave_rx_fixt.h"
#include "zwave_s2_fixt.h"
#include "zwave_smartstart_management_fixt.h"
#include "zwave_tx_fixt.h"
#include "zwave_transports_fixt.h"
#include "zcl_cluster_servers.h"
#include "zpc_dotdot_mqtt.h"
#include "zpc_datastore_fixt.h"


// Z-Wave Command Classes
#include "zwave_command_classes_fixt.h"

static uic_fixt_setup_step_t uic_fixt_setup_steps_list[] = {
  /** Initialize zpc_config. NB: This shall be first in the setup fixtures list */
  {&zpc_config_fixt_setup, "ZPC Config"},
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

  /**
   * Initialize the attribute store library.
   * Z-Wave RX, TX and the datastore shall be initialized first.
   */
  {&attribute_store_init, "Attribute store"},

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
   * Initialize the ZPC attribute resolver, and the UIC part of it
   * Keep that right before the network monitor,
   * in order to get proper paused/enabled resolution right after init.
   */
  {&zpc_attribute_resolver_init, "ZPC attribute resolver"},

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
  {&zpc_dodot_mqtt_init, "ZPC DotDot MQTT Topics"},
  /**
   * Initialize OnOff cluster mapper.
   * The cluster_serializer and attribute store MUST be initialized first.
   */
  {&dotdot_mapper_init, "DotDot mapper"},

  /**
   * Initialize DotDot MQTT hanlder that serializer and deserializer
   * dotdot messages.
   * All components registering callbacks to MQTT MUST be initialized before
   * this component.
   */
  {&uic_mqtt_dotdot_init, "DotDot MQTT"},

  /**
   * Initialize ZCL Cluster servers (simulated clusters
   * that the ZPC serves on behalf of the nodes)
   */
  {&zcl_cluster_servers_init, "ZCL Cluster servers"},

  /** Initializes all Z-Wave transport (Encapsulation CCs)*/
  {&zwave_transports_init, "Z-Wave Transports"},

  /** Initializes all supported / controlled CCs */
  {&zwave_command_classes_init, "Z-Wave Command Classes"},

  /** Initialize the Z-Wave Command handler framework after the Command Classes */
  {&zwave_command_handler_init, "Z-Wave Command Handler framework"},

  {&zwave_smartstart_management_setup_fixt, "Z-Wave SmartStart Management"},
  /**
   * UCL MQTT initialize all ucl mqtt handlers.
   */
  {&ucl_mqtt_setup_fixt, "UCL MQTT"},
  {&zpc_stdin_setup_fixt, "ZPC Stdin"},
  {&attribute_mapper_init, "Attribute Mapper"},

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
static uic_fixt_shutdown_step_t uic_fixt_shutdown_steps_list[] = {
  /** Demo dummy */
  {&zcl_cluster_servers_teardown, "ZCL Cluster servers"},
  {&zpc_attribute_resolver_teardown, "ZPC attribute resolver"},
  {&zwave_command_handler_teardown, "Z-Wave Command Handler framework"},
  {&zwave_network_management_fixt_teardown, "Z-Wave Network Management"},
  {&zwave_rx_fixt_teardown, "Z-Wave RX"},
  {&attribute_transitions_teardown, "Attribute transitions"},
  {&attribute_timeouts_teardown, "Attribute timeouts"},
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
