const baseAddonHelper = require('../../zcl_util/zap/addon-helper')

const COMPONENT_PREFIX = "zigpc_zclcmdparse"

const component_cmdparser_prefix = () => {
    return COMPONENT_PREFIX
}

/**
 * Setup to find the first unique element based on parent.
 *
 * @param {*} options
 * @returns
 */
let unique_parents = {}


function zcl_commands_filtered_reset() {
    unique_parents = {}
}

function zcl_commands_filtered_add() {
    if ((this.index != null) && (this.count != null) && (this.parent != null)) {
        const parent_label = this.parent.label || null
        if (parent_label != null) {
            if (unique_parents[parent_label] == null) {
                unique_parents[parent_label] = {
                    firstIndex: this.index,
                    lastIndex: this.index
                }
            }
            if (this.index < unique_parents[parent_label].firstIndex) {
                unique_parents[parent_label].firstIndex = this.index
            }
            if (this.index > unique_parents[parent_label].lastIndex) {
                unique_parents[parent_label].lastIndex = this.index
            }
        }
    }
}

function zcl_commands_filtered_first(options) {
    if ((this.index != null) && (this.count != null) && (this.parent != null)) {
        const parent_label = this.parent.label || null
        if ((parent_label != null) && (this.index === unique_parents[parent_label].firstIndex)) {
            return options.fn(this)
        }
    }
}

/**
 * Specify content that will be generated last ONLY once inside an iterator.
 *
 * @param {*} options
 * @returns
 */
function zcl_commands_filtered_last(options) {
    if ((this.index != null) && (this.count != null) && (this.parent != null)) {
        const parent_label = this.parent.label || null
        if ((parent_label != null) && (this.index === unique_parents[parent_label].lastIndex)) {
            return options.fn(this)
        }
    }
}

exports.component_cmdparser_prefix = component_cmdparser_prefix
exports.component_zigpc_zcl_prefix = baseAddonHelper.component_zigpc_zcl_prefix

exports.zcl_commands_filtered_reset = zcl_commands_filtered_reset
exports.zcl_commands_filtered_add = zcl_commands_filtered_add
exports.zcl_commands_filtered_first = zcl_commands_filtered_first
exports.zcl_commands_filtered_last = zcl_commands_filtered_last

exports.isBool = baseAddonHelper.isBool
exports.isString = baseAddonHelper.isString
exports.listComma = baseAddonHelper.listComma
exports.supportedCluster = baseAddonHelper.supportedCluster
exports.supportedStruct = baseAddonHelper.supportedStruct
exports.asSnakeCaseLower = baseAddonHelper.asSnakeCaseLower
exports.asSnakeCaseUpper = baseAddonHelper.asSnakeCaseUpper
exports.asZigPCZclDataType = baseAddonHelper.asZigPCZclDataType
exports.asZigPCZclClusterType = baseAddonHelper.asZigPCZclClusterType
exports.asZigPCZclCommandType = baseAddonHelper.asZigPCZclCommandType
exports.asZigPCZclAttributeType = baseAddonHelper.asZigPCZclAttributeType
exports.asZigPCZclStructType = baseAddonHelper.asZigPCZclStructType