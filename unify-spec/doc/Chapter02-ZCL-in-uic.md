# Dotdot ZCL in Unify Framework for Application Level Control

## Introduction

This chapter provides application examples about the usage of
the DotDot Zigbee cluster library specification [ZCL2018](doc/references.md)
in the Unify framework considering that the node capabilities
have been discovered (i.e., the supported clusters, commands and attributes).
The Unify framework uses MQTT transport protocol as an internal message
transportation between Clients (IoT Service API, Protocol Controller, and so on),
and unified controller application language that enables seamless
communications across a Unify network.

In the Unify framework, every Protocol Controller must discover the capability of
the PAN nodes in its network and advertise the capabilities of each node
(supported commands and attributes) to interested parties (i.e., IoT Service for
a cloud service like AWS-IOT) using MQTT protocol. This procedure known as
[service discovery](doc/Chapter06-service-discovery.md). After the PAN nodes
capabilities have been discovered, the IoT service MQTT clients can control and
observe the behavior of the nodes using the unified controller application
language. The unified application language is based on a publish/subscribe
messaging transport protocol and the dotdot/ZigBee Cluster library (ZCL)
specification.

## MQTT Topics

The MQTT topics are divided into sub-spaces for each device to allow a Protocol
Controller to advertise state and capabilities of a given device and IoT
Services to read the state and control a given device.

There are three main spaces in a cluster MQTT topic, as follows:

* `ucl/by-unid/+/+/&lt;ClusterName&gt;/Attributes/#` : This represents the
  cluster attributes state.
* `ucl/by-unid/+/+/&lt;ClusterName&gt;/SupportedCommands`: This represents the
  list of commands that the node can receive for this cluster.
* `ucl/by-unid/+/+/&lt;ClusterName&gt;/Commands/+` : This is the topic where
  IoT Services issue commands.

The `/+/+` wildcards can be replaced with a specific UNID and endpoint to
control/monitor only a single endpoint.

An overview of these topics is shown in the table below.

<table>
<caption>MQTT Topics</caption>
<tr>
<th>MQTT Topic</th>
<th>Protocol<br/>Controllers</th>
<th>IoT Services</th>
<th>Description</th>
</tr>
<tr>
<td>`ucl/by-unid/&lt;UNID&gt;/ep&lt;EndPointID&gt;/&lt;ClusterName&gt;/Attributes/&lt;AttributeName&gt;/Reported`</td>
<td>Publish (retained)</td>
<td>Subscribe</td>
<td>Advertises the reported value of an attribute for the Cluster</td>
</tr>
<tr>
<td>`ucl/by-unid/&lt;UNID&gt;/ep&lt;EndPointID&gt;/&lt;ClusterName&gt;/Attributes/&lt;AttributeName&gt;/Desired`</td>
<td>Publish (retained)</td>
<td>Subscribe</td>
<td>Advertises the desired value of an attribute for the Cluster</td>
</tr>
<tr>
<td>
`ucl/by-unid/&lt;UNID&gt;/ep&lt;EndPointID&gt;/&lt;ClusterName&gt;/Attributes/#`</td>
<td>Publish (retained)</td>
<td>Subscribe</td>
<td>
  Advertises the reported/desired value of a non-standard attribute for the
  Cluster. Non-standard attributes are defined in this specification.
  The last level of this topic space will always be /Reported or /Desired.
</td>
</tr>
<tr>
<td>`ucl/by-unid/&lt;UNID&gt;/ep&lt;EndPointID&gt;/&lt;ClusterName&gt;/SupportedCommands`</td>
<td>Publish (retained)</td>
<td>Subscribe</td>
<td>
  Advertises an array of commands that can be published under the Commands/namespace
  by IoT Services to control the device.
</td>
</tr>
<tr>
<td>`ucl/by-unid/&lt;UNID&gt;/ep&lt;EndPointID&gt;/&lt;ClusterName&gt;/Commands/&lt;CommandName&gt;`</td>
<td>Subscribe</td>
<td>Publish (unretained)</td>
<td>
  Instruct a Protocol Controller to send a command to the UNID/endpoint cluster
  server
</td>
</tr>
</table>

### Reported and Desired

The state of a cluster for a device/endpoint is represented by a list of
attributes, additional (non-standard) attributes, and a list of supported
commands.

A PAN node may perform state transitions or delays may occur between an IoT
Service issuing an MQTT command and the device itself confirming that its state
matches what the IoT Service wished to accomplish. For that purpose, each
attribute in a Cluster has two values, a *desired* and a *reported* value.
The *desired value* indicates what the Protocol Controller attempts to get
device/endpoint to reach. The *reported value* indicates the state confirmed by
the device/endpoint itself.

More information about the 'desired' and 'reported' state description can be
found in the [Amazon AWS Specification]
(https://docs.aws.amazon.com/iot/latest/developerguide/device-shadow-document-syntax.html#device-shadow-example-request-json).

### Cluster MQTT Topics

The non-protocol controller IoT Services (Cloud Service connectors,
Middleware Translator, and Rules Engines) should subscribe to the following topic
filter to get any application message corresponding to the state of given
cluster from any PAN node in the unified network:

> `ucl/by-unid/+/+/&lt;ClusterName&gt;/#`

The Protocol Controller MUST then publish to the following topics to advertise
the supported attributes of a PAN Node and also when the attributes state is
updated:

```mqtt
  ucl/by-unid/<UNID>/ep<EndPointID>/<ClusterName>/Attributes/<AttributeName>/Reported { "value": 23 }
```

For each attribute, the value property is required. This represents the value
of the attribute and it can be any of the following JSON types:

* string
* number
* boolean
* null
* an object, containing boolean types (only for bitmasks)

For example, a Protocol Controller advertising the state of a device/endpoint
being a OnOff and Level cluster server will publish the following:

```mqtt
  ucl/by-unid/zw-23485/ep0/OnOff/Attributes/OnOff/Desired { "value": true}
  ucl/by-unid/zw-23485/ep0/OnOff/Attributes/OnOff/Reported { "value": true}
  ucl/by-unid/zw-23485/ep0/Level/Attributes/CurrentLevel/Desired { "value": 50}
  ucl/by-unid/zw-23485/ep0/Level/Attributes/CurrentLevel/Reported { "value": 100}
```

Furthermore, in the example above, the PAN node is dimming its level from 100% to
50%. When it reaches 50% and the Protocol Controller has received a confirmation,
the reported state of the CurrentLevel attribute will be updated to match the
desired value.

Strings MUST be used by Protocol Controllers when a descriptive name is
given to the possible values for a given attribute by the DotDot XML files.
For example, the *LockState* attribute of a door lock MUST be published as
follows:

```mqtt
  ucl/by-unid/zw-23485/ep0/Lock/Attributes/LockState/Desired { "value": "Locked"}
```

If the individual values do not have a description string name, their value MUST
be presented using either a JSON number or boolean. See the
ZCL Cluster Library [ZCL2018](doc/references.md).

Finally, only JSON objects are allowed for bitmaps/bitmasks when names are
defined in the ZCL Cluster Library [ZCL2018](doc/references.md). Numbers MUST
NOT be used in this case. For example, the ColorCapabilities attribute of the
ColorControl cluster MUST be represented as follows:

> ucl/by-unid/zw-23485/ep0/ColorControl/Attributes/ColorCapabilities/Reported

```json
{
  "value": {
    "HueSaturationSupported": true,
    "EnhancedHueSupported": true,
    "ColorLoopSupported": true,
    "XYSupported": true,
    "ColorTemperatureSupported": true
  }
}
```

#### Mandatory Cluster Attributes

PAN Nodes may support more than the minimum mandatory set of attributes for a
given cluster. When a node is interviewed and its capabilities are discovered,
a Protocol Controller MUST, at a minimum, publish all attributes for a cluster
that are specified as *mandatory* in the ZigBee Cluster library.

For example, for a PAN Node supporting the OnOff cluster, it will be guaranteed
that the *OnOff* attribute will be published, however, the
*GlobalSceneControl* will be published only if supported by the PAN node.

```mqtt
  ucl/by-unid/zw-23485/ep0/OnOff/Attributes/OnOff/Desired { "value": true}
  ucl/by-unid/zw-23485/ep0/OnOff/Attributes/OnOff/Reported { "value": true}
  ucl/by-unid/zw-23485/ep0/OnOff/Attributes/GlobalSceneControl/Desired { "value": true}
  ucl/by-unid/zw-23485/ep0/OnOff/Attributes/GlobalSceneControl/Reported { "value": true}
```

#### Missing Mandatory Cluster Attributes

Because of multiple PHYs and the application level variety between them,
a node might be shown as a ZCL cluster server, despite missing a
mandatory attribute.

For example, in Z-Wave, it is possible to have Thermostats without a local
temperature reading. These thermostats can nonetheless be controlled by MQTT
clients and the lack of a temperature reading should not prevent the exposition
of such a device to the IoT Services.

In this case, the value of the attribute MUST be set to a null JSON type. For
instance, a thermostat without local temperature MAY be published as follows:

```mqtt
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/LocalTemperature/Desired { "value": null}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/LocalTemperature/Reported { "value": null}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/OccupiedCoolingSetpoint/Desired { "value": 2600}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/OccupiedCoolingSetpoint/Reported { "value": 2600}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/OccupiedHeatingSetpoint/Desired { "value": 2100}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/OccupiedHeatingSetpoint/Reported { "value": 2100}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/SystemMode/Desired { "value": "Auto"}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/SystemMode/Reported { "value": "Auto"}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/ThermostatProgrammingOperationMode/Desired { "value": 0}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/ThermostatProgrammingOperationMode/Reported { "value": 0}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/ControlSequenceOfOperation/Desired { "value": 4}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/ControlSequenceOfOperation/Reported { "value": 4}
```

In any case, a Protocol Controller MUST publish the *LocalTemperature* attribute
state when publishing a node as a Thermostat Cluster server because it is a
mandatory attribute.

When a reported state is updated from the PAN node, the Protocol
Controllers MUST update both the reported and desired values of the
corresponding attributes. In this case, Protocol Controllers SHOULD publish the
desired part of the updated state first.

#### Cluster Revisions

A special attribute under every cluster called ClusterRevision is used to
advertise the ZCL revision of the cluster that the node and/or Protocol
Controller are using.

ProtocolControllers MUST publish this information for all clusters of all
nodes they advertise.

See the the ZCL Cluster Library [ZCL2018](doc/references.md) for Cluster
revision details.

The JSON value type for the ClusterRevision attribute MUST be an integer.
Both Desired and Reported values MUST be published by Protocol Controllers.
The ClusterRevision is a read-only attribute.

For example, publishing the cluster revision of an OnOff node:

```mqtt
ucl/by-unid/zw-23485/ep0/OnOff/Attributes/OnOff/Desired {"value": true}
ucl/by-unid/zw-23485/ep0/OnOff/Attributes/OnOff/Reported {"value": true}
ucl/by-unid/zw-23485/ep0/OnOff/Attributes/ClusterRevision/Desired {"value": 2}
ucl/by-unid/zw-23485/ep0/OnOff/Attributes/ClusterRevision/Reported {"value": 2}
```

#### Additional Attributes

Additional attributes under a cluster state can be used to advertise state
information that is not standardized as Cluster attributes.

The format of this additional state information is UCL-specific and defined in
this specification. It follows as closely as possible the data models described
in the ZCL Cluster Library [ZCL2018](doc/references.md).

The additional state information can be affected by the use of commands and
it MUST also follow a desired/reported state. Most clusters do not have any
additional attributes defined.

Additional attributes MUST be published under the same `Attributes/` MQTT topic
hierarchy as the standard cluster attributes, however, the MQTT topic hierarchy
MAY contain additional levels before the final Desired or Reported level.

For example, the Scene Cluster definition uses a Scene Table, which
is advertised to the IoT Services using a UCL-specific definition. Non-standard
attributes MUST be represented using object JSON types. This specification
will provide a JSON schema for all non-standard attributes.

**Topic:** `ucl/by-unid/zw-23485/ep0/Scenes/Attributes/SceneTable/1/Desired`

**Payload:**

```json
{
  "GroupID": 12,
  "SceneID": 1,
  "SceneName": "Cozy Scene",
  "TransitionTime": 10,
  "TransitionTime100ms": 0,
  "SceneTableExtensions": {
    "OnOff": {
      "OnOff": true
    },
    "Level": {
      "CurrentLevel": 25
    },
    "ColorControl": {
      "CurrentX": 25,
      "CurrentY": 3455
    }
  }
}
```

#### Supported Commands

IoT Services MUST be able to read the commands that are supported for a Cluster
server to use them.

The list of supported Commands for each PAN node MUST advertised by the Protocol
Controllers in the *SupportedCommands* topic under the Cluster level.

The list of supported Commands MUST be advertised using a JSON array containing
strings. For example, the list of supported commands for the OnOff cluster could
look like this:

**Topic:** `ucl/by-unid/zw-23485/ep0/OnOff/SupportedCommands`

**Payload:**

```json
{
  "value": ["On","Off","Toggle","ForceReadAttributes","WriteAttributes"]
}
```

It is OPTIONAL for a Protocol Controller to publish the availability of the
*WriteAttributes* and *ForceReadAttributes* commands. These commands MUST be supported
for all clusters advertised by a Protocol Controller.

  Note: A ProtocolController MAY publish an empty array for the SupportedCommands
  of a cluster. In this case, IoT Services can only use the WriteAttributes commands.

  Note 2: IoT Services MUST NOT wait for a feedback after sending a ForceReadAttributes command.
  This command is used to *force* an update on the usual feedback channels - i.e.,
  the *reported* MQTT topic.

Protocol Controllers SHOULD explicitly advertise the availability of the
*WriteAttributes* Command in the SupportedCommands topic when there is at least
one writable attribute supported by the PAN node
(i.e., marked as `writable=true` in the ZCL cluster definition).

Protocol Controllers SHOULD NOT advertise the availability of the
*WriteAttributes* Command in the SupportedCommands topic when there is no
writable attribute supported by the PAN node for the cluster. IoT Services MAY
use this command but it will have no effect.

Protocol Controllers SHOULD explicitly advertise the availability of the
*ForceReadAttributes* Command in the SupportedCommands topic when there is at least
one readable attribute that is of a type not of an integer or floating point type
(i.e., an attribute of type "String" or other complex data types).

Protocol Controller SHOULD advertise the *ForceReadAttributes* Command for all
endpoints with readable mandatory attributes, as supported in the ZCL cluster definition.

#### ZCLversion

IoT Services can identify the ZCL version of each PAN Node for any of its clusters
by reading the ZCLversion attribute of the Basic Cluster.

This helps to track the cluster version supported by the Protocol Controller
and/or end node. In addition, this could be needed if a certain cluster version
can't be mapped from another PHY.

By default, if no version information is provided, an IoT service SHOULD assume
version 3.

### Command MQTT Topic

When a PAN node is included to the network, a Protocol Controller servicing
the node MUST subscribe to the following topic filter to receive
incoming Commands:

> `ucl/by-unid/&lt;UNID&gt;/ep&lt;EndPointID&gt;/&lt;ClusterName&gt;/Commands/+`

It is OPTIONAL for a Protocol Controller to subscribe to the command topic
filter only if the PAN node supports no commands and all attributes are read-only
attributes (i.e., the WriteAttributes command will have no effect).

When the IoT Services (AWS IoT connectors, Web-based UIs, Rules Engines,
Data Loggers, and so on.) want a PAN node to execute a given command, the Clients can
publish a command payload using the respective command topic:

**Topic:** `ucl/by-unid/&lt;UNID&gt;/ep&lt;EndPointID&gt;/&lt;ClusterName&gt;/Commands/&lt;CommandName&gt;`

**Payload:**

```json
{
  "FieldName1": "value",
  "FieldName2": "value"
}
```

If the command does not have any mandatory fields, an empty JSON object
(i.e., {}) MUST be sent as a payload.

If the command has mandatory fields, the payload MUST contain all of
them. For example, in the 'Level' cluster, the 'MoveToLevel' command consists
of two mandatory fields (i.e., 'Level' and 'Transition Time' ).
In this case, the payload MUST be: `{"Level": 100, "TransitionTime": 0}`.

Each property representing a field MUST be of the following JSON types:

* string
* number
* boolean
* null
* an object, containing boolean types (only for bitmasks)

Strings MUST be used in command payloads when a descriptive name is
given to the possible values for a given attribute by the DotDot XML files.

If the individual values do not have a description string name, their value MUST
be presented using either a JSON number or boolean. See the
ZCL Cluster Library [ZCL2018](doc/references.md).

Finally, only JSON objects are allowed for bitmaps/bitmasks when names are
defined in the ZCL Cluster Library [ZCL2018](doc/references.md). Numbers MUST
NOT be used in this case. For example, the UpdateFlags field of the
ColorLoopSet command in the ColorControl cluster MUST be represented as
follows:

> ucl/by-unid/zw-23485/ep0/ColorControl/Commands/ColorLoopSet

```json
{
  "UpdateFlags" : {
    "UpdateAction": true,
    "UpdateDirection": true,
    "UpdateTime": true,
    "UpdateStartHue": false
  }
}
```

#### WriteAttributes Generic Command

A Protocol Controller MUST always publish the desired/reported state of all
mandatory attributes in a cluster, using the Cluster state topic space.
An attribute state can be changed in three ways:

* Locally on the PAN node
* Using a specific command as described above
* Using a generic command called 'WriteAttributes'

For example, an IoT Service MAY issue the following payload to modify
the desired values of writable attributes directly:

**Topic:** `ucl/by_unid/&lt;UNID&gt;/ep&lt;EndPointID&gt;/&lt;ClusterName&gt;/Commands/WriteAttributes`

**Payload:**

```json
{
  "AttributeName1": "value",
  "AttributeName2": "value"
}
```

Note that all PAN nodes, which support writable attributes MUST be writable directly
using the *WriteAttributes* Command. Read-only attributes received in this
Command will be ignored. See [ZCL2018](doc/references.md) for details
about writable/read-only attributes.

To ensure an end-to-end quality of service, a Protocol Controller MUST always
publish the attributes desired/reported state when it detects a state update.
IoT Services can therefore see the effect of their published commands by
subscribing to the cluster *Attributes* space.

Non-standard attribute cannot be updated or modified using the *WriteAttributes*
command.

### ForceReadAttributes Generic Command

The *ForceReadAttributes* command is used to force an update of specified
attributes that may not be properly reported using standard reporting mechanisms.
On reception of this command, the Protocol Controller SHOULD request the latest
available data from the network for each attribute specified in the command. The
data SHOULD be published on the *reported* MQTT topic for each attribute.

ForceReadAttributes should be used sparingly because it can generate significant
radio traffic. The default behavior of the protocol controllers already
guarantee a good trade off between timely updates and radio traffic without the
use of ForceReadAttributes.

If the newly read value of an attribute matches its previously
reported value, a new MQTT message MAY not be published.

If the payload of the received MQTT command is empty, the Protocol Controller
SHOULD request new data for all supported attributes of that cluster.

The *ForceReadAttributes* command is intended only for attributes with values that
cannot be adequately handled by standard reporting mechanisms. For example,
the 'String' or 'Array' types on Zigbee clusters, which would NOT notify the
Protocol Controller when their values change. For a complete list of expected types,
see [ZCL2018](doc/references.md).

For standard attribute types, a Protocol Controller SHOULD rely on the *reported*
MQTT topic alone.

For example, an IoT Service MAY issue the following payload to directly read
several attributes of a given cluster

**Topic:** `ucl/by-unid/&lt;UNID&gt;/ep&lt;EndPointID&gt;/&lt;ClusterName&gt;/Commands/ForceReadAttributes`

**Payload:**

```json
{
  "value" :
    [
      "AttributeName1",
      "AttributeName2",
      "AttributeName6"
    ]
}
```

In response, the Protocol Controller should update the 'reported' MQTT topics for
'AttributeName1' and 'AttributeName2', as discussed in the [MQTT Topics](Reported and desired).

Another example, an IoT Service MAY issue the following payload to directly read
ALL the attributes of a given cluster

**Topic:** `ucl/by-unid/&lt;UNID&gt;/ep&lt;EndPointID&gt;/&lt;ClusterName&gt;/Commands/ForceReadAttributes`

**Payload:**

```json
{
  "value" : []
}
```

In response, the Protocol Controller should update ALL available attributes for *ClusterName*, as
specified in [ZCL2018](doc/references.md).

*ForceReadAttribute* support for non-standard ZCL cluster attributes is optional.

*ForceReadAttribute* is EXPLICITLY not supported for groups or multicast due to network performance
considerations.

## Clusters for Lights, Switches, DoorLock, Thermostat, and Occupancy Sensor Applications

The following subsections provide specific cluster examples using the Cluster
state and Commands topics, such as Dimmer lighting, Switch, Door Lock,
Thermostat, and Occupancy Sensor applications. The detailed description of each
cluster can be found in [ZCL2018](doc/references.md).

Each cluster has a list of mandatory attributes and commands. All mandatory
attributes in a given cluster MUST be advertised by a Protocol Controller for
all devices/endpoints. All mandatory Commands SHOULD be advertised in the
SupportedCommand topic, when possible.

### Groups Cluster (0x0004)

This cluster is used to define group memberships for PAN nodes.

This cluster requires additional attributes, because the *Group list* is not captured
by its defined attributes. See [ZCL2018](doc/references.md) for the
*Groups* cluster definition.

Each group is identified using a 16 bits unsigned integer value.
Below is an example of a PAN node Group Cluster server:

```mqtt
ucl/by-unid/zw-23485/ep0/Groups/Attributes/ClusterRevision/Desired {"value": 2}
ucl/by-unid/zw-23485/ep0/Groups/Attributes/ClusterRevision/Reported {"value": 2}
ucl/by-unid/zw-23485/ep0/Groups/Attributes/NameSupport/Desired {"value": {"Supported":true}}
ucl/by-unid/zw-23485/ep0/Groups/Attributes/NameSupport/Reported {"value": {"Supported":true}}
ucl/by-unid/zw-23485/ep0/Groups/Attributes/GroupList/Desired {"value": [1, 2] }
ucl/by-unid/zw-23485/ep0/Groups/Attributes/GroupList/Reported {"value": [1] }
ucl/by-unid/zw-23485/ep0/Groups/Attributes/1/Name/Desired {"value": "Kitchen nodes"}
ucl/by-unid/zw-23485/ep0/Groups/Attributes/1/Name/Reported {"value": "Kitchen nodes"}
ucl/by-unid/zw-23485/ep0/Groups/Attributes/2/Name/Desired {"value": "Light bulbs"}
ucl/by-unid/zw-23485/ep0/Groups/SupportedCommands {"value": ["AddGroup", "ViewGroup", "GetGroupMembership", "RemoveGroup", "RemoveAllGroups", "AddGroupIfIdentifying"] }
```

The only mandatory attribute in the group cluster is the NameSupport attribute.
If a PAN node supports name for groups, it will advertise the assigned names
for each of its groups under the following topics:

**Topic:** `ucl/by_unid/&lt;UNID&gt;/ep&lt;id&gt;/Groups/Attributes/&lt;GroupID&gt;/Name/Reported`

**Topic:** `ucl/by_unid/&lt;UNID&gt;/ep&lt;id&gt;/Groups/Attributes/&lt;GroupID&gt;/Name/Desired`

The names assigned to each group will be a value containing a string as if
the name was a regular string attribute.

**JSON Schema:**

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "Group name objects",
  "description": "Names assigned to individual groups. (Group cluster)",
  "type": "object",
  "properties": {
    "value": {
      "type": "string"
    }
  },
  "required": [
    "value"
  ]
}
```

The list of groups that a node is part of will be advertised under the GroupList
topic. Note that GroupID 0 is not a valid group and MUST NOT be used.

**Topic:** `ucl/by_unid/&lt;UNID&gt;/ep&lt;id&gt;/Groups/Attributes/GroupList/Desired`

**JSON Schema:**

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "GroupList Object",
  "description": "List of Groups that a PAN Node belongs to. (Group cluster)",
  "type": "object",
  "properties": {
    "value": {
       "type": "array",
        "items": {
          "type": "integer",
          "minimum": 1,
          "maximum": 65535
        }
    }
  },
  "required": [
    "value"
  ]
}
```

### Scenes Cluster (0x0005)

This cluster is used to define scene and configuration of each device/endpoint.
In the Unify context, it can be used both for scene controlling devices
(e.g., wall controller button) and for activating scenes for other nodes
(e.g., light dimmer).

This cluster requires additional attributes because the *Scene Table* is not captured
by its defined attributes. See [ZCL2018](doc/references.md) for the
*Scene Table* definition.

Below is an example of a PAN node Scene Cluster server:

```mqtt
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/ClusterRevision/Desired {"value": 2}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/ClusterRevision/Reported {"value": 2}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/SceneCount/Desired {"value": 5}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/SceneCount/Reported {"value": 5}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/CurrentScene/Desired {"value": 2}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/CurrentScene/Reported {"value": 2}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/CurrentGroup/Desired {"value": 1}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/CurrentGroup/Reported {"value": 1}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/SceneValid/Desired {"value": true}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/SceneValid/Reported {"value": true}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/NameSupport/Desired {"value": true}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/NameSupport/Reported {"value": true}
ucl/by-unid/zw-23485/ep0/Scenes/SupportedCommands {"value" :["AddScene", "ViewScene", "RemoveScene", "RemoveAllScenes", "StoreScene", "RecallScene", "GetSceneMembership", "WriteAttributes"] }
```

Note that in the example above, the *ViewScene* and *GetSceneMembership* commands
are available, but will have no effect, because their purpose is to retrieve
data. Retrieving data is done automatically by the protocol controllers.
In this case, a Protocol Controller SHOULD remove these commands from the
*SupportedCommands* list.

Note that in the example above, the *WriteAttributes* commands is available, but
will also have no effect because all standard attributes are read-only in this cluster.
In this case, a Protocol Controller SHOULD remove these commands from the
*SupportedCommands* list.

The Scene Table data is represented using this topic hierarchy:

**Topic:** `ucl/by_unid/&lt;UNID&gt;/ep&lt;id&gt;/Scenes/Attributes/SceneTable/&lt;SceneNumber&gt;/Reported`

**Topic:** `ucl/by_unid/&lt;UNID&gt;/ep&lt;id&gt;/Scenes/Attributes/SceneTable/&lt;SceneNumber&gt;/Desired`

The payload of the JSON data published for a scene entry are defined in
in the following JSON schema:

**JSON Schema:**

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "Scene Object",
  "description": "Scene object for the Scenes Table in the Scene Cluster",

  "type": "object",
  "properties": {
    "GroupID" : {
      "type": "number",
      "minimum": 0,
      "maximum": 65535
    },
    "SceneID" : {"type": "number"},
    "SceneName" : {"type": "string"},
    "TransitionTime" : {"type": "number"},
    "TransitionTime100ms" : {"type": "number"},
    "SceneTableExtensions": {
      "type": "object",
      "patternProperties": {
        "": { "type": "object" }
      }
    }
  },

  "required": ["GroupID", "SceneID", "SceneName", "TransitionTime", "TransitionTime100ms", "SceneTableExtensions"]

}
```

An example of a scene object is shown below.

**Payload:**

```json
{
  "GroupID": 12,
  "SceneID": 1,
  "SceneName": "Cozy Scene",
  "TransitionTime": 10,
  "TransitionTime100ms": 0,
  "SceneTableExtensions": {
    "OnOff": {
      "OnOff": true
    },
    "Level": {
      "CurrentLevel": 25
    },
    "ColorControl": {
      "CurrentX": 25,
      "CurrentY": 3455
    }
  }
}
```

#### Scenes Actuators

Scenes actuators do not support any of the ZCL required "SupportedCommands",
and instead use the Scene Cluster attributes to indicate which scene they
wish to activate. An MQTT client can therefore decide to activate scenes in a
number of devices/endpoints based on the spontaneous changes made by these
scenes actuator devices/endpoints.

For example, a wall switch with four buttons and three possible gestures for each button
may be presented as a scene device, supporting twelve read-only scenes that would
transition automatically (i.e., *CurrentScene* attribute will be updated in a
read-only fashion). The Scene table will stay empty and will not be writable
by MQTT Clients.

Note that the GroupID can be set to 0 if it is not used.

```mqtt
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/ClusterRevision/Desired {"value": 2}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/ClusterRevision/Reported {"value": 2}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/SceneCount/Desired {"value": 12}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/SceneCount/Reported {"value": 12}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/CurrentScene/Desired {"value": 2}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/CurrentScene/Reported {"value": 2}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/CurrentGroup/Desired {"value": 0}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/CurrentGroup/Reported {"value": 0}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/SceneValid/Desired {"value": true}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/SceneValid/Reported {"value": true}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/NameSupport/Desired {"value": false}
ucl/by-unid/zw-23485/ep0/Scenes/Attributes/NameSupport/Reported {"value": false}
ucl/by-unid/zw-23485/ep0/Scenes/SupportedCommands {"value":[ ] }
```

#### Scenes Supporting Nodes

Protocol Controller MUST advertise the Scene Table attribute for PAN nodes that
can be configured to react and change state based on scenes.

All PAN nodes/endpoints supporting attributes marked as *sceneRequired="true"*
in the [dotdot definitions](https://zigbeealliance.org/solution/dotdot/)
SHOULD be presented as a Scene cluster server by Protocol Controllers.

The Scene Extension table is described in the [ZCL2018](doc/references.md).

Each scene configuration can be done using the supported commands.
The "SceneTableExtensions" JSON object may contain all attributes of other
clusters marked as *sceneRequired="true"* in the
[dotdot definitions](https://zigbeealliance.org/solution/dotdot/).

For example, a properly formed *AddScene* command could be as follows:

**Topic:** `ucl/by_unid/&lt;UNID&gt;/ep<id>/Cluster/Scenes/AddScene`

**Payload:**

```json
{
  "GroupID": 0,
  "SceneID": 1,
  "TransitionTime": 1,
  "SceneName": "New Scene",
  "ExtensionFieldSets": {
    "OnOff": {
      "OnOff": true
    },
    "Level": {
      "CurrentLevel": 100
    }
  }
}
```

In this case, the "ExtensionFieldSets" data will be parsed and copied into the
"SceneTableExtensions" object of the Scene Table.

Only Cluster Attributes marked as `sceneRequired="true"` MUST be used by MQTT
Client in the `ExtensionFieldSets` JSON object.
Protocol Controllers MUST accept any CLuster/attribute combination in the
`ExtensionFieldSets` JSON object as long as they are marked as
`sceneRequired="true"`.

### On/Off Cluster (0x0006)

Mandatory Attributes:

* OnOff(0x0000); expected state values Off(0x00) and On(0x01).

Mandatory Commands:

* Off (0x00): when a device receive this command, the device should enter to "Off" state.
* On (0x01): when a device receive this command, the device should enter to "On" state.
* Toggle (0x02): when a device receive this command, the device should enter to "On" state if it was on " Off" state and vice versa.

### Level Cluster(0x0008)

The level cluster controls the light brightness level,
the door closure level, or the heater power output.

Mandatory attributes:

* CurrentLevel 0x0000 (accepted value range: MinLevel ... MaxLevel)
represents the current level of the device (the meaning of 'level' is device
dependent). Current Level is read only and cannot be updated using the
WriteAttributes command.

Some Optional attributes that define the minimum and maximum level supported by the device:

* MinLevel (0x0002): the minimum level the device supports.
* MaxLevel (0x0003): the maximum level the device supports.
* onLevel (0x0010): determines the value that the CurrentLevel attribute is set
to when the OnOff attribute is set to "On", as a result of processing an On/Off
cluster command. If the OnLevel attribute is not implemented or is set to 0xff,
the CurrentLevel value is not modified when the OnOff attribute of the OnOff
Cluster is modified.

Mandatory Commands:

* MoveToLevel (0x00): should contain a Level and TransitionTime
[in tenths of seconds] in the command JSON payload.
* Move (0x01): should contain a MoveMode (Up or Down) and the Rate (define the
rate of the movement in units per second) in the command JSON payload.
* Step (0x02): should contain StepMode (Up or Down), StepSize, and
TransitionTime in the command JSON payload.
* Stop (0x03): command that stops the level changes that was triggered by the
MoveToLevel, Move and Step commands.

Commands that will affect the On/Off Attribute of the device if On/Off Cluster
also supported:

* MoveToLevelWithOnOff (0x04)
* MovewithOnOff (0x05)
* StepWithOnOff (0x06)

Note that the the behavior of these commands is similar to the above mandatory commands. The main difference is the MoveToLevelWithOnOff, MovewithOnOff, and StepWithOnOff commands impact the On/Off Attribute of the device if On/Off Cluster is also supported. If the targeted level value is higher than the minimum level, the On/Off attribute should be set to "On" (light bulb). If the target level is set to the minimum level allowed by the device, the OnOff attribute should be set to "Off" (grey lightbulb).

### Door Lock Cluster (0x0101)

This cluster is used as a generic interface to secure a door. The ZCL Door Lock
cluster contains a lot of optional attributes, such as logging events,
alarms, security setting, and so on and commands, such as setting user PIN, getting
log events, setting schedules, and so on. However, this section lists only the mandatory
attributes and commands used for basic door operations.

Mandatory Attributes:

* LockState (0x0000): NotFullyLocked (0x00), Locked (0x01), Unlocked (0x02) and Undefined (0xFF) are the possible attribute values that describe the state of the door.
* LockType(0x0001): defines the door lock types such as Deadbolt (0x00), Magnetic (0x01), Other (0x02), Latch Bolt (0x05) and etc.
* ActuatorEnabled (0x002): defines if the actuator is Enables (true) or Disable (false).

Mandatory Request Commands (client to a device):

* LockDoor (0x00): a command used to lock a door with PINOrRFIDCode code.
* UnlockDoor( 0x01): a command used to unlock the door with PINOrRFIDCode Code.

Note if the PIN or RFID is not defined on the device, the PINOrRFIDCode field value can be empty string (i.e., " ").

### Thermostat Cluster (0x0201)

This cluster defines the interface to thermostats.
The ZCL Thermostat cluster contains a lot of optional attributes, such as Outdoor Temperature, Occupancy, PID configuration, and so on, and a few optional commands used primarily for scheduling.
However, this section only covers the (only) mandatory attribute, LocalTemperature, as well as the (only) mandatory command, Setpoint Raise/Lower.

> Note that for the ZCL Thermostat Cluster, all temperatures are in **Celcius (°C)** and any necessary conversion to/from, for example, Fahrenheit °F must be done by the Unify components (Protocol Controllers and GUI).

#### Fingerprinting of Z-Wave Nodes

For example, to map a Z-Wave node's functionalities to the Thermostat
Cluster, a protocol Controller may expect the following conditions to be met
before it exposes a Node as a Thermostat Cluster server:

* Node supports the Multilevel Sensor Command Class, version 1 or newer
  + Node supports the Multilevel sensor type 0x01 (Air Temperature)
* Node supports the Thermostat Setpoint Command Class, version 1 or newer
* Node supports the Thermostat Mode Command Class, version 1 or newer
  + Node supports at least one of the following modes: 0x01 (HEAT), 0x02 (COOL), 0x03 (AUTO).

#### Mandatory Attributes

The following attributes are mandatory for the ZigBee thermostat cluster.

* LocalTemperature (0x0000): The temperature in **Celsius (°C)** multiplied by 100 measured by the Thermostat. This gives a resolution of 0.01°C.
This is mapped from the Multilevel Sensor Report for the "Air temperature" sensor type.

* OccupiedCoolingSetpoint (0x0011) indicates the configured temperature setpoint for the thermostat when it operates in Cooling mode. This value is writable directly, which is supported only if the node supports 0x02 (COOL) or 0x03 (AUTO) modes.

* OccupiedHeatingSetpoint (0x0012) indicates the configured temperature setpoint for the thermostat when it operates in Heating mode. This value is writable directly, which is supported only if the node supports 0x01 (HEAT) or 0x03 (AUTO) modes.

* SystemMode (0x001C) indicates the current operating mode in the system. Note that all modes are not supported. Only 0 (Off) is supported by default. Other modes are supported only if the respective Z-Wave mode is supported (Auto, Cool and Heat).

* ControlSequenceOfOperation (0x001B): This indicates the supported SystemModes and restricts which ones can be used.
This attribute should be writable, but not all values are mandatory to support.
To start with, this attirbute will be mapped based on the supported modes in the Z-Wave Thermostat Mode. Only values 0x00, 0x02 or 0x04 will be used and the "With reheat" option will not be used. Since only 1 value is supported, it will appear as if this attribute is read-only.

Read only attributes:

* ThermostatProgrammingOperationMode (0x0025): This attribute is read only, showing the thermostats' capabilities. Z-Wave Thermostats will have SimpleOrSetpoint support to start with. When the Schedule Command Class is supported, the Schedule bit will be updated to 1.

Typically, the state topic for the Thermostat cluster will look like this:

```mqtt
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/ClusterRevision/Desired {"value": 2}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/ClusterRevision/Reported {"value": 2}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/LocalTemperature/Desired {"value": 2300}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/LocalTemperature/Reported {"value": 2300}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/OccupiedCoolingSetpoint/Desired {"value": 2600}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/OccupiedCoolingSetpoint/Reported {"value": 2600}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/OccupiedHeatingSetpoint/Desired {"value": 2100}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/OccupiedHeatingSetpoint/Reported {"value": 2100}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/SystemMode/Desired {"value": "Auto"}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/SystemMode/Reported {"value": "Auto"}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/ThermostatProgrammingOperationMode/Desired {"value": 0}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/ThermostatProgrammingOperationMode/Reported {"value": 0}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/ControlSequenceOfOperation/Desired {"value": 4}
ucl/by-unid/zw-23485/ep0/Thermostat/Attributes/ControlSequenceOfOperation/Reported {"value": 4}
```

#### Mandatory Commands

Only one command is mandatory command (alongside WriteAttributes), which allows
changing the operating mode and applying an offset to the currently-configured
setpoints.

* SetpointRaiseOrLower Command (0x00).

This command is used to change the Mode and adjust the setpoint(s).
For example, to change to heat mode and increase by 1 degree:

**Topic:** `ucl/by_unid/&lt;UNID&gt;/ep<id>/Thermostat/Commands/SetpointRaiseOrLower`

**Payload:**

```json
{
  "Mode": "Heat",
  "Amount": 100
}
```

The mode field can take the following values: "Heat", "Cool" or "Auto".

The Amount field in the SetpointRaiseOrLower will trigger a change in the setpoint.
This value is signed. OccupiedCoolingSetpoint and OccupiedHeatingSetpoint are
the two possible setpoints.

If the Mode field is set to Heat or Auto, the OccupiedHeatingSetpoint
(if supported) will be adjusted by the amount indicated in the Amount field.

If the Mode field is set to Cool or Auto, the OccupiedCoolingSetpoint
(if supported) will be adjusted by the amount indicated in the Amount field.

* WriteAttributes Command.

This command can be used to directly modify the writable attributes.

**Topic:** `ucl/by_unid/&lt;UNID&gt;/ep<id>/Thermostat/Commands/WriteAttributes`

**Payload:**

```json
{
  "SystemMode": "Auto",
  "OccupiedCoolingSetpoint": 3200,
  "OccupiedHeatingSetpoint": 2200
}
```

Note that it is not possible to detect which SystemModes are supported or if
the setpoints will be accepted. For example, in most cases the
`ControlSequenceOfOperation` will have only 1 valid value. In this case, the
cluster state topic will reflect back that no change was applied.

### Occupancy Sensing Cluster (0x0406)

The cluster defines an interface to occupancy sensing functionality including
configuration and getting the occupancy status. Here, only the mandatory
attributes that can enable the basic functionality of occupancy sensing are
described.

Mandatory Attributes:

* Occupancy (0x0000): is a bitmap and Bit 0 defines the sensed occupancy (Occupied = 1 and Unoccupied = 0)
* OccupancySensorType (0x0001): defines the occupancy sensor, and the values can be PIR (0x00), Ultrasonic(0x01), PIR and ultrasonic(0x02) and Physical contact(0x03).

There are no supported command for Occupancy Sensing cluster. The
*WriteAttributes* command will have no effect.

## Examples of Application Level Control Using Unify Framework

### Turning On the On/Off Light Device using Unify Framework

The following diagram shows an example of On/Off Cluster control using the
Unify framework.

\startuml

' Allows to do simultaneous transmissions
!pragma teoz true

' Style for the diagram
skinparam classFontColor black
skinparam classFontSize 10
skinparam classFontName Helvetica
skinparam sequenceMessageAlign center
skinparam shadowing false
skinparam ArrowColor #000000
skinparam ParticipantBackgroundColor #FFFFFF
skinparam ParticipantBorderColor #480509
skinparam SequenceLifeLineBorderColor #001111
skinparam SequenceLifeLineBorderThickness 2
skinparam NoteBackgroundColor #FFFFFF
skinparam NoteBorderColor #000000

hide footbox
title On Off node control example using the UCL / Unify Framework

legend top
<font color=#0039FB>MQTT Subscription</font>
<font color=#00003C>Retained MQTT Publication</font>
<font color=#6C2A0D>Unretained MQTT Publication</font>
endlegend

' List of participants
participant "IoT Service" as iot_service
participant "MQTT Broker" as mqtt_broker
participant "Protocol Controller" as protocol_controller
participant "PAN Node" as pan_node

iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/OnOff/Attributes/#</font>
iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/OnOff/SupportedCommands</font>
protocol_controller -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/OnOff/Commands/+</font>

rnote over iot_service, pan_node: Node inclusion and capabilities discovery

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Desired \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Reported \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/SupportedCommands \n<font color=#00003C><b>{"value":[ "On", "Off", "Toggle", "WriteAttributes" ]}</b>
& mqtt_broker -> iot_service

iot_service -> mqtt_broker
& mqtt_broker -> protocol_controller : <font color=#6C2A0D>ucl/by-unid/zw-1234/ep0/OnOff/Commands/On \n<font color=#6C2A0D><b>{ }</b>

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Desired \n<font color=#00003C><b>{ "value": true }</b>
& mqtt_broker -> iot_service

protocol_controller -> pan_node : PHY command
pan_node -> protocol_controller : PHY command

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Reported \n<font color=#00003C><b>{ "value": true }</b>
& mqtt_broker -> iot_service

\enduml

### Changing the Brightness Level of the Light Using Unify Framework

The following diagram shows an example of a device with both On/Off and Level
Clusters server.

\startuml

' Allows to do simultaneous transmissions
!pragma teoz true

' Style for the diagram
skinparam classFontColor black
skinparam classFontSize 10
skinparam classFontName Helvetica
skinparam sequenceMessageAlign center
skinparam shadowing false
skinparam ArrowColor #000000
skinparam ParticipantBackgroundColor #FFFFFF
skinparam ParticipantBorderColor #480509
skinparam SequenceLifeLineBorderColor #001111
skinparam SequenceLifeLineBorderThickness 2
skinparam NoteBackgroundColor #FFFFFF
skinparam NoteBorderColor #000000

hide footbox
title Dimmer node example using the UCL / Unify Framework

legend top
<font color=#0039FB>MQTT Subscription</font>
<font color=#00003C>Retained MQTT Publication</font>
<font color=#6C2A0D>Unretained MQTT Publication</font>
endlegend

' List of participants
participant "IoT Service" as iot_service
participant "MQTT Broker" as mqtt_broker
participant "Protocol Controller" as protocol_controller
participant "PAN Node" as pan_node

iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/OnOff/Attributes/#</font>
iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/OnOff/SupportedCommands</font>
iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Level/Attributes/#</font>
iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Level/SupportedCommands</font>

protocol_controller -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/OnOff/Commands/+</font>
protocol_controller -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Level/Commands/+</font>

rnote over iot_service, pan_node: Node inclusion and capabilities discovery

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Desired \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Reported \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/SupportedCommands \n<font color=#00003C><b>{"value":[ "On", "Off", "Toggle", "WriteAttributes" ]}</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/CurrentLevel/Desired \n<font color=#00003C><b>{ "value": 0 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Level/Attributes/CurrentLevel/Reported \n<font color=#00003C><b>{ "value": 0 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Level/SupportedCommands \n<font color=#00003C><b>{"value": [ "MoveToLevel", "Move", "Step", "Stop", "MoveToLevelWithOnOff", \n<font color=#00003C><b>"MoveWithOnOff", "StepWithOnOff", "StopWithOnOff", "WriteAttributes" ]}</b>
& mqtt_broker -> iot_service

== ==

iot_service -> mqtt_broker
& mqtt_broker -> protocol_controller : <font color=#6C2A0D>ucl/by-unid/zw-1234/ep0/Level/Commands/MoveToLevelWithOnOff \n<font color=#6C2A0D><b>{ "Level" : 50, "TransitionTime" : 50, "OptionsMask" : 0 , "OptionsOverride" : 0 }</b>

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Desired \n<font color=#00003C><b>{ "value": true }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Level/Attributes/CurrentLevel/Desired \n<font color=#00003C><b>{ "value": 50 }</b>
& mqtt_broker -> iot_service


protocol_controller -> pan_node : PHY command
pan_node -> protocol_controller : PHY command

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Reported \n<font color=#00003C><b>{ "value": true }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Level/Attributes/CurrentLevel/Reported \n<font color=#00003C><b>{ "value": 50 }</b>
& mqtt_broker -> iot_service

\enduml

### Basic DoorLock Operation Using Unify Framework

The following diagram shows an example of a Door Lock controlled using the
Unify Framework.

\startuml

' Allows to do simultaneous transmissions
!pragma teoz true

' Style for the diagram
skinparam classFontColor black
skinparam classFontSize 10
skinparam classFontName Helvetica
skinparam sequenceMessageAlign center
skinparam shadowing false
skinparam ArrowColor #000000
skinparam ParticipantBackgroundColor #FFFFFF
skinparam ParticipantBorderColor #480509
skinparam SequenceLifeLineBorderColor #001111
skinparam SequenceLifeLineBorderThickness 2
skinparam NoteBackgroundColor #FFFFFF
skinparam NoteBorderColor #000000

hide footbox
title Door Lock node control example using the UCL / Unify Framework

legend top
<font color=#0039FB>MQTT Subscription</font>
<font color=#00003C>Retained MQTT Publication</font>
<font color=#6C2A0D>Unretained MQTT Publication</font>
endlegend

' List of participants
participant "IoT Service" as iot_service
participant "MQTT Broker" as mqtt_broker
participant "Protocol Controller" as protocol_controller
participant "PAN Node" as pan_node

iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/DoorLock/Attributes/#</font>
iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/DoorLock/SupportedCommands</font>
protocol_controller -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/DoorLock/Commands/+</font>

rnote over iot_service, pan_node: Node inclusion and capabilities discovery

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/DoorLock/Attributes/LockType/Desired \n<font color=#00003C><b>{ "value": "LatchBolt" }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/DoorLock/Attributes/LockType/Reported \n<font color=#00003C><b>{ "value": "LatchBolt" }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/DoorLock/Attributes/LockState/Desired \n<font color=#00003C><b>{ "value": "Unlocked" }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/DoorLock/Attributes/LockState/Reported \n<font color=#00003C><b>{ "value": "Unlocked" }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/DoorLock/Attributes/ActuatorEnabled/Desired \n<font color=#00003C><b>{ "value": true }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/DoorLock/Attributes/ActuatorEnabled/Reported \n<font color=#00003C><b>{ "value": true }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/DoorLock/SupportedCommands \n<font color=#00003C><b>{"value":[ "LockDoor", "UnlockDoor", "WriteAttributes" ]}</b>
& mqtt_broker -> iot_service

iot_service -> mqtt_broker
& mqtt_broker -> protocol_controller : <font color=#6C2A0D>ucl/by-unid/zw-1234/ep0/DoorLock/Commands/LockDoor \n<font color=#6C2A0D><b>{ "PINOrRFIDCode": "1234" }</b>

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/DoorLock/Attributes/LockState/Desired \n<font color=#00003C><b>{ "value": "Locked" }</b>
& mqtt_broker -> iot_service

protocol_controller -> pan_node : PHY command
pan_node -> protocol_controller : PHY command

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/DoorLock/Attributes/LockState/Reported \n<font color=#00003C><b>{ "value": "Locked" }</b>
& mqtt_broker -> iot_service

\enduml

### PIR Sensor Notification Application Using Unify Framework

The following diagram shows an example of PIR Sensor device
represented as an OccupancySensing Cluster server with the Unify framework.

\startuml

' Allows to do simultaneous transmissions
!pragma teoz true

' Style for the diagram
skinparam classFontColor black
skinparam classFontSize 10
skinparam classFontName Helvetica
skinparam sequenceMessageAlign center
skinparam shadowing false
skinparam ArrowColor #000000
skinparam ParticipantBackgroundColor #FFFFFF
skinparam ParticipantBorderColor #480509
skinparam SequenceLifeLineBorderColor #001111
skinparam SequenceLifeLineBorderThickness 2
skinparam NoteBackgroundColor #FFFFFF
skinparam NoteBorderColor #000000

hide footbox
title Occupancy Sensing example using the UCL / Unify Framework

legend top
<font color=#0039FB>MQTT Subscription</font>
<font color=#00003C>Retained MQTT Publication</font>
<font color=#6C2A0D>Unretained MQTT Publication</font>
endlegend

' List of participants
participant "IoT Service" as iot_service
participant "MQTT Broker" as mqtt_broker
participant "Protocol Controller" as protocol_controller
participant "PAN Node" as pan_node

iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/OccupancySensing/Attributes/#</font>
iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/OccupancySensing/SupportedCommands</font>
protocol_controller -> mqtt_broker: <b>Optional</b> (<font color=#0039FB>ucl/by-unid/+/+/OccupancySensing/Commands/+</font>)

rnote over iot_service, pan_node: Node inclusion and capabilities discovery

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OccupancySensing/Attributes/Occupancy/Desired \n<font color=#00003C><b>{ "value": {SensedOccupancy : false} }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OccupancySensing/Attributes/Occupancy/Reported \n<font color=#00003C><b>{ "value": {SensedOccupancy : false} }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OccupancySensing/Attributes/OccupancySensorType/Desired \n<font color=#00003C><b>{ "value": "PIR" }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OccupancySensing/Attributes/OccupancySensorType/Reported \n<font color=#00003C><b>{ "value": "PIR" }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OccupancySensing/SupportedCommands \n<font color=#00003C><b>{"value":[ ]}</b>
& mqtt_broker -> iot_service

== ==

pan_node -> protocol_controller : PHY Specific \nPIR occupancy \ndetected

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OccupancySensing/Attributes/Occupancy/Desired \n<font color=#00003C><b>{ "value": {SensedOccupancy : true} }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OccupancySensing/Attributes/Occupancy/Reported \n<font color=#00003C><b>{ "value": {SensedOccupancy : true} }</b>
& mqtt_broker -> iot_service

== ==

pan_node -> protocol_controller : PHY Specific \nPIR occupancy \nnot detected

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OccupancySensing/Attributes/Occupancy/Desired \n<font color=#00003C><b>{ "value": {SensedOccupancy : false} }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OccupancySensing/Attributes/Occupancy/Reported \n<font color=#00003C><b>{ "value": {SensedOccupancy : false} }</b>
& mqtt_broker -> iot_service

\enduml

### Using the WriteAttributes Generic Topics

Scenario: A device supports the `ManufacturerName` and `LocationDescription`
attributes in the Basic Cluster. An MQTT Client wants to update the location of
the device using the *WriteAttributes* Command.

Note that the `ManufacturerName` is a read only attribute
and the `LocationDescription` is a writable attribute.

\startuml

' Allows to do simultaneous transmissions
!pragma teoz true

' Style for the diagram
skinparam classFontColor black
skinparam classFontSize 10
skinparam classFontName Helvetica
skinparam sequenceMessageAlign center
skinparam shadowing false
skinparam ArrowColor #000000
skinparam ParticipantBackgroundColor #FFFFFF
skinparam ParticipantBorderColor #480509
skinparam SequenceLifeLineBorderColor #001111
skinparam SequenceLifeLineBorderThickness 2
skinparam NoteBackgroundColor #FFFFFF
skinparam NoteBorderColor #000000

hide footbox
title Writing to attributes with the WriteAttributes Command.

legend top
<font color=#0039FB>MQTT Subscription</font>
<font color=#00003C>Retained MQTT Publication</font>
<font color=#6C2A0D>Unretained MQTT Publication</font>
endlegend

' List of participants
participant "IoT Service" as iot_service
participant "MQTT Broker" as mqtt_broker
participant "Protocol Controller" as protocol_controller
participant "PAN Node" as pan_node

iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Basic/Attributes/#</font>
iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Basic/SupportedCommands</font>
protocol_controller -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Basic/Commands/+</font>

== ==

iot_service -> mqtt_broker
& mqtt_broker -> protocol_controller : <font color=#6C2A0D>ucl/by-unid/zw-1234/ep0/Basic/Commands/WriteAttributes \n<font color=#6C2A0D><b>{ "LocationDescription": "Bathroom", "ManufacturerName" : "Silabs" }</b>

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Basic/Attributes/LocationDescription/Desired \n<font color=#00003C><b>{ "value": "Bathroom" }</b>
& mqtt_broker -> iot_service

protocol_controller -> pan_node : PHY command
pan_node -> protocol_controller : PHY command

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Basic/Attributes/LocationDescription/Reported \n<font color=#00003C><b>{ "value": "Bathroom" }</b>
& mqtt_broker -> iot_service

\enduml

### On/Off and Scene-Capable PAN Node

The following diagram shows an example of On/Off and Scene Cluster control
using the Unify framework.

\startuml

' Allows to do simultaneous transmissions
!pragma teoz true

' Style for the diagram
skinparam classFontColor black
skinparam classFontSize 10
skinparam classFontName Helvetica
skinparam sequenceMessageAlign center
skinparam shadowing false
skinparam ArrowColor #000000
skinparam ParticipantBackgroundColor #FFFFFF
skinparam ParticipantBorderColor #480509
skinparam SequenceLifeLineBorderColor #001111
skinparam SequenceLifeLineBorderThickness 2
skinparam NoteBackgroundColor #FFFFFF
skinparam NoteBorderColor #000000

hide footbox
title Scene and On/Off node control example using the UCL / Unify Framework

legend top
<font color=#0039FB>MQTT Subscription</font>
<font color=#00003C>Retained MQTT Publication</font>
<font color=#6C2A0D>Unretained MQTT Publication</font>
endlegend

' List of participants
participant "IoT Service" as iot_service
participant "MQTT Broker" as mqtt_broker
participant "Protocol Controller" as protocol_controller
participant "PAN Node" as pan_node

iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/OnOff/Attributes/#</font>
iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/OnOff/SupportedCommands</font>
iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Scenes/Attributes/#</font>
iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Scenes/SupportedCommands</font>
protocol_controller -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/OnOff/Commands/+</font>
protocol_controller -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Scenes/Commands/+</font>

rnote over iot_service, pan_node: Node inclusion and capabilities discovery

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Desired \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Reported \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/SupportedCommands \n<font color=#00003C><b>{"value":[ "On", "Off", "Toggle", "WriteAttributes" ]}</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/SceneCount/Desired \n<font color=#00003C><b>{ "value": 0 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/SceneCount/Reported \n<font color=#00003C><b>{ "value": 0 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentScene/Desired \n<font color=#00003C><b>{ "value": 0 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentScene/Reported \n<font color=#00003C><b>{ "value": 0 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentGroup/Desired \n<font color=#00003C><b>{ "value": 0 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentGroup/Reported \n<font color=#00003C><b>{ "value": 0 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/SceneValid/Desired \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/SceneValid/Reported \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/NameSupport/Desired \n<font color=#00003C><b>{ "value": true }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/NameSupport/Reported \n<font color=#00003C><b>{ "value": true }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/SupportedCommands \n<font color=#00003C><b>{"value":["AddScene", "ViewScene", "RemoveScene", "RemoveAllScenes", "StoreScene", \n<font color=#00003C><b>"RecallScene", "GetSceneMembership", "WriteAttributes"]}</b>
& mqtt_broker -> iot_service

iot_service -> mqtt_broker
& mqtt_broker -> protocol_controller : <font color=#6C2A0D>ucl/by-unid/zw-1234/ep0/Scenes/Commands/StoreScene \n<font color=#6C2A0D><b>{ "GroupID" : 0, "SceneID": 1}</b>

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/SceneTable/1/Desired \n<font color=#00003C><b>{  "GroupID": 0, "SceneID": 1, "SceneName": "", "TransitionTime": 0,\n<font color=#00003C><b> "TransitionTime100ms": 0, "SceneTableExtensions": { "OnOff": { "OnOff": false } } }</b>
& mqtt_broker -> iot_service

protocol_controller -> pan_node : PHY command
pan_node -> protocol_controller : PHY command

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/SceneTable/1/Reported \n<font color=#00003C><b>{  "GroupID": 0, "SceneID": 1, "SceneName": "", "TransitionTime": 0,\n<font color=#00003C><b> "TransitionTime100ms": 0, "SceneTableExtensions": { "OnOff": { "OnOff": false } } }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentScene/Reported \n<font color=#00003C><b>{ "value": 1 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentScene/Desired \n<font color=#00003C><b>{ "value": 1 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/SceneValid/Reported \n<font color=#00003C><b>{ "value": true }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/SceneValid/Desired \n<font color=#00003C><b>{ "value": true }</b>
& mqtt_broker -> iot_service

iot_service -> mqtt_broker
& mqtt_broker -> protocol_controller : <font color=#6C2A0D>ucl/by-unid/zw-1234/ep0/OnOff/Commands/On \n<font color=#6C2A0D><b>{ }</b>

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Desired \n<font color=#00003C><b>{ "value": true }</b>
& mqtt_broker -> iot_service

protocol_controller -> pan_node : PHY command
pan_node -> protocol_controller : PHY command

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Reported \n<font color=#00003C><b>{ "value": true }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/SceneValid/Reported \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/SceneValid/Desired \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

iot_service -> mqtt_broker
& mqtt_broker -> protocol_controller : <font color=#6C2A0D>ucl/by-unid/zw-1234/ep0/Scenes/Commands/RecallScene \n<font color=#6C2A0D><b>{ "GroupID" : 0, "SceneID": 1, "TransitionTime": 25}</b>

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/SceneValid/Desired \n<font color=#00003C><b>{ "value": true }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Desired \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> pan_node : PHY command
pan_node -> protocol_controller : PHY command

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/SceneValid/Reported \n<font color=#00003C><b>{ "value": true }</b>
& mqtt_broker -> iot_service

== Wait for TransitionTime (2.5 seconds) ==

pan_node -> protocol_controller : PHY command

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Reported \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

\enduml

### Scene PAN Node without Actuator Capabilities

The following diagram shows an example of a node using scenes without
supporting any of the actuator functionalities.

\startuml

' Allows to do simultaneous transmissions
!pragma teoz true

' Style for the diagram
skinparam classFontColor black
skinparam classFontSize 10
skinparam classFontName Helvetica
skinparam sequenceMessageAlign center
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
title Scene only PAN node example using the UCL / Unify Framework

legend top
<font color=#0039FB>MQTT Subscription</font>
<font color=#00003C>Retained MQTT Publication</font>
<font color=#6C2A0D>Unretained MQTT Publication</font>
endlegend

' List of participants
participant "IoT Service" as iot_service
participant "MQTT Broker" as mqtt_broker
participant "Protocol Controller" as protocol_controller
participant "PAN Node" as pan_node
Actor "User" as end_user

iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Scenes/Attributes/#</font>
iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Scenes/SupportedCommands</font>
protocol_controller -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Scenes/Commands/+</font>

rnote over iot_service, pan_node: Node inclusion and capabilities discovery

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/SceneCount/Desired \n<font color=#00003C><b>{ "value": 12 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/SceneCount/Reported \n<font color=#00003C><b>{ "value": 12 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentScene/Desired \n<font color=#00003C><b>{ "value": 0 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentScene/Reported \n<font color=#00003C><b>{ "value": 0 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentGroup/Desired \n<font color=#00003C><b>{ "value": 0 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentGroup/Reported \n<font color=#00003C><b>{ "value": 0 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/SceneValid/Desired \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/SceneValid/Reported \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/NameSupport/Desired \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/NameSupport/Reported \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/SupportedCommands \n<font color=#00003C><b>{"value":[ ]}
& mqtt_broker -> iot_service

end_user -> pan_node : Activate scene 5 button
pan_node -> protocol_controller : PHY command

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentScene/Desired \n<font color=#00003C><b>{ "value": 5 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentScene/Reported \n<font color=#00003C><b>{ "value": 5 }</b>
& mqtt_broker -> iot_service

== ==

end_user -> pan_node : Activate scene 2 button
pan_node -> protocol_controller : PHY command

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentScene/Desired \n<font color=#00003C><b>{ "value": 2 }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentScene/Reported \n<font color=#00003C><b>{ "value": 2 }</b>
& mqtt_broker -> iot_service


\enduml


### Manipulating PAN Node Group Membership

The following diagram shows an example of adding/removing groups for a PAN
node

\startuml

' Allows to do simultaneous transmissions
!pragma teoz true

' Style for the diagram
skinparam classFontColor black
skinparam classFontSize 10
skinparam classFontName Helvetica
skinparam sequenceMessageAlign center
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
title Group membership manipulation with the Group Cluster using the UCL / Unify Framework

legend top
<font color=#0039FB>MQTT Subscription</font>
<font color=#00003C>Retained MQTT Publication</font>
<font color=#6C2A0D>Unretained MQTT Publication</font>
endlegend


' List of participants
participant "IoT Service" as iot_service
participant "MQTT Broker" as mqtt_broker
participant "Protocol Controller" as protocol_controller
participant "PAN Node" as pan_node

iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Groups/Attributes/#</font>
iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Groups/SupportedCommands</font>
protocol_controller -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Groups/Commands/+</font>

rnote over iot_service, pan_node: Node inclusion and capabilities discovery

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Groups/Attributes/NameSupport/Desired \n<font color=#00003C><b>{ "value": {"Supported":true} }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Groups/Attributes/NameSupport/Reported \n<font color=#00003C><b>{ "value": {"Supported":true} }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Groups/Attributes/GroupList/Desired \n<font color=#00003C><b>{"value":[]}</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Groups/Attributes/GroupList/Reported \n<font color=#00003C><b>{"value":[]}</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Groups/SupportedCommands \n<font color=#00003C><b>{"value": ["AddGroup", "ViewGroup", "GetGroupMembership", \n<font color=#00003C><b>"RemoveGroup", "RemoveAllGroups", "AddGroupIfIdentifying"]}</b>
& mqtt_broker -> iot_service

== ==

iot_service -> mqtt_broker
& mqtt_broker -> protocol_controller : <font color=#6C2A0D>ucl/by-unid/zw-1234/ep0/Groups/Commands/AddGroup \n<font color=#6C2A0D><b>{ "GroupId" : 1, "GroupName": "Living room"}</b>

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Groups/Attributes/GroupList/Desired \n<font color=#00003C><b>{"value":[1]}</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Groups/Attributes/1/Name/Desired \n<font color=#00003C><b>{ "value": "Living room"}</b>
& mqtt_broker -> iot_service

pan_node <-> protocol_controller : PHY commands

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Groups/Attributes/GroupList/Reported \n<font color=#00003C><b>{"value":[1]}</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Groups/Attributes/1/Name/Reported \n<font color=#00003C><b>{ "value": "Living room"}</b>
& mqtt_broker -> iot_service

== ==
iot_service -> mqtt_broker
& mqtt_broker -> protocol_controller : <font color=#6C2A0D>ucl/by-unid/zw-1234/ep0/Groups/Commands/RemoveGroup \n<font color=#6C2A0D><b>{ "GroupId" : 1}</b>

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Groups/Attributes/GroupList/Desired \n<font color=#00003C><b>{"value":[]}</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Groups/Attributes/1/Name/Desired \n<font color=#00003C>
& mqtt_broker -> iot_service

pan_node <-> protocol_controller : PHY commands

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Groups/Attributes/GroupList/Reported \n<font color=#00003C><b>{"value":[]}</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Groups/Attributes/1/Name/Reported \n<font color=#00003C>
& mqtt_broker -> iot_service


\enduml
