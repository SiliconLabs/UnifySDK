.. meta::
  :description: The Unify Descriptor Cluster 
  :keywords: Descriptor

.. raw:: latex

 \newpage

.. _unify_specifications_chapter_descriptor_cluster:

The Unify Descriptor Cluster (0xFD13) (Experimental) 
====================================================

This cluster describes an endpoint instance on the node to conform to complex device type patterns. This cluster 
supports a list of one or more device type identifiers that represent conformance to device type specifications.

For Example: An Extended Color Light device type may support device type IDs for both a
Dimmable Light and On/Off Light, because those are subsets of an Extended Color Light (the
superset).

Attributes
-----------
.. list-table::
  :name: table_unify_specifications_attributes
  :header-rows: 1

  * - Name  
    - Type 
  * - DeviceTypeList
    - list  [DeviceTypeStruct]

DeviceTypeList Attribute
'''''''''''''''''''''''''

This Attribute contains the list of device types and corresponding revisions declaring endpoint conformance. An endpoint 
SHALL conform to all device types listed in the DeviceTypeList. Every list entry must contain both "DeviceType" and 
"Revision" keys. There should be atleast one entry in list. The Protocol Controller must set this attribute in the
attribute store when it is supported. 

The Device Type List is represented using the below topics:

**Topic:** ``ucl/by-unid/<UNID>/ep<Endpoint-ID>/Descriptor/Attributes/DeviceTypeList/Reported``

**Topic:** ``ucl/by-unid/<UNID>/ep<Endpoint-ID>/Descriptor/Attributes/DeviceTypeList/Desired``

**Payload Example:**

.. code-block:: json
    
    {
        "value": [
        {
            "DeviceType": "DimmableLight",
            "Revision": 1
        },
        {
            "DeviceType": "ColorTemperatureLight",
            "Revision": 2
        },
        {
            "DeviceType": "ExtendedColorLight",
            "Revision": 1
        }
        ]
    }

Data Types
-----------

DeviceTypeStruct Type
''''''''''''''''''''''
.. list-table::
  :name: table_unify_specifications_device_type_struct
  :header-rows: 1

  * - Name  
    - Type 
  * - DeviceType
    - DevTypeId 
  * - Revision 
    - uint16 

DeviceType Field
~~~~~~~~~~~~~~~~
This SHALL indicate the device type definition. The endpoint SHALL conform to the device type
definition and cluster specifications required by the device type. The DeviceType string value
is derived from the name of the Device Type in the Matter Specification, by removing all spaces
and special characters in the name. 

E.g. Device Type "On/Off Light" in the Matter spec becomes a DeviceType value of "OnOffLight" in UCL.

Revision Field
~~~~~~~~~~~~~~~
This is the implemented revision of the device type definition. The endpoint SHALL conform to this
revision of the device type.


Supported UCL Device Types
--------------------------
Below UCL Device types are taken from Device Types in the Matter Specification. Definition, cluster 
specifications, elements and cluster conformance required by the device type will be same as in 
the Matter Specification. Matter specifications can be obtained from CSA Alliance on request.

.. list-table:: UCL Device Type to ZCL Device type Mapping and differences
  :name: table_unify_specifications_ucl_to_zcl_device_type_map
  :widths: 10 10 50
  :header-rows: 1

  * - UCLDeviceType
    - ZCLDeviceType
    - Mandatory Cluster Differences
  * - DoorLock
    - N/A
    - N/A
  * - DoorLockController
    - N/A
    - N/A
  * - Fan
    - N/A
    - N/A
  * - AirPurifier
    - N/A
    - N/A
  * - RoomAirConditioner
    - N/A
    - N/A
  * - Cooktop
    - N/A
    - N/A
  * - ExtractorHood
    - N/A
    - N/A
  * - OnOffLight
    - On/OffLight
    - No Differences
  * - DimmableLight
    - DimmableLight
    - No Differences
  * - OnOffLightSwitch
    - On/OffLightSwitch
    - No Differences
  * - DimmerSwitch
    - DimmerSwitch
    - No Differences
  * - ColorDimmerSwitch
    - ColorDimmerSwitch
    - No Differences
  * - LightSensor
    - LightSensor
    - ZCL has Mandatory Identify CLuster while UCL doesn't.
  * - OccupancySensor
    - OccupancySensor
    - ZCL has Mandatory Identify CLuster while UCL doesn't.
  * - OnOffPluginUnit
    - On/OffPluginUnit
    - No Differences
  * - DimmablePlugInUnit
    - DimmablePlugInUnit
    - No Differences
  * - ColorTemperatureLight
    - ColorTemperatureLight
    - No Differences
  * - ExtendedColorLight
    - ExtendedColorLight
    - HS,CL,EHUE feature are optional in UCL while madatory in ZCL.
  * - WindowCovering
    - N/A
    - N/A
  * - WindowCoveringController
    - N/A
    - N/A
  * - HeatingCoolingUnit
    - N/A
    - N/A
  * - Thermostat
    - N/A
    - N/A
  * - TemperatureSensor
    - N/A
    - N/A
  * - Pump
    - N/A
    - N/A
  * - PressureSensor
    - N/A
    - N/A
  * - FlowSensor
    - N/A
    - N/A
  * - HumiditySensor
    - N/A
    - N/A
  * - ControlBridge
    - N/A
    - N/A
  * - OnOffSensor
    - On/OffSensor
    - No Differences
  
  
.. list-table:: Elemental Differences between UCL and ZCL Device Type CLuster specifications
  :name: table_unify_specifications_device_type_elemental_differences
  :widths: 10 30 30
  :header-rows: 1
  
  * - ClusterName
    - Attributes
    - Commands
  * - Identify
    - IdentifyType missing in ZCL
    - IdentifyQuery missing in UCL
  * - Level
    - OnLevel missing in ZCL
    - N/A
  * - Occupancy
    - OccupancySensorTypeBitmap missing in ZCL
    - N/A 
  * - Color
    - Attribute names have "Mireds" appended in UCL
      EnhancedColorMode missing in ZCL
      EnhancedCurrentHue optional in UCL while Mandatory in UCL
    - N/A

  
    
.. note::
  Any other protocol device types should be mapped as closely as possible to UCL device types, 
  and if no close match is found, new device types may be proposed for UCL. Also minor cluster 
  differences should be disregarded and the closest matching UCL device type should be used. 
  Service providers MUST consider these potential minor cluster differences while rendering their UI's.

  