# Service Discovery

## Concept

All MQTT Clients (possibly including Protocol Controllers) must be able to find out the existing devices and their capabilities to provide functionalities. Using MQTT subscriptions as a service discovery, each client can receive information about what they are interested in given that they have a starting point.

### MQTT Topics

Some high-level functionalities will be expected to be present at the MQTT broker from the beginning, as follows:

* ucl/by-unid/#
* ucl/by-location/#
* ucl/by-type/#
* ucl/by-group/#

More than this can be appended to the initial list in future versions.
An MQTT client MAY subscribe to the 'ucl/#' topic filter and get all published
messages by the MQTT broker. However, this is not optimal from a traffic
point of view. MQTT Clients SHOULD subscribe to the minimum they need, while
still ensuring that they do not miss anything.

The minimum requirement to discover nodes and their capabilities is a single
subscription to <em>ucl/by-unid/#</em>. If the resources are available, the
discovery will happen via the publication of new MQTT messages, which will show
that devices have been added to the network and their associated capabilities.

### Discovery of the Protocol Controllers

Protocol Controllers will start by advertising themselves as a resource by
publishing to <em>ucl/by-unid/&lt;their-unid&gt;/ProtocolController/NetworkManagement</em>.
They will be able to issue a payload indicating their current network management
state and what they can do to set up a network.
For more details, see
[Network Management](doc/Chapter03-network-management.md).

An IoT Service can therefore find out more about Protocol Controllers in
general by subscribing to <em>ucl/by-unid/+/ProtocolController/+</em>.

### Discovery Done by Protocol Controllers

The Protocol Controllers are devices that are capable to add/remove devices
in their PAN and therefore publish about their identity and capabilities.
For example, after a network inclusion, a Protocol Controller will perform the
necessary node interview, which will allow it to publish about the newly added
PAN node. It is expected that the Protocol Controller assign a unique identifier
to the node and start by publishing using the assigned UNID.

All nodes will have a generic state topic, where general information is
published about the node. The topic is <em>ucl/by-unid/&lt;unid&gt;/State</em>.

The state topic is a JSON object that MUST follow the following JSON Schema.

> FIXME: Here we need the other PHYs/protocols to fill up the Security enum.

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "UNID State",
  "description": "State of a PAN Node",
  "type": "object",
  "properties": {
    "NetworkStatus": {
      "type": "string",
      "enum": [
        "Online functional",
        "Online interviewing",
        "Online non-functional",
        "Unavailable",
        "Offline"
      ]
    },
    "Security": {
      "type": "string",
      "enum": [
        "None",
        "Z-Wave S0",
        "Z-Wave S2 Unauthenticated",
        "Z-Wave S2 Authenticated",
        "Z-Wave S2 Access Control"
      ]
    },
    "MaximumCommandDelay": {
      "type": [
        "number",
        "string"
      ],
      "pattern": "^(unknown)|(infinite)$"
    }
  },
  "required": [
    "NetworkStatus",
    "Security",
    "MaximumCommandDelay"
  ]
}
```

The properties in the JSON object of the state topic is shown below:

<table>
<caption>MQTT topic properties for nodes general state information</caption>
<tr>
<th>Property name</th>
<th>Description</th>
<th>Example value</th>
</tr>
<tr>
<td>NetworkStatus</td>
<td>Indicated if the node is part of the network or available in general.
This should include intermediate states to give progress feedback during inclusion.
More details about each value is provided in the NetworkStatus subsection</td>
<td>"Online functional"<br/>"Online interviewing"<br/>"Online non-functional"<br/>"Unavailable"<br/>"Offline"</td>
</tr>
<tr>
<td>Security</td>
<td>Indicates if the node uses encryption for communication on the PAN</td>
<td>"Z-Wave S2 Authenticated"</td>
</tr>
<tr>
<td>MaximumCommandDelay</td>
<td>Indicates the maximum delay that it can take to deliver a command to the
node, in seconds
It can either be deterministic (with a precise value), "unknown", or "infinite".
It can therefore either be a JSON number or JSON String.</td>
<td>4200<br/>"unknown"<br/>"infinite"</td>
</tr>
</table>

So for example, when a new node joined the network and interview/setup is
completed, the Protocol Controller MUST publish:

> Topic: <em>ucl/by-unid/984540640/State</em><br/>
> Payload:<br/>
> ```json
> {
>   "NetworkStatus": "Online functional",
>   "Security": "Z-Wave S0",
>   "MaximumCommandDelay": 4200
> }
> ```

This will indicate that the node is now operational. It has been securely
included using maximum requested security. For a Wake Up /
sleeping device, there may be a delay before a given command is issued to the
node before a publish back confirms that the application state has changed.
For always listening/reachable nodes, "MaximumCommandDelay" can be set to 0.

A Protocol Controller managing several networks with separate network IDs
may expose the network IDs as an optional parameter in the State topic.
The UNIDs assigned to nodes from each network must be unique.

The network state is shared for all endpoints of a PAN node.

However, an IoT Service still does not know what the node's capabilities are.
The Protocol Controller is in charge of figuring it out and publish on topics
representing the Endpoints and their ZigBee server clusters.
After they are discovered, the Protocol Controller MUST publish in a
sub-topic under the node's unid/endpoint the supported commands and attributes
state of a given cluster.

> Topic: <em>ucl/by-unid/+/+/&lt;ClusterName&gt;/Attributes/&lt;AttributeName&gt;/Reported</em><br/>
> Topic: <em>ucl/by-unid/+/+/&lt;ClusterName&gt;/Attributes/&lt;AttributeName&gt;/Desired</em><br/>
> Topic: <em>ucl/by-unid/+/+/&lt;ClusterName&gt;/SupportedCommands</em><br/>

It means that when a new node joined the network and interview/setup is
completed, the Protocol Controller figures out the list of supported attributes,
their values, and the list of supported commands that it publishes back under
each endpoint. For example, for a multi-endpoint dimming node (not including all
mandatory attributes here for readability:

> 1. Publish
>>   Topic: <em>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Desired</em><br/>
>>   Payload: `{ "value" : true }`
> 2. Publish
>>   Topic: <em>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Reported</em><br/>
>>   Payload: `{ "value" : true }`
> 3. Publish
>>   Topic: <em>ucl/by-unid/zw-1234/ep0/OnOff/SupportedCommands</em><br/>
>>   Payload: `{ "value" : [ "On", "Off", "Toggle", "WriteAttributes"]}`
> 4. Publish
>>   Topic: <em>ucl/by-unid/zw-1234/ep1/OnOff/Attributes/OnOff/Desired</em><br/>
>>   Payload: `{ "value" : false }`
> 5. Publish
>>   Topic: <em>ucl/by-unid/zw-1234/ep1/OnOff/Attributes/OnOff/Reported</em><br/>
>>   Payload: `{ "value" : true }`
> 6. Publish
>>   Topic: <em>ucl/by-unid/zw-1234/ep1/OnOff/SupportedCommands</em><br/>
>>   Payload: `{ "value" : [ "On", "Off", "Toggle", "WriteAttributes"]}`
> 7. Publish
>>   Topic: <em>ucl/by-unid/zw-1234/ep2/Level/Attributes/CurrentLevel/Desired</em><br/>
>>   Payload: `{ "value" : 100 }`
> 8. Publish
>>   Topic: <em>ucl/by-unid/zw-1234/ep2/Level/Attributes/OnOff/Reported</em><br/>
>>   Payload: `{ "value" : 100 }`
> 9. Publish
>>   Topic: <em>ucl/by-unid/zw-1234/ep2/OnOff/SupportedCommands</em><br/>
>>   Payload: `{ "value" :[ "MoveToLevel", "Move", "Step", "Stop", "WriteAttributes"]}`


To fit into the proper MQTT topic hierarchy, commands names are stripped from their "/" and whitespace characters.

To receive the supported commands (used by IoT Service to control end nodes),
the Protocol Controller will subscribe to the topics according to what the node
capabilities are, so that it can receive the commands from other MQTT clients:

> subscribe: <em>ucl/by-unid/984540640/ep0/OnOff/Commands/WriteAttributes</em><br/>
> subscribe: <em>ucl/by-unid/984540640/ep0/OnOff/Commands/On</em><br/>
> subscribe: <em>ucl/by-unid/984540640/ep0/OnOff/Commands/Off</em><br/>
> subscribe: <em>ucl/by-unid/984540640/ep0/OnOff/Commands/Toggle</em><br/>
> subscribe: <em>ucl/by-unid/984540640/ep0/Level/Commands/WriteAttributes</em><br/>
> subscribe: <em>ucl/by-unid/984540640/ep0/Level/Commands/MoveToLevel</em><br/>
> etc.
> Alternatively, subscribe to the topic filters:
> subscribe: <em>ucl/by-unid/984540640/ep0/OnOff/Commands/+</em><br/>
> subscribe: <em>ucl/by-unid/984540640/ep0/Level/Commands/+</em><br/>

The following generic command will always be available for all clusters.

> Topic: <em>ucl/by-unid/+/+/&lt;ClusterName&gt;/Commands/WriteAttributes</em><br/>
> Payload:
> ```json
> {
>   "AttributeName1": "AttributeValue1",
>   "AttributeName2": "AttributeValue2"
> }
> ```

If none of the attributes in a cluster are writable (i.e., they are all read-only),
this command will not have any effect.

Commands that cannot be translated by the Protocol Controller translator module
(e.g., Z-Wave Commands that are not translated into any ZigBee Cluster) will
not be advertised as capabilities as it is required that the commands are
translated correctly before delivery to the end node.

Commands will be mapped as topics under each cluster. When an MQTT Client issues
a command that will modify one or more attributes in a cluster, the Protocol
Controller will publish back the list of attributes and their values after it
made sure that the attributes values were correctly changed. For protocols
like Z-Wave, it will include the translator to add Supervision Encapsulation
or issue a subsequent Get Command in order to verify the state/value of each
attribute.

#### NetworkStatus

The NetworkStatus property of the UNID state topic JSON object can take the
following values.

<table>
<caption>NetworkStatus property value</caption>
<tr>
<th>Value</th>
<th>Description</th>
</tr>
<tr>
<td>"Online interviewing"</td>
<td>The node has joined the network but it is not ready to be operational yet because
an initial setup and interview need to be done before it can be controlled.</td>
</tr>
<tr>
<td>"Online functional"</td>
<td>The node has joined the network, all required setup is functional, and the
node capabilities are known. IoT Services can fully operate the node.</td>
</tr>
<tr>
<td>"Online non-functional"</td>
<td>The node has joined the network and the Protocol Controller can communicate
with it, but important setup failed and therefore its capabilities may
be degraded. It can nonetheless be controlled, but fewer capabilities than
expected may be exposed.</td>
</tr>
<tr>
<td>"Unavailable"</td>
<td>The node has joined the network and is most likely "Online functional" but the
Protocol Controller cannot service any command or state information at the moment.
This can happen, for example if the Protocol Controller reboots or is offline.</td>
</tr>
<tr>
<td>"Offline"</td>
<td>The node has joined the network but cannot be reached by the Protocol Controller
within expected delays. The node has a connectivity issue.</td>
</tr>
</table>

## MQTT Client Controlling Examples

### OnOff Cluster Control

An IoT Service interested only in OnOff Cluster control will subscribe to
the following two topic filters:
* <em>ucl/by-unid/+/+/OnOff/Attributes/#</em>.
* <em>ucl/by-unid/+/+/OnOff/SupportedCommands</em>.

When an IoT Service receives publish messages, such as
<em>ucl/by-unid/98146584/ep10/OnOff/SupportedCommands</em>, it will then know that a
Protocol Controller can service commands for the the OnOff Cluster for
unid 98146584, endpoint 10.
The MQTT Client can issue the cluster commands (e.g On) to the corresponding
subtopic: <em>ucl/by-unid/98146584/ep10/OnOff/Commands/On</em>.

When an IoT Service publishes a command such as
<em>ucl/by-unid/98146584/ep10/OnOff/Commands/On</em>, the Protocol Controller
will translate the command to one or more commands specific to its PHY to achieve
the desired effect. When the operation is completed, the Protocol Controller
will publish the updated state of the attributes for the cluster server.
In other words, attributes updates within the OnOff Cluster can be used as a confirmation
that the command was executed (or not) and also to tell other MQTT clients
that the state of the end device has changed.

For example:
> Publish
>   Topic: <em>ucl/by-unid/98146584/ep10/OnOff/Attributes/On/Reported</em><br/>
>   Payload: `{"value": true}`

### Delay in Command Execution

If an IoT Service wants to display accurate state information, it needs to
subscribe to the <em>ucl/by-unid/+/State</em> topic.
This topic contains information that may affect the way they can expect to
interact with the end device, such as the <em>MaximumCommandDelay</em> parameter.
This value indicates how long they have to expect a publish message back showing
the effect of their command in the worst case.

The Maximum Command Delay is identical for all endpoints under an UNID. If a
Protocol Controller has several endpoints that have a different response delay,
this value MUST advertise the worst case scenario.

Every time a desired state or reported state is updated for a given end node,
the Protocol Controller servicing the end node will publish on the State topic.
For example:

> 1. MQTT Client Publishes
>>   Topic: <em>ucl/by-unid/98146584/ep10/OnOff/Commands/On</em><br/>
>>   Payload: `{ }`
> 2. The servicing Protocol Controller acknowledges the request and publishes:
>>   Topic: <em>ucl/by-unid/98146584/ep10/OnOff/Attributes/On/Desired</em><br/>
>>   Payload: `{ "value": true }`
> 3. When the protocol controller has confirmed that the OnOff device is turned
> on, it publishes again the state information:
>>   Topic: <em>ucl/by-unid/98146584/ep10/OnOff/Attributes/On/Reported</em><br/>
>>   Payload: `{ "value": true }`

A more detailed example of this mechanism is shown in the diagrams in
[Chapter 2 - ZCL in Unify](doc/Chapter02-ZCL-in-uic.md).

### Unresponsive Node

A Protocol Controller will publish using the node state topic when it finds
out that a node is failing and may have left the network. It may not
constantly monitor nodes' link and it may figure out an unexpected delay when
an MQTT Client publishes to the following:

> Topic: <em>ucl/by-unid/984540640/ep3/Identify/Commands/Identify</em><br/>
> Payload: ```{ "IdentifyTime":"0x003C" }```

The Protocol Controller will try to issue the commands and realize that the
node is not responsive. In this case, the Protocol Controller will also publish
back on the node state (instead of usually the cluster state).

> Topic: <em>ucl/by-unid/984540640/State</em><br/>
> Payload:
> ```json
> {
>   "NetworkStatus": "Offline",
>   "Security": "Bluetooth passkey",
>   "MaximumCommandDelay": 0
> }
> ```

If the Protocol Controller needs to advertise that a PAN node is no longer
part of the network, it MUST  publish a zero byte payload with retain flag = 1
at the state and all cluster subtopics belonging to the node:

> Topic: <em>ucl/by-unid/984540640/ep0/OnOff/Attributes/OnOff/Reported</em> - No payload<br/>
> Topic: <em>ucl/by-unid/984540640/ep0/OnOff/Attributes/OnOff/Desired</em> - No payload<br/>
> Topic: <em>ucl/by-unid/984540640/ep0/OnOff/SupportedCommands</em> - No payload<br/>
> Topic: <em>ucl/by-unid/984540640/ep0/Level/Attributes/CurrentLevel/Reported</em> - No payload<br/>
> Topic: <em>ucl/by-unid/984540640/ep0/Level/Attributes/CurrentLevel/Desired</em> - No payload<br/>
> Topic: <em>ucl/by-unid/984540640/ep0/Level/SupportedCommands</em> - No payload<br/>
> Topic: <em>ucl/by-unid/984540640/ep1/OnOff/Attributes/OnOff/Reported</em> - No payload<br/>
> Topic: <em>ucl/by-unid/984540640/ep1/OnOff/Attributes/OnOff/Desired</em> - No payload<br/>
> Topic: <em>ucl/by-unid/984540640/ep1/OnOff/SupportedCommands</em> - No payload<br/>
> Topic: <em>ucl/by-unid/984540640/ep1/Level/Attributes/CurrentLevel/Reported</em> - No payload<br/>
> Topic: <em>ucl/by-unid/984540640/ep1/Level/Attributes/CurrentLevel/Desired</em> - No payload<br/>
> Topic: <em>ucl/by-unid/984540640/ep1/Level/SupportedCommands</em> - No payload<br/>
> Topic: <em>ucl/by-unid/984540640/State</em> - No payload<br/>

Doing this will allow MQTT clients to know that the resource is no longer
available and SHOULD be deleted from their node list. Subsequent subscribers
will not receive the retained cluster states and states because of the zero byte
payload.

If a node has dynamic capabilities and a cluster server is removed, the same
procedure must take place and the cluster Attribute and SupportedCommands topics
MUST be published as an empty retained payload. In this case, the State topic
will stay and the node MUST still be shown as available and operational by the
MQTT clients.

### Protocol Controller is Unable to Service PAN Nodes

A Protocol Controller may be carrying operations that will restrict sending
commands to the end devices. This could for example be rebooting or performing
a firmware update.
When this happens, the Protocol Controller MUST publish on the state of every
UNID it can no longer service, for example if it has four nodes:

> 1. Publish
>> Topic: <em>ucl/by-unid/984540641/State</em><br/>
>> Payload:
>> ```json
>> {
>>   "NetworkStatus" : "Unavailable",
>>   "Security" : "Z-Wave S0",
>>   "MaximumCommandDelay" : 4200
>> }
>> ```
> 2. Publish
>> Topic: <em>ucl/by-unid/984540642/State</em><br/>
>> Payload:
>> ```json
>> {
>>   "NetworkStatus" : "Unavailable",
>>   "Security" : "None",
>>   "MaximumCommandDelay" : 0
>> }
>> ```
> 3. Publish
>> Topic: <em>ucl/by-unid/984540643/State</em><br/>
>> Payload:
>> ```json
>> {
>>   "NetworkStatus" : "Unavailable",
>>   "Security" : "Z-Wave S2 Authenticated",
>>   "MaximumCommandDelay" : 0
>> }
>> ```
> 4. Publish
>> Topic: <em>ucl/by-unid/984540644/State</em><br/>
>> Payload:
>> ```json
>> {
>>   "NetworkStatus" : "Unavailable",
>>   "Security" : "Z-Wave S2 Access Control",
>>   "MaximumCommandDelay" : 5
>> }
>> ```

This status indicates that any publication made by MQTT clients for this
PAN node will be ignored. MQTT Clients have to wait until the node is online
again.

## Group Discovery

The MQTT Topic ucl/by-unid/# is the only one that provides a "per end device"
granularity. The same end nodes will also be represented by conceptual groupings.

For example:

* <em>ucl/by-location/Kitchen</em>: All devices in the kitchen
* <em>ucl/by-type/DoorLock</em>: All Door Lock devices
* <em>ucl/by-group/GroupID</em>: Some devices that the user has grouped together for simultaneous actuation

### By Location

The location setting can be modified for each individual node using a dedicated cluster named NameAndLocation. This cluster contains two string attributes, "Name" and "Location". This data can be associated with a node, but it will be identical for each and every endpoint and it is not possible to set a different name and/or location to the different endpoints.

A ResourceDirectory MQTT Client will be in charge of aggregating and re-distributing this data. Protocol Controllers will not be aware of nodes names and location and will not use it for anything. The ResourceDirectory MQTT Client will listen to <em>ucl/by-unid/+/State</em> to get notifications of new nodes in the network. When a new node is added, it starts by assigning a default name a location, publish on a cluster for the node, and advertises it by publishing in the following two topics:

Example :

> MQTT Client publishes:
>> Topic: <em>ucl/by-unid/984540640/NameAndLocation</em><br/>
>> Payload:
>> ```json
>> {
>>   "Name": "node-984540640",
>>   "Location": "Unknown location"
>> }
>> ```
>
> Resource Directory MQTT Client publishes back:
>> Topic: <em>ucl/by-location/unknown_location/984540640</em><br/>
>> Payload: ```{ "unid":"984540640" }```
> The payload indicates that the node (unid:984540640) is available.

Application MQTT Clients can write a new location for a node/endpoint by publishing for example:

> Topic: <em>ucl/by-unid/984540640/NameAndLocation/WriteAttributes</em><br/>
> Payload:
> ```json
> {
>   "Name": "MySuperDoorLock",
>   "Location": "Entrance"
> }
> ```

Application MQTT Clients can discover which devices are part of the location by subscribing to the <em>ucl/by-location/#</em> topic.

After a node is added in the network or the name and/or location is set for a node, the Resource Directory MQTT Client will publish back on both the <em>ucl/by-location/&lt;location&gt;/&lt;unid&gt;</em> with a dummy payload indicating that the node is live (e.g., <em>ucl/by-location/living_room/32049350</em> ```{ "unid":"32049350" }```, and the clusters it serves on behalf on the node <em>ucl/by-unid/984540640/NameAndLocation</em> so that Application MQTT clients can see all the UNID of the nodes assigned in the locations they are interested into.

To transmit a message/command to all nodes in the kitchen, an Application MQTT client needs to publish using individual UNID commands (plus endpoints). For example, if the kitchen contains two nodes identified by UNID 32049350 and 32049351, the client can publish the following:

> Topic: <em>ucl/by-unid/32049350/ep0/OnOff/On</em> (Empty payload)<br/>
> Topic: <em>ucl/by-unid/32049351/ep2/OnOff/On</em> (Empty payload)<br/>

When a node is removed from the network, the ResourceDirectory MQTT Client will update the location by publishing without the retain flag at <em>ucl/by-location/&lt;location&gt;/&lt;unid&gt;</em> and <em>ucl/by-unid/&lt;unid&gt;/NameAndLocation</em> with an empty payload without the retain flag.

### By Type

The by-type namespace is postponed for now, because few PHYs use it. It is always possible to iterate over the by-unid namespace and filter by Cluster or derive the types manually.

The type is based on the ZCL Device Type. The ZCL device ID type is read-only and cannot be changed. In ZigBee, it can be read using the Simple_Desc_Req for each endpoint. The Simple_Desc_rsp contains a 2-byte field named "Application device identifier". This group should work in a similar fashion as the by-location group, except that it is read-only and clients cannot change the device type of a given node.

For example, when a Protocol Controller includes a Door Lock in a network (device identifier 0x000A), it will read its device identifier and will publish its unid/endpoint under the corresponding Device Type:

> Topic: <em>ucl/by-type/DoorLock/32049351/ep0</em><br/>
> Payload:
> ```json
> {
>  "unid" : "32049351",
>  "endpoint":"0"
> }
> ```

### By Group

The ZCL also allows to group nodes with actual user-defined groups.
These groups are available under the Groups Cluster (0x0004). The by-group
namespace has the advantage of being both editable and addressable directly.

If an IoT Service wants to control a set of nodes that belong to the same group,
it can issue the commands directly to the GroupId topic.


An helper component is used to keep track of group membership and capabilities,
mimicking the behavior in the `by-unid` space.

**Topic:** `ucl/by-group/&lt;GroupID&gt;/&lt;ClusterName&gt;/Commands/+`

This is the topic on which the Protocol Controllers will listen for incoming
commands that have to be issued to a group of nodes. It works as for any
application level clusters, described in [ZCL in Unify](doc/Chapter02-ZCL-in-uic.md).

Identically, IoT Service will be able to read the list of supported commands for
each cluster for a given group. This topic will show the common capabilities of
a group, as follows:

**Topic:** `ucl/by-group/&lt;GroupID&gt;/&lt;ClusterName&gt;/SupportedCommands`

The group membership of each node can be seen on the Group cluster
topic of the node itself, but a summary is also available under the following topic:

**Topic:** `ucl/by-group/&lt;GroupID&gt;/NodeList/&lt;UNID&gt;`

**JSON Schema:**

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "Group UNID list",
  "description": "List of endpoints under a UNID assigned to individual groups.",
  "type": "object",
  "properties": {
    "value": {
       "type": "array",
        "items": {
          "type": "integer",
          "minimum": 1,
          "maximum": 65535
        },
        "minItems": 1
    }
  },
  "required": [
    "value"
  ]
}
```

The payload under this topic is an array of endpoints under a given UNID that
belong to the group. The array MUST comprise at least 1 element, and an unretained
MQTT message with empty payload indicates that a UNID has no more endpoints
that are part of the actual group.

Each group may have a name assigned to it. The Name of the group will be
advertised at the following topic:

**Topic:** `ucl/by-group/&lt;GroupID&gt;/GroupName`

The JSON-Schema for this topic MUST be identical to the Name attribute defined
the [Group Cluster additional attributes](doc/Chapter02-ZCL-in-uic.md).

The name attribute is pushed individually by IoT Service to Protocol Controllers
that forward the information to the PAN nodes. There is a risk that PAN nodes
are assigned as part of the same group (e.g., Group 3) but with different names.

A Group Manager component MUST ensure that GroupName is updated and uniform
for all nodes that are part of the same group.

#### Protocol Controller and Groups Topics

Protocol Controllers will only listen to incoming commands for groups and
dispatch them to individual nodes.

When the group cluster is used to add groups at nodes/enpoints for an end-node,
a Protocol Controller MUST subscribe to group command topic, for example using
one of the following filters:

> Most specific: <em>ucl/by-group/&lt;groupID&gt;/&lt;ClusterName&gt;/Commands/&lt;CommandName&gt;</em><br/>
> For all commands: <em>ucl/by-group/&lt;groupID&gt;/&lt;ClusterName&gt;/Commands/+</em><br/>
> For all commands in any cluster: <em>ucl/by-group/&lt;groupID&gt;/+/Commands/+</em><br/>
> For any command in any group: <em>ucl/by-group/+/+/Commands/+</em><br/>


The following diagram shows an example of controlling PAN nodes that are part
of several groups.

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
title Controlling PAN nodes using the group topics

legend top
<font color=#0039FB>MQTT Subscription</font>
<font color=#00003C>Retained MQTT Publication</font>
<font color=#6C2A0D>Unretained MQTT Publication</font>
endlegend


' List of participants
participant "IoT Service" as iot_service
participant "MQTT Broker" as mqtt_broker
participant "Protocol Controller" as protocol_controller
participant "PAN Node 1\nzw-1234" as pan_node_1
participant "PAN Node 2\nzw-4321" as pan_node_2

protocol_controller -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/+/Commands/+
protocol_controller -> mqtt_broker: <font color=#0039FB>ucl/by-group/+/+/Commands/+

rnote over iot_service, pan_node_2: Node inclusion and capabilities discovery, group setup.

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Groups/Attributes/GroupList/Reported \n<font color=#00003C><b>{"value":[1, 2, 3]}</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-4321/ep0/Groups/Attributes/GroupList/Reported \n<font color=#00003C><b>{"value":[1, 4]}</b>
& mqtt_broker -> iot_service

== ==

iot_service -> mqtt_broker
& mqtt_broker -> protocol_controller : <font color=#6C2A0D>ucl/by-group/0/OnOff/Commands/Toggle

rnote over protocol_controller: no node in group 0.\nCommand is ignored.

== ==

iot_service -> mqtt_broker
& mqtt_broker -> protocol_controller : <font color=#6C2A0D>ucl/by-group/2/OnOff/Commands/On

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Desired \n<font color=#00003C><b>{ "value": true}</b>
& mqtt_broker -> iot_service

protocol_controller <-> pan_node_1 : PHY commands

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Reported \n<font color=#00003C><b>{ "value": true}</b>
& mqtt_broker -> iot_service

== ==

iot_service -> mqtt_broker
& mqtt_broker -> protocol_controller : <font color=#6C2A0D>ucl/by-group/1/OnOff/Commands/Off

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Desired \n<font color=#00003C><b>{ "value": false}</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-4321/ep0/OnOff/Attributes/OnOff/Desired \n<font color=#00003C><b>{ "value": false}</b>
& mqtt_broker -> iot_service

protocol_controller -> pan_node_1
& protocol_controller -> pan_node_2 : PHY commands

protocol_controller <-> pan_node_1 : PHY commands

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Reported \n<font color=#00003C><b>{ "value": false}</b>
& mqtt_broker -> iot_service

protocol_controller <-> pan_node_2 : PHY commands

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-4321/ep0/OnOff/Attributes/OnOff/Reported \n<font color=#00003C><b>{ "value": false}</b>
& mqtt_broker -> iot_service

\enduml

#### Group Manager Helper Component

Groups MAY span several Protocol Controllers, so the capabilities of a group
will not be advertised by Protocol Controllers themselves.

A special MQTT Client named the "Group Manager" can alleviate the aggregation
logic for other MQTT Clients such as IoT Service that want to make use
of the group functionalities.

The Group Manager will monitor group cluster updates and publish to these three
topics:

* ucl/by-group/&lt;GroupID&gt;/&lt;ClusterName&gt;/SupportedCommands
* ucl/by-group/&lt;GroupID&gt;/NodeList/&lt;UNID&gt;
* ucl/by-group/&lt;GroupID&gt;/GroupName


Additionally, the Group Manager MUST publish commands to the individual PAN nodes
using the ucl/by-group/&lt;GroupID&gt;/Groups/Commands/AddGroup topic
to keep the Group Names consistent.

An example of the interaction between the different components is shown below:

\startuml

node "MQTT Broker" as mqtt_broker

Package "Group Manager" as group_manager #FFFFFF {
  card group_manager_card #FFFFFF [
    <b>Subscribed to</b>:
    * ucl/by-unid/+/+/Groups/Attributes/GroupList/Reported
    * ucl/by-unid/+/+/Groups/Attributes/+/Name/Reported
    * ucl/by-unid/+/+/+/SupportedCommands
    <b>Publishes to</b>:
    * ucl/by-group/<GroupID>/<ClusterName>/SupportedCommands
    * ucl/by-group/<GroupID>/NodeList/<UNID>
    * ucl/by-group/<GroupID>/GroupName
    * ucl/by-group/<GroupID>/Groups/Commands/AddGroup
  ]
  node "Group Manager" as group_manager_1
}

Package "Protocol Controllers" as protocol_controller {
  node "Protocol Controller #1" as protocol_controller_1
  node "Protocol Controller #2" as protocol_controller_2
  card protocol_controller_card #FFFFFF [
    <b>Subscribed to</b>:
    * ucl/by-group/+/+/Commands/+
    <b>Publishes to</b>:
    * ucl/by-unid/<UNID>/ep<endpointID>/Groups/Attributes/GroupList/Reported
    * ucl/by-unid/<UNID>/ep<endpointID>/<ClusterName>/SupportedCommands
  ]
}

Package "IoT Services" as iot_service {
  node "IoT Service #1" as iot_service_1
  node "IoT Service #2" as iot_service_2
  card iot_service_card #FFFFFF [
    <b>Subscribed to</b>:
    * ucl/by-group/+/+/SupportedCommands
    * ucl/by-group/+/NodeList/+
    * ucl/by-group/+/GroupName
    <b>Publishes to</b>:
    * ucl/by-group/<GroupID>/<ClusterName>/Commands/<CommandName>
    * ucl/by-unid/<UNID>/ep<EndPointID>/Groups/Commands/<CommandName>
  ]
}

group_manager_1 <-down-> mqtt_broker
mqtt_broker <-right-> protocol_controller
mqtt_broker <-down-> iot_service

iot_service_card -[hidden]- iot_service_1
iot_service_card -[hidden]- iot_service_2
protocol_controller_card -[hidden]- protocol_controller_1
protocol_controller_card -[hidden]- protocol_controller_2

\enduml

An example for the Group Manager logic is shown in the diagram below.

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
title Group manager updates

legend top
<font color=#0039FB>MQTT Subscription</font>
<font color=#00003C>Retained MQTT Publication</font>
<font color=#6C2A0D>Unretained MQTT Publication</font>
endlegend

' List of participants
participant "Group Manager" as group_manager
participant "MQTT Broker" as mqtt_broker

group_manager -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Groups/Attributes/GroupList/Reported
group_manager -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Groups/Attributes/+/Name/Reported
group_manager -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/+/SupportedCommands

== New PAN node joins the group ==

mqtt_broker -> group_manager : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Groups/Attributes/GroupList/Reported \n<font color=#00003C><b>{"value":[1]}</b>

group_manager -> mqtt_broker : <font color=#00003C>ucl/by-group/1/NodeList/zw-4321/\n<font color=#00003C><b>{"value":[0]}</b>

mqtt_broker -> group_manager : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Groups/Attributes/1/Name/Reported \n<font color=#00003C><b>{"value":"Kitchen group"}</b>

group_manager -> mqtt_broker : <font color=#00003C>ucl/by-group/1/GroupName\n<font color=#00003C><b>{"value":"Kitchen group"}</b>

mqtt_broker -> group_manager : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/SupportedCommands \n<font color=#00003C><b>{"value":[ "On", "Off", "Toggle", "WriteAttributes" ]}</b>

group_manager -> mqtt_broker : <font color=#00003C>ucl/by-group/1/OnOff/SupportedCommands\n<font color=#00003C><b>{"value":[ "On", "Off", "Toggle", "WriteAttributes" ]}</b>

== New PAN node with toggle Command only joins the group ==

mqtt_broker -> group_manager : <font color=#00003C>ucl/by-unid/zb-0001/ep0/OnOff/SupportedCommands \n<font color=#00003C><b>{"value":[ "Toggle" ]}</b>

mqtt_broker -> group_manager : <font color=#00003C>ucl/by-unid/zb-0001/ep3/Groups/Attributes/GroupList/Reported \n<font color=#00003C><b>{"value":[1]}</b>

group_manager -> mqtt_broker : <font color=#00003C>ucl/by-group/1/NodeList/zb-0001/\n<font color=#00003C><b>{"value":[3]}</b>

group_manager -> mqtt_broker : <font color=#00003C>ucl/by-group/1/OnOff/SupportedCommands\n<font color=#00003C><b>{"value":[ "Toggle"]}</b>

mqtt_broker -> group_manager : <font color=#00003C>ucl/by-unid/zb-0001/ep3/Groups/Attributes/1/Name/Reported \n<font color=#00003C><b>{"value":"Kitchen Group Renamed"}</b>

group_manager -> mqtt_broker : <font color=#6C2A0D>ucl/by-group/1/Groups/Commands/AddGroup\n<font color=#6C2A0D><b>{ "GroupId" : 1, "GroupName": "Kitchen Group Renamed"}</b>

group_manager -> mqtt_broker : <font color=#00003C>ucl/by-group/1/GroupName\n<font color=#00003C><b>{"value":"Kitchen Group Renamed"}</b>

== New PAN node without OnOff Cluster joins the group ==

mqtt_broker -> group_manager : <font color=#00003C>ucl/by-unid/zb-0002/ep1/Groups/Attributes/GroupList/Reported \n<font color=#00003C><b>{"value":[1]}</b>

group_manager -> mqtt_broker : <font color=#00003C>ucl/by-group/1/NodeList/zb-0002/\n<font color=#00003C><b>{"value":[2]}</b>

group_manager -> mqtt_broker : <font color=#00003C>ucl/by-group/1/OnOff/SupportedCommands<font color=#00003C>

\enduml



#### IoT Services and Groups

The IoT services have the ability to define groups using the application level
group cluster. [See Chapter 2 - ZCL in Unify](doc/Chapter02-ZCL-in-uic.md).

With the help of the Group Manager component, the IoT services will get notified
of the groups capabilities after each update.

IoT Service SHOULD use names that are already assigned to a GroupID when
manipulating node group membership. i.e., if assigning a PAN Node to GroupID 3,
it SHOULD use the name already assigned to the group by reading the value published
at `ucl/by-group/3/GroupName`.

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
title Group usage example using the UCL / Unify Framework

legend top
<font color=#0039FB>MQTT Subscription</font>
<font color=#00003C>Retained MQTT Publication</font>
<font color=#6C2A0D>Unretained MQTT Publication</font>
endlegend


' List of participants
participant "Group Manager" as group_manager
participant "IoT Service" as iot_service
participant "MQTT Broker" as mqtt_broker
participant "Protocol Controller" as protocol_controller

iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/+/Attributes/#
iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/+/SupportedCommands
iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-group/+/+/SupportedCommands
iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-group/+/NodeList/+

protocol_controller -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/+/Commands/+
protocol_controller -> mqtt_broker: <font color=#0039FB>ucl/by-group/+/+/Commands/+

group_manager -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Groups/Attributes/GroupList/Reported
group_manager -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Groups/Attributes/+/Name/Reported
group_manager -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/+/SupportedCommands

== New OnOff node 1 joins the network ==

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0001/ep0/OnOff/Attributes/OnOff/Desired \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0001/ep0/OnOff/Attributes/OnOff/Reported \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0001/ep0/OnOff/SupportedCommands \n<font color=#00003C><b>{"value":[ "On", "Off", "Toggle", "WriteAttributes" ]}</b>
& mqtt_broker -> iot_service
& mqtt_broker -> group_manager

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0001/ep0/Groups/Attributes/NameSupport/Desired \n<font color=#00003C><b>{ "value": {"Supported":false} }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0001/ep0/Groups/Attributes/NameSupport/Reported \n<font color=#00003C><b>{ "value": {"Supported":false} }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0001/ep0/Groups/SupportedCommands \n<font color=#00003C><b>{"value":["AddGroup", "ViewGroup", "GetGroupMembership",\n<font color=#00003C><b>"RemoveGroup", "RemoveAllGroups", "AddGroupIfIdentifying"]}</b>
& mqtt_broker -> iot_service

== New OnOff node 2 joins the network ==

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0002/ep2/OnOff/Attributes/OnOff/Desired \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0002/ep2/OnOff/Attributes/OnOff/Reported \n<font color=#00003C><b>{ "value": false }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0002/ep2/OnOff/SupportedCommands \n<font color=#00003C><b>{"value":[ "On", "Off" ]}</b>
& mqtt_broker -> iot_service
& mqtt_broker -> group_manager

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0002/ep2/Groups/Attributes/NameSupport/Desired \n<font color=#00003C><b>{ "value": {"Supported":false} }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0002/ep2/Groups/Attributes/NameSupport/Reported \n<font color=#00003C><b>{ "value": {"Supported":false} }</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0002/ep2/Groups/SupportedCommands \n<font color=#00003C><b>{"value":["AddGroup", "ViewGroup", "GetGroupMembership",\n<font color=#00003C><b>"RemoveGroup", "RemoveAllGroups", "AddGroupIfIdentifying"]}</b>
& mqtt_broker -> iot_service

== OnOff node 1 joins Group 1 ==

iot_service -> mqtt_broker : <font color=#6C2A0D>ucl/by-unid/zw-0001/ep0/Groups/Commands/AddGroup \n<font color=#6C2A0D><b>{ "GroupId" : 1, "GroupName": "Kitchen"}</b>
& mqtt_broker -> protocol_controller

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0001/ep0/Groups/Attributes/GroupList/Desired \n<font color=#00003C><b>{"value":[1]}</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0001/ep0/Groups/Attributes/GroupList/Reported \n<font color=#00003C><b>{"value":[1]}</b>
& mqtt_broker -> iot_service
& mqtt_broker -> group_manager

group_manager -> mqtt_broker : <font color=#00003C>ucl/by-group/1/NodeList/zw-0001\n<font color=#00003C><b>{"value":[0]}</b>
mqtt_broker --> iot_service

group_manager -> mqtt_broker : <font color=#00003C>ucl/by-group/1/GroupName\n<font color=#00003C><b>{"value": "Kitchen"}</b>
mqtt_broker --> iot_service

group_manager -> mqtt_broker : <font color=#00003C>ucl/by-group/1/OnOff/SupportedCommands \n<font color=#00003C><b>{"value":[ "On", "Off", "Toggle", "WriteAttributes" ]}</b>
mqtt_broker --> iot_service

== OnOff node 2 joins Group 1 ==

iot_service -> mqtt_broker : <font color=#6C2A0D>ucl/by-unid/zw-0002/ep2/Groups/Commands/AddGroup \n<font color=#6C2A0D><b>{ "GroupId" : 1, "GroupName": "Kitchen"}</b>
& mqtt_broker -> protocol_controller

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0002/ep2/Groups/Attributes/GroupList/Desired \n<font color=#00003C><b>{"value":[1]}</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0002/ep2/Groups/Attributes/GroupList/Reported \n<font color=#00003C><b>{"value":[1]}</b>
& mqtt_broker -> iot_service
& mqtt_broker -> group_manager

group_manager -> mqtt_broker : <font color=#00003C>ucl/by-group/1/NodeList/zw-0002\n<font color=#00003C><b>{"value":[2]}</b>
mqtt_broker --> iot_service

group_manager -> mqtt_broker : <font color=#00003C>ucl/by-group/1/OnOff/SupportedCommands \n<font color=#00003C><b>{"value":[ "On", "Off"]}</b>
mqtt_broker --> iot_service


== Group 1 get turned on ==

iot_service -> mqtt_broker : <font color=#6C2A0D>ucl/by-group/1/OnOff/Commands/On
& mqtt_broker -> protocol_controller

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0001/ep0/OnOff/Attributes/OnOff/Desired \n<font color=#00003C><b>{"value": true}</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0002/ep2/OnOff/Attributes/OnOff/Desired \n<font color=#00003C><b>{"value": true}</b>
& mqtt_broker -> iot_service

... Command Execution ...

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0001/ep0/OnOff/Attributes/OnOff/Reported \n<font color=#00003C><b>{"value": true}</b>
& mqtt_broker -> iot_service

protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-0002/ep2/OnOff/Attributes/OnOff/Reported \n<font color=#00003C><b>{"value": true}</b>
& mqtt_broker -> iot_service

\enduml
