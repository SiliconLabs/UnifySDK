Thermostat Operating State
==========================

Version supported : 1 (2 partially supported see note bellow)

.. note:: 
    Log commands in version 2 are supported in ZPC but not exposed to MQTT yet.

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Thermostat Operating State Get <thermostat_operating_state_get>`
#. (v2 only, zpc only) :ref:`Thermostat Operating State Logging Supported Get <thermostat_operating_state_logging_supported_get>` 
#. (v2 only, zpc only) :ref:`Thermostat Operating State Logging Get <thermostat_operating_state_logging_get>` 


Command Class Commands
**********************

.. _thermostat_operating_state_get:

Thermostat Operating State Get
------------------------------

Trigger on undefined **reported** value of ``CURRENT_STATE``

Thermostat Operating State Report
---------------------------------

Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Operating State``
    - ``CURRENT_STATE``


.. _thermostat_operating_state_logging_supported_get:

Thermostat Operating State Logging Supported Get
------------------------------------------------

.. warning:: 
    Not yet exposed to MQTT

Trigger on undefined **reported** value of ``LOG_SUPPORTED_BITMASK``


Thermostat Operating State Logging Supported Report
---------------------------------------------------

.. warning:: 
    Not yet exposed to MQTT
    
Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Bitmask 1-N``
    - ``LOG_SUPPORTED_BITMASK``

.. note:: 
   Bit Mask 1-N will be merged in ``SUPPORTED_MODES``. Max bitmask length currently supported is N=4.
   You can increase this value by changing the underlying type of ``SUPPORTED_MODES`` (e.g. uint32_t -> uint64_t).


This command will also create : 

 - An attribute ``STATE_LOG_SUPPORTED`` for each supported log state supported.
 - Set ``LOG_BITMASK`` desired value to ``LOG_SUPPORTED_BITMASK`` to get all the logs. 


.. _thermostat_operating_state_logging_get:

Thermostat Operating State Logging Get
--------------------------------------

.. warning:: 
    Not yet exposed to MQTT

Trigger on undefined **reported** value of ``LOG_BITMASK``


Thermostat Operating State Logging Report
-----------------------------------------

.. warning:: 
    Not yet exposed to MQTT

For each ``Reports to Follow`` create an associated ``Operating State Log Type`` with its children : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Operating State Log Type``
    - ``LOG_STATE``
  * - ``Usage Today (Hours)``
    - ``USAGE_TODAY_HOURS``
  * - ``Usage Today (Minutes)``
    - ``USAGE_TODAY_MIN``
  * - ``Usage Yesterday  (Hours)``
    - ``USAGE_YESTERDAY_HOURS``
  * - ``Usage Yesterday  (Minutes)``
    - ``USAGE_YESTERDAY_MIN``


.. note:: 
    The structure of the attribute store is : 

    .. code:: text
        
        |__ LOG_SUPPORTED_BITMASK
        |__ LOG_BITMASK 
            |__ LOG_STATE
                |__ USAGE_TODAY_HOURS
                |__ USAGE_TODAY_MIN
                |__ USAGE_YESTERDAY_HOURS
                |__ USAGE_YESTERDAY_MIN
            |__ LOG_STATE
                |__ USAGE_TODAY_HOURS
                |__ USAGE_TODAY_MIN
                |__ USAGE_YESTERDAY_HOURS
                |__ USAGE_YESTERDAY_MIN


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
  * - ``CURRENT_STATE``
    - Thermostat ThermostatRunningState
    - Most of the values are mapped (but not all). Z-Wave -> Cluster (Read only). 
  * - ``CURRENT_STATE``
    - Thermostat OperatingState
    - Z-Wave -> Cluster (Read only). 1:1 mapping.
  
Command actions
---------------

.. list-table:: 
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Get thermostat state (native)
    - ``ucl/by-unid/+/+/Thermostat/Attributes/ThermostatRunningState/Reported``
    - Most of the values are mapped (but not all). Use uint and not enums.
  * - Turn on the fan (Z-Wave)
    - ``ucl/by-unid/+/+/UnifyThermostat/Attributes/OperatingState/Reported``
    - Enum values are available in ``Unify_Thermostat.xml`` (``OperatingState``). 1:1 mapping.