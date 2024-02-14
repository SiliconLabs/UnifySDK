const base_helpers = require('../../../../../components/uic_dotdot/zap/addon-helper')

const component_zigpc_attrmgmt_prefix = () => "zigpc_attrmgmt"
const component_zigpc_profiles_prefix = () => "zigpc_profiles"
const component_zigpc_zcl_prefix = () => "zigpc_zcl"
const component_zigpc_cmdmapper_prefix = () => "zigpc_command_mapper"
const component_dotdot_mqtt_prefix = () => "uic_mqtt_dotdot"
const component_zigpc_cmdparser_prefix = () => "zigpc_zclcmdparse"

/**
 * List of ZCL structs supported.
 */
const SUPPORTED_STRUCTS = [
    'TransitionType',
    'CredentialStruct',
]

/**
 * List of ZCL supported enums.
 */
const SUPPORTED_ENUMS = [
    'zclStatus',
    'CredentialTypeEnum',
]

/**
 * List of ZCL server-side cluster supported.
 */
const SUPPORTED_CLUSTERS = [
    'Basic',
    'OnOff',
    'Level',
    'OccupancySensing',
    'ColorControl',
    'Identify',
    'Groups',
    'PollControl',
    'DoorLock',
    'Thermostat',
    'IASZone',
    'IASACE,',
    'IASWD',
    'Metering',
    'ElectricalMeasurement',
    'PowerConfiguration',
    'WindowCovering',
    'TemperatureMeasurement',
]

const SUPPORTED_CLUSTER_ATTRIBUTES = [
    'Basic',
    'OnOff',
    'Level',
    'OccupancySensing',
    'ColorControl',
    'Identify',
    'Groups',
    'PollControl',
    'DoorLock',
    'OTAUpgrade',
    'Thermostat',
    'IASZone',
    'IASACE,',
    'IASWD',
    'Metering',
    'ElectricalMeasurement',
    'PowerConfiguration',
    'WindowCovering',
    'TemperatureMeasurement',
]

const supportedCluster = (clusterName) => {
    return SUPPORTED_CLUSTERS.includes(clusterName)
}

const supportedAttributes = (clusterName) => {
    return SUPPORTED_CLUSTER_ATTRIBUTES.includes(clusterName)
}
const supportedStruct = (structLabel) => {
    return SUPPORTED_STRUCTS.includes(structLabel)
}

const supportedEnums = (enumLabel) => {
    return SUPPORTED_ENUMS.includes(enumLabel)
}

/**
 * Operations related to zigpc_zcl_util component.
 */
const zigpc_zcl = {
    prefix: base_helpers.asSnakeCaseUpper(component_zigpc_zcl_prefix()),
    data_type: (type) => {
        return zigpc_zcl.prefix + '_DATA_TYPE_' + base_helpers.asSnakeCaseUpper(type)
    },
    cluster_type: (clusterObj) => {
        return zigpc_zcl.prefix + '_CLUSTER_' + base_helpers.asSnakeCaseUpper(clusterObj.label)
    },
    command_type: (clusterObj, commandObj) => {
        return zigpc_zcl.cluster_type(clusterObj) + '_COMMAND_' + base_helpers.asSnakeCaseUpper(commandObj.label)
    },
    attr_type: (clusterObj, attrObj) => {
        return zigpc_zcl.cluster_type(clusterObj) + '_ATTR_' + base_helpers.asSnakeCaseUpper(attrObj.label)
    },
    struct_type: (type) => {
        return component_zigpc_zcl_prefix() + '_' + base_helpers.asSnakeCaseLower(type) + '_t'
    },
    enum_type: (type) => {
        return  'zigpc_' + base_helpers.asSnakeCaseLower(type) + '_t'
    }
};

exports.supportedEnums = supportedEnums
exports.isBool = base_helpers.isBool
exports.isString = base_helpers.isString
exports.isEnum = base_helpers.isEnum
exports.component_zigpc_attrmgmt_prefix = component_zigpc_attrmgmt_prefix
exports.component_zigpc_profiles_prefix = component_zigpc_profiles_prefix
exports.component_zigpc_zcl_prefix = component_zigpc_zcl_prefix
exports.component_zigpc_cmdmapper_prefix = component_zigpc_cmdmapper_prefix
exports.component_zigpc_cmdparser_prefix = component_zigpc_cmdparser_prefix
exports.component_dotdot_mqtt_prefix = component_dotdot_mqtt_prefix
exports.listComma = base_helpers.listComma
exports.supportedCluster = supportedCluster
exports.supportedAttributes = supportedAttributes
exports.supportedStruct = supportedStruct
exports.asSnakeCaseLower = base_helpers.asSnakeCaseLower
exports.asSnakeCaseUpper = base_helpers.asSnakeCaseUpper
exports.asZigPCZclDataType = zigpc_zcl.data_type
exports.asZigPCZclClusterType = zigpc_zcl.cluster_type
exports.asZigPCZclCommandType = zigpc_zcl.command_type
exports.asZigPCZclAttributeType = zigpc_zcl.attr_type
exports.asZigPCZclStructType = zigpc_zcl.struct_type
exports.asZigPCZclEnumType = zigpc_zcl.enum_type
exports.asDotDotMqttCommandFieldsType = base_helpers.asCommandFieldsType
exports.asDotDotMqttByGroupCommandCallback = base_helpers.asByGroupCommandCallback
exports.asDotDotMqttByGroupWriteAttributesCallback = base_helpers.asByGroupWriteAttributesCallback
    // exports.devInspect = (e) => JSON.stringify(e)
