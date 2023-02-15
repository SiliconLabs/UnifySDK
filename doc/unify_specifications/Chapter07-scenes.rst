.. meta::
  :description: The Dotdot Scenes Cluster in the Unify Framework
  :keywords: Scenes, ScenesTable

.. |ZCL2019| replace:: `ZCL 2019 <https://zigbeealliance.org/wp-content/uploads/2019/12/07-5123-06-zigbee-cluster-library-specification.pdf>`__

.. raw:: latex

 \newpage

.. _unify_specifications_chapter_scenes:

The Dotdot Scenes Cluster (0x0005) in the Unify Framework
=========================================================

This cluster is used to define scene and configuration of each device/endpoint.
In the Unify context, it can be used both for scene controlling devices
(e.g., wall controller button) and for activating scenes for other nodes
(e.g., light dimmer).

This cluster requires additional attributes because the *Scene Table* is not
captured by its defined attributes. See |ZCL2019| for the *Scene Table* definition.

Below is an example of a PAN node Scene Cluster server:

.. code-block:: mqtt

  # Omitted the Desired values of attributes
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/ClusterRevision/Reported - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/SceneCount/Reported - {"value": 0}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/CurrentScene/Reported - {"value": 0}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/CurrentGroup/Reported - {"value": 0}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/SceneValid/Reported - {"value": false}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/NameSupport/Reported - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/SceneTable/Reported - {"value": []}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/SupportedCommands {"value" :["AddScene", "ViewScene", "RemoveScene", "RemoveAllScenes", "StoreScene", "RecallScene", "GetSceneMembership"]}

Note that in the example above, the *ViewScene* and *GetSceneMembership* commands
are available, but MAY have no effect, because their purpose is to retrieve
data. Retrieving data is sometimes done automatically by the Protocol Controllers.
In this case, a Protocol Controller MAY remove these commands from the
*SupportedCommands* list.

The Scene Table data is represented using this topic hierarchy:

**Topic:** ``ucl/by-unid/<UNID>/ep<id>/Scenes/Attributes/SceneTable/Reported``

**Topic:** ``ucl/by-unid/<UNID>/ep<id>/Scenes/Attributes/SceneTable/Desired``

The payload of the JSON data published for the Scene Table is defined
in the following JSON schema:

.. _unify_specifications_chapter_scenes_scene_table_schema:

**JSON Schema:**

.. code-block:: json

  {
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "Scene Table",
    "description": "Scene Table for the Scene Cluster",
    "definitions": {
      "SceneTableEntry": {
        "type": "object",
        "properties": {
          "GroupID": {
            "type": "number",
            "minimum": 0,
            "maximum": 65535
          },
          "SceneID": {
            "type": "number"
          },
          "SceneName": {
            "type": "string"
          },
          "TransitionTime": {
            "type": "number"
          },
          "TransitionTime100ms": {
            "type": "number"
          },
          "SceneTableExtensions": {
            "type": "object",
            "patternProperties": {
              "": {
                "type": "object"
              }
            }
          }
        },
        "required": [
          "GroupID",
          "SceneID",
          "SceneName",
          "TransitionTime",
          "TransitionTime100ms",
          "SceneTableExtensions"
        ]
      }
    },
    "type": "object",
    "properties": {
      "value": {
        "type": "array",
        "items": {
          "$ref": "#/definitions/SceneTableEntry"
        }
      }
    },
    "required": [
      "value"
    ]
  }

An example of a *SceneTableEntry* is shown below. The *SceneTableEntry* contains
a set of Clusters, which are JSON objects and defined by their Cluster Name.
Each cluster Objects contains a set of attributes, defined by their Attribute
Name, and each attribute value can be either a JSON boolean, number, or string
(in the case the value has a name).

**Payload:**

.. code-block:: json

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

For each UNID/Endpoint, each Scene is uniquely identified by the
``GroupID``/ ``SceneID`` pair, so SceneID 1 may be defined for multiple
groups, or group 1 may have multiple scenes.

Scenes Supporting Nodes
''''''''''''''''''''''''

Protocol Controller MUST advertise the Scene Table attribute for PAN nodes that
can be configured to react and change state based on scenes.

All PAN nodes/endpoints supporting attributes marked as ``sceneRequired="true"``
in the |ZCL2019| SHOULD be presented as a Scene cluster server by Protocol
Controllers.

Protocol Controllers MAY add additional attributes than the ones marked at
``sceneRequired="true"`` as part of the Scenes extension fieldsets/settings.
For example, the ZPC exposes both the ``CurrentHue`` and ``CurrentSaturation``
from the ColorControl cluster, so that all the color information is
contained in the table.

Each scene configuration can be done using the supported commands,
either ``AddScene`` or ``StoreScene``.


Example with StoreScene Command
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Let's assume the following is published for a device in the Unify Framework:

.. code-block:: mqtt

  # OnOff/Level/ColorControl: (omitting Desired values and SupportedCommands)
  ucl/by-unid/<UNID>/ep<EndpointId>/OnOff/Attributes/OnOff/Reported - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/Level/Attributes/CurrentLevel/Reported - {"value":20}
  ucl/by-unid/<UNID>/ep<EndpointId>/Level/Attributes/CurrentLevel/Reported - {"value" :20}
  ucl/by-unid/<UNID>/ep<EndpointId>/ColorControl/Attributes/CurrentHue/Reported -  {"value":52}
  ucl/by-unid/<UNID>/ep<EndpointId>/ColorControl/Attributes/CurrentSaturation - {"value":254}
  ucl/by-unid/<UNID>/ep<EndpointId>/ColorControl/Attributes/ColorMode/Reported - {"value":"CurrentHueAndCurrentSaturation"}

  # Scenes: (omitting Desired values)
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/ClusterRevision/Reported - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/SceneCount/Reported - {"value": 0}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/CurrentScene/Reported - {"value": 0}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/CurrentGroup/Reported - {"value": 0}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/SceneValid/Reported - {"value": false}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/NameSupport/Reported - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/SceneTable/Reported - {"value":[]}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/SupportedCommands - {"value": ["AddScene", "ViewScene", "RemoveScene", "RemoveAllScenes", "StoreScene", "RecallScene", "GetSceneMembership"]}


Sending a StoreScene will capture the current device settings the defined Scene:

.. code-block:: mqtt

  # Sent by the IoT Service
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Commands/StoreScene - {"GroupID":0,"SceneID":1}

  # Devices Publishes back: (Omitting Desired values)
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/SceneCount/Reported - {"value": 1}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/SceneTable/Reported -
  {
    "value": [
      {
        "GroupID": 0,
        "SceneID": 1,
        "SceneName": "",
        "SceneTableExtensions": {
          "ColorControl": {
            "ColorMode": "CurrentHueAndCurrentSaturation",
            "CurrentHue": 52,
            "CurrentSaturation": 254
          },
          "Level": {
            "CurrentLevel": 20
          },
          "OnOff": {
            "OnOff": false
          }
        },
        "TransitionTime": 0,
        "TransitionTime100ms": 0
      }
    ]
  }

Example with AddScene Command
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The AddScene Command needs to provide the contents of a SceneTableEntry in the
``ExtensionFieldSets`` field of the ``AddScene`` Command.

.. code-block:: json

  {
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "Scenes Cluster AddScene Command Properties",
    "type": "object",
    "definitions": {
      "SceneTableEntry": {
        "type": "object",
        "properties": {
          "GroupID": {
            "type": "number",
            "minimum": 0,
            "maximum": 65535
          },
          "SceneID": {
            "type": "number"
          },
          "SceneName": {
            "type": "string"
          },
          "TransitionTime": {
            "type": "number"
          },
          "TransitionTime100ms": {
            "type": "number"
          },
          "SceneTableExtensions": {
            "type": "object",
            "$ref": "#/definitions/SceneTableEntry"
          }
        },
        "required": [
          "GroupID",
          "SceneID",
          "SceneName",
          "TransitionTime",
          "TransitionTime100ms",
          "SceneTableExtensions"
        ]
      }
    },
    "properties": {
      "GroupID": {
        "type": "SGroupId"
      },
      "SceneID": {
        "type": "integer"
      },
      "TransitionTime": {
        "type": "integer"
      },
      "SceneName": {
        "type": "SSceneName"
      },
      "ExtensionFieldSets": {
        "type": "object",
        "$ref": "#/definitions/SceneTableEntry"
      }
    },
    "required": [
      "GroupID",
      "SceneID",
      "TransitionTime",
      "SceneName",
      "ExtensionFieldSets"
    ]
  }

For example, a properly formed *AddScene* command could be as follows:

.. code-block:: mqtt

  # Sent by the IoT Service
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Commands/AddScene -
    {
      "GroupID": 0,
      "SceneID": 2,
      "SceneName": "Red time!",
      "ExtensionFieldSets": {
        "ColorControl": {
          "CurrentHue": 0,
          "CurrentSaturation": 254
        },
        "Level": {
          "CurrentLevel": 99
        },
        "OnOff": {
          "OnOff": true
        }
      },
      "TransitionTime": 0,
      "TransitionTime100ms": 0
    }


The Protocol Controller MAY use a subset of the ZCL attributes defined in the
``ExtensionFieldSets`` if the end node does not serve the Clusters or
Attributes defined in this payload.

If the Scene already exists, it will be replaced with a new name,
transition times and ``ExtensionFieldSets``.
If the Scene does not already exist, it will be created.

Note that it is possible to create Scenes with empty ``ExtensionFieldSets``.

(Z-Wave) Scenes Actuators
'''''''''''''''''''''''''

Scenes actuators do not support any of the ZCL required *SupportedCommands*,
and instead use the Scene Cluster attributes to indicate which scene they
wish to activate. Such devices are both Scene Cluster servers and clients.

An MQTT client can therefore decide to activate scenes in a
number of devices/endpoints based on the spontaneous changes made by these
scenes actuator devices/endpoints.

For example, a wall switch with four buttons and three possible gestures for each button
may be presented as a scene device, supporting twelve read-only scenes that would
transition automatically (i.e., *CurrentScene* attribute will be updated in a
read-only fashion). At each activation, the wall switch will aditionally generate
a Recall Scene command that will be forwarded to the IoT Services.
The Scene table will stay empty and will not be writable by IoT Services.

In this case, the wall switch Scenes numbers will not "match" the configured
scenes in other devices. IoT Services SHOULD interpret the Scene ID as a unique
combination with the UNID/ep to decide what to do.

IoT Services SHOULD pay attention to the security level (see the "Security" in
the State topic) of a UNID before modifying the state (or sending commands) of
other PAN nodes based on Generated Commands from a PAN node.

Note that the GroupID can be set to 0 if it is not used.

.. code-block:: mqtt

  # Omitted the Desired values of attributes
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/ClusterRevision/Reported - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/SceneCount/Reported - {"value": 12}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/CurrentScene/Reported - {"value": 2}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/CurrentGroup/Reported - {"value": 0}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/SceneValid/Reported - {"value": true}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/NameSupport/Reported - {"value": false}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/SupportedCommands - {"value": [] }
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/SupportedGeneratedCommands - {"value":["RecallScene"] }


For example, when such a device tries to activate SceneID 5, it will trigger
the following publications:

.. code-block:: mqtt

  # Trying to activate Scene 5 by issuing a command, unretained
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/GeneratedCommands/RecallScene - {"GroupID":0,"SceneID":5,"TransitionTime":0}
  
  # Updates its own state, moving into Scene 5 itself.
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/CurrentScene/Reported - {"value": 5}
  ucl/by-unid/<UNID>/ep<EndpointId>/Scenes/Attributes/SceneValid/Reported - {"value": true}


Frame flow example PAN Node without Actuator Capabilities
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following diagram shows an example of a node using scenes without
supporting any of the actuator functionalities.

.. uml::

  ' Allows to do simultaneous transmissions
  !pragma teoz true

  ' Style for the diagram
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

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
  iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Scenes/SupportedGeneratedCommands</font>
  iot_service -> mqtt_broker: <font color=#0039FB>ucl/by-unid/+/+/Scenes/GeneratedCommands/+</font>
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

  rnote over iot_service, protocol_controller: Skipping additional Attribute Publications

  protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/SupportedCommands \n<font color=#00003C><b>{"value":[ ]}
  & mqtt_broker -> iot_service

  protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/SupportedGeneratedCommands \n<font color=#00003C><b>{"value":[ "RecallScene"]}
  & mqtt_broker -> iot_service

  == ==

  end_user -> pan_node : Activate scene 5 button
  pan_node -> protocol_controller : PHY command

  protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentScene/Desired \n<font color=#00003C><b>{ "value": 5 }</b>
  & mqtt_broker -> iot_service

  protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentScene/Reported \n<font color=#00003C><b>{ "value": 5 }</b>
  & mqtt_broker -> iot_service

  protocol_controller -> mqtt_broker : <font color=#6C2A0D>ucl/by-unid/zw-1234/ep0/Scenes/GeneratedCommands/RecallScene \n<font color=#6C2A0D><b>{ "GroupID": 0, "SceneID": 5, "TransitionTime":0 }</b>
  & mqtt_broker -> iot_service

  == ==

  end_user -> pan_node : Activate scene 5 button again
  pan_node -> protocol_controller : PHY command

  protocol_controller -> mqtt_broker : <font color=#6C2A0D>ucl/by-unid/zw-1234/ep0/Scenes/GeneratedCommands/RecallScene \n<font color=#6C2A0D><b>{ "GroupID": 0, "SceneID": 5, "TransitionTime":0 }</b>
  & mqtt_broker -> iot_service

  == ==

  end_user -> pan_node : Activate scene 2 button
  pan_node -> protocol_controller : PHY command

  protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentScene/Desired \n<font color=#00003C><b>{ "value": 2 }</b>
  & mqtt_broker -> iot_service

  protocol_controller -> mqtt_broker : <font color=#00003C>ucl/by-unid/zw-1234/ep0/Scenes/Attributes/CurrentScene/Reported \n<font color=#00003C><b>{ "value": 2 }</b>
  & mqtt_broker -> iot_service

  protocol_controller -> mqtt_broker : <font color=#6C2A0D>ucl/by-unid/zw-1234/ep0/Scenes/GeneratedCommands/RecallScene \n<font color=#6C2A0D><b>{ "GroupID": 0, "SceneID": 2, "TransitionTime":0 }</b>
  & mqtt_broker -> iot_service
