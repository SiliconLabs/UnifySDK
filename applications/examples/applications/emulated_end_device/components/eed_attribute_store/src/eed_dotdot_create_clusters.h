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

#ifndef EED_DOTDOT_CREATE_CLUSTERS_H
#define EED_DOTDOT_CREATE_CLUSTERS_H

#ifdef __cplusplus

#include "dotdot_mqtt.h"
#include <map> // Include map header
#include <string>

// Define your function pointer type
typedef void (*eed_cluster_attribute_wrapper)(const dotdot_unid_t unid, const dotdot_endpoint_id_t endpoint_id);

// Map to cluster name and function pointer
extern std::map<std::string, eed_cluster_attribute_wrapper> CreateClusterMap;
extern std::map<std::string, eed_cluster_attribute_wrapper> CreateUnretainMap;

extern "C" {
#endif

// Creates all possible attributes/commands under the endpoint for all supported clusters
void eed_dotdot_create_clusters(const dotdot_unid_t unid, const dotdot_endpoint_id_t endpoint_id);

// Creates all possible attributes/commands under the endpoint for provided cluster id
void eed_dotdot_create_cluster_by_name(const dotdot_unid_t unid, const dotdot_endpoint_id_t endpoint_id, const char* cluster_name);

// Unretaining all attributes/commands created under the endpoint for provided cluster id
void eed_dotdot_unretain_cluster_by_name(const dotdot_unid_t unid, const dotdot_endpoint_id_t endpoint_id, const char* cluster_name);

#ifdef __cplusplus
}
#endif

#endif // EED_DOTDOT_CREATE_CLUSTERS_H