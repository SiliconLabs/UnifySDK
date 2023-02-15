.. meta::
  :description: Unify Name and Location Service
  :keywords: Name, Location, IoT Service

.. |ZCL2019| replace:: `ZCL 2019 <https://zigbeealliance.org/wp-content/uploads/2019/12/07-5123-06-zigbee-cluster-library-specification.pdf>`__

.. raw:: latex

 \newpage

.. _unify_specifications_chapter_name_and_location_service:

Name and Location Service
=========================

This chapter describes the usage of a common naming and location service for
PAN nodes in the Unify system.

This functionality allows IoT Services to assign and readback a Name and a
Location for each node/endpoint.

The Name and Location Service is a helper MQTT component that allows to
perform book-keeping on text names and locations that have been assigned.

Overview
--------

The Name and Location Service will operate on the NameAndLocation cluster
(see the *Unify_NameAndLocation.xml*).

.. uml::

  ' Style for the diagram
  allow_mixing
  skinparam objectBorderColor black

  title Name and Location Service

  package "PAN 1" <<Cloud>> {

      object "Protocol Controller 1" as pc_1 #FFFFFF
      package "Nodes" as pan_1_nodes  <<Rectangle>> {
          object "Node 1" as node_1_1 #FFFFFF
          object "Node 2" as node_1_2 #FFFFFF
      }
  }

  package "PAN 2" <<Cloud>> {

      object "Protocol Controller 2" as pc_2 #FFFFFF
      package "Nodes" as pan_2_nodes <<Rectangle>> {
          object "Node 3" as node_2_1 #FFFFFF
          object "Node 4" as node_2_2 #FFFFFF
      }
  }

  object "MQTT Broker" as mqtt_broker #FFFFFF
  object "Name and Location\nService" as naming_service #FFFFFF

  pc_1 -down-> mqtt_broker: network info\napplication info
  pc_2 -down-> mqtt_broker: network info\napplication info

  mqtt_broker -down-> naming_service: State information about nodes
  naming_service -up-> mqtt_broker : Provides Name And Location

Name and Location Service mechanism
-----------------------------------

The Name and Location Service listens to publications to the following topic
filters:

* ``ucl/by-unid/+/State``
* ``ucl/by-unid/+/+/#``

to determine a list of nodes (UNID) and endpoints in the network.

For each Node/Endpoint pair in the network, it will provide a
NameAndLocation cluster on behalf of all nodes.

The Name and Location Service MUST monitor the presence of a State topic for
nodes before it provides a cluster for a node. i.e. there must be data published
at the ``ucl/by-unid/<unid>/State`` topic for all nodes that it provides
the NameAndLocation cluster for.
If the State topic for a node is removed (empty retained publication), the
Name and Location Service MUST stop providing the NameAndLocation cluster
for all the endpoints for that UNID.

A simple example with only 1 endpoint is shown in the following diagram:

.. uml::

  'Style matching the other figures in chapter.
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  legend top
  <font color=#0039FB>MQTT Subscription</font>
  <font color=#008000>MQTT Unsubscription</font>
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  endlegend

  participant "Name and Location Service\nMQTT Client" as naming_service
  participant "MQTT Broker" as mqtt_broker
  participant "IoT Service" as iot_service
  participant "Protocol Controller\nUNID: pc_1" as pc_1
  participant "PAN Node\nUNID:node_1" as node_1

  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/State</font>
  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/State/Attributes/EndpointIdList/Reported</font>
  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/#</font>

  == Name and Location Service is ready ==

  rnote over pc_1, node_1: Node added to the\nnetwork and discovered.

  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/State <b>{Non-empty}
  & mqtt_broker -> naming_service

  rnote over naming_service: node_1 detected.\nAssuming 1 endpoint

  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/node_1/ep0/NameAndLocation/Commands/WriteAttributes</font>
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Desired\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Reported\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Desired\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Reported\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/SupportedCommands\n<font color=#00003C><b>{"value":["WriteAttributes"]}

  == IoT Service can manipulate the Name cluster ==

  iot_service -> mqtt_broker: <font color=#6C2A0D>ucl/by-unid/node_1/ep0/NameAndLocation/Commands/WriteAttributes\n<font color=#6C2A0D><b>{"Name":"Wall outlet"}
  & mqtt_broker -> naming_service

  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Desired\n<font color=#00003C><b>{"value":"Wall outlet"}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Reported\n<font color=#00003C><b>{"value":"Wall outlet"}

  rnote over pc_1: reboots

  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/State <b>{"NetworkStatus": "Unavailable", ...}
  & mqtt_broker -> naming_service

  rnote over naming_service: node_1 not removed,\nNameAndLocation is still available\neven offline

  iot_service -> mqtt_broker: <font color=#6C2A0D>ucl/by-unid/node_1/ep0/NameAndLocation/Commands/WriteAttributes\n<font color=#6C2A0D><b>{"Location":"Entrance"}
  & mqtt_broker -> naming_service

  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Desired\n<font color=#00003C><b>{"value":"Entrance"}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Reported\n<font color=#00003C><b>{"value":"Entrance"}

  rnote over pc_1, node_1: Removes the node\nfrom the network

  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/State <b><i>Empty
  & mqtt_broker -> naming_service

  rnote over naming_service: node_1 removed\nunretaining all publications\nfor all its endpoints.

  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Desired
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Reported
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Desired
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Reported
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/SupportedCommands
  naming_service -> mqtt_broker: <font color=#008000>ucl/by-unid/node_1/ep0/NameAndLocation/Commands/WriteAttributes</font>

If an node has multiple endpoints with functionalities, the Name and Location
Service will be able to detect publications under for additional endpoints.

The Name and Location Service SHOULD assume that endpoints do not disappear, even
if they seem to temporarily loose application functionalities.
For example, if endpoint 3 was detected but an re-interview makes it go away,
the Name and Location Service SHOULD continue to provide the NameAndLocation
for endpoint 3.

A multi-endpoint example is shown in the following diagram:

.. uml::

  'Style matching the other figures in chapter.
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  legend top
  <font color=#0039FB>MQTT Subscription</font>
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  endlegend

  participant "Name and Location Service\nMQTT Client" as naming_service
  participant "MQTT Broker" as mqtt_broker
  participant "IoT Service" as iot_service
  participant "Protocol Controller\nUNID: pc_1" as pc_1
  participant "PAN Node\nUNID:node_1" as node_1

  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/State</font>
  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/#</font>

  == Name and Location Service is ready ==

  rnote over pc_1, node_1: Node added to the\nnetwork and discovered.

  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/State <b>{Non-empty}
  & mqtt_broker -> naming_service

  rnote over naming_service: node_1 detected.\nAssuming 1 endpoint

  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/node_1/ep0/NameAndLocation/Commands/WriteAttributes</font>
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Desired\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Reported\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/SupportedCommands\n<font color=#00003C><b>{"value":["WriteAttributes"]}

  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep1/Basic/Attributes/PowerSource/Reported \n<font color=#6C2A0D><b>{Non-empty}
  & mqtt_broker -> naming_service

  rnote over naming_service: node_1 / ep1 detected.

  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/node_1/ep1/NameAndLocation/Commands/WriteAttributes</font>
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep1/NameAndLocation/Attributes/Name/Desired\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep1/NameAndLocation/Attributes/Name/Reported\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep1/NameAndLocation/SupportedCommands\n<font color=#00003C><b>{"value":["WriteAttributes"]}

  == IoT Service can manipulate the NameAndLocation cluster ==

  iot_service -> mqtt_broker: <font color=#6C2A0D>ucl/by-unid/node_1/ep1/NameAndLocation/Commands/WriteAttributes\n<font color=#6C2A0D><b>{"Name":"Wall outlet"}
  & mqtt_broker -> naming_service

  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep1/NameAndLocation/Attributes/Name/Desired\n<font color=#00003C><b>{"value":"Wall outlet"}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep1/NameAndLocation/Attributes/Name/Reported\n<font color=#00003C><b>{"value":"Wall outlet"}

  == If all other application functionalities disappear from ep1, Name And Location stays available ==

  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep1/Basic/Attributes/PowerSource/Reported \n<font color=#00003C><b><i>Empty
  & mqtt_broker -> naming_service

  iot_service -> mqtt_broker: <font color=#6C2A0D>ucl/by-unid/node_1/ep1/NameAndLocation/Commands/WriteAttributes\n<font color=#6C2A0D><b>{"Location":"Living room"}
  & mqtt_broker -> naming_service

  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep1/NameAndLocation/Attributes/Location/Desired\n<font color=#00003C><b>{"value":"Living room"}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep1/NameAndLocation/Attributes/Location/Reported\n<font color=#00003C><b>{"value":"Living room"}

Endpoint ID list provided
-------------------------

If the entire list of Endpoint ID is published under the state cluster
(``EndpointIdList`` attribute), the name and location cluster SHOULD skip
monitoring existing endpoints based on publications under the
``ucl/by-unid/+/+/#`` topic filter, and rely exclusively on the value published
for the reported value of the ``EndpointIdList`` attribute

.. uml::

  'Switch to more legible color theme when all figures are ported to plantuml
  skinparam classFontColor black
  skinparam classFontSize 10
  skinparam classFontName Helvetica
  skinparam sequenceMessageAlign left
  skinparam shadowing false
  skinparam ArrowColor #000000
  skinparam ParticipantBackgroundColor #FFFFFF
  skinparam ParticipantBorderColor #001111
  skinparam SequenceLifeLineBorderColor #001111
  skinparam SequenceLifeLineBorderThickness 2
  skinparam NoteBackgroundColor #FFFFFF
  skinparam NoteBorderColor #000000
  hide footbox
  hide footbox

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  legend top
  <font color=#0039FB>MQTT Subscription</font>
  <font color=#008000>MQTT Unsubscription</font>
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  endlegend

  participant "Name and Location Service\nMQTT Client" as naming_service
  participant "MQTT Broker" as mqtt_broker
  participant "IoT Service" as iot_service
  participant "Protocol Controller\nUNID: pc_1" as pc_1
  participant "PAN Node\nUNID:node_1" as node_1

  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/State</font>
  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/State/Attributes/EndpointIdList/Reported</font>
  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/#</font>

  == Name and Location Service is ready ==

  rnote over pc_1, node_1: Node added to the\nnetwork and discovered.

  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/State <b>{Non-empty}
  & mqtt_broker -> naming_service

  rnote over naming_service: node_1 detected.\nAssuming 1 endpoint

  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/node_1/ep0/NameAndLocation/Commands/WriteAttributes</font>
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Desired\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Reported\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/SupportedCommands\n<font color=#00003C><b>{"value":["WriteAttributes"]}

  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep1/Basic/Attributes/PowerSource/Reported \n<font color=#6C2A0D><b>{Non-empty}
  & mqtt_broker -> naming_service

  rnote over naming_service: node_1 / ep1 detected.

  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/node_1/ep1/NameAndLocation/Commands/WriteAttributes</font>
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep1/NameAndLocation/Attributes/Name/Desired\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep1/NameAndLocation/Attributes/Name/Reported\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep1/NameAndLocation/SupportedCommands\n<font color=#00003C><b>{"value":["WriteAttributes"]}


  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/State/Attributes/EndpointIdList/Reported\n<font color=#00003C><b>{"value":[1,2]}
  & mqtt_broker -> naming_service

  rnote over naming_service: node_1:\n- ep0 does not exist.\n- ep1/ep2 detected.

  naming_service -> mqtt_broker: <font color=#008000>ucl/by-unid/node_1/ep0/NameAndLocation/Commands/WriteAttributes</font>
  naming_service -> mqtt_broker: <font color=#6C2A0D>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Desired</font>
  naming_service -> mqtt_broker: <font color=#6C2A0D>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Reported</font>
  naming_service -> mqtt_broker: <font color=#6C2A0D>ucl/by-unid/node_1/ep0/NameAndLocation/SupportedCommands</font>

  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/node_1/ep2/NameAndLocation/Commands/WriteAttributes</font>
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep2/NameAndLocation/Attributes/Name/Desired\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep2/NameAndLocation/Attributes/Name/Reported\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep2/NameAndLocation/SupportedCommands\n<font color=#00003C><b>{"value":["WriteAttributes"]}


Overlap with the Basic Cluster
------------------------------

Some PAN nodes are Basic Cluster Servers, and provide the ``LocationDescription``
attributes. If such an attribute is supported by a Basic Cluster Server,
the Name and Location Service MUST proxy its Location Attribute to the
Basic Cluster attribute.

.. uml::

  'Style matching the other figures in chapter.
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

  title Proxying the Basic cluster <b>LocationDescription</b>

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  legend top
  <font color=#0039FB>MQTT Subscription</font>
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  <font color=#000000>PHY Messages</font>
  endlegend

  participant "Name and Location Service\nMQTT Client" as naming_service
  participant "MQTT Broker" as mqtt_broker
  participant "IoT Service" as iot_service
  participant "Protocol Controller\nUNID: pc_1" as pc_1
  participant "PAN Node\nUNID:node_1" as node_1

  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/State</font>
  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/State/Attributes/EndpointIdList/Reported</font>
  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/#</font>

  == Name and Location Service is ready ==

  rnote over pc_1, node_1: Node added to the\nnetwork and discovered.

  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/State <b>{Non-empty}
  & mqtt_broker -> naming_service

  rnote over naming_service: node_1 detected.\nAssuming 1 endpoint

  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/node_1/ep0/NameAndLocation/Commands/WriteAttributes</font>
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Desired\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Reported\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Desired\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Reported\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/SupportedCommands\n<font color=#00003C><b>{"value":["WriteAttributes"]}

  == Protocol Controller publishes more as it interviews the node ==

  ...

  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/Basic/Attributes/LocationDescription/Desired\n<font color=#00003C><b>{"value":"Rooftop"}
  & mqtt_broker -> naming_service

  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Desired\n<font color=#00003C><b>{"value":"Rooftop"}

  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/Basic/Attributes/LocationDescription/Reported\n<font color=#00003C><b>{"value":"Rooftop"}
  & mqtt_broker -> naming_service

  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Reported\n<font color=#00003C><b>{"value":"Rooftop"}

  ...

  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/Basic/Attributes/LocationDescription/Desired\n<font color=#00003C><b>{"value":"Basement"}
  & mqtt_broker -> naming_service

  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Desired\n<font color=#00003C><b>{"value":"Basement"}

  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/Basic/Attributes/LocationDescription/Reported\n<font color=#00003C><b>{"value":"Basement"}
  & mqtt_broker -> naming_service

  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Reported\n<font color=#00003C><b>{"value":"Basement"}

  ...


  iot_service -> mqtt_broker: <font color=#6C2A0D>ucl/by-unid/node_1/ep1/NameAndLocation/Commands/WriteAttributes\n<font color=#6C2A0D><b>{"Location":"Kitchen"}
  & mqtt_broker -> naming_service

  naming_service -> mqtt_broker: <font color=#6C2A0D>ucl/by-unid/node_1/ep0/Basic/Commands/WriteAttributes\n<font color=#6C2A0D><b>{"LocationDescription":"Kitchen"}
  & mqtt_broker -> pc_1

  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/Basic/Attributes/LocationDescription/Desired\n<font color=#00003C><b>{"value":"Kitchen"}
  & mqtt_broker -> naming_service

  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Desired\n<font color=#00003C><b>{"value":"Kitchen"}

  pc_1 -> node_1: WriteAttributes
  node_1 -> pc_1: WriteAttributes Response

  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/Basic/Attributes/LocationDescription/Reported\n<font color=#00003C><b>{"value":"Kitchen"}
  & mqtt_broker -> naming_service

  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Reported\n<font color=#00003C><b>{"value":"Kitchen"}


Pushing the information to the end node
---------------------------------------

The Name and Location Service is provided on behalf of all PAN nodes,
meaning that PAN node will not be aware that they have a name assigned.

It is possible for Protocol Controllers to push the information down
to PAN nodes, if they want to make sure that they would retain this
information locally.

If a Protocol Controller intends to do so, it SHOULD subscribe to
the Reported ``Name`` attribute in the NameAndLocation cluster and
assign it to the node when it is updated.

Protocol Controller MUST NOT try to push the ``Location`` attribute to a
PAN node. It SHOULD show the PAN node with the ``LocationDescription``
Attribute supported in the Basic Cluster.


.. uml::

  'Style matching the other figures in chapter.
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  legend top
  <font color=#0039FB>MQTT Subscription</font>
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  <font color=#000000>PHY messages</font>
  endlegend

  participant "Name and Location Service\nMQTT Client" as naming_service
  participant "MQTT Broker" as mqtt_broker
  participant "Protocol Controller\nUNID: pc_1" as pc_1
  participant "PAN Node\nUNID:node_1" as node_1

  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/#</font>

  == Name and Location Service is ready ==

  rnote over pc_1, node_1: Node added to the\nnetwork and discovered.

  pc_1 -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/NameAndLocation/Attributes/+/Reported</font>

  pc_1 -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/State <b>{Non-empty}
  & mqtt_broker -> naming_service

  rnote over naming_service: node_1 detected.\nAssuming 1 endpoint

  naming_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/node_1/ep0/NameAndLocation/Commands/WriteAttributes</font>
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Desired\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Reported\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Desired\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Location/Reported\n<font color=#00003C><b>{"value":""}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/SupportedCommands\n<font color=#00003C><b>{"value":["WriteAttributes"]}

  == Name was updated ==

  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Desired\n<font color=#00003C><b>{"value":"Wall outlet"}
  naming_service -> mqtt_broker: <font color=#00003C>ucl/by-unid/node_1/ep0/NameAndLocation/Attributes/Name/Reported\n<font color=#00003C><b>{"value":"Wall outlet"}
  & mqtt_broker -> pc_1

  rnote over pc_1: push the information\nto the node

  pc_1 -> node_1: Your name is "Wall outlet"
  node_1 -> pc_1: Name updated.

