// List of attributes from the AoXLocator cluster to include as supported
// in the AoXPC configuration.

const baseAddonHelper = require('../../../../../../../components/uic_dotdot/zap/addon-helper.js');

// Adjust here the list of attributes that should be supported by the application
// Unit test needs fixing after changing this list.
const AOX_LOCATOR_CONFIGURATION_ATTRIBUTES = [
  "PositionAndOrientationValid",    // Required = true
  "PositionAndOrientation",         // Required = true
  "AzimuthMask",
  "ElevationMask",
  "AllowList",
  "CTEMode"
];

function supportedCluster(clusterName) {
  switch (clusterName) {
    case "AoXLocator": return true
    default: return false
  }
}

function isAttributeSupported(clusterName, label) {
  if (clusterName === "AoXLocator") {
    return AOX_LOCATOR_CONFIGURATION_ATTRIBUTES.includes(label);
  }
  return false;
}

exports.isAttributeSupported = isAttributeSupported
exports.supportedCluster = supportedCluster
exports.asSnakeCaseLower = baseAddonHelper.asSnakeCaseLower
exports.asSnakeCaseUpper = baseAddonHelper.asSnakeCaseUpper
exports.isMonotonousArray = baseAddonHelper.isMonotonousArray
exports.isString = baseAddonHelper.isString
exports.isStruct = baseAddonHelper.isStruct
exports.asAttributeIdDefine = baseAddonHelper.asAttributeIdDefine
exports.attributeID = baseAddonHelper.attributeID
exports.asAttributeIsSupportedFunction = baseAddonHelper.asAttributeIsSupportedFunction
exports.asAttributeGetFunction = baseAddonHelper.asAttributeGetFunction
exports.asAttributeGetCountFunction = baseAddonHelper.asAttributeGetCountFunction
exports.asAttributeSetFunction = baseAddonHelper.asAttributeSetFunction