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
    default: return true
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

function isSameString(string1, string2) {
  return string1 == string2
}

function isSubstringInString(sub_string, string) {
  return string.includes(sub_string)
}

function asType(value) {
  return value.replace(/[ |-]/g, '')
}

function asCleanSymbol(label) {
  var l = label.trim()
  l = l.replace(' ', '_')
  l = l.replace(' ', '_')
  l = l.replace(/[:/-]/g, '_')
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

function asSnakeCaseUpper(str) {
  return asSnakeCaseLower(str).toUpperCase()
}

function asAttributeIdDefine(parentLabel, label) {
  return "DOTDOT_ATTRIBUTE_ID_" + asSnakeCaseUpper(parentLabel) + "_" + asSnakeCaseUpper(label)
}

function asAttributeGetFunction(parentLabel, label) {
  return "dotdot_get_" + asSnakeCaseLower(parentLabel) + "_" + asSnakeCaseLower(label)
}

function asAttributeSetFunction(parentLabel, label) {
  return "dotdot_set_" + asSnakeCaseLower(parentLabel) + "_" + asSnakeCaseLower(label)
}

function asAttributeIsSupportedFunction(parentLabel, label) {
  return "dotdot_is_supported_" + asSnakeCaseLower(parentLabel) +
    "_" + asSnakeCaseLower(label)
}

function asCommandCallback(zclCommand) {
  return dotdot_prefix() + "_" + asSnakeCaseLower(zclCommand.parent.label) +
    "_" + asSnakeCaseLower(zclCommand.label) + "_callback"
}

function asWriteAttributesCallback(zclCluster) {
  return dotdot_prefix() + "_" + asSnakeCaseLower(zclCluster.label) +
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
exports.asAttributeIdDefine = asAttributeIdDefine
exports.asAttributeGetFunction = asAttributeGetFunction
exports.asAttributeSetFunction = asAttributeSetFunction
exports.asAttributeIsSupportedFunction = asAttributeIsSupportedFunction
exports.attributeID = attributeID
exports.asCommandCallback = asCommandCallback
exports.asWriteAttributesCallback = asWriteAttributesCallback
exports.asForceReadAttributesCallback = asForceReadAttributesCallback
