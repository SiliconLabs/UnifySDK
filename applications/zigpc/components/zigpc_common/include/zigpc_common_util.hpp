#include <string>
#include "zigpc_common_zigbee.h"


bool operator==(const zigbee_binding_t &A,const zigbee_binding_t &B);

bool operator!=(const zigbee_binding_t &A,const zigbee_binding_t &B);

    std::string binding_to_string(const zigbee_binding_t binding);
    zigbee_binding_t string_to_binding(const std::string &binding_str);
