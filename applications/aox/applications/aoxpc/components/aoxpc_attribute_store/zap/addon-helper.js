// A set of AddOn helpers for the attribute store helper

const baseAddonHelper = require('../../../../../../../components/uic_dotdot/zap/addon-helper.js');

function supportedCluster(clusterName) {
  switch (clusterName) {
    case "AoXLocator": return true
    case "AoXPositionEstimation": return true
    default: return false
  }
}

exports.supportedCluster = supportedCluster
exports.asSnakeCaseLower = baseAddonHelper.asSnakeCaseLower
exports.asSnakeCaseUpper = baseAddonHelper.asSnakeCaseUpper
exports.isMonotonousArray = baseAddonHelper.isMonotonousArray
exports.asAttributeIdDefine = baseAddonHelper.asAttributeIdDefine
exports.attributeID = baseAddonHelper.attributeID
exports.asAttributeIsSupportedFunction = baseAddonHelper.asAttributeIsSupportedFunction
exports.asAttributeGetCountFunction = baseAddonHelper.asAttributeGetCountFunction
exports.asAttributeGetFunction = baseAddonHelper.asAttributeGetFunction
exports.asAttributeSetFunction = baseAddonHelper.asAttributeSetFunction
exports.isString = baseAddonHelper.isString
exports.isStruct = baseAddonHelper.isStruct