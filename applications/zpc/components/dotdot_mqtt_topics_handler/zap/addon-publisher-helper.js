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