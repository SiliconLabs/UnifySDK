/**
 *
 * @param size
 * @param name
 * @returns The appropriate c type for an enum
 */
function enumType(size, name) {
  if (name && !name.startsWith('enum')) {
    return name
  } else {
    let enumSize = size ? size * 8 : 8
    return 'uint' + enumSize + '_t'
  }
}

/**
 *
 *
 * @param size
 * @param isSigned
 * @param name
 * @returns The appropriate c type for a number
 */
function numberType(size, isSigned, name) {
    let prefix = isSigned ? 'int' : 'uint'
    if (name == 'no_data') {
      return 'uint8_t *'
    }
    if (name == 'bool'){ // Adding this because ZAP treats bool as an unsigned 8-bit unsigned integer
      return 'bool'
    }
    if (name == 'double'){ // Adding this because ZAP treats double as an 64-bit unsigned integer
      return 'double'
    }
    if (name == 'single'){ // Adding this because ZAP treats float as an 32-bit unsigned integer
      return 'float'
    }
    if (size > 8) {
      if (isSigned) {
        return 'int8_t *'
      } else {
        return 'uint8_t *'
      }
    }
    let numberSize = size ? size * 8 : 8
    if (size == 3) {
      numberSize = (size + 1) * 8
    }
    return prefix + numberSize + '_t'
}

/**
 *
 * @returns 'const char*'
 */
function stringType() {
  return 'const char*'
}

exports.enumType = enumType
exports.numberType = numberType
exports.stringType = stringType
