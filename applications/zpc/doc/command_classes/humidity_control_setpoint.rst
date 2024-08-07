Humidity Control Setpoint 
=========================

Version supported : 2

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Humidity Control Setpoint Supported Get <humidity-control-setpoint-supported-get>`
#. For all supported types : 

   #. :ref:`Humidity Control Setpoint Capabilities Get <humidity-control-setpoint-capabilities-get>`
   
   #. :ref:`Humidity Control Setpoint Get <humidity-control-setpoint-get>`

Command Class Commands
**********************

Humidity Control Setpoint Set
-----------------------------

Trigger on new **desired** value of ``VALUE``

Mapping between attribute store and Set command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Set Field 
  * - ``TYPE`` (parent of ``VALUE``)
    - Reported
    - ``Setpoint Type``
  * - ``Precision``
    - Desired or Reported
    - ``PRECISION``
  * - ``Scale`` 
    - Desired or Reported
    - ``SCALE``
  * - ``Size`` 
    - None.
    - None. Will be inferred from the ``VALUE`` field.
  * - ``Value 1..N``
    - Desired
    - ``VALUE`` (max size = 4)

.. note:: 
    The structure of the attribute store is : 

    .. code:: text
        
        |__ TYPE
            |__ MIN_VALUE
                |__ MIN_SCALE
                |__ MIN_PRECISION
            |__ MAN_VALUE
                |__ MAX_SCALE
                |__ MAX_PRECISION
            |__ VALUE
                |__ SCALE
                |__ PRECISION


    This means we can deduce the Setpoint type from the ``VALUE`` and all other related attributes.


.. _humidity-control-setpoint-get:

Humidity Control Setpoint Get
-----------------------------

Trigger on undefined **reported** value of ``VALUE``

Mapping between attribute store and Get command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Get Field 
  * - ``TYPE``
    - Reported
    - ``Setpoint Type``

.. note:: 
    The structure of the attribute store is : 

    .. code:: text
        
        |__ TYPE
            |__ VALUE

    This means we can deduce the Setpoint type from the ``VALUE`` and all other related attributes.

Humidity Control Setpoint Report
--------------------------------

Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Setpoint Type``
    - Look for an existing reported ``TYPE`` that match this value
  * - ``Precision`` 
    - ``PRECISION``
  * - ``Scale`` 
    - ``SCALE``
  * - ``Size`` 
    - None. Will be inferred from the ``VALUE`` field.
  * - ``Value 1..N``
    - ``VALUE`` (max size = 4)


.. _humidity-control-setpoint-supported-get:

Humidity Control Setpoint Supported Get
---------------------------------------

Trigger on undefined **reported** value of ``SUPPORTED_TYPES``


Humidity Control Setpoint Supported Report
------------------------------------------

Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``SUPPORTED_TYPES``
    - ``Bitmask 1-N``

.. note:: 
   Bit Mask 1-N will be merged in ``SUPPORTED_TYPES``. Max bitmask length currently supported is N=4.
   You can increase this value by changing the underlying type of ``SUPPORTED_TYPES`` (e.g. uint32_t -> uint64_t).

.. note:: 
   For each supported type, zpc will create an ``TYPE`` attribute that contains the value of the supported type.

.. _humidity-control-setpoint-scale-supported-get:

Humidity Control Setpoint Scale Supported Get
---------------------------------------------

.. warning::
    Due to error in the specification this command is not supported.

    See :ref:`section below <humidity-control-setpoint-scale-supported-report>` for more details. 


.. _humidity-control-setpoint-scale-supported-report:

Humidity Control Setpoint Scale Supported Report
------------------------------------------------

.. warning::
    Due to error in the specification this command is not supported.

    The Setpoint Type is not repeated in the report command, so it is not possible to know which setpoint type the supported scale is applied. 



.. _humidity-control-setpoint-capabilities-get:

Humidity Control Setpoint Capabilities Get 
------------------------------------------

Trigger on undefined **reported** value of ``MIN_VALUE``

Mapping between attribute store and Get command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Get Field 
  * - ``TYPE`` (parent of ``MIN_VALUE``)
    - Reported
    - ``Setpoint Type``

.. note:: 
    We only need one binding to either ``MIN_VALUE`` or ``MAX_VALUE``. This implementation choose to listen to ``MIN_VALUE`` arbitrarily. 


Humidity Control Setpoint Capabilities Report
---------------------------------------------

Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Setpoint Type``
    - Look for an existing reported ``TYPE`` that match this value
  * - ``Precision`` (min)
    - ``MIN_PRECISION``
  * - ``Scale`` (min)
    - ``MIN_SCALE``
  * - ``Size`` (min)
    - None. Will be inferred from the ``MIN_VALUE`` field.
  * - ``Minimum value 1..N`` (min)
    - ``MIN_VALUE`` (max size = 4)
  * - ``Precision`` (max)
    - ``MAX_PRECISION``
  * - ``Scale`` (max)
    - ``MAX_SCALE``
  * - ``Size`` (max)
    - None. Will be inferred from the ``MAX_VALUE`` field.
  * - ``Maximum value 1..N`` (max)
    - ``MAX_VALUE`` (max size = 4)


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
  :widths: 25 50 25

  * - Z-Wave Attribute Store
    - Cluster attribute
    - Comments
  * -  ``SUPPORTED_TYPES``
    - UnifyHumidity SupportedSetPoints
    - Z-Wave -> Cluster (Read only). 1:1 Mapping
  * - ``VALUE`` (setpoint type = X)
    - UnifyHumidity {SETPOINT_TYPE}Setpoint (e.g if setpoint type = 1, SETPOINT_TYPE Humidifier)
    - Z-Wave -> Cluster (Read only). 1:1 Mapping
  * - ``MIN_VALUE`` (setpoint type = X)
    - UnifyHumidity {SETPOINT_TYPE}SetpointMin (e.g if setpoint type = 1, SETPOINT_TYPE Humidifier)
    - Z-Wave -> Cluster (Read only). 1:1 Mapping
  * - ``MAX_VALUE`` (setpoint type = X)
    - UnifyHumidity {SETPOINT_TYPE}SetpointMax (e.g if setpoint type = 1, SETPOINT_TYPE Humidifier)
    - Z-Wave -> Cluster (Read only). 1:1 Mapping
  * - ``SCALE`` (setpoint type = X)
    - UnifyHumidity {SETPOINT_TYPE}SetpointScale (e.g if setpoint type = 1, SETPOINT_TYPE Humidifier)
    - Z-Wave -> Cluster (Read only). 1:1 Mapping
  * - ``PRECISION`` (setpoint type = X)
    - UnifyHumidity {SETPOINT_TYPE}SetpointPrecision (e.g if setpoint type = 1, SETPOINT_TYPE Humidifier)
    - Z-Wave -> Cluster (Read only). 1:1 Mapping


Command actions
---------------

.. list-table:: 
  :widths: 20 50 30
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Get device supported Setpoints
    - ``ucl/by-unid/+/+/UnifyHumidityControl/Attributes/SupportedSetPoints/Reported`` ``{}``
    - Return a array of supported modes
  * - Set setpoint value
    - ``'ucl/by-unid/+/+/UnifyHumidityControl/Commands/SetpointSet' -m  '{ "Type": "Humidifier","Precision": 1,"Scale": 0,"Value": 1000 }'``
    - Setpoint type must be supported by the end device. Checks will be performed to see if value is in bounds.
