const supportedClusters = [
    "AoXLocator",
    "AoXPositionEstimation",
    "BarrierControl",
    "Basic",
    "Binding",
    "ColorControl",
    "CarbonDioxide",
    "CarbonMonoxide",
    "ConfigurationParameters",
    "Descriptor",
    "DoorLock",
    "ElectricalConductivityMeasurement",
    "ElectricalMeasurement",
    "FlowMeasurement",
    "IASZone",
    "Identify",
    "IlluminanceMeasurement",
    "Level",
    "Metering",
    "NameAndLocation",
    "OccupancySensing",
    "OnOff",
    "PhMeasurement",
    "PM25",
    "PowerConfiguration",
    "PressureMeasurement",
    "ProtocolController-NetworkManagement",
    "ProtocolController-RFTelemetry",
    "RelativityHumidity",
    "Scenes",
    "SoilMoisture",
    "SystemMetrics",
    "TemperatureMeasurement",
    "Thermostat",
    "WindSpeedMeasurement",
    "WindowCovering"
];

let enums = new Set();
let enumList = {};
let bitmaps = new Set();
let bitmapList = {};
const unifyClusters = ["ProtocolController-RFTelemetry", "ProtocolController-NetworkManagement", "NameAndLocation", "AoXLocator", "AoXPositionEstimation", "ConfigurationParameters", "SystemMetrics", "Binding"];
const clusterArrayAttributes = [
    "Groups.GetGroupMembership.GroupList",
    "Groups.GetGroupMembershipResponse.GroupList",
    "Scenes.AddScene.ExtensionFieldSets",
    "Scenes.AddScene.ExtensionFieldSets.ExtensionFieldSet",
    "Scenes.EnhancedAddScene.ExtensionFieldSets",
    "Scenes.EnhancedAddScene.ExtensionFieldSets.ExtensionFieldSet",
    "Scenes.ViewSceneResponse.ExtensionFieldSets",
    "Scenes.ViewSceneResponse.ExtensionFieldSets.ExtensionFieldSet",
    "Scenes.GetSceneMembershipResponse.SceneList",
    "Scenes.EnhancedViewSceneResponse.ExtensionFieldSets",
    "Scenes.EnhancedViewSceneResponse.ExtensionFieldSets.ExtensionFieldSet",
    "Thermostat.SetWeeklySchedule.Transitions",
    "Thermostat.GetWeeklyScheduleResponse.Transitions",
    "AoXLocator.AzimuthMask",
    "AoXLocator.ElevationMask",
    "AoXLocator.AllowList",
    "AoXLocator.AntennaArray",
    "AoXLocator.IQReport.Samples",
    "ConfigurationParameters.ConfigurationParameters",
    "ProtocolController-RFTelemetry.TxReport.LastRouteRepeaters",
    "ProtocolController-RFTelemetry.TxReport.IncomingRSSIRepeaters",
    "ProtocolController-NetworkManagement.NetworkManagementState.SupportedStateList",
    "ProtocolController-NetworkManagement.NetworkManagementState.RequestedStateParameters",
    "SystemMetrics.CPUUsagePercent",
    "SystemMetrics.CPUFrequencyMHz",
    "SystemMetrics.CPUAverageUsagePercent",
    "SystemMetrics.CPUMinUsagePercent",
    "SystemMetrics.CPUMaxUsagePercent",
    "SystemMetrics.DisksUsage",
    "SystemMetrics.DisksCounters",
    "SystemMetrics.NetworkInterfacesData",
    "SystemMetrics.WirelessNetworkInterfacesData",
    "ElectricalMeasurement.GetProfileInfoResponse.ListOfAttributes",
    "ElectricalMeasurement.GetMeasurementProfileResponse.Intervals",
    "Binding.BindableClusterList",
    "Binding.BindingTable"
];

const inheritsFromUint = ["CCMinMiredsField",
    "CCMaxMiredsField",
    "DrlkPINUserID",
    "DrlkRFIDUserID",
    "DrlkTotalUserID",
    "DrlkWeekDayScheduleID",
    "DrlkYearDayScheduleID",
    "DrlkHolidayScheduleID",
    "OTAImageType",
    "SGroupId",
    "TLRangeBeginEnd"
];

function initEnums(enumType, name, value) {
    if (typeof name === 'string') {
        if (!enumList[enumType])
            enumList[enumType] = {};
        enumList[enumType][name] = value;
    }
    enums.add(enumType);
}

function isEnum(type) {
    return (castType(type) === "enum") || enums.has(type);
}

function getEnum(parentLabel, label, type, additionalTab = 0) {
    let enumObject = enumList[parentLabel + label] || enumList[label] || enumList[type];
    if (enumObject !== undefined) {
        let enumArray = Object.keys(enumObject).map(i => "{ name: \"" + i + "\", value: " + (enumObject[i] === undefined ? i : enumObject[i]) + "}");
        let joinString = ",\n" + "\t".repeat(8 + additionalTab);
        return enumArray.join(joinString);
    }
}

function initBitmaps(bitmapType, name, mask, type, offset) {
    if (typeof name === 'string') {
        if (!bitmapList[bitmapType])
            bitmapList[bitmapType] = {};
        bitmapList[bitmapType][name] = { mask: mask, type: type, offset: offset };
    }
    bitmaps.add(bitmapType);
}

function isBitmap(type) {
    return (castType(type) === "bitmap") || bitmaps.has(type);
}

function getBitmap(parentLabel, label) {
    let bitmapObject = bitmapList[parentLabel + label] || bitmapList[label];
    if (bitmapObject !== undefined) {
        let bitmapArray = Object.keys(bitmapObject).map(i => {
            let type = castType(bitmapObject[i].type);
            let item = "{ name: \"" + i + "\", mask: " + bitmapObject[i].mask + ", type: \"" + type + "\", offset: " + bitmapObject[i].offset;
            if (type === "enum")
                item += ", enum: \n" + "\t".repeat(9) + "[ " + getEnum(label, i, 2) + " ]";
            item += " }";
            return item;
        });
        return bitmapArray.join(",\n" + "\t".repeat(8));
    }
}

function isArray() {
    let indexes = Object.keys(arguments);
    indexes.pop();
    let label = arguments[indexes.shift()];
    indexes.forEach(i => label = label.concat(".").concat(arguments[i]));
    return clusterArrayAttributes.indexOf(label) > -1;
}

function getClusterTypesEnum() {
    return supportedClusters.filter(i => unifyClusters.indexOf(i) === -1).map((i) => `${i} = "${i}"`).join(', \n\t');
}

function getSupportedClusters() {
    return supportedClusters.filter(i => i !== "ProtocolController-RFTelemetry" && i != "ProtocolController-NetworkManagement" && i != "SystemMetrics").map((i) => `${i}: "${i}"`).join(', \n\t');
}

function getClusterName(name) {
    switch (name) {
        case "ProtocolController-RFTelemetry":
            return "RFTelemetry";
        case "ProtocolController-NetworkManagement":
            return "NetworkManagement";
        default:
            return name;
    }
}

function isSupportedCluster(cluster) {
    return supportedClusters.indexOf(cluster) > -1;
}

function listComma(obj) {
    if (obj.index == obj.count - 1) return ""
    return ","
}

function isCurrentType(parent, label) {
    return ((parent.parent.label + parent.label) == label) || (parent.type == label) || (parent.label == label)
}

function isCurrentBitmapEnum(parent, label) {
    return (parent.parent.parent.label + parent.label) == label
}

function castType(arg) {
    switch (arg) {
        case "bool":
            return "boolean"
        case "octstr":
        case "string":
        case "SSceneName":
            return "text"
        case "uint8":
        case "uint16":
        case "uint16":
        case "uint32":
        case "uint32":
        case "uint64":
        case "int8":
        case "int16":
        case "int32":
        case "int64":
        case "attribId":
        case "double":
        case "SGroupId":
            return "number"
        case "enum8":
        case "enum16":
            return "enum"
        case "map8":
        case "map16":
        case "map32":
        case "bitmap8":
        case "bitmap16":
            return "bitmap"
        default:
            {
                if (enums.has(arg)) return "enum";
                if (bitmaps.has(arg)) return "bitmap";
                if (inheritsFromUint.indexOf(arg) > -1) return "number";
                return "struct";
            }
    }
}

function isStruct(type) {
    return castType(type) === "struct";
}

function asNumber(value) {
    return Number(value.toString().replace("0x", ""));
}

function asBoolean(value) {
    return value === true || value === "true" || value === "True" || value === 1
}

function serialize(obj) {
    return JSON.stringify(obj);
}

function isNotEmpty(value) {
    return value != undefined && value != null && value != ""
}

function not(value) {
    return !value;
}

async function initHeplersComplete() {
    await new Promise(resolve => setTimeout(resolve, 5000));
    return true;
}

exports.initHeplersComplete = initHeplersComplete
exports.initEnums = initEnums
exports.initBitmaps = initBitmaps
exports.isEnum = isEnum
exports.isBitmap = isBitmap
exports.listComma = listComma
exports.getSupportedClusters = getSupportedClusters
exports.isSupportedCluster = isSupportedCluster
exports.castType = castType
exports.asNumber = asNumber
exports.asBoolean = asBoolean
exports.isNotEmpty = isNotEmpty
exports.serialize = serialize
exports.isCurrentType = isCurrentType
exports.not = not
exports.isCurrentBitmapEnum = isCurrentBitmapEnum
exports.getClusterTypesEnum = getClusterTypesEnum
exports.getClusterName = getClusterName
exports.isStruct = isStruct
exports.isArray = isArray
exports.getEnum = getEnum
exports.getBitmap = getBitmap