Alarm Sensor
============

Version supported : 1

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Alarm Sensor Supported Get <alarm-sensor-supported-get>`
#. For each supported Sensor Type: :ref:`Alarm Sensor Get <alarm-sensor-get>`

Command Class Commands
**********************

.. _alarm-sensor-supported-get:

Alarm Sensor Supported Get
--------------------------

Trigger on undefined **reported** value of ``BITMASK``


Alarm Sensor Supported Report
-----------------------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Bit Mask 1-N``
    - ``BITMASK``

.. _alarm-sensor-get:

Alarm Sensor Get
----------------

Trigger on undefined **reported** value of ``STATE``


Alarm Sensor Report
-------------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Source Node ID``
    - ``NODE_ID``
  * - ``Sensor Type``
    - ``TYPE``
  * - ``Sensor State``
    - ``STATE``
  * - ``Second``
    - ``SECONDS``

.. note:: 
    The structure of the attribute store is : 

    .. code:: text
        
        |__ BITMASK
        |__ TYPE
            |__ STATE
            |__ NODE_ID
            |__ SECONDS


Unify Clusters
**************

UAM files
---------

.. list-table:: 
  :header-rows: 1

  * - UAM File
    - Cluster
    - Comments
  * - ``IASZone_MotionSensor_to_AlarmSensor.uam``
    - ``IASZone.xml``
    - Mapping IAS Zone Motion Sensor to Alarm Sensor. All of bound attributes are read only.
 
Bindings
--------

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Cluster attribute
    - Comments
  * - ``1``
    - IASZone ZoneState
    - Will be undefined if STATE doesn't exists otherwise 1. Z-Wave -> Cluster (Read only)
  * - ``0``
    - IASZone ZoneType
    - Will be undefined if STATE doesn't exists otherwise 1. Z-Wave -> Cluster (Read only)
  * - ``STATE``
    - IASZone ZoneStatus
    - Z-Wave -> Cluster (Read only)
 

Command actions
---------------

.. list-table:: 
  :widths: 20 50 30
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Get current operating state
    - ``ucl/by-unid/+/+/IASZone/Attributes/ZoneStatus/Reported {“value” : <REPORTED_ZONE_STATUS>}``
    - Only reported values. Return Object of Zone Status

