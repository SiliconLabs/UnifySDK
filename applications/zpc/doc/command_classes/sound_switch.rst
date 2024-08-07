Sound Switch
============

Version supported : 2

.. contents::
   :depth: 2
   :local:
   :backlinks: none


Interview process
*****************

#. :ref:`Tones Number Get <tones-number-get-command>`
#. For each tone number reported : :ref:`Tone Info Get <tone-info-get-command>`
#. :ref:`Configuration Get <configuration-get-command>`
#. :ref:`Tone Play Get <tone-play-get-command>`

Command Class Commands
**********************

.. _tones-number-get-command:

Tones Number Get 
----------------

Trigger on undefined **reported** value of ``TONES_NUMBER``

Tones Number Report 
-------------------

Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Supported Tones``
    - ``TONES_NUMBER`` 

.. _tone-info-get-command:

Tone Info Get
-------------

Trigger on undefined **reported** value of ``TONE_INFO_IDENTIFIER``

Mapping between attribute store and Get command : 

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Get Field 
  * - ``TONE_INFO_IDENTIFIER``
    - Desired 
    - ``Tone Identifier``


Tone Info Report
----------------

Mapping between Report command and attribute store : 

.. list-table:: 
  :header-rows: 1

  * - Report Field Command 
    - Z-Wave Attribute Store 
  * - ``Tone Identifier``
    - ``TONE_INFO_IDENTIFIER``
  * - ``Tone Duration (1 & 2)``
    - ``TONE_INFO_DURATION_SECONDS``
  * - ``Name Length``
    - None. Deducted from ``TONE_INFO_NAME``
  * - ``Name``
    - ``TONE_INFO_NAME``

Configuration Set
-----------------

Trigger on new **desired** value of : 

   - ``CONFIGURED_DEFAULT_VOLUME``
   - ``CONFIGURED_DEFAULT_TONE_IDENTIFIER``

.. note:: 
  Trigger function is mapped to both attributes. Any of those attributes that have a desired value will trigger this function. The Set command look for desired and reported value so you don't have to set both desired values.

Mapping between attribute store and Set command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Set Field 
  * - ``CONFIGURED_DEFAULT_VOLUME``
    - Desired else reported
    - ``Volume``
  * - ``CONFIGURED_DEFAULT_TONE_IDENTIFIER``
    - Desired else reported
    - ``Default Tone Identifier``

.. _configuration-get-command:

Configuration Get
-----------------

Trigger on undefined **reported** value any of :
   - ``CONFIGURED_DEFAULT_VOLUME``
   - ``CONFIGURED_DEFAULT_TONE_IDENTIFIER``

.. note:: 
  Trigger function is mapped to both attributes. Any of those attributes that doesn't have a reported value will trigger this command.

Configuration Report
--------------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Report Field Command 
    - Z-Wave Attribute Store 
  * - ``Volume``
    - ``CONFIGURED_DEFAULT_VOLUME``
  * - ``Default Tone Identifier``
    - ``CONFIGURED_DEFAULT_TONE_IDENTIFIER``


Tone Play Set
-------------

Trigger on new **desired** value of  ``TONE_PLAY``

``TONE_PLAY`` represent the current node played. It follows the Table 2.517 of the Z-Wave specification.

.. list-table:: 
  :header-rows: 1

  * - Value
    - Description
  * - 0x00
    - No tone is currently played
  * - 0x01..0xFE
    - Specific tone is played
  * - 0xFF
    - Default tone is currently played


Mapping between attribute store and Set command :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Attribute State
    - Z-Wave Set Field 
  * - ``TONE_PLAY``
    - Desired else reported
    - ``Tone Identifier``
  * - ``CONFIGURED_DEFAULT_VOLUME``
    - Desired else reported
    - ``Play Command Tone Volume``

.. _tone-play-get-command:

Tone Play Get
-------------

Trigger on undefined **reported** value of ``TONE_PLAY``

Tone Play Report
----------------

Mapping between Report command and attribute store :

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Command Attribute 
    - Z-Wave Attribute Store
  * - ``Tone Identifier``
    - ``TONE_PLAY``
  * - ``Play Command Tone Volume``
    - ``CONFIGURED_DEFAULT_VOLUME``


Unify Clusters
**************

UAM files
---------

.. list-table:: 
  :header-rows: 1

  * - UAM File
    - Cluster
    - Comments
  * - ``Level_to_SoundSwitchCC.uam``
    - ``Level.xml``
    - Used to control volume and current tone
  * - ``OnOff_to_SoundSwitchCC.uam``
    - ``OnOff.xml``
    - Used to play/stop tone

Bindings
--------

.. list-table:: 
  :header-rows: 1

  * - Z-Wave Attribute Store
    - Cluster attribute
    - Comments
  * - 0
    - Level MinLevel
    - Z-Wave -> Cluster (Read only)
  * - 100
    - Level MaxLevel
    - Z-Wave -> Cluster (Read only)
  * - 1
    - Level MinFrequency
    - Z-Wave -> Cluster (Read only)
  * - ``TONES_NUMBER``
    - Level MaxFrequency
    - Z-Wave -> Cluster (Read only)
  * - ``CONFIGURED_DEFAULT_VOLUME``
    - Level CurrentLevel
    - Z-Wave <-> Cluster (Read/Write)
  * - ``CONFIGURED_DEFAULT_TONE_IDENTIFIER``
    - Level CurrentFrequency
    - Z-Wave <-> Cluster (Read/Write)
  * - ``TONE_PLAY``
    - OnOff On/Off
    - Z-Wave <-> Cluster (Read/Write). ``TONE_PLAY`` is converted to 0/1 and converted back to 0/255 if written from cluster
  * - ``TONE_PLAY``
    - OnOff OnTime
    - Z-Wave <- Cluster (Write)

``TONE_PLAY`` is mapped to 2 clusters attributes :

.. code-block:: cpp

  // Workaround to allow to send a specific tone ID though this attribute.
  //
  // The OnOff UCL cluster only support boolean states and we need a uint8_t to be able to 
  // send specific toneID though the Play Set command.
  //  
  // There is 3 way to do this :
  //  - Add an attribute to an existing UCL cluster
  //  - Add a whole new cluster for sound switch
  //  - Map this functionality to an unused property. 
  //
  // The latest has been choosen to avoid changes on clusters. This approach might change
  // in future release, don't take this as a reference for now.
  //
  // With this workaround we can :
  //  - Play the default tone (255) with the ZCL OnOff attribute (1)
  //  - Stop the tone (0) with the ZCL OnOff attribute (0)
  //  - Play specific tone (1-254) with the ZCL OnTime attribute (1-254) that will be "copied" into our ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_PLAY
  //    - It can be controlled by the command WriteAttribute of the OnOff cluster though MQTT : ucl/by-unid/+/ep0/OnOff/Commands/WriteAttributes {"OnTime":2}


Command actions
---------------

.. list-table:: 
  :widths: 20 20 60
  :header-rows: 1

  * - Action
    - MQTT Topic
    - Comments
  * - Change default tone to 12
    - ``ucl/by-unid/+/+/Level/Commands/MoveToClosestFrequency`` ``{"Frequency": 12}``
    - On Command will play this tone.
  * - Change default volume to 12
    - ``ucl/by-unid/+/+/Level/Commands/MoveToClosestFrequency`` ``{"Level": 12, "TransitionTime":0,"OptionsMask": {"ExecuteIfOff":true,"CoupleColorTempToLevel":false}, "OptionsOverride":{"ExecuteIfOff":true,"CoupleColorTempToLevel":false} }``
    - Each tone will be played with this volume. ``TransitionTime`` is not used but mandatory. `ExecuteIfOff` must be set to true.
  * - Play default tone
    - ``ucl/by-unid/+/+/OnOff/Commands/On`` ``{}``
    - Will set ``TONE_PLAY`` to 255 and send the Tone Play Set.
  * - Stop tone
    - ``ucl/by-unid/+/+/OnOff/Commands/Off`` ``{}``
    - Will set ``TONE_PLAY`` to 1 and send the Tone Play Set.
  * - Play specific tone (tone 12)
    - ``ucl/by-unid/+/+/OnOff/Commands/WriteAttributes`` ``{"OnTime": 12}``
    - Will set ``TONE_PLAY`` to 12 and send the Tone Play Set.

