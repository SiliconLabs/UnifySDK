Humidity Control Operating State 
================================

Version supported : 1

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Humidity Control Operating State Get <humidity-control-operating-state-get>`

Command Class Commands
**********************


.. _humidity-control-operating-state-get:

Humidity Control Operating State Get
------------------------------------

Trigger on undefined **reported** value of ``CURRENT_STATE``

Humidity Control Operating State Report
---------------------------------------

Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``CURRENT_STATE``
    - ``Operating State``

Unify Clusters
**************

UAM files
---------

.. list-table:: 
  :header-rows: 1

  * - UAM File
    - Cluster
    - Comments
  * - ``Humidity.uam``
    - ``Unify_Humidity.xml``
    - No native cluster was available for this mapping
 
Bindings
--------

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Cluster attribute
    - Comments
  * -  ``CURRENT_STATE``
    - UnifyHumidity CurrentState
    - Z-Wave -> Cluster (Read only). 1:1 Mapping


Command actions
---------------

.. list-table:: 
  :widths: 20 60 20
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Get current operating state
    - ``ucl/by-unid/+/+/UnifyHumidityControl/Attributes/CurrentState/Reported`` ``{}``
    - Only reported values
