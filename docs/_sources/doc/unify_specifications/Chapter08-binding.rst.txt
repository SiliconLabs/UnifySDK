.. meta::
  :description: Unify Binding Cluster
  :keywords: Binding

.. raw:: latex

 \newpage

.. _unify_specifications_chapter_binding:

Binding
=======

This chapter describes how to use the binding functionality in Unify.

The unify binding feature can be used by an IoT service / rules engine to
impose direct rules between nodes on the PAN or even for nodes between
PAN and wireless protocols. The benefit of the direct rules are a
binding between two nodes in the same network will impose less network
latency compared to a star topology where all events needs to be passed
though an IoT service. Another benefit is that in the event of the a protocol
controller going offline the binding rules setup in the network will still work.

The Unify Binding framework also features a system to identify which nodes
can be bound. Care must be taken when IoT services setup bindings,
because the Protocol Controllers will not sanitise whether a binding is safe
from a security perspective.

An IoT service MUST verify what a binding is acceptable
from a security perspective before setting up the binding.

Establishing a binding
----------------------

The main use case of a binding is to have a PAN node cluster client issue commands
directly to another PAN node cluster server.

An example is shown below.

.. uml::

  ' Allows simultaneous transmissions
  !pragma teoz true

  ' Style for the diagram
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

  title Direct PAN node Binding example

  legend top
  <font color=#0039FB>MQTT Subscription</font>
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  <font color=#000000>PHY messages</font>
  endlegend


  ' List of participants
  participant "IoT Service" as iot_service

  box "PAN 1"
    participant "Protocol Controller\nUNID: pc" as protocol_controller
    participant "OnOff cluster client\nUNID: node_1" as client_node
    participant "OnOff cluster server\nUNID: node_2" as server_node
  end box

  protocol_controller -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Reported \n<font color=#00003C><b>{"value": []}

  protocol_controller -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindableClusterList/Reported \n<font color=#00003C><b>{"value": ["OnOff", "Level","Identify"]}

  protocol_controller -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTableFull/Reported \n<font color=#00003C><b>{"value": false}

  iot_service -> protocol_controller: <font color=#6C2A0D>ucl/by-unid/node_1/ep0/Binding/Commands/Bind \n<font color=#6C2A0D><b>{"ClusterName" : "OnOff", \n<font color=#6C2A0D><b>"DestinationUnid": "node_2",\n<font color=#6C2A0D><b>"DestinationEp": 0}

  protocol_controller -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Desired \n<font color=#00003C><b>{"value": [\n\t<font color=#00003C><b>{"ClusterName" : "OnOff",  \n\t<font color=#00003C><b>"DestinationUnid": "node_2",\n\t<font color=#00003C><b>"DestinationEp": 0}\n<font color=#00003C><b>]}

  protocol_controller -> client_node: Establish Binding
  client_node -> protocol_controller: Establish Binding Response

  protocol_controller -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Reported \n<font color=#00003C><b>{"value": [\n\t<font color=#00003C><b>{"ClusterName" : "OnOff",  \n\t<font color=#00003C><b>"DestinationUnid": "node_2",\n\t<font color=#00003C><b>"DestinationEp": 0}\n<font color=#00003C><b>]}

  rnote over client_node: Node OnOff cluster\n is now bound to "node_2/ep0".

  rnote over client_node: Event Triggering\n OnOff Commands
  client_node -> server_node: OnOff Cluster: On Command

  rnote over client_node: Event Triggering\n OnOff Commands
  client_node -> server_node: OnOff Cluster: Toggle Command

Binding object
--------------

  A binding consists of the following elements/fields

* **SourceUnid**:

  The UNID of the client node that will issue commands to the server.

  This information will be found in the topic used to issue the command or
  read the attributes

* **SourceEp**:

  The Endpoint ID of the client node that will issue commands to the server

  This information will be found in the topic used to issue the command or
  read the attributes

* **ClusterName**:

  The name of the ClusterID of the client node that will issue commands
  to the server

* **DestinationUnid**:

  The UNID of the server node that will receive commands from the client

* **DestinationEp**:

  The Endpoint ID of the server node that will receive commands from the client

There is no real control over what triggers which commands from the client to
the server. For example, an OccupancySensor server that is also an OnOff cluster
client may decide to send On commands to bound device when OccupancySensing is
True and Off when False, or vice-versa.

MQTT Topics and Parameters
''''''''''''''''''''''''''

.. list-table:: Binding MQTT Topics
  :name: table_unify_specifications_binding_mqtt_topics
  :widths: 40 60
  :width: 80%
  :header-rows: 1

  * - MQTT Topic
    - Description
  * - ucl/by-unid/+/+/Binding/Commands/<CommandName>
    - Command topic space. IoT services can publish under this topic.

      See the XML file for command definitions.
  * - ucl/by-unid/+/+/Binding/Attributes/<AttributeName>/Reported
    - Used to publish the standard and additionnal attributes of the cluster.

      See the XML file for attributes definitions.
  * - ucl/by-unid/+/+/Binding/Attributes/<AttributeName>/Desired
    - Used to publish the standard and additionnal attributes of the cluster.

      See the XML file for attributes definitions.
  * - ucl/by-unid/+/+/Binding/SupportedCommands
    - Used to publish the supported commands.
  * - ucl/by-unid/+/+/Binding/SupportedGeneratedCommands
    - Used to publish the supported generated commands.
  * - ucl/by-unid/+/+/Binding/GeneratedCommands/<CommandName>
    - Used to publish the generated commands.

Additional attributes
'''''''''''''''''''''

This cluster makes use of several additional attributes. These attributes are
not present in the XML file.

Binding Table
~~~~~~~~~~~~~

A binding table will be kept by each PAN node, keeping track of the established
bindings.

The value of the binding table is an array of binding objects. Each binding
object consists in a Cluster ID, Destination UNID and Destination Endpoint.

For example:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/Binding/Attributes/BindingTable/Reported -
    { "value": [
        {"ClusterName" : "OnOff", "DestinationUnid": "node_2", "DestinationEp": 1},
        {"ClusterName" : "OnOff", "DestinationUnid": "node_2", "DestinationEp": 2},
        {"ClusterName" : "Level", "DestinationUnid": "node_2", "DestinationEp": 2}]
    }

The JSON schema for the BindingTable MUST be as follows:

.. code-block:: json

  {
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "Binding Table",
    "description": "Binding Table",
    "definitions": {
      "Binding Table Entry": {
        "type": "object",
        "properties": {
          "ClusterName": {
            "type": "string",
            "minLength": 1
          },
          "DestinationUnid": {
            "type": "string",
            "minLength": 1
          },
          "DestinationEp": {
            "type": "integer",
            "minimum": 0,
            "maximum": 254
          }
        },
        "required": [
          "ClusterName",
          "DestinationUnid",
          "DestinationEp"
        ]
      }
    },
    "type": "object",
    "properties": {
      "value": {
        "type": "array",
        "items": {
          "$ref": "#/definitions/Binding Table Entry"
        }
      }
    },
    "required":["value"]
  }

Discovering what can be bound
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Protocol Controller MUST advertise the list of available Clusters for binding
for each PAN node. The *BindableClusterList* attribute indicates an array
of Cluster IDs that can be used for Binding Commands.

If the list of Cluster IDs that can be bound is unknown to the Protocol
Controller, it MUST NOT publish any Binding capabilities for that
UNID/Endpoint.

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/Binding/Attributes/BindableClusterList/Desired - { "value": ["OnOff", "Level"]}
  ucl/by-unid/<UNID>/ep<EndpointId>/Binding/Attributes/BindableClusterList/Reported - { "value": ["OnOff", "Level"]}

The JSON schema for this topic MUST be as follows:

.. code-block:: json

  {
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "Binding Cluster ID list",
    "description": "Binding Cluster ID list",
    "type": "object",
    "properties": {
      "value": {
        "type": "array",
        "items": {
          "type": "string",
          "minLength": 1
        }
      }
    },

    "required":["value"]
  }

IoT Service SHOULD NOT attempt to bind a node to a destination, if the
destination does not have any Attributes or SupportedCommands for the Cluster
used in the binding.

Protocol Controller MUST reject bindings if the destination cannot receive
commands from the cluster defined in the binding.

Binding levels
--------------

When a binding is estalished in the Unify system, it is possible that the
2 nodes cannot communicate directly with each other.

Therefore, an IoT Service requesting to Bind 2 nodes can lead to 3 (hidden) substates

* **Direct PAN binding**

  When such a binding is established, 2 nodes will communicate directly with
  each other and no Protocol Controller will be involved.

* **Intra Protocol Controller binding**

  The 2 nodes cannot communicate directly with each other, but are serviced by
  the same Protocol Controller and therefore the Protocol Controller can "relay"  messages.

* **Inter Protocol Controller binding**

  The 2 nodes have 2 different Protocol Controllers and therefore the Protocol
  Controllers will need to send commands to each other to make the binding work.

Both Intra and Inter Protocol Controller Bindings may be treated the same way
by a Protocol Controller, which can publish a Command to the destination UNID/Ep using the MQTT command space.

If the Protocol Controller itself is servicing the destination, it will receive
its own command publication and proceed with sending the command to the destination.

.. uml::

  ' Allows simultaneous transmissions
  !pragma teoz true

  ' Style for the diagram
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

  title Inter/Intra Protocol Controller binding example

  legend top
  <font color=#0039FB>MQTT Subscription</font>
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  <font color=#000000>PHY messages</font>
  endlegend


  ' List of participants
  participant "IoT Service" as iot_service
  box "PAN 1"
    participant "Protocol Controller 1\nUNID: pc_1" as protocol_controller_1
    participant "OnOff cluster client\nUNID: node_1" as client_node
  end box

  box "PAN 2" #LightBlue
    participant "Protocol Controller 2\nUNID: pc_2" as protocol_controller_2
    participant "OnOff cluster server\nUNID: node_2" as server_node
  end box


  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Reported \n<font color=#00003C><b>{"value": []}

  iot_service -> protocol_controller_1: <font color=#6C2A0D>ucl/by-unid/node_1/ep0/Binding/Commands/Bind \n<font color=#6C2A0D><b>{"ClusterName" : "OnOff", \n<font color=#6C2A0D><b>"DestinationUnid": "node_2",\n<font color=#6C2A0D><b>"DestinationEp": 0}

  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Desired \n<font color=#00003C><b>{"value": [\n\t<font color=#00003C><b>{"ClusterName" : "OnOff",  \n\t<font color=#00003C><b>"DestinationUnid": "node_2",\n\t<font color=#00003C><b>"DestinationEp": 0}\n<font color=#00003C><b>]}

  rnote over protocol_controller_1: node_1 cannot \ncommunicate with node_2

  protocol_controller_1 -> client_node: Establish Binding\ntowards Protocol Controller 1
  client_node -> protocol_controller_1: Establish Binding Response

  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Reported \n<font color=#00003C><b>{"value": [\n\t<font color=#00003C><b>{"ClusterName" : "OnOff",  \n\t<font color=#00003C><b>"DestinationUnid": "node_2",\n\t<font color=#00003C><b>"DestinationEp": 0}\n<font color=#00003C><b>]}

  rnote over client_node: Node OnOff cluster\n is now "Inter PC" bound\nto "node_2/ep0".

  rnote over client_node: Event Triggering\n OnOff Commands
  client_node -> protocol_controller_1: OnOff Cluster: Toggle Command

  protocol_controller_1 -> protocol_controller_2: <font color=#6C2A0D>ucl/by-unid/node_2/ep0/OnOff/Commands/Toggle <b>{}

  protocol_controller_2 -> iot_service: <font color=#00003C>ucl/by-unid/node_2/ep0/OnOff/Attributes/OnOff/Desired\n<font color=#00003C><b>{"value": true}

  protocol_controller_2 -> server_node: OnOff Cluster:\nToggle Command
  server_node -> protocol_controller_2: OnOff Cluster\nattribute reporting

  protocol_controller_2 -> iot_service: <font color=#00003C>ucl/by-unid/node_2/ep0/OnOff/Attributes/OnOff/Reported\n<font color=#00003C><b>{"value": true}

Protocol Controllers are not mandated to support creating Intra or Inter
Protocol Controller bindings. In this case, it will appear from the IoT Service
as if the Bind command was ignored or rejected.

Binding table capacity
''''''''''''''''''''''

A PAN node may have a limited capacity in its binding table. When that happens,
a protocol controller can use the ``BindingTableFull`` attribute to indicate
that we have reached the limit, and no more bindings can be made.

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/Binding/Attributes/BindingTableFull/Reported - {"value": true}

Note that Protocol Controllers MAY start establishing Intra Protocol Controller
bindings when the capacity of the PAN node is reached. In this case,
the Protocol Controller MUST NOT advertise that the binding table is full.

A Protocol Controller establishing Intra Protocol Controller Bindings MUST
advertise that the Binding table is full only if both the PAN node and the
Protocol Controller have exhausted their binding capacity.

MQTT publications example
'''''''''''''''''''''''''

An example of a PAN node with the Bindings server PAN node able to send commands
for the OnOff and Level clusters will be shown as follows:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/Binding/Attributes/BindingTableFull/Reported - {"value": false}
  ucl/by-unid/<UNID>/ep<EndpointId>/Binding/Attributes/BindingTableFull/Desired - {"value": false}
  ucl/by-unid/<UNID>/ep<EndpointId>/Binding/Attributes/BindableClusterList/Reported - {"value": ["OnOff", "Level"]}
  ucl/by-unid/<UNID>/ep<EndpointId>/Binding/Attributes/BindableClusterList/Desired - {"value": ["OnOff", "Level"]}
  ucl/by-unid/<UNID>/ep<EndpointId>/Binding/Attributes/BindingTable/Reported - {"value": []}
  ucl/by-unid/<UNID>/ep<EndpointId>/Binding/Attributes/BindingTable/Desired - {"value": []}
  ucl/by-unid/<UNID>/ep<EndpointId>/Binding/SupportedCommands - {"value": ["Bind","Unbind"]}
  ucl/by-unid/<UNID>/ep<EndpointId>/Binding/SupportedGeneratedCommands - {"value": []}

Detailed examples
'''''''''''''''''

Happy case
~~~~~~~~~~

.. uml::

  ' Allows simultaneous transmissions
  !pragma teoz true

  ' Style for the diagram
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

  title Direct binding happy case full example

  legend top
  <font color=#0039FB>MQTT Subscription</font>
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  <font color=#000000>PHY messages</font>
  endlegend


  ' List of participants
  participant "IoT Service" as iot_service
  box "PAN 1"
    participant "Protocol Controller 1\nUNID: pc_1" as protocol_controller_1
    participant "OnOff cluster client\nUNID: node_1" as client_node
    participant "OnOff cluster server\nUNID: node_2" as server_node
  end box

  rnote over protocol_controller_1, client_node: Capability discovery

  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Desired \n<font color=#00003C><b>{"value": []}
  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Reported \n<font color=#00003C><b>{"value": []}
  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindableClusterList/Desired \n<font color=#00003C><b> {"value": ["OnOff", "Level"]}
  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindableClusterList/Reported \n<font color=#00003C><b> {"value": ["OnOff", "Level"]}
  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTableFull/Desired \n<font color=#00003C><b>{"value": false}
  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTableFull/Reported \n<font color=#00003C><b>{"value": false}

  iot_service -> protocol_controller_1: <font color=#6C2A0D>ucl/by-unid/node_1/ep0/Binding/Commands/Bind \n<font color=#6C2A0D><b>{"ClusterName" : "OnOff", \n<font color=#6C2A0D><b>"DestinationUnid": "node_2",\n<font color=#6C2A0D><b>"DestinationEp": 2}

  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Desired \n<font color=#00003C><b>{"value": [\n\t<font color=#00003C><b>{"ClusterName" : "OnOff",  \n\t<font color=#00003C><b>"DestinationUnid": "node_2",\n\t<font color=#00003C><b>"DestinationEp": 2}\n<font color=#00003C><b>]}

  protocol_controller_1 -> client_node: Establish Binding\ntowards node_2, ep 2
  client_node -> protocol_controller_1: Establish Binding Response

  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Reported \n<font color=#00003C><b>{"value": [\n\t<font color=#00003C><b>{"ClusterName" : "OnOff",  \n\t<font color=#00003C><b>"DestinationUnid": "node_2",\n\t<font color=#00003C><b>"DestinationEp": 2}\n<font color=#00003C><b>]}

  rnote over client_node: Event Triggering\n OnOff Commands
  client_node -> server_node: OnOff Cluster: On Command

  rnote over client_node: Event Triggering\n OnOff Commands
  client_node -> server_node: OnOff Cluster: Off Command

  iot_service -> protocol_controller_1: <font color=#6C2A0D>ucl/by-unid/node_1/ep0/Binding/Commands/Unbind \n<font color=#6C2A0D><b>{"ClusterName" : "OnOff", \n<font color=#6C2A0D><b>"DestinationUnid": "node_2",\n<font color=#6C2A0D><b>"DestinationEp": 2}

  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Desired \n<font color=#00003C><b>{"value": []}

  protocol_controller_1 -> client_node: Remove Binding\ntowards node_2, ep 2
  client_node -> protocol_controller_1: Remove Binding Response

  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Reported \n<font color=#00003C><b>{"value": []}

  rnote over client_node: Event Triggering\n OnOff Commands
  rnote over client_node: Do nothing

Failed binding attempts
~~~~~~~~~~~~~~~~~~~~~~~

.. uml::

  ' Style for the diagram
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

  title Binding rejection examples

  legend top
  <font color=#0039FB>MQTT Subscription</font>
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  <font color=#000000>PHY messages</font>
  endlegend


  ' List of participants
  participant "IoT Service" as iot_service
  box "PAN 1"
    participant "Protocol Controller 1\nUNID: pc_1" as protocol_controller_1
    participant "OnOff cluster client\nUNID: node_1" as client_node
    participant "OnOff cluster server\nUNID: node_2" as server_node
  end box

  rnote over protocol_controller_1, client_node: Capability discovery

  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Desired \n<font color=#00003C><b>{"value": []}
  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Reported \n<font color=#00003C><b>{"value": []}
  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindableClusterList/Desired \n<font color=#00003C><b> {"value": ["OnOff", "Level"]}
  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindableClusterList/Reported \n<font color=#00003C><b> {"value": ["OnOff", "Level"]}
  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTableFull/Desired \n<font color=#00003C><b>{"value": false}
  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTableFull/Reported \n<font color=#00003C><b>{"value": false}

  iot_service -> protocol_controller_1: <font color=#6C2A0D>ucl/by-unid/node_1/ep0/Binding/Commands/Bind \n<font color=#6C2A0D><b>{"ClusterName" : "OnOff", \n<font color=#6C2A0D><b>"DestinationUnid": "node_2",\n<font color=#6C2A0D><b>"DestinationEp": 2}

  rnote over protocol_controller_1: No Endpoint 2 for node_2.\nIgnoring without trying

  iot_service -> protocol_controller_1: <font color=#6C2A0D>ucl/by-unid/node_1/ep0/Binding/Commands/Bind \n<font color=#6C2A0D><b>{"ClusterName" : "OnOff", \n<font color=#6C2A0D><b>"DestinationUnid": "node_2",\n<font color=#6C2A0D><b>"DestinationEp": 0}

  rnote over protocol_controller_1: Binding accepted

  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Desired \n<font color=#00003C><b>{"value": [\n\t<font color=#00003C><b>{"ClusterName" : "OnOff",  \n\t<font color=#00003C><b>"DestinationUnid": "node_2",\n\t<font color=#00003C><b>"DestinationEp": 0}\n<font color=#00003C><b>]}

  protocol_controller_1 -> client_node: Establish Binding\ntowards node_2, ep 2
  client_node -> protocol_controller_1: Establish Binding Response\n(Rejected)

  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Desired \n<font color=#00003C><b>{"value": []}

Explicit binding towards a Protocol Controller
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

IoT Service MAY use the binding cluster to force binding towards a Protocol
Controller. The Binding cluster provides a special command for this, so
IoT services do not need to figure out which UNID/ep to use for the
Binding to point at the Protocol Controller.

Protocol Controllers supporting to establish bindings towards themselves
SHOULD forward commands received by PAN nodes to IoT Services using the
``ucl/by-unid/<unid>/ep<id>/<ClusterName>/GeneratedCommands``
topic space.

The diagram below shows an example of an explicit binding to the Protocol
Controller.

.. uml::

  ' Style for the diagram
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

  title Binding toward the Protocol Controller

  legend top
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  <font color=#000000>PHY messages</font>
  endlegend


  ' List of participants
  participant "IoT Service" as iot_service
  box "PAN 1"
    participant "Protocol Controller 1\nUNID: pc_1" as protocol_controller_1
    participant "OnOff cluster client\nUNID: node_1" as client_node
  end box

  rnote over protocol_controller_1, client_node: Capability discovery

  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Desired \n<font color=#00003C><b>{"value": []}
  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Reported \n<font color=#00003C><b>{"value": []}
  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindableClusterList/Desired \n<font color=#00003C><b> {"value": ["OnOff", "Level"]}
  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindableClusterList/Reported \n<font color=#00003C><b> {"value": ["OnOff", "Level"]}
  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTableFull/Desired \n<font color=#00003C><b>{"value": false}
  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTableFull/Reported \n<font color=#00003C><b>{"value": false}

  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/SupportedCommands \n<font color=#00003C><b>{"value": ["BindToProtocolController","UnbindFromProtocolController"]}

  iot_service -> protocol_controller_1: <font color=#6C2A0D>ucl/by-unid/node_1/ep0/Binding/Commands/BindToProtocolController \n<font color=#6C2A0D><b>{"ClusterName" : "OnOff"}

  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Desired \n<font color=#00003C><b>{"value": [\n\t<font color=#00003C><b>{"ClusterName" : "OnOff",  \n\t<font color=#00003C><b>"DestinationUnid": "pc_1",\n\t<font color=#00003C><b>"DestinationEp": 2}\n<font color=#00003C><b>]}

  protocol_controller_1 -> client_node: Establish Binding\ntowards pc_1, ep 2
  client_node -> protocol_controller_1: Establish Binding Response\n(Accepted)

  protocol_controller_1 -> iot_service: <font color=#00003C>ucl/by-unid/node_1/ep0/Binding/Attributes/BindingTable/Desired \n<font color=#00003C><b>{"value": [\n\t<font color=#00003C><b>{"ClusterName" : "OnOff",  \n\t<font color=#00003C><b>"DestinationUnid": "pc_1",\n\t<font color=#00003C><b>"DestinationEp": 2}\n<font color=#00003C><b>]}

  rnote over client_node: Event Triggering\n OnOff Commands
  client_node -> protocol_controller_1: OnOff Cluster: Toggle Command

  rnote over iot_service: Gets notified of\nOnOff commands\ngenerated by node_2

  protocol_controller_1 -> iot_service: <font color=#6C2A0D>ucl/by-unid/node_2/ep0/OnOff/GeneratedCommand/Toggle <b>{}

