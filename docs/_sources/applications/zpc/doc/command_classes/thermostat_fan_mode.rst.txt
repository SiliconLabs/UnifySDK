Thermostat Fan Mode 
===================

Version supported : 5

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Thermostat Fan Mode Supported Get <fan-mode-supported-get>`
#. :ref:`Thermostat Fan Mode Get <fan-mode-get>`

Command Class Commands
**********************


.. _fan-mode-supported-get:

Thermostat Fan Mode Supported Get 
---------------------------------

Trigger on undefined **reported** value of ``SUPPORTED_MODES``


Thermostat Fan Mode Supported Report 
------------------------------------

Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Bit Mask 1-N``
    - ``SUPPORTED_MODES``

.. note:: 
   Bit Mask 1-N will be merged in ``SUPPORTED_MODES``. Max bitmask length currently supported is N=4.
   You can increase this value by changing the underlying type of ``SUPPORTED_MODES`` (e.g. uint32_t -> uint64_t).


Thermostat Fan Mode Set 
-----------------------

Trigger on new **desired** value of ``CURRENT_MODE``

.. note::
  For version 2+ : Also trigger on new **desired** value of ``OFF_FLAG``


Mapping between attribute store and Set command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Set Field 
    - Version
  * - ``CURRENT_MODE``
    - Desired else Reported
    - ``Fan Mode``
    - All
  * - ``OFF_FLAG``
    - Desired else Reported
    - ``Off Flag``
    - 2-5 

.. note::
    If ``OFF_FLAG`` has a value > 1, then it will be forced to 1.

.. _fan-mode-get:

Thermostat Fan Mode Get
-----------------------

Trigger on undefined **reported** value of ``CURRENT_MODE``

.. note::
  For version 2+ : Also trigger on undefined **reported** value of ``OFF_FLAG``

Thermostat Fan Mode Report
--------------------------

Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
    - Version
  * - ``Fan Mode``
    - ``CURRENT_MODE``
    - All
  * - ``Off Flag``
    - ``OFF_FLAG``
    - 2-5



Unify Clusters
**************

UAM files
---------

.. list-table:: 
  :header-rows: 1

  * - UAM File
    - Cluster
    - Comments
  * - ``FanControl_to_ThermostatFanModeAndStateCC.uam``
    - ``FanControl.xml``
    - Map some attributes to the ones in the **native** FanControl cluster
  * - ``FanControl_to_ThermostatFanModeAndStateCC.uam``
    - ``Unify_FanControl.xml``
    - Used to map all the Z-Wave attributes to a custom cluster

.. note:: 
    Since FanControl.xml native cluster doesn't have all the attributes we need, we made the choice to map what we can to it.
    If you need exact Z-Wave attributes, you can use the Unify_FanControl.xml cluster.

Bindings
--------

.. list-table:: 
  :header-rows: 1
  :widths: 20 20 60

  * - Z-Wave Attribute Store
    - Cluster attribute
    - Comments
  * - ``OFF_FLAG``
    - FanControl FanMode
    - Z-Wave <-> Cluster (Read/Write). ``OFF_FLAG`` set if FanMode == 0. FanMode set to 0 if ``OFF_FLAG`` is set.
  * - ``CURRENT_MODE``
    - FanControl FanMode
    - Z-Wave <-> Cluster (Read/Write). Some values are not mapped since they doesn't exists (e.g. HumidityCirculation) and will be mapped to On instead
  * - ``CURRENT_MODE``
    - UnifyFanControl ZWaveFanMode
    - Z-Wave <-> Cluster (Read/Write). 1:1 Mapping
  * - ``SUPPORTED_MODES``
    - UnifyFanControl ZWaveSupportedFanMode
    - Z-Wave -> Cluster (Read only). 1:1 Mapping 
 
Command actions
---------------

.. list-table:: 
  :widths: 20 20 60
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Turn off the fan (native)
    - ``ucl/by-unid/+/+/FanControl/Commands/WriteAttributes`` ``{"FanMode": 0}``
    - Will send Z-Wave Thermostat Fan Mode Set command with ``OFF_FLAG`` set to 1 (version 2+), or with Off value for ```Fan Mode`` (version 1)
  * - Turn on the fan (native)
    - ``ucl/by-unid/+/+/FanControl/Commands/WriteAttributes`` ``{"FanMode": 4}``
    - You can set any value from 1-5 here to have the fan turned on. See Turn on the fan (Z-Wave) for more precision.
  * - Turn on the fan (Z-Wave)
    - ``ucl/by-unid/+/+/UnifyFanControl/Commands/SetFanMode`` ``{"FanMode": "RunningHigh"}``
    - Enum values are available in ``Unify_FanControl.xml`` (``ZWaveFanModeEnum``).
  * - Turn off the fan (Z-Wave)
    - ``ucl/by-unid/+/+/UnifyFanControl/Commands/TurnOff`` ``{}``
    -  Only supported for version 2+. Use SetFanMode with the right value (usually "Idle") for version 1.
  * - Get supported fan modes
    - ``ucl/by-unid/+/+/UnifyFanControl/Attributes/ZWaveSupportedFanMode``
    - Only available with the Unify_FanControl.xml cluster
