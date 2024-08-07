Door Lock
============

Version supported : 4

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Door Lock Operation Get <door-lock-operation-get>`
#. :ref:`Door Lock Configuration Get <door-lock-configuration-get>`
#. :ref:`Door Lock Capabilities Get <door-lock-capabilities-get>`

Command Class Commands
**********************

Door Lock Operation Set
-------------------------

Trigger on new **desired** value of ``ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_LOCK_MODE``

Mapping between attribute store and Set command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Set Field 
  * - ``ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_LOCK_MODE``
    - Desired 
    - ``Door Lock Mode``


.. _door-lock-operation-get:

Door Lock Operation Get
-------------------------

Trigger on undefined **reported** value of :

   - ``ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_LOCK_MODE``
   - ``ATTRIBUTE_DOOR_LOCK_OPERATION_INSIDE_DOOR_HANDLES_MODE``
   - ``ATTRIBUTE_DOOR_LOCK_OPERATION_OUTSIDE_DOOR_HANDLES_MODE``
   - ``ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_CONDITION``
   - ``ATTRIBUTE_DOOR_LOCK_OPERATION_LOCK_TIMEOUT_MINUTES``
   - ``ATTRIBUTE_DOOR_LOCK_OPERATION_LOCK_TIMEOUT_SECONDS``
   - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_TARGET_MODE``
   - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_DURATION``


Door Lock Operation Report
--------------------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Current Door Lock Mode``
    - ``ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_LOCK_MODE``
  * - ``Outside Door Handles Mode``
    - ``ATTRIBUTE_DOOR_LOCK_OPERATION_OUTSIDE_DOOR_HANDLES_MODE``
  * - ``Inside Door Handles Mode``
    - ``ATTRIBUTE_DOOR_LOCK_OPERATION_INSIDE_DOOR_HANDLES_MODE``
  * - ``Door Condition``
    - ``ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_CONDITION``
  * - ``Remaining Lock Time Minutes``
    - ``ATTRIBUTE_DOOR_LOCK_OPERATION_LOCK_TIMEOUT_MINUTES``
  * - ``Remaining Lock Time Seconds``
    - ``ATTRIBUTE_DOOR_LOCK_OPERATION_LOCK_TIMEOUT_SECONDS``
  * - ``Target Door Lock Mode``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_TARGET_MODE``
  * - ``Duration``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_DURATION``


Door Lock Configuration Set
---------------------------

Trigger on new **desired** value of:

   - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OPERATION_TYPE``
   - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE``
   - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE``
   - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_MINUTES``
   - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_SECONDS``
   - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK``
   - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_HOLD_RELEASE_TIME``
   - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_BTB``
   - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_TA``

Mapping between attribute store and Set command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Set Field 
  * - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OPERATION_TYPE``
    - Desired 
    - ``Operation Type``
  * - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE``
    - Desired 
    - ``Outside Door Handles Enabled``
  * - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE``
    - Desired 
    - ``Inside Door Handles Enabled``


.. _door-lock-configuration-get:

Door Lock Configuration Get
---------------------------

Trigger on undefined **reported** value of : 

   - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OPERATION_TYPE``
   - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE``
   - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE``
   - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_MINUTES``
   - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_SECONDS``
   - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK``
   - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_HOLD_RELEASE_TIME``
   - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_BTB``
   - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_TA``

Door Lock Configuration Report
------------------------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Operation Type``
    - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OPERATION_TYPE``
  * - ``Outside Door Handles Enabled``
    - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE``
  * - ``Inside Door Handles Enabled``
    - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE``
  * - ``Lock Timeout Minutes``
    - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_MINUTES``
  * - ``Lock Timeout Seconds``
    - ``ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_SECONDS``
  * - ``Auto-relock time 1 (MSB)``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK``
  * - ``Auto-relock time 2 (LSB)``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK``
  * - ``Hold and release time 1 (MSB)``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_HOLD_RELEASE_TIME``
  * - ``Hold and release time 2 (LSB)``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_HOLD_RELEASE_TIME``
  * - ``BTB``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_BTB``
  * - ``TA``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_TA``


.. _door-lock-capabilities-get:

Door Lock Capabilities Get
--------------------------

Trigger on undefined **reported** value of : 

- ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_OPERATION_TYPE_BITMASK``
- ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_LOCK_MODE``
- ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_OUTSIDE_HANDLE_MODES_BITMASK``
- ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_INSIDE_HANDLE_MODES_BITMASK``
- ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_DOOR_COMPONENT_BITMASK``
- ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_ARS``
- ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_HRS``
- ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_TAS``
- ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_BTBS``


Door Lock Capabilities Report
-----------------------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Supported Operation Type Bit Mask 1-N``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_OPERATION_TYPE_BITMASK``
  * - ``Supported Door Lock Mode 1-N``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_LOCK_MODE``
  * - ``Supported Outside Handle Modes Bitmask``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_OUTSIDE_HANDLE_MODES_BITMASK``
  * - ``Supported Inside Handle Modes Bitmask``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_INSIDE_HANDLE_MODES_BITMASK``
  * - ``Supported door components``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_DOOR_COMPONENT_BITMASK``
  * - ``ARS``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_ARS``
  * - ``HRS``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_HRS``
  * - ``TAS``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_TAS``
  * - ``BTBS``
    - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_BTBS``


.. note:: 
    The structure of the attribute store is : 

    .. code:: text
        
        |__ ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OPERATION_TYPE
        |__ ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE
        |__ ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE
        |__ ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_MINUTES
        |__ ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_SECONDS
        |__ ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_LOCK_MODE
        |__ ATTRIBUTE_DOOR_LOCK_OPERATION_INSIDE_DOOR_HANDLES_MODE
        |__ ATTRIBUTE_DOOR_LOCK_OPERATION_OUTSIDE_DOOR_HANDLES_MODE
        |__ ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_CONDITION
        |__ ATTRIBUTE_DOOR_LOCK_OPERATION_LOCK_TIMEOUT_MINUTES
        |__ ATTRIBUTE_DOOR_LOCK_OPERATION_LOCK_TIMEOUT_SECONDS
        |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION
        |   |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK
        |   |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_HOLD_RELEASE_TIME
        |   |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_BTB
        |   |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_TA
        |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT
        |   |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_TARGET_MODE
        |   |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_DURATION
        |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES
            |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_OPERATION_TYPE_BITMASK
            |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_LOCK_MODE
            |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_OUTSIDE_HANDLE_MODES_BITMASK
            |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_INSIDE_HANDLE_MODES_BITMASK
            |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_DOOR_COMPONENT_BITMASK
            |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_ARS
            |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_HRS
            |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_TAS
            |__ ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_BTBS



Unify Clusters
**************

UAM files
---------

.. list-table:: 
  :header-rows: 1

  * - UAM File
    - Cluster
    - Comments
  * - ``DoorLock_to_DoorLockCC.uam``
    - ``DoorLock.xml``
    - Mapping with DoorLock Cluster
 
Bindings
--------

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Cluster attribute
    - Comments
  * - ``ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_LOCK_MODE``
    - DoorLock LockState
    - Z-Wave <-> Cluster (Read/Write)
  * - ``ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_DOOR_COMPONENT_BITMASK``
    - DoorLock LockType
    - If the device support version 4. Z-Wave -> Cluster (Read Only)
  * - ``0x00``
    - DoorLock LockType
    - Will be 0x00 If the device support version 2 .Z-Wave -> Cluster (Read Only)
  * - ``0xFF``
    - DoorLock ActuatorEnabled
    - Will be undefined if ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_LOCK_MODE  not define. otherwise 0xFF. Z-Wave -> Cluster (Read only)
 

Command actions
---------------

.. list-table:: 
  :widths: 20 50 30
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Get Door Lock state
    - ``ucl/by-unid/+/+/DoorLock/Attributes/LockState/[Reported|Desired] { "value": 1}``
    - Reported and Desired values
  * - Lock Door
    - ``ucl/by-unid/+/+/DoorLock/Commands/LockDoor {"PINOrRFIDCode": <PIN_OR_RFID_CODE_VALUE>}``
    - ``<PIN_OR_RFID_CODE_VALUE>`` is string.
  * - UnLock Door
    - ``ucl/by-unid/+/+/DoorLock/Commands/UnlockDoor {"PINOrRFIDCode": <PIN_OR_RFID_CODE_VALUE>}``
    - ``<PIN_OR_RFID_CODE_VALUE>`` is string.
  * - UnLock Door with Timeout
    - ``ucl/by-unid/+/+/DoorLock/Commands/UnlockWithTimeout {"TimeoutInSeconds": 5, "PINOrRFIDCode": <PIN_OR_RFID_CODE_VALUE>}``
    - UnLock Door and after  ``4 Second`` Lock Door. ``<PIN_OR_RFID_CODE_VALUE>`` is string.
  * - Toggle Door
    - ``ucl/by-unid/+/+/ DoorLock/Commands/Toggle {"PINOrRFIDCode": <PIN_OR_RFID_CODE_VALUE>}``
    - ``<PIN_OR_RFID_CODE_VALUE>`` is string.
  * - Get Door Lock Type
    - ``ucl/by-unid/+/+/DoorLock/Attributes/LockType/Reported { "value": 1}``
    - Only Reported value
  * - Get the ability to handle Lock, Unlock or Remote Unlock with Timeout
    - ``ucl/by-unid/+/+/DoorLock/Attributes/ActuatorEnabled/Reported { "value": 1}``
    - Only Reported value
