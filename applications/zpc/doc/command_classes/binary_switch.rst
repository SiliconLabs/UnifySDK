Binary Switch
=============

Version supported : 2

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Binary Switch Get <binary-switch-get>`

Command Class Commands
**********************

Binary Switch Set
-----------------

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


.. _binary-switch-get:

Binary Switch Get
-----------------

Trigger on undefined **reported** value of ``STATE``


Binary Switch Report
--------------------

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


.. note:: 
    The structure of the attribute store is : 

    .. code:: text
        
        |__ STATE
            |__ VALUE
            |__ DURATION


Unify Clusters
**************

UAM files
---------

.. list-table:: 
  :header-rows: 1

  * - UAM File
    - Cluster
    - Comments
  * - ``OnOff_to_BinarySwitch.uam``
    - ``OnOff.xml``
    - Used to control On/Off Binary Switch
 
Bindings
--------

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Cluster attribute
    - Comments
  * - ``VALUE``
    - OnOff On/Off
    - Z-Wave <-> Cluster (Read/Write)
 

Command actions
---------------

.. list-table:: 
  :widths: 20 50 30
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Turn On Switch
    - ``ucl/by-unid/+/+/OnOff/Commands/On {}``
    - Will set ``VALUE`` to 255
  * - Turn Off Switch
    - ``ucl/by-unid/+/+/OnOff/Commands/Off {}``
    - Will set ``VALUE`` to 0
  * - Toggle Switch
    - ``ucl/by-unid/+/+/OnOff/Commands/Toggle {}``
    - Will toggle ``VALUE``
  * - Get On/Off Value
    - ``ucl/by-unid/+/+/OnOff/Attributes/OnOff/[Desired|Reported] {"value": 1}``
    - 0 is Off. 1 is On

