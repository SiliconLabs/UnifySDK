// Function to determine if command from dotdot xml files is Optional or Client
function notClientCommandOrOptional (source, optional) {
  const isClientLabel = source == 'client';
  return isClientLabel || optional;
}

exports.notClientCommandOrOptional = notClientCommandOrOptional

function helper_print() {
  console.log(this);
}
exports.helper_print = helper_print

function isZwaveWritable() {
  if (writable_block_list.filter(e => e[0] === this.parent.label &&
        e[1] === this.label).length > 0) {
    return false;
  }
  return this.isWritable
}
exports.isZwaveWritable = isZwaveWritable

const writable_block_list = [
  ["Thermostat", "MaxHeatSetpointLimit"],
  ["Thermostat", "HVACSystemTypeConfiguration"],
  ["Thermostat", "LocalTemperatureCalibration"],
  ["Thermostat", "UnoccupiedCoolingSetpoint"],
  ["Thermostat", "UnoccupiedHeatingSetpoint"],
  ["Thermostat", "MinSetpointDeadBand"],
  ["Thermostat", "MaxHeatSetpointLimit"],
  ["Thermostat", "MinCoolSetpointLimit"],
  ["Thermostat", "MaxCoolSetpointLimit"],
  ["Thermostat", "MinSetpointDeadBand"],
  ["Thermostat", "RemoteSensing"],
  ["Thermostat", "ControlSequenceOfOperation"],
  ["Thermostat", "TemperatureSetpointHold"],
  ["Thermostat", "TemperatureSetpointHoldDuration"],
  ["Thermostat", "ThermostatProgrammingOperationMode"],
  ["Thermostat", "OccupiedSetback"],
  ["Thermostat", "UnoccupiedSetback"],
  ["Thermostat", "EmergencyHeatDelta"],
  ["Thermostat", "ACType"],
  ["Thermostat", "ACCapacity"],
  ["Thermostat", "ACRefrigerantType"],
  ["Thermostat", "ACCompressorType"],
  ["Thermostat", "ACErrorCode"],
  ["Thermostat", "ACLouverPosition"],
  ["Thermostat", "ACCapacityFormat"],
]