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
]

/**
 * List of ZCL server-side cluster supported.
 */
const SUPPORTED_CLUSTERS = [
  'OnOff',
  'Level',
  'OccupancySensing',
  'ColorControl',
  'Identify',
  'Groups',
  'PollControl',
  'DoorLock',
  'Thermostat',
]

const SUPPORTED_CLUSTER_ATTRIBUTES = [
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

const isEnumZclStatus = (label) => {
  return label.toLowerCase() === 'zclstatus'
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
  }
};

/**
 * ZCL type map used to retrieving the underlying ZCL types for:
 * - enum types
 * - atomic types
 * - bitmap types
 */
zcl_type_map = (function () {
  /* FIXME: Type-workarounds where ZAP fails to properly create our required
   * types.
   */
  const _map = {
    CCMinMiredsField: 'uint16',
    CCMaxMiredsField: 'uint16',
    DrlkPINUserID: 'uint16',
    DrlkRFIDUserID: 'uint16',
    DrlkTotalUserID: 'uint16',
    DrlkWeekDayScheduleID: 'uint8',
    DrlkYearDayScheduleID: 'uint8',
    DrlkHolidayScheduleID: 'uint8',

    // generated struct in zcl_definitions.h
    TransitionType: 'StructTransitionType',
  };

  return {
    /**
     * Retrieve the underlying type.
     *
     * @param {*} label ZCL type label
     * @returns Underlying type
     */
    get: function (label) {
      return _map[label] || ('Unknown ' + label)
    },

    /**
     * Assign the underlying type based on label passed in.
     *
     * @param {*} label
     * @param {*} type
     */
    set: function (label, type) {
      if (label && type) {
        _map[label] = type
      }
    }
  };
})()

exports.isEnumZclStatus = isEnumZclStatus
exports.isBool = base_helpers.isBool
exports.isString = base_helpers.isString
exports.isEnum = base_helpers.isEnum
exports.component_zigpc_attrmgmt_prefix = component_zigpc_attrmgmt_prefix
exports.component_zigpc_profiles_prefix = component_zigpc_profiles_prefix
exports.component_zigpc_zcl_prefix = component_zigpc_zcl_prefix
exports.component_zigpc_cmdmapper_prefix = component_zigpc_cmdmapper_prefix
exports.component_zigpc_cmdparser_prefix = component_zigpc_cmdparser_prefix
exports.component_dotdot_mqtt_prefix = component_dotdot_mqtt_prefix
exports.zcl_type_map_set_item = zcl_type_map.set
exports.zcl_type_map_get_item = zcl_type_map.get
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
exports.asDotDotMqttCommandFieldsType = base_helpers.asCommandFieldsType
exports.asDotDotMqttByGroupCommandCallback = base_helpers.asByGroupCommandCallback
exports.asDotDotMqttByGroupWriteAttributesCallback = base_helpers.asByGroupWriteAttributesCallback
// exports.devInspect = (e) => JSON.stringify(e)
