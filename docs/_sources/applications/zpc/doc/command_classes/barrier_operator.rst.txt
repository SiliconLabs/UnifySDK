Barrier Operator
================

Version supported : 1

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Barrier Operator Get Signaling Capabilities Supported <barrier-operator-signaling_capabilities-get>`
#. For each supported Subsystem type : :ref:`Barrier Operator Event Signaling Get <barrier-operator-event-signal-get>`
#. :ref:`Barrier Operator Get <barrier-operator-get>`



Command Class Commands
**********************

Barrier Operator Set 
--------------------

Trigger on new **desired** value of ``STATE``

Mapping between attribute store and Set command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Set Field 
  * - ``STATE``
    - Desired or Reported
    - ``Target Value``

.. _barrier-operator-get:

Barrier Operator Get
--------------------

Trigger on undefined **reported** value of ``STATE``


Barrier Operator Report
-----------------------

Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``State``
    - ``STATE``

.. _barrier-operator-signaling_capabilities-get:

Barrier Operator Get Signaling Capabilities Supported
-----------------------------------------------------

Trigger on undefined **reported** value of ``BITMASK``


Barrier Operator Report Signaling Capabilities Supported
--------------------------------------------------------

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``BITMASK``
    - ``Bitmask 1-N``

.. note:: 
   ``BITMASK`` is an array and each field will be put in this array.

.. note:: 
   For each supported type, zpc will create an ``SUBSYSTEM_TYPE`` attribute that contains the value of the supported type.
   Also create ``SUBSYSTEM_STATE`` to it : 

    .. code:: text

      |__ SUBSYSTEM_TYPE (reported value defined)
          |__ SUBSYSTEM_STATE (no value defined)
      |__ SUBSYSTEM_TYPE (reported value defined)
          |__ SUBSYSTEM_STATE (no value defined)
      |__ SUBSYSTEM_TYPE (reported value defined)
          |__ SUBSYSTEM_STATE (no value defined)

Barrier Operator Event Signal Set
---------------------------------

Trigger on new **desired** value of ``SUBSYSTEM_STATE``

Mapping between attribute store and Set command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Set Field 
  * - ``SUBSYSTEM_TYPE``
    - Reported
    - ``Subsystem Type``
  * - ``SUBSYSTEM_STATE``
    - Desired or Reported
    - ``Subsystem State``

.. note:: 
   Structure of subsystem :

    .. code:: text

      |__ SUBSYSTEM_TYPE
          |__ SUBSYSTEM_STATE

.. _barrier-operator-event-signal-get:

Barrier Operator Event Signaling Get
------------------------------------

Trigger on undefined **reported** value of ``SUBSYSTEM_STATE``

Mapping between attribute store and Get command : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Get Field 
  * - ``SUBSYSTEM_TYPE``
    - Reported
    - ``Subsystem Type``

.. note:: 
   Structure of subsystem :

    .. code:: text

      |__ SUBSYSTEM_TYPE
          |__ SUBSYSTEM_STATE

Barrier Operator Event Signaling Report
---------------------------------------

Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Subsystem Type`` 
    - ``SUBSYSTEM_TYPE``
  * - ``Subsystem State``
    - ``SUBSYSTEM_STATE`` 

Unify Clusters
**************

UAM files
---------

.. list-table:: 
  :header-rows: 1

  * - UAM File
    - Cluster
    - Comments
  * - ``BarrierControl_to_Barrier_Operator.uam``
    - ``BarrierControl.xml``
    - All of bound attributes a readonly.
 
Bindings
--------

.. list-table:: 
  :header-rows: 1
  :widths: 20 30 50

  * - Z-Wave Attribute Store
    - Cluster attribute
    - Comments
  * - 0
    - BarrierControl SafetyStatus
    - Will be undefined if ``STATE`` == 0 otherwise 0. Z-Wave -> Cluster (Read only).
  * - 0
    - BarrierControl Capabilities
    - Will be undefined if ``STATE`` == 0 otherwise 0. Z-Wave -> Cluster (Read only).
  * - ``STATE``
    - BarrierControl MovingState
    - MovingState is determined by the value of current State. Z-Wave -> Cluster (Read only).
  * - ``STATE``
    - BarrierControl BarrierPosition
    - Map current position based on the current State. Z-Wave -> Cluster (Read only).


Command actions
---------------

.. warning:: 
   Most of attributes are read only. You can only control the door opening and get the status back.

.. list-table:: 
  :widths: 20 50 30
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Make the door open
    - ``ucl/by-unid/+/+/BarrierControl/Commands/GoToPercent`` ``{ "PercentOpen": 12 }``
    - Any value > 0 will be mapped to ``STATE`` = 0xFF
  * - Make the door close
    - ``ucl/by-unid/+/+/BarrierControl/Commands/GoToPercent`` ``{ "PercentOpen": 0 }``
    - ``STATE`` will be 0x00