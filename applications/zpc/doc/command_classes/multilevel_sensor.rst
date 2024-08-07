Multilevel Sensor
=================

Version supported : 11

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Multilevel Sensor Get Supported Sensor <multilevel-sensor-get-supported-sensor>`
#. For each supported Sensor Type: :ref:`Multilevel Sensor Get Supported Scale <multilevel-sensor-get-supported-scale>`
#. For first Scale Supported: :ref:`Multilevel Sensor Get Command <multilevel-sensor-get>`


Command Class Commands
**********************

.. _multilevel-sensor-get:

Multilevel Sensor Get
---------------------

Trigger on undefined **reported** value of ``SENSOR_VALUE``

Mapping between command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Sensor Type``
    - ``SENSOR_TYPE``
  * - ``Scale``
    - ``SCALE``

Multilevel Sensor Report
------------------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Sensor Type``
    - ``SENSOR_TYPE``
  * - ``Scale``
    - ``SCALE``
  * - ``Sensor Value``
    - ``SENSOR_VALUE``

.. _multilevel-sensor-get-supported-sensor:

Multilevel Sensor Get Supported Sensor
--------------------------------------

Trigger on undefined **reported** value of ``SUPPORTED_SENSOR_TYPE``

Multilevel Sensor Supported Sensor Report
-----------------------------------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Bit Mask 1-N``
    - ``SUPPORTED_SENSOR_TYPES``

.. _multilevel-sensor-get-supported-scale:

Multilevel Sensor Get Supported Scale
-------------------------------------

Trigger on undefined **reported** value of ``SUPPORTED_SCALE``

Mapping between command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Sensor Type``
    - ``SENSOR_TYPES``

Multilevel Sensor Supported Scale Report
----------------------------------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Sensor Type``
    - ``SENSOR_TYPES``
  * - ``Scale Bit Mask``
    - ``SUPPORTED_SCALE``

.. note:: 
    The structure of the attribute store is : 

    .. code:: text
        
        |__ SENSOR_TYPE
        |   |__ SUPPORTED_SCALES
        |   |__ SCALE
        |   |__ SENSOR_VALUE
        |__ SUPPORTED_SENSOR_TYPES



Unify Clusters
**************

UAM files
---------

.. list-table:: 
  :header-rows: 1

  * - UAM File
    - Cluster
    - Comments
  * - ``CarbonDioxideMeasurement_to_MultilevelSensorCC.uam``
    - ``ConcentrationMeasurement_CarbonDioxide.xml``
    - Use to operate CarbonDioxide Sensor
  * - ``CarbonMonoxideMeasurement_to_MultilevelSensorCC.uam``
    - ``ConcentrationMeasurement.xml``
    - Use to operate CarbonMonoxide Sensor
  * - ``IlluminanceMeasurement_to_MultilevelSensorCC.uam``
    - ``IlluminanceMeasurement.xml``
    - Use to operate Illuminance Sensor
  * - ``pHMeasurement_to_MultilevelSensorCC.uam``
    - ``pHMeasurement.xml``
    - Use to operate pH Sensor
  * - ``PressureMeasurement_to_MultilevelSensorCC.uam``
    - ``PressureMeasurement.xml``
    - Use to operate Pressure Sensor
  * - ``RelativeHumidity_to_MultilevelSensorCC.uam``
    - ``WaterContentMeasurement.xml``
    - Use to operate Humidity Sensor
  * - ``SoilMoisture_to_MultilevelSensorCC.uam``
    - ``WaterContentMeasurement_SoilMoisture.xml``
    - Use to operate Soil Moisture Sensor
  * - ``TemperatureMeasurement_to_MultilevelSensorCC.uam``
    - ``TemperatureMeasurement.xml``
    - Use to operate Temperature Sensor
  * - ``Time_to_MultilevelSensorCC.uam``
    - ``Time.xml``
    - Use to operate Local Time

Bindings
--------

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Cluster attribute
    - Comments
  * - ``SENSOR_VALUE``
    - CarbonDioxide MeasuredValue
    - If ``SENSOR_TYPE`` = 0x11. Z-Wave -> Cluster (ReadOnly)
  * - ``SENSOR_VALUE``
    - CarbonMonoxide MeasuredValue
    - If ``SENSOR_TYPE`` = 0x28. Z-Wave -> Cluster (ReadOnly)
  * - ``SENSOR_VALUE``
    - IlluminanceMeasurement MeasuredValue
    - If ``SENSOR_TYPE`` = 0x03. Z-Wave -> Cluster (ReadOnly)
  * - ``SENSOR_VALUE``
    - PhMeasurement MeasuredValue
    - If ``SENSOR_TYPE`` = 0x43. Z-Wave -> Cluster (ReadOnly)
  * - ``SENSOR_VALUE``
    - PressureMeasurement MeasuredValue
    - If ``SENSOR_TYPE`` is one of the following :  ``0x08``, ``0x09``, ``0x38``, ``0x4E``, ``0x4F``. Z-Wave -> Cluster (ReadOnly)
  * - ``SENSOR_VALUE``
    - RelativityHumidity MeasuredValue
    - If ``SENSOR_TYPE`` = 0x05. Z-Wave -> Cluster (ReadOnly)
  * - ``SENSOR_VALUE``
    - SoilMoisture MeasuredValue
    - If ``SENSOR_TYPE`` = 0x2A. Z-Wave -> Cluster (ReadOnly)
  * - ``SENSOR_VALUE``
    - TemperatureMeasurement MeasuredValue
    - If ``SENSOR_TYPE`` = 0x01. Z-Wave -> Cluster (ReadOnly)
  * - ``SENSOR_VALUE``
    - Time MeasuredValue
    - If ``SENSOR_TYPE`` = 0x21. Z-Wave -> Cluster (ReadOnly)

Command actions
---------------

.. list-table:: 
  :widths: 20 50 30
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Get Measured Value of CarbonDioxide Sensor
    - ``ucl/by-unid/+/+/CarbonDioxide/Attributes/MeasuredValue/Reported {"value": <REPORTED_MEASURED_VALUE>}``
    - Only reported values
  * - Get Min Measured Value of CarbonDioxide Sensor
    - ``ucl/by-unid/+/+/CarbonDioxide/Attributes/MinMeasuredValue/Reported {"value": <REPORTED_MIN_MEASURED_VALUE>}``
    - Only reported values
  * - Get Max Measured Value of CarbonDioxide Sensor
    - ``ucl/by-unid/+/+/CarbonDioxide/Attributes/MaxMeasuredValue/Reported {"value": <REPORTED_MAX_MEASURED_VALUE>}``
    - Only reported values
  * - Get Tolerance Value of CarbonDioxide Sensor
    - ``ucl/by-unid/+/+/CarbonDioxide/Attributes/Tolerance/Reported {"value": <REPORTED_TOLERANCE>}``
    - Only reported values
  * - Get Measured Value of CarbonMonoxide Sensor
    - ``ucl/by-unid/+/+/CarbonMonoxide/Attributes/MeasuredValue/Reported {"value": <REPORTED_MEASURED_VALUE>}``
    - Only reported values
  * - Get Min Measured Value of CarbonMonoxide Sensor
    - ``ucl/by-unid/+/+/CarbonMonoxide/Attributes/MinMeasuredValue/Reported {"value": <REPORTED_MIN_MEASURED_VALUE>}``
    - Only reported values
  * - Get Max Measured Value of CarbonMonoxide Sensor
    - ``ucl/by-unid/+/+/CarbonMonoxide/Attributes/MaxMeasuredValue/Reported {"value": <REPORTED_MAX_MEASURED_VALUE>}``
    - Only reported values
  * - Get Tolerance Value of CarbonMonoxide Sensor
    - ``ucl/by-unid/+/+/CarbonMonoxide/Attributes/Tolerance/Reported {"value": <REPORTED_TOLERANCE>}``
    - Only reported values
  * - Get Measured Value of PhMeasurement Sensor
    - ``ucl/by-unid/+/+/PhMeasurement/Attributes/MeasuredValue/Reported {"value": <REPORTED_MEASURED_VALUE>}``
    - Only reported values
  * - Get Min Measured Value of PhMeasurement Sensor
    - ``ucl/by-unid/+/+/PhMeasurement/Attributes/MinMeasuredValue/Reported {"value": <REPORTED_MIN_MEASURED_VALUE>}``
    - Only reported values
  * - Get Max Measured Value of PhMeasurement Sensor
    - ``ucl/by-unid/+/+/PhMeasurement/Attributes/MaxMeasuredValue/Reported {"value": <REPORTED_MAX_MEASURED_VALUE>}``
    - Only reported values
  * - Get Tolerance Value of PhMeasurement Sensor
    - ``ucl/by-unid/+/+/PhMeasurement/Attributes/Tolerance/Reported {"value": <REPORTED_TOLERANCE>}``
    - Only reported values
  * - Get Measured Value of PressureMeasurement Sensor
    - ``ucl/by-unid/+/+/PressureMeasurement/Attributes/MeasuredValue/Reported {"value": <REPORTED_MEASURED_VALUE>}``
    - Only reported values
  * - Get Min Measured Value of PressureMeasurement Sensor
    - ``ucl/by-unid/+/+/PressureMeasurement/Attributes/MinMeasuredValue/Reported {"value": <REPORTED_MIN_MEASURED_VALUE>}``
    - Only reported values
  * - Get Max Measured Value of PressureMeasurement Sensor
    - ``ucl/by-unid/+/+/PressureMeasurement/Attributes/MaxMeasuredValue/Reported {"value": <REPORTED_MAX_MEASURED_VALUE>}``
    - Only reported values
  * - Get Tolerance Value of PressureMeasurement Sensor
    - ``ucl/by-unid/+/+/PressureMeasurement/Attributes/Tolerance/Reported {"value": <REPORTED_TOLERANCE>}``
    - Only reported values
  * - Get Measured Value of RelativityHumidity Sensor
    - ``ucl/by-unid/+/+/RelativityHumidity/Attributes/MeasuredValue/Reported {"value": <REPORTED_MEASURED_VALUE>}``
    - Only reported values
  * - Get Min Measured Value of RelativityHumidity Sensor
    - ``ucl/by-unid/+/+/RelativityHumidity/Attributes/MinMeasuredValue/Reported {"value": <REPORTED_MIN_MEASURED_VALUE>}``
    - Only reported values
  * - Get Max Measured Value of RelativityHumidity Sensor
    - ``ucl/by-unid/+/+/RelativityHumidity/Attributes/MaxMeasuredValue/Reported {"value": <REPORTED_MAX_MEASURED_VALUE>}``
    - Only reported values
  * - Get Tolerance Value of RelativityHumidity Sensor
    - ``ucl/by-unid/+/+/RelativityHumidity/Attributes/Tolerance/Reported {"value": <REPORTED_TOLERANCE>}``
    - Only reported values
  * - Get Measured Value of SoilMoisture Sensor
    - ``ucl/by-unid/+/+/SoilMoisture/Attributes/MeasuredValue/Reported {"value": <REPORTED_MEASURED_VALUE>}``
    - Only reported values
  * - Get Min Measured Value of SoilMoisture Sensor
    - ``ucl/by-unid/+/+/SoilMoisture/Attributes/MinMeasuredValue/Reported {"value": <REPORTED_MIN_MEASURED_VALUE>}``
    - Only reported values
  * - Get Max Measured Value of SoilMoisture Sensor
    - ``ucl/by-unid/+/+/SoilMoisture/Attributes/MaxMeasuredValue/Reported {"value": <REPORTED_MAX_MEASURED_VALUE>}``
    - Only reported values
  * - Get Tolerance Value of SoilMoisture Sensor
    - ``ucl/by-unid/+/+/SoilMoisture/Attributes/Tolerance/Reported {"value": <REPORTED_TOLERANCE>}``
    - Only reported values
  * - Get Measured Value of TemperatureMeasurement Sensor
    - ``ucl/by-unid/+/+/TemperatureMeasurement/Attributes/MeasuredValue/Reported {"value": <REPORTED_MEASURED_VALUE>}``
    - Only reported values
  * - Get Min Measured Value of TemperatureMeasurement Sensor
    - ``ucl/by-unid/+/+/TemperatureMeasurement/Attributes/MinMeasuredValue/Reported {"value": <REPORTED_MIN_MEASURED_VALUE>}``
    - Only reported values
  * - Get Max Measured Value of TemperatureMeasurement Sensor
    - ``ucl/by-unid/+/+/TemperatureMeasurement/Attributes/MaxMeasuredValue/Reported {"value": <REPORTED_MAX_MEASURED_VALUE>}``
    - Only reported values
  * - Get Tolerance Value of TemperatureMeasurement Sensor
    - ``ucl/by-unid/+/+/TemperatureMeasurement/Attributes/Tolerance/Reported {"value": <REPORTED_TOLERANCE>}``
    - Only reported values
  * - Get Local Time
    - ``ucl/by-unid/+/+/Time/Attributes/LocalTime/Reported { "value": <REPORTED_LOCAL_TIME>}``
    - Only reported values
