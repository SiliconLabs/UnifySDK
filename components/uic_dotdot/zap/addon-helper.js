function listComma(obj) {
  if (obj.index == obj.count - 1) return ""
  return ","
}

function isUnknownBitmap(label) {
  switch (label) {
    case "RelayStatus": return true
    case "ExtendedStatus": return true
    default: return false
  }
}

function supportedCluster(clusterName) {
  switch (clusterName) {
    case "TouchlinkCommissioning": return false
    case "IASACE": return false
    case "Commissioning": return false
    case "OTAUpgrade": return false
    default: return true
  }
}

// Verify if the cluster is located directly under the UNID or if it should be
// under an Endpoint.
function clusterWithoutEndpoints(clusterName) {
  switch (clusterName) {
    case "ProtocolController-RFTelemetry": return true
    case "ProtocolController-NetworkManagement": return true
    case "State": return true
    case "SystemMetrics": return true
    case "ApplicationMonitoring": return true
    default: return false
  }
}

// Cluster that uses the Machine ID instead of the UNID.
function clusterWithMachineID(clusterName) {
  switch (clusterName) {
    case "SystemMetrics": return true
    default: return false
  }
}

// Cluster that uses the MQTT Client ID instead of the UNID.
function clusterWithMQTTClientID(clusterName) {
  switch (clusterName) {
    case "ApplicationMonitoring": return true
    default: return false
  }
}


function dotdot_prefix() {
  return "uic_mqtt_dotdot"
}

// WARNING: Should not be used, use handlebar {{#if_is_enum type}} {{/if_is_enum}} instead!
function isEnum(type) {
  if (type.startsWith("enum")) {
    return true;
  }
  return false;
}

function enumType(type) {
  if (type.startsWith("enum")) {
    return "uint" + type.replace(/^\D+/g, '') + "_t"
  }
  return "uint8_t"
}

function isString(type) {
  switch (type) {
    case 'string': return true
    case 'octstr': return true
    case 'SSceneName': return true
    default: return false;
  }
}

function chooseTypeOrBitmapName(type, parent_label, label) {
  if (type.startsWith("map")) {
    return asType(parent_label + label)
  }
  return asType(type)
}

function chooseTypeOrEnumName(type, parent_label, label) {
  if (type.startsWith("enum")) {
    return asType(parent_label + label)
  }
  return asType(type)
}

function checkSpecialTypeBitmapsEnum(type) {
  if (type == "SHDCFGDirection" || type == "IaswdLevel") {
    return "enum8";
  }
  return type;
}

function isBool(type) {
  switch (type) {
    case 'bool': return true
    default: return false
  }
}

function isStruct(type) {
  switch (type) {
    case 'TransitionType': return true
    case 'SExtensionFieldSetList': return true
    case 'BindingObject': return true
    case 'CoordinateAndOrientation': return true
    case 'MinMaxPair': return true
    case 'SphericalCoordinates': return true
    case 'EstimatedPosition': return true
    case 'ConfigurationParameter' : return true
    case 'NetworkInterfaceData' : return true
    case 'WirelessNetworkInterfaceData' : return true
    case 'DiskIOCounters' : return true
    case 'DiskUsageData': return true
    case 'NetworkManagementState': return true
    case 'SSceneTable': return true
    default: return false
  }
}

function isMonotonousArray(label) {
  switch (label) {
    // Binding cluster:
    case 'BindableClusterList': return true
    case 'BindingTable': return true
    // State cluster:
    case 'EndpointIdList': return true
    case 'NetworkList': return true
    // AoXLocator cluster:
    case 'AzimuthMask': return true
    case 'ElevationMask': return true
    case 'AllowList': return true
    case 'AntennaArray': return true
    // ConfigurationParameters cluster:
    case 'ConfigurationParameters': return true
    // SystemMetrics cluster:
    case 'CPUUsagePercent': return true
    case 'CPUFrequencyMHz': return true
    case 'CPUAverageUsagePercent': return true
    case 'CPUMinUsagePercent': return true
    case 'CPUMaxUsagePercent': return true
    case 'DisksUsage': return true
    case 'DisksCounters': return true
    case 'NetworkInterfacesData': return true
    case 'WirelessNetworkInterfacesData': return true
    // ApplicationMonitoring cluster:
    case 'ApplicationMQTTTopics': return true
    // ProtocolController-NetworkManagement
    case 'SupportedStateList': return true
    case 'RequestedStateParameters': return true
    // Scenes cluster:
    case 'SceneTable': return true
    default: return false
  }
}


function isSameString(string1, string2) {
  return string1 == string2
}

function isSubstringInString(sub_string, string) {
  return string.includes(sub_string)
}

function asType(value) {
  return value.replace(/[ |-]/g, '')
}

/**
 * @brief Retrieve the JSON data type for a DotDot cluster attribute or command argument.
 *
 * @param {*} type    DotDot type.
 * @param {*} label   Label for DotDot attribute or command.
 * @param {*} cluster DotDot cluster.
 * @returns String representation of JSON type, UCL enum, or UCL bitmap type.
 */
function asJSONType(type, label, cluster) {
  switch (type) {
    case "bool": return "boolean"
    case "single":
    case "double":
      return "number"
    case "uint8":
    case "uint16":
    case "uint24":
    case "uint32":
    case "uint64":
    case "int8":
    case "int16":
    case "int32":
    case "int64":
      return "integer"
    case "octstr":
    case "string":
      return "string"
    case "map8":
    case "map16":
    case "map32":
    case "enum8":
    case "enum16":
      // treat array count fields as integers
      if (label.includes("NumberOf")) {
        return "integer"
      } else {
        return cluster + label
      }
    default: return type
  }
}

function asCleanSymbol(label) {
  var l = label.trim()
  l = l.replace(/ /g, '_')
  l = l.replace(/-/g, '_')
  l = l.replace(/[/]/g, '_')
  l = l.replace(/:/g, '_')
  l = l.replace(/__+/g, '_').toLowerCase()
  return l
}

function asSnakeCaseLower(str) {
  if (!str) {
    return str
  }
  var label = str.replace(/\.?([A-Z][a-z])/g, function (x, y) {
    return '_' + y
  })
  label = asCleanSymbol(label)
  if (label.startsWith('_')) {
    label = label.substring(1)
  }
  return label
}

function asPublicationName(str) {
  // Replace - with / for publications. It allows for Cluster names like
  // ProtocolController-SubFunctionality, which will be published as
  // ProtocolController/SubFunctionality
  return str.replace("-", "/")
}

function asSnakeCaseUpper(str) {
  return asSnakeCaseLower(str).toUpperCase()
}

function asAttributeIdDefine(parentLabel, label) {
  return "DOTDOT_ATTRIBUTE_ID_" + asSnakeCaseUpper(parentLabel) + "_" + asSnakeCaseUpper(label)
}

function asAttributeGetFunction(parentLabel, label) {
  return "dotdot_get_" + asSnakeCaseLower(parentLabel) + "_" + asSnakeCaseLower(label)
}

function asAttributeGetCountFunction(parentLabel, label) {
  return "dotdot_get_" + asSnakeCaseLower(parentLabel) + "_" + asSnakeCaseLower(label) + "_count"
}

function asAttributeSetFunction(parentLabel, label) {
  return "dotdot_set_" + asSnakeCaseLower(parentLabel) + "_" + asSnakeCaseLower(label)
}

function asAttributeIsSupportedFunction(parentLabel, label) {
  return "dotdot_is_supported_" + asSnakeCaseLower(parentLabel) +
    "_" + asSnakeCaseLower(label)
}

function asAttributeCreateFunction(parentLabel, label) {
  return "dotdot_create_" + asSnakeCaseLower(parentLabel) + "_" + asSnakeCaseLower(label)
}

function asAttributeUndefineReportedFunction(parentLabel, label) {
  return "dotdot_" + asSnakeCaseLower(parentLabel) +
    "_" + asSnakeCaseLower(label) + "_undefine_reported"
}

function asAttributeUndefineDesiredFunction(parentLabel, label) {
  return "dotdot_" + asSnakeCaseLower(parentLabel) +
    "_" + asSnakeCaseLower(label) + "_undefine_desired"
}

function asAttributeIsReportedDefinedFunction(parentLabel, label) {
  return "dotdot_" + asSnakeCaseLower(parentLabel) +
    "_" + asSnakeCaseLower(label) + "_is_reported_defined"
}

function asAttributeIsDesiredDefinedFunction(parentLabel, label) {
  return "dotdot_" + asSnakeCaseLower(parentLabel) +
    "_" + asSnakeCaseLower(label) + "_is_desired_defined"
}

function asCommandCallback(zclCommand) {
  return dotdot_prefix() + "_" + asSnakeCaseLower(zclCommand.parent.label) +
    "_" + asSnakeCaseLower(zclCommand.label) + "_callback"
}

function asGeneratedCommandCallback(zclCommand) {
  return dotdot_prefix() + "_" + asSnakeCaseLower(zclCommand.parent.label) +
    "_generated_" + asSnakeCaseLower(zclCommand.label) + "_callback"
}

function asAttributeCallback(zclAttribute) {
  return dotdot_prefix() + "_" + asSnakeCaseLower(zclAttribute.parent.label) +
    "_attribute_" + asSnakeCaseLower(zclAttribute.label) + "_callback"
}

/**
 * Form signature for by-group command callbacks.
 *
 * @param {*} zclCommand ZAP Object for ZCL Command.
 * @returns command callback signature.
 */
function asByGroupCommandCallback(zclCommand) {
  return dotdot_prefix() + "_by_group_" + asSnakeCaseLower(zclCommand.parent.label) +
    "_" + asSnakeCaseLower(zclCommand.label) + "_callback"
}

/**
 * Form the command parse function name.
 *
 * @param {*} zclCommand ZAP Object for ZCL Command
 * @returns command parse function name.
 */
function asCommandParseFunction(zclCommand) {
  return dotdot_prefix() + "_parse_" + asSnakeCaseLower(zclCommand.parent.label) +
    "_" + asSnakeCaseLower(zclCommand.label)
}

/**
 * Get the ZCL command fields structure type name.
 *
 * @param {*} zclCommand ZAP Object for ZCL Command
 * @returns type.
 */
function asCommandFieldsType(zclCommand) {
  return dotdot_prefix() + "_" + asSnakeCaseLower(zclCommand.parent.label) +
    "_command_" + asSnakeCaseLower(zclCommand.label) + "_fields_t"
}

function asWriteAttributesParseFunction(zclCluster) {
  return dotdot_prefix() + "_parse_" + asSnakeCaseLower(zclCluster.label) +
    "_write_attributes"
}

function asWriteAttributesCallback(zclCluster) {
  return dotdot_prefix() + "_" + asSnakeCaseLower(zclCluster.label) +
    "_write_attributes_callback"
}

/**
 * Form signature for by-group cluster write-attributes callbacks.
 *
 * @param {*} zclCluster ZAP Object for ZCL Cluster.
 * @returns write-attributes callback signature.
 */
function asByGroupWriteAttributesCallback(zclCluster) {
  return dotdot_prefix() + "_by_group_" + asSnakeCaseLower(zclCluster.label) +
    "_write_attributes_callback"
}

function asForceReadAttributesCallback(zclCluster) {
  return dotdot_prefix() + "_" + asSnakeCaseLower(zclCluster.label) +
    "_force_read_attributes_callback"
}

/**
 * Helper function to build the hex representation of an attribute, the argument
 * must be the attribute javascript object
 */
function attributeID(attribute) {
  if (attribute.parent.code == 0) {
    // Shift cluster id 0 (Basic) to 0xBA5C as the 0-space is used for Attribute Type and Z-Wave CC attributes
    attribute.parent.code = 0xBA5C
  }
  return (attribute.parent.code * 0x10000 + attribute.code).toString(16).padStart(8, 0);
}

exports.isBool = isBool
exports.isString = isString
exports.isEnum = isEnum
exports.isStruct = isStruct
exports.isMonotonousArray = isMonotonousArray
exports.enumType = enumType
exports.chooseTypeOrBitmapName = chooseTypeOrBitmapName
exports.chooseTypeOrEnumName = chooseTypeOrEnumName
exports.isUnknownBitmap = isUnknownBitmap
exports.isSubstringInString = isSubstringInString
exports.checkSpecialTypeBitmapsEnum = checkSpecialTypeBitmapsEnum
exports.isSameString = isSameString
exports.dotdot_prefix = dotdot_prefix
exports.listComma = listComma
exports.supportedCluster = supportedCluster
exports.asSnakeCaseLower = asSnakeCaseLower
exports.asSnakeCaseUpper = asSnakeCaseUpper
exports.asPublicationName = asPublicationName
exports.clusterWithoutEndpoints = clusterWithoutEndpoints
exports.clusterWithMachineID = clusterWithMachineID
exports.clusterWithMQTTClientID = clusterWithMQTTClientID
exports.asAttributeIdDefine = asAttributeIdDefine
exports.asAttributeGetFunction = asAttributeGetFunction
exports.asAttributeGetCountFunction = asAttributeGetCountFunction
exports.asAttributeSetFunction = asAttributeSetFunction
exports.asAttributeIsSupportedFunction = asAttributeIsSupportedFunction
exports.asAttributeUndefineReportedFunction = asAttributeUndefineReportedFunction
exports.asAttributeUndefineDesiredFunction = asAttributeUndefineDesiredFunction
exports.asAttributeIsReportedDefinedFunction = asAttributeIsReportedDefinedFunction
exports.asAttributeIsDesiredDefinedFunction = asAttributeIsDesiredDefinedFunction
exports.asAttributeCreateFunction = asAttributeCreateFunction
exports.asJSONType = asJSONType
exports.attributeID = attributeID
exports.asCommandCallback = asCommandCallback
exports.asAttributeCallback = asAttributeCallback
exports.asGeneratedCommandCallback = asGeneratedCommandCallback
exports.asByGroupCommandCallback = asByGroupCommandCallback
exports.asCommandFieldsType = asCommandFieldsType
exports.asCommandParseFunction = asCommandParseFunction
exports.asWriteAttributesParseFunction = asWriteAttributesParseFunction
exports.asWriteAttributesCallback = asWriteAttributesCallback
exports.asByGroupWriteAttributesCallback = asByGroupWriteAttributesCallback
exports.asForceReadAttributesCallback = asForceReadAttributesCallback
