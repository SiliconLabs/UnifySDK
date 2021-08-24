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

#ifndef NETWORK_MONITOR_H
#define NETWORK_MONITOR_H

/**
 * @defgroup network_monitor Network Monitor
 * @ingroup components
 * @brief The Network State Monitor
 *
 * The Network Monitor acts as an interface between the \ref attribute_store and
 * \ref zwave_network_management and the \ref zwave_controller.
 * It keeps track of the current state of the network. It reacts to events from
 * the zwave_controller component and add/remove nodes to/from the attribute
 * system. It will also sanitize the HomeID and NodeID list of the controller
 * with the HomeID provided and NodeID list in the attribute system. If a
 * mismatch occurs, the component will update the attribute system to reflect
 * the controller information.
 *
 * The network state monitor will also keep track of which nodes in the
 * network are unreachable.
 *
 * @startuml Network Monitor Sequence Diagram
    participant AttributeStore
    box "ZPC" #LightBlue
    participant NetworkMonitor
    participant NetworkManagement
    end box

    note over AttributeStore,NetworkManagement: Init
    NetworkMonitor        -> AttributeStore       : Create/Set the ZPC Node ID and Security keys nodes in AttributeStore
    note over AttributeStore,NetworkManagement: Node is Added
    NetworkManagement     -> NetworkMonitor       : on_node_id_assigned()
    NetworkMonitor        -> AttributeStore       : Create Endpoint node in AttributeStore
    NetworkMonitor        -> AttributeStore       : Create NetworkStatus node in AttributeStore
    NetworkManagement     -> NetworkMonitor       : on_node_added()
    NetworkMonitor        -> AttributeStore       : Create Security key node in AttributeStore
    NetworkMonitor        -> AttributeStore       : Create NIF under Endpoint nodes in AttributeStore
    note over AttributeStore,NetworkManagement: Node is Removed
    NetworkManagement     -> NetworkMonitor       : on_node_deleted()
    NetworkMonitor        -> AttributeStore       : Remove NodeID node in AttributeStore
    note over AttributeStore,NetworkManagement: New Network
    NetworkManagement     -> NetworkMonitor       : new_network()
    NetworkMonitor        -> AttributeStore       : Delete the oldHomeID node from AttributeStore
    NetworkMonitor        -> AttributeStore       : Create the newHomeID node in AttributeStore
    NetworkMonitor        -> AttributeStore       : Create nodes in the attribute store for all nodes currently in the network
  @enduml
 *
 * The network monitor will also keep track of the nodes being interviewed
 * by monitoring Z-Wave NIF creations in the attribute store.
 *
 *  @startuml Network Monitor Interview monitoring
    hide footbox
    title Z-Wave node interview detection and monitoring
    participant "Attribute Resolver" as AttributeResolver
    participant "Attribute Store" as AttributeStore
    box "ZPC" #LightBlue
    participant "Network Monitor" as NetworkMonitor
    end box

    activate NetworkMonitor
    NetworkMonitor -> NetworkMonitor: Init
    NetworkMonitor -> AttributeStore: Register callback for NIFs attributes\nnetwork_monitor_on_nif_updated()
    deactivate NetworkMonitor
    ...
    AttributeStore        -> NetworkMonitor       : network_monitor_on_nif_updated()
    activate NetworkMonitor
    NetworkMonitor        -> AttributeStore       : Update the ATTRIBUTE_NETWORK_STATUS to \nNODE_STATE_TOPIC_INTERVIEWING
    NetworkMonitor        -> AttributeResolver    : Register listener for the NodeID\nnetwork_monitor_node_id_resolution_listener()
    deactivate NetworkMonitor
    ...
    note over AttributeResolver: Node is resolved
    AttributeResolver     -> NetworkMonitor       : network_monitor_node_id_resolution_listener()
    NetworkMonitor        -> AttributeStore       : Update the ATTRIBUTE_NETWORK_STATUS to \nNODE_STATE_TOPIC_STATE_INCLUDED
  @enduml
 *
 * @{
 */

#include "process.h"
#include "sl_enum.h"
/**
 * @brief Name the of @ref contiki process for the Network Monitor.
 *
 * This is used to register the name of the Network Monitor Process.
 */
PROCESS_NAME(network_monitor_process);

#ifdef __cplusplus
extern "C" {
#endif

void network_state_monitor_init();

#ifdef __cplusplus
}
#endif
/** @} end network_monitor */

#endif  //NETWORK_MONITOR_H
