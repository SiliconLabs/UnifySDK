Humidity Control Mode 
=====================

Version supported : 2

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Humidity Control Mode Supported Get <supported-modes-command>`
#. :ref:`Humidity Control Mode Get <mode-get-command>`

Command Class Commands
**********************

Humidity Control Mode Set
-------------------------

Trigger on new **desired** value of ``CURRENT_MODE``

Mapping between attribute store and Set command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Set Field 
  * - ``CURRENT_MODE``
    - Desired
    - ``Mode``

.. note:: 
   ZPC will check before sending that the ``CURRENT_MODE`` is compatible with the ``SUPPORTED_MODES``.


.. _mode-get-command:

Humidity Control Mode Get
-------------------------

Trigger on undefined **reported** value of ``CURRENT_MODE``


Humidity Control Mode Report
----------------------------

Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Mode``
    - ``CURRENT_MODE``

.. note:: 
   ZPC will check that the received ``CURRENT_MODE`` is compatible with the ``SUPPORTED_MODES``.

.. _supported-modes-command:

Humidity Control Mode Supported Get
-----------------------------------

Trigger on undefined **reported** value of ``SUPPORTED_MODES``


Humidity Control Mode Supported Report
--------------------------------------

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
  * -  ``CURRENT_MODE``
    - UnifyHumidity ReportingMode
    - Z-Wave -> Cluster (Read only). 1:1 Mapping
  * - ``SUPPORTED_MODES``
    - UnifyHumidity SupportedReportingMode
    - Z-Wave -> Cluster (Read only). 1:1 Mapping

Command actions
---------------

.. list-table:: 
  :widths: 20 40 40
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Change current mode
    - ``ucl/by-unid/+/+/UnifyHumidityControl/Commands/ModeSet`` ``{ "Mode": "ModeType" }``
    - Mode is one of the "ModeType" enum8 from the cluster.
  * - Get supported modes
    - ``ucl/by-unid/+/+/UnifyHumidityControl/Attributes/SupportedReportingMode/Reported`` ``{}``
    - Return a array of supported modes
  * - Get current mode
    - ``ucl/by-unid/+/+/UnifyHumidityControl/Attributes/ReportingMode/Reported`` ``{}``
    - Can also be Desired instead of reported