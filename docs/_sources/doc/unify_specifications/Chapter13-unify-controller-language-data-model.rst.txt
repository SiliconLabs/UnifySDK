.. meta::
  :description: UCL Data Model Chapter
  :keywords: UCL, Data Model, Door Lock

.. |ZCL2019Rev7| replace:: `ZCL 2019 Revision 7 <https://www.zigbeealliance.org/wp-content/uploads/2021/10/07-5123-07-ZigbeeClusterLibrary_Revision_7-1.pdf>`__

.. raw:: latex

 \newpage

.. _unify_specifications_chapter_unify_controller_language_data_model:

Unify Controller Language Data Model (Experimental)
===================================================

This chapter describes the data model used in the Unify Controller Language (UCL).

Overview
--------

The Unify Controller Language (UCL) serves as an abstraction layer between the PHY-specific data models
and the models used in various Unify services for controlling IoT devices.

With the exception of the Door Lock cluster, all clusters in the UCL data model are based on the ZigBee
Cluster Library [ZCL2019Rev7] / DotDot (DotDot is ZCL transported over CoAP, whereas UCL will be transported
over MQTT). ZCL was chosen to align with the high-profile `Matter project <https://github.com/project-chip/connectedhomeip>`__. 

The Door Lock cluster in UCL is based on the Matter protocol specification which recently became available, specifically version
1.2 of the specification. New features of Matter such as feature maps have been adopted in the Door Lock cluster and additional
Matter data elements have been added which are not present in ZCL. Existing Door Lock attribute names, command names, command 
parameter names, names for enum values, and names for bitmap values that were based on ZCL have been kept the same in order to 
maintain backward compatability with previous Unify versions.

See the Door Lock Cluster section below for the details of the data elements in the UCL data model for door lock devices.

Door Lock Cluster
-----------------

The Door Lock cluster provides an interface to a generic way to secure a door.

Protocol Controllers and Services may follow the Matter specification for the implementation of features, attributes,
and commands in the Door Lock cluster. 

Although it's not mandatory, it is highly recommended to follow the Matter specification especially for new implementations.

Feature Map
'''''''''''

See the Matter specification for the details of the features supported by the UCL Door Lock cluster, the optional and/or
provisional status of features, and the attributes, commands, command parameters, enum values, and bitmap values that are mandatory or 
optional depending on each feature.

.. note::

  Matter specification access is restricted to member organizations of the `Connectivity Standards Alliance <https://www.csa-iot.org/>`__ (CSA) 
  that is responsible for the Matter standard.

Protocol Controllers MAY advertise the features supported by the Door Lock cluster of a node in its PAN.

Services MAY expose this information to end users or user-facing applications they interface with.

Although it's not mandatory, it is recommended that this attribute is utilized for a better user experience.

.. code-block:: mqtt

  unid/<UNID>/ep<EndpointID>/DoorLock/Attributes/FeatureMap/Desired -  { "value": { "CredentialOverTheAirAccess": false,
                                                                                    "DoorPositionSensor": false,
                                                                                    "FaceCredentials": false,
                                                                                    "FingerCredentials": true,
                                                                                    "HolidaySchedules": false,
                                                                                    "Logging": false,
                                                                                    "Notification": false,
                                                                                    "PINCredential": true,
                                                                                    "RFIDCredential": false,
                                                                                    "Unbolting": false,
                                                                                    "User": false,
                                                                                    "WeekDayAccessSchedules": true,
                                                                                    "YearDayAccessSchedules": false }
                                                                       }

  unid/<UNID>/ep<EndpointID>/DoorLock/Attributes/FeatureMap/Reported - { "value": { "CredentialOverTheAirAccess": false,
                                                                                    "DoorPositionSensor": false,
                                                                                    "FaceCredentials": false,
                                                                                    "FingerCredentials": true,
                                                                                    "HolidaySchedules": false,
                                                                                    "Logging": false,
                                                                                    "Notification": false,
                                                                                    "PINCredential": true,
                                                                                    "RFIDCredential": false,
                                                                                    "Unbolting": false,
                                                                                    "User": false,
                                                                                    "WeekDayAccessSchedules": true,
                                                                                    "YearDayAccessSchedules": false }
                                                                       }

Attributes
''''''''''

See the table below for the details of the attributes in the Door Lock cluster.

Matter attribute names may differ from UCL attribute names for backward compatability, so the mapping to Matter attributes has 
been provided in the table below.

.. warning ::
  Optional attributes may become mandatory when certain features are implemented - see the Matter specification for details.

.. note::

  The UCL attribute name is the one used in MQTT message topics and payloads.

See the Enums and Bitmaps sections below for the valid values of enum and bitmap attributes.

See the Matter specification for the descriptions of attributes and the valid value ranges of other attributes such as those having a numeric data type.

.. list-table:: UCL Door Lock Cluster Attributes
  :name: table_unify_specifications_ucl_door_lock_cluster_attributes
  :widths: 30 10 30 10
  :width: 500px
  :header-rows: 1

  * - UCL Attribute Name
    - Data Type
    - Matter Attribute Name
    - Mandatory?
  * - LockState
    - enum
    - LockState
    - Yes
  * - LockType
    - enum
    - LockType
    - Yes
  * - ActuatorEnabled
    - boolean
    - ActuatorEnabled
    - Yes
  * - DoorState
    - enum
    - DoorState
    - No
  * - DoorOpenEvents
    - integer
    - DoorOpenEvents
    - No
  * - DoorClosedEvents
    - integer
    - DoorClosedEvents
    - No
  * - OpenPeriod
    - integer
    - OpenPeriod
    - No
  * - NumberOfLogRecordsSupported
    - integer
    - NumberOfLogRecordsSupported
    - No
  * - NumberOfTotalUsersSupported
    - integer
    - NumberOfTotalUsersSupported
    - No
  * - NumberOfPINUsersSupported
    - integer
    - NumberOfPINUsersSupported
    - No
  * - NumberOfRFIDUsersSupported
    - integer
    - NumberOfRFIDUsersSupported
    - No
  * - NumberOfWeekDaySchedulesSupportedPerUser
    - integer
    - NumberOfWeekDaySchedulesSupportedPerUser
    - No
  * - NumberOfYearDaySchedulesSupportedPerUser
    - integer
    - NumberOfYearDaySchedulesSupportedPerUser
    - No
  * - NumberOfHolidaySchedulesSupported
    - integer
    - NumberOfHolidaySchedulesSupported
    - No
  * - MaxPINCodeLength
    - integer
    - MaxPINCodeLength
    - No
  * - MinPINCodeLength
    - integer
    - MinPINCodeLength
    - No
  * - MaxRFIDCodeLength
    - integer
    - MaxRFIDCodeLength
    - No
  * - MinRFIDCodeLength
    - integer
    - MinRFIDCodeLength
    - No
  * - CredentialRulesSupport
    - bitmap
    - CredentialRulesSupport
    - No
  * - NumberOfCredentialsSupportedPerUser
    - integer
    - NumberOfCredentialsSupportedPerUser
    - No
  * - EnableLogging
    - boolean
    - EnableLogging
    - No
  * - Language
    - string
    - Language
    - No
  * - LEDSettings
    - integer
    - LEDSettings
    - No
  * - AutoRelockTime
    - integer
    - AutoRelockTime
    - No
  * - SoundVolume
    - integer
    - SoundVolume
    - No
  * - OperatingMode
    - enum
    - OperatingMode
    - Yes
  * - SupportedOperatingModes
    - enum
    - SupportedOperatingModes
    - Yes
  * - DefaultConfigurationRegister
    - bitmap
    - DefaultConfigurationRegister
    - No
  * - EnableLocalProgramming
    - boolean
    - EnableLocalProgramming
    - No
  * - EnableOneTouchLocking
    - boolean
    - EnableOneTouchLocking
    - No
  * - EnableInsideStatusLED
    - boolean
    - EnableInsideStatusLED
    - No
  * - EnablePrivacyModeButton
    - boolean
    - EnablePrivacyModeButton
    - No
  * - LocalProgrammingFeatures
    - bitmap
    - LocalProgrammingFeatures
    - No
  * - WrongCodeEntryLimit
    - integer
    - WrongCodeEntryLimit
    - No
  * - UserCodeTemporaryDisableTime
    - integer
    - UserCodeTemporaryDisableTime
    - No
  * - SendPINOverTheAir
    - boolean
    - SendPINOverTheAir
    - No
  * - RequirePINforRFOperation
    - boolean
    - RequirePINforRemoteOperation
    - No
  * - SecurityLevel
    - enum
    - SecurityLevel (Deprecated)
    - No
  * - ExpiringUserTimeout
    - integer
    - ExpiringUserTimeout
    - No
  * - AlarmMask
    - bitmap
    - AlarmMask
    - No
  * - KeypadOperationEventMask
    - bitmap
    - KeypadOperationEventMask
    - No
  * - RFOperationEventMask
    - bitmap
    - RemoteOperationEventMask
    - No
  * - ManualOperationEventMask
    - bitmap
    - ManualOperationEventMask
    - No
  * - RFIDOperationEventMask
    - bitmap
    - RFIDOperationEventMask
    - No
  * - KeypadProgrammingEventMask
    - bitmap
    - KeypadProgrammingEventMask
    - No
  * - RFProgrammingEventMask
    - bitmap
    - RemoteProgrammingEventMask
    - No
  * - RFIDProgrammingEventMask
    - bitmap
    - RFIDProgrammingEventMask
    - No

Commands
''''''''

See the table below for the details of the commands in the Door Lock cluster.

Matter command names, command parameter names, and data types may differ from UCL names and types for backward compatibility, 
so the mapping is available in the table below.

.. note::

  The UCL command name, command parameter name and data type are the ones used in MQTT message topics and payloads. With the exception of the common 
  Matter Status Response command frame, the Matter command names here are translated to UCL format i.e. with spaces and special characters removed 
  from the name.

Optional commands or parameters may become mandatory when certain features are implemented or based on other conditions such as the values of
specific attributes - see the Matter specification for details.

See the Enums and Bitmaps sections below for the valid values of enum and bitmap command parameters.

See the Matter specification for the descriptions of commands and the valid value ranges of other command parameters such as those having a 
numeric data type.

.. list-table:: UCL Door Lock Cluster Commands and Parameters
  :name: table_unify_specifications_ucl_door_lock_cluster_commands_and_parameters
  :widths: 20 40 20 40 20 10
  :width: 500px
  :header-rows: 1

  * - UCL Command Name
    - UCL Command Parameters
    - Matter Command Name
    - Matter Command Parameters (in the same order as the UCL column)
    - Direction
    - Mandatory?
  * - LockDoor
    - PINOrRFIDCode, string, optional
    - LockDoor
    - PINCode, string
    - Client to Server
    - Yes
  * - LockDoorResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - Yes
  * - UnlockDoor
    - PINOrRFIDCode, string, optional
    - UnlockDoor
    - PINCode, string
    - Client to Server
    - Yes
  * - UnlockDoorResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - Yes
  * - Toggle
    - PINOrRFIDCode, string, optional
    - Disallowed
    - N/A
    - Client to Server
    - No
  * - ToggleResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - No
  * - UnlockWithTimeout
    - | TimeoutInSeconds, integer, mandatory
      | PINOrRFIDCode, string, mandatory
    - UnlockWithTimeout
    - | Timeout, integer, mandatory
      | PINCode, string, mandatory
    - Client to Server
    - No
  * - UnlockwithTimeoutResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - No
  * - GetLogRecord
    - LogIndex, integer, mandatory
    - GetLogRecord
    - Same as UCL
    - Client to Server
    - No
  * - GetLogRecordResponse
    - | LogEntryID, integer, mandatory
      | Timestamp, integer, mandatory
      | EventType, enum, mandatory
      | SourceOperationEvent, enum, mandatory
      | EventIDOrAlarmCode, integer, mandatory
      | UserID, integer, mandatory
      | PIN, string, optional
    - GetLogRecordResponse
    - | LogEntryID, integer, mandatory
      | Timestamp, integer, mandatory
      | EventType, enum, mandatory
      | Source, integer, mandatory
      | EventIDOrAlarmCode, integer, mandatory
      | UserID, integer, mandatory
      | PIN, string, optional
    - Server to Client
    - No
  * - SetPINCode
    - | UserID, integer, mandatory
      | UserStatus, enum, mandatory
      | UserType, enum, mandatory
      | PIN, string, mandatory
    - SetPINCode
    - Same as UCL
    - Client to Server
    - No
  * - SetPINCodeResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - No
  * - GetPINCode
    - UserID, integer, mandatory
    - GetPINCode
    - Same as UCL
    - Client to Server
    - No
  * - GetPINCodeResponse
    - | UserID, integer, mandatory
      | UserStatus, enum, mandatory
      | UserType, enum, mandatory
      | Code, string, mandatory
    - GetPINCodeResponse
    - | UserID, integer, mandatory
      | UserStatus, enum, mandatory
      | UserType, enum, mandatory
      | PINCode, string, mandatory
    - Server to Client
    - No
  * - ClearPINCode
    - UserID, integer, mandatory
    - ClearPINCode
    - PINSlotIndex, integer, mandatory
    - Client to Server
    - No
  * - ClearPINCodeResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - No
  * - ClearAllPINCodes
    - None
    - ClearAllPINCodes
    - None
    - Client to Server
    - No
  * - ClearAllPINCodesResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - No
  * - SetUserStatus
    - | UserID, integer, mandatory
      | UserStatus, enum, mandatory
    - SetUserStatus
    - Same as UCL
    - Client to Server
    - No
  * - SetUserStatusResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - No
  * - GetUserStatus
    - UserID, integer, mandatory
    - GetUserStatus
    - Same as UCL
    - Client to Server
    - No
  * - GetUserStatusResponse
    - | UserID, integer, mandatory
      | UserStatus, enum, mandatory
    - GetUserStatusResponse
    - Same as UCL
    - Server to Client
    - No
  * - SetWeekdaySchedule
    - | ScheduleID, integer, mandatory
      | UserID, integer, mandatory
      | DaysMask, bitmap, mandatory
      | StartHour, integer, mandatory
      | StartMinute, integer, mandatory
      | EndHour, integer, mandatory
      | EndMinute, integer, mandatory
    - SetWeekdaySchedule
    - | WeekDayIndex, integer, mandatory
      | UserIndex, integer, mandatory
      | DaysMask, bitmap, mandatory
      | StartHour, integer, mandatory
      | StartMinute, integer, mandatory
      | EndHour, integer, mandatory
      | EndMinute, integer, mandatory
    - Client to Server
    - No
  * - SetWeekdayScheduleResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - No
  * - GetWeekdaySchedule
    - | ScheduleID, integer, mandatory
      | UserID, integer, mandatory
    - GetWeekdaySchedule
    - | WeekDayIndex, integer, mandatory
      | UserIndex, integer, mandatory
    - Client to Server
    - No
  * - GetWeekdayScheduleResponse
    - | ScheduleID, integer, mandatory
      | UserID, integer, mandatory
      | Status, enum, mandatory
      | DaysMask, bitmap, optional
      | StartHour, integer, optional
      | StartMinute, integer, optional
      | EndHour, integer, optional
      | EndMinute, integer, optional
    - GetWeekdayScheduleResponse
    - | WeekDayIndex, integer, mandatory
      | UserIndex, integer, mandatory
      | Status, enum, mandatory
      | DaysMask, bitmap, optional
      | StartHour, integer, optional
      | StartMinute, integer, optional
      | EndHour, integer, optional
      | EndMinute, integer, optional
    - Server to Client
    - No
  * - ClearWeekdaySchedule
    - | ScheduleID, integer, mandatory
      | UserID, integer, mandatory
    - ClearWeekdaySchedule
    - | WeekDayIndex, integer, mandatory
      | UserIndex, integer, mandatory
    - Client to Server
    - No
  * - ClearWeekdayScheduleResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - No
  * - SetYearDaySchedule
    - | ScheduleID, integer, mandatory
      | UserID, integer, mandatory
      | LocalStartTime, integer, mandatory
      | LocalEndTime, integer, mandatory
    - SetYearDaySchedule
    - | YearDayIndex, integer, mandatory
      | UserIndex, integer, mandatory
      | LocalStartTime, integer, mandatory
      | LocalEndTime, integer, mandatory
    - Client to Server
    - No
  * - SetYearDayScheduleResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - No
  * - GetYearDaySchedule
    - | ScheduleID, integer, mandatory
      | UserID, integer, mandatory
    - GetYearDaySchedule
    - | YearDayIndex, integer, mandatory
      | UserIndex, integer, mandatory
    - Client to Server
    - No
  * - GetYearDayScheduleResponse
    - | ScheduleID, integer, mandatory
      | UserID, integer, mandatory
      | Status, enum, mandatory
      | LocalStartTime, integer, optional
      | LocalEndTime, integer, optional
    - GetYearDayScheduleResponse
    - | YearDayIndex, integer, mandatory
      | UserIndex, integer, mandatory
      | Status, enum, mandatory
      | LocalStartTime, integer, optional
      | LocalEndTime, integer, optional
    - Server to Client
    - No
  * - ClearYearDaySchedule
    - | ScheduleID, integer, mandatory
      | UserID, integer, mandatory
    - ClearYearDaySchedule
    - | YearDayIndex, integer, mandatory
      | UserIndex, integer, mandatory
    - Client to Server
    - No
  * - ClearYearDayScheduleResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - No
  * - SetHolidaySchedule
    - | HolidayScheduleID, integer, mandatory
      | LocalStartTime, integer, mandatory
      | LocalEndTime, integer, mandatory
      | OperatingMode, enum, mandatory
    - SetHolidaySchedule
    - | HolidayIndex, integer, mandatory
      | LocalStartTime, integer, mandatory
      | LocalEndTime, integer, mandatory
      | OperatingModeDuringHoliday, enum, mandatory
    - Client to Server
    - No
  * - SetHolidayScheduleResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - No
  * - GetHolidaySchedule
    - HolidayScheduleID, integer, mandatory
    - GetHolidaySchedule
    - HolidayIndex, integer, mandatory
    - Client to Server
    - No
  * - GetHolidayScheduleResponse
    - | HolidayScheduleID, integer, mandatory
      | Status, enum, mandatory
      | LocalStartTime, integer, optional
      | LocalEndTime, integer, optional
      | OperatingModeDuringHoliday, enum, optional
    - GetHolidayScheduleResponse
    - | HolidayIndex, integer, mandatory
      | Status, enum, mandatory
      | LocalStartTime, integer, optional
      | LocalEndTime, integer, optional
      | OperatingMode, enum, optional
    - Server to Client
    - No
  * - ClearHolidaySchedule
    - HolidayScheduleID, integer, mandatory
    - ClearHolidaySchedule
    - HolidayIndex, integer, mandatory
    - Client to Server
    - No
  * - ClearHolidayScheduleResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - No
  * - SetUserType
    - | UserID, integer, mandatory
      | UserType, enum, mandatory
    - SetUserType
    - Same as UCL
    - Client to Server
    - No
  * - SetUserTypeResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - No
  * - GetUserType
    - UserID, integer, mandatory
    - GetUserType
    - Same as UCL
    - Client to Server
    - No
  * - GetUserTypeResponse
    - | UserID, integer, mandatory
      | UserType, enum, mandatory
    - GetUserTypeResponse
    - Same as UCL
    - Server to Client
    - No
  * - SetRFIDCode
    - | UserID, integer, mandatory
      | UserStatus, enum, mandatory
      | UserType, enum, mandatory
      | RFIDCode, string, mandatory
    - SetRFIDCode
    - Same as UCL
    - Client to Server
    - No
  * - SetRFIDCodeResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - No
  * - GetRFIDCode
    - UserID, integer, mandatory
    - GetRFIDCode
    - Same as UCL
    - Client to Server
    - No
  * - GetRFIDCodeResponse
    - | UserID, integer, mandatory
      | UserStatus, enum, mandatory
      | UserType, enum, mandatory
      | RFIDCode, string, mandatory
    - GetRFIDCodeResponse
    - Same as UCL
    - Server to Client
    - No
  * - ClearRFIDCode
    - UserID, integer, mandatory
    - ClearRFIDCode
    - RFIDSlotIndex, integer, mandatory
    - Client to Server
    - No
  * - ClearRFIDCodeResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - No
  * - ClearAllRFIDCodes
    - None
    - ClearAllRFIDCodes
    - None
    - Client to Server
    - No
  * - ClearAllRFIDCodesResponse
    - Status, enum, mandatory
    - Status Response (common interaction response)
    - Status, integer, mandatory
    - Server to Client
    - No
  * - SetUser
    - | OperationType, enum, mandatory
      | UserIndex, integer, mandatory
      | UserName, string, mandatory
      | UserUniqueID, integer, mandatory
      | UserStatus, enum, mandatory
      | UserType, enum, mandatory
      | CredentialRule, enum, mandatory
    - SetUser
    - Same as UCL
    - Client to Server
    - No
  * - GetUser
    - UserIndex, integer, mandatory
    - GetUser
    - Same as UCL
    - Client to Server
    - No
  * - GetUserResponse
    - | UserIndex, integer, mandatory
      | UserName, string, mandatory
      | UserUniqueID, integer, mandatory
      | UserStatus, enum, mandatory
      | UserType, enum, mandatory
      | CredentialRule, enum, mandatory
      | Credentials, list of structs, mandatory
      | NextUserIndex, integer, mandatory
    - GetUserResponse
    - | UserIndex, integer, mandatory
      | UserName, string, mandatory
      | UserUniqueID, integer, mandatory
      | UserStatus, enum, mandatory
      | UserType, enum, mandatory
      | CredentialRule, enum, mandatory
      | Credentials, list of structs, mandatory
      | CreatorFabricIndex, integer, mandatory
      | LastModifiedFabricIndex, integer, mandatory
      | NextUserIndex, integer, mandatory
    - Server to Client
    - No
  * - ClearUser
    - UserIndex, integer, mandatory
    - ClearUser
    - Same as UCL
    - Client to Server
    - No
  * - SetCredential
    - | OperationType, enum, mandatory
      | Credential, struct, mandatory
      | CredentialData, string, mandatory
      | UserIndex, integer, mandatory
      | UserStatus, enum, mandatory
      | UserType, enum, mandatory
    - SetCredential
    - Same as UCL
    - Client to Server
    - No
  * - SetCredentialResponse
    - | Status, integer, mandatory
      | UserIndex, integer, mandatory
      | NextCredentialIndex, integer, optional
    - SetCredentialResponse
    - Same as UCL
    - Server to Client
    - No
  * - GetCredentialStatus
    - Credential, struct, mandatory
    - GetCredentialStatus
    - Same as UCL
    - Client to Server
    - No
  * - GetCredentialStatusResponse
    - | CredentialExists, boolean, mandatory
      | UserIndex, integer, mandatory
      | NextCredentialIndex, integer, optional
    - GetCredentialStatusResponse
    - | CredentialExists, boolean, mandatory
      | UserIndex, integer, mandatory
      | CreatorFabricIndex, integer, mandatory
      | LastModifiedFabricIndex, integer, mandatory
      | NextCredentialIndex, integer, optional
    - Server to Client
    - No
  * - ClearCredential
    - Credential, struct, mandatory
    - ClearCredential
    - Same as UCL
    - Client to Server
    - No
  * - UnboltDoor
    - PINCode, string, optional
    - UnboltDoor
    - Same as UCL
    - Client to Server
    - No
  * - OperatingEventNotification
    - | OperationEventSource, enum, mandatory
      | OperationEventCode, enum, mandatory
      | UserID, integer, mandatory
      | PIN, string, mandatory
      | LocalTime, integer, mandatory
      | Data, string, optional
    - OperationEventNotification
    - | OperationEventSource, integer, mandatory
      | OperationEventCode, integer, mandatory
      | UserID, integer, mandatory
      | PIN, string, mandatory
      | LocalTime, integer, mandatory
      | Data, string, optional
    - Server to Client
    - No
  * - ProgrammingEventNotification
    - | ProgramEventSource, enum, mandatory
      | ProgramEventCode, enum, mandatory
      | UserID, integer, mandatory
      | PIN, string, mandatory
      | UserType, enum, mandatory
      | UserStatus, enum, mandatory
      | LocalTime, integer, mandatory
      | Data, string, optional
    - ProgrammingEventNotification
    - | ProgramEventSource, integer, mandatory
      | ProgramEventCode, integer, mandatory
      | UserID, integer, mandatory
      | PIN, string, mandatory
      | UserType, enum, mandatory
      | UserStatus, enum, mandatory
      | LocalTime, integer, mandatory
      | Data, string, optional
    - Server to Client
    - No

Enums
'''''

See the table below for the details of the enum attributes or command parameters in the Door Lock cluster.

Matter enum names and/or values may differ from UCL names for backward compatability, so the mapping to Matter names has 
been provided.

.. note::

  The UCL enum name is the one used in MQTT message topics and payloads. Enums are often accompanied by just descriptions for the values 
  in the ZCL and Matter specifications. The Matter names here are translated to UCL format i.e. with spaces and special characters 
  removed from the name.

See the Matter specification for the descriptions of enums and their values.

Protocol Controllers MAY follow the Matter specification for the mandatory or optional enum values supported.

Although it's not mandatory, it is recommended that the Matter specification is followed for a better user experience.

.. list-table:: UCL Door Lock Cluster Enums
  :name: table_unify_specifications_ucl_door_lock_cluster_enums
  :widths: 20 40 20 40
  :width: 500px
  :header-rows: 1

  * - UCL Enum Name
    - UCL Enum Values
    - Matter Enum Name
    - Matter Enum Values (in the same order as the UCL column)
  * - LockState
    - | NotFullyLocked
      | Locked
      | Unlocked
      | Unlatched
      | Undefined
    - LockState
    - | NotFullyLocked
      | Locked
      | Unlocked
      | Unlatched
      | null
  * - LockType
    - | DeadBolt
      | Magnetic
      | Other
      | Mortise
      | Rim
      | LatchBolt
      | CylindricalLock
      | TubularLock
      | InterconnectedLock
      | DeadLatch
      | DoorFurniture
      | Eurocylinder
    - LockType
    - Same as UCL
  * - DoorState
    - | Open
      | Closed
      | ErrorJammed
      | ErrorForcedOpen
      | ErrorUnspecified
      | DoorAjar
      | Undefined
    - DoorState
    - | DoorOpen
      | DoorClosed
      | DoorJammed
      | DoorForcedOpen
      | DoorUnspecifiedError
      | DoorAjar
      | null
  * - CredentialRuleEnum
    - | Single
      | Dual
      | Tri
    - CredentialRuleEnum
    - Same as UCL
  * - OperatingMode
    - | Normal
      | Vacation
      | Privacy
      | NoRFLockOrUnlock
      | Passage
    - OperatingMode
    - | Normal
      | Vacation
      | Privacy
      | NoRemoteLockUnlock
      | Passage
  * - LEDSettings
    - | NeverUseLED
      | UseLEDExceptForAccessAllowed
      | UseLEDForAllEvents
    - LEDSettings
    - Same as UCL
  * - SoundVolume
    - | SilentMode
      | LowVolume
      | HighVolume
      | MediumVolume
    - SoundVolume
    - Same as UCL
  * - SecurityLevel
    - | Network
      | APS
    - SecurityLevel (deprecated)
    - N/A
  * - UserStatus
    - | Available
      | OccupiedEnabled
      | OcupiedDisabled
      | (Value 2 not defined)
      | NotSupported
    - UserStatus
    - | Available
      | OccupiedEnabled
      | OcupiedDisabled
      | (Value 2 not defined)
      | null
  * - UserType
    - | Unrestricted
      | YearDayScheduleUser
      | WeekDayScheduleUser
      | MasterUser
      | NonAccessUser
      | ForcedUser
      | DisposableUser
      | ExpiringUser
      | ScheduleRestrictedUser
      | RemoteOnlyUser
      | NotSupported
    - UserType
    - | Unrestricted
      | YearDayScheduleUser
      | WeekDayScheduleUser
      | ProgrammingUser
      | NonAccessUser
      | ForcedUser
      | DisposableUser
      | ExpiringUser
      | ScheduleRestrictedUser
      | RemoteOnlyUser
      | null
  * - CredentialTypeEnum
    - | ProgrammingPIN
      | PIN
      | RFID
      | Fingerprint
      | FingerVein
      | Face
    - CredentialTypeEnum
    - Same as UCL
  * - DataOperationTypeEnum
    - | Add
      | Clear
      | Modify
    - DataOperationTypeEnum
    - Same as UCL

Bitmaps
'''''''

See the table below for the details of the bitmap attributes or command parameters in the Door Lock cluster.

Matter bitmap names and/or value names may differ from UCL names and types for backward compatibility, so the mapping is available in the table below.

.. note::

  The UCL name is the one used in MQTT message topics and payloads. Bitmaps are often accompanied by just descriptions for the bits 
  in the ZCL and Matter specifications. The Matter names here are translated to UCL format i.e. with spaces and special characters 
  removed from the name.

See the Matter specification for the descriptions of bitmaps and their values.

Protocol Controllers MAY follow the Matter specification for the mandatory or optional bitmap values supported.

Although it's not mandatory, it is recommended that the Matter specification is followed for a better user experience.

.. list-table:: UCL Door Lock Cluster Bitmaps
  :name: table_unify_specifications_ucl_door_lock_cluster_bitmaps
  :widths: 20 40 20 40
  :width: 500px
  :header-rows: 1

  * - UCL Bitmap Name
    - UCL Bitmap Values
    - Matter Bitmap Name
    - Matter Bitmap Values (in the same order as the UCL column)
  * - CredentialRulesSupport
    - | Single
      | Dual
      | Tri
    - CredentialRulesSupport
    - Same as UCL
  * - SupportedOperatingModes
    - | NormalModeSupported
      | VacationModeSupported
      | PrivacyModeSupported
      | NoRFLockOrUnlockModeSupported
      | PassageModeSupported
    - SupportedOperatingModes
    - | Normal
      | Vacation
      | Privacy
      | NoRemoteLockUnlock
      | Passage
  * - DefaultConfigurationRegister
    - | DefaultEnableLocalProgrammingAttributeIsEnabled
      | DefaultKeypadInterfaceIsEnabled
      | DefaultRFInterfaceIsEnabled
      | (Bits 3 and 4 not defined)
      | DefaultSoundVolumeIsEnabled
      | DefaultAutoRelockTimeIsEnabled
      | DefaultLEDSettingsIsEnabled
    - DefaultConfigurationRegister
    - | DefaultEnableLocalProgrammingAttributeIsEnabled
      | DefaultKeypadInterfaceIsEnabled
      | DefaultRemoteInterfaceIsEnabled
      | (Bits 3 and 4 not defined)
      | DefaultSoundVolumeIsEnabled
      | DefaultAutoRelockTimeIsEnabled
      | DefaultLEDSettingsIsEnabled
  * - LocalProgrammingFeatures
    - | AddUsersCredentialsSchedulesIsEnabled
      | ModifyUsersCredentialsSchedulesIsEnabled
      | ClearUsersCredentialsSchedulesIsEnabled
      | AdjustLockSettingsLocallyIsEnabled
    - LocalProgrammingFeatures
    - Same as UCL
  * - AlarmMask
    - | DeadboltJammed
      | LockResetToFactoryDefaults
      | (Bit 2 reserved)
      | RFPowerModuleCycled
      | TamperAlarmWrongCodeEntryLimit
      | TamperAlarmFrontEscutcheonRemovedFromMain
      | ForcedDoorOpenUnderDoorLockedCondition
    - AlarmMask
    - | LockingMechanismJammed
      | LockResetToFactoryDefaults
      | (Bit 2 reserved)
      | RFPowerModuleCycled
      | TamperAlarmWrongCodeEntryLimit
      | TamperAlarmFrontEscutcheonRemovedFromMain
      | ForcedDoorOpenUnderDoorLockedCondition
  * - DaysMask
    - | Sun
      | Mon
      | Tue
      | Wed
      | Thu
      | Fri
      | Sat
    - DaysMask
    - | Sunday
      | Monday
      | Tuesday
      | Wednesday
      | Thursday
      | Friday
      | Saturday
  * - EventType
    - | Operation
      | Programming
      | Alarm
    - EventType
    - Same as UCL
  * - OperationEventSource
    - | Keypad
      | RF
      | Manual
      | RFID
      | Indeterminate
    - OperationEventSource
    - | Keypad
      | Remote
      | Manual
      | RFID
      | Indeterminate
  * - SourceOperationEvent
    - | UnknownOrMS
      | Lock
      | Unlock
      | LockFailureInvalidPINOrID
      | LockFailureInvalidSchedule
      | UnlockFailureInvalidPINOrID
      | UnlockFailureInvalidSchedule
      | OneTouchLock
      | KeyLock
      | KeyUnlock
      | AutoLock
      | ScheduleLock
      | ScheduleUnlock
      | ManualLock
      | ManualUnlock
      | NonAccessUserOperationalEvent
    - OperationEventCode
    - Same as UCL
  * - KeypadOperationEventMask
    - | KeypadOpUnknownOrMS
      | KeypadOpLock
      | KeypadOpUnlock
      | KeypadOpLockErrorInvalidPIN
      | KeypadOpLockErrorInvalidSchedule
      | KeypadOpUnlockInvalidPIN
      | KeypadOpUnlockInvalidSchedule
      | KeypadOpNonAccessUser
    - KeypadOperationEventMask
    - Same as UCL
  * - RFOperationEventMask
    - | RFOpUnknownOrMS
      | RFOpLock
      | RFOpUnlock
      | RFOpLockErrorInvalidCode
      | RFOpLockErrorInvalidSchedule
      | RFOpUnlockInvalidCode
      | RFOpUnlockInvalidSchedule
    - RemoteOperationEventMask
    - | RemoteOpUnknownOrMS
      | RemoteOpLock
      | RemoteOpUnlock
      | RemoteOpLockErrorInvalidCode
      | RemoteOpLockErrorInvalidSchedule
      | RemoteOpUnlockInvalidCode
      | RemoteOpUnlockInvalidSchedule
  * - ManualOperationEventMask
    - | ManualOpUnknownOrMS
      | ManualOpThumbturnLock
      | ManualOpThumbturnUnlock
      | ManualOpOneTouchLock
      | ManualOpKeyLock
      | ManualOpKeyUnlock
      | ManualOpAutoLock
      | ManualOpScheduleLock
      | ManualOpScheduleUnlock
      | ManualOpLock
      | ManualOpUnlock
    - ManualOperationEventMask
    - Same as UCL
  * - RFIDOperationEventMask
    - | RFIDOpUnknownOrMS
      | RFIDOpLock
      | RFIDOpUnlock
      | RFIDOpLockErrorInvalidRFID
      | RFIDOpLockErrorInvalidSchedule
      | RFIDOpUnlockErrorInvalidRFID
      | RFIDOpUnlockErrorInvalidSchedule
    - RFIDOperationEventMask
    - Same as UCL
  * - ProgramEventSource
    - | Keypad
      | RF
      | (Bit 2 reserved)
      | RFID
      | Indeterminate
    - ProgramEventSource
    - | Keypad
      | Remote
      | (Bit 2 reserved)
      | RFID
      | Indeterminate
  * - ProgramEventCode
    - | UnknownOrMS
      | MasterCodeChanged
      | PINCodeAdded
      | PINCodeDeleted
      | PINCodeChanged
      | RFIDCodeAdded
      | RFIDCodeDeleted
    - ProgrammingEventCode
    - | UnknownOrMfgSpecific
      | ProgrammingCodeChanged
      | PINCodeAdded
      | PINCodeCleared
      | PINCodeChanged
      | RFIDCodeAdded
      | RFIDCodeCleared
  * - KeypadProgrammingEventMask
    - | KeypadProgUnknownOrMS
      | KeypadProgMasterCodeChanged
      | KeypadProgPINAdded
      | KeypadProgPINDeleted
      | KeypadProgPINChanged
    - KeypadProgrammingEventMask
    - | KeypadProgUnknownOrMS
      | KeypadProgPINCodeChanged
      | KeypadProgPINAdded
      | KeypadProgPINCleared
      | KeypadProgPINChanged
  * - RFProgrammingEventMask
    - | RFProgUnknownOrMS
      | (Bit 1 not defined)
      | RFProgPINAdded
      | RFProgPINDeleted
      | RFProgPINChanged
      | RFProgRFIDAdded
      | RFProgRFIDDeleted
    - RemoteProgrammingEventMask
    - | RemoteProgUnknownOrMS
      | (Bit 1 not defined)
      | RemoteProgPINAdded
      | RemoteProgPINCleared
      | RemoteProgPINChanged
      | RemoteProgRFIDAdded
      | RemoteProgRFIDCleared
  * - RFIDProgrammingEventMask
    - | RFIDProgUnknownOrMS
      | (Bits 1-4 not defined)
      | RFIDProgRFIDAdded
      | RFIDProgRFIDDeleted
    - RFIDProgrammingEventMask
    - | RFIDProgUnknownOrMS
      | (Bits 1-4 not defined)
      | RFIDProgRFIDAdded
      | RFIDProgRFIDCleared

Events
''''''

The eventing framework in Matter allows the edges of attribute history to be preserved, as opposed to attribute changes 
whose history may be lost due to a node being offline. Events are logged in an event table and reported to nodes
that subscribe to the event, and can be queried by client nodes.

The Matter Events are not used in UCL. So none of the Door locks Matter Events are supported for now.

The UCL Door Lock cluster does not support all of the Matter Door Lock events at this time.

See the Matter specification for details.
