# Unify Library Overview

This page describes the Unify Framework library. The purpose of the Unify Framework
library is to share code among the Unify components and applications developed
by Silicon Labs and to provide a platform on which Silicon Labs customers can build
their own components and applications.
Besides the obvious advantage of sharing code in terms of maintainability and
rapid development, the library will also ensure a common look and feel of the
various Unify components in terms of operation and system integration.

The Unify Framework Library provides the following overall features

- Base platform which provides event loop, config system and logging system.
- Unit test framework
- Build system with docker containers
- Documentation framework
- Packaging system for distributing components and basic scripts for system
  integration with Debian Bullseye.
- Coding standard

The figure below show the meta functionality that the Unify Frameworks provides.
Other Guides related to develop

```{uml}
title Unify Framework packages

Package "Base Platform"
Package "Build System"
Package "Test System"
Package "Documentation framework"
Package "Packaging framework"
Package "Coding Standard"
```

## Base Platform

The Base Platform provides a number of components which can be shared among Unify
applications. Applications are totally free to use a subset of these components.
but Unify applications should avoid duplicating the functionality of these Unify
components.

The figure below illustrates the inter dependencies between various components
in the Unify Framework Library. Each component (blue block in the figure) has a public
interface which is exported by the library.

```{uml}
title Unify Platform Components

package "Management helpers" as management_helpers {
  component "SmartStart" as smartstart_management_helper
  component "Node State Topic" as node_state_topic_management_helper
  component "Protocol Controller" as protocol_controller_management_helper
  component "OTA Client" as ota_management_helper
}

package "Attribute System" as attribute_system {
  component "Attribute Store" as attribute_store
  component "Attribute Resolver" as attribute_resolver
  component "Attribute Mapper" as attribute_mapper
}

package "DotDot components" as dotdot_components {
  component "DotDot Definitions" as dotdot_definitions
  component "DotDot MQTT" as dotdot_mqtt
  component "DotDot Attributes" as dotdot_attributes
}

component "Config System" as config_system
component "Logging System" as log_system
component "MQTT Client" as mqtt_client
component "Contiki" as contiki
component "Main Loop" as main_loop
component "Datastore" as datastore
component "Console Interface"  as console_interface

main_loop -[hidden]-> log_system
log_system -[hidden]-> config_system

console_interface -right-> main_loop
main_loop -> contiki
mqtt_client -> main_loop
dotdot_mqtt -left-> mqtt_client
management_helpers -> mqtt_client
dotdot_attributes -up-> dotdot_mqtt
dotdot_attributes -down-> attribute_store
attribute_store -right-> datastore
attribute_resolver -u-> attribute_store
attribute_mapper -u-> attribute_store

attribute_system -left-> contiki
mqtt_client -> contiki
```

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
found in the [Unify Developer guide](readme_developer.md).

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

### DotDot Components

The Dotdot Components are helper components that provide definitions and ease
of use of the UCL/DotDot data model.

#### DotDot Definitions

The DotDot definitions (*uic_dotdot* component) comprises DotDot XML files as
well as a few C/C++ headers with definitions.

This represents the data model used between all entities in the Unify
Framework. Note that the definitions in these XML files take precedence
over examples from the
[Unify Framework Specifications](./unify_specifications/index.rst).

These XML files represent ZCL/Dotdot clusters and can be split into 2 ranges:

- [0x0000..0xFBFF]: Official ZCL Clusters.
- [0xFC00..0xFFFF]: Proprietary clusters.

The Unify Framework added a few propritary cluster definitions. The Unify Framework uses
the [0xFC00..0xFEFF] range to add its own clusters.

To keep compability with subsequent Unify versions, XML files provided as
part of the Unify Library must not be modified.

Unify Framework users can add additional proprietary clusters
(and therefore proprietary attributes/commands) to the Unify Framework,
by adding their own XML files.

The range [0xFF00..0xFFFF] may be used to create new XML files. This will not be
used in subsequent release. To create a new XML file, add it in the
*uic_dotdot* component *dotdot-xml* folder. Assign a new unique Cluster ID from
the reserved range, for example using the 0xFF01 value:

```xml
<zcl:cluster
  xmlns:zcl="http://zigbee.org/zcl/clusters"
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xmlns:type="http://zigbee.org/zcl/types"
  xmlns:xi="http://www.w3.org/2001/XInclude"
  xsi:schemaLocation="http://zigbee.org/zcl/clusters cluster.xsd http://zigbee.org/zcl/types type.xsd"
  id="FF01" revision="1" name="ExampleApplicationCluster">
  <classification role="application" picsCode="EAC" />
```

Finally, add it to the list in the `library.xml` file.

```xml
  <xi:include href="ExampleApplicationCluster.xml" parse="xml"/>
```

Building the Unify library will use the new XML definitions and provide
automatically a comprehensive set of APIs enabling the use of the new cluster
immediately by components and applications using the Unify library or
derived DotDot Components.

#### DotDot Attributes

The DotDot attribute are helper functions that will register the properties of
all existing UCL / DotDot attribute to the Attribute Store.

#### DotDot MQTT

This components will automatically publish MQTT messages based on changes in the
Attribute Store DotDot attributes and handles write MQTT messages and update
the Attribute Store accordingly.

[DotDot Attributes API](api.md)

### Datastore

The Datastore is a generic data store that stores key-value pairs in an SQLite
database. The Datastore is also used by the Attribute System to store data in a
tree structure.

[Datastore API documentation](@ref datastore)

### Attribute Store

The Attribute Store provides an interface for reading and writing attributes
of any data type in a tree structure, representing the state of the network.

[Attribute Store API documentation](@ref attribute_store)

### Attribute Mapper

Attribute Mapper is a component that offers a text file based system
that allows to define Attribute relations, allowing automatic rules and
attribute manipulations using assignments based on text script files.
The attribute mapper is able to map attributes in the Attribute System to each
other. The mapper is using UAM grammar to perform the mapping. Complex
mappings are supported, where one attribute might depend on multiple other
attributes. Detailed information about Mapper component is documented in
[Attribute Mapper Documentation](attribute_mapper_overview.md)

### Attribute Resolver

The Attribute Resolver is a frame generator that uses functions registered
for attribute types. It will arbitrate when to create and schedule frames
for a given node interacting with the Attribute Store.

### Console Interface

The Console Interface serves as an optional debugging interface to the Unify
components. It features help menus and auto completion.

[Console Interface API documentation ](@ref uic_stdin)

### Attribute Poll

The Attribute Poll component is a component that help undefine the values
of attributes periodically, so that they are requested again to PAN nodes and
the network information is kept as accurate as possible.

### SmartStart

The SmartStart component is a common component that subscribes to the
SmartStart UCL topic and can be used by a Protocol Controller to check if a
given node is in the SmartStart list. The SmartStart component is also
responsible for publishing changes to the SmartStart list on node add and
remove events.

### Node State Topic

The role of the Network Monitor is to keep track of the network state and in
particular the node states.

The Network Monitor react to node add / remove events from the a Protocol Controller
component and insert nodes in the attributes system. Periodically the network
monitor will check if nodes are responsive and update their states accordingly.
At boot the Network Monitor will check if the node list reported by the
controller matches the data in the Attribute System. If a mismatch is found the
the Attribute Store is updated by deleting or inserting nodes. The attribute
resolver will make sure to resolve newly added nodes.

### OTA MQTT Client

This component handles OTA related MQTT topics that are used to announces OTA
images availability and OTA status.

## Test Framework

The Test Framework consists of the *Throw the Switch Unity* and
*Throw the Switch CMOCK* frameworks for making unit tests and mocks.
The two libraries has been integrated into the cmake build system and provides
automatic mock generation for C header files.

See [Developer documentation.](readme_developer.md)

## Build System

The Unify Framework Library provides a Build System based on the CMAKE build system.
Various cmake macros has been defined for making mocks, unit tests and packages.

See [Developer documentation.](readme_developer.md)

## Packaging Framework

The Packaging Framework supports Debian and cmake scripts which facilitates
packaging of Unify components.

## Coding Standard

All Unify component should adhere to a common coding standard.

See the [Unify Coding Standard](standards/coding-standard.md)


