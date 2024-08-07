Thermostat Fan State
====================

Version supported : 2

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Thermostat Fan State Get <thermostat-fan-state-get>`

Command Class Commands
**********************

.. _thermostat-fan-state-get:

Thermostat Fan State Get
------------------------

Trigger on undefined **reported** value of ``FAN_OPERATING_STATE``


Thermostat Fan State Report
---------------------------

Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Fan Operating State``
    - ``FAN_OPERATING_STATE`` 


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
    - ``Unify_FanControl.xml``
    - No equivalent are available in native clusters.
 
Bindings
--------

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Cluster attribute
    - Comments
  * - ``FAN_OPERATING_STATE``
    - UnifyFanControl ZWaveFanState
    - Z-Wave -> Cluster (Read only). 1:1 Mapping 
 
Command actions
---------------

.. list-table:: 
  :widths: 20 60 20
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Get the current fan state
    - ``ucl/by-unid/+/+/UnifyFanMode/Attributes/ZWaveSupportedFanState``
    - Read only
