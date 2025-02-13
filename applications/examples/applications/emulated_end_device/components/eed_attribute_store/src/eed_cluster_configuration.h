/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef EED_CLUSTER_CONFIGURATION_H
#define EED_CLUSTER_CONFIGURATION_H

#ifdef __cplusplus
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include "uic_typedefs.h"
#include "attribute_store.h"

struct cluster_config_entry {
    std::string unid_node;
    dotdot_endpoint_id_t endpoint_id;
    std::vector<std::string> cluster_list;
};

namespace std {
    template <>
    struct hash<std::pair<std::string, dotdot_endpoint_id_t>> {
        size_t operator()(const std::pair<std::string, dotdot_endpoint_id_t>& key) const {
            return hash<std::string>()(key.first) ^ (hash<dotdot_endpoint_id_t>()(key.second) << 1);
        }
    };
}

class ClusterConfig {
public:
    static ClusterConfig& GetInstance();
    ClusterConfig();
    void configure(const std::string& unid_node, dotdot_endpoint_id_t endpoint_node, const std::string& cluster_list_str);
    cluster_config_entry* findEntry(const std::string& unid_node, dotdot_endpoint_id_t endpoint_node);
    void removeEntry(const std::string& unid_node, dotdot_endpoint_id_t endpoint_node);

private:
    void parseClusterList(const std::string& cluster_list_str, std::vector<std::string>& cluster_list);
    std::unordered_map<std::pair<std::string, dotdot_endpoint_id_t>, cluster_config_entry> cluster_map;
};

// C-compatible declarations for Cluster Config class members
extern "C" {
#endif

typedef struct ClusterConfig ClusterConfig;
typedef struct cluster_config_entry cluster_config_entry;

cluster_config_entry* cluster_config_find_entry(const char* unid_node, dotdot_endpoint_id_t endpoint_node);
int cluster_config_entry_get_cluster_list_size(cluster_config_entry* entry);
const char* cluster_config_entry_get_cluster_list_element(cluster_config_entry* entry, int index);
void cluster_config_remove_entry(const char* unid_node, dotdot_endpoint_id_t endpoint_node);

#ifdef __cplusplus
}
#endif

#endif // EED_CLUSTER_CONFIGURATION_H
