Thermostat Setpoint
===================


.. warning:: 
    It has been found that early implementations of this Command Class specification apply two
    non-interoperable interpretations of the bit mask advertising the support for specific Setpoint Types

    ZPC supports by default the "Interpretation A" supported by default in Thermostat Setpoint version 3.

    You can switch to "Interpretation B" for a specific device by using .uam quirks and set the  reported value of ``USE_B_INTERPRETATION`` to something > 0. (id : 0x430A)

Version supported : 3

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Thermostat Setpoint Supported Get <thermostat-control-setpoint-supported-get>`
#. For all supported types : 

   #. :ref:`Thermostat Setpoint Capabilities Get <thermostat-control-setpoint-capabilities-get>` (version 3 only)
   
   #. :ref:`Thermostat Setpoint Get <thermostat-control-setpoint-get>`

Command Class Commands
**********************

Thermostat Setpoint Set
-----------------------

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
    - Reported
    - ``PRECISION``
  * - ``Scale`` 
    - Reported
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


.. _thermostat-control-setpoint-get:

Thermostat Setpoint Get
-----------------------

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

Thermostat Setpoint Report
--------------------------

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

.. _thermostat-control-setpoint-supported-get:

Thermostat Setpoint Supported Get
---------------------------------

Trigger on undefined **reported** value of ``SUPPORTED_SETPOINT_TYPES``


Thermostat Setpoint Supported Report
------------------------------------

Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``SUPPORTED_SETPOINT_TYPES``
    - ``Bitmask 1-N``

.. note:: 
   Bit Mask 1-N will be merged in ``SUPPORTED_SETPOINT_TYPES``. Max bitmask length currently supported is N=4.
   You can increase this value by changing the underlying type of ``SUPPORTED_SETPOINT_TYPES`` (e.g. uint32_t -> uint64_t).

.. note:: 
   For each supported type, zpc will create an ``TYPE`` attribute that contains the value of the supported type.

    .. code:: text

        |__ SUPPORTED_SETPOINT_TYPES
        |__ TYPE
            |__ VALUE
        |__ TYPE
            |__ VALUE


.. _thermostat-control-setpoint-capabilities-get:

Thermostat Setpoint Capabilities Get
------------------------------------

.. note:: 
    Only if version 3 is supported.

Trigger on undefined **reported** value of ``MIN_VALUE``

Mapping between attribute store and Get command : (optional)

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


Thermostat Setpoint Capabilities Report
---------------------------------------

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

Z-Wave value can have a precision attribute, while on the UCL side the precision is always 2 in C°.

To handle this we don't use .uam file and instead we monitor changes on the zwave and ucl side and tries to adapt to one another.
See ``on_zwave_value_update`` and ``on_ucl_value_update`` in the ``applications/zpc/components/zwave_command_classes/src/zwave_command_class_thermostat_setpoint.cpp`` file.

.. note::
    Native cluster (ucl) used : ``Thermostat.xml``
 
Bindings
--------

.. warning:: 
    Only Heating and Cooling Setpoint are exposed. Other are available in the attribute store only.

    The ``VALUE`` will be converted to precision 2 and in C°. ``PRECISION`` value is not exposed. 

Applicable if parent ``TYPE`` is Heating (0x01) :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Cluster attribute
  * - ``VALUE`` 
    - Thermostat OccupiedHeatingSetpoint
  * - ``MIN_VALUE``
    - Thermostat MinHeatSetpointLimit
  * - ``MAX_VALUE``
    - Thermostat MaxHeatSetpointLimit

Applicable if parent ``TYPE`` is Cooling (0x02) :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Cluster attribute
  * - ``VALUE`` 
    - Thermostat OccupiedCoolingSetpoint
  * - ``MIN_VALUE``
    - Thermostat MinCoolSetpointLimit
  * - ``MAX_VALUE``
    - Thermostat MaxCoolSetpointLimit

Command actions
---------------

.. list-table:: 
  :widths: 20 50 30
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Adjust (raise of lower) a exposed setpoint
    - ``ucl/by-unid/+/+/Thermostat/Commands/SetpointRaiseOrLower`` ``{ "Mode": "Heat", "Amount": 100 }``
    - ``Mode`` can be ``Heat`` or ``Cool``. ``Amount`` is the amount to raise or lower the setpoint.
  * - Change Setpoint value (Heat)
    - ``ucl/by-unid/+/+/Thermostat/Commands/WriteAttributes`` ``{ "OccupiedHeatingSetpoint" : 2200 }``
    - Must be of precision 2 and in °C. ZPC will convert it to the previous received precision and scale.
  * - Change Setpoint value (Cool)
    - ``ucl/by-unid/+/+/Thermostat/Commands/WriteAttributes`` ``{ "OccupiedCoolingSetpoint" : 3200 }``
    - Must be of precision 2 and in °C. ZPC will convert it to the previous received precision and scale.
