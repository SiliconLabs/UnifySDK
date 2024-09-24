function asUnderlyingType(arg) {
    switch (arg) {
        case "bool":
            return "bool"
        case "uint8":
        case "map8":
        case "enum8":
        case "BatteryAlarmMask":
        case "CCColorOptions":
        case "ColorLoopSetUpdateFlags":
        case "CopySceneMode":
        case "DrlkDaysMask":
        case "ImagePageRequestFieldControl":
        case "ImageBlockRequestFieldControl":
        case "LevelOptions":
        case "OnWithTimedOffOnOffControl":
        case "QueryNextImageRequestFieldControl":
        case "SquawkSquawkConfiguration":
        case "StartWarningSirenConfiguration":
        case "TstatScheduleDOW":
        case "TstatScheduleMode":
            return "uint8_t"
        case "IasZoneStatus":
        case "uint16":
        case "map16":
        case "attribId":
            return "uint16_t"
        case "uint24":
        case "int24":
        case "uint32":
        case "map32":
        case "BatteryAlarmState":
            return "uint32_t"
        case "uint64":
            return "uint64_t"
        case "int8":
            return "int8_t"
        case "int16":
            return "int16_t"
        case "int32":
            return "int32_t"
        case "int64":
            return "int64_t"
        case "octstr":
        case "string":
        case "bitmap8":
        case "key128":
            return "const char*"
        case "single":
            return "float"
        case "double":
            return "double"
        case "EUI64":
            return "EUI64"
        case "UTC":
            return "UTC"
        case "clusterId":
            return "clusterId"
        default:
            return arg.replace("-", "")
    }
}

/**
 * ZCL type map used to retrieving the underlying ZCL types for:
 * - enum types
 * - atomic types
 * - bitmap types
 */
zcl_type_map = (function() {
    /* FIXME: Type-workarounds where ZAP fails to properly create our required types.  */
    const _map = {
        CCMinMiredsField: 'uint16',
        CCMaxMiredsField: 'uint16',
        DrlkPINUserID: 'uint16',
        DrlkRFIDUserID: 'uint16',
        DrlkTotalUserID: 'uint16',
        DrlkWeekDayScheduleID: 'uint8',
        DrlkYearDayScheduleID: 'uint8',
        DrlkHolidayScheduleID: 'uint8',

        CopySceneMode: 'uint8',
        OnWithTimedOffOnOffControl: 'uint8',
        IasZoneType: 'uint16',

        DrlkUserType: 'enum8',
        DrlkOperMode: 'enum8',
        GetLogRecordResponseEventType: 'enum8',
        CCDirection: 'enum8',
        CCMoveMode: 'enum8',
        CCStepMode: 'enum8',
        ColorLoopSetAction: 'enum8',
        IaswdLevel: 'enum8',
        MoveStepMode: 'enum8',
        SetpointRaiseOrLowerMode: 'enum8',
        ZoneEnrollResponseEnrollResponseCode: 'enum8',
        zclStatus: 'enum8',
        CredentialTypeEnum: 'enum8',
        DataOperationTypeEnum: 'enum8',
        CredentialRuleEnum: 'enum8',
        ZWaveFanModeEnum: 'enum8',
        ZWaveFanStateEnum: 'enum8',
        CredRule: 'enum8',
        UserTypeEnum: 'enum8',
        CredType: 'enum8',
        UserNameEncodingType: 'enum8',
        // zclType: 'uint8',

        BarrierControlSafetyStatus: 'map16',
        BatteryAlarmState: 'map32',
        ColorControlColorCapabilities: 'map16',
        DoorLockAlarmMask: 'map16',
        DoorLockDefaultConfigurationRegister: 'map16',
        DoorLockKeypadOperationEventMask: 'map16',
        DoorLockKeypadProgrammingEventMask: 'map16',
        DoorLockManualOperationEventMask: 'map16',
        DoorLockRFIDOperationEventMask: 'map16',
        DoorLockRFIDProgrammingEventMask: 'map16',
        DoorLockRFOperationEventMask: 'map16',
        DoorLockRFProgrammingEventMask: 'map16',
        DoorLockSupportedOperatingModes: 'map16',
        ElectricalMeasurementACAlarmsMask: 'map16',
        ElectricalMeasurementMeasurementType: 'map32',
        IasZoneStatus: 'map16',
        PumpConfigurationAndControlAlarmMask: 'map16',
        PumpConfigurationAndControlPumpStatus: 'map16',
        ThermostatACErrorCode: 'map32',
        ThermostatThermostatRunningState: 'map16',

        map16: 'map16',
        map24: 'map32',
        map32: 'map32',
        map40: 'map32',
        map48: 'map32',
        map56: 'map32',
        map64: 'map32',
        map8: 'map8',

        AoXLocatorAntennaMode: 'uint8',
        AoXLocatorAoXMode: 'uint8',
        AoXLocatorCTEMode: 'uint8',
        AoXLocatorReportingMode: 'uint8',
        ArmArmMode: 'uint8',
        ArmResponseArmNotification: 'uint8',
        BarrierControlMovingState: 'uint8',
        'BasicGenericDevice-Class': 'uint8',
        'BasicGenericDevice-Type': 'uint8',
        BasicPhysicalEnvironment: 'uint8',
        BasicPowerSource: 'uint8',
        BatterySize: 'uint8',
        BypassResponseZoneIDBypassResult: 'uint8',
        CCColorLoopDirection: 'uint8',
        ColorControlColorLoopActive: 'uint8',
        ColorControlColorMode: 'uint8',
        ColorControlDriftCompensation: 'uint8',
        ColorControlEnhancedColorMode: 'uint8',
        CommissioningNetworkKeyType: 'uint8',
        CommissioningProtocolVersion: 'uint8',
        CommissioningStackProfile: 'uint8',
        CommissioningStartupControl: 'uint8',
        DehumidificationControlDehumidificationLockout: 'uint8',
        DehumidificationControlRelativeHumidityDisplay: 'uint8',
        DehumidificationControlRelativeHumidityMode: 'uint8',
        DoorLockDoorState: 'uint8',
        DoorLockLEDSettings: 'uint8',
        DoorLockLockState: 'uint8',
        DoorLockLockType: 'uint8',
        DoorLockSecurityLevel: 'uint8',
        DoorLockSoundVolume: 'uint8',
        DrlkOperEventSource: 'uint8',
        DrlkPassFailStatus: 'uint8',
        DrlkSetCodeStatus: 'uint8',
        DrlkSettableUserStatus: 'uint8',
        DrlkUserStatus: 'uint8',
        FanControlFanMode: 'uint8',
        FanControlFanModeSequence: 'uint8',
        GetMeasurementProfileResponseStatus: 'uint8',
        HVACSystemTypeConfigurationCoolingSystemStage: 'uint8',
        HVACSystemTypeConfigurationHeatingFuelSource: 'uint8',
        HVACSystemTypeConfigurationHeatingSystemStage: 'uint8',
        HVACSystemTypeConfigurationHeatingSystemType: 'uint8',
        IASZoneZoneState: 'uint8',
        IasacPanelStatus: 'uint8',
        IasaceAlarmStatus: 'uint8',
        IasaceAudibleNotification: 'uint8',
        IlluminanceLevelSensingLevelStatus: 'uint8',
        IlluminanceLevelSensingLightSensorType: 'uint8',
        IlluminanceMeasurementLightSensorType: 'uint8',
        ImageNotifyPayloadType: 'uint8',
        LoggingLevelEnum: 'uint8',
        MeteringEnergyCarrierUnitOfMeasure: 'uint8',
        MeteringMeteringDeviceType: 'uint8',
        MeteringSupplyStatus: 'uint8',
        MeteringTemperatureUnitOfMeasure: 'uint8',
        MeteringUnitofMeasure: 'uint8',
        NM_State: 'uint8',
        NodeStateNetworkStatus: 'uint8',
        NodeStateSecurity: 'uint8',
        OTADeviceSpecificImageType: 'uint8',
        OTAUpgradeImageUpgradeStatus: 'uint8',
        OTAUpgradeUpgradeActivationPolicy: 'uint8',
        OTAUpgradeUpgradeTimeoutPolicy: 'uint8',
        OccupancySensingOccupancySensorType: 'uint8',
        OffWithEffectEffectIdentifier: 'uint8',
        OnOffStartUpOnOff: 'uint8',
        OperatingEventNotificationOperationEventCode: 'uint8',
        OptionsStartupMode: 'uint8',
        ProfileIntervalPeriod: 'uint8',
        ProgrammingEventNotificationProgramEventCode: 'uint8',
        ProgrammingEventNotificationProgramEventSource: 'uint8',
        PumpControlMode: 'uint8',
        PumpOperationMode: 'uint8',
        SHDCFGDirection: 'uint8',
        ShadeConfigurationMode: 'uint8',
        SirenConfigurationStrobe: 'uint8',
        SirenConfigurationWarningMode: 'uint8',
        SquawkConfigurationSquawkMode: 'uint8',
        ThermostatACCapacityFormat: 'uint8',
        ThermostatACCompressorType: 'uint8',
        ThermostatACLouverPosition: 'uint8',
        ThermostatACRefrigerantType: 'uint8',
        ThermostatACType: 'uint8',
        ThermostatControlSequenceOfOperation: 'uint8',
        ThermostatProgrammingOperationModeProgrammingMode: 'uint8',
        ThermostatSetpointChangeSource: 'uint8',
        ThermostatStartOfWeek: 'uint8',
        ThermostatSystemMode: 'uint8',
        ThermostatTemperatureSetpointHold: 'uint8',
        ThermostatThermostatRunningMode: 'uint8',
        ThermostatUserInterfaceConfigurationKeypadLockout: 'uint8',
        ThermostatUserInterfaceConfigurationScheduleProgrammingVisibility: 'uint8',
        ThermostatUserInterfaceConfigurationTemperatureDisplayMode: 'uint8',
        TriggerEffectEffectIdentifier: 'uint8',
        TriggerEffectEffectVariant: 'uint8',
        TxReportTransmissionSpeed: 'uint8',
        WindowCoveringWindowCoveringType: 'uint8',
        attributeReportingStatus: 'uint8',

        BallastConfigurationBallastStatus: 'map8',
        BallastConfigurationLampAlarmMode: 'map8',
        BarrierControlCapabilities: 'map8',
        BasicAlarmMask: 'map8',
        BasicDisableLocalConfig: 'map8',
        BatteryAlarmMask: 'map8',
        CCColorOptions: 'map8',
        ColorLoopSetUpdateFlags: 'map8',
        DeviceTemperatureConfigurationDeviceTempAlarmMask: 'map8',
        DrlkDaysMask: 'map8',
        ElectricalMeasurementDCOverloadAlarmsMask: 'map8',
        GroupsNameSupport: 'map8',
        ImageBlockRequestFieldControl: 'map8',
        ImagePageRequestFieldControl: 'map8',
        LevelOptions: 'map8',
        MeteringDemandFormatting: 'map8',
        MeteringEnergyCarrierDemandFormatting: 'map8',
        MeteringEnergyCarrierSummationFormatting: 'map8',
        MeteringHistoricalConsumptionFormatting: 'map8',
        MeteringSummationFormatting: 'map8',
        MeteringTemperatureFormatting: 'map8',
        OccupancySensingOccupancy: 'map8',
        OccupancySensingOccupancySensorTypeBitmap: 'map8',
        PowerConfigurationMainsAlarmMask: 'map8',
        QueryNextImageRequestFieldControl: 'map8',
        ResetStartupParametersOptions: 'map8',
        RestartDeviceOptions: 'map8',
        ScenesNameSupport: 'map8',
        ShadeConfigurationStatus: 'map8',
        SquawkSquawkConfiguration: 'map8',
        StartWarningSirenConfiguration: 'map8',
        ThermostatAlarmMask: 'map8',
        ThermostatHVACSystemTypeConfiguration: 'map8',
        ThermostatOccupancy: 'map8',
        ThermostatRemoteSensing: 'map8',
        ThermostatThermostatProgrammingOperationMode: 'map8',
        TimeTimeStatus: 'map8',
        TstatScheduleDOW: 'map8',
        TstatScheduleMode: 'map8',
        WindowCoveringConfigOrStatus: 'map8',
        WindowCoveringMode: 'map8',

        // generated struct in zcl_definitions.h
        TransitionType: 'StructTransitionType',
        CredentialStruct: 'CredentialStructType',
    };

    return {
        /**
         * Retrieve the underlying type.
         *
         * @param {*} label ZCL type label
         * @returns Underlying type
         */
        get: function(label) {
            return _map[label] || ('Unknown ' + label)
        },

        /**
         * Assign the underlying type based on label passed in.
         *
         * @param {*} label
         * @param {*} type
         */
        set: function(label, type) {
            if (label && type) {
                _map[label] = type
            }
        }
    };
})()

exports.zcl_type_map_set_item = zcl_type_map.set
exports.zcl_type_map_get_item = zcl_type_map.get
exports.asUnderlyingType = asUnderlyingType