Multilevel Switch
=================

Version supported : 4

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Multilevel Switch Get <multilevel-switch-get>`
#. :ref:`Multilevel Switch Supported Get <multilevel-switch-supported-get>`

Command Class Commands
**********************

Multilevel Switch Set
-------------------------

Trigger on new **desired** value of ``STATE``

Mapping between attribute store and Set command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Set Field 
  * - ``VALUE``
    - Desired 
    - ``Target Value``
  * - ``DURATION``
    - Desired
    - ``Duration``


.. _multilevel-switch-get:

Multilevel Switch Get
---------------------

Trigger on undefined **reported** value of ``STATE``


Multilevel Switch Report
------------------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Current Value``
    - ``VALUE``
  * - ``Target Value``
    - ``VALUE``
  * - ``Duration``
    - ``DURATION``


.. _multilevel-switch-supported-get:

Multilevel Switch Supported Get
-------------------------------

Trigger on undefined **reported** value of ``CAPABILITIES_REQUESTED``


Multilevel Switch Supported Report
----------------------------------

Mapping between Report command and attribute store :

.. warning::
    We do nothing with this frame, just mark value of ``CAPABILITIES_REQUESTED`` in the attribute store that the capabilities have been requested. 

.. note:: 
    The structure of the attribute store is : 

    .. code:: text
        
        |__ STATE
        |   |__ VALUE
        |   |__ DURATION
        |__ CAPABILITIES_REQUESTED


Unify Clusters
**************

UAM files
---------

.. list-table:: 
  :header-rows: 1

  * - UAM File
    - Cluster
    - Comments
  * - ``OnOff_to_MultilevelSwitchCC.uam``
    - ``OnOff.xml``
    - Mapping with OnOff Cluster
  * - ``Level_to_MultilevelSwitchCC.uam``
    - ``Level.xml``
    - Mapping with Level Cluster
 
Bindings
--------

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Cluster attribute
    - Comments
  * - ``VALUE``
    - OnOff On/Off
    - Z-Wave -> Cluster (Read Only)
  * - ``VALUE``
    - Level CurentLevel
    - Z-Wave <-> Cluster (Read/Write)
  * - ``DURATION``
    - Level TransitionTime
    - Z-Wave <-> Cluster (Read/Write)
  * - ``DURATION``
    - Level RemainingTime
    - Z-Wave -> Cluster (Read Only)
  * - ``0``
    - Level MinLevel
    - Z-Wave -> Cluster (Read Only)
  * - ``99``
    - Level MaxLevel
    - Z-Wave -> Cluster (Read Only)
 

Command actions
---------------

.. list-table:: 
  :widths: 20 50 30
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Move to Level in time
    - ``ucl/by-unid/+/+/Level/Commands/MoveToLevel { "Level": 50, "TransitionTime": 50, "OptionsMask": {"ExecuteIfOff" : 0, "CoupleColorTempToLevel" : 0},"OptionsOverride": {"ExecuteIfOff" : 0, "CoupleColorTempToLevel" : 0} }``
    - Will set ``VALUE`` to 50 and ``DURATION`` to 50 second
  * - Move to Level in time
    - ``ucl/by-unid/+/+/Level/Commands/Move { "MoveMode": 1, "Rate": 10,"OptionsMask": {"ExecuteIfOff" : 0, "CoupleColorTempToLevel" : 0},"OptionsOverride": {"ExecuteIfOff" : 0, "CoupleColorTempToLevel" : 0} }``
    - 
  * - Move to Level in time
    - ``ucl/by-unid/+/+/Level/Commands/Step { "StepMode": 1,"StepSize": 10, "TransitionTime": 30,"OptionsMask": {"ExecuteIfOff" : 0, "CoupleColorTempToLevel" : 0},"OptionsOverride": {"ExecuteIfOff" : 0, "CoupleColorTempToLevel" : 0} }``
    - 
  * - Stop at Current Level now
    - ``ucl/by-unid/+/+/Level/Commands/Stop { "OptionsMask": {"ExecuteIfOff" : 0, "CoupleColorTempToLevel" : 0},"OptionsOverride": {"ExecuteIfOff" : 0, "CoupleColorTempToLevel" : 0} }``
    - 
  * - Move to Level in time with OnOff
    - ``ucl/by-unid/+/+/Level/Commands/MoveToLevelWithOnOff { "Level": 50, "TransitionTime": 50, "OptionsMask": {"ExecuteIfOff" : 0, "CoupleColorTempToLevel" : 0},"OptionsOverride": {"ExecuteIfOff" : 0, "CoupleColorTempToLevel" : 0} }}``
    - 
  * - Move to Level in time with OnOff
    - ``ucl/by-unid/+/+/Level/Commands/MoveWithOnOff { "MoveMode": 1, "Rate": 10, "OptionsMask": {"ExecuteIfOff" : 0, "CoupleColorTempToLevel" : 0},"OptionsOverride": {"ExecuteIfOff" : 0, "CoupleColorTempToLevel" : 0} }``
    - 
  * - Move to Level in time with OnOff
    - ``ucl/by-unid/+/+/Level/Commands/StepWithOnOff { "StepMode": 1,"StepSize": 10, "TransitionTime": 30,"OptionsMask": {"ExecuteIfOff" : 0, "CoupleColorTempToLevel" : 0},"OptionsOverride": {"ExecuteIfOff" : 0, "CoupleColorTempToLevel" : 0} }``
    - 
  * - Stop at Current Level now with OnOff
    - ``ucl/by-unid/+/+/Level/Commands/StopWithOnOff { "OptionsMask": {"ExecuteIfOff" : 0, "CoupleColorTempToLevel" : 0},"OptionsOverride": {"ExecuteIfOff" : 0, "CoupleColorTempToLevel" : 0} }``
    - 
  * - Get Current Level
    - ``ucl/by-unid/+/+/Level/Attributes/CurrentLevel/Desired { "value": 10}    ucl/by-unid/+/+/Level/Attributes/CurrentLevel/Reported { "value": 10}``
    - Reported and Desired value
  * - Get Remaining Time
    - ``ucl/by-unid/+/+/Attributes/RemainingTime/Reported { "value": 60}``
    - Only Report value
  * - Get Transition Time
    - ``ucl/by-unid/+/+/Attributes/OnOffTransitionTime/Desired { "value": 60}   ucl/by-unid/+/+/Attributes/OnOffTransitionTime/Reported { "value": 60}``
    - Reported and Desired value