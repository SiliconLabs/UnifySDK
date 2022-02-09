# Unify Library Overview

This page describes the Unify SDK library. The purpose of the Unify SDK
library is to share code among the Unify components developed by Silicon Labs and
to provide a platform on which Slabs customers can build their own components.
Besides the obvious advantage of sharing code in terms of maintainability and
rapid development, the library will also ensure a common look and feel of the
various Unify components in terms of operation and system integration.

The Unify SDK Library provides the following overall features

- Base platform which provides event loop, config system and logging system.
- Unit test framework
- Build system with docker containers
- Documentation framework
- Packaging system for distributing components and basic scripts for system
  integration with debian buster. _not quite done_
- Coding standard


The figure below show the meta functionality that the Unify Frameworks provides.

@startuml unify_framework

title Unify Framework packages

Package "Base Platform"
Package "Build System"
Package "Test System"
Package "Documentation framework"
Package "Packaging framework"
Package "Coding Standard"
@enduml

## Base Platform

The Base Platform provides a number of components which can be shared among Unify
applications. Applications are totally free to use a subset of these components.
but Unify application should avoid duplicating the functionality of these Unify
components.

The figure below illustrates the inter dependencies between various components
in the the Unify SDK Library. Each component (blue block in the figure) has a public
interface which is exported by the library.


@startuml platform_components

title Unify Platform Components

component "Main Loop" as main_loop
component "Config System" as config_system
component "Logging System" as log_system
component "MQTT Client" as mqtt_client
component "DotDot Serializer" as dotdot_serializer
component "DotDot Attributes" as dotdot_attributes
component "Datastore" as datastore
component "Attribute Store" as attribute_store
component "Attribute Mapper" as attribute_mapper
component "Console Interface"  as console_interface
component "SmartStart"
component "Node State Topic"
component "Protocol Controller MQTT"
component "OTA MQTT Client"

main_loop <- console_interface
mqtt_client -> main_loop
dotdot_serializer -u-> mqtt_client
dotdot_attributes -u-> dotdot_serializer
dotdot_attributes -> attribute_store
attribute_store -> datastore
attribute_mapper -u-> attribute_store
@enduml



### Main loop

The main loop component implements as the name hints a main loop which can be
used as the event system of a Unify application. The main loop components does not
implement the C main function itself, but has a uic_main(...) function which has
the main loop. The main loop uses the event system from the Contiki operating
system and implements a select loop that reacts on events on a set of file
descriptors. The main loop also implements a framework to initialize and
shutdown other components as well as a posix signal handler to shut down the
application in a controlled manner.

The main loop is hardcoded to use the config system component to parse
config options and command line arguments and the logging system printing
log messages.

Details on how to build a new application using the main loop component can be
found [here](readme_developer.md).

[Main loop API documentation](@ref uic_main)

### Config System

The config system implements parsing of config files and command line arguments.
All Unify applications should use the config system to present a uniform look and
feel across all applications.

[Config system API documentation](@ref unify_config)

### Logging System

The logging system features logging with severity level tagging and log
filtering. All Unify applications should use the logging system to present a
uniform look and feel across all applications.

[Logging System API documentation](@ref sl_log)

### MQTT Client

The MQTT client is build on top a Eclipse Mosquitto MQTT and handles MQTT
connection, subscribing and publishing. It implements re-connections and
caching of publish messages which has been published which the client has
been disconnected from the broker.

[MQTT Client API documentation](@ref uic_mqtt)

### DotDot Serializer

The role of the DotDot Serializer is to compose and parse Unify json documents which
are sent and received over the MQTT interface.

[DotDot Serializer API documentation] (@ref dotdot_mqtt)

### DotDot Attributes

This components will automatically publish MQTT messages based on changes in the
Attribute Store and handles write MQTT messages and update
the Attribute Store accordingly.

[DotDot Attributes API]( )
### Datastore

The Datastore is a generic data store that stores key-value pairs in an SQLite
database. The Datastore is also use by the Attribute System to store data in a
tree structure.

[Datastore API documentation](@ref datastore)

### Attribute Store

The Attribute Store provides an interface for reading and writing key value
pairs in a tree structure.

[Attribute Store API documentation](@ref attribute_store)

### Attribute Mapper

The attribute mapper is able to map attributes in the Attribute System to each
other. The mapper is using a scripting langrage to perform the mapping. Complex
mappings are supported, where one attribute might depend on multiple other
attributes.

### Console Interface

The Console Interface serves as an optional debugging interface to the Unify
components. It features help menus and auto completion.

[Console Interface API documentation ](@ref uic_stdin)

### SmartStart

The SmartStart component is a common component that subscribes to the
SmartStart UCL topic and can be used by a Protocol Controller to check if a
given node is in the SmartStart list. The SmartStart component is also
responsible for publishing changes to the SmartStart list on node add and
remove events.

### Node State Topic

_We need to investigate how much of this code can be shared._

The role of the Network Monitor is to keep track of the network state and in
particular the node states.

The Network Monitor react to node add / remove events from the a Protocol Controller
component and insert nodes in the attributes system. Periodically the network
monitor will check if nodes are responsive and update their states accordingly.
At boot the Network Monitor will check if the node list reported by the
controller matches the data in the Attribute System. If a mismatch is found the
the Attribute Store is updated by deleting or inserting nodes. The attribute
resolver will make sure to resolve newly added nodes.

### Protocol Controller MQTT

The Protocol Controller MQTT component manages the following MQTT topics:

- ucl/by-unid/+/ProtocolController/NetworkManagement/#

It provides an interface for protocol controllers to publish their state and for
registering callbacks when state changes are requested from the MQTT side.

### OTA MQTT Client

This component handles OTA related MQTT topics that are used to announces OTA
images availability and OTA status.

## Test Framework

The Test Framework consists of the the _Throw the Switch Unity_ and  _Throw the
Switch CMOCK_ frameworks for making unit tests and mocks. The two libraries has
been integrated into the cmake build system and provides automatic mock
generation for C header files.

See [Developer documentation.](readme_developer.md)

## Build System

The Unify SDK Library provides a Build System based on the CMAKE build system.
Various cmake macros has been defined for making mocks, unit tests and packages.

See [Developer documentation.](readme_developer.md)

## Packaging Framework

The Packaging Framework supports debian and cmake scripts which facilitates
packaging of Unify components.

## Coding Standard
All Unify component should adhere to a common coding standard.

See the [Unify Coding Standard](standards/coding-standard.md)