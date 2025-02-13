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

#include "eed_cluster_configuration.h"
#include "sl_log.h"

static ClusterConfig cluster_config;

ClusterConfig& ClusterConfig::GetInstance() {
    return cluster_config;
}

ClusterConfig::ClusterConfig() {}

void ClusterConfig::configure(const std::string& unid_node, dotdot_endpoint_id_t endpoint_node, const std::string& cluster_list_str) {
    std::vector<std::string> cluster_list;
    parseClusterList(cluster_list_str, cluster_list);
    cluster_config_entry entry = {unid_node, endpoint_node, cluster_list};
    cluster_map[std::make_pair(unid_node, endpoint_node)] = entry;
    sl_log_info("cluster_config", "Successfully added entry for %s unid and %d endpoint_node", unid_node.c_str(), endpoint_node);
}

cluster_config_entry* ClusterConfig::findEntry(const std::string& unid_node, dotdot_endpoint_id_t endpoint_node) {
    auto key = std::make_pair(unid_node, endpoint_node);
    auto it = cluster_map.find(key);
    if (it != cluster_map.end()) {
        sl_log_debug("cluster_config", "Found entry for %s unid and %d endpoint_node", unid_node.c_str(), endpoint_node);
        return &(it->second);
    }
    return nullptr;
}

void ClusterConfig::parseClusterList(const std::string& cluster_list_str, std::vector<std::string>& cluster_list) {
    std::istringstream ss(cluster_list_str);
    std::string token;
    while (std::getline(ss, token, ';')) {
        cluster_list.push_back(token);
    }
}
void ClusterConfig::removeEntry(const std::string& unid_node, dotdot_endpoint_id_t endpoint_node) {
    auto key = std::make_pair(unid_node, endpoint_node);
    auto it = cluster_map.find(key);
    if (it != cluster_map.end()) {
        sl_log_debug("cluster_config", "Removing entry for %s unid and %d endpoint_node", unid_node.c_str(), endpoint_node);
        cluster_map.erase(it);
    }
    return;
}

// C-compatible declarations for Cluster Config class members
extern "C" {
    void cluster_config_configure(const char* unid_node, dotdot_endpoint_id_t endpoint_node, const char* cluster_list_str) {
        ClusterConfig::GetInstance().configure(unid_node, endpoint_node, cluster_list_str);
    }

    cluster_config_entry* cluster_config_find_entry(const char* unid_node, dotdot_endpoint_id_t endpoint_node) {
        return ClusterConfig::GetInstance().findEntry(unid_node, endpoint_node);
    }

    int cluster_config_entry_get_cluster_list_size(cluster_config_entry* entry) {
        return entry->cluster_list.size();
    }

    const char* cluster_config_entry_get_cluster_list_element(cluster_config_entry* entry, int index) {
        return entry->cluster_list[index].c_str();
    }

    void cluster_config_remove_entry(const char* unid_node, dotdot_endpoint_id_t endpoint_node) {
        return ClusterConfig::GetInstance().removeEntry(unid_node, endpoint_node);
    }
}
