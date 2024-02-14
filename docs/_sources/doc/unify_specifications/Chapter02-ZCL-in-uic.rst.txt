.. meta::
  :description: Dotdot ZCL in Unify Framework for Application level control
  :keywords: Application, control, dotdot

.. |ZCL2019| replace:: `ZCL 2019 <https://zigbeealliance.org/wp-content/uploads/2019/12/07-5123-06-zigbee-cluster-library-specification.pdf>`__

.. raw:: latex

 \newpage

.. _unify_specifications_chapter_zcl_in_unify_application_level_control:

Dotdot ZCL in Unify Framework for Application Level Control
=================================================================================

Introduction
------------

This chapter provides application examples about the usage of
the DotDot Zigbee cluster library specification |ZCL2019|
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
:ref:`unify_specifications_chapter_service_discovery`.

After the PAN nodes capabilities have been discovered, the IoT service MQTT
clients can control and observe the behavior of the nodes using the unified controller application
language. The unified application language is based on a publish/subscribe
messaging transport protocol and the dotdot/ZigBee Cluster library (ZCL)
specification.

MQTT Topics
-----------

The MQTT topics are divided into sub-spaces for each device to allow a Protocol
Controller to advertise state and capabilities of a given device and IoT
Services to read the state and control a given device.

There are three main spaces in a cluster MQTT topic, as follows:

* ``ucl/by-unid/+/+/<ClusterName>/Attributes/#`` : This represents the
  cluster attributes state.
* ``ucl/by-unid/+/+/<ClusterName>/SupportedCommands``: This represents the
  list of commands that the node can receive for this cluster.
* ``ucl/by-unid/+/+/<ClusterName>/Commands/+`` : This is the topic where
  IoT Services issue commands.
* ``ucl/by-unid/+/+/<ClusterName>/SupportedGeneratedCommands``: This represents
  the list of commands that the node can generate for this cluster.
* ``ucl/by-unid/+/+/<ClusterName>/GeneratedCommands/+`` : This is the topic where
  IoT Services receives commands from PAN nodes.

The ``/+/+`` wildcards can be replaced with a specific UNID and endpoint to
control/monitor only a single endpoint.

An overview of these topics is shown in
:numref:`table_unify_specifications_mqtt_topic_space_overview`.

.. list-table:: MQTT Topic space overview
  :name: table_unify_specifications_mqtt_topic_space_overview
  :widths: 60 10 10 20
  :width: 50%
  :header-rows: 1

  * - MQTT Topic
    - Protocol

      Controllers
    - IoT

      Services
    - Description
  * - ucl/by-unid/<UNID>/ep<EndPointID>/<ClusterName>/Attributes/<AttributeName>/Reported
    - Publish

      (retained)
    - Subscribe
    - Advertises the reported value of an attribute for the Cluster.
  * - ucl/by-unid/<UNID>/ep<EndPointID>/<ClusterName>/Attributes/<AttributeName>/Desired
    - Publish

      (retained)
    - Subscribe
    - Advertises the desired value of an attribute for the Cluster.
  * - ucl/by-unid/<UNID>/ep<EndPointID>/<ClusterName>/Attributes/#
    - Publish

      (retained)
    - Subscribe
    - Advertises the reported/desired value of a non-standard attribute

      for the Cluster. Non-standard attributes are defined in this

      specification. The last level of this topic space will always

      be /Reported or /Desired.
  * - ucl/by-unid/<UNID>/ep<EndPointID>/<ClusterName>/SupportedCommands
    - Publish

      (retained)
    - Subscribe
    - Advertises an array of commands that can be published under

      the Commands/# namespace by IoT Services to control the device.
  * - ucl/by-unid/<UNID>/ep<EndPointID>/<ClusterName>/Commands/<CommandName>
    - Subscribe
    - Publish

      (unretained)
    - Instruct a Protocol Controller to send a command

      to the UNID/endpoint server cluster.
  * - ucl/by-unid/<UNID>/ep<EndPointID>/<ClusterName>/SupportedGeneratedCommands
    - Publish

      (retained)
    - Subscribe
    - Advertises an array of commands that can be generated under

      the GeneratedCommands/# namespace by Protocol Controller

      as commands sent from the device.
  * - ucl/by-unid/<UNID>/ep<EndPointID>/<ClusterName>/GeneratedCommands/<CommandName>
    - Publish

      (unretained)
    - Subscribe
    - Receive commands sent from the UNID/endpoint cluster to Unify.

.. _unify_specifications_section_reported_and_desired:

Reported and Desired
''''''''''''''''''''

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
found in the
`Amazon AWS Specification <https://docs.aws.amazon.com/iot/latest/developerguide/device-shadow-document-syntax.html#device-shadow-example-request-json>`__.

Cluster MQTT Topics
'''''''''''''''''''

IoT Services (Cloud Service connectors, Middleware Translator, and Rules Engines)
should subscribe to the following topic filter to get any application message
corresponding to the state of given cluster from any PAN node in the
Unify network:

.. code-block:: mqtt

  ucl/by-unid/+/+/<ClusterName>/#


The Protocol Controllers MUST then publish to the following topics to advertise
the supported attributes of a PAN Node and also when the attributes state is
updated:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndPointID>/<ClusterName>/Attributes/<AttributeName>/Reported - { "value": 23 }

For each attribute, the value property is required. This represents the value
of the attribute and it can be any of the following JSON types:

* string
* number
* boolean
* null
* an object, containing boolean types (only for bitmasks)

Strings must always be UTF-8 encoded, have a maxlength of 256 bytes, regardless of the
limits specified in ZCL.

For example, a Protocol Controller advertising the state of a device/endpoint
being a OnOff and Level cluster server will publish the following:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Desired - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Reported - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/Level/Attributes/CurrentLevel/Desired - {"value": 50}
  ucl/by-unid/<UNID>/ep<EndpointId>/Level/Attributes/CurrentLevel/Reported - {"value": 100}


Furthermore, in the example above, the PAN node is dimming its level from 100% to
50%. When it reaches 50% and the Protocol Controller has received a confirmation,
the reported state of the CurrentLevel attribute will be updated to match the
desired value.

Strings MUST be used by Protocol Controllers when a descriptive name is
given to the possible values for a given attribute by the DotDot XML files.
For example, the *LockState* attribute of a door lock MUST be published as
follows:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/Lock/Attributes/LockState/Desired - {"value": "Locked"}

If the individual values do not have a description string name, their value MUST
be presented using either a JSON number or boolean. See the
ZCL Cluster Library |ZCL2019|.

Finally, only JSON objects are allowed for bitmaps/bitmasks when names are
defined in the ZCL Cluster Library |ZCL2019|. Numbers MUST
NOT be used in this case. For example, the ColorCapabilities attribute of the
ColorControl cluster MUST be represented as follows:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/ColorControl/Attributes/ColorCapabilities/Reported -
  {
    "value": {
      "HueSaturationSupported": true,
      "EnhancedHueSupported": true,
      "ColorLoopSupported": true,
      "XYSupported": true,
      "ColorTemperatureSupported": true
    }
  }

Mandatory Cluster Attributes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

PAN Nodes may support more than the minimum mandatory set of attributes for a
given cluster. When a node is interviewed and its capabilities are discovered,
a Protocol Controller MUST, at a minimum, publish all attributes for a cluster
that are specified as *mandatory* in the ZigBee Cluster library.

For example, for a PAN Node supporting the OnOff cluster, it will be guaranteed
that the *OnOff* attribute will be published, however, the
*GlobalSceneControl* will be published only if supported by the PAN node.

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Desired - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Reported - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/GlobalSceneControl/Desired - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/GlobalSceneControl/Reported - {"value": true}

Missing Mandatory Cluster Attributes and Commands
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Because of multiple PHYs and the application level variety between them,
a node might be shown as a ZCL cluster server, despite missing mandatory
attributes and/or commands. A protocol controller MUST do whatever it can to
map or emulate all required attributes and commands. Only in situations
where there is no meaningful way to do a map, the protocol controller may
omit the mapping.

For example, in Z-Wave, it is possible to have Thermostats without a local
temperature reading. These thermostats can nonetheless be controlled by MQTT
clients and the lack of a temperature reading should not prevent the exposition
of such a device to the IoT Services.

In this case, the value of the attribute MUST be set to a null JSON type. For
instance, a thermostat without local temperature MAY be published as follows:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/LocalTemperature/Desired - {"value": null}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/LocalTemperature/Reported - {"value": null}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/OccupiedCoolingSetpoint/Desired - {"value": 2600}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/OccupiedCoolingSetpoint/Reported - {"value": 2600}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/OccupiedHeatingSetpoint/Desired - {"value": 2100}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/OccupiedHeatingSetpoint/Reported - {"value": 2100}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/SystemMode/Desired - {"value": "Auto"}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/SystemMode/Reported - {"value": "Auto"}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/ThermostatProgrammingOperationMode/Desired - {"value": 0}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/ThermostatProgrammingOperationMode/Reported - {"value": 0}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/ControlSequenceOfOperation/Desired - {"value": 4}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/ControlSequenceOfOperation/Reported - {"value": 4}

In any case, a Protocol Controller SHOULD publish the *LocalTemperature* attribute
state when publishing a node as a Thermostat Cluster server because it is a
mandatory attribute.

When a reported state is updated from the PAN node, the Protocol
Controllers MUST update both the reported and desired values of the
corresponding attributes. In this case, Protocol Controllers SHOULD publish the
desired part of the updated state first.

Cluster Revisions
~~~~~~~~~~~~~~~~~

A special attribute under every cluster called ClusterRevision is used to
advertise the ZCL revision of the cluster that the node and/or Protocol
Controller are using.

ProtocolControllers MUST publish this information for all clusters of all
nodes they advertise.

See the ZCL Cluster Library |ZCL2019| for Cluster
revision details.

The JSON value type for the ClusterRevision attribute MUST be an integer.
Both Desired and Reported values MUST be published by Protocol Controllers.
The ClusterRevision is a read-only attribute.

For example, publishing the cluster revision of an OnOff node:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Desired - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Reported - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/ClusterRevision/Desired - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/ClusterRevision/Reported - {"value": 2}


Additional Attributes
~~~~~~~~~~~~~~~~~~~~~

Additional attributes under a cluster state can be used to advertise state
information that is not standardized as Cluster attributes.

The format of this additional state information is UCL-specific and defined in
this specification. It follows as closely as possible the data models described
in the ZCL Cluster Library |ZCL2019|.

The additional state information can be affected by the use of commands and
it MUST also follow a desired/reported state. Most clusters do not have any
additional attributes defined.

Additional attributes MUST be published under the same ``Attributes/`` MQTT topic
hierarchy as the standard cluster attributes, however, the MQTT topic hierarchy
MAY contain additional levels before the final Desired or Reported level.

For example, the Scene Cluster definition uses a Scene Table, which
is advertised to the IoT Services using a UCL-specific definition.
This specification will provide a JSON schema for all Additional Attributes
not present in the dotdot XML files.

For example, from the :ref:`unify_specifications_chapter_scenes`, this is the
JSON schema for the SceneTable:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/SceneTable/Desired -
  {
    "value": [
      {
        "SceneID": 1,
        "GroupID": 12,
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
      },
      {
        "SceneID": 2,
        "GroupID": 0,
        "SceneName": "Away from home",
        "TransitionTime": 0,
        "TransitionTime100ms": 0,
        "SceneTableExtensions": {
          "OnOff": {
            "OnOff": false
          },
          "Level": {
            "CurrentLevel": 0
          }
        }
      }
    ]
  }

Commands Supported by PAN Node
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

IoT Services MUST be able to read the commands that are supported for a cluster
server under a UNID/Endpoint to control them.

The list of supported Commands for each PAN node MUST be advertised by the Protocol
Controllers in the *SupportedCommands* topic under the Cluster level.

The list of supported Commands MUST be advertised using a JSON array containing
strings. For example, the list of supported commands for the OnOff cluster could
look like this:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/SupportedCommands -
  {
    "value": ["On","Off","Toggle","ForceReadAttributes","WriteAttributes"]
  }

It is OPTIONAL for a Protocol Controller to publish the availability of the
*WriteAttributes* and *ForceReadAttributes* commands.

.. note::

  A ProtocolController MAY publish an empty array for the SupportedCommands
  of a cluster. In this case, IoT Services can only use the WriteAttributes commands
  if the cluster contains writable attributes.

.. note::

  IoT Services MUST NOT wait for a feedback after sending a ForceReadAttributes command.
  This command is used to *force* an update on the usual feedback channels - i.e.,
  the *reported* MQTT topic.

Protocol Controllers SHOULD explicitly advertise the availability of the
*WriteAttributes* Command in the SupportedCommands topic when there is at least
one writable attribute supported by the PAN node
(i.e., marked as ``writable=true`` in the ZCL cluster definition).

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

Sending Supported Commands to PAN Node
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When a PAN node is included to the network, a Protocol Controller servicing
the node MUST subscribe to the following topic filter to receive
incoming Commands:

> ``ucl/by-unid/<UNID>/ep<EndpointId>/<ClusterName>/Commands/+``

It is OPTIONAL for a Protocol Controller to subscribe to the command topic
filter only if the PAN node supports no commands and all attributes are read-only
attributes (i.e., the WriteAttributes command will have no effect).

When the IoT Services (AWS IoT connectors, Web-based UIs, Rules Engines,
Data Loggers, and so on.) want a PAN node to execute a given command, the Clients can
publish a command payload using the respective command topic:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/<ClusterName>/Commands/<CommandName> -
  {
    "FieldName1": "value",
    "FieldName2": "value"
  }


Commands Generated by PAN Node
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

IoT Services SHOULD be able to read the commands that are generated by a
cluster under a UNID/Endpoint to listen to unsolicited events and responses.

The list of generated commands for each PAN node SHOULD be advertised by the
Protocol Controllers in the *SupportedGeneratedCommands* topic under the Cluster level.

The list of generated commands SHOULD be advertised using a JSON array
containing strings. For example, the list of generated commands for the OnOff
client cluster could look like this:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/SupportedGeneratedCommands -
  {
    "value": ["On","Off","Toggle"]
  }

.. note::

  No publications to this topic is equivalent to the Protocol Controller
  publishing an empty array of strings to indicate no commands can be generated
  by this PAN node.

Receiving Generated Commands from PAN Node
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When a PAN node is included to the network, an IoT Service monitoring
the node SHOULD subscribe to the following topic filter to receive
incoming commands from the PAN node:

> ``ucl/by-unid/<UNID>/ep<EndpointId>/<ClusterName>/GeneratedCommands/+```

It is OPTIONAL for a Protocol Controller to publish to the command topic
since not all PAN nodes can generate commands. IoT Services SHOULD NOT expect
to see publications for all PAN nodes on this topic filter.

When the IoT Services (AWS IoT connectors, Web-based UIs, Rules Engines,
Data Loggers, and so on.) want to receive commands from a PAN node, the Clients can
subscribe to the topic and parse the command payload:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/<ClusterName>/GeneratedCommands/<CommandName> -
  {
    "FieldName1": "value",
    "FieldName2": "value"
  }

Command Payloads
~~~~~~~~~~~~~~~~

If the command does not have any mandatory fields, an empty JSON object
(i.e., {}) MUST be sent as a payload.

If the command has mandatory fields, the payload MUST contain all of
them. For example, in the 'Level' cluster, the 'MoveToLevel' command consists
of two mandatory fields (i.e., 'Level' and 'Transition Time' ).
In this case, the payload MUST be: ``{"Level": 100, "TransitionTime": 0}``.

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
ZCL Cluster Library |ZCL2019|.

Finally, only JSON objects are allowed for bitmaps/bitmasks when names are
defined in the ZCL Cluster Library |ZCL2019|. Numbers MUST
NOT be used in this case. For example, the UpdateFlags field of the
ColorLoopSet command in the ColorControl cluster MUST be represented as
follows:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/ColorControl/Commands/ColorLoopSet -
  {
    "UpdateFlags" : {
      "UpdateAction": true,
      "UpdateDirection": true,
      "UpdateTime": true,
      "UpdateStartHue": false
    }
  }

WriteAttributes Generic Command
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A Protocol Controller MUST always publish the desired/reported state of all
mandatory attributes in a cluster, using the Cluster state topic space.
An attribute state can be changed in three ways:

* Locally on the PAN node
* Using a specific command as described above
* Using a generic command called 'WriteAttributes'

For example, an IoT Service MAY issue the following payload to modify
the desired values of writable attributes directly:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndPointID>/<ClusterName>/Commands/WriteAttributes -
  {
    "AttributeName1": "value",
    "AttributeName2": "value"
  }

Note that all PAN nodes, which support writable attributes MUST be writable directly
using the *WriteAttributes* Command. Read-only attributes received in this
Command will be ignored. See |ZCL2019| for details
about writable/read-only attributes.

To ensure an end-to-end quality of service, a Protocol Controller MUST always
publish the attributes desired/reported state when it detects a state update.
IoT Services can therefore see the effect of their published commands by
subscribing to the cluster *Attributes* space.

Non-standard attribute cannot be updated or modified using the *WriteAttributes*
command.

ForceReadAttributes Generic Command
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The *ForceReadAttributes* command is used to force an update of specified
attributes that may not be properly reported using standard reporting mechanisms.

The Protocol Controllers are expected to keep track of the state of nodes
attributes as much as they possibly can, without degrading network performance.

If an IoT Service wants to double check the state of an attribute, it can use
the **ForceReadAttributes**. On reception of this command, Protocol Controllers
SHOULD request the latest available data from the network for each attribute
specified in the command. The data SHOULD be published on the *reported*
MQTT topic for each attribute.

**ForceReadAttributes** should be used sparingly because it can generate significant
radio traffic. Protocol Controllers should by default
guarantee a good trade off between timely updates and radio traffic without the
use of **ForceReadAttributes**.

If the newly read value of an attribute matches its previously
reported value, a new MQTT message MAY not be published.

If the payload of the received MQTT command is empty, the Protocol Controller
SHOULD request new data for all supported attributes of that cluster.
For example, an IoT Service MAY issue the following payload to directly read
several attributes of a given cluster

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndPointID>/<ClusterName>/Commands/ForceReadAttributes -
  {
    "value" :
      [
        "AttributeName1",
        "AttributeName2",
        "AttributeName6"
      ]
  }

In response, the Protocol Controller should update the 'reported' MQTT topics for
'AttributeName1' and 'AttributeName2', as discussed in the
:ref:`unify_specifications_section_reported_and_desired`.

Another example, an IoT Service MAY issue the following payload to directly read
ALL the attributes of a given cluster

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndPointID>/<ClusterName>/Commands/ForceReadAttributes -
  {
    "value" : []
  }

In response, the Protocol Controller should update ALL available attributes
for *ClusterName*, as specified in |ZCL2019|.

Note that Protocol Controller may not support the *ForceReadAttributes*
for any attribute under a cluster. For example, it can be that a Protocol Controller
is able to force read Scenes Cluster attributes but not the SceneTable.

*ForceReadAttributes* is EXPLICITLY not supported for groups or multicast due
to network performance considerations.

Observing the effects of a command
----------------------------------

An IoT service MAY follow the effect of a published (write-type) command by
observing the associated attributes.

Read type of commands, like ForceReadAttributes, ViewGroup, ViewScene, MAY trigger
Protocol Controllers to interact with PAN nodes, but there will be no guaranteed
feedback on MQTT.

.. note::

  |ZCL2019| is not always fully explicit on how commands and their fields will
  affect the value of cluster attributes. The Unify SDK provides a shared
  components that links incoming commands to attribute updates, so updates to
  desired will be uniform across Protocol Controllers.

Let's consider an OnOff Server PAN node. The Protocol Controller MUST publish
its supported attributes and supported commands as a minimum:

.. code-block:: mqtt

  # Initial publications by the Protocol Controller
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/ClusterRevision/Desired - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/ClusterRevision/Reported - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Desired - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Reported - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/SupportedCommands -  {"value": ["Off", "On", "Toggle"]}

  # IoT Service publishes:
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Commands/Off -  { }

  # Protocol Controller MUST acknowledge reception of the command by updating all affected attributes
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Desired - {"value": false}

  # Protocol Controller Communicates with the node...

  # Protocol Controller MUST update the affected attributes again, as part of the outcome of the command
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Reported - {"value": false}

If a Protocol Controller fails to execute a command or if the node reports that
the OnOff attribute is still On (true), the Protocol Controller MUST roll back
the desired value, so that the desired and reported values match again.

Whenever an attribute has a Desired/Reported value mismatch, an IoT Service
can assume that either:

* The Protocol Controller is trying to communicate with the node to set the values
* The PAN node is performing a transition to the desired value that takes time.

IoT Services SHOULD use the WriteAttributes commands whenever available if they
wish to keep a high level of certainty between issued commands and their effect
on attributes.

PAN Nodes capabilities vs ZCL Compliance
----------------------------------------

Protocol Controllers SHOULD do their best to have the devices they expose
resemble devices that comply with the |ZCL2019|.
It can be simulating attributes, entire clusters, or manipulating values
sent by end nodes.

Protocol Controllers SHOULD simulate cluster servers if needed, so that devices
behave as closely to the actual set of defined Device Types in |ZCL2019|.

Clusters for Lights, Switches, DoorLock, Thermostat, and Occupancy Sensor Applications
--------------------------------------------------------------------------------------

The following subsections provide specific cluster examples using the Cluster
state and Commands topics, such as Dimmer lighting, Switch, Door Lock,
Thermostat, and Occupancy Sensor applications. The detailed description of each
cluster can be found in |ZCL2019|.

Each cluster has a list of mandatory attributes and commands. All mandatory
attributes in a given cluster MUST be advertised by a Protocol Controller for
all devices/endpoints. All mandatory Commands SHOULD be advertised in the
SupportedCommand topic, when possible.

Groups Cluster (0x0004)
'''''''''''''''''''''''

This cluster is used to define group memberships for PAN nodes.

This cluster requires additional attributes, because the *Group list* is not captured
by its defined attributes. See |ZCL2019| for the
*Groups* cluster definition.

Each group is identified using a 16 bits unsigned integer value.
Below is an example of a PAN node Group Cluster server:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/Groups/Attributes/ClusterRevision/Desired - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/Groups/Attributes/ClusterRevision/Reported - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/Groups/Attributes/NameSupport/Desired - {"value": {"Supported":true}}
  ucl/by-unid/<UNID>/ep<EndpointId>/Groups/Attributes/NameSupport/Reported - {"value": {"Supported":true}}
  ucl/by-unid/<UNID>/ep<EndpointId>/Groups/Attributes/GroupList/Desired - {"value": [1, 2] }
  ucl/by-unid/<UNID>/ep<EndpointId>/Groups/Attributes/GroupList/Reported - {"value": [1] }
  ucl/by-unid/<UNID>/ep<EndpointId>/Groups/Attributes/1/Name/Desired - {"value": "Kitchen nodes"}
  ucl/by-unid/<UNID>/ep<EndpointId>/Groups/Attributes/1/Name/Reported - {"value": "Kitchen nodes"}
  ucl/by-unid/<UNID>/ep<EndpointId>/Groups/Attributes/2/Name/Desired - {"value": "Light bulbs"}
  ucl/by-unid/<UNID>/ep<EndpointId>/Groups/SupportedCommands - {"value": ["AddGroup", "ViewGroup", "GetGroupMembership", "RemoveGroup", "RemoveAllGroups", "AddGroupIfIdentifying"] }


The only mandatory attribute in the group cluster is the NameSupport attribute.
If a PAN node supports name for groups, it will advertise the assigned names
for each of its groups under the following topics:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<id>/Groups/Attributes/<GroupID>/Name/Reported
  ucl/by-unid/<UNID>/ep<id>/Groups/Attributes/<GroupID>/Name/Desired

The names assigned to each group will be a value containing a string
as in the AddGroup Command GroupName field.

**JSON Schema:**

.. code-block:: json

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

The list of groups that a node is part of will be advertised under the GroupList
topic. Note that GroupID 0 is not a valid group and MUST NOT be used.

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<id>/Groups/Attributes/GroupList/Desired
  ucl/by-unid/<UNID>/ep<id>/Groups/Attributes/GroupList/Reported

**JSON Schema:**

.. code-block:: json

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


Scenes Cluster (0x0005)
'''''''''''''''''''''''

The usage of this cluster for application level control is described in
:ref:`unify_specifications_chapter_scenes`.

On/Off Cluster (0x0006)
'''''''''''''''''''''''

Here is an example of OnOff Server PAN node with the minimum required attributes
and commands.

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/ClusterRevision/Desired - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/ClusterRevision/Reported - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Desired - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Reported - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/SupportedCommands -  {"value": ["Off", "On", "Toggle"]}

Level Cluster(0x0008)
'''''''''''''''''''''

The level cluster controls the light brightness level,
the door closure level, or the heater power output.

|ZCL2019| indicates that Level Servers must also be OnOff servers.
Here is a minimal example:

.. code-block:: mqtt

  # OnOff Cluster
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/ClusterRevision/Desired - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/ClusterRevision/Reported - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Desired - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Reported - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/SupportedCommands -  {"value": ["Off", "On", "Toggle"]}

  # Level Cluster
  ucl/by-unid/<UNID>/ep<EndpointId>/Level/Attributes/ClusterRevision/Desired - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/Level/Attributes/ClusterRevision/Reported - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/Level/Attributes/CurrentLevel/Desired - {"value": 255}
  ucl/by-unid/<UNID>/ep<EndpointId>/Level/Attributes/CurrentLevel/Reported - {"value": 255}
  ucl/by-unid/<UNID>/ep<EndpointId>/Level/SupportedCommands -  {"value": ["MoveToLevel", "Move", "Step", "Stop", "MoveToLevelWithOnOff", "MoveWithOnOff", "StepWithOnOff", "StopWithOnOff"]}

Note that the OnOff cluster command will affect the level value.

.. code-block:: mqtt

  # IoT Service publishes:
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Commands/Off - { }

  # Protocol Controller acknowledges:
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Desired - {"value": false}

  # Communication ongoing ...

  # Protocol Controller updates the state of the PAN node:
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Reported - {"value": false}
  ucl/by-unid/<UNID>/ep<EndpointId>/Level/Attributes/CurrentLevel/Desired - {"value": 0}
  ucl/by-unid/<UNID>/ep<EndpointId>/Level/Attributes/CurrentLevel/Reported - {"value": 0}

  # Alternatively, Level commands May affect OnOff attributes:
  ucl/by-unid/<UNID>/ep<EndpointId>/Level/Commands/MoveToLevelWithOnOff - {"Level":45,"TransitionTime":0,"OptionsMask":{"ExecuteIfOff":false,"CoupleColorTempToLevel":false},"OptionsOverride":{"ExecuteIfOff":false,"CoupleColorTempToLevel":false}}
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Desired - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/Level/Attributes/CurrentLevel/Desired - {"value": 45}
  # Communication ongoing ...
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Reported - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/Level/Attributes/CurrentLevel/Reported - {"value": 255}

Note that the IoT Service SHOULD assume default values for the MinLevel/MaxLevel
if they are not published/advertised for a node.

Door Lock Cluster (0x0101)
''''''''''''''''''''''''''

This cluster is used as a generic interface to secure a door. The Door Lock cluster in 
UCL is based on the Matter version 1.2 specification. The Matter Door Lock
cluster contains a lot of optional attributes, such as logging events,
alarms, security setting, and so on and commands, such as setting user PIN, getting
log events, setting schedules, and so on. However, this section lists only the mandatory
attributes and commands used for basic door operations.

Mandatory Attributes:

* LockState (0x0000): NotFullyLocked (0x00), Locked (0x01), Unlocked (0x02), Unlatched (0x03), and Undefined (0xFF) are the possible attribute values that describe the state of the door.
* LockType(0x0001): defines the door lock types such as Deadbolt (0x00), Magnetic (0x01), Other (0x02), Latch Bolt (0x05), etc.
* ActuatorEnabled (0x002): defines if the actuator is Enabled (true) or Disabled (false).
* OperatingMode (0x0025): defines the current operating mode of the door lock, such as Normal (0x00), Vacation (0x01), Privacy (0x02), etc.
* SupportedOperatingModes (0x0026): defines the operating modes that are supported by the lock, represented in MQTT as an array of strings. For e.g.
  
.. code-block:: mqtt
  
  ucl/by-unid/<UNID>/ep<EndpointId>/DoorLock/Attributes/SupportedOperatingModes/Actual - { "value": [ "Normal", "Vacation" ] }
  ucl/by-unid/<UNID>/ep<EndpointId>/DoorLock/Attributes/SupportedOperatingModes/Desired - { "value": [ "Normal", "Vacation" ] }

.. note:: Protocol controllers MAY implement all the mandatory elements, the implementation is recommended.

Mandatory Request Commands (client to a device):

* LockDoor (0x00): a command used to lock a door with an optional PINOrRFIDCode parameter value.
* UnlockDoor (0x01): a command used to unlock a door with an optional PINOrRFIDCode parameter value.

.. note::

  If the RequirePINForRemoteOperation attribute is true, PINOrRFIDCode shall be a mandatory parameter 
  in the above commands and other commands that require it.

For more details on the UCL Door Lock cluster based on Matter, see :ref:`unify_specifications_chapter_unify_controller_language_data_model`.

Thermostat Cluster (0x0201)
'''''''''''''''''''''''''''

This cluster defines the interface to thermostats.
The ZCL Thermostat cluster contains a lot of optional attributes, such as
Outdoor Temperature, Occupancy, PID configuration, and so on, and a few
optional commands used primarily for scheduling.
However, this section only covers the (only) mandatory attribute,
LocalTemperature, as well as the (only) mandatory command,
Setpoint Raise/Lower.

.. note::

  In the ZCL Thermostat Cluster, all temperatures are in **Celcius (°C)**
  and any necessary conversion to/from, for example, Fahrenheit °F must be
  done by the Unify components (Protocol Controllers and GUI).

Mandatory Attributes
~~~~~~~~~~~~~~~~~~~~

The following attributes are mandatory for the ZigBee thermostat cluster.

* LocalTemperature (0x0000): The temperature in **Celsius (°C)** multiplied
  by 100 measured by the Thermostat. This gives a resolution of 0.01°C.
  This is mapped from the Multilevel Sensor Report for the "Air temperature"
  sensor type.

* OccupiedCoolingSetpoint (0x0011) indicates the configured temperature
  setpoint for the thermostat when it operates in Cooling mode. This value
  is writable directly, which is supported only if the node supports
  0x02 (COOL) or 0x03 (AUTO) modes.

* OccupiedHeatingSetpoint (0x0012) indicates the configured temperature
  setpoint for the thermostat when it operates in Heating mode. This value
  is writable directly, which is supported only if the node supports 0x01
  (HEAT) or 0x03 (AUTO) modes.

* SystemMode (0x001C) indicates the current operating mode in the system.
  Note that all modes are not supported. Only 0 (Off) is supported by default.
  Other modes are supported only if the respective Z-Wave mode is supported
  (Auto, Cool and Heat).

* ControlSequenceOfOperation (0x001B): This indicates the supported SystemModes
  and restricts which ones can be used. This attribute should be writable,
  but not all values are mandatory to support. To start with, this attribute
  will be mapped based on the supported modes in the Z-Wave Thermostat Mode.
  Only values 0x00, 0x02 or 0x04 will be used and the "With reheat" option
  will not be used. Since only 1 value is supported, it will appear as if
  this attribute is read-only.

Read only attributes:

* ThermostatProgrammingOperationMode (0x0025): This attribute is read only,
  showing the thermostats' capabilities. Z-Wave Thermostats will have
  SimpleOrSetpoint support to start with. When the Schedule Command Class
  is supported, the Schedule bit will be updated to 1.

Typically, the state topic for the Thermostat cluster will look like this:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/ClusterRevision/Desired - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/ClusterRevision/Reported - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/LocalTemperature/Desired - {"value": 2300}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/LocalTemperature/Reported - {"value": 2300}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/OccupiedCoolingSetpoint/Desired - {"value": 2600}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/OccupiedCoolingSetpoint/Reported - {"value": 2600}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/OccupiedHeatingSetpoint/Desired - {"value": 2100}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/OccupiedHeatingSetpoint/Reported - {"value": 2100}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/SystemMode/Desired - {"value": "Auto"}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/SystemMode/Reported - {"value": "Auto"}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/ThermostatProgrammingOperationMode/Desired - {"value": 0}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/ThermostatProgrammingOperationMode/Reported - {"value": 0}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/ControlSequenceOfOperation/Desired - {"value": 4}
  ucl/by-unid/<UNID>/ep<EndpointId>/Thermostat/Attributes/ControlSequenceOfOperation/Reported - {"value": 4}


Mandatory Commands
~~~~~~~~~~~~~~~~~~

Only one command is mandatory command (alongside WriteAttributes), which allows
changing the operating mode and applying an offset to the currently-configured
setpoints.

* SetpointRaiseOrLower Command (0x00).

This command is used to change the Mode and adjust the setpoint(s).
For example, to change to heat mode and increase by 1 degree:

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<id>/Thermostat/Commands/SetpointRaiseOrLower -
  {
    "Mode": "Heat",
    "Amount": 100
  }

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

.. code-block:: mqtt

  ucl/by-unid/<UNID>/ep<id>/Thermostat/Commands/WriteAttributes -
  {
    "SystemMode": "Auto",
    "OccupiedCoolingSetpoint": 3200,
    "OccupiedHeatingSetpoint": 2200
  }

Note that it is not possible to detect which SystemModes are supported or if
the setpoints will be accepted. For example, in most cases the
`ControlSequenceOfOperation` will have only 1 valid value. In this case, the
cluster state topic will reflect back that no change was applied.

Occupancy Sensing Cluster (0x0406)
''''''''''''''''''''''''''''''''''

The cluster defines an interface to occupancy sensing functionality including
configuration and getting the occupancy status. Here, only the mandatory
attributes that can enable the basic functionality of occupancy sensing are
described.

Mandatory Attributes:

* Occupancy (0x0000): is a bitmap and Bit 0 defines the sensed occupancy
  (Occupied = 1 and Unoccupied = 0)
* OccupancySensorType (0x0001): defines the occupancy sensor, and the
  values can be PIR (0x00), Ultrasonic(0x01), PIR and ultrasonic(0x02) and
  Physical contact(0x03).

There are no supported command for Occupancy Sensing cluster. The
*WriteAttributes* command will have no effect.

Examples of Application Level Control Using Unify Framework
-----------------------------------------------------------

Turning On the On/Off Light Device using Unify Framework
''''''''''''''''''''''''''''''''''''''''''''''''''''''''

The following diagram shows an example of On/Off Cluster control using the
Unify framework.

.. uml::

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  ' Style for the diagram
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

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

  protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/SupportedCommands \n<font color=#00003C><b>{"value":[ "On", "Off", "Toggle"]}</b>
  & mqtt_broker -> iot_service

  iot_service -> mqtt_broker
  & mqtt_broker -> protocol_controller : <font color=#6C2A0D>ucl/by-unid/zw-1234/ep0/OnOff/Commands/On \n<font color=#6C2A0D><b>{ }</b>

  protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Desired \n<font color=#00003C><b>{ "value": true }</b>
  & mqtt_broker -> iot_service

  protocol_controller -> pan_node : PHY command
  pan_node -> protocol_controller : PHY command

  protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/OnOff/Attributes/OnOff/Reported \n<font color=#00003C><b>{ "value": true }</b>
  & mqtt_broker -> iot_service


Changing the Brightness Level of the Light Using Unify Framework
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

The following diagram shows an example of a device with both On/Off and Level
Clusters server.

.. uml::

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  ' Style for the diagram
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

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


### DoorLock Operation Using Unify Framework

The following diagram shows an example of a Door Lock controlled using the
Unify Framework.

.. uml::

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  ' Style for the diagram
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

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
  iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/DoorLock/SupportedGeneratedCommands</font>
  protocol_controller -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/DoorLock/Commands/+</font>
  iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/DoorLock/GeneratedCommands/+</font>

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
  protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/DoorLock/SupportedGeneratedCommands \n<font color=#00003C><b>{"value":[ "LockDoorResponse", "UnlockDoorResponse"]}</b>
  & mqtt_broker -> iot_service

  iot_service -> mqtt_broker
  & mqtt_broker -> protocol_controller : <font color=#6C2A0D>ucl/by-unid/zw-1234/ep0/DoorLock/Commands/LockDoor \n<font color=#6C2A0D><b>{ "PINOrRFIDCode": "1234" }</b>
  protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/DoorLock/Attributes/LockState/Desired \n<font color=#00003C><b>{ "value": "Locked" }</b>
  & mqtt_broker -> iot_service

  protocol_controller -> pan_node : PHY command
  pan_node -> protocol_controller : PHY command

  protocol_controller -> mqtt_broker : <font color=#6C2A0D>ucl/by-unid/zw-1234/ep0/DoorLock/GeneratedCommands/LockDoorResponse \n<font color=#6C2A0D><b>{ "Status": "SUCCESS" }</b>
  & mqtt_broker -> iot_service

  protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/DoorLock/Attributes/LockState/Reported \n<font color=#00003C><b>{ "value": "Locked" }</b>
  & mqtt_broker -> iot_service


DoorLockController Operation Using Unify Framework
''''''''''''''''''''''''''''''''''''''''''''''''''

The following diagram shows an example of a Door Lock Controller being used in the
Unify Framework.

.. uml::

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  ' Style for the diagram
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

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
  iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/DoorLock/GeneratedCommands</font>
  iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/DoorLock/SupportedCommands</font>
  protocol_controller -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/DoorLock/Commands/+</font>
  iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/DoorLock/GeneratedCommands/+</font>

  rnote over iot_service, pan_node: Node inclusion and capabilities discovery

  protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/DoorLock/SupportedCommands \n<font color=#00003C><b>{"value":[ "LockDoorResponse", "UnlockDoorResponse"]}</b>
  & mqtt_broker -> iot_service
  protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/DoorLock/SupportedGeneratedCommands \n<font color=#00003C><b>{"value":[ "LockDoor", "UnlockDoor"]}</b>
  & mqtt_broker -> iot_service
  ...
  rnote over iot_service, pan_node: Lock Door command initiated from a stimulus (button pressed)

  pan_node -> protocol_controller : LockDoor command sent over PHY
  protocol_controller -> mqtt_broker : <font color=#6C2A0D>ucl/by-unid/zw-1234/ep0/DoorLock/GeneratedCommands/LockDoor \n<font color=#6C2A0D><b>{ "PINOrRFIDCode": "4567" }</b>
  & mqtt_broker -> iot_service
  rnote over iot_service, pan_node: IoT Service processes LockDoor command (assuming the IoT Service evaluates the security level of the PAN node as acceptable)


PIR Sensor Notification Application Using Unify Framework
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''

The following diagram shows an example of PIR Sensor device
represented as an OccupancySensing Cluster server with the Unify framework.

.. uml::

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  ' Style for the diagram
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

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


Using the WriteAttributes Generic Command
'''''''''''''''''''''''''''''''''''''''''

Scenario: A device supports the ``ManufacturerName`` and ``LocationDescription``
attributes in the Basic Cluster. An MQTT Client wants to update the location of
the device using the *WriteAttributes* Command.

Note that the ``ManufacturerName`` is a read only attribute
and the ``LocationDescription`` is a writable attribute.

.. uml::

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  ' Style for the diagram
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

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

Manipulating PAN Node Group Membership
''''''''''''''''''''''''''''''''''''''

The following diagram shows an example of adding/removing groups for a PAN
node

.. uml::

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  ' Style for the diagram
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

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
