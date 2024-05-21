//This file is generated automatically. Don't try to change something here.
//To add support for new clusters, modify addon-helper.js
//To change the stucture of the ClusterTypeAttrs, modify cluster-type-attributes.zapt
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
//generate ClusterTypes
export let ClusterTypeAttrs: any = {
    Basic: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "ZCLVersion",
              type: "number",            
              max: 255,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 3,
            },
            {                
              id: 1,
              name: "ApplicationVersion",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 2,
              name: "StackVersion",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 3,
              name: "HWVersion",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 4,
              name: "ManufacturerName",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 5,
              name: "ModelIdentifier",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 6,
              name: "DateCode",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 7,
              name: "PowerSource",
              type: "enum",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              enum: [
                { name: "Unknown", value: 0},
																{ name: "SinglePhaseMains", value: 1},
																{ name: "ThreePhaseMains", value: 2},
																{ name: "Battery", value: 3},
																{ name: "DCSource", value: 4},
																{ name: "EmergencyMainsConstantlyPowered", value: 5},
																{ name: "EmergencyMainsAndTransferSwitch", value: 6},
																{ name: "UnknownWithBatteryBackup", value: 128},
																{ name: "SinglePhaseMainsWithBatteryBackup", value: 129},
																{ name: "ThreePhaseMainsWithBatteryBackup", value: 130},
																{ name: "BatteryWithBatteryBackup", value: 131},
																{ name: "DCSourceWithBatteryBackup", value: 132},
																{ name: "EmergencyMainsConstantlyPoweredWithBatteryBackup", value: 133},
																{ name: "EmergencyMainsAndTransferSwitchWithBatteryBackup", value: 134}
              ],
            },
            {                
              id: 8,
              name: "GenericDevice-Class",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 255,
              enum: [
                { name: "Lighting", value: 0}
              ],
            },
            {                
              id: 9,
              name: "GenericDevice-Type",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 255,
              enum: [
                { name: "Incandescent", value: 0},
																{ name: "SpotlightHalogen", value: 1},
																{ name: "HalogenBulb", value: 2},
																{ name: "CFL", value: 3},
																{ name: "LinearFluorescent", value: 4},
																{ name: "LEDBulb", value: 5},
																{ name: "SpotlightLED", value: 6},
																{ name: "LEDStrip", value: 7},
																{ name: "LEDTube", value: 8},
																{ name: "GenericIndoorLuminaireOrLightFixture", value: 9},
																{ name: "GenericOutdoorLuminaireOrLightFixture", value: 10},
																{ name: "PendantLuminaireOrLightFixture", value: 11},
																{ name: "FloorStandingLuminaireOrLightFixture", value: 12},
																{ name: "GenericController", value: 224},
																{ name: "WallSwitch", value: 225},
																{ name: "PortableRemoteController", value: 226},
																{ name: "MotionSensorOrLightSensor", value: 227},
																{ name: "GenericActuator", value: 240},
																{ name: "WallSocket", value: 241},
																{ name: "GatewayOrBridge", value: 242},
																{ name: "PlugInUnit", value: 243},
																{ name: "RetrofitActuator", value: 244},
																{ name: "Unspecified", value: 255}
              ],
            },
            {                
              id: 10,
              name: "ProductCode",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 11,
              name: "ProductURL",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 12,
              name: "ManufacturerVersionDetails",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 13,
              name: "SerialNumber",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 14,
              name: "ProductLabel",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 16,
              name: "LocationDescription",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 17,
              name: "PhysicalEnvironment",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              enum: [
                { name: "UnspecifiedEnvironment", value: 0},
																{ name: "Deprecated", value: 1},
																{ name: "Bar", value: 2},
																{ name: "Courtyard", value: 3},
																{ name: "Bathroom", value: 4},
																{ name: "Bedroom", value: 5},
																{ name: "BilliardRoom", value: 6},
																{ name: "UtilityRoom", value: 7},
																{ name: "Cellar", value: 8},
																{ name: "StorageCloset", value: 9},
																{ name: "Theater", value: 10},
																{ name: "Office", value: 11},
																{ name: "Deck", value: 12},
																{ name: "Den", value: 13},
																{ name: "DiningRoom", value: 14},
																{ name: "ElectricalRoom", value: 15},
																{ name: "Elevator", value: 16},
																{ name: "Entry", value: 17},
																{ name: "FamilyRoom", value: 18},
																{ name: "MainFloor", value: 19},
																{ name: "Upstairs", value: 20},
																{ name: "Downstairs", value: 21},
																{ name: "Basement/LowerLevel", value: 22},
																{ name: "Gallery", value: 23},
																{ name: "GameRoom", value: 24},
																{ name: "Garage", value: 25},
																{ name: "Gym", value: 26},
																{ name: "Hallway", value: 27},
																{ name: "House", value: 28},
																{ name: "Kitchen", value: 29},
																{ name: "LaundryRoom", value: 30},
																{ name: "Library", value: 31},
																{ name: "MasterBedroom", value: 32},
																{ name: "MudRoom", value: 33},
																{ name: "Nursery", value: 34},
																{ name: "Pantry", value: 35},
																{ name: "SecondaryOffice", value: 36},
																{ name: "Outside", value: 37},
																{ name: "Pool", value: 38},
																{ name: "Porch", value: 39},
																{ name: "SewingRoom", value: 40},
																{ name: "SittingRoom", value: 41},
																{ name: "Stairway", value: 42},
																{ name: "Yard", value: 43},
																{ name: "Attic", value: 44},
																{ name: "HotTub", value: 45},
																{ name: "LivingRoom", value: 46},
																{ name: "Sauna", value: 47},
																{ name: "ShopOrWorkshop", value: 48},
																{ name: "GuestBedroom", value: 49},
																{ name: "GuestBath", value: 50},
																{ name: "PowderRoom", value: 51},
																{ name: "BackYard", value: 52},
																{ name: "FrontYard", value: 53},
																{ name: "Patio", value: 54},
																{ name: "Driveway", value: 55},
																{ name: "SunRoom", value: 56},
																{ name: "SecondaryLivingRoom", value: 57},
																{ name: "Spa", value: 58},
																{ name: "Whirlpool", value: 59},
																{ name: "Shed", value: 60},
																{ name: "EquipmentStorage", value: 61},
																{ name: "HobbyOrCraftRoom", value: 62},
																{ name: "Fountain", value: 63},
																{ name: "Pond", value: 64},
																{ name: "ReceptionRoom", value: 65},
																{ name: "BreakfastRoom", value: 66},
																{ name: "Nook", value: 67},
																{ name: "Garden", value: 68},
																{ name: "Balcony", value: 69},
																{ name: "PanicRoom", value: 70},
																{ name: "Terrace", value: 71},
																{ name: "Roof", value: 72},
																{ name: "Toilet", value: 73},
																{ name: "ToiletMain", value: 74},
																{ name: "OutsideToilet", value: 75},
																{ name: "ShowerRoom", value: 76},
																{ name: "Study", value: 77},
																{ name: "FrontGarden", value: 78},
																{ name: "BackGarden", value: 79},
																{ name: "Kettle", value: 80},
																{ name: "Television", value: 81},
																{ name: "Stove", value: 82},
																{ name: "Microwave", value: 83},
																{ name: "Toaster", value: 84},
																{ name: "Vacuum", value: 85},
																{ name: "Appliance", value: 86},
																{ name: "FrontDoor", value: 87},
																{ name: "BackDoor", value: 88},
																{ name: "FridgeDoor", value: 89},
																{ name: "MedicationCabinetDoor", value: 96},
																{ name: "WardrobeDoor", value: 97},
																{ name: "FrontCupboardDoor", value: 98},
																{ name: "OtherDoor", value: 99},
																{ name: "WaitingRoom", value: 100},
																{ name: "TriageRoom", value: 101},
																{ name: "DoctorsOffice", value: 102},
																{ name: "PatientsPrivateRoom", value: 103},
																{ name: "ConsultationRoom", value: 104},
																{ name: "NurseStation", value: 105},
																{ name: "Ward", value: 106},
																{ name: "Corridor", value: 107},
																{ name: "OperatingTheatre", value: 108},
																{ name: "DentalSurgeryRoom", value: 109},
																{ name: "MedicalImagingRoom", value: 110},
																{ name: "DecontaminationRoom", value: 111},
																{ name: "Atrium", value: 112},
																{ name: "Mirror", value: 113},
																{ name: "UnknownEnvironment", value: 255}
              ],
            },
            {                
              id: 18,
              name: "DeviceEnabled",
              type: "boolean",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 1,
            },
            {                
              id: 19,
              name: "AlarmMask",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "GeneralHardwareFault", mask: 1, type: "boolean", offset: 0 },
								{ name: "GeneralSoftwareFault", mask: 2, type: "boolean", offset: 1 }
              ]
            },
            {                
              id: 20,
              name: "DisableLocalConfig",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "DisableResetToFactoryDefaults", mask: 1, type: "boolean", offset: 0 },
								{ name: "DisableDeviceConfiguration", mask: 2, type: "boolean", offset: 1 }
              ]
            },
            {                
              id: 16384,
              name: "SWBuildID",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            }
        ],
        commands: [
            {
              id: 0,
              name: "ResetToFactoryDefaults",
              required: false,
              fields: [
              ]              
            }
        ] 
      }
    },
    PowerConfiguration: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "MainsVoltage",
              type: "number",            
              max: 65535,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 1,
              name: "MainsFrequency",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 16,
              name: "MainsAlarmMask",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: NaN,
              bitmap: [
                { name: "MainsVoltageTooLow", mask: 1, type: "boolean", offset: 0 },
								{ name: "MainsVoltageTooHigh", mask: 2, type: "boolean", offset: 1 },
								{ name: "MainsPowerSupplyLost", mask: 4, type: "boolean", offset: 2 }
              ]
            },
            {                
              id: 17,
              name: "MainsVoltageMinThreshold",
              type: "number",            
              max: 65535,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 18,
              name: "MainsVoltageMaxThreshold",
              type: "number",            
              max: 65535,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 65535,
            },
            {                
              id: 19,
              name: "MainsVoltageDwellTripPoint",
              type: "number",            
              max: 65535,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 32,
              name: "BatteryVoltage",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 33,
              name: "BatteryPercentageRemaining",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 48,
              name: "BatteryManufacturer",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 49,
              name: "BatterySize",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 255,
              enum: [
                { name: "NoBattery", value: 0},
																{ name: "BuiltIn", value: 1},
																{ name: "Other", value: 2},
																{ name: "AA", value: 3},
																{ name: "AAA", value: 4},
																{ name: "C", value: 5},
																{ name: "D", value: 6},
																{ name: "CR2", value: 7},
																{ name: "CR123A", value: 8},
																{ name: "Unknown", value: 255}
              ],
            },
            {                
              id: 50,
              name: "BatteryAHrRating",
              type: "number",            
              max: 65535,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 51,
              name: "BatteryQuantity",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 52,
              name: "BatteryRatedVoltage",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 53,
              name: "BatteryAlarmMask",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "BatteryVoltageTooLow", mask: 1, type: "boolean", offset: 0 },
								{ name: "BatteryAlarm1", mask: 2, type: "boolean", offset: 1 },
								{ name: "BatteryAlarm2", mask: 4, type: "boolean", offset: 2 },
								{ name: "BatteryAlarm3", mask: 8, type: "boolean", offset: 3 }
              ]
            },
            {                
              id: 54,
              name: "BatteryVoltageMinThreshold",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 55,
              name: "BatteryVoltageThreshold1",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 56,
              name: "BatteryVoltageThreshold2",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 57,
              name: "BatteryVoltageThreshold3",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 58,
              name: "BatteryPercentageMinThreshold",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 59,
              name: "BatteryPercentageThreshold1",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 60,
              name: "BatteryPercentageThreshold2",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 61,
              name: "BatteryPercentageThreshold3",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 62,
              name: "BatteryAlarmState",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "Battery1MinThreshold", mask: 1, type: "boolean", offset: 0 },
								{ name: "Battery1Threshold1", mask: 2, type: "boolean", offset: 1 },
								{ name: "Battery1Threshold2", mask: 4, type: "boolean", offset: 2 },
								{ name: "Battery1Threshold3", mask: 8, type: "boolean", offset: 3 },
								{ name: "Battery2MinThreshold", mask: 1024, type: "boolean", offset: 10 },
								{ name: "Battery2Threshold1", mask: 2048, type: "boolean", offset: 11 },
								{ name: "Battery2Threshold2", mask: 4096, type: "boolean", offset: 12 },
								{ name: "Battery2Threshold3", mask: 8192, type: "boolean", offset: 13 },
								{ name: "Battery3MinThreshold", mask: 1048576, type: "boolean", offset: 20 },
								{ name: "Battery3Threshold1", mask: 2097152, type: "boolean", offset: 21 },
								{ name: "Battery3Threshold2", mask: 4194304, type: "boolean", offset: 22 },
								{ name: "Battery3Threshold3", mask: 8388608, type: "boolean", offset: 23 },
								{ name: "MainsPowerSupplyLost", mask: 1073741824, type: "boolean", offset: 30 }
              ]
            },
            {                
              id: 64,
              name: "Battery2Voltage",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 65,
              name: "Battery2PercentageRemaining",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 80,
              name: "Battery2Manufacturer",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 81,
              name: "Battery2Size",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 255,
              enum: [
                { name: "NoBattery", value: 0},
																{ name: "BuiltIn", value: 1},
																{ name: "Other", value: 2},
																{ name: "AA", value: 3},
																{ name: "AAA", value: 4},
																{ name: "C", value: 5},
																{ name: "D", value: 6},
																{ name: "CR2", value: 7},
																{ name: "CR123A", value: 8},
																{ name: "Unknown", value: 255}
              ],
            },
            {                
              id: 82,
              name: "Battery2AHrRating",
              type: "number",            
              max: 65535,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 83,
              name: "Battery2Quantity",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 84,
              name: "Battery2RatedVoltage",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 85,
              name: "Battery2AlarmMask",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              bitmap: [
                
              ]
            },
            {                
              id: 86,
              name: "Battery2VoltageMinThreshold",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 87,
              name: "Battery2VoltageThreshold1",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 88,
              name: "Battery2VoltageThreshold2",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 89,
              name: "Battery2VoltageThreshold3",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 90,
              name: "Battery2PercentageMinThreshold",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 91,
              name: "Battery2PercentageThreshold1",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 92,
              name: "Battery2PercentageThreshold2",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 93,
              name: "Battery2PercentageThreshold3",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 94,
              name: "Battery2AlarmState",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              bitmap: [
                
              ]
            },
            {                
              id: 96,
              name: "Battery3Voltage",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 97,
              name: "Battery3PercentageRemaining",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 112,
              name: "Battery3Manufacturer",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 113,
              name: "Battery3Size",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 255,
              enum: [
                { name: "NoBattery", value: 0},
																{ name: "BuiltIn", value: 1},
																{ name: "Other", value: 2},
																{ name: "AA", value: 3},
																{ name: "AAA", value: 4},
																{ name: "C", value: 5},
																{ name: "D", value: 6},
																{ name: "CR2", value: 7},
																{ name: "CR123A", value: 8},
																{ name: "Unknown", value: 255}
              ],
            },
            {                
              id: 114,
              name: "Battery3AHrRating",
              type: "number",            
              max: 65535,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 115,
              name: "Battery3Quantity",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 116,
              name: "Battery3RatedVoltage",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 117,
              name: "Battery3AlarmMask",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              bitmap: [
                
              ]
            },
            {                
              id: 118,
              name: "Battery3VoltageMinThreshold",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 119,
              name: "Battery3VoltageThreshold1",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 120,
              name: "Battery3VoltageThreshold2",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 121,
              name: "Battery3VoltageThreshold3",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 122,
              name: "Battery3PercentageMinThreshold",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 123,
              name: "Battery3PercentageThreshold1",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 124,
              name: "Battery3PercentageThreshold2",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 125,
              name: "Battery3PercentageThreshold3",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 126,
              name: "Battery3AlarmState",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              bitmap: [
                
              ]
            }
        ],
        commands: [
        ] 
      }
    },
    Identify: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "IdentifyTime",
              type: "number",            
              max: 65535,
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            }
        ],
        commands: [
            {
              id: 0,
              name: "Identify",
              required: true,
              fields: [
                  {
                    name: "IdentifyTime",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 0,
              name: "IdentifyQueryResponse",
              required: true,
              fields: [
                  {
                    name: "Timeout",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 1,
              name: "IdentifyQuery",
              required: true,
              fields: [
              ]              
            },
            {
              id: 64,
              name: "TriggerEffect",
              required: false,
              fields: [
                  {
                    name: "EffectIdentifier",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Blink",
                              value: 0
                            },
                            {
                              name: "Breathe",
                              value: 1
                            },
                            {
                              name: "Okay",
                              value: 2
                            },
                            {
                              name: "ChannelChange",
                              value: 11
                            },
                            {
                              name: "FinishEffect",
                              value: 254
                            },
                            {
                              name: "StopEffect",
                              value: 255
                            }
                    ],
                  },
                  {
                    name: "EffectVariant",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Default",
                              value: 0
                            }
                    ],
                  }
              ]              
            }
        ] 
      }
    },
    Scenes: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "SceneCount",
              type: "number",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 1,
              name: "CurrentScene",
              type: "number",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 2,
              name: "CurrentGroup",
              type: "number",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 3,
              name: "SceneValid",
              type: "boolean",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 4,
              name: "NameSupport",
              type: "bitmap",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              bitmap: [
                { name: "SceneNamesSupported", mask: 128, type: "boolean", offset: 7 }
              ]
            },
            {                
              id: 5,
              name: "LastConfiguredBy",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            },
            {                
              id: 3841,
              name: "SceneTable",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
                {
                  name:"SceneID",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"GroupID",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"SceneName",
                  type:"text",
                  isArray: false,
                },
                {
                  name:"TransitionTime",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"TransitionTime100ms",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"SceneTableExtensions",
                  type:"text",
                  isArray: false,
                }
              ],
            }
        ],
        commands: [
            {
              id: 0,
              name: "AddScene",
              required: true,
              fields: [
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneName",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "ExtensionFieldSets",
                    type: "struct",
                    isArray: true,
                    struct:[
                      {
                        name:"ClusterId",
                        type:"number",
                        isArray: false
                      },
                      {
                        name:"ExtensionFieldSet",
                        type:"text",
                        isArray: true
                      }
                    ],
                  },
                  {
                    name: "TransitionTime100ms",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 0,
              name: "AddSceneResponse",
              required: true,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "SUCCESS",
                              value: 0
                            },
                            {
                              name: "FAILURE",
                              value: 1
                            },
                            {
                              name: "NOT_AUTHORIZED",
                              value: 126
                            },
                            {
                              name: "MALFORMED_COMMAND",
                              value: 128
                            },
                            {
                              name: "UNSUP_CLUSTER_COMMAND",
                              value: 129
                            },
                            {
                              name: "UNSUP_GENERAL_COMMAND",
                              value: 130
                            },
                            {
                              name: "UNSUP_MANUF_CLUSTER_COMMAND",
                              value: 131
                            },
                            {
                              name: "UNSUP_MANUF_GENERAL_COMMAND",
                              value: 132
                            },
                            {
                              name: "INVALID_FIELD",
                              value: 133
                            },
                            {
                              name: "UNSUPPORTED_ATTRIBUTE",
                              value: 134
                            },
                            {
                              name: "INVALID_VALUE",
                              value: 135
                            },
                            {
                              name: "READ_ONLY",
                              value: 136
                            },
                            {
                              name: "INSUFFICIENT_SPACE",
                              value: 137
                            },
                            {
                              name: "DUPLICATE_EXISTS",
                              value: 138
                            },
                            {
                              name: "NOT_FOUND",
                              value: 139
                            },
                            {
                              name: "UNREPORTABLE_ATTRIBUTE",
                              value: 140
                            },
                            {
                              name: "INVALID_DATA_TYPE",
                              value: 141
                            },
                            {
                              name: "INVALID_SELECTOR",
                              value: 142
                            },
                            {
                              name: "WRITE_ONLY",
                              value: 143
                            },
                            {
                              name: "INCONSISTENT_STARTUP_STATE",
                              value: 144
                            },
                            {
                              name: "DEFINED_OUT_OF_BAND",
                              value: 145
                            },
                            {
                              name: "INCONSISTENT",
                              value: 146
                            },
                            {
                              name: "ACTION_DENIED",
                              value: 147
                            },
                            {
                              name: "TIMEOUT",
                              value: 148
                            },
                            {
                              name: "ABORT",
                              value: 149
                            },
                            {
                              name: "INVALID_IMAGE",
                              value: 150
                            },
                            {
                              name: "WAIT_FOR_DATA",
                              value: 151
                            },
                            {
                              name: "NO_IMAGE_AVAILABLE",
                              value: 152
                            },
                            {
                              name: "REQUIRE_MORE_IMAGE",
                              value: 153
                            },
                            {
                              name: "NOTIFICATION_PENDING",
                              value: 154
                            },
                            {
                              name: "HARDWARE_FAILURE",
                              value: 192
                            },
                            {
                              name: "SOFTWARE_FAILURE",
                              value: 193
                            },
                            {
                              name: "CALIBRATION_ERROR",
                              value: 194
                            },
                            {
                              name: "UNSUPPORTED_CLUSTER",
                              value: 195
                            },
                            {
                              name: "LIMIT_REACHED",
                              value: 196
                            }
                    ],
                  },
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 1,
              name: "ViewScene",
              required: true,
              fields: [
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 1,
              name: "ViewSceneResponse",
              required: true,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "SUCCESS",
                              value: 0
                            },
                            {
                              name: "FAILURE",
                              value: 1
                            },
                            {
                              name: "NOT_AUTHORIZED",
                              value: 126
                            },
                            {
                              name: "MALFORMED_COMMAND",
                              value: 128
                            },
                            {
                              name: "UNSUP_CLUSTER_COMMAND",
                              value: 129
                            },
                            {
                              name: "UNSUP_GENERAL_COMMAND",
                              value: 130
                            },
                            {
                              name: "UNSUP_MANUF_CLUSTER_COMMAND",
                              value: 131
                            },
                            {
                              name: "UNSUP_MANUF_GENERAL_COMMAND",
                              value: 132
                            },
                            {
                              name: "INVALID_FIELD",
                              value: 133
                            },
                            {
                              name: "UNSUPPORTED_ATTRIBUTE",
                              value: 134
                            },
                            {
                              name: "INVALID_VALUE",
                              value: 135
                            },
                            {
                              name: "READ_ONLY",
                              value: 136
                            },
                            {
                              name: "INSUFFICIENT_SPACE",
                              value: 137
                            },
                            {
                              name: "DUPLICATE_EXISTS",
                              value: 138
                            },
                            {
                              name: "NOT_FOUND",
                              value: 139
                            },
                            {
                              name: "UNREPORTABLE_ATTRIBUTE",
                              value: 140
                            },
                            {
                              name: "INVALID_DATA_TYPE",
                              value: 141
                            },
                            {
                              name: "INVALID_SELECTOR",
                              value: 142
                            },
                            {
                              name: "WRITE_ONLY",
                              value: 143
                            },
                            {
                              name: "INCONSISTENT_STARTUP_STATE",
                              value: 144
                            },
                            {
                              name: "DEFINED_OUT_OF_BAND",
                              value: 145
                            },
                            {
                              name: "INCONSISTENT",
                              value: 146
                            },
                            {
                              name: "ACTION_DENIED",
                              value: 147
                            },
                            {
                              name: "TIMEOUT",
                              value: 148
                            },
                            {
                              name: "ABORT",
                              value: 149
                            },
                            {
                              name: "INVALID_IMAGE",
                              value: 150
                            },
                            {
                              name: "WAIT_FOR_DATA",
                              value: 151
                            },
                            {
                              name: "NO_IMAGE_AVAILABLE",
                              value: 152
                            },
                            {
                              name: "REQUIRE_MORE_IMAGE",
                              value: 153
                            },
                            {
                              name: "NOTIFICATION_PENDING",
                              value: 154
                            },
                            {
                              name: "HARDWARE_FAILURE",
                              value: 192
                            },
                            {
                              name: "SOFTWARE_FAILURE",
                              value: 193
                            },
                            {
                              name: "CALIBRATION_ERROR",
                              value: 194
                            },
                            {
                              name: "UNSUPPORTED_CLUSTER",
                              value: 195
                            },
                            {
                              name: "LIMIT_REACHED",
                              value: 196
                            }
                    ],
                  },
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneName",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "ExtensionFieldSets",
                    type: "struct",
                    isArray: true,
                    struct:[
                      {
                        name:"ClusterId",
                        type:"number",
                        isArray: false
                      },
                      {
                        name:"ExtensionFieldSet",
                        type:"text",
                        isArray: true
                      }
                    ],
                  }
              ]              
            },
            {
              id: 2,
              name: "RemoveScene",
              required: true,
              fields: [
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 2,
              name: "RemoveSceneResponse",
              required: true,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "SUCCESS",
                              value: 0
                            },
                            {
                              name: "FAILURE",
                              value: 1
                            },
                            {
                              name: "NOT_AUTHORIZED",
                              value: 126
                            },
                            {
                              name: "MALFORMED_COMMAND",
                              value: 128
                            },
                            {
                              name: "UNSUP_CLUSTER_COMMAND",
                              value: 129
                            },
                            {
                              name: "UNSUP_GENERAL_COMMAND",
                              value: 130
                            },
                            {
                              name: "UNSUP_MANUF_CLUSTER_COMMAND",
                              value: 131
                            },
                            {
                              name: "UNSUP_MANUF_GENERAL_COMMAND",
                              value: 132
                            },
                            {
                              name: "INVALID_FIELD",
                              value: 133
                            },
                            {
                              name: "UNSUPPORTED_ATTRIBUTE",
                              value: 134
                            },
                            {
                              name: "INVALID_VALUE",
                              value: 135
                            },
                            {
                              name: "READ_ONLY",
                              value: 136
                            },
                            {
                              name: "INSUFFICIENT_SPACE",
                              value: 137
                            },
                            {
                              name: "DUPLICATE_EXISTS",
                              value: 138
                            },
                            {
                              name: "NOT_FOUND",
                              value: 139
                            },
                            {
                              name: "UNREPORTABLE_ATTRIBUTE",
                              value: 140
                            },
                            {
                              name: "INVALID_DATA_TYPE",
                              value: 141
                            },
                            {
                              name: "INVALID_SELECTOR",
                              value: 142
                            },
                            {
                              name: "WRITE_ONLY",
                              value: 143
                            },
                            {
                              name: "INCONSISTENT_STARTUP_STATE",
                              value: 144
                            },
                            {
                              name: "DEFINED_OUT_OF_BAND",
                              value: 145
                            },
                            {
                              name: "INCONSISTENT",
                              value: 146
                            },
                            {
                              name: "ACTION_DENIED",
                              value: 147
                            },
                            {
                              name: "TIMEOUT",
                              value: 148
                            },
                            {
                              name: "ABORT",
                              value: 149
                            },
                            {
                              name: "INVALID_IMAGE",
                              value: 150
                            },
                            {
                              name: "WAIT_FOR_DATA",
                              value: 151
                            },
                            {
                              name: "NO_IMAGE_AVAILABLE",
                              value: 152
                            },
                            {
                              name: "REQUIRE_MORE_IMAGE",
                              value: 153
                            },
                            {
                              name: "NOTIFICATION_PENDING",
                              value: 154
                            },
                            {
                              name: "HARDWARE_FAILURE",
                              value: 192
                            },
                            {
                              name: "SOFTWARE_FAILURE",
                              value: 193
                            },
                            {
                              name: "CALIBRATION_ERROR",
                              value: 194
                            },
                            {
                              name: "UNSUPPORTED_CLUSTER",
                              value: 195
                            },
                            {
                              name: "LIMIT_REACHED",
                              value: 196
                            }
                    ],
                  },
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 3,
              name: "RemoveAllScenes",
              required: true,
              fields: [
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 3,
              name: "RemoveAllScenesResponse",
              required: true,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "SUCCESS",
                              value: 0
                            },
                            {
                              name: "FAILURE",
                              value: 1
                            },
                            {
                              name: "NOT_AUTHORIZED",
                              value: 126
                            },
                            {
                              name: "MALFORMED_COMMAND",
                              value: 128
                            },
                            {
                              name: "UNSUP_CLUSTER_COMMAND",
                              value: 129
                            },
                            {
                              name: "UNSUP_GENERAL_COMMAND",
                              value: 130
                            },
                            {
                              name: "UNSUP_MANUF_CLUSTER_COMMAND",
                              value: 131
                            },
                            {
                              name: "UNSUP_MANUF_GENERAL_COMMAND",
                              value: 132
                            },
                            {
                              name: "INVALID_FIELD",
                              value: 133
                            },
                            {
                              name: "UNSUPPORTED_ATTRIBUTE",
                              value: 134
                            },
                            {
                              name: "INVALID_VALUE",
                              value: 135
                            },
                            {
                              name: "READ_ONLY",
                              value: 136
                            },
                            {
                              name: "INSUFFICIENT_SPACE",
                              value: 137
                            },
                            {
                              name: "DUPLICATE_EXISTS",
                              value: 138
                            },
                            {
                              name: "NOT_FOUND",
                              value: 139
                            },
                            {
                              name: "UNREPORTABLE_ATTRIBUTE",
                              value: 140
                            },
                            {
                              name: "INVALID_DATA_TYPE",
                              value: 141
                            },
                            {
                              name: "INVALID_SELECTOR",
                              value: 142
                            },
                            {
                              name: "WRITE_ONLY",
                              value: 143
                            },
                            {
                              name: "INCONSISTENT_STARTUP_STATE",
                              value: 144
                            },
                            {
                              name: "DEFINED_OUT_OF_BAND",
                              value: 145
                            },
                            {
                              name: "INCONSISTENT",
                              value: 146
                            },
                            {
                              name: "ACTION_DENIED",
                              value: 147
                            },
                            {
                              name: "TIMEOUT",
                              value: 148
                            },
                            {
                              name: "ABORT",
                              value: 149
                            },
                            {
                              name: "INVALID_IMAGE",
                              value: 150
                            },
                            {
                              name: "WAIT_FOR_DATA",
                              value: 151
                            },
                            {
                              name: "NO_IMAGE_AVAILABLE",
                              value: 152
                            },
                            {
                              name: "REQUIRE_MORE_IMAGE",
                              value: 153
                            },
                            {
                              name: "NOTIFICATION_PENDING",
                              value: 154
                            },
                            {
                              name: "HARDWARE_FAILURE",
                              value: 192
                            },
                            {
                              name: "SOFTWARE_FAILURE",
                              value: 193
                            },
                            {
                              name: "CALIBRATION_ERROR",
                              value: 194
                            },
                            {
                              name: "UNSUPPORTED_CLUSTER",
                              value: 195
                            },
                            {
                              name: "LIMIT_REACHED",
                              value: 196
                            }
                    ],
                  },
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 4,
              name: "StoreScene",
              required: true,
              fields: [
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 4,
              name: "StoreSceneResponse",
              required: true,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "SUCCESS",
                              value: 0
                            },
                            {
                              name: "FAILURE",
                              value: 1
                            },
                            {
                              name: "NOT_AUTHORIZED",
                              value: 126
                            },
                            {
                              name: "MALFORMED_COMMAND",
                              value: 128
                            },
                            {
                              name: "UNSUP_CLUSTER_COMMAND",
                              value: 129
                            },
                            {
                              name: "UNSUP_GENERAL_COMMAND",
                              value: 130
                            },
                            {
                              name: "UNSUP_MANUF_CLUSTER_COMMAND",
                              value: 131
                            },
                            {
                              name: "UNSUP_MANUF_GENERAL_COMMAND",
                              value: 132
                            },
                            {
                              name: "INVALID_FIELD",
                              value: 133
                            },
                            {
                              name: "UNSUPPORTED_ATTRIBUTE",
                              value: 134
                            },
                            {
                              name: "INVALID_VALUE",
                              value: 135
                            },
                            {
                              name: "READ_ONLY",
                              value: 136
                            },
                            {
                              name: "INSUFFICIENT_SPACE",
                              value: 137
                            },
                            {
                              name: "DUPLICATE_EXISTS",
                              value: 138
                            },
                            {
                              name: "NOT_FOUND",
                              value: 139
                            },
                            {
                              name: "UNREPORTABLE_ATTRIBUTE",
                              value: 140
                            },
                            {
                              name: "INVALID_DATA_TYPE",
                              value: 141
                            },
                            {
                              name: "INVALID_SELECTOR",
                              value: 142
                            },
                            {
                              name: "WRITE_ONLY",
                              value: 143
                            },
                            {
                              name: "INCONSISTENT_STARTUP_STATE",
                              value: 144
                            },
                            {
                              name: "DEFINED_OUT_OF_BAND",
                              value: 145
                            },
                            {
                              name: "INCONSISTENT",
                              value: 146
                            },
                            {
                              name: "ACTION_DENIED",
                              value: 147
                            },
                            {
                              name: "TIMEOUT",
                              value: 148
                            },
                            {
                              name: "ABORT",
                              value: 149
                            },
                            {
                              name: "INVALID_IMAGE",
                              value: 150
                            },
                            {
                              name: "WAIT_FOR_DATA",
                              value: 151
                            },
                            {
                              name: "NO_IMAGE_AVAILABLE",
                              value: 152
                            },
                            {
                              name: "REQUIRE_MORE_IMAGE",
                              value: 153
                            },
                            {
                              name: "NOTIFICATION_PENDING",
                              value: 154
                            },
                            {
                              name: "HARDWARE_FAILURE",
                              value: 192
                            },
                            {
                              name: "SOFTWARE_FAILURE",
                              value: 193
                            },
                            {
                              name: "CALIBRATION_ERROR",
                              value: 194
                            },
                            {
                              name: "UNSUPPORTED_CLUSTER",
                              value: 195
                            },
                            {
                              name: "LIMIT_REACHED",
                              value: 196
                            }
                    ],
                  },
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 5,
              name: "RecallScene",
              required: true,
              fields: [
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 6,
              name: "GetSceneMembership",
              required: true,
              fields: [
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 6,
              name: "GetSceneMembershipResponse",
              required: true,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "SUCCESS",
                              value: 0
                            },
                            {
                              name: "FAILURE",
                              value: 1
                            },
                            {
                              name: "NOT_AUTHORIZED",
                              value: 126
                            },
                            {
                              name: "MALFORMED_COMMAND",
                              value: 128
                            },
                            {
                              name: "UNSUP_CLUSTER_COMMAND",
                              value: 129
                            },
                            {
                              name: "UNSUP_GENERAL_COMMAND",
                              value: 130
                            },
                            {
                              name: "UNSUP_MANUF_CLUSTER_COMMAND",
                              value: 131
                            },
                            {
                              name: "UNSUP_MANUF_GENERAL_COMMAND",
                              value: 132
                            },
                            {
                              name: "INVALID_FIELD",
                              value: 133
                            },
                            {
                              name: "UNSUPPORTED_ATTRIBUTE",
                              value: 134
                            },
                            {
                              name: "INVALID_VALUE",
                              value: 135
                            },
                            {
                              name: "READ_ONLY",
                              value: 136
                            },
                            {
                              name: "INSUFFICIENT_SPACE",
                              value: 137
                            },
                            {
                              name: "DUPLICATE_EXISTS",
                              value: 138
                            },
                            {
                              name: "NOT_FOUND",
                              value: 139
                            },
                            {
                              name: "UNREPORTABLE_ATTRIBUTE",
                              value: 140
                            },
                            {
                              name: "INVALID_DATA_TYPE",
                              value: 141
                            },
                            {
                              name: "INVALID_SELECTOR",
                              value: 142
                            },
                            {
                              name: "WRITE_ONLY",
                              value: 143
                            },
                            {
                              name: "INCONSISTENT_STARTUP_STATE",
                              value: 144
                            },
                            {
                              name: "DEFINED_OUT_OF_BAND",
                              value: 145
                            },
                            {
                              name: "INCONSISTENT",
                              value: 146
                            },
                            {
                              name: "ACTION_DENIED",
                              value: 147
                            },
                            {
                              name: "TIMEOUT",
                              value: 148
                            },
                            {
                              name: "ABORT",
                              value: 149
                            },
                            {
                              name: "INVALID_IMAGE",
                              value: 150
                            },
                            {
                              name: "WAIT_FOR_DATA",
                              value: 151
                            },
                            {
                              name: "NO_IMAGE_AVAILABLE",
                              value: 152
                            },
                            {
                              name: "REQUIRE_MORE_IMAGE",
                              value: 153
                            },
                            {
                              name: "NOTIFICATION_PENDING",
                              value: 154
                            },
                            {
                              name: "HARDWARE_FAILURE",
                              value: 192
                            },
                            {
                              name: "SOFTWARE_FAILURE",
                              value: 193
                            },
                            {
                              name: "CALIBRATION_ERROR",
                              value: 194
                            },
                            {
                              name: "UNSUPPORTED_CLUSTER",
                              value: 195
                            },
                            {
                              name: "LIMIT_REACHED",
                              value: 196
                            }
                    ],
                  },
                  {
                    name: "Capacity",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneList",
                    type: "number",
                    isArray: true,
                  }
              ]              
            },
            {
              id: 64,
              name: "EnhancedAddScene",
              required: false,
              fields: [
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneName",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "ExtensionFieldSets",
                    type: "struct",
                    isArray: true,
                    struct:[
                      {
                        name:"ClusterId",
                        type:"number",
                        isArray: false
                      },
                      {
                        name:"ExtensionFieldSet",
                        type:"text",
                        isArray: true
                      }
                    ],
                  }
              ]              
            },
            {
              id: 64,
              name: "EnhancedAddSceneResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "SUCCESS",
                              value: 0
                            },
                            {
                              name: "FAILURE",
                              value: 1
                            },
                            {
                              name: "NOT_AUTHORIZED",
                              value: 126
                            },
                            {
                              name: "MALFORMED_COMMAND",
                              value: 128
                            },
                            {
                              name: "UNSUP_CLUSTER_COMMAND",
                              value: 129
                            },
                            {
                              name: "UNSUP_GENERAL_COMMAND",
                              value: 130
                            },
                            {
                              name: "UNSUP_MANUF_CLUSTER_COMMAND",
                              value: 131
                            },
                            {
                              name: "UNSUP_MANUF_GENERAL_COMMAND",
                              value: 132
                            },
                            {
                              name: "INVALID_FIELD",
                              value: 133
                            },
                            {
                              name: "UNSUPPORTED_ATTRIBUTE",
                              value: 134
                            },
                            {
                              name: "INVALID_VALUE",
                              value: 135
                            },
                            {
                              name: "READ_ONLY",
                              value: 136
                            },
                            {
                              name: "INSUFFICIENT_SPACE",
                              value: 137
                            },
                            {
                              name: "DUPLICATE_EXISTS",
                              value: 138
                            },
                            {
                              name: "NOT_FOUND",
                              value: 139
                            },
                            {
                              name: "UNREPORTABLE_ATTRIBUTE",
                              value: 140
                            },
                            {
                              name: "INVALID_DATA_TYPE",
                              value: 141
                            },
                            {
                              name: "INVALID_SELECTOR",
                              value: 142
                            },
                            {
                              name: "WRITE_ONLY",
                              value: 143
                            },
                            {
                              name: "INCONSISTENT_STARTUP_STATE",
                              value: 144
                            },
                            {
                              name: "DEFINED_OUT_OF_BAND",
                              value: 145
                            },
                            {
                              name: "INCONSISTENT",
                              value: 146
                            },
                            {
                              name: "ACTION_DENIED",
                              value: 147
                            },
                            {
                              name: "TIMEOUT",
                              value: 148
                            },
                            {
                              name: "ABORT",
                              value: 149
                            },
                            {
                              name: "INVALID_IMAGE",
                              value: 150
                            },
                            {
                              name: "WAIT_FOR_DATA",
                              value: 151
                            },
                            {
                              name: "NO_IMAGE_AVAILABLE",
                              value: 152
                            },
                            {
                              name: "REQUIRE_MORE_IMAGE",
                              value: 153
                            },
                            {
                              name: "NOTIFICATION_PENDING",
                              value: 154
                            },
                            {
                              name: "HARDWARE_FAILURE",
                              value: 192
                            },
                            {
                              name: "SOFTWARE_FAILURE",
                              value: 193
                            },
                            {
                              name: "CALIBRATION_ERROR",
                              value: 194
                            },
                            {
                              name: "UNSUPPORTED_CLUSTER",
                              value: 195
                            },
                            {
                              name: "LIMIT_REACHED",
                              value: 196
                            }
                    ],
                  },
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 65,
              name: "EnhancedViewScene",
              required: false,
              fields: [
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 65,
              name: "EnhancedViewSceneResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "SUCCESS",
                              value: 0
                            },
                            {
                              name: "FAILURE",
                              value: 1
                            },
                            {
                              name: "NOT_AUTHORIZED",
                              value: 126
                            },
                            {
                              name: "MALFORMED_COMMAND",
                              value: 128
                            },
                            {
                              name: "UNSUP_CLUSTER_COMMAND",
                              value: 129
                            },
                            {
                              name: "UNSUP_GENERAL_COMMAND",
                              value: 130
                            },
                            {
                              name: "UNSUP_MANUF_CLUSTER_COMMAND",
                              value: 131
                            },
                            {
                              name: "UNSUP_MANUF_GENERAL_COMMAND",
                              value: 132
                            },
                            {
                              name: "INVALID_FIELD",
                              value: 133
                            },
                            {
                              name: "UNSUPPORTED_ATTRIBUTE",
                              value: 134
                            },
                            {
                              name: "INVALID_VALUE",
                              value: 135
                            },
                            {
                              name: "READ_ONLY",
                              value: 136
                            },
                            {
                              name: "INSUFFICIENT_SPACE",
                              value: 137
                            },
                            {
                              name: "DUPLICATE_EXISTS",
                              value: 138
                            },
                            {
                              name: "NOT_FOUND",
                              value: 139
                            },
                            {
                              name: "UNREPORTABLE_ATTRIBUTE",
                              value: 140
                            },
                            {
                              name: "INVALID_DATA_TYPE",
                              value: 141
                            },
                            {
                              name: "INVALID_SELECTOR",
                              value: 142
                            },
                            {
                              name: "WRITE_ONLY",
                              value: 143
                            },
                            {
                              name: "INCONSISTENT_STARTUP_STATE",
                              value: 144
                            },
                            {
                              name: "DEFINED_OUT_OF_BAND",
                              value: 145
                            },
                            {
                              name: "INCONSISTENT",
                              value: 146
                            },
                            {
                              name: "ACTION_DENIED",
                              value: 147
                            },
                            {
                              name: "TIMEOUT",
                              value: 148
                            },
                            {
                              name: "ABORT",
                              value: 149
                            },
                            {
                              name: "INVALID_IMAGE",
                              value: 150
                            },
                            {
                              name: "WAIT_FOR_DATA",
                              value: 151
                            },
                            {
                              name: "NO_IMAGE_AVAILABLE",
                              value: 152
                            },
                            {
                              name: "REQUIRE_MORE_IMAGE",
                              value: 153
                            },
                            {
                              name: "NOTIFICATION_PENDING",
                              value: 154
                            },
                            {
                              name: "HARDWARE_FAILURE",
                              value: 192
                            },
                            {
                              name: "SOFTWARE_FAILURE",
                              value: 193
                            },
                            {
                              name: "CALIBRATION_ERROR",
                              value: 194
                            },
                            {
                              name: "UNSUPPORTED_CLUSTER",
                              value: 195
                            },
                            {
                              name: "LIMIT_REACHED",
                              value: 196
                            }
                    ],
                  },
                  {
                    name: "GroupID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneName",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "ExtensionFieldSets",
                    type: "struct",
                    isArray: true,
                    struct:[
                      {
                        name:"ClusterId",
                        type:"number",
                        isArray: false
                      },
                      {
                        name:"ExtensionFieldSet",
                        type:"text",
                        isArray: true
                      }
                    ],
                  }
              ]              
            },
            {
              id: 66,
              name: "CopyScene",
              required: false,
              fields: [
                  {
                    name: "Mode",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "CopyAllScenes",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "GroupIdentifierFrom",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneIdentifierFrom",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "GroupIdentifierTo",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneIdentifierTo",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 66,
              name: "CopySceneResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "SUCCESS",
                              value: 0
                            },
                            {
                              name: "FAILURE",
                              value: 1
                            },
                            {
                              name: "NOT_AUTHORIZED",
                              value: 126
                            },
                            {
                              name: "MALFORMED_COMMAND",
                              value: 128
                            },
                            {
                              name: "UNSUP_CLUSTER_COMMAND",
                              value: 129
                            },
                            {
                              name: "UNSUP_GENERAL_COMMAND",
                              value: 130
                            },
                            {
                              name: "UNSUP_MANUF_CLUSTER_COMMAND",
                              value: 131
                            },
                            {
                              name: "UNSUP_MANUF_GENERAL_COMMAND",
                              value: 132
                            },
                            {
                              name: "INVALID_FIELD",
                              value: 133
                            },
                            {
                              name: "UNSUPPORTED_ATTRIBUTE",
                              value: 134
                            },
                            {
                              name: "INVALID_VALUE",
                              value: 135
                            },
                            {
                              name: "READ_ONLY",
                              value: 136
                            },
                            {
                              name: "INSUFFICIENT_SPACE",
                              value: 137
                            },
                            {
                              name: "DUPLICATE_EXISTS",
                              value: 138
                            },
                            {
                              name: "NOT_FOUND",
                              value: 139
                            },
                            {
                              name: "UNREPORTABLE_ATTRIBUTE",
                              value: 140
                            },
                            {
                              name: "INVALID_DATA_TYPE",
                              value: 141
                            },
                            {
                              name: "INVALID_SELECTOR",
                              value: 142
                            },
                            {
                              name: "WRITE_ONLY",
                              value: 143
                            },
                            {
                              name: "INCONSISTENT_STARTUP_STATE",
                              value: 144
                            },
                            {
                              name: "DEFINED_OUT_OF_BAND",
                              value: 145
                            },
                            {
                              name: "INCONSISTENT",
                              value: 146
                            },
                            {
                              name: "ACTION_DENIED",
                              value: 147
                            },
                            {
                              name: "TIMEOUT",
                              value: 148
                            },
                            {
                              name: "ABORT",
                              value: 149
                            },
                            {
                              name: "INVALID_IMAGE",
                              value: 150
                            },
                            {
                              name: "WAIT_FOR_DATA",
                              value: 151
                            },
                            {
                              name: "NO_IMAGE_AVAILABLE",
                              value: 152
                            },
                            {
                              name: "REQUIRE_MORE_IMAGE",
                              value: 153
                            },
                            {
                              name: "NOTIFICATION_PENDING",
                              value: 154
                            },
                            {
                              name: "HARDWARE_FAILURE",
                              value: 192
                            },
                            {
                              name: "SOFTWARE_FAILURE",
                              value: 193
                            },
                            {
                              name: "CALIBRATION_ERROR",
                              value: 194
                            },
                            {
                              name: "UNSUPPORTED_CLUSTER",
                              value: 195
                            },
                            {
                              name: "LIMIT_REACHED",
                              value: 196
                            }
                    ],
                  },
                  {
                    name: "GroupIdentifierFrom",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SceneIdentifierFrom",
                    type: "number",
                    isArray: false,
                  }
              ]              
            }
        ] 
      }
    },
    OnOff: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "OnOff",
              type: "boolean",            
              max: 1,
              required: true,
              reportRequired: true,
              sceneRequired: true,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 16384,
              name: "GlobalSceneControl",
              type: "boolean",            
              max: 1,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 16385,
              name: "OnTime",
              type: "number",            
              max: 65535,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 16386,
              name: "OffWaitTime",
              type: "number",            
              max: 65535,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 16387,
              name: "StartUpOnOff",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              enum: [
                { name: "SetOnOffTo0", value: 0},
																{ name: "SetOnOffTo1", value: 1},
																{ name: "TogglePreviousOnOff", value: 2},
																{ name: "SetPreviousOnOff", value: 255}
              ],
            }
        ],
        commands: [
            {
              id: 0,
              name: "Off",
              required: true,
              fields: [
              ]              
            },
            {
              id: 1,
              name: "On",
              required: true,
              fields: [
              ]              
            },
            {
              id: 2,
              name: "Toggle",
              required: true,
              fields: [
              ]              
            },
            {
              id: 64,
              name: "OffWithEffect",
              required: false,
              fields: [
                  {
                    name: "EffectIdentifier",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "DelayedAllOff",
                              value: 0
                            },
                            {
                              name: "DyingLight",
                              value: 1
                            }
                    ],
                  },
                  {
                    name: "EffectVariant",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 65,
              name: "OnWithRecallGlobalScene",
              required: false,
              fields: [
              ]              
            },
            {
              id: 66,
              name: "OnWithTimedOff",
              required: false,
              fields: [
                  {
                    name: "OnOffControl",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "AcceptOnlyWhenOn",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OnTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OffWaitTime",
                    type: "number",
                    isArray: false,
                  }
              ]              
            }
        ] 
      }
    },
    Level: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "CurrentLevel",
              type: "number",            
              required: true,
              reportRequired: true,
              sceneRequired: true,
              writable: false,
              isArray: false,
              default: 255,
            },
            {                
              id: 1,
              name: "RemainingTime",
              type: "number",            
              max: 65535,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 2,
              name: "MinLevel",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 3,
              name: "MaxLevel",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 255,
            },
            {                
              id: 4,
              name: "CurrentFrequency",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: true,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 5,
              name: "MinFrequency",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 6,
              name: "MaxFrequency",
              type: "number",            
              max: 65535,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 15,
              name: "Options",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "ExecuteIfOff", mask: 1, type: "boolean", offset: 0 },
								{ name: "CoupleColorTempToLevel", mask: 2, type: "boolean", offset: 1 }
              ]
            },
            {                
              id: 16,
              name: "OnOffTransitionTime",
              type: "number",            
              max: 65535,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 17,
              name: "OnLevel",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 255,
            },
            {                
              id: 18,
              name: "OnTransitionTime",
              type: "number",            
              max: 65534,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 65535,
            },
            {                
              id: 19,
              name: "OffTransitionTime",
              type: "number",            
              max: 65534,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 65535,
            },
            {                
              id: 20,
              name: "DefaultMoveRate",
              type: "number",            
              max: 254,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 16384,
              name: "StartUpCurrentLevel",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            }
        ],
        commands: [
            {
              id: 0,
              name: "MoveToLevel",
              required: true,
              fields: [
                  {
                    name: "Level",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "CoupleColorTempToLevel",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "CoupleColorTempToLevel",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 1,
              name: "Move",
              required: true,
              fields: [
                  {
                    name: "MoveMode",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Up",
                              value: 0
                            },
                            {
                              name: "Down",
                              value: 1
                            }
                    ],
                  },
                  {
                    name: "Rate",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "CoupleColorTempToLevel",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "CoupleColorTempToLevel",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 2,
              name: "Step",
              required: true,
              fields: [
                  {
                    name: "StepMode",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Up",
                              value: 0
                            },
                            {
                              name: "Down",
                              value: 1
                            }
                    ],
                  },
                  {
                    name: "StepSize",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "CoupleColorTempToLevel",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "CoupleColorTempToLevel",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 3,
              name: "Stop",
              required: true,
              fields: [
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "CoupleColorTempToLevel",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "CoupleColorTempToLevel",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 4,
              name: "MoveToLevelWithOnOff",
              required: true,
              fields: [
                  {
                    name: "Level",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "CoupleColorTempToLevel",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "CoupleColorTempToLevel",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 5,
              name: "MoveWithOnOff",
              required: true,
              fields: [
                  {
                    name: "MoveMode",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Up",
                              value: 0
                            },
                            {
                              name: "Down",
                              value: 1
                            }
                    ],
                  },
                  {
                    name: "Rate",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "CoupleColorTempToLevel",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "CoupleColorTempToLevel",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 6,
              name: "StepWithOnOff",
              required: true,
              fields: [
                  {
                    name: "StepMode",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Up",
                              value: 0
                            },
                            {
                              name: "Down",
                              value: 1
                            }
                    ],
                  },
                  {
                    name: "StepSize",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "CoupleColorTempToLevel",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "CoupleColorTempToLevel",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 7,
              name: "StopWithOnOff",
              required: true,
              fields: [
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "CoupleColorTempToLevel",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "CoupleColorTempToLevel",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 8,
              name: "MoveToClosestFrequency",
              required: false,
              fields: [
                  {
                    name: "Frequency",
                    type: "number",
                    isArray: false,
                  }
              ]              
            }
        ] 
      }
    },
    DoorLock: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "LockState",
              type: "enum",            
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              enum: [
                { name: "NotFullyLocked", value: 0},
																{ name: "Locked", value: 1},
																{ name: "Unlocked", value: 2},
																{ name: "Unlatched", value: 3},
																{ name: "Undefined", value: 255}
              ],
            },
            {                
              id: 1,
              name: "LockType",
              type: "enum",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              enum: [
                { name: "DeadBolt", value: 0},
																{ name: "Magnetic", value: 1},
																{ name: "Other", value: 2},
																{ name: "Mortise", value: 3},
																{ name: "Rim", value: 4},
																{ name: "LatchBolt", value: 5},
																{ name: "CylindricalLock", value: 6},
																{ name: "TubularLock", value: 7},
																{ name: "InterconnectedLock", value: 8},
																{ name: "DeadLatch", value: 9},
																{ name: "DoorFurniture", value: 10},
																{ name: "Eurocylinder", value: 11}
              ],
            },
            {                
              id: 2,
              name: "ActuatorEnabled",
              type: "boolean",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 3,
              name: "DoorState",
              type: "enum",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              enum: [
                { name: "Open", value: 0},
																{ name: "Closed", value: 1},
																{ name: "ErrorJammed", value: 2},
																{ name: "ErrorForcedOpen", value: 3},
																{ name: "ErrorUnspecified", value: 4},
																{ name: "DoorAjar", value: 5},
																{ name: "Undefined", value: 255}
              ],
            },
            {                
              id: 4,
              name: "DoorOpenEvents",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 5,
              name: "DoorClosedEvents",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 6,
              name: "OpenPeriod",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 16,
              name: "NumberOfLogRecordsSupported",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 17,
              name: "NumberOfTotalUsersSupported",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 18,
              name: "NumberOfPINUsersSupported",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 19,
              name: "NumberOfRFIDUsersSupported",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 20,
              name: "NumberOfWeekDaySchedulesSupportedPerUser",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 21,
              name: "NumberOfYearDaySchedulesSupportedPerUser",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 22,
              name: "NumberOfHolidaySchedulesSupported",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 23,
              name: "MaxPINCodeLength",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 8,
            },
            {                
              id: 24,
              name: "MinPINCodeLength",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 4,
            },
            {                
              id: 25,
              name: "MaxRFIDCodeLength",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 20,
            },
            {                
              id: 26,
              name: "MinRFIDCodeLength",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 8,
            },
            {                
              id: 27,
              name: "CredentialRulesSupport",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
              bitmap: [
                { name: "Single", mask: 1, type: "boolean", offset: 0 },
								{ name: "Dual", mask: 2, type: "boolean", offset: 1 },
								{ name: "Tri", mask: 4, type: "boolean", offset: 2 }
              ]
            },
            {                
              id: 28,
              name: "NumberOfCredentialsSupportedPerUser",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 32,
              name: "EnableLogging",
              type: "boolean",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 33,
              name: "Language",
              type: "text",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 34,
              name: "LEDSettings",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 35,
              name: "AutoRelockTime",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 36,
              name: "SoundVolume",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 37,
              name: "OperatingMode",
              type: "enum",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              enum: [
                { name: "Normal", value: 0},
																{ name: "Vacation", value: 1},
																{ name: "Privacy", value: 2},
																{ name: "NoRFLockOrUnlock", value: 3},
																{ name: "Passage", value: 4}
              ],
            },
            {                
              id: 38,
              name: "SupportedOperatingModes",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: NaN,
              bitmap: [
                { name: "NormalModeSupported", mask: 1, type: "boolean", offset: 0 },
								{ name: "VacationModeSupported", mask: 2, type: "boolean", offset: 1 },
								{ name: "PrivacyModeSupported", mask: 4, type: "boolean", offset: 2 },
								{ name: "NoRFLockOrUnlockModeSupported", mask: 8, type: "boolean", offset: 3 },
								{ name: "PassageModeSupported", mask: 16, type: "boolean", offset: 4 }
              ]
            },
            {                
              id: 39,
              name: "DefaultConfigurationRegister",
              type: "bitmap",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "DefaultEnableLocalProgrammingAttributeIsEnabled", mask: 1, type: "boolean", offset: 0 },
								{ name: "DefaultKeypadInterfaceIsEnabled", mask: 2, type: "boolean", offset: 1 },
								{ name: "DefaultRFInterfaceIsEnabled", mask: 4, type: "boolean", offset: 2 },
								{ name: "DefaultSoundVolumeIsEnabled", mask: 32, type: "boolean", offset: 5 },
								{ name: "DefaultAutoRelockTimeIsEnabled", mask: 64, type: "boolean", offset: 6 },
								{ name: "DefaultLEDSettingsIsEnabled", mask: 128, type: "boolean", offset: 7 }
              ]
            },
            {                
              id: 40,
              name: "EnableLocalProgramming",
              type: "boolean",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 1,
            },
            {                
              id: 41,
              name: "EnableOneTouchLocking",
              type: "boolean",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 42,
              name: "EnableInsideStatusLED",
              type: "boolean",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 43,
              name: "EnablePrivacyModeButton",
              type: "boolean",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 44,
              name: "LocalProgrammingFeatures",
              type: "bitmap",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "AddUsersCredentialsSchedulesLocally", mask: 1, type: "boolean", offset: 0 },
								{ name: "ModifyUsersCredentialsSchedulesLocally", mask: 2, type: "boolean", offset: 1 },
								{ name: "ClearUsersCredentialsSchedulesLocally", mask: 4, type: "boolean", offset: 2 },
								{ name: "AdjustLockSettingsLocally", mask: 8, type: "boolean", offset: 3 }
              ]
            },
            {                
              id: 48,
              name: "WrongCodeEntryLimit",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 49,
              name: "UserCodeTemporaryDisableTime",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 50,
              name: "SendPINOverTheAir",
              type: "boolean",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 51,
              name: "RequirePINforRFOperation",
              type: "boolean",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 52,
              name: "SecurityLevel",
              type: "enum",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              enum: [
                { name: "Network", value: 0},
																{ name: "APS", value: 1}
              ],
            },
            {                
              id: 53,
              name: "ExpiringUserTimeout",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 64,
              name: "AlarmMask",
              type: "bitmap",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: NaN,
              bitmap: [
                { name: "DeadboltJammed", mask: 1, type: "boolean", offset: 0 },
								{ name: "LockResetToFactoryDefaults", mask: 2, type: "boolean", offset: 1 },
								{ name: "RFPowerModuleCycled", mask: 8, type: "boolean", offset: 3 },
								{ name: "TamperAlarmWrongCodeEntryLimit", mask: 16, type: "boolean", offset: 4 },
								{ name: "TamperAlarmFrontEscutcheonRemovedFromMain", mask: 32, type: "boolean", offset: 5 },
								{ name: "ForcedDoorOpenUnderDoorLockedCondition", mask: 64, type: "boolean", offset: 6 }
              ]
            },
            {                
              id: 65,
              name: "KeypadOperationEventMask",
              type: "bitmap",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: NaN,
              bitmap: [
                { name: "KeypadOpUnknownOrMS", mask: 1, type: "boolean", offset: 0 },
								{ name: "KeypadOpLock", mask: 2, type: "boolean", offset: 1 },
								{ name: "KeypadOpUnlock", mask: 4, type: "boolean", offset: 2 },
								{ name: "KeypadOpLockErrorInvalidPIN", mask: 8, type: "boolean", offset: 3 },
								{ name: "KeypadOpLockErrorInvalidSchedule", mask: 16, type: "boolean", offset: 4 },
								{ name: "KeypadOpUnlockInvalidPIN", mask: 32, type: "boolean", offset: 5 },
								{ name: "KeypadOpUnlockInvalidSchedule", mask: 64, type: "boolean", offset: 6 },
								{ name: "KeypadOpNonAccessUser", mask: 128, type: "boolean", offset: 7 }
              ]
            },
            {                
              id: 66,
              name: "RFOperationEventMask",
              type: "bitmap",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: NaN,
              bitmap: [
                { name: "RFOpUnknownOrMS", mask: 1, type: "boolean", offset: 0 },
								{ name: "RFOpLock", mask: 2, type: "boolean", offset: 1 },
								{ name: "RFOpUnlock", mask: 4, type: "boolean", offset: 2 },
								{ name: "RFOpLockErrorInvalidCode", mask: 8, type: "boolean", offset: 3 },
								{ name: "RFOpLockErrorInvalidSchedule", mask: 16, type: "boolean", offset: 4 },
								{ name: "RFOpUnlockInvalidCode", mask: 32, type: "boolean", offset: 5 },
								{ name: "RFOpUnlockInvalidSchedule", mask: 64, type: "boolean", offset: 6 }
              ]
            },
            {                
              id: 67,
              name: "ManualOperationEventMask",
              type: "bitmap",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: NaN,
              bitmap: [
                { name: "ManualOpUnknownOrMS", mask: 1, type: "boolean", offset: 0 },
								{ name: "ManualOpThumbturnLock", mask: 2, type: "boolean", offset: 1 },
								{ name: "ManualOpThumbturnUnlock", mask: 4, type: "boolean", offset: 2 },
								{ name: "ManualOpOneTouchLock", mask: 8, type: "boolean", offset: 3 },
								{ name: "ManualOpKeyLock", mask: 16, type: "boolean", offset: 4 },
								{ name: "ManualOpKeyUnlock", mask: 32, type: "boolean", offset: 5 },
								{ name: "ManualOpAutoLock", mask: 64, type: "boolean", offset: 6 },
								{ name: "ManualOpScheduleLock", mask: 128, type: "boolean", offset: 7 },
								{ name: "ManualOpScheduleUnlock", mask: 256, type: "boolean", offset: 8 },
								{ name: "ManualOpLock", mask: 512, type: "boolean", offset: 9 },
								{ name: "ManualOpUnlock", mask: 1024, type: "boolean", offset: 10 }
              ]
            },
            {                
              id: 68,
              name: "RFIDOperationEventMask",
              type: "bitmap",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: NaN,
              bitmap: [
                { name: "RFIDOpUnknownOrMS", mask: 1, type: "boolean", offset: 0 },
								{ name: "RFIDOpLock", mask: 2, type: "boolean", offset: 1 },
								{ name: "RFIDOpUnlock", mask: 4, type: "boolean", offset: 2 },
								{ name: "RFIDOpLockErrorInvalidRFID", mask: 8, type: "boolean", offset: 3 },
								{ name: "RFIDOpLockErrorInvalidSchedule", mask: 16, type: "boolean", offset: 4 },
								{ name: "RFIDOpUnlockErrorInvalidRFID", mask: 32, type: "boolean", offset: 5 },
								{ name: "RFIDOpUnlockErrorInvalidSchedule", mask: 64, type: "boolean", offset: 6 }
              ]
            },
            {                
              id: 69,
              name: "KeypadProgrammingEventMask",
              type: "bitmap",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: NaN,
              bitmap: [
                { name: "KeypadProgUnknownOrMS", mask: 1, type: "boolean", offset: 0 },
								{ name: "KeypadProgMasterCodeChanged", mask: 2, type: "boolean", offset: 1 },
								{ name: "KeypadProgPINAdded", mask: 4, type: "boolean", offset: 2 },
								{ name: "KeypadProgPINDeleted", mask: 8, type: "boolean", offset: 3 },
								{ name: "KeypadProgPINChanged", mask: 16, type: "boolean", offset: 4 }
              ]
            },
            {                
              id: 70,
              name: "RFProgrammingEventMask",
              type: "bitmap",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: NaN,
              bitmap: [
                { name: "RFProgUnknownOrMS", mask: 1, type: "boolean", offset: 0 },
								{ name: "RFProgPINAdded", mask: 4, type: "boolean", offset: 2 },
								{ name: "RFProgPINDeleted", mask: 8, type: "boolean", offset: 3 },
								{ name: "RFProgPINChanged", mask: 16, type: "boolean", offset: 4 },
								{ name: "RFProgRFIDAdded", mask: 32, type: "boolean", offset: 5 },
								{ name: "RFProgRFIDDeleted", mask: 64, type: "boolean", offset: 6 }
              ]
            },
            {                
              id: 71,
              name: "RFIDProgrammingEventMask",
              type: "bitmap",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: NaN,
              bitmap: [
                { name: "RFIDProgUnknownOrMS", mask: 1, type: "boolean", offset: 0 },
								{ name: "RFIDProgRFIDAdded", mask: 32, type: "boolean", offset: 5 },
								{ name: "RFIDProgRFIDDeleted", mask: 64, type: "boolean", offset: 6 }
              ]
            },
            {                
              id: 65532,
              name: "FeatureMap",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "PINCredential", mask: 1, type: "boolean", offset: 0 },
								{ name: "RFIDCredential", mask: 2, type: "boolean", offset: 1 },
								{ name: "FingerCredentials", mask: 4, type: "boolean", offset: 2 },
								{ name: "Logging", mask: 8, type: "boolean", offset: 3 },
								{ name: "WeekDayAccessSchedules", mask: 16, type: "boolean", offset: 4 },
								{ name: "DoorPositionSensor", mask: 32, type: "boolean", offset: 5 },
								{ name: "FaceCredentials", mask: 64, type: "boolean", offset: 6 },
								{ name: "CredentialOverTheAirAccess", mask: 128, type: "boolean", offset: 7 },
								{ name: "User", mask: 256, type: "boolean", offset: 8 },
								{ name: "Notification", mask: 512, type: "boolean", offset: 9 },
								{ name: "YearDayAccessSchedules", mask: 1024, type: "boolean", offset: 10 },
								{ name: "HolidaySchedules", mask: 2048, type: "boolean", offset: 11 },
								{ name: "Unbolting", mask: 4096, type: "boolean", offset: 12 }
              ]
            }
        ],
        commands: [
            {
              id: 0,
              name: "LockDoor",
              required: true,
              fields: [
                  {
                    name: "PINOrRFIDCode",
                    type: "text",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 0,
              name: "LockDoorResponse",
              required: true,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "SUCCESS",
                              value: 0
                            },
                            {
                              name: "FAILURE",
                              value: 1
                            },
                            {
                              name: "NOT_AUTHORIZED",
                              value: 126
                            },
                            {
                              name: "MALFORMED_COMMAND",
                              value: 128
                            },
                            {
                              name: "UNSUP_CLUSTER_COMMAND",
                              value: 129
                            },
                            {
                              name: "UNSUP_GENERAL_COMMAND",
                              value: 130
                            },
                            {
                              name: "UNSUP_MANUF_CLUSTER_COMMAND",
                              value: 131
                            },
                            {
                              name: "UNSUP_MANUF_GENERAL_COMMAND",
                              value: 132
                            },
                            {
                              name: "INVALID_FIELD",
                              value: 133
                            },
                            {
                              name: "UNSUPPORTED_ATTRIBUTE",
                              value: 134
                            },
                            {
                              name: "INVALID_VALUE",
                              value: 135
                            },
                            {
                              name: "READ_ONLY",
                              value: 136
                            },
                            {
                              name: "INSUFFICIENT_SPACE",
                              value: 137
                            },
                            {
                              name: "DUPLICATE_EXISTS",
                              value: 138
                            },
                            {
                              name: "NOT_FOUND",
                              value: 139
                            },
                            {
                              name: "UNREPORTABLE_ATTRIBUTE",
                              value: 140
                            },
                            {
                              name: "INVALID_DATA_TYPE",
                              value: 141
                            },
                            {
                              name: "INVALID_SELECTOR",
                              value: 142
                            },
                            {
                              name: "WRITE_ONLY",
                              value: 143
                            },
                            {
                              name: "INCONSISTENT_STARTUP_STATE",
                              value: 144
                            },
                            {
                              name: "DEFINED_OUT_OF_BAND",
                              value: 145
                            },
                            {
                              name: "INCONSISTENT",
                              value: 146
                            },
                            {
                              name: "ACTION_DENIED",
                              value: 147
                            },
                            {
                              name: "TIMEOUT",
                              value: 148
                            },
                            {
                              name: "ABORT",
                              value: 149
                            },
                            {
                              name: "INVALID_IMAGE",
                              value: 150
                            },
                            {
                              name: "WAIT_FOR_DATA",
                              value: 151
                            },
                            {
                              name: "NO_IMAGE_AVAILABLE",
                              value: 152
                            },
                            {
                              name: "REQUIRE_MORE_IMAGE",
                              value: 153
                            },
                            {
                              name: "NOTIFICATION_PENDING",
                              value: 154
                            },
                            {
                              name: "HARDWARE_FAILURE",
                              value: 192
                            },
                            {
                              name: "SOFTWARE_FAILURE",
                              value: 193
                            },
                            {
                              name: "CALIBRATION_ERROR",
                              value: 194
                            },
                            {
                              name: "UNSUPPORTED_CLUSTER",
                              value: 195
                            },
                            {
                              name: "LIMIT_REACHED",
                              value: 196
                            }
                    ],
                  }
              ]              
            },
            {
              id: 1,
              name: "UnlockDoor",
              required: true,
              fields: [
                  {
                    name: "PINOrRFIDCode",
                    type: "text",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 1,
              name: "UnlockDoorResponse",
              required: true,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "SUCCESS",
                              value: 0
                            },
                            {
                              name: "FAILURE",
                              value: 1
                            },
                            {
                              name: "NOT_AUTHORIZED",
                              value: 126
                            },
                            {
                              name: "MALFORMED_COMMAND",
                              value: 128
                            },
                            {
                              name: "UNSUP_CLUSTER_COMMAND",
                              value: 129
                            },
                            {
                              name: "UNSUP_GENERAL_COMMAND",
                              value: 130
                            },
                            {
                              name: "UNSUP_MANUF_CLUSTER_COMMAND",
                              value: 131
                            },
                            {
                              name: "UNSUP_MANUF_GENERAL_COMMAND",
                              value: 132
                            },
                            {
                              name: "INVALID_FIELD",
                              value: 133
                            },
                            {
                              name: "UNSUPPORTED_ATTRIBUTE",
                              value: 134
                            },
                            {
                              name: "INVALID_VALUE",
                              value: 135
                            },
                            {
                              name: "READ_ONLY",
                              value: 136
                            },
                            {
                              name: "INSUFFICIENT_SPACE",
                              value: 137
                            },
                            {
                              name: "DUPLICATE_EXISTS",
                              value: 138
                            },
                            {
                              name: "NOT_FOUND",
                              value: 139
                            },
                            {
                              name: "UNREPORTABLE_ATTRIBUTE",
                              value: 140
                            },
                            {
                              name: "INVALID_DATA_TYPE",
                              value: 141
                            },
                            {
                              name: "INVALID_SELECTOR",
                              value: 142
                            },
                            {
                              name: "WRITE_ONLY",
                              value: 143
                            },
                            {
                              name: "INCONSISTENT_STARTUP_STATE",
                              value: 144
                            },
                            {
                              name: "DEFINED_OUT_OF_BAND",
                              value: 145
                            },
                            {
                              name: "INCONSISTENT",
                              value: 146
                            },
                            {
                              name: "ACTION_DENIED",
                              value: 147
                            },
                            {
                              name: "TIMEOUT",
                              value: 148
                            },
                            {
                              name: "ABORT",
                              value: 149
                            },
                            {
                              name: "INVALID_IMAGE",
                              value: 150
                            },
                            {
                              name: "WAIT_FOR_DATA",
                              value: 151
                            },
                            {
                              name: "NO_IMAGE_AVAILABLE",
                              value: 152
                            },
                            {
                              name: "REQUIRE_MORE_IMAGE",
                              value: 153
                            },
                            {
                              name: "NOTIFICATION_PENDING",
                              value: 154
                            },
                            {
                              name: "HARDWARE_FAILURE",
                              value: 192
                            },
                            {
                              name: "SOFTWARE_FAILURE",
                              value: 193
                            },
                            {
                              name: "CALIBRATION_ERROR",
                              value: 194
                            },
                            {
                              name: "UNSUPPORTED_CLUSTER",
                              value: 195
                            },
                            {
                              name: "LIMIT_REACHED",
                              value: 196
                            }
                    ],
                  }
              ]              
            },
            {
              id: 2,
              name: "Toggle",
              required: false,
              fields: [
                  {
                    name: "PINOrRFIDCode",
                    type: "text",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 2,
              name: "ToggleResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "SUCCESS",
                              value: 0
                            },
                            {
                              name: "FAILURE",
                              value: 1
                            },
                            {
                              name: "NOT_AUTHORIZED",
                              value: 126
                            },
                            {
                              name: "MALFORMED_COMMAND",
                              value: 128
                            },
                            {
                              name: "UNSUP_CLUSTER_COMMAND",
                              value: 129
                            },
                            {
                              name: "UNSUP_GENERAL_COMMAND",
                              value: 130
                            },
                            {
                              name: "UNSUP_MANUF_CLUSTER_COMMAND",
                              value: 131
                            },
                            {
                              name: "UNSUP_MANUF_GENERAL_COMMAND",
                              value: 132
                            },
                            {
                              name: "INVALID_FIELD",
                              value: 133
                            },
                            {
                              name: "UNSUPPORTED_ATTRIBUTE",
                              value: 134
                            },
                            {
                              name: "INVALID_VALUE",
                              value: 135
                            },
                            {
                              name: "READ_ONLY",
                              value: 136
                            },
                            {
                              name: "INSUFFICIENT_SPACE",
                              value: 137
                            },
                            {
                              name: "DUPLICATE_EXISTS",
                              value: 138
                            },
                            {
                              name: "NOT_FOUND",
                              value: 139
                            },
                            {
                              name: "UNREPORTABLE_ATTRIBUTE",
                              value: 140
                            },
                            {
                              name: "INVALID_DATA_TYPE",
                              value: 141
                            },
                            {
                              name: "INVALID_SELECTOR",
                              value: 142
                            },
                            {
                              name: "WRITE_ONLY",
                              value: 143
                            },
                            {
                              name: "INCONSISTENT_STARTUP_STATE",
                              value: 144
                            },
                            {
                              name: "DEFINED_OUT_OF_BAND",
                              value: 145
                            },
                            {
                              name: "INCONSISTENT",
                              value: 146
                            },
                            {
                              name: "ACTION_DENIED",
                              value: 147
                            },
                            {
                              name: "TIMEOUT",
                              value: 148
                            },
                            {
                              name: "ABORT",
                              value: 149
                            },
                            {
                              name: "INVALID_IMAGE",
                              value: 150
                            },
                            {
                              name: "WAIT_FOR_DATA",
                              value: 151
                            },
                            {
                              name: "NO_IMAGE_AVAILABLE",
                              value: 152
                            },
                            {
                              name: "REQUIRE_MORE_IMAGE",
                              value: 153
                            },
                            {
                              name: "NOTIFICATION_PENDING",
                              value: 154
                            },
                            {
                              name: "HARDWARE_FAILURE",
                              value: 192
                            },
                            {
                              name: "SOFTWARE_FAILURE",
                              value: 193
                            },
                            {
                              name: "CALIBRATION_ERROR",
                              value: 194
                            },
                            {
                              name: "UNSUPPORTED_CLUSTER",
                              value: 195
                            },
                            {
                              name: "LIMIT_REACHED",
                              value: 196
                            }
                    ],
                  }
              ]              
            },
            {
              id: 3,
              name: "UnlockWithTimeout",
              required: false,
              fields: [
                  {
                    name: "TimeoutInSeconds",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "PINOrRFIDCode",
                    type: "text",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 3,
              name: "UnlockWithTimeoutResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "SUCCESS",
                              value: 0
                            },
                            {
                              name: "FAILURE",
                              value: 1
                            },
                            {
                              name: "NOT_AUTHORIZED",
                              value: 126
                            },
                            {
                              name: "MALFORMED_COMMAND",
                              value: 128
                            },
                            {
                              name: "UNSUP_CLUSTER_COMMAND",
                              value: 129
                            },
                            {
                              name: "UNSUP_GENERAL_COMMAND",
                              value: 130
                            },
                            {
                              name: "UNSUP_MANUF_CLUSTER_COMMAND",
                              value: 131
                            },
                            {
                              name: "UNSUP_MANUF_GENERAL_COMMAND",
                              value: 132
                            },
                            {
                              name: "INVALID_FIELD",
                              value: 133
                            },
                            {
                              name: "UNSUPPORTED_ATTRIBUTE",
                              value: 134
                            },
                            {
                              name: "INVALID_VALUE",
                              value: 135
                            },
                            {
                              name: "READ_ONLY",
                              value: 136
                            },
                            {
                              name: "INSUFFICIENT_SPACE",
                              value: 137
                            },
                            {
                              name: "DUPLICATE_EXISTS",
                              value: 138
                            },
                            {
                              name: "NOT_FOUND",
                              value: 139
                            },
                            {
                              name: "UNREPORTABLE_ATTRIBUTE",
                              value: 140
                            },
                            {
                              name: "INVALID_DATA_TYPE",
                              value: 141
                            },
                            {
                              name: "INVALID_SELECTOR",
                              value: 142
                            },
                            {
                              name: "WRITE_ONLY",
                              value: 143
                            },
                            {
                              name: "INCONSISTENT_STARTUP_STATE",
                              value: 144
                            },
                            {
                              name: "DEFINED_OUT_OF_BAND",
                              value: 145
                            },
                            {
                              name: "INCONSISTENT",
                              value: 146
                            },
                            {
                              name: "ACTION_DENIED",
                              value: 147
                            },
                            {
                              name: "TIMEOUT",
                              value: 148
                            },
                            {
                              name: "ABORT",
                              value: 149
                            },
                            {
                              name: "INVALID_IMAGE",
                              value: 150
                            },
                            {
                              name: "WAIT_FOR_DATA",
                              value: 151
                            },
                            {
                              name: "NO_IMAGE_AVAILABLE",
                              value: 152
                            },
                            {
                              name: "REQUIRE_MORE_IMAGE",
                              value: 153
                            },
                            {
                              name: "NOTIFICATION_PENDING",
                              value: 154
                            },
                            {
                              name: "HARDWARE_FAILURE",
                              value: 192
                            },
                            {
                              name: "SOFTWARE_FAILURE",
                              value: 193
                            },
                            {
                              name: "CALIBRATION_ERROR",
                              value: 194
                            },
                            {
                              name: "UNSUPPORTED_CLUSTER",
                              value: 195
                            },
                            {
                              name: "LIMIT_REACHED",
                              value: 196
                            }
                    ],
                  }
              ]              
            },
            {
              id: 4,
              name: "GetLogRecord",
              required: false,
              fields: [
                  {
                    name: "LogIndex",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 4,
              name: "GetLogRecordResponse",
              required: false,
              fields: [
                  {
                    name: "LogEntryID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "Timestamp",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "EventType",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Operation",
                              value: 0
                            },
                            {
                              name: "Programming",
                              value: 1
                            },
                            {
                              name: "Alarm",
                              value: 2
                            }
                    ],
                  },
                  {
                    name: "SourceOperationEvent",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Keypad",
                              value: 0
                            },
                            {
                              name: "RF",
                              value: 1
                            },
                            {
                              name: "Manual",
                              value: 2
                            },
                            {
                              name: "RFID",
                              value: 3
                            },
                            {
                              name: "Indeterminate",
                              value: 255
                            }
                    ],
                  },
                  {
                    name: "EventIDOrAlarmCode",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "PIN",
                    type: "text",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 5,
              name: "SetPINCode",
              required: false,
              fields: [
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserStatus",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "OccupiedEnabled",
                              value: 1
                            },
                            {
                              name: "OccupiedDisabled",
                              value: 3
                            }
                    ],
                  },
                  {
                    name: "UserType",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "UnrestrictedUser",
                              value: 0
                            },
                            {
                              name: "YearDayScheduleUser",
                              value: 1
                            },
                            {
                              name: "WeekDayScheduleUser",
                              value: 2
                            },
                            {
                              name: "MasterUser",
                              value: 3
                            },
                            {
                              name: "NonAccessUser",
                              value: 4
                            },
                            {
                              name: "ForcedUser",
                              value: 5
                            },
                            {
                              name: "DisposableUser",
                              value: 6
                            },
                            {
                              name: "ExpiringUser",
                              value: 7
                            },
                            {
                              name: "ScheduleRestrictedUser",
                              value: 8
                            },
                            {
                              name: "RemoteOnlyUser",
                              value: 9
                            },
                            {
                              name: "NotSupported",
                              value: 255
                            }
                    ],
                  },
                  {
                    name: "PIN",
                    type: "text",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 5,
              name: "SetPINCodeResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Success",
                              value: 0
                            },
                            {
                              name: "GeneralFailure",
                              value: 1
                            },
                            {
                              name: "MemoryFull",
                              value: 2
                            },
                            {
                              name: "DuplicateCode",
                              value: 3
                            }
                    ],
                  }
              ]              
            },
            {
              id: 6,
              name: "GetPINCode",
              required: false,
              fields: [
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 6,
              name: "GetPINCodeResponse",
              required: false,
              fields: [
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserStatus",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Available",
                              value: 0
                            },
                            {
                              name: "OccupiedEnabled",
                              value: 1
                            },
                            {
                              name: "OccupiedDisabled",
                              value: 3
                            },
                            {
                              name: "NotSupported",
                              value: 255
                            }
                    ],
                  },
                  {
                    name: "UserType",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "UnrestrictedUser",
                              value: 0
                            },
                            {
                              name: "YearDayScheduleUser",
                              value: 1
                            },
                            {
                              name: "WeekDayScheduleUser",
                              value: 2
                            },
                            {
                              name: "MasterUser",
                              value: 3
                            },
                            {
                              name: "NonAccessUser",
                              value: 4
                            },
                            {
                              name: "ForcedUser",
                              value: 5
                            },
                            {
                              name: "DisposableUser",
                              value: 6
                            },
                            {
                              name: "ExpiringUser",
                              value: 7
                            },
                            {
                              name: "ScheduleRestrictedUser",
                              value: 8
                            },
                            {
                              name: "RemoteOnlyUser",
                              value: 9
                            },
                            {
                              name: "NotSupported",
                              value: 255
                            }
                    ],
                  },
                  {
                    name: "Code",
                    type: "text",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 7,
              name: "ClearPINCode",
              required: false,
              fields: [
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 7,
              name: "ClearPINCodeResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Pass",
                              value: 0
                            },
                            {
                              name: "Fail",
                              value: 1
                            }
                    ],
                  }
              ]              
            },
            {
              id: 8,
              name: "ClearAllPINCodes",
              required: false,
              fields: [
              ]              
            },
            {
              id: 8,
              name: "ClearAllPINCodesResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Pass",
                              value: 0
                            },
                            {
                              name: "Fail",
                              value: 1
                            }
                    ],
                  }
              ]              
            },
            {
              id: 9,
              name: "SetUserStatus",
              required: false,
              fields: [
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserStatus",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "OccupiedEnabled",
                              value: 1
                            },
                            {
                              name: "OccupiedDisabled",
                              value: 3
                            }
                    ],
                  }
              ]              
            },
            {
              id: 9,
              name: "SetUserStatusResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Pass",
                              value: 0
                            },
                            {
                              name: "Fail",
                              value: 1
                            }
                    ],
                  }
              ]              
            },
            {
              id: 10,
              name: "GetUserStatus",
              required: false,
              fields: [
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 10,
              name: "GetUserStatusResponse",
              required: false,
              fields: [
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserStatus",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Available",
                              value: 0
                            },
                            {
                              name: "OccupiedEnabled",
                              value: 1
                            },
                            {
                              name: "OccupiedDisabled",
                              value: 3
                            },
                            {
                              name: "NotSupported",
                              value: 255
                            }
                    ],
                  }
              ]              
            },
            {
              id: 11,
              name: "SetWeekdaySchedule",
              required: false,
              fields: [
                  {
                    name: "ScheduleID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "DaysMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "Sun",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "Mon",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            },
                            {
                              name: "Tue",
                              mask: 4,
                              type: "boolean",
                              offset: 2,
                            },
                            {
                              name: "Wed",
                              mask: 8,
                              type: "boolean",
                              offset: 3,
                            },
                            {
                              name: "Thu",
                              mask: 16,
                              type: "boolean",
                              offset: 4,
                            },
                            {
                              name: "Fri",
                              mask: 32,
                              type: "boolean",
                              offset: 5,
                            },
                            {
                              name: "Sat",
                              mask: 64,
                              type: "boolean",
                              offset: 6,
                            },
                            {
                              name: "Enable",
                              mask: 128,
                              type: "boolean",
                              offset: 7,
                            }
                    ]
                  },
                  {
                    name: "StartHour",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "StartMinute",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "EndHour",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "EndMinute",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 11,
              name: "SetWeekdayScheduleResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Pass",
                              value: 0
                            },
                            {
                              name: "Fail",
                              value: 1
                            }
                    ],
                  }
              ]              
            },
            {
              id: 12,
              name: "GetWeekdaySchedule",
              required: false,
              fields: [
                  {
                    name: "ScheduleID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 12,
              name: "GetWeekdayScheduleResponse",
              required: false,
              fields: [
                  {
                    name: "ScheduleID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "SUCCESS",
                              value: 0
                            },
                            {
                              name: "FAILURE",
                              value: 1
                            },
                            {
                              name: "NOT_AUTHORIZED",
                              value: 126
                            },
                            {
                              name: "MALFORMED_COMMAND",
                              value: 128
                            },
                            {
                              name: "UNSUP_CLUSTER_COMMAND",
                              value: 129
                            },
                            {
                              name: "UNSUP_GENERAL_COMMAND",
                              value: 130
                            },
                            {
                              name: "UNSUP_MANUF_CLUSTER_COMMAND",
                              value: 131
                            },
                            {
                              name: "UNSUP_MANUF_GENERAL_COMMAND",
                              value: 132
                            },
                            {
                              name: "INVALID_FIELD",
                              value: 133
                            },
                            {
                              name: "UNSUPPORTED_ATTRIBUTE",
                              value: 134
                            },
                            {
                              name: "INVALID_VALUE",
                              value: 135
                            },
                            {
                              name: "READ_ONLY",
                              value: 136
                            },
                            {
                              name: "INSUFFICIENT_SPACE",
                              value: 137
                            },
                            {
                              name: "DUPLICATE_EXISTS",
                              value: 138
                            },
                            {
                              name: "NOT_FOUND",
                              value: 139
                            },
                            {
                              name: "UNREPORTABLE_ATTRIBUTE",
                              value: 140
                            },
                            {
                              name: "INVALID_DATA_TYPE",
                              value: 141
                            },
                            {
                              name: "INVALID_SELECTOR",
                              value: 142
                            },
                            {
                              name: "WRITE_ONLY",
                              value: 143
                            },
                            {
                              name: "INCONSISTENT_STARTUP_STATE",
                              value: 144
                            },
                            {
                              name: "DEFINED_OUT_OF_BAND",
                              value: 145
                            },
                            {
                              name: "INCONSISTENT",
                              value: 146
                            },
                            {
                              name: "ACTION_DENIED",
                              value: 147
                            },
                            {
                              name: "TIMEOUT",
                              value: 148
                            },
                            {
                              name: "ABORT",
                              value: 149
                            },
                            {
                              name: "INVALID_IMAGE",
                              value: 150
                            },
                            {
                              name: "WAIT_FOR_DATA",
                              value: 151
                            },
                            {
                              name: "NO_IMAGE_AVAILABLE",
                              value: 152
                            },
                            {
                              name: "REQUIRE_MORE_IMAGE",
                              value: 153
                            },
                            {
                              name: "NOTIFICATION_PENDING",
                              value: 154
                            },
                            {
                              name: "HARDWARE_FAILURE",
                              value: 192
                            },
                            {
                              name: "SOFTWARE_FAILURE",
                              value: 193
                            },
                            {
                              name: "CALIBRATION_ERROR",
                              value: 194
                            },
                            {
                              name: "UNSUPPORTED_CLUSTER",
                              value: 195
                            },
                            {
                              name: "LIMIT_REACHED",
                              value: 196
                            }
                    ],
                  },
                  {
                    name: "DaysMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "Sun",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "Mon",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            },
                            {
                              name: "Tue",
                              mask: 4,
                              type: "boolean",
                              offset: 2,
                            },
                            {
                              name: "Wed",
                              mask: 8,
                              type: "boolean",
                              offset: 3,
                            },
                            {
                              name: "Thu",
                              mask: 16,
                              type: "boolean",
                              offset: 4,
                            },
                            {
                              name: "Fri",
                              mask: 32,
                              type: "boolean",
                              offset: 5,
                            },
                            {
                              name: "Sat",
                              mask: 64,
                              type: "boolean",
                              offset: 6,
                            },
                            {
                              name: "Enable",
                              mask: 128,
                              type: "boolean",
                              offset: 7,
                            }
                    ]
                  },
                  {
                    name: "StartHour",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "StartMinute",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "EndHour",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "EndMinute",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 13,
              name: "ClearWeekdaySchedule",
              required: false,
              fields: [
                  {
                    name: "ScheduleID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 13,
              name: "ClearWeekdayScheduleResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Pass",
                              value: 0
                            },
                            {
                              name: "Fail",
                              value: 1
                            }
                    ],
                  }
              ]              
            },
            {
              id: 14,
              name: "SetYearDaySchedule",
              required: false,
              fields: [
                  {
                    name: "ScheduleID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "LocalStartTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "LocalEndTime",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 14,
              name: "SetYearDayScheduleResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Pass",
                              value: 0
                            },
                            {
                              name: "Fail",
                              value: 1
                            }
                    ],
                  }
              ]              
            },
            {
              id: 15,
              name: "GetYearDaySchedule",
              required: false,
              fields: [
                  {
                    name: "ScheduleID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 15,
              name: "GetYearDayScheduleResponse",
              required: false,
              fields: [
                  {
                    name: "ScheduleID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "SUCCESS",
                              value: 0
                            },
                            {
                              name: "FAILURE",
                              value: 1
                            },
                            {
                              name: "NOT_AUTHORIZED",
                              value: 126
                            },
                            {
                              name: "MALFORMED_COMMAND",
                              value: 128
                            },
                            {
                              name: "UNSUP_CLUSTER_COMMAND",
                              value: 129
                            },
                            {
                              name: "UNSUP_GENERAL_COMMAND",
                              value: 130
                            },
                            {
                              name: "UNSUP_MANUF_CLUSTER_COMMAND",
                              value: 131
                            },
                            {
                              name: "UNSUP_MANUF_GENERAL_COMMAND",
                              value: 132
                            },
                            {
                              name: "INVALID_FIELD",
                              value: 133
                            },
                            {
                              name: "UNSUPPORTED_ATTRIBUTE",
                              value: 134
                            },
                            {
                              name: "INVALID_VALUE",
                              value: 135
                            },
                            {
                              name: "READ_ONLY",
                              value: 136
                            },
                            {
                              name: "INSUFFICIENT_SPACE",
                              value: 137
                            },
                            {
                              name: "DUPLICATE_EXISTS",
                              value: 138
                            },
                            {
                              name: "NOT_FOUND",
                              value: 139
                            },
                            {
                              name: "UNREPORTABLE_ATTRIBUTE",
                              value: 140
                            },
                            {
                              name: "INVALID_DATA_TYPE",
                              value: 141
                            },
                            {
                              name: "INVALID_SELECTOR",
                              value: 142
                            },
                            {
                              name: "WRITE_ONLY",
                              value: 143
                            },
                            {
                              name: "INCONSISTENT_STARTUP_STATE",
                              value: 144
                            },
                            {
                              name: "DEFINED_OUT_OF_BAND",
                              value: 145
                            },
                            {
                              name: "INCONSISTENT",
                              value: 146
                            },
                            {
                              name: "ACTION_DENIED",
                              value: 147
                            },
                            {
                              name: "TIMEOUT",
                              value: 148
                            },
                            {
                              name: "ABORT",
                              value: 149
                            },
                            {
                              name: "INVALID_IMAGE",
                              value: 150
                            },
                            {
                              name: "WAIT_FOR_DATA",
                              value: 151
                            },
                            {
                              name: "NO_IMAGE_AVAILABLE",
                              value: 152
                            },
                            {
                              name: "REQUIRE_MORE_IMAGE",
                              value: 153
                            },
                            {
                              name: "NOTIFICATION_PENDING",
                              value: 154
                            },
                            {
                              name: "HARDWARE_FAILURE",
                              value: 192
                            },
                            {
                              name: "SOFTWARE_FAILURE",
                              value: 193
                            },
                            {
                              name: "CALIBRATION_ERROR",
                              value: 194
                            },
                            {
                              name: "UNSUPPORTED_CLUSTER",
                              value: 195
                            },
                            {
                              name: "LIMIT_REACHED",
                              value: 196
                            }
                    ],
                  },
                  {
                    name: "LocalStartTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "LocalEndTime",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 16,
              name: "ClearYearDaySchedule",
              required: false,
              fields: [
                  {
                    name: "ScheduleID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 16,
              name: "ClearYearDayScheduleResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Pass",
                              value: 0
                            },
                            {
                              name: "Fail",
                              value: 1
                            }
                    ],
                  }
              ]              
            },
            {
              id: 17,
              name: "SetHolidaySchedule",
              required: false,
              fields: [
                  {
                    name: "HolidayScheduleID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "LocalStartTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "LocalEndTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OperatingModeDuringHoliday",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Normal",
                              value: 0
                            },
                            {
                              name: "Vacation",
                              value: 1
                            },
                            {
                              name: "Privacy",
                              value: 2
                            },
                            {
                              name: "NoRFLockOrUnlock",
                              value: 3
                            },
                            {
                              name: "Passage",
                              value: 4
                            }
                    ],
                  }
              ]              
            },
            {
              id: 17,
              name: "SetHolidayScheduleResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Pass",
                              value: 0
                            },
                            {
                              name: "Fail",
                              value: 1
                            }
                    ],
                  }
              ]              
            },
            {
              id: 18,
              name: "GetHolidaySchedule",
              required: false,
              fields: [
                  {
                    name: "HolidayScheduleID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 18,
              name: "GetHolidayScheduleResponse",
              required: false,
              fields: [
                  {
                    name: "HolidayScheduleID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "SUCCESS",
                              value: 0
                            },
                            {
                              name: "FAILURE",
                              value: 1
                            },
                            {
                              name: "NOT_AUTHORIZED",
                              value: 126
                            },
                            {
                              name: "MALFORMED_COMMAND",
                              value: 128
                            },
                            {
                              name: "UNSUP_CLUSTER_COMMAND",
                              value: 129
                            },
                            {
                              name: "UNSUP_GENERAL_COMMAND",
                              value: 130
                            },
                            {
                              name: "UNSUP_MANUF_CLUSTER_COMMAND",
                              value: 131
                            },
                            {
                              name: "UNSUP_MANUF_GENERAL_COMMAND",
                              value: 132
                            },
                            {
                              name: "INVALID_FIELD",
                              value: 133
                            },
                            {
                              name: "UNSUPPORTED_ATTRIBUTE",
                              value: 134
                            },
                            {
                              name: "INVALID_VALUE",
                              value: 135
                            },
                            {
                              name: "READ_ONLY",
                              value: 136
                            },
                            {
                              name: "INSUFFICIENT_SPACE",
                              value: 137
                            },
                            {
                              name: "DUPLICATE_EXISTS",
                              value: 138
                            },
                            {
                              name: "NOT_FOUND",
                              value: 139
                            },
                            {
                              name: "UNREPORTABLE_ATTRIBUTE",
                              value: 140
                            },
                            {
                              name: "INVALID_DATA_TYPE",
                              value: 141
                            },
                            {
                              name: "INVALID_SELECTOR",
                              value: 142
                            },
                            {
                              name: "WRITE_ONLY",
                              value: 143
                            },
                            {
                              name: "INCONSISTENT_STARTUP_STATE",
                              value: 144
                            },
                            {
                              name: "DEFINED_OUT_OF_BAND",
                              value: 145
                            },
                            {
                              name: "INCONSISTENT",
                              value: 146
                            },
                            {
                              name: "ACTION_DENIED",
                              value: 147
                            },
                            {
                              name: "TIMEOUT",
                              value: 148
                            },
                            {
                              name: "ABORT",
                              value: 149
                            },
                            {
                              name: "INVALID_IMAGE",
                              value: 150
                            },
                            {
                              name: "WAIT_FOR_DATA",
                              value: 151
                            },
                            {
                              name: "NO_IMAGE_AVAILABLE",
                              value: 152
                            },
                            {
                              name: "REQUIRE_MORE_IMAGE",
                              value: 153
                            },
                            {
                              name: "NOTIFICATION_PENDING",
                              value: 154
                            },
                            {
                              name: "HARDWARE_FAILURE",
                              value: 192
                            },
                            {
                              name: "SOFTWARE_FAILURE",
                              value: 193
                            },
                            {
                              name: "CALIBRATION_ERROR",
                              value: 194
                            },
                            {
                              name: "UNSUPPORTED_CLUSTER",
                              value: 195
                            },
                            {
                              name: "LIMIT_REACHED",
                              value: 196
                            }
                    ],
                  },
                  {
                    name: "LocalStartTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "LocalEndTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OperatingModeDuringHoliday",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Normal",
                              value: 0
                            },
                            {
                              name: "Vacation",
                              value: 1
                            },
                            {
                              name: "Privacy",
                              value: 2
                            },
                            {
                              name: "NoRFLockOrUnlock",
                              value: 3
                            },
                            {
                              name: "Passage",
                              value: 4
                            }
                    ],
                  }
              ]              
            },
            {
              id: 19,
              name: "ClearHolidaySchedule",
              required: false,
              fields: [
                  {
                    name: "HolidayScheduleID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 19,
              name: "ClearHolidayScheduleResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Pass",
                              value: 0
                            },
                            {
                              name: "Fail",
                              value: 1
                            }
                    ],
                  }
              ]              
            },
            {
              id: 20,
              name: "SetUserType",
              required: false,
              fields: [
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserType",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "UnrestrictedUser",
                              value: 0
                            },
                            {
                              name: "YearDayScheduleUser",
                              value: 1
                            },
                            {
                              name: "WeekDayScheduleUser",
                              value: 2
                            },
                            {
                              name: "MasterUser",
                              value: 3
                            },
                            {
                              name: "NonAccessUser",
                              value: 4
                            },
                            {
                              name: "ForcedUser",
                              value: 5
                            },
                            {
                              name: "DisposableUser",
                              value: 6
                            },
                            {
                              name: "ExpiringUser",
                              value: 7
                            },
                            {
                              name: "ScheduleRestrictedUser",
                              value: 8
                            },
                            {
                              name: "RemoteOnlyUser",
                              value: 9
                            },
                            {
                              name: "NotSupported",
                              value: 255
                            }
                    ],
                  }
              ]              
            },
            {
              id: 20,
              name: "SetUserTypeResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Pass",
                              value: 0
                            },
                            {
                              name: "Fail",
                              value: 1
                            }
                    ],
                  }
              ]              
            },
            {
              id: 21,
              name: "GetUserType",
              required: false,
              fields: [
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 21,
              name: "GetUserTypeResponse",
              required: false,
              fields: [
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserType",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "UnrestrictedUser",
                              value: 0
                            },
                            {
                              name: "YearDayScheduleUser",
                              value: 1
                            },
                            {
                              name: "WeekDayScheduleUser",
                              value: 2
                            },
                            {
                              name: "MasterUser",
                              value: 3
                            },
                            {
                              name: "NonAccessUser",
                              value: 4
                            },
                            {
                              name: "ForcedUser",
                              value: 5
                            },
                            {
                              name: "DisposableUser",
                              value: 6
                            },
                            {
                              name: "ExpiringUser",
                              value: 7
                            },
                            {
                              name: "ScheduleRestrictedUser",
                              value: 8
                            },
                            {
                              name: "RemoteOnlyUser",
                              value: 9
                            },
                            {
                              name: "NotSupported",
                              value: 255
                            }
                    ],
                  }
              ]              
            },
            {
              id: 22,
              name: "SetRFIDCode",
              required: false,
              fields: [
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserStatus",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "OccupiedEnabled",
                              value: 1
                            },
                            {
                              name: "OccupiedDisabled",
                              value: 3
                            }
                    ],
                  },
                  {
                    name: "UserType",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "UnrestrictedUser",
                              value: 0
                            },
                            {
                              name: "YearDayScheduleUser",
                              value: 1
                            },
                            {
                              name: "WeekDayScheduleUser",
                              value: 2
                            },
                            {
                              name: "MasterUser",
                              value: 3
                            },
                            {
                              name: "NonAccessUser",
                              value: 4
                            },
                            {
                              name: "ForcedUser",
                              value: 5
                            },
                            {
                              name: "DisposableUser",
                              value: 6
                            },
                            {
                              name: "ExpiringUser",
                              value: 7
                            },
                            {
                              name: "ScheduleRestrictedUser",
                              value: 8
                            },
                            {
                              name: "RemoteOnlyUser",
                              value: 9
                            },
                            {
                              name: "NotSupported",
                              value: 255
                            }
                    ],
                  },
                  {
                    name: "RFIDCode",
                    type: "text",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 22,
              name: "SetRFIDCodeResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Success",
                              value: 0
                            },
                            {
                              name: "GeneralFailure",
                              value: 1
                            },
                            {
                              name: "MemoryFull",
                              value: 2
                            },
                            {
                              name: "DuplicateCode",
                              value: 3
                            }
                    ],
                  }
              ]              
            },
            {
              id: 23,
              name: "GetRFIDCode",
              required: false,
              fields: [
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 23,
              name: "GetRFIDCodeResponse",
              required: false,
              fields: [
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserStatus",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Available",
                              value: 0
                            },
                            {
                              name: "OccupiedEnabled",
                              value: 1
                            },
                            {
                              name: "OccupiedDisabled",
                              value: 3
                            },
                            {
                              name: "NotSupported",
                              value: 255
                            }
                    ],
                  },
                  {
                    name: "UserType",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "UnrestrictedUser",
                              value: 0
                            },
                            {
                              name: "YearDayScheduleUser",
                              value: 1
                            },
                            {
                              name: "WeekDayScheduleUser",
                              value: 2
                            },
                            {
                              name: "MasterUser",
                              value: 3
                            },
                            {
                              name: "NonAccessUser",
                              value: 4
                            },
                            {
                              name: "ForcedUser",
                              value: 5
                            },
                            {
                              name: "DisposableUser",
                              value: 6
                            },
                            {
                              name: "ExpiringUser",
                              value: 7
                            },
                            {
                              name: "ScheduleRestrictedUser",
                              value: 8
                            },
                            {
                              name: "RemoteOnlyUser",
                              value: 9
                            },
                            {
                              name: "NotSupported",
                              value: 255
                            }
                    ],
                  },
                  {
                    name: "RFIDCode",
                    type: "text",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 24,
              name: "ClearRFIDCode",
              required: false,
              fields: [
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 24,
              name: "ClearRFIDCodeResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Pass",
                              value: 0
                            },
                            {
                              name: "Fail",
                              value: 1
                            }
                    ],
                  }
              ]              
            },
            {
              id: 25,
              name: "ClearAllRFIDCodes",
              required: false,
              fields: [
              ]              
            },
            {
              id: 25,
              name: "ClearAllRFIDCodesResponse",
              required: false,
              fields: [
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Pass",
                              value: 0
                            },
                            {
                              name: "Fail",
                              value: 1
                            }
                    ],
                  }
              ]              
            },
            {
              id: 26,
              name: "SetUser",
              required: false,
              fields: [
                  {
                    name: "OperationType",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Add",
                              value: 0
                            },
                            {
                              name: "Clear",
                              value: 1
                            },
                            {
                              name: "Modify",
                              value: 2
                            }
                    ],
                  },
                  {
                    name: "UserIndex",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserName",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "UserUniqueID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserStatus",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Available",
                              value: 0
                            },
                            {
                              name: "OccupiedEnabled",
                              value: 1
                            },
                            {
                              name: "OccupiedDisabled",
                              value: 3
                            },
                            {
                              name: "NotSupported",
                              value: 255
                            }
                    ],
                  },
                  {
                    name: "UserType",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "UnrestrictedUser",
                              value: 0
                            },
                            {
                              name: "YearDayScheduleUser",
                              value: 1
                            },
                            {
                              name: "WeekDayScheduleUser",
                              value: 2
                            },
                            {
                              name: "MasterUser",
                              value: 3
                            },
                            {
                              name: "NonAccessUser",
                              value: 4
                            },
                            {
                              name: "ForcedUser",
                              value: 5
                            },
                            {
                              name: "DisposableUser",
                              value: 6
                            },
                            {
                              name: "ExpiringUser",
                              value: 7
                            },
                            {
                              name: "ScheduleRestrictedUser",
                              value: 8
                            },
                            {
                              name: "RemoteOnlyUser",
                              value: 9
                            },
                            {
                              name: "NotSupported",
                              value: 255
                            }
                    ],
                  },
                  {
                    name: "CredentialRule",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Single",
                              value: 0
                            },
                            {
                              name: "Dual",
                              value: 1
                            },
                            {
                              name: "Tri",
                              value: 2
                            }
                    ],
                  }
              ]              
            },
            {
              id: 27,
              name: "GetUser",
              required: false,
              fields: [
                  {
                    name: "UserIndex",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 28,
              name: "GetUserResponse",
              required: false,
              fields: [
              ]              
            },
            {
              id: 29,
              name: "ClearUser",
              required: false,
              fields: [
                  {
                    name: "UserIndex",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 32,
              name: "OperatingEventNotification",
              required: false,
              fields: [
                  {
                    name: "OperationEventSource",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Keypad",
                              value: 0
                            },
                            {
                              name: "RF",
                              value: 1
                            },
                            {
                              name: "Manual",
                              value: 2
                            },
                            {
                              name: "RFID",
                              value: 3
                            },
                            {
                              name: "Indeterminate",
                              value: 255
                            }
                    ],
                  },
                  {
                    name: "OperationEventCode",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "UnknownOrMS",
                              value: 0
                            },
                            {
                              name: "Lock",
                              value: 1
                            },
                            {
                              name: "Unlock",
                              value: 2
                            },
                            {
                              name: "LockFailureInvalidPINOrID",
                              value: 3
                            },
                            {
                              name: "LockFailureInvalidSchedule",
                              value: 4
                            },
                            {
                              name: "UnlockFailureInvalidPINOrID",
                              value: 5
                            },
                            {
                              name: "UnlockFailureInvalidSchedule",
                              value: 6
                            },
                            {
                              name: "OneTouchLock",
                              value: 7
                            },
                            {
                              name: "KeyLock",
                              value: 8
                            },
                            {
                              name: "KeyUnlock",
                              value: 9
                            },
                            {
                              name: "AutoLock",
                              value: 10
                            },
                            {
                              name: "ScheduleLock",
                              value: 11
                            },
                            {
                              name: "ScheduleUnlock",
                              value: 12
                            },
                            {
                              name: "ManualLock",
                              value: 13
                            },
                            {
                              name: "ManualUnlock",
                              value: 14
                            },
                            {
                              name: "NonAccessUserOperationalEvent",
                              value: 14
                            }
                    ],
                  },
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "PIN",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "LocalTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "Data",
                    type: "text",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 33,
              name: "ProgrammingEventNotification",
              required: false,
              fields: [
                  {
                    name: "ProgramEventSource",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Keypad",
                              value: 0
                            },
                            {
                              name: "RF",
                              value: 1
                            },
                            {
                              name: "RFID",
                              value: 3
                            },
                            {
                              name: "Indeterminate",
                              value: 255
                            }
                    ],
                  },
                  {
                    name: "ProgramEventCode",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "UnknownOrMS",
                              value: 0
                            },
                            {
                              name: "MasterCodeChanged",
                              value: 1
                            },
                            {
                              name: "PINCodeAdded",
                              value: 2
                            },
                            {
                              name: "PINCodeDeleted",
                              value: 3
                            },
                            {
                              name: "PINCodeChanged",
                              value: 4
                            },
                            {
                              name: "RFIDCodeAdded",
                              value: 5
                            },
                            {
                              name: "RFIDCodeDeleted",
                              value: 6
                            }
                    ],
                  },
                  {
                    name: "UserID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "PIN",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "UserType",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "UnrestrictedUser",
                              value: 0
                            },
                            {
                              name: "YearDayScheduleUser",
                              value: 1
                            },
                            {
                              name: "WeekDayScheduleUser",
                              value: 2
                            },
                            {
                              name: "MasterUser",
                              value: 3
                            },
                            {
                              name: "NonAccessUser",
                              value: 4
                            },
                            {
                              name: "ForcedUser",
                              value: 5
                            },
                            {
                              name: "DisposableUser",
                              value: 6
                            },
                            {
                              name: "ExpiringUser",
                              value: 7
                            },
                            {
                              name: "ScheduleRestrictedUser",
                              value: 8
                            },
                            {
                              name: "RemoteOnlyUser",
                              value: 9
                            },
                            {
                              name: "NotSupported",
                              value: 255
                            }
                    ],
                  },
                  {
                    name: "UserStatus",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Available",
                              value: 0
                            },
                            {
                              name: "OccupiedEnabled",
                              value: 1
                            },
                            {
                              name: "OccupiedDisabled",
                              value: 3
                            },
                            {
                              name: "NotSupported",
                              value: 255
                            }
                    ],
                  },
                  {
                    name: "LocalTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "Data",
                    type: "text",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 34,
              name: "SetCredential",
              required: false,
              fields: [
                  {
                    name: "OperationType",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Add",
                              value: 0
                            },
                            {
                              name: "Clear",
                              value: 1
                            },
                            {
                              name: "Modify",
                              value: 2
                            }
                    ],
                  },
                  {
                    name: "Credential",
                    type: "struct",
                    isArray: false,
                    struct:[
                      {
                        name:"CredentialType",
                        type:"enum",
                        isArray: false
                      },
                      {
                        name:"CredentialIndex",
                        type:"number",
                        isArray: false
                      }
                    ],
                  },
                  {
                    name: "CredentialData",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "UserIndex",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UserStatus",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Available",
                              value: 0
                            },
                            {
                              name: "OccupiedEnabled",
                              value: 1
                            },
                            {
                              name: "OccupiedDisabled",
                              value: 3
                            },
                            {
                              name: "NotSupported",
                              value: 255
                            }
                    ],
                  },
                  {
                    name: "UserType",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "UnrestrictedUser",
                              value: 0
                            },
                            {
                              name: "YearDayScheduleUser",
                              value: 1
                            },
                            {
                              name: "WeekDayScheduleUser",
                              value: 2
                            },
                            {
                              name: "MasterUser",
                              value: 3
                            },
                            {
                              name: "NonAccessUser",
                              value: 4
                            },
                            {
                              name: "ForcedUser",
                              value: 5
                            },
                            {
                              name: "DisposableUser",
                              value: 6
                            },
                            {
                              name: "ExpiringUser",
                              value: 7
                            },
                            {
                              name: "ScheduleRestrictedUser",
                              value: 8
                            },
                            {
                              name: "RemoteOnlyUser",
                              value: 9
                            },
                            {
                              name: "NotSupported",
                              value: 255
                            }
                    ],
                  }
              ]              
            },
            {
              id: 35,
              name: "SetCredentialResponse",
              required: false,
              fields: [
              ]              
            },
            {
              id: 36,
              name: "GetCredentialStatus",
              required: false,
              fields: [
                  {
                    name: "Credential",
                    type: "struct",
                    isArray: false,
                    struct:[
                      {
                        name:"CredentialType",
                        type:"enum",
                        isArray: false
                      },
                      {
                        name:"CredentialIndex",
                        type:"number",
                        isArray: false
                      }
                    ],
                  }
              ]              
            },
            {
              id: 37,
              name: "GetCredentialStatusResponse",
              required: false,
              fields: [
              ]              
            },
            {
              id: 38,
              name: "ClearCredential",
              required: false,
              fields: [
                  {
                    name: "Credential",
                    type: "struct",
                    isArray: false,
                    struct:[
                      {
                        name:"CredentialType",
                        type:"enum",
                        isArray: false
                      },
                      {
                        name:"CredentialIndex",
                        type:"number",
                        isArray: false
                      }
                    ],
                  }
              ]              
            },
            {
              id: 39,
              name: "UnboltDoor",
              required: false,
              fields: [
                  {
                    name: "PINCode",
                    type: "text",
                    isArray: false,
                  }
              ]              
            }
        ] 
      }
    },
    WindowCovering: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "WindowCoveringType",
              type: "enum",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              enum: [
                { name: "Rollershade", value: 0},
																{ name: "Rollershade2Motor", value: 1},
																{ name: "RollershadeExterior", value: 2},
																{ name: "RollershadeExterior2Motor", value: 3},
																{ name: "Drapery", value: 4},
																{ name: "Awning", value: 5},
																{ name: "Shutter", value: 6},
																{ name: "TiltBlindTiltOnly", value: 7},
																{ name: "TiltBlindLiftAndTilt", value: 8},
																{ name: "ProjectorScreen", value: 9}
              ],
            },
            {                
              id: 1,
              name: "PhysicalClosedLimitLift",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 2,
              name: "PhysicalClosedLimitTilt",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 3,
              name: "CurrentPositionLift",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 4,
              name: "CurrentPositionTilt",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 5,
              name: "NumberOfActuationsLift",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 6,
              name: "NumberOfActuationsTilt",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 7,
              name: "ConfigOrStatus",
              type: "bitmap",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 3,
              bitmap: [
                { name: "Operational", mask: 1, type: "boolean", offset: 0 },
								{ name: "Online", mask: 2, type: "boolean", offset: 1 },
								{ name: "OpenAndUpCommandsReversed", mask: 4, type: "boolean", offset: 2 },
								{ name: "LiftClosedLoop", mask: 8, type: "boolean", offset: 3 },
								{ name: "TiltClosedLoop", mask: 16, type: "boolean", offset: 4 },
								{ name: "LiftEncoderControlled", mask: 32, type: "boolean", offset: 5 },
								{ name: "TiltEncoderControlled", mask: 64, type: "boolean", offset: 6 }
              ]
            },
            {                
              id: 8,
              name: "CurrentPositionLiftPercentage",
              type: "number",            
              max: 100,
              required: false,
              reportRequired: true,
              sceneRequired: true,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 9,
              name: "CurrentPositionTiltPercentage",
              type: "number",            
              max: 100,
              required: false,
              reportRequired: true,
              sceneRequired: true,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 256,
              name: "InstalledOpenLimitLift",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 257,
              name: "InstalledClosedLimitLift",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 258,
              name: "InstalledOpenLimitTilt",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 259,
              name: "InstalledClosedLimitTilt",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 260,
              name: "VelocityLift",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 261,
              name: "AccelerationTimeLift",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 262,
              name: "DecelerationTimeLift",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 263,
              name: "Mode",
              type: "bitmap",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 4,
              bitmap: [
                { name: "MotorDirectionReversed", mask: 1, type: "boolean", offset: 0 },
								{ name: "CalibrationMode", mask: 2, type: "boolean", offset: 1 },
								{ name: "MaintenanceMode", mask: 4, type: "boolean", offset: 2 },
								{ name: "LEDFeedback", mask: 8, type: "boolean", offset: 3 }
              ]
            },
            {                
              id: 264,
              name: "IntermediateSetpointsLift",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: NaN,
            },
            {                
              id: 265,
              name: "IntermediateSetpointsTilt",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: NaN,
            }
        ],
        commands: [
            {
              id: 0,
              name: "UpOrOpen",
              required: true,
              fields: [
              ]              
            },
            {
              id: 1,
              name: "DownOrClose",
              required: true,
              fields: [
              ]              
            },
            {
              id: 2,
              name: "Stop",
              required: true,
              fields: [
              ]              
            },
            {
              id: 4,
              name: "GoToLiftValue",
              required: false,
              fields: [
                  {
                    name: "LiftValue",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 5,
              name: "GoToLiftPercentage",
              required: false,
              fields: [
                  {
                    name: "PercentageLiftValue",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 7,
              name: "GoToTiltValue",
              required: false,
              fields: [
                  {
                    name: "TiltValue",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 8,
              name: "GoToTiltPercentage",
              required: false,
              fields: [
                  {
                    name: "PercentageTiltValue",
                    type: "number",
                    isArray: false,
                  }
              ]              
            }
        ] 
      }
    },
    BarrierControl: {
      server:{
        attributes: [
            {                
              id: 1,
              name: "MovingState",
              type: "enum",            
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              enum: [
                { name: "Stopped", value: 0},
																{ name: "Closing", value: 1},
																{ name: "Opening", value: 2}
              ],
            },
            {                
              id: 2,
              name: "SafetyStatus",
              type: "bitmap",            
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              bitmap: [
                { name: "RemoteLockout", mask: 1, type: "boolean", offset: 0 },
								{ name: "TamperDetected", mask: 2, type: "boolean", offset: 1 },
								{ name: "FailedCommunication", mask: 4, type: "boolean", offset: 2 },
								{ name: "PositionFailure", mask: 8, type: "boolean", offset: 3 }
              ]
            },
            {                
              id: 3,
              name: "Capabilities",
              type: "bitmap",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              bitmap: [
                { name: "PartialBarrier", mask: 1, type: "boolean", offset: 0 }
              ]
            },
            {                
              id: 4,
              name: "OpenEvents",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 5,
              name: "CloseEvents",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 6,
              name: "CommandOpenEvents",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 7,
              name: "CommandCloseEvents",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 8,
              name: "OpenPeriod",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 9,
              name: "ClosePeriod",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 10,
              name: "BarrierPosition",
              type: "number",            
              max: 100,
              required: true,
              reportRequired: true,
              sceneRequired: true,
              writable: false,
              isArray: false,
            }
        ],
        commands: [
            {
              id: 0,
              name: "GoToPercent",
              required: true,
              fields: [
                  {
                    name: "PercentOpen",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 1,
              name: "Stop",
              required: true,
              fields: [
              ]              
            }
        ] 
      }
    },
    Thermostat: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "LocalTemperature",
              type: "number",            
              max: 32767,
              min: -27315,
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 1,
              name: "OutdoorTemperature",
              type: "number",            
              max: 32767,
              min: -27315,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 2,
              name: "Occupancy",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "Occupied", mask: 1, type: "boolean", offset: 0 }
              ]
            },
            {                
              id: 3,
              name: "AbsMinHeatSetpointLimit",
              type: "number",            
              max: 32767,
              min: -27315,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 700,
            },
            {                
              id: 4,
              name: "AbsMaxHeatSetpointLimit",
              type: "number",            
              max: 32767,
              min: -27315,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 3000,
            },
            {                
              id: 5,
              name: "AbsMinCoolSetpointLimit",
              type: "number",            
              max: 32767,
              min: -27315,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1600,
            },
            {                
              id: 6,
              name: "AbsMaxCoolSetpointLimit",
              type: "number",            
              max: 32767,
              min: -27315,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 3200,
            },
            {                
              id: 7,
              name: "PICoolingDemand",
              type: "number",            
              max: 100,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 8,
              name: "PIHeatingDemand",
              type: "number",            
              max: 100,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 9,
              name: "HVACSystemTypeConfiguration",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "CoolingSystemStage", mask: 3, type: "enum", offset: 0, enum: 
									[ { name: "CoolStage1", value: 0},
								{ name: "CoolStage2", value: 1},
								{ name: "CoolStage3", value: 2} ] },
								{ name: "HeatingSystemStage", mask: 12, type: "enum", offset: 2, enum: 
									[ { name: "HeatStage1", value: 0},
								{ name: "HeatStage2", value: 1},
								{ name: "HeatStage3", value: 2} ] },
								{ name: "HeatingSystemType", mask: 16, type: "enum", offset: 4, enum: 
									[ { name: "Conventional", value: 0},
								{ name: "HeatPump", value: 1} ] },
								{ name: "HeatingFuelSource", mask: 32, type: "enum", offset: 5, enum: 
									[ { name: "ElectricOrB", value: 0},
								{ name: "GasOrO", value: 1} ] }
              ]
            },
            {                
              id: 16,
              name: "LocalTemperatureCalibration",
              type: "number",            
              max: 25,
              min: -25,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 17,
              name: "OccupiedCoolingSetpoint",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: true,
              writable: true,
              isArray: false,
              default: 2600,
            },
            {                
              id: 18,
              name: "OccupiedHeatingSetpoint",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: true,
              writable: true,
              isArray: false,
              default: 2000,
            },
            {                
              id: 19,
              name: "UnoccupiedCoolingSetpoint",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 2600,
            },
            {                
              id: 20,
              name: "UnoccupiedHeatingSetpoint",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 2000,
            },
            {                
              id: 21,
              name: "MinHeatSetpointLimit",
              type: "number",            
              max: 32767,
              min: -27315,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 700,
            },
            {                
              id: 22,
              name: "MaxHeatSetpointLimit",
              type: "number",            
              max: 32767,
              min: -27315,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 3000,
            },
            {                
              id: 23,
              name: "MinCoolSetpointLimit",
              type: "number",            
              max: 32767,
              min: -27315,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 1600,
            },
            {                
              id: 24,
              name: "MaxCoolSetpointLimit",
              type: "number",            
              max: 32767,
              min: -27315,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 3200,
            },
            {                
              id: 25,
              name: "MinSetpointDeadBand",
              type: "number",            
              max: 25,
              min: 10,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 25,
            },
            {                
              id: 26,
              name: "RemoteSensing",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "LocalTemperatureRemote", mask: 1, type: "boolean", offset: 0 },
								{ name: "OutdoorTemperatureRemote", mask: 2, type: "boolean", offset: 1 },
								{ name: "OccupancyRemote", mask: 4, type: "boolean", offset: 2 }
              ]
            },
            {                
              id: 27,
              name: "ControlSequenceOfOperation",
              type: "enum",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 4,
              enum: [
                { name: "CoolingOnly", value: 0},
																{ name: "CoolingWithReheat", value: 1},
																{ name: "HeatingOnly", value: 2},
																{ name: "HeatingWithReheat", value: 3},
																{ name: "CoolingAndHeating4Pipes", value: 4},
																{ name: "CoolingAndHeating4PipesWithReheat", value: 5}
              ],
            },
            {                
              id: 28,
              name: "SystemMode",
              type: "enum",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 1,
              enum: [
                { name: "Off", value: 0},
																{ name: "Auto", value: 1},
																{ name: "Cool", value: 3},
																{ name: "Heat", value: 4},
																{ name: "EmergencyHeating", value: 5},
																{ name: "Precooling", value: 6},
																{ name: "FanOnly", value: 7},
																{ name: "Dry", value: 8},
																{ name: "Sleep", value: 9}
              ],
            },
            {                
              id: 29,
              name: "AlarmMask",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "InitializationFailure", mask: 1, type: "boolean", offset: 0 },
								{ name: "HardwareFailure", mask: 2, type: "boolean", offset: 1 },
								{ name: "SelfCalibrationFailure", mask: 4, type: "boolean", offset: 2 }
              ]
            },
            {                
              id: 30,
              name: "ThermostatRunningMode",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              enum: [
                { name: "Off", value: 0},
																{ name: "Cool", value: 3},
																{ name: "Heat", value: 4}
              ],
            },
            {                
              id: 32,
              name: "StartOfWeek",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              enum: [
                { name: "Sunday", value: 0},
																{ name: "Monday", value: 1},
																{ name: "Tuesday", value: 2},
																{ name: "Wednesday", value: 3},
																{ name: "Thursday", value: 4},
																{ name: "Friday", value: 5},
																{ name: "Saturday", value: 6}
              ],
            },
            {                
              id: 33,
              name: "NumberOfWeeklyTransitions",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 34,
              name: "NumberOfDailyTransitions",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 35,
              name: "TemperatureSetpointHold",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              enum: [
                { name: "SetpointHoldOff", value: 0},
																{ name: "SetpointHoldOn", value: 1}
              ],
            },
            {                
              id: 36,
              name: "TemperatureSetpointHoldDuration",
              type: "number",            
              max: 1440,
              min: 0,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 65535,
            },
            {                
              id: 37,
              name: "ThermostatProgrammingOperationMode",
              type: "bitmap",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "ProgrammingMode", mask: 1, type: "enum", offset: 0, enum: 
									[ { name: "SimpleOrSetpoint", value: 0},
								{ name: "Schedule", value: 1} ] },
								{ name: "AutoOrRecovery", mask: 2, type: "boolean", offset: 1 },
								{ name: "EconomyOrEnergyStar", mask: 4, type: "boolean", offset: 2 }
              ]
            },
            {                
              id: 41,
              name: "ThermostatRunningState",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              bitmap: [
                { name: "HeatOn", mask: 1, type: "boolean", offset: 0 },
								{ name: "CoolOn", mask: 2, type: "boolean", offset: 1 },
								{ name: "FanOn", mask: 4, type: "boolean", offset: 2 },
								{ name: "HeatSecondStageOn", mask: 8, type: "boolean", offset: 3 },
								{ name: "CoolSecondStageOn", mask: 16, type: "boolean", offset: 4 },
								{ name: "FanSecondStageOn", mask: 32, type: "boolean", offset: 5 },
								{ name: "FanThirdStageOn", mask: 64, type: "boolean", offset: 6 }
              ]
            },
            {                
              id: 48,
              name: "SetpointChangeSource",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              enum: [
                { name: "Manual", value: 0},
																{ name: "ScheduleOrInternalProgramming", value: 1},
																{ name: "External", value: 2}
              ],
            },
            {                
              id: 49,
              name: "SetpointChangeAmount",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 32768,
            },
            {                
              id: 50,
              name: "SetpointChangeSourceTimestamp",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              struct:[
              ],
            },
            {                
              id: 52,
              name: "OccupiedSetback",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 255,
            },
            {                
              id: 53,
              name: "OccupiedSetbackMin",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 255,
            },
            {                
              id: 54,
              name: "OccupiedSetbackMax",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 255,
            },
            {                
              id: 55,
              name: "UnoccupiedSetback",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 255,
            },
            {                
              id: 56,
              name: "UnoccupiedSetbackMin",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 255,
            },
            {                
              id: 57,
              name: "UnoccupiedSetbackMax",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 255,
            },
            {                
              id: 58,
              name: "EmergencyHeatDelta",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 255,
            },
            {                
              id: 64,
              name: "ACType",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              enum: [
                { name: "CoolingAndFixedSpeed", value: 1},
																{ name: "HeatPumpAndFixedSpeed", value: 2},
																{ name: "CoolingAndInverter", value: 3},
																{ name: "HeatPumpAndInverter", value: 4}
              ],
            },
            {                
              id: 65,
              name: "ACCapacity",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 66,
              name: "ACRefrigerantType",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              enum: [
                { name: "R22", value: 1},
																{ name: "R410a", value: 2},
																{ name: "R407c", value: 3}
              ],
            },
            {                
              id: 67,
              name: "ACCompressorType",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              enum: [
                { name: "T1", value: 1},
																{ name: "T2", value: 2},
																{ name: "T3", value: 3}
              ],
            },
            {                
              id: 68,
              name: "ACErrorCode",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "CompressorFailureOrRefrigerantLeakage", mask: 1, type: "boolean", offset: 0 },
								{ name: "RoomTemperatureSensorFailure", mask: 2, type: "boolean", offset: 1 },
								{ name: "OutdoorTemperatureSensorFailure", mask: 4, type: "boolean", offset: 2 },
								{ name: "IndoorCoilTemperatureSensorFailure", mask: 8, type: "boolean", offset: 3 },
								{ name: "FanFailure", mask: 16, type: "boolean", offset: 4 }
              ]
            },
            {                
              id: 69,
              name: "ACLouverPosition",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              enum: [
                { name: "FullyClosed", value: 1},
																{ name: "FullyOpen", value: 2},
																{ name: "QuarterOpen", value: 3},
																{ name: "HalfOpen", value: 4},
																{ name: "ThreeQuartersOpen", value: 5}
              ],
            },
            {                
              id: 70,
              name: "ACCoilTemperature",
              type: "number",            
              max: 32767,
              min: -27315,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 71,
              name: "ACCapacityFormat",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              enum: [
                { name: "BTUh", value: 0}
              ],
            }
        ],
        commands: [
            {
              id: 0,
              name: "SetpointRaiseOrLower",
              required: true,
              fields: [
                  {
                    name: "Mode",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Heat",
                              value: 0
                            },
                            {
                              name: "Cool",
                              value: 1
                            },
                            {
                              name: "Both",
                              value: 2
                            }
                    ],
                  },
                  {
                    name: "Amount",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 0,
              name: "GetWeeklyScheduleResponse",
              required: false,
              fields: [
                  {
                    name: "NumberOfTransitions",
                    type: "enum",
                    isArray: false,
                    enum:[
                    ],
                  },
                  {
                    name: "DayOfWeek",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "Sunday",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "Monday",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            },
                            {
                              name: "Tuesday",
                              mask: 4,
                              type: "boolean",
                              offset: 2,
                            },
                            {
                              name: "Wednesday",
                              mask: 8,
                              type: "boolean",
                              offset: 3,
                            },
                            {
                              name: "Thursday",
                              mask: 16,
                              type: "boolean",
                              offset: 4,
                            },
                            {
                              name: "Friday",
                              mask: 32,
                              type: "boolean",
                              offset: 5,
                            },
                            {
                              name: "Saturday",
                              mask: 64,
                              type: "boolean",
                              offset: 6,
                            },
                            {
                              name: "AwayOrVacation",
                              mask: 128,
                              type: "boolean",
                              offset: 7,
                            }
                    ]
                  },
                  {
                    name: "Mode",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "Heat",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "Cool",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  },
                  {
                    name: "Transitions",
                    type: "struct",
                    isArray: true,
                    struct:[
                      {
                        name:"TransitionTime",
                        type:"number",
                        isArray: false
                      },
                      {
                        name:"HeatSetPoint",
                        type:"number",
                        isArray: false
                      },
                      {
                        name:"CoolSetPoint",
                        type:"number",
                        isArray: false
                      }
                    ],
                  }
              ]              
            },
            {
              id: 1,
              name: "SetWeeklySchedule",
              required: false,
              fields: [
                  {
                    name: "NumberOfTransitions",
                    type: "enum",
                    isArray: false,
                    enum:[
                    ],
                  },
                  {
                    name: "DayOfWeek",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "Sunday",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "Monday",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            },
                            {
                              name: "Tuesday",
                              mask: 4,
                              type: "boolean",
                              offset: 2,
                            },
                            {
                              name: "Wednesday",
                              mask: 8,
                              type: "boolean",
                              offset: 3,
                            },
                            {
                              name: "Thursday",
                              mask: 16,
                              type: "boolean",
                              offset: 4,
                            },
                            {
                              name: "Friday",
                              mask: 32,
                              type: "boolean",
                              offset: 5,
                            },
                            {
                              name: "Saturday",
                              mask: 64,
                              type: "boolean",
                              offset: 6,
                            },
                            {
                              name: "AwayOrVacation",
                              mask: 128,
                              type: "boolean",
                              offset: 7,
                            }
                    ]
                  },
                  {
                    name: "Mode",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "Heat",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "Cool",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  },
                  {
                    name: "Transitions",
                    type: "struct",
                    isArray: true,
                    struct:[
                      {
                        name:"TransitionTime",
                        type:"number",
                        isArray: false
                      },
                      {
                        name:"HeatSetPoint",
                        type:"number",
                        isArray: false
                      },
                      {
                        name:"CoolSetPoint",
                        type:"number",
                        isArray: false
                      }
                    ],
                  }
              ]              
            },
            {
              id: 1,
              name: "GetRelayStatusLogResponse",
              required: false,
              fields: [
                  {
                    name: "TimeOfDay",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "RelayStatus",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                    ]
                  },
                  {
                    name: "LocalTemperature",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "HumidityPercentage",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "SetPoint",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "UnreadEntries",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 2,
              name: "GetWeeklySchedule",
              required: false,
              fields: [
                  {
                    name: "DaysToReturn",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "Sunday",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "Monday",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            },
                            {
                              name: "Tuesday",
                              mask: 4,
                              type: "boolean",
                              offset: 2,
                            },
                            {
                              name: "Wednesday",
                              mask: 8,
                              type: "boolean",
                              offset: 3,
                            },
                            {
                              name: "Thursday",
                              mask: 16,
                              type: "boolean",
                              offset: 4,
                            },
                            {
                              name: "Friday",
                              mask: 32,
                              type: "boolean",
                              offset: 5,
                            },
                            {
                              name: "Saturday",
                              mask: 64,
                              type: "boolean",
                              offset: 6,
                            },
                            {
                              name: "AwayOrVacation",
                              mask: 128,
                              type: "boolean",
                              offset: 7,
                            }
                    ]
                  },
                  {
                    name: "ModeToReturn",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "Heat",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "Cool",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 3,
              name: "ClearWeeklySchedule",
              required: false,
              fields: [
              ]              
            },
            {
              id: 4,
              name: "GetRelayStatusLog",
              required: false,
              fields: [
              ]              
            }
        ] 
      }
    },
    ColorControl: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "CurrentHue",
              type: "number",            
              max: 254,
              required: false,
              reportRequired: true,
              sceneRequired: true,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 1,
              name: "CurrentSaturation",
              type: "number",            
              max: 254,
              required: false,
              reportRequired: true,
              sceneRequired: true,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 2,
              name: "RemainingTime",
              type: "number",            
              max: 65534,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 3,
              name: "CurrentX",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: true,
              sceneRequired: true,
              writable: false,
              isArray: false,
              default: 24939,
            },
            {                
              id: 4,
              name: "CurrentY",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: true,
              sceneRequired: true,
              writable: false,
              isArray: false,
              default: 24701,
            },
            {                
              id: 5,
              name: "DriftCompensation",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              enum: [
                { name: "None", value: 0},
																{ name: "OtherOrUnknown", value: 1},
																{ name: "TemperatureMonitoring", value: 2},
																{ name: "OpticalLuminanceMonitoringAndFeedback", value: 3},
																{ name: "OpticalColorMonitoringAndFeedback", value: 4}
              ],
            },
            {                
              id: 6,
              name: "CompensationText",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 7,
              name: "ColorTemperatureMireds",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: true,
              sceneRequired: true,
              writable: false,
              isArray: false,
              default: 250,
            },
            {                
              id: 8,
              name: "ColorMode",
              type: "enum",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
              enum: [
                { name: "CurrentHueAndCurrentSaturation", value: 0},
																{ name: "CurrentXAndCurrentY", value: 1},
																{ name: "ColorTemperatureMireds", value: 2}
              ],
            },
            {                
              id: 15,
              name: "Options",
              type: "bitmap",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              bitmap: [
                
              ]
            },
            {                
              id: 16,
              name: "NumberOfPrimaries",
              type: "number",            
              max: 6,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 17,
              name: "Primary1X",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 18,
              name: "Primary1Y",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 19,
              name: "Primary1Intensity",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 21,
              name: "Primary2X",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 22,
              name: "Primary2Y",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 23,
              name: "Primary2Intensity",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 25,
              name: "Primary3X",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 26,
              name: "Primary3Y",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 27,
              name: "Primary3Intensity",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 32,
              name: "Primary4X",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 33,
              name: "Primary4Y",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 34,
              name: "Primary4Intensity",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 36,
              name: "Primary5X",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 37,
              name: "Primary5Y",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 38,
              name: "Primary5Intensity",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 40,
              name: "Primary6X",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 41,
              name: "Primary6Y",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 42,
              name: "Primary6Intensity",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 48,
              name: "WhitePointX",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 49,
              name: "WhitePointY",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 50,
              name: "ColorPointRX",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 51,
              name: "ColorPointRY",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 52,
              name: "ColorPointRIntensity",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 54,
              name: "ColorPointGX",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 55,
              name: "ColorPointGY",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 56,
              name: "ColorPointGIntensity",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 58,
              name: "ColorPointBX",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 59,
              name: "ColorPointBY",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 60,
              name: "ColorPointBIntensity",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 16384,
              name: "EnhancedCurrentHue",
              type: "number",            
              max: 65535,
              required: false,
              reportRequired: false,
              sceneRequired: true,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 16385,
              name: "EnhancedColorMode",
              type: "enum",            
              required: true,
              reportRequired: false,
              sceneRequired: true,
              writable: false,
              isArray: false,
              default: 1,
              enum: [
                { name: "CurrentHueAndCurrentSaturation", value: 0},
																{ name: "CurrentXAndCurrentY", value: 1},
																{ name: "ColorTemperatureMireds", value: 2},
																{ name: "EnhancedCurrentHueAndCurrentSaturation", value: 3}
              ],
            },
            {                
              id: 16386,
              name: "ColorLoopActive",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: true,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 16387,
              name: "ColorLoopDirection",
              type: "enum",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: true,
              writable: false,
              isArray: false,
              default: 0,
              enum: [
                { name: "DecrementEnhancedCurrentHue", value: 0},
																{ name: "IncrementEnhancedCurrentHue", value: 1}
              ],
            },
            {                
              id: 16388,
              name: "ColorLoopTime",
              type: "number",            
              max: 65535,
              required: false,
              reportRequired: false,
              sceneRequired: true,
              writable: false,
              isArray: false,
              default: 25,
            },
            {                
              id: 16389,
              name: "ColorLoopStartEnhancedHue",
              type: "number",            
              max: 65535,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 8960,
            },
            {                
              id: 16390,
              name: "ColorLoopStoredEnhancedHue",
              type: "number",            
              max: 65535,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 16394,
              name: "ColorCapabilities",
              type: "bitmap",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "HueSaturationSupported", mask: 1, type: "boolean", offset: 0 },
								{ name: "EnhancedHueSupported", mask: 2, type: "boolean", offset: 1 },
								{ name: "ColorLoopSupported", mask: 4, type: "boolean", offset: 2 },
								{ name: "XYSupported", mask: 8, type: "boolean", offset: 3 },
								{ name: "ColorTemperatureSupported", mask: 16, type: "boolean", offset: 4 }
              ]
            },
            {                
              id: 16395,
              name: "ColorTempPhysicalMinMireds",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 16396,
              name: "ColorTempPhysicalMaxMireds",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65279,
            },
            {                
              id: 16397,
              name: "CoupleColorTempToLevelMinMireds",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 16400,
              name: "StartUpColorTemperatureMireds",
              type: "number",            
              max: 65279,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            }
        ],
        commands: [
            {
              id: 0,
              name: "MoveToHue",
              required: false,
              fields: [
                  {
                    name: "Hue",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "Direction",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "ShortestDistance",
                              value: 0
                            },
                            {
                              name: "LongestDistance",
                              value: 1
                            },
                            {
                              name: "Up",
                              value: 2
                            },
                            {
                              name: "Down",
                              value: 3
                            }
                    ],
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 1,
              name: "MoveHue",
              required: false,
              fields: [
                  {
                    name: "MoveMode",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Stop",
                              value: 0
                            },
                            {
                              name: "Up",
                              value: 1
                            },
                            {
                              name: "Down",
                              value: 3
                            }
                    ],
                  },
                  {
                    name: "Rate",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 2,
              name: "StepHue",
              required: false,
              fields: [
                  {
                    name: "StepMode",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Up",
                              value: 1
                            },
                            {
                              name: "Down",
                              value: 3
                            }
                    ],
                  },
                  {
                    name: "StepSize",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 3,
              name: "MoveToSaturation",
              required: false,
              fields: [
                  {
                    name: "Saturation",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 4,
              name: "MoveSaturation",
              required: false,
              fields: [
                  {
                    name: "MoveMode",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Stop",
                              value: 0
                            },
                            {
                              name: "Up",
                              value: 1
                            },
                            {
                              name: "Down",
                              value: 3
                            }
                    ],
                  },
                  {
                    name: "Rate",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 5,
              name: "StepSaturation",
              required: false,
              fields: [
                  {
                    name: "StepMode",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Up",
                              value: 1
                            },
                            {
                              name: "Down",
                              value: 3
                            }
                    ],
                  },
                  {
                    name: "StepSize",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 6,
              name: "MoveToHueAndSaturation",
              required: false,
              fields: [
                  {
                    name: "Hue",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "Saturation",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 7,
              name: "MoveToColor",
              required: false,
              fields: [
                  {
                    name: "ColorX",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "ColorY",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 8,
              name: "MoveColor",
              required: false,
              fields: [
                  {
                    name: "RateX",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "RateY",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 9,
              name: "StepColor",
              required: false,
              fields: [
                  {
                    name: "StepX",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "StepY",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 10,
              name: "MoveToColorTemperature",
              required: false,
              fields: [
                  {
                    name: "ColorTemperatureMireds",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 64,
              name: "EnhancedMoveToHue",
              required: false,
              fields: [
                  {
                    name: "EnhancedHue",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "Direction",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "ShortestDistance",
                              value: 0
                            },
                            {
                              name: "LongestDistance",
                              value: 1
                            },
                            {
                              name: "Up",
                              value: 2
                            },
                            {
                              name: "Down",
                              value: 3
                            }
                    ],
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 65,
              name: "EnhancedMoveHue",
              required: false,
              fields: [
                  {
                    name: "MoveMode",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Stop",
                              value: 0
                            },
                            {
                              name: "Up",
                              value: 1
                            },
                            {
                              name: "Down",
                              value: 3
                            }
                    ],
                  },
                  {
                    name: "Rate",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 66,
              name: "EnhancedStepHue",
              required: false,
              fields: [
                  {
                    name: "StepMode",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Up",
                              value: 1
                            },
                            {
                              name: "Down",
                              value: 3
                            }
                    ],
                  },
                  {
                    name: "StepSize",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 67,
              name: "EnhancedMoveToHueAndSaturation",
              required: false,
              fields: [
                  {
                    name: "EnhancedHue",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "Saturation",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 68,
              name: "ColorLoopSet",
              required: false,
              fields: [
                  {
                    name: "UpdateFlags",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "UpdateAction",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "UpdateDirection",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            },
                            {
                              name: "UpdateTime",
                              mask: 4,
                              type: "boolean",
                              offset: 2,
                            },
                            {
                              name: "UpdateStartHue",
                              mask: 8,
                              type: "boolean",
                              offset: 3,
                            }
                    ]
                  },
                  {
                    name: "Action",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "DeactivateColorLoop",
                              value: 0
                            },
                            {
                              name: "ActivateColorLoopFromColorLoopStartEnhancedHue",
                              value: 1
                            },
                            {
                              name: "ActivateColorLoopFromEnhancedCurrentHue",
                              value: 2
                            }
                    ],
                  },
                  {
                    name: "Direction",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "DecrementEnhancedCurrentHue",
                              value: 0
                            },
                            {
                              name: "IncrementEnhancedCurrentHue",
                              value: 1
                            }
                    ],
                  },
                  {
                    name: "Time",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "StartHue",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 71,
              name: "StopMoveStep",
              required: true,
              fields: [
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 75,
              name: "MoveColorTemperature",
              required: false,
              fields: [
                  {
                    name: "MoveMode",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Stop",
                              value: 0
                            },
                            {
                              name: "Up",
                              value: 1
                            },
                            {
                              name: "Down",
                              value: 3
                            }
                    ],
                  },
                  {
                    name: "Rate",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "ColorTemperatureMinimumMireds",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "ColorTemperatureMaximumMireds",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            },
            {
              id: 76,
              name: "StepColorTemperature",
              required: false,
              fields: [
                  {
                    name: "StepMode",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Up",
                              value: 1
                            },
                            {
                              name: "Down",
                              value: 3
                            }
                    ],
                  },
                  {
                    name: "StepSize",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TransitionTime",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "ColorTemperatureMinimumMireds",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "ColorTemperatureMaximumMireds",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "OptionsMask",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  },
                  {
                    name: "OptionsOverride",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "ExecuteIfOff",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            }
                    ]
                  }
              ]              
            }
        ] 
      }
    },
    IlluminanceMeasurement: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "MeasuredValue",
              type: "number",            
              max: 65535,
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 1,
              name: "MinMeasuredValue",
              type: "number",            
              max: 65533,
              min: 1,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 2,
              name: "MaxMeasuredValue",
              type: "number",            
              max: 65534,
              min: 2,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 3,
              name: "Tolerance",
              type: "number",            
              max: 2048,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 4,
              name: "LightSensorType",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 255,
              enum: [
                { name: "Photodiode", value: 0},
																{ name: "CMOS", value: 1},
																{ name: "Unknown", value: 255}
              ],
            }
        ],
        commands: [
        ] 
      }
    },
    TemperatureMeasurement: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "MeasuredValue",
              type: "number",            
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 1,
              name: "MinMeasuredValue",
              type: "number",            
              max: 32766,
              min: -27315,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 2,
              name: "MaxMeasuredValue",
              type: "number",            
              max: 32767,
              min: -27314,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 3,
              name: "Tolerance",
              type: "number",            
              max: 2048,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            }
        ],
        commands: [
        ] 
      }
    },
    PressureMeasurement: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "MeasuredValue",
              type: "number",            
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 1,
              name: "MinMeasuredValue",
              type: "number",            
              max: 32766,
              min: -32767,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 2,
              name: "MaxMeasuredValue",
              type: "number",            
              max: 32767,
              min: -32766,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 3,
              name: "Tolerance",
              type: "number",            
              max: 2048,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 16,
              name: "ScaledValue",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 17,
              name: "MinScaledValue",
              type: "number",            
              max: 32766,
              min: -32767,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 18,
              name: "MaxScaledValue",
              type: "number",            
              max: 32767,
              min: -32766,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 19,
              name: "ScaledTolerance",
              type: "number",            
              max: 2048,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 20,
              name: "Scale",
              type: "number",            
              max: 127,
              min: -127,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            }
        ],
        commands: [
        ] 
      }
    },
    FlowMeasurement: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "MeasuredValue",
              type: "number",            
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 1,
              name: "MinMeasuredValue",
              type: "number",            
              max: 65533,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2,
              name: "MaxMeasuredValue",
              type: "number",            
              max: 65534,
              min: 1,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 3,
              name: "Tolerance",
              type: "number",            
              max: 2048,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            }
        ],
        commands: [
        ] 
      }
    },
    RelativityHumidity: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "MeasuredValue",
              type: "number",            
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 1,
              name: "MinMeasuredValue",
              type: "number",            
              max: 9999,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2,
              name: "MaxMeasuredValue",
              type: "number",            
              max: 10000,
              min: 1,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 3,
              name: "Tolerance",
              type: "number",            
              max: 2048,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            }
        ],
        commands: [
        ] 
      }
    },
    OccupancySensing: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "Occupancy",
              type: "bitmap",            
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              bitmap: [
                { name: "SensedOccupancy", mask: 1, type: "boolean", offset: 0 }
              ]
            },
            {                
              id: 1,
              name: "OccupancySensorType",
              type: "enum",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: NaN,
              enum: [
                { name: "PIR", value: 0},
																{ name: "Ultrasonic", value: 1},
																{ name: "PIRAndUltrasonic", value: 2},
																{ name: "PhysicalContact", value: 3}
              ],
            },
            {                
              id: 2,
              name: "OccupancySensorTypeBitmap",
              type: "bitmap",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              bitmap: [
                { name: "PIR", mask: 1, type: "boolean", offset: 0 },
								{ name: "Ultrasonic", mask: 2, type: "boolean", offset: 1 },
								{ name: "PhysicalContact", mask: 4, type: "boolean", offset: 2 }
              ]
            },
            {                
              id: 16,
              name: "PIROccupiedToUnoccupiedDelay",
              type: "number",            
              max: 65534,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 17,
              name: "PIRUnoccupiedToOccupiedDelay",
              type: "number",            
              max: 65534,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 18,
              name: "PIRUnoccupiedToOccupiedThreshold",
              type: "number",            
              max: 254,
              min: 1,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 1,
            },
            {                
              id: 32,
              name: "UltrasonicOccupiedToUnoccupiedDelay",
              type: "number",            
              max: 65534,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 33,
              name: "UltrasonicUnoccupiedToOccupiedDelay",
              type: "number",            
              max: 65534,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 34,
              name: "UltrasonicUnoccupiedToOccupiedThreshold",
              type: "number",            
              max: 254,
              min: 1,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 1,
            },
            {                
              id: 48,
              name: "PhysicalContactOccupiedToUnoccupiedDelay",
              type: "number",            
              max: 65534,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 49,
              name: "PhysicalContactUnoccupiedToOccupiedDelay",
              type: "number",            
              max: 65534,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 50,
              name: "PhysicalContactUnoccupiedToOccupiedThreshold",
              type: "number",            
              max: 254,
              min: 1,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 1,
            }
        ],
        commands: [
        ] 
      }
    },
    SoilMoisture: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "MeasuredValue",
              type: "number",            
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 1,
              name: "MinMeasuredValue",
              type: "number",            
              max: 9999,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2,
              name: "MaxMeasuredValue",
              type: "number",            
              max: 10000,
              min: 1,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 3,
              name: "Tolerance",
              type: "number",            
              max: 2048,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            }
        ],
        commands: [
        ] 
      }
    },
    PhMeasurement: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "MeasuredValue",
              type: "number",            
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 1,
              name: "MinMeasuredValue",
              type: "number",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2,
              name: "MaxMeasuredValue",
              type: "number",            
              max: 1400,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 3,
              name: "Tolerance",
              type: "number",            
              max: 200,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            }
        ],
        commands: [
        ] 
      }
    },
    ElectricalConductivityMeasurement: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "MeasuredValue",
              type: "number",            
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 1,
              name: "MinMeasuredValue",
              type: "number",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2,
              name: "MaxMeasuredValue",
              type: "number",            
              max: 65534,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 3,
              name: "Tolerance",
              type: "number",            
              max: 100,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            }
        ],
        commands: [
        ] 
      }
    },
    WindSpeedMeasurement: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "MeasuredValue",
              type: "number",            
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 1,
              name: "MinMeasuredValue",
              type: "number",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2,
              name: "MaxMeasuredValue",
              type: "number",            
              max: 65534,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 3,
              name: "Tolerance",
              type: "number",            
              max: 776,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            }
        ],
        commands: [
        ] 
      }
    },
    CarbonMonoxide: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "MeasuredValue",
              type: "struct",            
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            },
            {                
              id: 1,
              name: "MinMeasuredValue",
              type: "struct",            
              min: 0,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            },
            {                
              id: 2,
              name: "MaxMeasuredValue",
              type: "struct",            
              max: 1,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            },
            {                
              id: 3,
              name: "Tolerance",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            }
        ],
        commands: [
        ] 
      }
    },
    CarbonDioxide: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "MeasuredValue",
              type: "struct",            
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            },
            {                
              id: 1,
              name: "MinMeasuredValue",
              type: "struct",            
              min: 0,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            },
            {                
              id: 2,
              name: "MaxMeasuredValue",
              type: "struct",            
              max: 1,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            },
            {                
              id: 3,
              name: "Tolerance",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            }
        ],
        commands: [
        ] 
      }
    },
    PM25: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "MeasuredValue",
              type: "struct",            
              required: true,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            },
            {                
              id: 1,
              name: "MinMeasuredValue",
              type: "struct",            
              min: 0,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            },
            {                
              id: 2,
              name: "MaxMeasuredValue",
              type: "struct",            
              max: 1,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            },
            {                
              id: 3,
              name: "Tolerance",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            }
        ],
        commands: [
        ] 
      }
    },
    IASZone: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "ZoneState",
              type: "enum",            
              max: 255,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              enum: [
                { name: "NotEnrolled", value: 0},
																{ name: "Enrolled", value: 1}
              ],
            },
            {                
              id: 1,
              name: "ZoneType",
              type: "enum",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              enum: [
                { name: "StandardCIE", value: 0},
																{ name: "MotionSensor", value: 13},
																{ name: "ContactSwitch", value: 21},
																{ name: "DoorOrWindowHandle", value: 22},
																{ name: "FireSensor", value: 40},
																{ name: "WaterSensor", value: 42},
																{ name: "CarbonMonoxideSensor", value: 43},
																{ name: "PersonalEmergencyDevice", value: 44},
																{ name: "VibrationOrMovementSensor", value: 45},
																{ name: "RemoteControl", value: 271},
																{ name: "KeyFob", value: 277},
																{ name: "Keypad", value: 541},
																{ name: "StandardWarningDevice", value: 549},
																{ name: "GlassBreakSensor", value: 550},
																{ name: "SecurityRepeater", value: 553},
																{ name: "Invalid", value: 65535}
              ],
            },
            {                
              id: 2,
              name: "ZoneStatus",
              type: "bitmap",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              bitmap: [
                
              ]
            },
            {                
              id: 16,
              name: "IASCIEAddress",
              type: "struct",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              struct:[
              ],
            },
            {                
              id: 17,
              name: "ZoneID",
              type: "number",            
              max: 255,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 255,
            },
            {                
              id: 18,
              name: "NumberOfZoneSensitivityLevelsSupported",
              type: "number",            
              max: 255,
              min: 2,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 2,
            },
            {                
              id: 19,
              name: "CurrentZoneSensitivityLevel",
              type: "number",            
              max: 255,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            }
        ],
        commands: [
            {
              id: 0,
              name: "ZoneEnrollResponse",
              required: true,
              fields: [
                  {
                    name: "EnrollResponseCode",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Success",
                              value: 0
                            },
                            {
                              name: "NotSupported",
                              value: 1
                            },
                            {
                              name: "NoEnrollPermit",
                              value: 2
                            },
                            {
                              name: "TooManyZones",
                              value: 3
                            }
                    ],
                  },
                  {
                    name: "ZoneID",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 0,
              name: "ZoneStatusChangeNotification",
              required: true,
              fields: [
                  {
                    name: "ZoneStatus",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                            {
                              name: "Alarm1",
                              mask: 1,
                              type: "boolean",
                              offset: 0,
                            },
                            {
                              name: "Alarm2",
                              mask: 2,
                              type: "boolean",
                              offset: 1,
                            },
                            {
                              name: "Tamper",
                              mask: 4,
                              type: "boolean",
                              offset: 2,
                            },
                            {
                              name: "BatteryLow",
                              mask: 8,
                              type: "boolean",
                              offset: 3,
                            },
                            {
                              name: "SupervisionNotify",
                              mask: 16,
                              type: "boolean",
                              offset: 4,
                            },
                            {
                              name: "RestoreNotify",
                              mask: 32,
                              type: "boolean",
                              offset: 5,
                            },
                            {
                              name: "Trouble",
                              mask: 64,
                              type: "boolean",
                              offset: 6,
                            },
                            {
                              name: "ACMainsFault",
                              mask: 128,
                              type: "boolean",
                              offset: 7,
                            },
                            {
                              name: "Test",
                              mask: 256,
                              type: "boolean",
                              offset: 8,
                            },
                            {
                              name: "BatteryDefect",
                              mask: 512,
                              type: "boolean",
                              offset: 9,
                            }
                    ]
                  },
                  {
                    name: "ExtendedStatus",
                    type: "bitmap",
                    isArray: false,
                    bitmap:[
                    ]
                  },
                  {
                    name: "ZoneID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "Delay",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 1,
              name: "InitiateNormalOperationMode",
              required: false,
              fields: [
              ]              
            },
            {
              id: 1,
              name: "ZoneEnrollRequest",
              required: true,
              fields: [
                  {
                    name: "ZoneType",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "StandardCIE",
                              value: 0
                            },
                            {
                              name: "MotionSensor",
                              value: 13
                            },
                            {
                              name: "ContactSwitch",
                              value: 21
                            },
                            {
                              name: "DoorOrWindowHandle",
                              value: 22
                            },
                            {
                              name: "FireSensor",
                              value: 40
                            },
                            {
                              name: "WaterSensor",
                              value: 42
                            },
                            {
                              name: "CarbonMonoxideSensor",
                              value: 43
                            },
                            {
                              name: "PersonalEmergencyDevice",
                              value: 44
                            },
                            {
                              name: "VibrationOrMovementSensor",
                              value: 45
                            },
                            {
                              name: "RemoteControl",
                              value: 271
                            },
                            {
                              name: "KeyFob",
                              value: 277
                            },
                            {
                              name: "Keypad",
                              value: 541
                            },
                            {
                              name: "StandardWarningDevice",
                              value: 549
                            },
                            {
                              name: "GlassBreakSensor",
                              value: 550
                            },
                            {
                              name: "SecurityRepeater",
                              value: 553
                            },
                            {
                              name: "Invalid",
                              value: 65535
                            }
                    ],
                  },
                  {
                    name: "ManufacturerCode",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 2,
              name: "InitiateTestMode",
              required: false,
              fields: [
                  {
                    name: "TestModeDuration",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "CurrentZoneSensitivityLevel",
                    type: "number",
                    isArray: false,
                  }
              ]              
            }
        ] 
      }
    },
    Metering: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "CurrentSummationDelivered",
              type: "number",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 1,
              name: "CurrentSummationReceived",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 2,
              name: "CurrentMaxDemandDelivered",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 3,
              name: "CurrentMaxDemandReceived",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 6,
              name: "PowerFactor",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 7,
              name: "ReadingSnapShotTime",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              struct:[
              ],
            },
            {                
              id: 8,
              name: "CurrentMaxDemandDeliveredTime",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              struct:[
              ],
            },
            {                
              id: 9,
              name: "CurrentMaxDemandReceivedTime",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              struct:[
              ],
            },
            {                
              id: 10,
              name: "DefaultUpdatePeriod",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: NaN,
            },
            {                
              id: 20,
              name: "SupplyStatus",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              enum: [
                { name: "SupplyOFF", value: 0},
																{ name: "SupplyOFFOrARMED", value: 1},
																{ name: "SupplyON", value: 2}
              ],
            },
            {                
              id: 21,
              name: "CurrentInletEnergyCarrierSummation",
              type: "number",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 22,
              name: "CurrentOutletEnergyCarrierSummation",
              type: "number",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 23,
              name: "InletTemperature",
              type: "struct",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            },
            {                
              id: 24,
              name: "OutletTemperature",
              type: "struct",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            },
            {                
              id: 768,
              name: "UnitofMeasure",
              type: "enum",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              enum: [
                { name: "KilowattHoursOrKilowatts", value: 0},
																{ name: "CubicMeterOrCubicMeterPerHour", value: 1},
																{ name: "CubicFeetOrCubicFeetPerHour", value: 2},
																{ name: "CentumCubicFeetOrCentum", value: 3},
																{ name: "USGallonsOrUSGalonsPerHour", value: 4},
																{ name: "ImperialGallonsOrImperialGallonsPerhour", value: 5},
																{ name: "BTUsOrBTUPerhour", value: 6},
																{ name: "LitersOrLitersPerHour", value: 7},
																{ name: "kPAGaugein", value: 8},
																{ name: "kPAabsolute", value: 9},
																{ name: "mcfCubicFeetOr mcfPerHour", value: 10},
																{ name: "Unitless", value: 11},
																{ name: "MegaJouleMegaJoulePerSecond", value: 12}
              ],
            },
            {                
              id: 769,
              name: "Multiplier",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            },
            {                
              id: 770,
              name: "Divisor",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
              ],
            },
            {                
              id: 771,
              name: "SummationFormatting",
              type: "bitmap",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              bitmap: [
                { name: "SuppressLeadingZeros", mask: 1, type: "boolean", offset: 0 },
								{ name: "NumberOfDigitsToTheRightOfDecimalPoint", mask: 224, type: "boolean", offset: 5 },
								{ name: "NumberOfDigitsToTheLeftOfTheDecimalPoint", mask: 30, type: "boolean", offset: 1 }
              ]
            },
            {                
              id: 772,
              name: "DemandFormatting",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              bitmap: [
                { name: "SuppressLeadingZeros", mask: 1, type: "boolean", offset: 0 },
								{ name: "NumberOfDigitsToTheRightOfDecimalPoint", mask: 224, type: "boolean", offset: 5 },
								{ name: "NumberOfDigitsToTheLeftOfTheDecimalPoint", mask: 30, type: "boolean", offset: 1 }
              ]
            },
            {                
              id: 773,
              name: "HistoricalConsumptionFormatting",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              bitmap: [
                { name: "SuppressLeadingZeros", mask: 1, type: "boolean", offset: 0 },
								{ name: "NumberOfDigitsToTheRightOfDecimalPoint", mask: 224, type: "boolean", offset: 5 },
								{ name: "NumberOfDigitsToTheLeftOfTheDecimalPoint", mask: 30, type: "boolean", offset: 1 }
              ]
            },
            {                
              id: 774,
              name: "MeteringDeviceType",
              type: "enum",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              enum: [
                { name: "ElectricMetering", value: 0},
																{ name: "GasMetering", value: 1},
																{ name: "WaterMetering", value: 2},
																{ name: "ThermalMetering", value: 3},
																{ name: "PressureMetering", value: 4},
																{ name: "HeatMetering", value: 5},
																{ name: "CoolingMetering", value: 6},
																{ name: "MirroredGasMetering", value: 128},
																{ name: "MirroredWaterMetering", value: 129},
																{ name: "MirroredThermalMetering", value: 130},
																{ name: "MirroredPressureMetering", value: 131},
																{ name: "MirroredHeatMetering", value: 132},
																{ name: "MirroredCoolingMetering", value: 133}
              ],
            },
            {                
              id: 777,
              name: "EnergyCarrierUnitOfMeasure",
              type: "enum",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              enum: [
                { name: "KilowattHoursOrKilowatts", value: 0},
																{ name: "CubicMeterOrCubicMeterPerHour", value: 1},
																{ name: "CubicFeetOrCubicFeetPerHour", value: 2},
																{ name: "CentumCubicFeetOrCentum", value: 3},
																{ name: "USGallonsOrUSGalonsPerHour", value: 4},
																{ name: "ImperialGallonsOrImperialGallonsPerhour", value: 5},
																{ name: "BTUsOrBTUPerhour", value: 6},
																{ name: "LitersOrLitersPerHour", value: 7},
																{ name: "kPAGaugein", value: 8},
																{ name: "kPAabsolute", value: 9},
																{ name: "mcfCubicFeetOr mcfPerHour", value: 10},
																{ name: "Unitless", value: 11},
																{ name: "MegaJouleMegaJoulePerSecond", value: 12}
              ],
            },
            {                
              id: 778,
              name: "EnergyCarrierSummationFormatting",
              type: "bitmap",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              bitmap: [
                { name: "SuppressLeadingZeros", mask: 1, type: "boolean", offset: 0 },
								{ name: "NumberOfDigitsToTheRightOfDecimalPoint", mask: 224, type: "boolean", offset: 5 },
								{ name: "NumberOfDigitsToTheLeftOfTheDecimalPoint", mask: 30, type: "boolean", offset: 1 }
              ]
            },
            {                
              id: 779,
              name: "EnergyCarrierDemandFormatting",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              bitmap: [
                { name: "SuppressLeadingZeros", mask: 1, type: "boolean", offset: 0 },
								{ name: "NumberOfDigitsToTheRightOfDecimalPoint", mask: 224, type: "boolean", offset: 5 },
								{ name: "NumberOfDigitsToTheLeftOfTheDecimalPoint", mask: 30, type: "boolean", offset: 1 }
              ]
            },
            {                
              id: 780,
              name: "TemperatureUnitOfMeasure",
              type: "enum",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              enum: [
                { name: "DegreesKelvin", value: 0},
																{ name: "DegreesCelsius", value: 1},
																{ name: "DegreesFahrenheit", value: 2}
              ],
            },
            {                
              id: 781,
              name: "TemperatureFormatting",
              type: "bitmap",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              bitmap: [
                { name: "SuppressLeadingZeros", mask: 1, type: "boolean", offset: 0 },
								{ name: "NumberOfDigitsToTheRightOfDecimalPoint", mask: 224, type: "boolean", offset: 5 },
								{ name: "NumberOfDigitsToTheLeftOfTheDecimalPoint", mask: 30, type: "boolean", offset: 1 }
              ]
            }
        ],
        commands: [
        ] 
      }
    },
    ElectricalMeasurement: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "MeasurementType",
              type: "bitmap",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "ActiveMeasurementAC", mask: 1, type: "boolean", offset: 0 },
								{ name: "ReactiveMeasurementAC", mask: 2, type: "boolean", offset: 1 },
								{ name: "ApparentMeasurementAC", mask: 4, type: "boolean", offset: 2 },
								{ name: "PhaseAMeasurement", mask: 8, type: "boolean", offset: 3 },
								{ name: "PhaseBMeasurement", mask: 16, type: "boolean", offset: 4 },
								{ name: "PhaseCMeasurement", mask: 32, type: "boolean", offset: 5 },
								{ name: "DCMeasurement", mask: 64, type: "boolean", offset: 6 },
								{ name: "HarmonicsMeasurement", mask: 128, type: "boolean", offset: 7 },
								{ name: "PowerQualityMeasurement", mask: 256, type: "boolean", offset: 8 }
              ]
            },
            {                
              id: 256,
              name: "DCVoltage",
              type: "number",            
              min: -32767,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 257,
              name: "DCVoltageMin",
              type: "number",            
              min: -32767,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 258,
              name: "DCVoltageMax",
              type: "number",            
              min: -32767,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 259,
              name: "DCCurrent",
              type: "number",            
              min: -32767,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 260,
              name: "DCCurrentMin",
              type: "number",            
              min: -32767,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 261,
              name: "DCCurrentMax",
              type: "number",            
              min: -32767,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 262,
              name: "DCPower",
              type: "number",            
              min: -32767,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 263,
              name: "DCPowerMin",
              type: "number",            
              min: -32767,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 264,
              name: "DCPowerMax",
              type: "number",            
              min: -32767,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 512,
              name: "DCVoltageMultiplier",
              type: "number",            
              min: 1,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 513,
              name: "DCVoltageDivisor",
              type: "number",            
              min: 1,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 514,
              name: "DCCurrentMultiplier",
              type: "number",            
              min: 1,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 515,
              name: "DCCurrentDivisor",
              type: "number",            
              min: 1,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 516,
              name: "DCPowerMultiplier",
              type: "number",            
              min: 1,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 517,
              name: "DCPowerDivisor",
              type: "number",            
              min: 1,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 768,
              name: "ACFrequency",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 769,
              name: "ACFrequencyMin",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 770,
              name: "ACFrequencyMax",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 771,
              name: "NeutralCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 772,
              name: "TotalActivePower",
              type: "number",            
              max: 8388607,
              min: -8388607,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 773,
              name: "TotalReactivePower",
              type: "number",            
              max: 8388607,
              min: -8388607,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 774,
              name: "TotalApparentPower",
              type: "number",            
              max: 16777215,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 775,
              name: "Measured1stHarmonicCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 776,
              name: "Measured3rdHarmonicCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 777,
              name: "Measured5thHarmonicCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 778,
              name: "Measured7thHarmonicCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 779,
              name: "Measured9thHarmonicCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 780,
              name: "Measured11thHarmonicCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 781,
              name: "MeasuredPhase1stHarmonicCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 782,
              name: "MeasuredPhase3rdHarmonicCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 783,
              name: "MeasuredPhase5thHarmonicCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 784,
              name: "MeasuredPhase7thHarmonicCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 785,
              name: "MeasuredPhase9thHarmonicCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 786,
              name: "MeasuredPhase11thHarmonicCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 1024,
              name: "ACFrequencyMultiplier",
              type: "number",            
              min: 1,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 1025,
              name: "ACFrequencyDivisor",
              type: "number",            
              min: 1,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 1026,
              name: "PowerMultiplier",
              type: "number",            
              max: 16777215,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 1027,
              name: "PowerDivisor",
              type: "number",            
              max: 16777215,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 1028,
              name: "HarmonicCurrentMultiplier",
              type: "number",            
              min: -127,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 1029,
              name: "PhaseHarmonicCurrentMultiplier",
              type: "number",            
              min: -127,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 1281,
              name: "LineCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 1282,
              name: "ActiveCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 1283,
              name: "ReactiveCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 1285,
              name: "RMSVoltage",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 1286,
              name: "RMSVoltageMin",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 1287,
              name: "RMSVoltageMax",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 1288,
              name: "RMSCurrent",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 1289,
              name: "RMSCurrentMin",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 1290,
              name: "RMSCurrentMax",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 1291,
              name: "ActivePower",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 1292,
              name: "ActivePowerMin",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 1293,
              name: "ActivePowerMax",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 1294,
              name: "ReactivePower",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 1295,
              name: "ApparentPower",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 1296,
              name: "PowerFactor",
              type: "number",            
              max: 100,
              min: -100,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 1297,
              name: "AverageRMSVoltageMeasurementPeriod",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 1298,
              name: "AverageRMSOverVoltageCounter",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 1299,
              name: "AverageRMSUnderVoltageCounter",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 1300,
              name: "RMSExtremeOverVoltagePeriod",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 1301,
              name: "RMSExtremeUnderVoltagePeriod",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 1302,
              name: "RMSVoltageSagPeriod",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 1303,
              name: "RMSVoltageSwellPeriod",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 1536,
              name: "ACVoltageMultiplier",
              type: "number",            
              min: 1,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 1537,
              name: "ACVoltageDivisor",
              type: "number",            
              min: 1,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 1538,
              name: "ACCurrentMultiplier",
              type: "number",            
              min: 1,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 1539,
              name: "ACCurrentDivisor",
              type: "number",            
              min: 1,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 1540,
              name: "ACPowerMultiplier",
              type: "number",            
              min: 1,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 1541,
              name: "ACPowerDivisor",
              type: "number",            
              min: 1,
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 1,
            },
            {                
              id: 1792,
              name: "DCOverloadAlarmsMask",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "VoltageOverload", mask: 1, type: "boolean", offset: 0 },
								{ name: "CurrentOverload", mask: 2, type: "boolean", offset: 1 }
              ]
            },
            {                
              id: 1793,
              name: "DCVoltageOverload",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -1,
            },
            {                
              id: 1794,
              name: "DCCurrentOverload",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -1,
            },
            {                
              id: 2048,
              name: "ACAlarmsMask",
              type: "bitmap",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              bitmap: [
                { name: "VoltageOverload", mask: 1, type: "boolean", offset: 0 },
								{ name: "CurrentOverload", mask: 2, type: "boolean", offset: 1 },
								{ name: "ActivePowerOverload", mask: 4, type: "boolean", offset: 2 },
								{ name: "ReactivePowerOverload", mask: 8, type: "boolean", offset: 3 },
								{ name: "AverageRMSOverVoltage", mask: 16, type: "boolean", offset: 4 },
								{ name: "AverageRMSUnderVoltage", mask: 32, type: "boolean", offset: 5 },
								{ name: "RMSExtremeOverVoltage", mask: 64, type: "boolean", offset: 6 },
								{ name: "RMSExtremeUnderVoltage", mask: 128, type: "boolean", offset: 7 },
								{ name: "RMSVoltageSag", mask: 256, type: "boolean", offset: 8 },
								{ name: "RMSVoltageSwell", mask: 512, type: "boolean", offset: 9 }
              ]
            },
            {                
              id: 2049,
              name: "ACVoltageOverload",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -1,
            },
            {                
              id: 2050,
              name: "ACCurrentOverload",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -1,
            },
            {                
              id: 2051,
              name: "ACActivePowerOverload",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -1,
            },
            {                
              id: 2052,
              name: "ACReactivePowerOverload",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -1,
            },
            {                
              id: 2053,
              name: "AverageRMSOverVoltage",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 2054,
              name: "AverageRMSUnderVoltage",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 2055,
              name: "RMSExtremeOverVoltage",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 2056,
              name: "RMSExtremeUnderVoltage",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 2057,
              name: "RMSVoltageSag",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 2058,
              name: "RMSVoltageSwell",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 2305,
              name: "LineCurrentPhB",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2306,
              name: "ActiveCurrentPhB",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 2307,
              name: "ReactiveCurrentPhB",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 2309,
              name: "RMSVoltagePhB",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2310,
              name: "RMSVoltageMinPhB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 32768,
            },
            {                
              id: 2311,
              name: "RMSVoltageMaxPhB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 32768,
            },
            {                
              id: 2312,
              name: "RMSCurrentPhB",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2313,
              name: "RMSCurrentMinPhB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2314,
              name: "RMSCurrentMaxPhB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2315,
              name: "ActivePowerPhB",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 2316,
              name: "ActivePowerMinPhB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 2317,
              name: "ActivePowerMaxPhB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 2318,
              name: "ReactivePowerPhB",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 2319,
              name: "ApparentPowerPhB",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2320,
              name: "PowerFactorPhB",
              type: "number",            
              max: 100,
              min: -100,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 2321,
              name: "AverageRMSVoltageMeasurementPeriodPhB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 2322,
              name: "AverageRMSOverVoltageCounterPhB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 2323,
              name: "AverageRMSUnderVoltageCounterPhB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 2324,
              name: "RMSExtremeOverVoltagePeriodPhB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 2325,
              name: "RMSExtremeUnderVoltagePeriodPhB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 2326,
              name: "RMSVoltageSagPeriodPhB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 2327,
              name: "RMSVoltageSwellPeriodPhB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 2561,
              name: "LineCurrentPhC",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2562,
              name: "ActiveCurrentPhC",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 2563,
              name: "ReactiveCurrentPhC",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 2565,
              name: "RMSVoltagePhC",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2566,
              name: "RMSVoltageMinPhC",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 32768,
            },
            {                
              id: 2567,
              name: "RMSVoltageMaxPhC",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 32768,
            },
            {                
              id: 2568,
              name: "RMSCurrentPhC",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2569,
              name: "RMSCurrentMinPhC",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2570,
              name: "RMSCurrentMaxPhC",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2571,
              name: "ActivePowerPhC",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 2572,
              name: "ActivePowerMinPhC",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 2573,
              name: "ActivePowerMaxPhC",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 2574,
              name: "ReactivePowerPhC",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: -32768,
            },
            {                
              id: 2575,
              name: "ApparentPowerPhC",
              type: "number",            
              required: false,
              reportRequired: true,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 65535,
            },
            {                
              id: 2576,
              name: "PowerFactorPhC",
              type: "number",            
              max: 100,
              min: -100,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 2577,
              name: "AverageRMSVoltageMeasurementPeriodPhC",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 2578,
              name: "AverageRMSOverVoltageCounterPhC",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 2579,
              name: "AverageRMSUnderVoltageCounterPhC",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 2580,
              name: "RMSExtremeOverVoltagePeriodPhC",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 2581,
              name: "RMSExtremeUnderVoltagePeriodPhC",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 2582,
              name: "RMSVoltageSagPeriodPhC",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 2583,
              name: "RMSVoltageSwellPeriodPhC",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            }
        ],
        commands: [
            {
              id: 0,
              name: "GetProfileInfoResponse",
              required: false,
              fields: [
                  {
                    name: "ProfileCount",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "ProfileIntervalPeriod",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Daily",
                              value: 0
                            },
                            {
                              name: "60Minutes",
                              value: 1
                            },
                            {
                              name: "30Minutes",
                              value: 2
                            },
                            {
                              name: "15Minutes",
                              value: 3
                            },
                            {
                              name: "10Minutes",
                              value: 4
                            },
                            {
                              name: "7dot5Minutes",
                              value: 5
                            },
                            {
                              name: "5Minutes",
                              value: 6
                            },
                            {
                              name: "2dot5Minutes",
                              value: 7
                            }
                    ],
                  },
                  {
                    name: "MaxNumberOfIntervals",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "ListOfAttributes",
                    type: "number",
                    isArray: true,
                  }
              ]              
            },
            {
              id: 0,
              name: "GetProfileInfo",
              required: false,
              fields: [
              ]              
            },
            {
              id: 1,
              name: "GetMeasurementProfileResponse",
              required: false,
              fields: [
                  {
                    name: "StartTime",
                    type: "struct",
                    isArray: false,
                    struct:[
                    ],
                  },
                  {
                    name: "Status",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Success",
                              value: 0
                            },
                            {
                              name: "AttributeProfileNotSupported",
                              value: 1
                            },
                            {
                              name: "InvalidStartTime",
                              value: 2
                            },
                            {
                              name: "MoreIntervalsRequestedThanCanBeReturned",
                              value: 3
                            },
                            {
                              name: "NoIntervalsAvailableForTheRequestedTime",
                              value: 4
                            }
                    ],
                  },
                  {
                    name: "ProfileIntervalPeriod",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Daily",
                              value: 0
                            },
                            {
                              name: "60Minutes",
                              value: 1
                            },
                            {
                              name: "30Minutes",
                              value: 2
                            },
                            {
                              name: "15Minutes",
                              value: 3
                            },
                            {
                              name: "10Minutes",
                              value: 4
                            },
                            {
                              name: "7dot5Minutes",
                              value: 5
                            },
                            {
                              name: "5Minutes",
                              value: 6
                            },
                            {
                              name: "2dot5Minutes",
                              value: 7
                            }
                    ],
                  },
                  {
                    name: "NumberOfIntervalsDelivered",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "AttributeId",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "Intervals",
                    type: "number",
                    isArray: true,
                  }
              ]              
            },
            {
              id: 1,
              name: "GetMeasurementProfile",
              required: false,
              fields: [
                  {
                    name: "AttributeID",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "StartTime",
                    type: "struct",
                    isArray: false,
                    struct:[
                    ],
                  },
                  {
                    name: "NumberOfIntervals",
                    type: "number",
                    isArray: false,
                  }
              ]              
            }
        ] 
      }
    },
    RFTelemetry: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "TxReportEnabled",
              type: "boolean",            
              max: 1,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 1,
              name: "PTIEnabled",
              type: "boolean",            
              max: 1,
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            }
        ],
        commands: [
            {
              id: 1,
              name: "TxReport",
              required: false,
              fields: [
                  {
                    name: "SourceUNID",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "DestinationUNID",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "TransmissionSuccessful",
                    type: "boolean",
                    isArray: false,
                  },
                  {
                    name: "TransmissionTimeMs",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TxPowerdBm",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "TxChannel",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "RoutingAttempts",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "RouteChanged",
                    type: "boolean",
                    isArray: false,
                  },
                  {
                    name: "TransmissionSpeed",
                    type: "enum",
                    isArray: false,
                    enum:[
                            {
                              name: "Z-Wave-9600-bits-per-second",
                              value: 0
                            },
                            {
                              name: "Z-Wave-40-kbits-per-second",
                              value: 1
                            },
                            {
                              name: "Z-Wave-100-kbits-per-second",
                              value: 2
                            },
                            {
                              name: "Z-WaveLongRange-100-kbits-per-second",
                              value: 3
                            },
                            {
                              name: "Other",
                              value: 254
                            },
                            {
                              name: "Unknown",
                              value: 255
                            }
                    ],
                  },
                  {
                    name: "MeasuredNoiseFloordBm",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "LastRouteRepeaters",
                    type: "text",
                    isArray: true,
                  },
                  {
                    name: "IncomingRSSIRepeaters",
                    type: "number",
                    isArray: true,
                  },
                  {
                    name: "AckRSSI",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "AckChannel",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "LastRouteFailedLinkFunctionalUNID",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "LastRouteFailedLinkNonFunctionalUNID",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "DestinationAckTxPowerdBm",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "DestinationAckMeasuredRSSI",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "DestinationAckMeasuredNoiseFloor",
                    type: "number",
                    isArray: false,
                  }
              ]              
            }
        ] 
      }
    },
    Binding: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "BindingTableFull",
              type: "boolean",            
              max: 1,
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              default: 0,
            },
            {                
              id: 1,
              name: "BindableClusterList",
              type: "text",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: true,
            },
            {                
              id: 2,
              name: "BindingTable",
              type: "struct",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: true,
              default: 0,
              struct:[
                {
                  name:"ClusterName",
                  type:"text",
                  isArray: false,
                },
                {
                  name:"DestinationUnid",
                  type:"text",
                  isArray: false,
                },
                {
                  name:"DestinationEp",
                  type:"number",
                  isArray: false,
                }
              ],
            }
        ],
        commands: [
            {
              id: 1,
              name: "Bind",
              required: true,
              fields: [
                  {
                    name: "ClusterName",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "DestinationUnid",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "DestinationEp",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 2,
              name: "Unbind",
              required: true,
              fields: [
                  {
                    name: "ClusterName",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "DestinationUnid",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "DestinationEp",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 3,
              name: "BindToProtocolController",
              required: true,
              fields: [
                  {
                    name: "ClusterName",
                    type: "text",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 4,
              name: "UnbindFromProtocolController",
              required: true,
              fields: [
                  {
                    name: "ClusterName",
                    type: "text",
                    isArray: false,
                  }
              ]              
            }
        ] 
      }
    },
    SystemMetrics: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "ReportingIntervalSeconds",
              type: "number",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 16,
              name: "CPUUsagePercent",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: true,
            },
            {                
              id: 17,
              name: "CPUFrequencyMHz",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: true,
            },
            {                
              id: 18,
              name: "CPUAverageUsagePercent",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: true,
            },
            {                
              id: 19,
              name: "CPUMinUsagePercent",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: true,
            },
            {                
              id: 20,
              name: "CPUMaxUsagePercent",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: true,
            },
            {                
              id: 32,
              name: "RAMTotalMB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 33,
              name: "RAMFreeMB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 34,
              name: "RAMAvailableMB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 35,
              name: "SWAPMemoryTotalMB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 36,
              name: "SWAPMemoryUsedMB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 37,
              name: "VirtualMemoryTotalMB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 38,
              name: "VirtualMemoryUsedMB",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 39,
              name: "DisksUsage",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: true,
              struct:[
                {
                  name:"UUID",
                  type:"text",
                  isArray: false,
                },
                {
                  name:"UsagePercent",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"FreeSpaceMB",
                  type:"number",
                  isArray: false,
                }
              ],
            },
            {                
              id: 40,
              name: "DisksCounters",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: true,
              struct:[
                {
                  name:"UUID",
                  type:"text",
                  isArray: false,
                },
                {
                  name:"ReadCount",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"WriteCount",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"ReadBytes",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"WriteBytes",
                  type:"number",
                  isArray: false,
                }
              ],
            },
            {                
              id: 48,
              name: "NetworkInterfacesData",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: true,
              struct:[
                {
                  name:"InterfaceName",
                  type:"text",
                  isArray: false,
                },
                {
                  name:"BytesSent",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"BytesReceived",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"PacketsSent",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"PacketsReceived",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"ErrorsIn",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"ErrorsOut",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"DropIn",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"DropOut",
                  type:"number",
                  isArray: false,
                }
              ],
            },
            {                
              id: 49,
              name: "WirelessNetworkInterfacesData",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: true,
              struct:[
                {
                  name:"InterfaceName",
                  type:"text",
                  isArray: false,
                },
                {
                  name:"SignalStrengthdBm",
                  type:"number",
                  isArray: false,
                }
              ],
            },
            {                
              id: 50,
              name: "Hostname",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 51,
              name: "FQDN",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 64,
              name: "UptimeMinutes",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 80,
              name: "CurrentTemperatureCelcius",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 81,
              name: "AverageTemperatureCelcius",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 82,
              name: "MinTemperatureCelcius",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 83,
              name: "MaxTemperatureCelcius",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 96,
              name: "PowerPlugged",
              type: "boolean",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 97,
              name: "BatteryPercentage",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            },
            {                
              id: 112,
              name: "SystemInterrupts",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
            }
        ],
        commands: [
        ] 
      }
    },
    NameAndLocation: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "Name",
              type: "text",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 1,
              name: "Location",
              type: "text",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            }
        ],
        commands: [
        ] 
      }
    },
    ConfigurationParameters: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "ConfigurationParameters",
              type: "struct",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: true,
              struct:[
                {
                  name:"ParameterId",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"Value",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"Name",
                  type:"text",
                  isArray: false,
                },
                {
                  name:"Info",
                  type:"text",
                  isArray: false,
                },
                {
                  name:"MinimumValue",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"MaximumValue",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"DefaultValue",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"DisplayFormat",
                  type:"enum",
                  isArray: false,
                    enum:[
                      
                    ],
                },
                {
                  name:"ReadOnly",
                  type:"boolean",
                  isArray: false,
                },
                {
                  name:"Advanced",
                  type:"boolean",
                  isArray: false,
                },
                {
                  name:"AlteringCapabilities",
                  type:"boolean",
                  isArray: false,
                }
              ],
            }
        ],
        commands: [
            {
              id: 1,
              name: "DiscoverParameter",
              required: false,
              fields: [
                  {
                    name: "ParameterId",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 2,
              name: "DefaultResetAllParameters",
              required: false,
              fields: [
              ]              
            },
            {
              id: 3,
              name: "SetParameter",
              required: false,
              fields: [
                  {
                    name: "ParameterId",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "Value",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 4,
              name: "DiscoverParameterRange",
              required: false,
              fields: [
                  {
                    name: "FirstParameterId",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "LastParameterId",
                    type: "number",
                    isArray: false,
                  }
              ]              
            }
        ] 
      }
    },
    AoXLocator: {
      server:{
        attributes: [
            {                
              id: 1,
              name: "ReportingMode",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 1,
              enum: [
                { name: "IQReport", value: 0},
																{ name: "AngleReport", value: 1}
              ],
            },
            {                
              id: 2,
              name: "PositionAndOrientationValid",
              type: "boolean",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
            },
            {                
              id: 3,
              name: "PositionAndOrientation",
              type: "struct",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              struct:[
                {
                  name:"CoordinateX",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"CoordinateY",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"CoordinateZ",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"OrientationX",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"OrientationY",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"OrientationZ",
                  type:"number",
                  isArray: false,
                }
              ],
            },
            {                
              id: 5,
              name: "AzimuthMask",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: true,
              struct:[
                {
                  name:"Min",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"Max",
                  type:"number",
                  isArray: false,
                }
              ],
            },
            {                
              id: 6,
              name: "ElevationMask",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: true,
              struct:[
                {
                  name:"Min",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"Max",
                  type:"number",
                  isArray: false,
                }
              ],
            },
            {                
              id: 7,
              name: "AllowList",
              type: "text",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: true,
            },
            {                
              id: 8,
              name: "AoXMode",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: NaN,
              enum: [
                { name: "One Shot Basic", value: 3},
																{ name: "One Shot Basic Lightweight", value: 4},
																{ name: "One Shot Fast Response", value: 5},
																{ name: "One Shot High Accuracy", value: 6},
																{ name: "One Shot Basic Azimuth Only", value: 7},
																{ name: "One Shot Fast Response Azimuth Only", value: 8},
																{ name: "One Shot High Accuracy Azimuth Only", value: 9},
																{ name: "Real Time Fast Response", value: 10},
																{ name: "Real Time Basic", value: 11},
																{ name: "Real Time High Accuracy", value: 12}
              ],
            },
            {                
              id: 9,
              name: "AntennaMode",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              enum: [
                { name: "Array 4x4 URA", value: 0},
																{ name: "Array 3x3 URA", value: 1},
																{ name: "Array 1x4 ULA", value: 2},
																{ name: "Array 4x4 DP URA", value: 3},
																{ name: "Array COREHW_15x15 DP", value: 4},
																{ name: "Array COREHW 12x12 DP", value: 5}
              ],
            },
            {                
              id: 10,
              name: "AntennaArray",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: true,
            },
            {                
              id: 11,
              name: "PeriodSamples",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 12,
              name: "AngleFiltering",
              type: "boolean",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: NaN,
            },
            {                
              id: 13,
              name: "AngleFilteringWeight",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 14,
              name: "AngleCorrectionTimeout",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 15,
              name: "AngleCorrectionDelay",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 16,
              name: "CTEMode",
              type: "enum",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
              default: 0,
              enum: [
                { name: "Silicon Labs", value: 0},
																{ name: "Connection", value: 1},
																{ name: "Connectionless", value: 2}
              ],
            },
            {                
              id: 17,
              name: "CTESamplingInterval",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 18,
              name: "CTELength",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            },
            {                
              id: 19,
              name: "SlotDuration",
              type: "number",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: true,
              isArray: false,
            }
        ],
        commands: [
            {
              id: 1,
              name: "IQReport",
              required: false,
              fields: [
                  {
                    name: "TagUnid",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "Channel",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "RSSI",
                    type: "number",
                    isArray: false,
                  },
                  {
                    name: "Samples",
                    type: "number",
                    isArray: true,
                  },
                  {
                    name: "Sequence",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 2,
              name: "AngleReport",
              required: false,
              fields: [
                  {
                    name: "TagUnid",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "Direction",
                    type: "struct",
                    isArray: false,
                    struct:[
                      {
                        name:"Azimuth",
                        type:"number",
                        isArray: false
                      },
                      {
                        name:"Elevation",
                        type:"number",
                        isArray: false
                      },
                      {
                        name:"Distance",
                        type:"number",
                        isArray: false
                      }
                    ],
                  },
                  {
                    name: "Deviation",
                    type: "struct",
                    isArray: false,
                    struct:[
                      {
                        name:"Azimuth",
                        type:"number",
                        isArray: false
                      },
                      {
                        name:"Elevation",
                        type:"number",
                        isArray: false
                      },
                      {
                        name:"Distance",
                        type:"number",
                        isArray: false
                      }
                    ],
                  },
                  {
                    name: "Sequence",
                    type: "number",
                    isArray: false,
                  }
              ]              
            },
            {
              id: 3,
              name: "AngleCorrection",
              required: false,
              fields: [
                  {
                    name: "TagUnid",
                    type: "text",
                    isArray: false,
                  },
                  {
                    name: "Direction",
                    type: "struct",
                    isArray: false,
                    struct:[
                      {
                        name:"Azimuth",
                        type:"number",
                        isArray: false
                      },
                      {
                        name:"Elevation",
                        type:"number",
                        isArray: false
                      },
                      {
                        name:"Distance",
                        type:"number",
                        isArray: false
                      }
                    ],
                  },
                  {
                    name: "Deviation",
                    type: "struct",
                    isArray: false,
                    struct:[
                      {
                        name:"Azimuth",
                        type:"number",
                        isArray: false
                      },
                      {
                        name:"Elevation",
                        type:"number",
                        isArray: false
                      },
                      {
                        name:"Distance",
                        type:"number",
                        isArray: false
                      }
                    ],
                  },
                  {
                    name: "Sequence",
                    type: "number",
                    isArray: false,
                  }
              ]              
            }
        ] 
      }
    },
    AoXPositionEstimation: {
      server:{
        attributes: [
            {                
              id: 1,
              name: "Position",
              type: "struct",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
                {
                  name:"ApplicationId",
                  type:"text",
                  isArray: false,
                },
                {
                  name:"X",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"Y",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"Z",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"DeviationX",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"DeviationY",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"DeviationZ",
                  type:"number",
                  isArray: false,
                },
                {
                  name:"Sequence",
                  type:"number",
                  isArray: false,
                }
              ],
            }
        ],
        commands: [
        ] 
      }
    },
    NetworkManagement: {
      server:{
        attributes: [
            {                
              id: 1,
              name: "NetworkManagementState",
              type: "struct",            
              required: false,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
                {
                  name:"State",
                  type:"enum",
                  isArray: false,
                    enum:[
                      { name: "idle", value: 0},
																{ name: "add node", value: 1},
																{ name: "remove node", value: 2},
																{ name: "join network", value: 3},
																{ name: "leave network", value: 4},
																{ name: "network repair", value: 5},
																{ name: "network update", value: 6},
																{ name: "reset", value: 7},
																{ name: "scan mode", value: 8}
                    ],
                },
                {
                  name:"SupportedStateList",
                  type:"text",
                  isArray: true,
                },
                {
                  name:"StateParameters",
                  type:"struct",
                  isArray: false,
                    struct:[
                      {
                        name:"ProvisioningMode",
                        type:"text",
                        isArray: false,
                      },
                      {
                        name:"UserAccept",
                        type:"boolean",
                        isArray: false,
                      },
                      {
                        name:"SecurityCode",
                        type:"text",
                        isArray: false,
                      },
                      {
                        name:"Unid",
                        type:"text",
                        isArray: false,
                      },
                      {
                        name:"AllowMultipleInclusions",
                        type:"boolean",
                        isArray: false,
                      }
                    ],
                },
                {
                  name:"RequestedStateParameters",
                  type:"text",
                  isArray: true,
                }
              ],
            }
        ],
        commands: [
            {
              id: 1,
              name: "Write",
              required: false,
              fields: [
              ]              
            }
        ] 
      }
    },
    Descriptor: {
      server:{
        attributes: [
            {                
              id: 0,
              name: "DeviceTypeList",
              type: "struct",            
              required: true,
              reportRequired: false,
              sceneRequired: false,
              writable: false,
              isArray: false,
              struct:[
                {
                  name:"DeviceType",
                  type:"enum",
                  isArray: false,
                    enum:[
                      { name: "DoorLock", value: 10},
																{ name: "DoorLockController", value: 11},
																{ name: "Fan", value: 43},
																{ name: "AirPurifier", value: 45},
																{ name: "RoomAirConditioner", value: 114},
																{ name: "Cooktop", value: 120},
																{ name: "ExtractorHood", value: 122},
																{ name: "OnOffLight", value: 256},
																{ name: "DimmableLight", value: 257},
																{ name: "OnOffLightSwitch", value: 259},
																{ name: "DimmerSwitch", value: 260},
																{ name: "ColorDimmerSwitch", value: 261},
																{ name: "LightSensor", value: 262},
																{ name: "OccupancySensor", value: 263},
																{ name: "OnOffPluginUnit", value: 266},
																{ name: "DimmablePlugInUnit", value: 267},
																{ name: "ColorTemperatureLight", value: 268},
																{ name: "ExtendedColorLight", value: 269},
																{ name: "WindowCovering", value: 514},
																{ name: "WindowCoveringController", value: 515},
																{ name: "HeatingCoolingUnit", value: 768},
																{ name: "Thermostat", value: 769},
																{ name: "TemperatureSensor", value: 770},
																{ name: "Pump", value: 771},
																{ name: "PressureSensor", value: 773},
																{ name: "FlowSensor", value: 774},
																{ name: "HumiditySensor", value: 775},
																{ name: "ControlBridge", value: 2112},
																{ name: "OnOffSensor", value: 2128}
                    ],
                },
                {
                  name:"Revision",
                  type:"number",
                  isArray: false,
                }
              ],
            }
        ],
        commands: [
        ] 
      }
    },
}
