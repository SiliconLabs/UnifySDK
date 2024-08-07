Thermostat Mode
===============

Version supported : 3

.. warning:: 
    The command class implementation doesn't support the MANUFACTURER SPECIFIC mode.
    
    You can still set the mode using the ``Mode`` field in the Set command. But you cannot set extra data to the Set command.

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Thermostat Mode Supported Get <thermostat-supported-mode-get>`
#. :ref:`Thermostat Mode Get <thermostat-mode-get>`


Command Class Commands
**********************

Thermostat Mode Set
-------------------

Trigger on new **desired** value of ``ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE``

Mapping between attribute store and Set command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Set Field 
  * - ``ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE``
    - Desired
    - ``Mode``	

.. _thermostat-mode-get:

Thermostat Mode Get
-------------------

Trigger on undefined **reported** value of ``ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE``


Thermostat Mode Report
----------------------

Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Mode``	
    - ``ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE`` 

.. _thermostat-supported-mode-get:

Thermostat Supported Mode Get
-----------------------------

Trigger on undefined **reported** value of ``ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES``

Thermostat Supported Mode Report
--------------------------------

Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Bitmask 1-N``	
    - ``ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES`` 

.. note:: 
   Bit Mask 1-N will be merged in ``ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES``. Max bitmask length currently supported is N=4.
   You can increase this value by changing the underlying type of ``ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES`` (e.g. uint32_t -> uint64_t).


Unify Clusters
**************

UAM files
---------

.. list-table:: 
  :header-rows: 1

  * - UAM File
    - Cluster
    - Comments
  * - ``Thermostat.uam``
    - ``Thermostat.xml``
    - Map some attributes to the ones in the **native** Thermostat cluster
  * - ``Thermostat.uam``
    - ``Unify_Thermostat.xml``
    - Used to map all the Z-Wave attributes to a custom cluster
 
Bindings
--------

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Cluster attribute
    - Comments
  * - ``ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE``
    - Thermostat SystemMode
    - Most of the values are mapped (but not all). Z-Wave <-> Cluster (Read/Write). 
  * - ``ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE``
    - UnifyThermostat ThermostatMode
    - Z-Wave <-> Cluster (Read/Write). 1:1 mapping.
  * - ``ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES``
    - UnifyThermostat SupportedThermostatMode
    - Z-Wave -> Cluster (Read only). 1:1 mapping.

Command actions
---------------

.. list-table:: 
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Change thermostat mode (native)
    - ``ucl/by-unid/+/+/Thermostat/Commands/WriteAttributes`` ``{"SystemMode": 4}``
    - Not all value could be mapped. Check ``Thermostat.uam`` (bindings of ``r'zwTHERMOSTAT_MODE`` for more details).
  * - Change thermostat mode (Z-Wave)
    - ``ucl/by-unid/+/+/UnifyThermostat/Commands/WriteAttributes`` ``{"ThermostatMode": "Heat"}``
    - Enum values are available in ``Unify_Thermostat.xml`` (``ZWaveFanStateEnum``).
  * - Get supported thermostat modes.
    - ``ucl/by-unid/+/+/UnifyThermostat/Attributes/SupportedThermostatMode/Reported``
    - Get and array of supported thermostat modes.
  