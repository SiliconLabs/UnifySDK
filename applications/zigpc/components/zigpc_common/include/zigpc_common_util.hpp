#include <string>
#include "zigpc_common_zigbee.h"

/**
 * @brief zigbee_binding_t operator==
 * An operator override for equality. Allows zigbee_binding_t to be used
 * in a variety of situations, including with std::algorithm and containers
 */ 
bool operator==(const zigbee_binding_t &A,const zigbee_binding_t &B);

/**
 * @brief zigbee_binding_t operator!=
 * An operator override for equality. Allows zigbee_binding_t to be used
 * in a variety of situations, including with std::algorithm and containers
 */ 
bool operator!=(const zigbee_binding_t &A,const zigbee_binding_t &B);

/**
 * @brief binding_to_string
 * Utility function that accepts a copy of a binding and returns
 * its serialized string
 *
 * @param binding - the binding to serialize
 * 
 * @return a string representing the input binding
 */ 
std::string binding_to_string(const zigbee_binding_t binding);

/**
 * @brief string_to_binding
 * Utility function to convert a string to a binding
 *
 * @param binding_str - a binding serialized as a string
 *
 * @return a copy of the binding serialized in the string
 */ 
zigbee_binding_t string_to_binding(const std::string &binding_str);
