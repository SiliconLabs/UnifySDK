# Unify Architecture Overview

## Terminology And Abbreviations

Term                       | Abbreviation   | Description
------------- | ------------- | -------------
Unify SDK | Unify | The system whose design is described here. Formerly known as Unified IoT Controller (UIC).
Software Development Kit | SDK | A software package used as a building block for other software. Unify is itself an SDK.
Unified Node ID | UNID | A unified addressing system with a separate sub-namespace for each PHY supported by the Unify SDK. It is arbitrarily chosen to resemble an IPv6 address in the current design.
Unify Controller Language | UCL | The unified command language used as an internal abstraction layer in the Unify SDK.
Cluster | CL | A grouping of related commands/attributes supported by an end-node. Examples include Door Lock, Multilevel Switch, Movement Sensor, and so on. For example, Bluetooth Profiles and Z-Wave Command Classes can be translated into CCs. CCs are encoded as MQTT topic strings, e.g., "door_lock", "movement_sensor", and so on.
Command | CMD | An attribute or command that can either be sent to the endnode or received from it. Grouped into Clusters. Zigbee Commands and Attributes, Z-Wave Commands, and Bluetooth Profiles can all be translated or broken down into CMDs. CMDs are encoded as MQTT topic strings, e.g., "doorlock_operation_set" and "doorlock_operation_get" and "doorlock_operation_report".
Endpoint | EndPointID | Endpoints are sub-addresses in unsigned integers of a UNID. Devices without PAN-side support for endpoints use ep0.
MQTT Broker | - | The "server" side of MQTT, which distributes published messages to a topic to all subscribers in the topic.
Personal Area Network | PAN | The network of end nodes behind a PHY Radio controlled by Unify.
IoT Service |  - | An MQTT client that translates the UCL language into the API of a cloud service or locally running high-level service (e.g., Web UI). This is a translator to a cloud service, not the actual cloud service. An AWS-IoT IoT Service runs on Unify hardware and connects to AWS IoT cloud, for example.
Protocol Controller | - | An MQTT client that controls a PHY Radio and translates commands between the UCL and the native language of that PHY. Also known as a PHY Translator.
Gateway | GW | An IP host connecting two different domains. A Unify Gateway consists of the Unify components and SDK installed on an IP host providing OS functionality and hardware support.

 *The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC2119](https://tools.ietf.org/html/rfc2119).*

## Introduction

The Unify SDK delivers a unified high-level API, such as cloud control, local HomeAssistant middle-wares, and so on to control, observe and manage end nodes in all PHYs supported by Silicon Labs. It also facilitates code reuse and allows easily adding new high-level APIs without the need to modify the PHY drivers. After a new high-level API module has been created, it should be possible to control all the supported PHYs without any changes to the PHY drivers. Likewise, after a new PHY driver has been created, it should immediately be possible to control that PHY from all the existing high-level APIs without modifying the high-level API module.

In other words, the Unify SDK decouples API modules from the PHY drivers.

The Unify SDK is a software component that runs inside a gateway product. It needs a hardware platform and an operating system to be a complete product.

\image html sl_gw_asset.png width=800px
\image latex sl_gw_asset.png

From a business perspective, Unify makes it possible to maintain a single IoT Service that can interface with multiple wireless PHY radios. The functionality includes SmartStart, best-in-class security, firmware updates, and so on. Unify makes it easier to develop gateway products by driving down the marginal effort needed in launching the next customer gateway and enables rapid development cycles.

The remainder of this document refers to PHY drivers as Protocol Controllers because they not only drive the modem but also translate the Application Layer commands from one PHY technology to another. Similarly, the high-level API modules are called IoT Services, which are named after the expected use case, but they can also connect to locally running services.

## System Components

The Unify SDK is a software component of a full plastic-enclosed GW box. You can think of it as a "device driver" for the IoT, seen from the Host MCU in the GW box. The Unify component uses MQTT as an internal message-bus for inter-process communication among MQTT clients that are each an independent process/thread. In a Linux or other full OS implementation, real processes should be used. The primary hardware platform of the Unify design is a Cortex A-class device or an equivalent.

The MQTT clients are one of these types:

* IoT Services (or IoT services): These modules expose the externally-visible APIs in Unify. IoT Services translate back and forth between external APIs and the intermediate UCL. The external API is typically consumed by a cloud-service or a local middleware/app. It is slightly confusing that an IoT Service can also provide APIs to a locally-running service. It is named after the expected use case, which is a cloud-based remote control.  Examples of IoT Services include:
  * AWS IoT
  * HomeAssistant
  * Azure IoT
  * Rules Engines (even if running locally)
  * Web UI for local control
* Protocol Controllers: These MQTT clients own a physical radio operated through Unify. They also translate from UCL to the native language of the radio. Examples include the following:
  * Zigbee Protocol Controller owning an NCP
  * Z-Wave Protocol Controller owning a SerialAPI
  * BLE Protocol Controller
  * BLE-Mesh Protocol Controller
  * Silicon Labs Connect Stack Protocol Controller
* A singleton Resource Directory MQTT client, responsible for aggregating node information and providing the ucl/<#>/discover topic for discovering nodes, where <#> can be one of (by-unid, by-location, by-roletype, by-devicetype, by-group).

## System Diagram

This diagram shows the overall architecture and some of the technologies that Unify connects.

\image html slide8_block_diagram.png width=800px
\image latex slide8_block_diagram.png

Because a central MQTT broker and standardized MQTT topics and JSON
payload (the UCL) are used, the architecture is more like a star topology, as shown below:

\image html uic-system-diagram.png width=800px
\image latex uic-system-diagram.png width=400px

The figure above shows the overall Unify architecture with MQTT as an integral
part. IoT Services (blue boxes) for AWS IoT, Azure, HomeAssistant and so on are
shown. Protocol Controllers (green boxes) for all Silicon Labs PHYs are
possible, with a random selection of Z-Wave, Zigbee, and Bluetooth shown. All
IoT Services and Protocol Controllers are connected by a common Unify
abstraction layer consisting of an MQTT broker transporting the Unify Controller
Language. Clients can roughly be grouped into IoT Services, local middlewares,
Protocol Controllers, and a Singleton Resource Directory.

The Resource Directory is also an MQTT client.

The MQTT server should run as a separate Linux process.

The Resource Directory facilitates device discovery and communicates with each Protocol Controller through specific MQTT topics. The Protocol Controller MUST \reqnumber{UNS:000001.1}  automatically notify the Resource Directory of nodes that join or leave the network, as well as multicast groups that are added/removed/modified.

The ACL Engine validates all publish and subscribe requests before being processed by the Broker. The permissions of each MQTT Client is established by looking up a permission token in an Access Control List (ACL). The key to the ACL is a security token contained in the TLS certificate presented by the MQTT client. Local clients SHOULD only be allowed to connect TLS, but they may use raw TCP if absolutely necessary (note that this will prevent the ACL Engine from identifying the client). Remote clients MUST only connect with TLS. If non-TLS connections are accepted against this RECOMMENDATION, another simple (and non-secure) permission token exchange must be added.

### Design Variants

Unify gateways comes in a few variants that differ in the number of PHYs controlled:

* A multi-homed Unify gateway has several Protocol Controllers installed. This is a more complicated product to develop and market.

* A single-homed Unify gateway has a single Protocol Controller and relies on MQTT
  bridging and/or Cloud Services to connect to other PHYs. To unify two PHYs, you need
  two physical Unify-based gateways, which must be connected to the same
  IP network or cloud service. The two single-Protocol Controller Unify GWs
  communicate via MQTT bridging and UCL or via a Cloud Service they are both
  connected to. The single-homed Unify GW is a simpler product to develop, allows
  the developer to focus solely on the PHY they are most comfortable with, and
  still end up with a complete, functional, gateway product. The process of
  interconnecting multiple independent single-Protocol Controller Unify GWs can be
  thought of as the "LEGO-block design principle for Unify".

Another variant of the design uses MQTT bridging to allow multiple Unify
gateways to cooperatively control and manage the same PAN. The bridging may
happen either locally or in the cloud.

\image html slide20_lego_block_ug_design.png width=800px
\image latex slide20_lego_block_ug_design.png

This figure shows an example of 3 single-Protocol Controller Unify-based
gateways, that when connected to the same IP backbone can unify Zigbee, Z-Wave
and the AWS IoT cloud service. Each gateway is represented by a green, blue or
yellow rectangle, showing which connectors it has attached.

## Packet Format

When Unify MQTT clients attach to the locally-running MQTT broker, they
publish and receive messages in the following format (i.e., a tcpdump attached to
the sockets going to and from the MQTT Broker, would see the packets in the
following two figures).

\image html UniversalGWPacketFormatv2.0.png "PUBlish messages style" width=800px
\image latex UniversalGWPacketFormatv2.0.png "PUBlish messages style"

\image html UniversalGWPacketFormatSUBSCRIBE.png "SUBscribe messages style" width=800px
\image latex UniversalGWPacketFormatSUBSCRIBE.png "SUBscribe messages style"

If the MQTT broker is running remotely, the MQTT MUST be using strong transport
encryption to provide adequate security (e.g., MQTT over TLS or WebSocket).

## Unify Controller Language Definition

The Unify Controller Language (UCL) is developed to allow the easiest possible
translation to and from as many PHYs and cloud services as possible. The UCL
serves as an abstraction layer between the PHY-specific data models and the
models used in various cloud services for controlling IoT devices. The UCL data
model MUST be the Zigbee Cluster Library/dotdot (dotdot is ZCL transported over
CoAP, whereas UCL will be transported over MQTT). ZCL has been chosen to align
Unify with the high-profile
[Matter project](https://https://buildwithmatter.com/).

In addition to the existing Zigbee Clusters, UCL is extended with the necessary
commands to support all features of the supported PHYs.

## Unified Node IDs

The UNIDs are unique text strings in the MQTT topic hierarchy, which MUST
follow the MQTT naming limitations.

The Protocol Controller generates the UNID for all nodes in the PAN (or PANs) it
controls. The UNID MUST consist of a prefix part and a PAN part, separated by a
minus string character "-".

The PAN part MUST be generated by the Protocol Controller and MUST be unique
among PAN parts generated by that PC. The Protocol Controller MUST prefix all
UNIDs with the same prefix. This prefix must be configurable at startup. The
system integrator MUST ensure that no two Protocol Controllers have the same
prefix configured. The prefix ensures that separate Protocol Controllers never
generate the same UNID and hence keeps them unique.

The following table shows examples of generated UNIDs:

Protocol Controller     | Prefix | Example UNID
------------- | ------------- | -------------
Z-Wave | "zw" | "zw-DEADBEEF-01" for nodeID 01 in homeID 0xDEADBEEF. NodeID is mutable on network reset.
Zigbee | "zb" | "zg-DEADBEEFC0FFEE12" for node with EUI-64 DEADBEEFC0FFEE12.
BLE | "ble" |  "ble-ri-11:22:33:44:55:66" for a BLE device with random identifier 11:22:33:44:55:66.
Z-Wave | "bld42" | "bld42-DEADBEEF-01" for nodeID 01 in homeID 0xDEADBEEF in building 42.
Z-Wave | "bld99" | "bld99-abcdef0123456789abcdef0123456789" for node with DSK (aka. public key fingerprint) abcdef0123456789abcdef0123456789. DSKs are non-mutable across network resets.

For example, Z-Wave UNIDs could be prefixed "zw", zigbee with "zb", and so on.
The prefix should be short (max 8-10 characters) to keep MQTT topics short.

The UNID PAN parts can be derived from a unique identifier, such as the EUI-64 of a
Bluetooth LE or Zigbee device, or the unique DSK of an S2 Z-Wave device. For
devices without a permanent unique identifier, something like the currently
assigned address on the PAN can be used, for example a combination of HomeID and
nodeID for Z-Wave nodes without S2.

Mutable UNID PAN parts may be used instead of permanent identifiers if they
allow simpler diagnostics and troubleshooting of the system. This may be
the case when choosing Z-Wave HomeID/NodeIDs instead of S2 DSKs. The
disadvantage is that nodes will change their UNID when the network is reset or
leaving and re-joining.

## MQTT Topic Hierarchical Namespace

MQTT topics are hierarchical namespaces that any MQTT client can publish and subscribe to. Subscriptions can use wildcards where # match multiple hierarchy levels and + matches one. As a result, subscribing to ucl/by-unid/# gets all ACs for all PAN nodes. On the other hand, subscribing to ucl/by-unid/+/door_lock receives only data related to Door Lock functionality on any and all Door Locks in the network.

The topic tree looks like the following:

```json
ucl/
  by-unid/
    <UNID>                            UNID can be e.g. "xy-123456-05" or  "bld42-<EUI-64>".
      ep<EndPointID>                  The endpoint to control/monitor. ep0 is used for devices
                                      without PAN-side endpoints.
        <ClusterName>                 Everything relating to a specific Cluster, e.g. "ColorControl".
          Attributes                  Attribute namespace, where all attributes will be advertised.
            <AttributeName>           Attribute name, e.g "ColorCapabilities".
              Reported                Reported state for a given attribute.
              Desired                 Desired state for a given attribute.
            <AdditionalAttributes>    Non Standard ZCL attribute are advertised here.
                                      ZCL Tables may use more than 1 level. E.g. "SceneTable".
              ../Reported             Reported state for a given attribute. ".." represents several levels. E.g. "SceneTable/1"
              ../Desired              Desired state for a given attribute.
          SupportedCommands           Array of supported commands in the cluster.
          Commands                    Namespace for triggering commands used to control a device.
            <CommandName>             Particular command under a cluster. Example: "MoveToLevel".
      ProtocolController
        NetworkManagement             Network Management related topics, see "Network Management Commands" page.
          Write                       Publish here to initiate Network Management operations
        Diagnostics                   Diagnostic information from the protocol controller is published here.
          Request                     On-demand diagnostic information is requested by publishing to this topic.
      State                           Network state, security and command delay of the node (for all endpoints).
        SupportedCommands             List of (Network management) supported
                                      commands attached to a node state, that
                                      will affect this node only.
        Command                       Namespace for triggering commands used to
                                      manage a device.
          <CommandName>               Particular command for node management. Example: "Interview", "Remove".
                                      node state, that will affect this node only.
      NameAndLocation                 Topic for setting node name and location. Discover via ucl/by-location/ .

by-location                           Subscribe to ucl/by-location/+ to receive all locations.
  <LOCATION>                          Payload is the pretty-printed name of location. E.g.
                                      {"location-name-utf8" : "walk-in closet"} for topic ucl/by-location/
                                      walk-in_closet .
    <UNID>                            Each node in <LOCATION> publishes a retained message here using its
                                      UNID as topic here.
                                      Subscribe to ucl/by-location/<LOCATION>/+ to receive a message from
                                      each UNID in <LOCATION>.

by-type/
  by-devicetype/
    <DEVICETYPE>                      e.g. "door_lock", "light" or "sensor".
      <UNID>                          Each node of <DEVICETYPE> publishes a retained message here using
                                      its own UNID as sub-topic.
                                      Subscribe to ucl/by-devicetype/<DEVICETYPE>/+ to receive a message from
                                      each UNID in of that type.

by-homeassistant-type/
  <HOMEASSISTANT_TYPE>                e.g. "binary_sensor" or "camera"
    <UNID>                            Subscribe to ucl/by-type/by-homeassistant-type/+ to receive a message
                                      for each HomeAssistant type in the network
                                      Subscribe to ucl/by-type/by-homeassistant-type/<HOMEASSISTANT_TYPE>/+ to
                                      receive messages from each UNID of that type
by-group/                             For multicast groups
  <GroupID>                           A unique, numeric group ID
                                      e.g. 1, 2, ... 65535.
    GroupName                         Name assigned to the group.
    NodeList
      <UNID>                          Used to advertise the membership of a UNID
                                      and a list of its endpoints for a group
    <ClusterName>
      Commands/
        <CommandName>                 Publish here to multicast <CommandName> to
                                      all UNIDs/endpoints that are part of the
                                      group.
        WriteAttributes               Publish here to multicast attribute write
                                      to all UNIDs/endpoints that are part of
                                      the group.
      SupportedCommands               List of commands supported by all UNIDs/
                                      endpoints that are part of the group.

  SmartStart                          Topics for managing SmartStart node provisioning
    List                              The full list of SmartStart entries is published here
    Update                            Publish here to add/update an entry to the list
    Remove                            Publish here to remove an entry from the list
  _well_known                         Versioning and list of supported ucl/ sub-topics
```

The &lt;DEVICETYPE&gt; MUST adequately describe the Device Types (i.e., the practical functions) of all the device types found in all PANs supported by Silicon Labs. The [dotdot Device Library specification](https://zigbeealliance.org/solution/dotdot/) has a good list of device types.

The &lt;HOMEASSISTANT_TYPE&gt; MUST be a strict superset of the [MQTT discovery types defined in the HomeAssistant open-source project](https://www.home-assistant.io/docs/mqtt/discovery/).

The subscribers to the ucl/_well_known topic/+ MUST receive a
[JSON-encoded](https://www.w3schools.com/js/js_json_datatypes.asp) payload in a
[Retained Message](https://www.hivemq.com/blog/mqtt-essentials-part-8-retained-messages/)
with at least the following contents:

**The JSON-encoded well-known payload:**

```json
{
  "schema_version" : 1,
  "wellknown_topics" :
   [
     {"name" : "ucl/by-unid", "description" : "All nodes grouped by UNID"},
     {"name" : "ucl/by-location", "description" : "All nodes grouped by location"},
     {"name" : "ucl/SmartStart", "description" : "SmartStart management"},
     ...
     {"name" : "ucl/by-group/", "description" : "Multicast transmission and group management"}
   ]
}
```

Each protocol controller MUST publish the well known topics they provide under
ucl/_well_known/&lt;UNID&gt;

## Payload Encoding

Everything after the Cluster, Command, Attribute and Table field in a given
dotdot ZCL command must be encoded in the payload of the MQTT message. The UCL
header must also be JSON-encoded in the format given in the example below.

Field names are taken from the Zigbee Cluster Library. Camel-case is used to
encode spaces, forward slashes are omitted (On/Off → OnOff).

## Payload Versioning

Future extensions of the ZCL or Unify framework MAY add new information in the
existing fields of a JSON payload.

IoT services and Protocol Controllers MUST be resistant to unknown/new
keys or key values when parsing JSON objects.

As an example, the following payload:

```json
{
  "value" : 1
}
```

May be extended in a future version to something alike:

```json
{
  "value" : 1,
  "value_options": [1,2,3]
}
```

## Protocol Controller and IoT Service MQTT Clients

The MQTT clients connecting to the MQTT broker of a Unify gateway can very
broadly be divided into two categories: Protocol Controllers and IoT Services.
These were also called PHY Translator/Connectors and Cloud
Connectors/Translators previously.

A Protocol Controller is an MQTT client that runs in its own OS process. It can
be written in any language and run on any platform that provides an MQTT client.

A Protocol Controller MUST implement these interfaces

* Unify Interface. The interface between the MQTT Broker and each Protocol Controller. This is an MQTT pub/sub connection, with topics and payloads specified by the UCL.

* PAN interface: NCP, SerialAPI or similar PHY-specific interface.

A IoT Service MUST implement these interfaces

* Unify Interface. The interface between the MQTT Broker and each Protocol
  Controller. This is an MQTT pub/sub connection with topics and payloads
  specified by the UCL.
* Cloud Interface: Interface from Cloud Service to IoT Service module in the
  Unify gateway. The first example is an AWS-IoT IoT Service module. Rules
  Engines can also be seen as special instances of IoT Services. The Cloud
  Interface is defined by the cloud service and implemented by the IoT Service.

### Protocol Controller API

Each Protocol Controller connects to the MQTT server. The Protocol Controller subscribes to an MQTT topic:

> `ucl/by-unid/#`

When packets are received, the Protocol Controller must drop all messages where
the UNID prefix does not match the manually-configured prefix of the Protocol
Controller. For example, the Zigbee Protocol Controller may be manually
configured to handle all UNIDs starting with, for example, `zb-example-pc-*` prefix, and
the BLE Protocol Controller might be configured for `ble001-*`. They will each
drop everything with UNID not matching that prefix.

A Protocol Controller MUST publish a message to the topic
`ucl/by-unid/<UNID>/State` to notify the IoT Services of the new node.

It MUST also process and translate all commands from a topic
`ucl/by-unid/<UNID>/+/+` to the PAN (i.e., the PHY Radio it owns), if the
`<UNID>` matches the prefix assigned to the Protocol Controller. The Cluster
and Command is derived by the Protocol Controller from the wildcard part of the
received topic and translated to PAN-specific commands as the Protocol
Controller sees fit. The UCL header in the MQTT payload must also be examined as
part of the translation to the native language. The translation of one UCL
command MAY result in a sequence of PHY native commands. For example, one UCL command
may be translated to a sequence of Z-Wave commands and then transmitted through
the NCP.

When commands are received from the PAN, the Protocol Controller must translate
and publish to the MQTT topic `ucl/by-unid/<source_UNID>/<CL>/<CMD>/<Attribute>` with a UCL header and
translated CMD payload in the MQTT payload field. The Protocol Controller must
fill out the UCL header fields in addition to concatenating with the
translated payload.

See [the usage of the Dotdot ZCL in Unify Framework for Application Level Control](doc/Chapter02-ZCL-in-uic.md) for details.

<b>Protocol Controller responsibilities</b>

1. Protocol Controller starts up and queries the current state of all PAN Nodes.
2. Configure all PAN nodes to send updates to the NCP controller by the Protocol Controller.
3. Whenever node state changes, publish to ucl/by-unid/&lt;UNID&gt;/ep&lt;EndPointID&gt;/&lt;Cluster&gt; for all UNIDs/EndPointIDs in PAN and for all Clusters supported. This allows IoT Services to discover nodes and read out the state by subscribing to ucl/by-unid/+/+/&lt;Cluster&gt;
4. Subscribe to ucl/by-unid/# and siphon off only the UNIDs assigned to this Protocol Controller.
5. Translate Clusters, Commands/WriteAttributes and Commands/ForceReadAttributes published to the node from UCL to the native Application Layer language of the PAN. Finally, transmit the translated sequence of frames to the radio.
6. Handle multicast:

  a. Subscribe to ucl/by-group/+/+/+ . The first wildcard is the group name being multicasted to. The Protocol Controller monitors the group name, and when appropriate multicasts to nodes in the PAN it controls. If the PAN does not support multicast or

  b. Subscribe to ucl/by-unid/+/+/Groups/+/+ . Group management is done via the groups cluster, so the Protocol Controller monitors all group management for nodes it controls. It maintains a list mapping groups to nodes it controls.

### IoT Service API

When creating IoT Service modules, the main goal is to keep this interface simple and easy to
implement correctly, securely, and robustly.

A cloud controller must implement an MQTT Client and TLS transport for the MQTT
connection. Unify abstracts away the differences between end-nodes on
different PHYs. The IoT Service can use a single unified API for all PHY types.
This is a primary goal of Unify.

<b>Note</b>: PHY-specific APIs MAY be added on demand, but a single API
covering all PHYs is strongly preferred. Of course, exceptions will arise over
the coming years as PHY technologies develop at break-neck pace and Unify may
as well be pragmatic about it. The UCL API is expected to develop with the PHY
technologies.

An IoT Service MUST implement at least the following API sections.

### IoT Service API for Discovering, Controlling, and Requesting Reports

An IoT Service that wants to control a command in a cluster on a device with a
given UNID MUST publish on the topic:

> `ucl/by-unid/&lt;UNID&gt;/ep&lt;EndPointID&gt;/&lt;ClusterName&gt;/Commands/&lt;CommandName&gt;`

It should also subscribe to the following topic filter to be notified of any
state update.

> ucl/by-unid/&lt;UNID&gt;/ep&lt;EndPointID&gt;/&lt;ClusterName&gt;/Attributes/#

AWS IoT Service (or any other IoT Service) can do the following:

* Subscribe to ucl/by-unid/# to get all messages from the Protocol Controllers
* Discover devices in one of these ways:
  * Discover all devices by subscribing to topic ucl/by-unid/+/+/State
  The client will receive a retained discovery message about on all available
  nodes and their endpoints
  * In a specific location by subscribing to topic ucl/by-location/living_room/+
  * Discover which device types are available by subscribing to
  'ucl/by-type/by-devicetype/#', which will yield a number of retained messages
  containing the UNID/endpoint of each node.
* Discover locations by
  * First discovering locations by subscribing to ucl/by-location/#.
  The client will now receive a retained message from each location with the
  MQTT-compatible name in the topic and the pretty-printed display name of the
  location in the JSON payload. This message has been sent by the
  Resource Directory, based on individual location announcements from nodes.
  * Discovering nodes in the location by subscribing to a topic filter such
  as ucl/by-location/&lt;LocationName&gt;/+. The publications will be retained
  messages advertising each node in that location. The MQTT payload will simply
  be the UNID repeated from the topic string, e.g., { "unid":"32049350" }.
  The node can then be probed and actuated in the ucl/by-unid/&lt;UNID&gt; namespace.

A subscribed IoT Service receives the following MQTT payload:  UCL hdr | CMD payload (see the Packet Format figure).

AWS IoT issue commands by publishing to
ucl/by-unid/&lt;UNID&gt;/ep&lt;EndPointID&gt;/&lt;ClusterName&gt;/Commands/&lt;CommandName&gt;
for a fire-and-forget transmission. On the PAN side, all reliability and
acknowledgment mechanisms will be used, but there will be no acknowledgment on
the MQTT client side.
An IoT Service should subscribe to
ucl/by-unid/&lt;UNID&gt;/ep&lt;EndPointID&gt;/&lt;ClusterName&gt;/Attributes/#
to be notified of attribute state changes.
The attribute changes reported will report if and when the command has been
carried out.

IoT Services must translate the entire UCL message including payload into
whatever format the Cloud Provider uses.

### Implementation Recommendations for IoT Services

IoT Service Reference Designs should preferably not be written in C
to avoid whole classes of common security and stability bugs, such as buffer
overflows and use-after-free pointer errors. Instead, a high level language with
focus on security MUST be used. Rust is a very good candidate with broad
HW platform support and focus on secure programs.

## The Resource Directory MQTT Client

A singleton Resource Directory MQTT client, responsible for aggregating node
information and providing the ucl/+/discover interface for discovering nodes,
where '+' can be one of (by-unid, by-location, by-type, by-clustercommand).
The Resource Discovery works in the following way:

* It is the only MQTT client guaranteed to connect immediately after the Unify broker starts up.

* It MUST add the NameLocation cluster to all nodes added to the Unify GW and
  handle all aspects of updating and discovering the name and location. The
  Protocol Controllers are not required to do anything regarding Name and
  Location.
  * RD subscribes to ucl/by-unid/&lt;UNID&gt;/ep&lt;EndPointID&gt;/NameAndLocation and stores all writes to the Name and Location attributes. This is done for each UNID and EndPointID in the network.
  * RD publishes to ucl/by-location/&lt;LOCATION&gt;/&lt;UID&gt;
  with payload {"unid":"987654321"} so other clients can discover
  the nodes by their location.
  All endpoints are assumed to be in the same location.
  * RD publishes to ucl/by-location/&lt;LOCATION&gt; with a payload of
  {"location-name-utf8" : "Children's Room"}. This allows location discovery by
  other clients. &lt;LOCATION&gt; is a valid MQTT topic name derived from the
  full UTF-8 location name.
* It MUST manage the global SmartStart provisioning list, which is common to all Protocol Controllers.

## ACL Engine

The ACL Engine acts as a filter between the MQTT broker and the various MQTT
clients. It ensures that IoT Service and Protocol
Controllers can only issue the commands they are trusted for. Without the ACL
Engine, any PAN Protocol Controller could send any and all commands to all other
PAN nodes managed by other Protocol Controllers. It may not be appropriate
for the BLE Protocol Controller to issue unlock commands directly to the Z-Wave
DoorLocks, for example. Similarly, just because an Azure IoT Service is allowed
to receive sensor data from the ZigBee PAN, it should not be allowed to unlock
BT Mesh doorlocks. Without an ACL engine, the pub/sub nature of the MQTT broker
would allow any MQTT client to publish and subscribe to any topic and hence
control and monitor anything connected to Unify.

Technically, the ACL Engine MUST identify each MQTT client by the TLS certificate it presents when connecting to the Broker. The ACL Engine MUST then look up the accessible topics for this client, either in an ACL stored in the ACL engine, or stored securely in the certificate itself (and signed by a trusted root). The exact mechanism to populate the internal ACL is TBD, but must be at least as strong as the TLS certificate validation and anchored in a strong-trust root certificate.

## Security Considerations

### Mandatory TLS Transport on Remote Access

TCP (non-TLS) MQTT transport for local connection SHOULD NOT be allowed. Raw TCP transport (non-TLS) MUST be rejected for remote connections including all non-monolithic variants with a remote MQTT broker.

MUST transport MQTT over TLS or WebSockets for Bridged Broker.

The type of TLS negotiated
[MUST be strong](https://www.w3.org/TR/wsc-ui/#typesoftls) and the Trust Anchors
used to validate certificates must be carefully selected. The trust anchors used
in the PKI for public web browsers MUST NOT be used for Unify gateways. Instead,
a trust anchor should be created and installed for a particular GW product.
Unify should provide an example and make it easy for Unify integrators and
manufacturing partners to generate and replace the trust anchor with their own.

### Unify Process Isolation

The Unify processes (Protocol Controllers, Resource Directory, IoT Services) MUST
be hardened against malformed input thorough input validation, for example, should
avoid common bug classes, such as array out-of-bounds addressing and pointer errors
by design. It is RECOMMENDED to use memory safe languages (not C/C++). The
processes SHOULD adopt the
[principle of least privileges](https://en.wikipedia.org/wiki/Principle_of_least_privilege),
and not require root privileges to run. If a root is required, the processes
SHOULD drop the privileges after initialization. A similar hardening SHOULD be
applied to the MQTT Broker, although this will probably be a third-party
implementation, at least in the first several phases of the Unify project.

The Protocol Controller and Resource Directory processes handle sensitive crypto
key material (at least the certificates and private keys for establishing TLS
session with MQTT broker, potentially also PAN network keys). PAN encryption
keys SHOULD (MUST in later phase) be stored in the PHY radio chip whenever
possible to protect from adversarial processes on the Unify GW itself.
Therefore, it must be protected against hostile processes reading the memory of
the Unify software. Similarly, the Unify software must have exclusive access to
the radios, so hostile processes cannot inject commands into the PAN. This must
be achieved through OS access control mechanisms.

### Mapping PAN Device Security to the Unified Controller Language

Protocol Controllers MUST fill out the UCL Header field "PAN Security" to indicate the security level of the received PAN frame that triggered the UCL publish message. The field will be used by the ACL to make sure non-secure commands can't control secure devices. This field MUST NOT control the outgoing security level transmitted on a PAN, that is to be determined by the Protocol Controller.

## Broker Discovery

To interact with the Unify GW and the PAN nodes behind it, the IP address of the
broker must be discovered. The mechanism of discovery is not yet defined, but
the most likely candidate is
[Multicast DNS (mDNS)](https://en.wikipedia.org/wiki/Multicast_DNS). Once
connected, Resource Discovery MUST be used to discover the nodes and
capabilities of the PAN nodes.

## Sleepy Devices

Protocol Controller must manage sleepy devices in the optimal way for each PHY. MQTT retained messages SHOULD be used to cache the latest status from a sleepy device.

### Z-Wave Battery Device

A Z-Wave Protocol Controller must handle messages to Sleepy Devices, which are known as Battery Devices or devices implementing the WakeUp Command Class, in the following way:

Whenever a Battery Device is included in the PAN, the Protocol Controller MUST set itself as the Lifeline of that node (i.e., make the node report all status commands to the Protocol Controller). Now the Battery Node will send Wakeup Notification Commands to the Protocol Controller whenever it is available for two-way communication where the node decides how often. Usually the frequency is selected to achieve a certain battery life.

After a message for a particular battery device has been published, and again when the message has been delivered and acknowledged on the application layer, the Protocol Controller must publish a state update to the usual ucl/by-unid/&lt;UNID&gt;/&lt;Cluster&gt; topic. The state must be described  both by a ["desired" and "reported" state](https://d1.awsstatic.com/whitepapers/Designing_MQTT_Topics_for_AWS_IoT_Core.pdf). The desired state must be updated when the message has been received by the Protocol Controller. When the message has been application layer acknowledged by the PAN Node ("the garage door is now fully lowered and closed"), another update must be published, this time changing the "reported" state. of the /&lt;Cluster&gt; topic. The Supervision command class can be used to obtain application level confirmations in Z-Wave.

### Zigbee Sleepy Device

<b>TBD:</b><b> Describe how to handle Zigbee Sleepy Devices</b>

### BT Mesh Low Power Devices

<b>TBD: Describe how to handle BT Mesh/BLE low-power devices</b>

## Mailbox

A mailbox functionality for storing messages to Sleepy devices until they wake up could exist as a shared MQTT module or a PHY specific mailbox could be in each Protocol Controller. It MAY start out PHY specific and move to a shared mailbox in the future.

## Reportables/Unsolicited Messages from the PAN

A IoT Service that wants to receive Reportables (a.k.a. unsolicited messages) from a PAN node MUST have a well-defined way to subscribe to all events from a PAN node. A IoT Service must also be able to subscribe to all event notifications from a particular cluster by subscribing to the topic ucl/by-unid/&lt;UNID&gt;/#. The IoT Service will then receive all reportables from the &lt;UNID&gt; device along with all commands sent to it.

To receive event notifications from multiple devices, MQTT topic wildcards can be used.

## True Status and Almost True Status for Sleepy Devices

[True Status is a feature that allows end nodes to immediately report changes in their status](https://community.smartthings.com/t/z-wave-switches-with-instant-update-2016/59328/7), especially when it is locally initiated. It has been covered by [patents for years](https://community.smartthings.com/t/z-wave-switches-with-instant-update-2016/59328), but at least [one patent](https://patents.google.com/patent/US5905442A/en) expired in 2016. This feature MUST be leveraged and exposed to the IoT Service in the Unify design, if the PAN nodes support it.

Status updates must be advertised to IoT Service as described in the [Reportables/Unsolicited messages from the PAN section](## Reportables/Unsolicited messages from the PAN).

## Rules Engines

Rules engines allow certain actions to be published when certain other events are received. Rules Engines should be distributed, in the sense that multiple IoT Services should be able to set up rules without conflicting with each other. For this, a single Rules Engine Module should be provided that manages a namespace likg ucl/rules_engine/ and providing an API for setting, getting and modifying if-then-else style rules. TBD: Should there be access controls in place to prevent IoT Services from modifying each other's rules? More advanced rules engines can be implemented by IoT Services using the raw publish/subscribe semantics. The Rules Engine SHOULD be implemented as a separate MQTT client to keep the Broker simple.

## Data Logging

A data logger is an MQTT client that subscribes to ucl/by-unid/+/+/&lt;CLUSTER-TO-LOG&gt;/# and logs the received messages somewhere. Multiple subscriptions and logging streams are possible.

## Command-Line Interface

It must be possible to control PAN devices and IoT Services from a Command-line interface. The CLI must allow SQA to test Protocol Controllers or IoT Services in isolation by injecting test stimulus to the broker. The CLI MUST also allow Gateway manufacturers to initiate RF performance tests.

## Data Caching or Status Database

A cache of recent status messages from the PAN SHOULD be maintained in the GW to reduce the load on the PAN network when facing repeated queries from the IoT Service. This Data Layer can be thought of as a continually updating database of the total PAN state. Ideally, IoT Services should query this self-refreshing Status Database, and never issue requests directly to the PAN. The subscriptions may need a mechanism to signal the desired freshness of the response, so the Data Caching component can decide to update the status before responding. Data caching should probably be handled in each Protocol Controller. The mechanism of distribution (Retained Messages in the MQTT broker) or a real database will be decided later. A replication mechanism between these data bases will allow multiple Unified Gateways to control the same PAN without multiplying the amount of probing traffic needed to keep their individual status data bases up to date.

## Latency Considerations

Unify must provide a perceived "latency-free" end-user experience. Lights and other devices should appear to respond immediately regardless of the type of Unify configuration (local MQTT broker, slim MQTT broker in cloud, full security with TLS for all broker connections, and so on). In practice, this requires a latency below 250 ms, which should be achievable given the following latency budget (all times are round-trips):

* Local Wi-Fi: 20-50 ms
* Local TLS processing 2 ms
* cloud connectivity including TLS: 20 ms
* cloud response time 20 ms
* PHY latency: 100 ms
* TOTAL: < 200 ms
Care must be taken in the Protocol Controller to avoid extra processing delays when translating and communicating with the PHY Radio.

## Examples and Use Cases

### Turning On a Zigbee Light Bulb from AWS IoT

Assume the Zigbee Protocol Controller has the prefix "zb" and the bulb has
EUI-64 0x9876543210abcdef. The Protocol Controller has assigned the UNID
"zb-9876543210abcdef" to the bulb.

1. Unify GW starts up.
2. Zigbee Protocol Controller initializes, connects to the NCP, and discovers the
   Zigbee PAN devices. It also probes which clusters the light bulb supports. For
   simplicity, assume that only the Level and basic clusters are supported.
   The Level cluster is trivially translated to UCL as Level cluster (same data
   model in UCL and Zigbee makes this easy). The Protocol Controller publishes
   the following Retained Message to allow service discovery:
   ucl/by-unid/zb-9876543210abcdef/ep0/LevelCluster payload = { "state" : {
   "reported" : {"OnOff":"0x00",
   "SupportedCommands":["Move to Level", "Move", "Step", "Stop", "Move to Level (with On/Off)", ...],
   "desired" : {...}}}}.
3. The Zigbee Protocol Controller subscribes to the following topics so it can execute commands in the cluster:
    * ucl/by-unid/zb-9876543210abcdef/ep0/OnOffCluster/WriteAttributes
    * ucl/by-unid/zb-9876543210abcdef/ep0/OnOffCluster/ForceReadAttributes
    * and one for each command in the Cluster: .../Move, .../MoveToLevel(withOnOff) etc.

4. The AWS-IoT IoT Service wants to display all devices that can be dimmed and
   turn on the first one. It connects to the Unify Broker and subscribes to
   ucl/by-unid/+/+/LevelCluster and receives the message described above. It now
   knows that UNID zb-9876543210abcdef Supports the Level cluster and can be
   dimmed.
5. To learn the type IoT device, the AWS-IoT IoT Service subscribes to
   ucl/by-type/dimmable_light/+ and receives a message back on the topic
   ucl/by-type/dimmable_light/zb-9876543210abcdef and knows that this is indeed
   a light.
6. The home owner's Alexa Smartphone App now displays the light bulb
   complete with a colorful bulb icon. The home owner
   clicks the "ON" button.
7. In a matter of milliseconds, the AWS-IoT IoT Service receives a command from
   the cloud to turn on the bulb. The IoT Service quickly translates this into
   UCL and publishes to the topic ucl/by-unid/LevelCluster/MoveToLevelOnOff with
   a payload = {"state" : {"Level":"100", "TransitionTime" : "0.5"}}
8. The Zigbee Protocol Controller receives this message, matches the prefix and
   translates the UCL message to a sequence of Zigbee Cluster Library Commands
   (maybe just one command) and sends those out over the NCP.
9. The bulb turns on.

### Updating the Firmware of a PIR Sensor from the Cloud

<b>Use case 1: Cloud service wants a fleet overview: A list of devices and their Firmware Versions.</b>

See [Common OTA FW Update Service](doc/Chapter05-common-ota.md)

<b>Use case 2: A cloud user wants to firmware update a Unify-managed device</b>

See [Common OTA FW Update Service](doc/Chapter05-common-ota.md)

### Multicasting to Two Light Bulbs

<b> Use case: A IoT Service wants to turn on two light bulbs simultaneously.</b>

Assume that the UNIDs (UNID1 and UNID2) have already been discovered.

1. Unify GW starts up and Protocol Controller initializes.
2. Protocol Controller subscribes to ucl/by-unid/+/+/Groups/Attributes/# and
   ucl/by-unid/+/+/Groups/SupportedCommands.
3. IoT Service uses the Groups cluster on UNID1/ep0 and UNID2/ep0 to add both
   UNIDs to the same group 1.
4. Protocol Controller receives the Groups commands via the subscription and
   adds UNID1/ep0 and UNID2/ep0 in group 1.
5. IoT Services reads the common capabilities of the group at
   ucl/by-group/1/+/SupportedCommands.
6. IoT Service publishes a command for the group. For example, {"Level:100",
   "TransitionTime":"0.05"} to topic
   ucl/by-group/1/Level/Commands/MoveToLevelWithOnOff.
7. Protocol Controller receives the message and issues a multicast message to
   those nodes. If managed by two different Protocol Controllers, a singlecast
   on each PHY will happen instead.

### Contractor Writing an IoT Service

<b>Type 1: Cloud provider uses MQTT</b>

Assume that a contractor is commissioned to build an IoT service. The IoT service connects UCL to a cloud solution of some sort. The contractor picks a MQTT client in a preferred language (in this use case Python), subscribes to relevant channels on the Unify broker, and publishes translated-to-cloud-solution commands to the cloud solution and vice-verse.

Translation should be a lot simpler than the Protocol Controllers due to structural similarities btw. UCL and prevalent IoT cloud solution architectures. For example, both AWS IoT and Azure IoT uses MQTT transport and fits in this category (AWS IoT is agnostic to the payload/data model, but UCL must be translated to the data model of the cloud service being targeted, so some translation is still needed).

<b>Type 2: Cloud provider does not use MQTT:</b>

Add MQTT client, subscribe to stuff on the GW broker, translate and transmit to cloud provider in whatever transport and format they use. This requires more work than type 1, obviously, but is still significantly easier than pre-Unify solutions.

### Contractor Writing a Protocol Controller

Contractor chooses an MQTT client in a preferred language and starts
implementing the APIs detailed above in the Protocol Controller API section.
Preferably, the contractor has in-depth knowledge of the PHY in question.

### IoT Service Discovering All Devices Supporting Cluster "DoorLock"

1. IoT Service subscribes to topic ucl/by-unid/+/+/DoorLock.
2. IoT Service receives a [Retained Message](https://www.hivemq.com/blog/mqtt-essentials-part-8-retained-messages/) from each endpoint supporting the DoorLock command class. These messages were published by the Protocol Controllers.
