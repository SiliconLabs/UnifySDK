let supportedClusters = [
    "Basic",
    "ColorControl",
    "DoorLock",
    "IASZone",
    "Identify",
    "Level",
    "OccupancySensing",
    "OnOff",
    "Thermostat"
];

const enums = new Set();
const bitmaps = new Set();

function initEnums(enumItem) {
    enums.add(enumItem);
}

function isEnum(type) {
    return (castType(type) === "enum") || enums.has(type);
}

function initBitmaps(bitmapItem) {
    bitmaps.add(bitmapItem);
}

function isBitmap(type) {
    return (castType(type) === "bitmap") || bitmaps.has(type);
}

function getClusterTypesEnum() {
    return supportedClusters.map((i) => `${i} = "${i}"`).join(', \n\t');
}

function getSupportedClusters() {
    return supportedClusters.map((i) => `${i}: "${i}"`).join(', \n\t');
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
                if (enums.has(arg)) return "enum"
                if (bitmaps.has(arg)) return "bitmap"
                return arg
            }
    }
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