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
 * @defgroup multi_channel_association_command_class Multi Channel Association Command Class
 * @ingroup command_classes
 * @brief Multi Channel Association Command Class handlers and control functions
 *
 * The Multi Channel Association and Association Command Class share the same
 * list of attributes, as the association groups and their content are the same
 * resource, that can be modified with either Command Class
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_H
#define ZWAVE_COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_H

#include "sl_status.h"
#include "attribute_store.h"

///< Index for parsing commands
#define GROUPING_REPORT_MINIMUM_LENGTH         3
#define GROUPING_REPORT_NUMBER_OF_GROUPS_INDEX 2

// Frame parsing indices
///> Index of the Group Identifier in MC Association Set, Get, Report and Remove
#define GROUP_IDENTIFIER_INDEX 2

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Setup fixture for @ref multi_channel_association_command_class
 *
 * This setup function will register callbacks to the Attribute Store, so that
 * it can be notified when to issue commands to the Z-Wave nodes.
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_command_class_multi_channel_association_init();

/**
 * @brief Ensures that the lifeline association or other associations are
 * set properly for a group_id node
 *
 * @param group_id_node       Attribute Store node for the Group ID.
 */
void establish_zpc_associations(attribute_store_node_t group_id_node);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_H
/** @} end zwave_command_class_multi_channel_association */
