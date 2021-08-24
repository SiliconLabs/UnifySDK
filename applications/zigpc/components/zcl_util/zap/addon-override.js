function atomicType(arg) {
  switch (arg.name) {
    case "bool": return "bool"
    case "map8": return "uint8_t"
    case "map16": return "uint16_t"
    case "map32": return "uint32_t"
    case "uint8": return "uint8_t"
    case "uint16": return "uint16_t"
    case "uint32": return "uint32_t"
    case "uint64": return "uint64_t"
    case "int8": return "int8_t"
    case "int16": return "int16_t"
    case "int32": return "int32_t"
    case "int64": return "int64_t"
    case "enum8": return "uint8_t"
    case "enum16": return "uint16_t"
    case "enum32": return "uint32_t"
    case "enum64": return "uint64_t"
    case "octstr": return "const char*"
    case "string": return "const char*"
    case "attribId": return "uint16_t"
    case "bitmap8": return "uint8_t"
  }
  throw "not overriding anything"
}

function nonAtomicType(arg = { name: 'unknown' }) {
  return `${arg.name}`
}

exports.atomicType = atomicType
exports.nonAtomicType = nonAtomicType
