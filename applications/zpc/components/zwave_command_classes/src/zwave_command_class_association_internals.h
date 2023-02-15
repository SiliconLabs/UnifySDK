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
#include "zwave_command_class_association_types.h"
#include "attribute_store.h"

// First index of the frame containing association data for (Multi Channel) Association Reports
#define REPORT_ASSOCIATION_BYTES_INDEX 5
#define ASSOCIATION_REPORTS_REPORTS_TO_FOLLOW 4


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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Listens to SUPPORTED_GROUPINGS value updates and create all the supported
 * association groups based on the value of the SUPPORTED_GROUPINGS
 *
 * @param updated_node  SUPPORTED_GROUPINGS node
 * @param change        Type of Attribute Store change
 *
 */
void zwave_command_class_association_on_supported_groupings_update(
  attribute_store_node_t updated_node, attribute_store_change_t change);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_ASSOCIATION_INTERNALS_H
/** @} end zwave_command_class_association_internals */
