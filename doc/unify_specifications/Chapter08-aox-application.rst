.. meta::
  :description: Unify AoX Application
  :keywords: AoX, Locator, Asset, Tag

.. raw:: latex

 \newpage

.. _unify_specifications_chapter_aox_application:

AoX Application
===============

This chapter contains cluster definitions used to support the AoX application.
These cluster definitions are unique to the Unify Controller Language.
This is an addition to the clusters already defined in the ZCL.

Terminology
-----------

We define here several entities as part of an AoX system:

* **Assets Tags**

  Assets tags are radio devices sending unencrypted radio beacons
  that allow to triangulate assets.

* **AoX Protocol Controller / Locator**

  The AoXPC is a component that processes incoming radio beacons, converts
  them into IQ Reports or Angle reports, which will allow another component
  to compute the position of the device.
  A locator is a part of the AoX Protocol Controller, it is basically an NCP
  with a configuration that will help to determine the relative position of
  an asset tag.

* **AoX Positioning Service**

  This is essentially a helper component (like the SmartStart list manager)
  that reads the data reported from the locators and will compute and publish
  the absolute position of each tag.

The system can be seen as follows:

.. uml::

  ' Style for the diagram
  allow_mixing
  skinparam objectBorderColor black

  title AoX components overview

  package "RF Area" <<Cloud>> {

      object "AoXPC" as aox_pc #FFFFFF
      aox_pc : ble-locator-1111

      object "AoXPC" as aox_pc_2 #FFFFFF
      aox_pc_2 : ble-locator-2222

      package "Assets" <<Rectangle>> {
          object "Asset Tag 1" as tag_1 #FFFFFF
          object "Asset Tag 2" as tag_2 #FFFFFF
          object "Asset Tag 3" as tag_3 #FFFFFF
          object "Asset Tag 4" as tag_4 #FFFFFF
      }
  }

  object "MQTT Broker" as mqtt_broker #FFFFFF
  object "AoX Positioning Service" as positioning_service #FFFFFF

  aox_pc -[hidden]right- aox_pc_2

  tag_1 -up-> aox_pc: beacons
  tag_2 -up-> aox_pc
  tag_3 -up-> aox_pc
  tag_4 -up-> aox_pc

  tag_1 -up-> aox_pc_2
  tag_2 -up-> aox_pc_2
  tag_3 -up-> aox_pc_2
  tag_4 -up-> aox_pc_2

  aox_pc -> mqtt_broker : tag_1 data (ble-locator-1111)
  aox_pc_2 -> mqtt_broker : tag_1 data (ble-locator-2222)

  mqtt_broker -down-> positioning_service: tags raw data\nlocator configuration
  positioning_service -up-> mqtt_broker : tags estimated position


AoX Clusters
------------

This section defines how to represent Angle-of-X (e.g, Angle of Arrival) IQ
report, Angle, and Position data in the Unify Framework.

The AoX system relies on several new clusters:

* AoXLocator, used to manage locator devices within the AoXPC.
* AoXPositionEstimation, used to report Assets estimated position.

Locators and the AoXLocator cluster
'''''''''''''''''''''''''''''''''''

The AoX Protocol Controller allows to read the configuration of its
Locator using the AoXLocator Cluster. For example, an AoX Protocol Controller
could advertise it like this:

.. code-block:: mqtt

  ucl/by-unid/<aoxpc_unid>/ep0/AoXLocator/Attributes/PositionAndOrientationValid/Desired - {"value": true}
  ucl/by-unid/<aoxpc_unid>/ep0/AoXLocator/Attributes/PositionAndOrientationValid/Reported - {"value": true}
  ucl/by-unid/<aoxpc_unid>/ep0/AoXLocator/Attributes/PositionAndOrientation/Desired - {
    "value": {
      "CoordinateX": 3.75,
      "CoordinateY": 0,
      "CoordinateZ": 1.25,
      "OrientationX": 50,
      "OrientationY": 0,
      "OrientationZ": 0
    }
  }
  ucl/by-unid/<aoxpc_unid>/ep0/AoXLocator/Attributes/PositionAndOrientation/Reported - {
    "value": {
      "CoordinateX": 3.75,
      "CoordinateY": 0,
      "CoordinateZ": 1.25,
      "OrientationX": 50,
      "OrientationY": 0,
      "OrientationZ": 0
    }
  }

The ``PositionAndOrientation`` represents the the location of the NCP radio
for the AoX locator. CoordinateX, CoordinateY, CoordinateZ are cartesian
coordinates, in meters. OrientationX, OrientationY and OrientationZ are the
axis rotation (Euler angles) in degrees.
More details are provided in
`Silicon Labs RTL Locator item <https://docs.silabs.com/rtl-lib/latest/group-sl-rtl-loc#structsl-rtl-loc-locator-item>`__

The additional ``PositionAndOrientationValid`` boolean attribute indicates if
the locator ``PositionAndOrientation`` attributes has been set
and the reported values reported are correct.

A subscription to `ucl/by-unid/<aoxpc_unid>/ep0/AoXLocator/Attributes/#` will allow
to read the entire configuration and possibly write back the configuration
that is writable.

The AoXLocator Cluster can be configured using the AoXLocator cluster
**WriteAttributes** command.

.. code-block:: mqtt

  ucl/by-unid/<aoxpc_unid>/ep0/AoXLocator/SupportedCommands - {"value": ["AngleCorrection", "WriteAttributes"]}
  ucl/by-unid/<aoxpc_unid>/ep0/AoXLocator/SupportedGeneratedCommands - {"value": ["IQReport"]}

An AoX Protocol Controller MUST generate an `IQReport` or an `AngleReport` Command
for every radio beacon that has been received by a locator.

.. code-block:: mqtt

  ucl/by-unid/<aoxpc_unid>/ep0/AoXLocator/GeneratedCommands/IQReport - {
    "TagUnid" : "ble-pd-842E1431C9F0",
    "Channel": 13,
    "Rssi": -50,
    "Sequence": 42767,
    "Samples": [23, 105, 106, -10, 2, -108]
  }

Or

.. code-block:: mqtt

  ucl/by-unid/<aoxpc_unid>/ep0/AoXLocator/GeneratedCommands/AngleReport -
  {
    "Deviation": {
      "Azimuth": 0,
      "Distance": 0,
      "Elevation": 0
    },
    "Direction": {
      "Azimuth": -6.4180755615234375,
      "Distance": 1.5519763231277466,
      "Elevation": 51.71907043457031
    },
    "Sequence": 42767,
    "TagUnid": "ble-pd-842E1431C9F0"
  }

In the ``AngleReport`` commands, ``Direction`` represents the measured angle data
while ``Deviation`` indicates the standard deviation of the that measurement.
Both ``Direction`` and ``Deviation`` are spherical coordinates.
In sphrical coordinates objects, the ``Azimuth`` and ``Elevation`` are in degrees and
the ``Distance`` is in meter.

More details about how to interpret the values in the IQReports or AngleReports
are given in
`Silicon Labs RTL Library <https://docs.silabs.com/rtl-lib/latest/>`__
and
`Silicon Labs User Guide 103.18 <https://www.silabs.com/documents/public/user-guides/ug103-18-bluetooth-direction-finding-fundamentals.pdf>`__

Each locator will generate 'IQReport' or 'AngleReport'.
Locators supporting both modes will advertise which mode is configured
using the ``ReportingMode`` attribute configuration.
If the ``ReportingMode`` attribute is not writable, it will not be advertised
and instead IoT Services will be able to know what commands are generated reading
the value published at the SupportedGeneratedCommands topic:

.. code-block:: mqtt

  ucl/by-unid/<aoxpc_unid>/ep0/AoXLocator/SupportedGeneratedCommands - {"value": ["AngleReport"]}


For example, the following diagram shows an example of configuring which
commands are generated by a locator.

.. uml::

  ' Allows simultaneous transmissions
  !pragma teoz true

  ' Style for the diagram
  !theme plain
  skinparam LegendBackgroundColor #F0F0F0

  title Configuring IQReport or AngleReports for an AoX Locator

  legend top
  <font color=#0039FB>MQTT Subscription</font>
  <font color=#00003C>Retained MQTT Publication</font>
  <font color=#6C2A0D>Unretained MQTT Publication</font>
  <font color=#000000>PHY messages</font>
  endlegend

  ' List of participants
  participant "IoT Service" as iot_service

  participant "AoX Protocol Controller\nUNID: pc" as protocol_controller
  participant "Asset Tag 1\nUNID: tag_1" as tag_1

  protocol_controller -> iot_service: <font color=#00003C>ucl/by-unid/pc/ep0/AoXLocator/Attributes/ReportingMode/Reported \n<font color=#00003C><b>{"value": "IQReport"}

  protocol_controller -> iot_service: <font color=#00003C>ucl/by-unid/pc/ep0/AoXLocator/SupportedGeneratedCommands \n<font color=#00003C><b>{"value": ["IQReport"]}

  rnote over protocol_controller: AoX Locator\nis configured to\nsend IQReport

  tag_1 -> protocol_controller: Beacon

  protocol_controller -> iot_service: <font color=#6C2A0D>ucl/by-unid/pc/ep0/AoXLocator/GeneratedCommands/IQReport \n<font color=#6C2A0D><b>{IQREport data}

  rnote over iot_service: I want AngleReports

  iot_service -> protocol_controller: <font color=#6C2A0D>ucl/by-unid/pc/ep0/AoXLocator/Commands/WriteAttributes \n<font color=#6C2A0D><b>{"ReportingMode" =  "AngleReport"}

  protocol_controller -> iot_service: <font color=#00003C>ucl/by-unid/pc/ep0/AoXLocator/Attributes/ReportingMode/Desired \n<font color=#00003C><b>{"value": "AngleReport"}

  protocol_controller -> iot_service: <font color=#00003C>ucl/by-unid/pc/ep0/AoXLocator/Attributes/ReportingMode/Reported \n<font color=#00003C><b>{"value": "AngleReport"}

  protocol_controller -> iot_service: <font color=#00003C>ucl/by-unid/pc/ep0/AoXLocator/SupportedGeneratedCommands \n<font color=#00003C><b>{"value": ["AngleReport"]}

  rnote over protocol_controller: AoX Locator\nis configured to\nsend AngleReport

  tag_1 -> protocol_controller: Beacon

  protocol_controller -> iot_service: <font color=#6C2A0D>ucl/by-unid/pc/ep0/AoXLocator/GeneratedCommands/AngleReport \n<font color=#6C2A0D><b>{AngleReport data}

AngleCorrection Command
~~~~~~~~~~~~~~~~~~~~~~~

For calibration and accuracy purposes, AoX locators
generating ``AngleReport`` commands SHOULD support receiving the
``AngleCorrection`` command. The ``AngleCorrection`` command
will indicate what values are expected for a given asset tag to an AoX locator.

Details on applying angle correction are given in
`Silicon Labs RTL Library: Get expected direction <https://docs.silabs.com/rtl-lib/latest/group-sl-rtl-loc#ga788dcffb47a4bbb26b110bb9e817b0e5>`__

AoXPositionEstimation cluster
'''''''''''''''''''''''''''''

A Positioning Service helper MQTT client can be used to listen to MQTT
publications coming from the AoXLocator cluster to determine the
position of each tag in the system.

The Positioning Service does not have a UNID itself. It connects to the
MQTT broker, and performs the following actions:

1. Read/Write the configuration:

  a. Reads the configuration for each AoXPC / Locator, using a subscription to
     ``ucl/by-unid/+/+/AoXLocator/Attributes/+/Reported``
  b. Optionally adjusts the configuration issuing WriteAttributes Command
     ``ucl/by-unid/+/+/AoXLocator/Commands/WriteAttributes``
    
2. Listen to incoming IQReport or AngleReports by subscribing to:
   ``ucl/by-unid/+/+/AoXLocator/GeneratedCommands/+``

Whenever position data for a tag is available, the Positioning Service MUST
publish the calculated Position attribute in the AoXPositionEstimation Cluster.


.. code-block:: mqtt

  ucl/by-unid/<TAG_UNID>/ep0/AoXPositionEstimation/Attributes/Position/Reported -
    {
    "value": {
      "ApplicationId": "positioning_4154",
      "DeviationX": 3.1297969818115234,
      "DeviationY": 0.6999226212501526,
      "DeviationZ": 3.8446271419525146,
      "Sequence": 47021,
      "X": 0.8519021272659302,
      "Y": 0.000349800189724192,
      "Z": 0.8335742950439453
    }
  }

In the ``Position`` attribute, Coordinate components (``X``, ``Y``, etc.) unit is
Meter.

The first time that a tag's position is determined, or a new tag is detected,
the Positioning Service application MUST publish a node state topic for that tag.

.. code-block:: mqtt

  ucl/by-unid/<TAG_UNID>/State - {
    "NetworkStatus": "Online functional",
    "Security": "None",
    "MaximumCommandDelay": "unknown"
  }

When a Positioning Service application shuts down, it MUST unretain
all the AoXPositionEstimation and State data that it published for tags.

Positioning Service
'''''''''''''''''''

The Positioning Service is a helper component that can determine
spatial coordinates, or position, for each tag UNID and publises the last
reported position.

Asset Tag UNIDs
'''''''''''''''

Asset Tag UNIDs MUST indicate the BLE address type in addition to the BLE address.
The generic format is `ble-<ADDRESS_TYPE>-<BLE_ADDRESS>`.
Refer to :ref:`unify_specifications_chapter_architecture_overview`
for details about UNID composition.
Static random addresses can be abbreviated **sr** and public device addresses **pd**.

An example of a public device address UNID is ``ble-pd-842E1431C9F0``.
