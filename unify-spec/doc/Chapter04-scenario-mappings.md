# Scenario Mapping to Specific PHYs: Z-Wave, BLE and ZigBee

This chapter describes use-cases or scenarios using the same Unify commands but showing the Protocol Controller translation details for several PHYs.

## Adding a Node to a Network

This use case explains how to include a new node into a network. It is assumed that the
implementations follow the lines described in Network Management Commands and
Service discovery.

### Z-Wave PAN

\image html AddingAnodeWithaZ-WavePAN.png width=450px
\image latex AddingAnodeWithaZ-WavePAN.png width=450px

### BLE PAN

\image html AddingAnodeWithaBLEPAN.png width=450px
\image latex AddingAnodeWithaBLEPAN.png width=450px

### ZigBee PAN
The following section shows the Zigbee 3.0 Install Code based commissioning method. This method uses a Unify SmartStart DSK (Device-Specific Key) to convey the Install Code that is flashed out-of-band to a joining device. See the Zigbee Protocol Controller implementation references on how the DSK is formatted according to one of the formats specified in [Network Management](doc/Chapter03-network-management.md).

\startuml

' Allows simultaneous transmissions
!pragma teoz true

' Style for the diagram
skinparam BoxPadding 0
skinparam classFontColor black
skinparam classFontSize 10
skinparam classFontName Helvetica
skinparam sequenceMessageAlign left
skinparam shadowing false
skinparam ArrowColor #000000
skinparam ParticipantBackgroundColor #FFFFFF
skinparam ParticipantBorderColor #480509
skinparam SequenceLifeLineBorderColor #001111
skinparam SequenceLifeLineBorderThickness 2
skinparam NoteBackgroundColor #FFFFFF
skinparam NoteBorderColor #000000
skinparam ActorBackgroundColor #FFFFFF
skinparam ActorBorderColor #480509

hide footbox
title SmartStart List based commissioning using Z3 Install Code based Device-Specific Keys (DSK)

legend top
<font color=#0039FB>MQTT Subscription</font>
<font color=#00003C>Retained MQTT Publication</font>
<font color=#6C2A0D>Unretained MQTT Publication</font>
endlegend


' List of participants
participant "IoT Service" as iot_service
participant "Unify MQTT Broker" as mqtt_broker
participant "Unify UPVL" as upvl
participant "Zigbee Protocol Controller" as protocol_controller
participant "Zigbee Node 1" as node_1


== Out-of-band Setup of Joining Device  ==
node_1 -> node_1: Flash Installation Code
note over node_1
   Extract Install Code CRC
   to be included in DSK
end note
...
== Unify Initialization ==
upvl -> mqtt_broker: <font color=#0039FB>ucl/SmartStart/List/Remove</font>
upvl -> mqtt_broker: <font color=#0039FB>ucl/SmartStart/List/Update</font>
iot_service -> mqtt_broker: <font color=#0039FB>ucl/SmartStart/List</font>
iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/ProtocolController/NetworkManagement</font>
protocol_controller -> mqtt_broker: <font color=#0039FB>ucl/SmartStart/List</font>
...
upvl -> mqtt_broker: <font color=#00003C>ucl/SmartStart/List</font> \n <font color=#00003C><b> {"value":[]}


== Zigbee Protocol Controller advertises it's ready for Network Management Operations ==
protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/<PC_UNID>/ProtocolController/NetworkManagement \n<font color=#00003C><b>{\n\t<font color=#00003C><b>"State" : "idle", \n\t<font color=#00003C><b>"SupportedStateList": ["remove node"], \n<font color=#00003C><b>}
& mqtt_broker -> iot_service
...


== Add Node 1 Install Code/DSK via the UPVL ==
note over iot_service
   Format EUI64, Install Code,
   and CRC extracted from the
   joining device into a
   recognized DSK format.
end note
iot_service -> mqtt_broker : <font color=#6C2A0D>ucl/SmartStart/List/Update \n<font color=#6C2A0D><b>{\n\t<font color=#6C2A0D><b>"DSK" : "NODE_1_DSK" \n\t<font color=#6C2A0D><b>"Include" : true \n\t<font color=#6C2A0D><b>"Unid" : "" \n\t<font color=#6C2A0D><b>"ProtocolControllerUnid" : "" \n<font color=#6C2A0D><b>}</b>
& mqtt_broker -> upvl
upvl -> mqtt_broker : <font color=#00003C>ucl/SmartStart/List \n<font color=#00003C><b>{"value":[{\n\t<font color=#00003C><b>"DSK" : "NODE_1_DSK" \n\t<font color=#00003C><b>"Include" : true \n\t<font color=#00003C><b>"Unid" : "" \n\t<font color=#00003C><b>"ProtocolControllerUnid" : "" \n<font color=#00003C><b>}]}</b>
& mqtt_broker -> protocol_controller

protocol_controller -> protocol_controller: NODE_1 Install Code based Link Key \nadded to Trust Center (TC)

== Node 1 Starts Network Steering ==

node_1 -> node_1: Network Steering \nwith TC Link key update
node_1 -> protocol_controller: Device Annouce + TC join
protocol_controller -> mqtt_broker: <font color=#00003C>ucl/by-unid/<PC_UNID>/ProtocolController/NetworkManagement \n<font color=#00003C><b>{\n\t<font color=#00003C><b>"State" : "add node", \n\t<font color=#00003C><b>"SupportedStateList": ["idle"], \n<font color=#00003C><b>}
& mqtt_broker -> iot_service
...
node_1 -> node_1: Perform TC Link key update
node_1 -> protocol_controller: Replacement APS Link Key Verified on NODE_1
protocol_controller -> mqtt_broker: <font color=#00003C>ucl/by-unid/<PC_UNID>/ProtocolController/NetworkManagement \n<font color=#00003C><b>{\n\t<font color=#00003C><b>"State" : "add node", \n\t<font color=#00003C><b>"SupportedStateList": ["idle"], \n<font color=#00003C><b>}
& mqtt_broker -> iot_service
protocol_controller -> mqtt_broker: <font color=#00003C>ucl/by-unid/<NODE_1_UNID>/State \n<font color=#00003C><b>{\n\t<font color=#00003C><b>"NetworkStatus" : "Online interviewing", \n<font color=#00003C><b>}
& mqtt_broker -> iot_service
== Update the Unid field in the SmartStart list via the UPVL ==
protocol_controller -> mqtt_broker: <font color=#6C2A0D>ucl/SmartStart/List/Update \n<font color=#6C2A0D><b>{\n\t<font color=#6C2A0D><b>"DSK" : "NODE_1_DSK" \n\t<font color=#6C2A0D><b>"Unid" : "NODE_1_UNID" \n<font color=#6C2A0D><b>}}</b>
mqtt_broker -> upvl
upvl -> mqtt_broker : <font color=#00003C>ucl/SmartStart/List \n<font color=#00003C><b>{"value":[{\n\t<font color=#00003C><b>"DSK" : "NODE_1_DSK" \n\t<font color=#00003C><b>"Include" : true \n\t<font color=#00003C><b>"Unid" : "NODE_1_UNID" \n\t<font color=#00003C><b>"ProtocolControllerUnid" : "" \n<font color=#00003C><b>}]}</b>
& mqtt_broker -> iot_service

== Zigbee Protocol Controller advertises it's ready for Network Management Operations again ==
protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/<PC_UNID>/ProtocolController/NetworkManagement \n<font color=#00003C><b>{\n\t<font color=#00003C><b>"State" : "idle", \n\t<font color=#00003C><b>"SupportedStateList": ["remove node"], \n<font color=#00003C><b>}
& mqtt_broker -> iot_service

\enduml

NOTE: ZigBee touchlink is not currently supported as a commissioning method.


## Retrieving Diagnostic Data from a Network

The following section shows how diagnostic data is retrieved in a Z-Wave PHY via UCL. You can add outlines of mappings to more PHYs using the same MQTT / DotDot interface (i.e., UCL) later.

## Special Requirements for BLE AoX Protocol Controllers

BLE-based AoX (Angle-of-X) solutions rely on locators to report the angle of
arrival of beacons transmitted from a tag. Locators typically control radio
boards connected to one or more antennas. Multi-locators are used to infer the
position of the tag based on the several angle information readings from
one or more locators.

Locators are Protocol Controllers in the Unify framework. Multi-locators are
typically IoT Services because they don't control a PHY radio. Both are only
required to support the Location cluster along with a minimal
ProtocolController/NetworkManagement topic.

### AoX Protocol Controller

All Unify nodes MUST present a `ucl/by-unid/<UNID>/ep<ID>/<CLUSTER>` topic,
and protocol controllers MUST publish a
`ucl/by-unid/<UNID>/ProtocolController/NetworkManagement` topic. The AoX
Protocol Controller MUST present publish both topics for each Locator.

The endpoint should always be ep0 for an AoX locator because it does not make sense
to have multiple endpoints for such a device.

The actual Location Data is shared via the
[Location Cluster](doc/Chapter08-application-clusters.md).

### Forming the Locator and Multi-locator UNID

The Unify spec only requires that UNIDs are unique as specified in
[the section called Unified Node IDs](doc/Chapter01-architecture-overview.md). A
good choice for the locator UNID could therefore be `ble-pd-aaaaaaaaaaaa`. Deriving
the UNID from the Bluetooth address guarantees uniqueness. The multi-locator
UNID can be any unique string, for example `multilocator-1234-lobby`.

### Network Management for AoX Protocol Controllers

Because an AoX Protocol Controller does not need any network management or
configuration options, it is enough to publish the following fixed payload for
each locator. This allows enumerating all nodes connected to a Unify, including
the AoX locators.

**MQTT topic**: `ucl/by-unid/<LOCATOR_UNID>/ProtocolController/NetworkManagement`

**Payload:**

```json
{
   "State": "idle",
   "SupportedStateList": []
}
```

The AoX PC may publish a state of "temporarily unavailable" if it has failure
modes where it cannot report position data. When entering this mode, it should
publish a state change to "temporarily unavailable". When normal operation
resumes, a state change to "idle" should be published.

Note that there are no other supported states because the AoX PC does not
require any network management functions to operate.
