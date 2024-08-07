User Code
=========

Version supported : 2

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Users Number Get  <users_number_get>`
#. For each User Identifier: :ref:`User Code Get <users_code_get>`
#. :ref:`User Code Capabilities Get  <user_code_capabilities_get>`
#. :ref:`Admin Code Get  <admin_code_get>`
#. :ref:`User Code Checksum Get  <user_code_checksum_get>`
#. :ref:`User Code Keypad Mode Get  <user_code_keypad_mode_get>`


Command Class Commands
**********************

User Code Set
-------------

Trigger on new **desired** value of ``CODE``

Mapping between attribute store and Set command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Set Field 
  * - ``USER_ID``
    - Desired 
    - ``User Identifier``
  * - ``USER_ID_STATUS``
    - Desired
    - ``User ID Status``
  * - ``CODE``
    - Desired
    - ``User Code  1-N``

.. _users_code_get:

User Code Get
-------------

Trigger on undefined **reported** value of ``CODE``

Mapping between attribute store and Get command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Set Field 
  * - ``USER_ID``
    - Reported 
    - ``User Identifier``

User Code Report
----------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``User Identifier``
    - ``USER_ID``
  * - ``User ID Status``
    - ``USER_ID_STATUS``
  * - ``User Code 1-N``
    - ``CODE``


.. _users_number_get:

Users Number Get
----------------

Trigger on undefined **reported** value of ``NUMBER_OF_USERS``


Users Number Report
-------------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Supported Users``
    - ``NUMBER_OF_USERS``
  * - ``Extended Supported Users 1 (MSB)``
    - ``NUMBER_OF_USERS``
  * - ``Extended Supported Users 2 (LSB)``
    - ``NUMBER_OF_USERS``

.. _user_code_capabilities_get:

User Code Capabilities Get
--------------------------

Trigger on undefined **reported** value of ``SUPPORTED_FLAGS``


User Code Capabilities Report
-----------------------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``AC Support``
    - ``SUPPORTED_FLAGS``
  * - ``ACD Support``
    - ``SUPPORTED_FLAGS``
  * - ``Res``
    - ``SUPPORTED_FLAGS``
  * - ``UCC Support``
    - ``SUPPORTED_FLAGS``
  * - ``MUCR Support``
    - ``SUPPORTED_FLAGS``
  * - ``MUCS Support``
    - ``SUPPORTED_FLAGS``
  * - ``Supported User ID Status Bit Mask 1-N``
    - ``SUPPORTED_USER_ID_STATUS``
  * - ``Supported Keypad Modes Bit Mask 1-M``
    - ``SUPPORTED_KEYPAD_MODES``
  * - ``Supported Keys Bit Mask 1-L``
    - ``SUPPORTED_KEYS``


User Code Keypad Mode Set
-------------------------

Trigger on new **desired** value of ``KEYPAD_MODE``

Mapping between attribute store and Set command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Set Field 
  * - ``KEYPAD_MODE``
    - Desired 
    - ``Keypad Mode``


.. _user_code_keypad_mode_get:

User Code Keypad Mode Get
-------------------------

Trigger on undefined **reported** value of ``KEYPAD_MODE``


User Code Keypad Mode Report
----------------------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Keypad Mode``
    - ``KEYPAD_MODE``


Admin Code Set
--------------

Trigger on new **desired** value of ``ADMIN_CODE``

Mapping between attribute store and Set command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Set Field 
  * - ``Length of ADMIN_CODE``
    - Desired 
    - ``Admin Code Length``
  * - ``ADMIN_CODE``
    - Desired 
    - ``Admin Code 1-N`


.. _admin_code_get:

Admin Code Get
--------------

Trigger on undefined **reported** value of ``ADMIN_CODE``


Admin Code Report
-----------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Admin Code 1-N``
    - ``ADMIN_CODE``

.. _user_code_checksum_get:

User Code Checksum Get
----------------------

Trigger on undefined **reported** value of ``CHECKSUM``


User Code Checksum Report
-------------------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``User Code Checksum 1 (MSB)``
    - ``CHECKSUM``
  * - ``User Code Checksum 2 (LSB)``
    - ``CHECKSUM``


.. note:: 
    The structure of the attribute store is : 

    .. code:: text
        
        |__DATA
           |__ NUMBER_OF_USERS
           |__ DELETE_ALL_REQUESTED
           |__ USER_ID
           |   |__ CODE
           |   |__ USER_ID_STATUS
           |__ ADMIN_CODE
           |__ CAPABILITIES
           |   |__ SUPPORTED_FLAGS
           |   |__ SUPPORTED_USER_ID_STATUS
           |   |__ SUPPORTED_KEYPAD_MODES
           |   |__ SUPPORTED_KEYS
           |__ CHECKSUM
           |__ KEYPAD_MODE



Unify Clusters
**************

UAM files
---------

.. warning:: 
    No UAM files are used by this command class
 
Bindings
--------

.. warning:: 
    No direct bindings are used for this command class. See next section to see how to interact with it.
 

Command actions
---------------

.. list-table:: 
  :widths: 20 50 30
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Set PIN Code
    - ``ucl/by-unid/+/+/DoorLock/Commands/SetPINCode {"UserID": <USERID_VALUE>,"UserStatus": <USER_STATUS_VALUE>,"UserType": 0,"PIN": <PIN_VALUE>}``
    - If ``UserType`` = 0 will set PIN Code for normal User
  * - Set Admin PIN Code
    - ``ucl/by-unid/+/+/DoorLock/Commands/SetPINCode {"UserID": <USERID_VALUE>,"UserStatus": <USER_STATUS_VALUE>,"UserType": 3,"PIN": <PIN_VALUE>}``
    - If ``UserType`` = 3 will set PIN Code for Admin. ``UserID`` and ``UserStatus`` can take any number
  * - Get PIN Code
    - ``ucl/by-unid/+/+/DoorLock/Commands/GetPINCode { "UserID": <USERID_VALUE> }``
    - Will get ``USER_ID_STATUS`` for corresponding ``USER_ID``
  * - Clear PIN Code
    - ``ucl/by-unid/+/+/DoorLock/Commands/ClearPINCode { "UserID": <USERID_VALUE> }``
    - Will clear PIN Code for corresponding ``USER_ID``
  * - Clear All PIN Code
    - ``ucl/by-unid/+/+/DoorLock/Commands/ClearAllPINCodes' {  }``
    - Will clear PIN Code of all ``USER_ID``
  * - Set User Status
    - ``ucl/by-unid/+/+/DoorLock/Commands/SetUserStatus' { "UserID": <USERID_VALUE>,"UserStatus": <USER_STATUS_VALUE> }``
    - Will set ``USER_ID_STATUS`` for corresponding ``USER_ID``
  * - Get User Status
    - ``ucl/by-unid/+/+/DoorLock/Commands/GetUserStatus' { "UserID": <USERID_VALUE> }``
    - Will get ``CODE`` for corresponding ``USER_ID``