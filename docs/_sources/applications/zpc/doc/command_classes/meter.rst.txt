Meter
============

Version supported : 6

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Meter Supported Get <meter-supported-get>`
#. For each supported Rate Type: :ref:`Meter Get <meter-get>`

Command Class Commands
**********************

.. _meter-get:

Meter Get
-------------------------

Trigger on undefined **reported** value of ``VALUE``


Meter Report
-----------------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Scale``
    - ``SCALE``
  * - ``Rate Type``
    - ``RATE_TYPE``
  * - ``Metter Type``
    - ``TYPE``
  * - ``Precision``
    - ``PRECISION``
  * - ``Metter Value``
    - ``VALUE``

.. _meter-supported-get:

Meter Supported Get
-----------------------------------------------------

Trigger on undefined **reported** value of ``SUPPORTED_SCALE``


Meter Supported Report
-----------------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Meter Reset``
    - ``RESET_SUPPORTED``
  * - ``Rate Type``
    - ``SUPPORTED_RATE_TYPES``
  * - ``Metter Type``
    - ``TYPE``
  * - ``Scale Supported``
    - ``SUPPORTED_SCALES``

.. note:: 
    The structure of the attribute store is : 

    .. code:: text
        
        |__ RESET_SUPPORTED
        |__ SUPPORTED_SCALES
        |__ SUPPORTED_RATE_TYPES
        |__ TYPES
            |__ SCALE
                |__ RATE_TYPE
                    |__ VALUE
                    |__ PRECISION



Unify Clusters
**************

UAM files
---------

.. list-table:: 
  :header-rows: 1

  * - UAM File
    - Cluster
    - Comments
  * - ``Metering_to_Meter.uam``
    - ``Metering.xml``
    - Use to Operate meter type, Unit of Measure and Current
  * - ``ElectricalMeasurement_to_Meter.uam``
    - ``ElectricalMeasurement.xml``
    - Use to Operate Electrical Measurement
 
Bindings
--------

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Cluster attribute
    - Comments
  * - ``TYPE``
    - Metering MeteringDeviceType
    - Z-Wave -> Cluster (ReadOnly)
  * - ``SCALE``
    - Metering UnitofMeasure
    - Z-Wave -> Cluster (ReadOnly)
  * - ``VALUE``
    - Metering CurrentSummationDelivered
    - Z-Wave -> Cluster (ReadOnly)
  * - ``VALUE``
    - Metering CurrentSummationReceived
    - Z-Wave -> Cluster (ReadOnly)
  * - ``VALUE``
    - ElectricalMeasurement ActivePower
    - Z-Wave -> Cluster (ReadOnly)
  * - ``VALUE``
    - ElectricalMeasurement RMSCurrent
    - Z-Wave -> Cluster (ReadOnly)
  * - ``VALUE``
    - ElectricalMeasurement PowerFactor
    - Z-Wave -> Cluster (ReadOnly)
  * - ``VALUE``
    - ElectricalMeasurement RMSVoltage
    - Z-Wave -> Cluster (ReadOnly)
  * - ``VALUE``
    - ElectricalMeasurement ReactivePower
    - Z-Wave -> Cluster (ReadOnly)

Command actions
---------------

.. list-table:: 
  :widths: 20 50 30
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Get Metering DeviceType
    - ``ucl/by-unid/+/+/Metering/Attributes/MeteringDeviceType/Reported {"value" : "ElectricMetering"}``
    - Only reported values
  * - Get Metering Unit of Measure
    - ``ucl/by-unid/+/+/Metering/Attributes/UnitofMeasure/Reported {"value" : "KilowattHoursOrKilowatts"}``
    - Only reported values
  * - Get Metering Unit of Current Summation Delivered
    - ``ucl/by-unid/+/+/Metering/Attributes/CurrentSummationDelivered/Reported {"value" : <REPORTED_CURRENT_SUMMATION_DELIVERED>}``
    - Only reported values
  * - Get Metering Unit of Current Summation Received
    - ``ucl/by-unid/+/+/Metering/Attributes/CurrentSummationReceived/Reported {"value" : <REPORTED_CURRENT_SUMMATION_RECEIVED>}``
    - Only reported values
  * - Get Electrical Measurement Active Power
    - ``ucl/by-unid/+/+/ElectricalMeasurement/Attributes/ActivePower/Reported {"value" :  <REPORTED_ACTIVE_POWER>}``
    - Only reported values
  * - Get Electrical Measurement RSM Current
    - ``ucl/by-unid/+/+/ElectricalMeasurement/Attributes/RMSCurrent/Reported {"value" :  <REPORTED_RMS_CURRENT>}``
    - Only reported values
  * - Get Electrical Measurement Power Factor
    - ``ucl/by-unid/+/+/ElectricalMeasurement/Attributes/PowerFactor/Reported {"value" :  <REPORTED_POWER_FACTOR>}``
    - Only reported values
  * - Get Electrical Measurement RSM Voltage
    - ``ucl/by-unid/+/+/ElectricalMeasurement/Attributes/RMSVoltage/Reported {"value" :  <REPORTED_RMS_VOLTAGE>}``
    - Only reported values
  * - Get Electrical Measurement Reactive Power
    - ``ucl/by-unid/+/+/ElectricalMeasurement/Attributes/ReactivePower/Reported {"value" :  <REPORTED_REACTIVE_POWER>}``
    - Only reported values