#include <sstream>
#include <stdlib.h>
#include "zigpc_common_util.hpp"

bool operator==(const zigbee_binding_t &A,const zigbee_binding_t &B)
{
    bool result =   (A.source_address == B.source_address) &&
                    (A.source_endpoint == B.source_endpoint) &&
                    (A.source_cluster == B.source_cluster) &&
                    (A.dest_address == B.dest_address) &&
                    (A.dest_endpoint == B.dest_endpoint);
    return result;
            
}

bool operator!=(const zigbee_binding_t &A,const zigbee_binding_t &B)
{
    return !(A == B);
}

std::string binding_to_string(const zigbee_binding_t binding)
{
    std::stringstream ss;
    ss << std::hex << binding.source_address;
    ss << ",";
    ss << std::dec << static_cast<unsigned int>(binding.source_endpoint);
    ss << ",";
    ss << std::dec << static_cast<unsigned int>(binding.source_cluster);
    ss << ",";
    ss << std::hex << binding.dest_address;
    ss << ","; 
    ss << std::dec << static_cast<unsigned int>(binding.dest_endpoint);
    return ss.str();
    
}

zigbee_binding_t string_to_binding(const std::string &binding_str)
{
    std::stringstream ss(binding_str);
    zigbee_binding_t binding;
    
    std::string source_addr_string;
    getline(ss, source_addr_string, ','); 
    binding.source_address = std::stoull(source_addr_string, nullptr, 16);

    std::string source_ep_string;
    getline(ss, source_ep_string, ','); 
    binding.source_endpoint = std::stoull(source_ep_string);
    
    std::string source_cluster_string;
    getline(ss, source_cluster_string, ','); 
    binding.source_cluster = std::stoull(source_cluster_string);
    
    std::string dest_addr_string;
    getline(ss, dest_addr_string, ','); 
    binding.dest_address = std::stoull(dest_addr_string, nullptr, 16);
    
    std::string dest_ep_string;
    getline(ss, dest_ep_string, ','); 
    binding.dest_endpoint = std::stoull(dest_ep_string);

    return binding;    
}
