export type AppState = {
    Status: Status,
    PreRequisitesCheck: PreRequisitesCheck,
    ProtocolList: ProtocolList,
    Frameworks: Frameworks,
    Applications: Applications,
    Emulator: Emulator,
    MultiProtocols: boolean,
    ServiceList: any[],
    DeviceList: string[],
    ErrorMessage: string,
    StatusList: any[],
    Conflicts: any,
    isDeviceTypeActive: boolean
}

export enum BuildStatus {
    Init,
    Waiting,
    Pass,
}

export type PreRequisitesCheck = {
    Docker: BuildStatus,
    DockerCompose: BuildStatus,
    UnifyLibrary: BuildStatus,
}

export enum Status {
    PreRequisitesCheck,
    Protocol,
    Framework,
    Configuration,
    Review,
    Running,
    Error
}

export type ProtocolList = {
    "Z-Wave": ZWaveProtocolItem,
    "Zigbee-NCP": ProtocolItem,
    "AoX-Bluetooth": ProtocolItem,
    "MultiProtocol": ProtocolItem
}

export type Frameworks = {
    NAL: boolean,
    "Image-Provider": boolean,
    GMS: boolean,
    UPVL: boolean
}

export type Applications = {
    UMB: boolean
}

export type Emulator = {
    "EED" : EmulatorItem
}

export type EmulatorItem = {
    Enabled: boolean
    ClusterList: string,
    DeviceType: string | DeviceTypeList
}

export type ProtocolItem = {
    Enabled: boolean,
    Device: string,
    IsManual: boolean
}

export type ZWaveProtocolItem = ProtocolItem & {
    RF: Frequency;
}

export const ClusterList = [
  {value: "Basic", label: "Basic"},
  {value: "PowerConfiguration", label: "Power Configuration"},
  {value: "DeviceTemperatureConfiguration", label: "Device Temperature Configuration"},
  {value: "Identify", label: "Identify"},
  {value: "Groups", label: "Groups"},
  {value: "Scenes", label: "Scenes"},
  {value: "OnOff", label: "OnOff"},
  {value: "Level", label: "Level"},
  {value: "Alarms", label: "Alarms"},
  {value: "Time", label: "Time"},
  {value: "PollControl", label: "Poll Control"},
  {value: "ShadeConfiguration", label: "Shade Configuration"},
  {value: "DoorLock", label: "Door Lock"},
  {value: "WindowCovering", label: "Window Covering"},
  {value: "BarrierControl", label: "Barrier Control"},
  {value: "PumpConfigurationAndControl", label: "Pump Configuration And Control"},
  {value: "Thermostat", label: "Thermostat"},
  {value: "FanControl", label: "Fan Control"},
  {value: "DehumidificationControl", label: "Dehumidification Control"},
  {value: "ThermostatUserInterfaceConfiguration", label: "Thermostat User Interface Configuration"},
  {value: "ColorControl", label: "Color Control"},
  {value: "BallastConfiguration", label: "Ballast Configuration"},
  {value: "IlluminanceMeasurement", label: "Illuminance Measurement"},
  {value: "IlluminanceLevelSensing", label: "Illuminance LevelSensing"},
  {value: "TemperatureMeasurement", label: "Temperature Measurement"},
  {value: "PressureMeasurement", label: "Pressure Measurement"},
  {value: "FlowMeasurement", label: "Flow Measurement"},
  {value: "RelativityHumidity", label: "Relativity Humidity"},
  {value: "OccupancySensing", label: "Occupancy Sensing"},
  {value: "SoilMoisture", label: "Soil Moisture"},
  {value: "PhMeasurement", label: "Ph Measurement"},
  {value: "ElectricalConductivityMeasurement", label: "Electrical Conductivity Measurement"},
  {value: "WindSpeedMeasurement", label: "Wind Speed Measurement"},
  {value: "CarbonMonoxide", label: "Carbon Monoxide"},
  {value: "CarbonDioxide", label: "Carbon Dioxide"},
  {value: "PM25", label: "PM25"},
  {value: "IASZone", label: "IAS Zone"},
  {value: "IASWD", label: "IASWD"},
  {value: "Metering", label: "Metering"},
  {value: "ElectricalMeasurement", label: "Electrical Measurement"},
  {value: "Diagnostics", label: "Diagnostics"},
  {value: "Binding", label: "Binding"},
  {value: "NameAndLocation", label: "Name And Location"},
  {value: "ConfigurationParameters", label: "Configuration Parameters"},
  {value: "AoXLocator", label: "AoX Locator"},
  {value: "AoXPositionEstimation", label: "AoXPosition Estimation"},
  {value: "Descriptor", label: "Descriptor"}
];

export enum DeviceTypeList {
    DimmableLight = "dimmablelight",
    OnOffLight = "onofflight",
    DoorLock = "doorlock",
    OnOffLightSwitch = "onofflightswitch",
    LightSensor = "lightsensor",
    OccupancySensor = "occupancysensor",
    OnOffPlugInUnit = "onoffpluginunit",
    DimmablePlugInUnit = "dimmablepluginunit",
    ExtendedColorLight = "extendedcolorlight",
    WindowCovering = "windowcovering",
    Thermostat = "thermostat",
    FlowSensor = "flowsensor",
}

export enum Frequency {
    EU = "EU",
    US = "US",
    AU_NZ = "ZU_NZ",
    HK = "HK",
    MY = "MY",
    IN = "IN",
    JP = "JP",
    RU = "RU",
    IL = "IL",
    KR = "KR",
    CN = "CH",
    US_LR1 = "US_LR1",
    US_LR2 = "US_LR2",
    US_LR3 = "US_LR3",
    EU_LR1 = "EU_LR1",
    EU_LR2 = "EU_LR2",
    EU_LR3 = "EU_LR3"
}