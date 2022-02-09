# Z-Wave Protocol Controller Design

This document contains the design details of the implementation of ZPC. 
The ZPC major functionalities of the ZPC are the following:

- Perform Z-Wave network provisioning task using the MQTT data model decried by
  the Unify Specification
- Perform translation of between the Z-Wave command class data model and the
  Unify data model.
- Be a best in class Certifiable Z-Wave Controller

## How translation works

The major role of the Z-Wave protocol controller is to translate the 
Z-Wave command class data model in to the Unify data model. This task has
been solved in much the same way that a compiler translate a computer program
into machine instructions, by first parse the programming language into an 
Internal Language Representation and then translate the into machine code 
operations. 

In the ZPC translation is done by first mapping all data in Z-Wave commands to
attributes similar to the ZCL attributes. As an example the commands class
binary switch has
3 commands:
  - BINARY_SWITCH_GET
  - BINARY_SWITCH_REPORT
  - BINARY_SWITCH_SET

The 3 commands above all deal with one state variable(attribute), ie the binary
switch value. The binary switch value is very similar to a ZCL attribute
realising this we might as well deal with the binary switch value attribute
instead on focusing on the individual commands.

With the attribute representation of the Z-Wave data model in hand its trivial 
to translate the Z-Wave attribute to ZCL attributes.

When interpreting ZCL commands, the command execution normally translates into
an attribute changing. For example the OnOff cluster toggle command should have
the effect that the OnOff value attribute to switch to its opposite value. In
Z-Wave there is no such thing as an toggle command, for this reason the ZPC
needs to _remember_ the last know state of the device in order to calculate the
new update state. For this reason the ZPC caches the reported attribute values
for all nodes.

## Framework
The ZPC operates in the generic Unify framework. The unify framework is base on 
the event system from the Contiki OS. It should be understood the Contiki OS
is solely used as an event system which also has support for timers. The
ZPC application is build up around a single look with has a linux _select_ call.
The timers of contiki are triggered from the timeout value of the select call.
Various components( using serial or ethernet communication ) can get their file
descriptors registered with the select allowing the processes of the components to
be polled on activity on the file descriptors. Because of this system the
whole ZPC runs in a single thread, even though the processes in the Contiki 
system are called "proto threads" they are not real threads. Contiki processes
cannot be preempted. See [uic_main] for details. 

There is a single exception to the single thread, that is uic_stdin. This component
uses a thread for handling keyboard input, this is because there are issues with
using libedit with an external select call.

# Overview
The overall block of the Z-Wave Protocol Controller is shown in figure 1

@startuml
title ZPC Block diagram
[Mqtt Cluster Handlers] as mqtt_handlers
[Attribute Store] as ats
[Z-Wave Network Monitor] as app
[Z-Wave Command Handlers] as handlers
[Attribute Resolver] as resolver
[Z-Wave Controller] as ctrl
[Z-Wave Interface] as iface
[Cluster Servers] as zcls
[Mapping Engine] as mapper

mqtt -down- mqtt_handlers
mqtt -down- zcls
mqtt_handlers -down- ats
zcls -down- ats

mapper -down- ats 

ats -down- app
app -down- ctrl
ats -down- resolver
ats -down- handlers
handlers -down- ctrl
resolver -down- ctrl

ctrl -down- iface 
iface -down- serial
@enduml

## Z-Wave Interface
Starting from the bottom we have the _Z-Wave interface_ which which has the low
level access to the z-wave serial api, the serial api is the NCP interface for
Z-Wave over a RS232 link.

This component implements the serial protocol interface towards the z-wave
controller module. Asynchronous messages from the Z-Wave Controller are
propagated upwards though callbacks. The zwave_api component is the only
component in the zpc which may block the whole zpc for short periods of time.
This will happen I the z-wave controller module does not acknowledge a serial
frame. Under normal operation calls to serial module is completed in <1ms.


## Z-Wave controller 
The zwave_controller super is a layer on top of the  Z-Wave Interface. It
implements low level Z-Wave functionality which could be argued should have been
implemented in the Z-Wave controller firmware. It performs transport
encapsulations and has a FSM for network operations. All in all it simplifies
the more low level zwave_api.

The Z-Wave controller block is designed such that it needs not to persistent
data, when the controller need persisted data it will call back to the Network
Monitor to retrieve the data.

The Z-Wave Controller is made of a number of sub components:
- [zwave_network_management] which is a state machine which is performs
  various mandatory inclusion steps, which as interfacing with the 
  two security S0 and S2 inclusion FSM's.
- [zwave_tx] which is a queue where each element has a priority. It interfaces
  with the zwave_transports module and keeps track on how many responses has been
  received to each element.
- [zwave_transports] A _plugin_ system where frame encapsulation schemes can
  be implemented.
- [zwave_rx] Responsible for updating zwave_network_management with various events,
  And feeding received frames in to the zwave_transports as well as notifying zwave_tx
  when new frames has been received such that is can keep track of the number of 
  responses there has been to each frame.
- [zwave_tx_groups] Is able to translate a node list into a unique number which is used when 
  sending multicast messages.
## Z-Wave Network Monitor

The network monitor is the component which persists data from the Z-Wave
controller using the attribute store. Is responsible for starting a node
interview process using rules registered with the attribute resolver. The
network monitor also provides information about nodes to the Z-Wave controller,
ie. which security keys has been granted to a node.

## Z-Wave Command Handlers

The Z-Wave command handlers are Z-Wave command parsers and generators. In
general there are two types of command handlers:
- Control handlers which are used to collect information about nodes in the
  network as well as performing node actuation. The control handlers uses the
  attribute store to persist retrieved information and to retrieve information
  needed to compose Z-Wave frames
- Support handlers, which are _services_ provided for other Z-Wave nodes in the
  network. The in general the support handlers are _not_ interfacing with the
  attribute store, but provides static data.

All command handlers are registered with the [zwave_command_handler] component
which is responsible for dispatching the right command handler when a new
application command is received from the [zwave_controller] component.

## Attribute Resolver

The Attribute resolver holds a set of rules on how to get and set attributes by
sending Z-Wave commands. The rules are all registered with the attribute
resolver early in the boot process and are not modified while the ZPC is
running. The _Network Monitor_ and the _Command Handlers_ are registering rules
into the attribute resolver. This block is the driver for doing network
interview and performing actuation on the Z-Wave network. Details of how
the resolver works can be found in [uic_attribute_resolver]

## Attribute Store

This block is responsible for persisting data in a database. All data is stored
in a tree data model which where each node in the tree has three values, a
type-, desired- and a reported value. The attribute store has callback system
which allows other components to get a callback triggered when a node of a given
type changes. A node the attribute store can in general be expressed in but the
tuple _{ parent_node, type, desired, reported}_. 

## Mapping Engine

The Mapping Engine solely interfaces the Attribute Store. The role of the
mapping engine is to perform the bidirectional _translation_ between Z-Wave and
the ZCL attributes and commands. The mapping engine is configured using a
scripting language called Unify Attribute Map (UAM). In the UAM definitions
relations between Z-Wave and ZCL attribute are defined. As the mapping engine
uses the callback system of tha Attribute Store the engine is able to react on
attribute changes according to the programmed relations.

## MQTT Cluster Handlers

This block is in charge of reacting on ZCL attribute changes and publish updated
information as JSON documents on the MQTT interface. The cluster handlers use
the attribute store callback system to monitor the ZCL attributes. The MQTT
cluster handlers are auto generated from the XML data model of the ZCL
specification. 

The MQTT cluster handlers also parses ZCL commands invoked by other clients on
the MQTT interface and translate those commands into attribute store writes.

# Cluster Servers
The Unify data model is mostly a copy of the ZigBee Cluster Library data model.
But there are exceptions, where functionality has been added ZCL data model such
that the model can cover provisioning, multicast frame transmission and protocol
agnostic Over The Air firmware update. Support for the _Unify_ only functionality
has been added into this block.


@startuml
title ZPC Component diagram

package "MQTT" {
[ucl_mqtt]
[zcl_cluster_servers]
[dotdot_mqtt_topics_handler]
[ucl_definitions]
}

package "System" {
[zpc_stdin]
[zpc_config]
}

package "Attribute System" {
[dotdot_mapper]
[zpc_attribute_store]
[zpc_attribute_store_attr_type_name_helper]]
[zpc_attribute_resolver]
}

package "Z-Wave Application Layer" {
[zwave_command_handler]
[zwave_command_classes]
[zwave_definitions]
}

package "Unify Z-Wave controller" {
[network_monitor]
[zwave_smartstart_management]
[zpc_utils]
[zwave_tx_scheme_selector]

}

package "Z-Wave controller" {
 [zwave_controller]
 [s2]
 [s0]
 [zwave_tx]
 [zwave_rx]
 [zwave_transports]
 [zwave_network_management]
 [zwave_tx_groups]
}

package "Z-Wave Interface" {
 [zwave_api]
}

[zwave_api] -> [zwave_controller]
[zwave_controller] -up-> [network_monitor]
[network_monitor] -up-> [zpc_attribute_store]
[zpc_attribute_store] -up-> [dotdot_mqtt_topics_handler]
[zwave_controller] -up-> [zwave_command_handler]
[zwave_command_handler] -up-> [zpc_attribute_store]
@enduml