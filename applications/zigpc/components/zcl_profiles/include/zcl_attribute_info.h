/*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

/**
 * @file attribute_info.h
 * @defgroup attribute_info Attribute Info Management
 * @ingroup attribute_management
 * @brief This header contains functions for accessing information
 * relating to attributes, such as number of attributes, attribute
 * names and more.
 *
 * @{
 */

#ifndef ATTRIBUTE_INFO_H
#define ATTRIBUTE_INFO_H

#include "sl_status.h"
#include "zigpc_common_zigbee.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ZCL_WRITE_MAX_DATA_SIZE 16

/**
 * @brief init_cluster_attributes - the API for instantiating cluster attributes
 * Required to set up all attributes on supported clusters.
 *
 * @return SL_STATUS_OK if able to initialize attributes for clusters.
 * SL_STATUS_FAIL otherwise
**/
sl_status_t zigpc_zcl_profiles_init(void);

/**
 * @brief Retrieve number of attributes of a given cluster
 *
 * @param cluster_id      ZCL Cluster ID
 *
 * @return size_t         Number of supported attributes found for cluster
 **/
size_t zigpc_zclprofiles_get_cluster_attribute_count(
  const zcl_cluster_id_t cluster_id);

/**
 * @brief zigpc_zclprofiles_get_cluster_attribute_list - Retrieve the attributes of a given
 * cluster ID. Expected to be called after zigpc_zclprofiles_get_cluster_attribute_count.
 *
 * @param cluster_id - the cluster id of the attributes desired
 * @param attribute_list - the pointer where the attributes will be stored
 *
 * @return SL_STATUS_OK if the operation succeeded, otherwise SL_STATUS_FAIL
 * if a null pointer was supplied or the cluster is not supported
**/
sl_status_t zigpc_zclprofiles_get_cluster_attribute_list(
  const zcl_cluster_id_t cluster_id, zcl_attribute_t *attribute_list);

#ifdef COMPILE_UNUSED_CODE

/**
 * @brief retrieve_cluster_name_length - API to retrieve the length of the
 * cluster name. separate from retrieving the cluster name to allow for
 * proper string allocation
 *
 * @param cluster_id - the id corresponding to the ZCL cluster number
 *
 * @param strlen - the pointer where the length of the cluster name
 *                  will be written
 *                  NOTE - this value will be the length of the name +1,
 *                  to account for space for \0
 *
 * @return SL_STATUS_OK if the length was found, SL_STATUS_FAIL otherwise
**/
sl_status_t retrieve_cluster_name_length(const zcl_cluster_id_t cluster_id,
                                         unsigned int *strlen);
/**
 * @brief retrieve_cluster_name - API to retrieve the name of a cluster.
 *
 * @param cluster_id - the id corresponding to the ZCL cluster number
 * @param cluster_name - the pointer where the cluster name will be written
 * @param strlen - the size of cluster_name
 *
 * @return SL_STATUS_OK if the name was found, and could be written.
 * SL_STATUS_FAIL otherwise
**/
sl_status_t retrieve_cluster_name(const zcl_cluster_id_t cluster_id,
                                  char *cluster_name,
                                  unsigned int strlen);

/**
 * @brief retrieve_cluster_id_from_name - API to retrieve the ID of a cluster
 * from its name
 *
 * @param cluster_name - the pointer where the cluster name will be written
 * @param strlen - the size of cluster_name
 * @param cluster_id - the destination where to copy the cluster_id result
 *
 * @return SL_STATUS_OK if the ID was found, and could be written.
 * SL_STATUS_FAIL otherwise
**/
sl_status_t retrieve_cluster_id_from_name(const char *cluster_name,
                                          unsigned int strlen,
                                          zcl_cluster_id_t *cluster_id);

/**
 * @brief retrieve_attribute_name_length - API to retrieve the length of the
 * attribute name. separate from retrieving the attribute name to allow for
 * proper string allocation
 *
 * @param cluster_id - the id corresponding to the ZCL cluster number
 * @param attribute_id - the id corresponding to the ZCL attribute on the
 *                       given cluster
 *
 * @param length - the pointer where the length of the cluster name
 *                  will be written
 *                  NOTE - this will be the lenght of the name +1 to account
 *                  for space of a \0
 *
 * @return SL_STATUS_OK if the length was found, SL_STATUS_FAIL otherwise
**/
sl_status_t
  retrieve_attribute_name_length(const zcl_cluster_id_t cluster_id,
                                 const zcl_attribute_id_t attribute_id,
                                 unsigned int *length);

/**
 * @brief retrieve_attribute_name - API to retrieve the name of an attribute
 * of a given cluster.
 *
 * @param cluster_id - the id corresponding to the ZCL cluster number
 * @param attribute_id - the id corresponding to the attribute of a given
 *                       cluster
 * @param attribute_name - the pointer to the destination array, where
 *                         the name of the attribute should be copied
 * @param str_length - the length of the destination array where the name
 *                     should be copied
 *
 * @return SL_STATUS_OK if the name was found, and could be written.
 * SL_STATUS_FAIL otherwise
**/
sl_status_t retrieve_attribute_name(const zcl_cluster_id_t cluster_id,
                                    const zcl_attribute_id_t attribute_id,
                                    char *attribute_name,
                                    unsigned int str_length);

/**
 * @brief retrieve_attribute_type - API to retrieve the type of an attribute
 * of a given cluster.
 *
 * @param cluster_id - the id corresponding to the ZCL cluster number
 * @param attribute_id - the id corresponding to the attribute of a given
 *                       cluster
 * @param type - the destination where to copy the type information
 *
 * @return SL_STATUS_OK if the type was found, and could be written.
 * SL_STATUS_FAIL otherwise
**/
sl_status_t retrieve_attribute_type(const zcl_cluster_id_t cluster_id,
                                    const zcl_attribute_id_t attribute_id,
                                    enum e_zcl_attribute_type *type);

/**
 * @brief retrieve_attribute_id_from_name - API to retrieve the id of an attribute
 * of a given cluster given its name
 *
 * @param cluster_id - the id corresponding to the ZCL cluster number
 * @param attribute_name - the string representing the name of the attribute
 * @param str_length - the length of attribute_name
 * @param attribute_id - the location to copy the id corresponding to
 * the attribute of a given cluster
 *
 * @return SL_STATUS_OK if the type was found, and could be written.
 * SL_STATUS_FAIL otherwise
**/
sl_status_t retrieve_attribute_id_from_name(const zcl_cluster_id_t cluster_id,
                                            const char *attribute_name,
                                            unsigned int str_length,
                                            zcl_attribute_id_t *attribute_id);

/**
 * @brief attribute_val_to_string - serialize a given attribute value as a
 * string
 *
 * @param data - the pointer to the buffer where the attribute value is
 *               stored
 * @param type - the ZCL type of the attribute data
 * @param str  - the pointer where the value will be stored as a string
 * @param strlen - the size of str
 *
 * @return SL_STATUS_OK if the operation was successful, SL_STATUS_FAIL
 * otherwise
**/
sl_status_t attribute_val_to_string(uint8_t *data,
                                    enum e_zcl_attribute_type type,
                                    char *str,
                                    unsigned int strlen);

/**
 * @brief attribute_string_to_val - serialize a given attribute value from
 * its string representation
 *
 * @param value_string - the pointer to the string representation of the value
 * @param strlen - the size of value_string
 * @param type - the ZCL type of the attribute data
 * @param data - the destination buffer where to copy the value
 * @param data_size - the size available in data
 *
 * @return SL_STATUS_OK if the operation was successful, SL_STATUS_FAIL
 * otherwise
**/
sl_status_t attribute_string_to_val(const char *value_string,
                                    unsigned int strlen,
                                    enum e_zcl_attribute_type type,
                                    uint8_t *data,
                                    size_t data_size);
#endif  // COMPILE_UNUSED_CODE

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  //ATTRIBUTES_INFO_H
/** @} end attribute_info */
