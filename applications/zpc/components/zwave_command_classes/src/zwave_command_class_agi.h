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
 * @defgroup agi_command_class Association Group Information (AGI) Command Class
 * @ingroup command_classes
 * @brief Association Group Information (AGI) Command Class handlers and control functions
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_AGI_H
#define ZWAVE_COMMAND_CLASS_AGI_H

#include "sl_status.h"
#include "zwave_command_class_association_internals.h"
#include "zwave_controller_types.h"

///< How many nodes/endpoint associations do we allow in each group.
///< Do not set this to more than 126, the attribute store won't be able to store it.
#define MAX_SUPPORTED_NODES_PER_GROUP 10

///< How many groups do we support.
#define NUMBER_OF_SUPPORTED_GROUPS 1

/// Not Applicable profile value, though it is used by somes nodes for the
/// Lifeline Group
#define AGI_NA_PROFILE 0x0000

///< Lifeline Group ID
#define LIFELINE_GROUP_ID 1
#define AGI_LIFELINE_PROFILE  0x0001
#define LIFELINE_NAME     "Lifeline"

/// Profile that is reserved, used to indicate that no profile data is available
/// or valid
#define AGI_RESERVED_PROFILE 0xFFFF

// Profile values for AGI
typedef uint32_t agi_profile_t;

/// Frame parsing defines
#define AGI_INFO_REPORT_GROUP_COUNT_MASK 0x3F

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Setup fixture for the @ref agi_command_class
 *
 * This setup function will initialize our group propertyes and register
 * callbacks to the Attribute Store, so that
 * it can be notified when to issue commands to the Z-Wave nodes.
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_command_class_agi_init();

/**
 * @brief Lets a component register commands that it sends via an association group.
 *
 * @param node_id           Z-Wave Node ID
 * @param endpoint_id       Z-Wave Endpoint ID
 * @param group_id          Grouping Identifier
 *
 * @returns AGI_RESERVED_PROFILE if no AGI information. Else the profile value.
 */
agi_profile_t
  zwave_command_class_agi_get_group_profile(zwave_node_id_t node_id,
                                            zwave_endpoint_id_t endpoint_id,
                                            association_group_id_t group_id);

/**
 * @brief Lets a component register commands that it sends via an association group.
 *
 * @param group_id        Association Group Identifier
 * @param command_class   Command Class Identifier
 * @param command         Command Identifier
 *
 * @returns SL_STATUS_OK if the command was added to the group
 *          SL_STATUS_NOT_FOUND if the Group ID is not found
 *          SL_STATUS_FAIL in case it could not be added.
 */
sl_status_t zwave_command_class_agi_add_group_commands(
  association_group_id_t group_id,
  zwave_command_class_t command_class,
  zwave_command_t command);

/**
 * @brief Verifies if a Command Class/Command pair is allowed for an Association
 *        group
 *
 * @param group_id        Association Group Identifier
 * @param command_class   Command Class Identifier
 * @param command         Command Identifier
 *
 * @returns true if the Command Class/Command pair is allowed
 *          false if it is not allowed
 */
bool zwave_command_class_agi_is_command_allowed_for_group(
  association_group_id_t group_id,
  zwave_command_class_t command_class,
  zwave_command_t command);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_AGI_H
/** @} end zwave_command_class_agi */
