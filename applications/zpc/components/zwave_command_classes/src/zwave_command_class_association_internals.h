/******************************************************************************
  * # License
  * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
  ******************************************************************************
  * The licensor of this software is Silicon Laboratories Inc. Your use of this
  * software is governed by the terms of Silicon Labs Master Software License
  * Agreement (MSLA) available at
  * www.silabs.com/about-us/legal/master-software-license-agreement. This
  * software is distributed to you in Source Code format and is governed by the
  * sections of the MSLA applicable to Source Code.
  *
  *****************************************************************************/

/**
  * @defgroup zwave_command_class_association_internals Internal helpers for
  *           (Multi Channel) Associations
  * @ingroup association_command_class
  * @brief Helper functions for Associations and Multi Channel Associations.
  *
  * @{
  */

#ifndef ZWAVE_COMMAND_CLASS_ASSOCIATION_INTERNALS_H
#define ZWAVE_COMMAND_CLASS_ASSOCIATION_INTERNALS_H

#include "sl_status.h"
#include "zwave_utils.h"             // For zwave_node_id_t
#include "zwave_controller_types.h"  // For zwave_endpoint_id_t
#include "attribute_store.h"

// Association marker that separates the NodeID association and Endpoint association
// in the association report bytestream.
#define ASSOCIATION_MARKER 0x00
#define RESERVED_BYTE      0x00

// First index of the frame containing association data for (Multi Channel) Association Reports
#define REPORT_ASSOCIATION_BYTES_INDEX 5

///< Maximum content (number of bytes) for a Association group. This is aligned
///< with the storage capacity of the attribute store.
#define MAX_GROUP_CONTENT_SIZE ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH

// Frame defines
///> Report frame. Can be shared between Multi Channel Association and Association.
typedef struct association_report_frame {
  uint8_t command_class;
  uint8_t command;
  uint8_t grouping_identifier;
  uint8_t max_nodes_supported;
  uint8_t reports_to_follow;
  uint8_t group_content[MAX_GROUP_CONTENT_SIZE];
} association_report_frame_t;

///> Set/Remove frames. Can be shared between Multi Channel Association and Association.
typedef struct association_set_frame {
  uint8_t command_class;
  uint8_t command;
  uint8_t grouping_identifier;
  uint8_t group_content[MAX_GROUP_CONTENT_SIZE];
} association_set_frame_t;

///> Get frames. Can be shared between Multi Channel Association and Association.
typedef struct association_get_frame {
  uint8_t command_class;
  uint8_t command;
  uint8_t grouping_identifier;
} association_get_frame_t;

///> Grouping Report frame. Can be shared between Multi Channel Association and Association.
typedef struct association_grouping_report_frame {
  uint8_t command_class;
  uint8_t command;
  uint8_t supported_groupings;
} association_grouping_report_frame_t;

/**
 * @brief Association type possible values
 */
typedef enum { NODE_ID = 0, ENDPOINT = 1 } association_type_t;

/**
 * @brief Association object
 */
typedef struct {
  ///> Association Type
  association_type_t type;
  ///> NodeID for the association. Always valid
  zwave_node_id_t node_id;
  ///> NodeID for the association. Valid only if type=ENDPOINT
  zwave_endpoint_id_t endpoint_id;
} association_t;

///> Association Group ID Type
typedef uint8_t association_group_id_t;
///> Association Group Capacity Type
typedef uint32_t association_group_capacity_t;
///> Association Group count Type
typedef uint32_t association_group_count_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Finds the Attribute Store Node for the group content
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group identifier
 *
 * @returns attribute_store_node_t for the group content.
 *          ATTRIBUTE_STORE_INVALID_NODE if it was not found.
 */
attribute_store_node_t get_group_content_node(zwave_node_id_t node_id,
                                              zwave_endpoint_id_t endpoint_id,
                                              association_group_id_t group_id);

/**
 * @brief Returns how many groups are supported by an endpoint
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 *
 * @returns The number of supported association groups for the endpoint.
 *          0 in case of error.
 */
association_group_count_t get_number_of_groups(zwave_node_id_t node_id,
                                               zwave_endpoint_id_t endpoint_id);

/**
 * @brief Checks in the attribute store the maximum number of associations for
 *        a group.
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group identifier
 *
 * @returns 0 in case of error.
 *          Else the maximum number of associations for the group.
 */
association_group_capacity_t
  get_group_capacity(zwave_node_id_t node_id,
                     zwave_endpoint_id_t endpoint_id,
                     association_group_id_t group_id);

/**
 * @brief Adds a desired association in a group in the Attribute Store
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 * @param association       The Association object to add
 *
 * @returns SL_STATUS_OK if accepted. SL_STATUS_FAIL in case of error
 */
sl_status_t add_desired_association(zwave_node_id_t node_id,
                                    zwave_endpoint_id_t endpoint_id,
                                    association_group_id_t group_id,
                                    association_t association);

/**
 * @brief Adds a reported association in a group in the Attribute Store
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 * @param association       The Association object to add
 *
 * @returns SL_STATUS_OK if accepted. SL_STATUS_FAIL in case of error
 */
sl_status_t add_reported_association(zwave_node_id_t node_id,
                                     zwave_endpoint_id_t endpoint_id,
                                     association_group_id_t group_id,
                                     association_t association);

/**
 * @brief Adds a desired association in a group in the Attribute Store
 * and remove an existing association if needed.
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 * @param association       The Association object to add
 *
 * @returns SL_STATUS_OK if accepted. SL_STATUS_FAIL in case of error
 */
sl_status_t force_add_desired_association(zwave_node_id_t node_id,
                                          zwave_endpoint_id_t endpoint_id,
                                          association_group_id_t group_id,
                                          association_t association);

/**
 * @brief Removes a desired association in a group in the Attribute Store
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 * @param association       The Association object to remove
 *
 * @returns SL_STATUS_OK if accepted.
 *          SL_STATUS_NOT_FOUND if the association was not set
 *          for the destination.
 */
sl_status_t remove_desired_association(zwave_node_id_t node_id,
                                       zwave_endpoint_id_t endpoint_id,
                                       association_group_id_t group_id,
                                       association_t association);

/**
 * @brief Removes a reported association in a group in the Attribute Store
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 * @param association       The Association object to remove
 *
 * @returns SL_STATUS_OK if accepted.
 *          SL_STATUS_NOT_FOUND if the association was not set
 *          for the destination.
 */
sl_status_t remove_reported_association(zwave_node_id_t node_id,
                                        zwave_endpoint_id_t endpoint_id,
                                        association_group_id_t group_id,
                                        association_t association);

/**
 * @brief Removes all desired association in a group in the Attribute Store.
 * It will set the attribute to an ASSOCIATION_MARKER byte.
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 *
 * @returns SL_STATUS_OK if accepted. SL_STATUS_FAIL in case of error
 */
sl_status_t remove_all_desired_associations(zwave_node_id_t node_id,
                                            zwave_endpoint_id_t endpoint_id,
                                            association_group_id_t group_id);

/**
 * @brief Removes all reported association in a group in the Attribute Store.
 * It will set the attribute to an ASSOCIATION_MARKER byte.
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 *
 * @returns SL_STATUS_OK if accepted. SL_STATUS_FAIL in case of error
 */
sl_status_t remove_all_reported_associations(zwave_node_id_t node_id,
                                             zwave_endpoint_id_t endpoint_id,
                                             association_group_id_t group_id);

/**
 * @brief Removes all reported NodeID associations in a group in the
 * Attribute Store.
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 *
 * @returns SL_STATUS_OK if accepted. SL_STATUS_FAIL in case of error
 */
sl_status_t
  remove_all_reported_node_id_associations(zwave_node_id_t node_id,
                                           zwave_endpoint_id_t endpoint_id,
                                           association_group_id_t group_id);

/**
 * @brief Checks if the reported value of an association group is full
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 *
 * @returns True if no more association can be established
 */
bool is_reported_group_full(zwave_node_id_t node_id,
                            zwave_endpoint_id_t endpoint_id,
                            association_group_id_t group_id);

/**
 * @brief Checks if the desired value of an association group is full
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Association Group ID
 *
 * @returns True if no more association can be established
 */
bool is_desired_group_full(zwave_node_id_t node_id,
                           zwave_endpoint_id_t endpoint_id,
                           association_group_id_t group_id);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_ASSOCIATION_INTERNALS_H
/** @} end zwave_command_class_association_internals */
