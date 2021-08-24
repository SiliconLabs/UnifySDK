/*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#ifndef ATTRIBUTE_MAP_H
#define ATTRIBUTE_MAP_H

#include <string.h>
#include "zigpc_common_zigbee.h"
#include "attribute_management_int.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * attribute_key
 *
 * @brief the data structure used to access attributes when they are stored
 * in this component. Each key is expected to correspond to one list of
 * attributes. 
**/
typedef struct {
  zigbee_eui64_t eui64;
  unsigned int endpoint_id;
  unsigned int cluster_id;

} attribute_key;

/**
 * @brief register_node_attributes - Registers the attributes of a node in a
 * map for later retrieval. Should be called when a new node is available on 
 * the network.
 *
 * @param node - the node whose attributes we wish to store for the first 
 *               time
 *
 * @return SL_STATUS_OK if able to successfully add attributes of clusters
 * on the endpoints of the supplied node.
**/
sl_status_t register_node_attributes(const zigbee_node_t node);

/**
 * @brief register_endpoint_attributes - Registers the attributes of an
 * endpoint in a map for later retrieval. Should be called when a new 
 * endpoint is available on the network.
 *
 * @param node - the eui64 of the node hosting the endpoint
 * @param endpoint - the endpoint id of the specified endpoint, whose
 *                   clusters' attributes should be stored
 *
 * @return SL_STATUS_OK if able to successfully add attributes of clusters
 * on the specified endpoint
**/
sl_status_t register_endpoint_attributes(const zigbee_eui64_t eui64,
                                         const zigbee_endpoint_t endpoint);

/**
 * @brief register_cluster_attributes - Registers the attributes of a cluster
 * in a map for later retrieval.
 *
 * @param eui64 - the eui64 of the node hosting the specified endpoint
 * @param endpoint_id - the endpoint hosting the specified cluster
 * @param cluster_id - the cluster id whose attributes should be stored 
 *
 * @return SL_STATUS_OK if able to successfully add attributes of the
 * specified cluster
**/
sl_status_t register_cluster_attributes(const zigbee_eui64_t eui64,
                                        const zigbee_endpoint_id_t endpoint_id,
                                        const zcl_cluster_id_t cluster_id);

/**
 * @brief register_attributes - Directly store a list of attributes with a
 * corresponding attribute_key
 *
 * @param key - the key, representing a specific cluster on an endpoint of a 
 *              node which can be used to retrieve a list of attributes
 * @param attributes - the pointer to a list of attributes that should be 
 *                     stored
 * @param num_attributes - the size of the above attributes list
 *
 * @return SL_STATUS_OK if the list was able to successfully be stored
**/
sl_status_t register_attributes(const attribute_key key,
                                zcl_attribute_t *attributes,
                                unsigned int num_attributes);

/**
 * @brief read_attributes - Retrieve a list of attributes corresponding to a 
 * single key
 *
 * @param key - the key, representing a specific cluster on an endpoint of a 
 *              node which can be used to retrieve a list of attributes
 * @param attributes - the pointer where a list of attributes should be 
 *                     stored
 * @param num_attributes - the size of the above attributes list
 *
 * @return SL_STATUS_OK if the list was able to successfully be retrieved 
**/
sl_status_t read_attributes(const attribute_key key,
                            zcl_attribute_t *attributes,
                            const unsigned int num_attributes);

/**
 * @brief read_single_attribute - Retrieve a single attribute
 *
 * @param key - the key, representing a specific cluster on an endpoint of a 
 *              node
 * @param attribute_id - the id of the specific attribute whose value should
 *                       be read
 * @param dest_attribute - the pointer where the retrieved attribute should
 *                         be stored
 *
 * @return SL_STATUS_OK if the attribute was able to successfully be retrieved 
**/
sl_status_t read_single_attribute(const attribute_key key,
                                  const zcl_attribute_id_t attribute_id,
                                  zcl_attribute_t *dest_attribute);

/**
 * @brief update_attributes - updates the value of stored attributes. A key
 * should be registered before calling this function.
 *
 * @param key - the key, representing a specific cluster on an endpoint of a 
 *              node
 * @param attributes - the pointer with a list of attribute values to store
 * @param num_attributes - the size of the provided attribute list
 * 
 * @return SL_STATUS_OK if the new attribute list (and values) was able to
 * be stored
**/
sl_status_t update_attributes(const attribute_key key,
                              const zcl_attribute_t *attributes,
                              unsigned int num_attributes);

/**
 * @brief remove_attributes - remove / deregister a list of attributes of
 * a specific cluster
 * 
 * @param key - the key, representing a specific cluster on an endpoint of a 
 *              node
 *
 *@return SL_STATUS_OK if the cluster's attributes were successfully removed
**/
sl_status_t remove_attributes(const attribute_key key);

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_MAP_H
