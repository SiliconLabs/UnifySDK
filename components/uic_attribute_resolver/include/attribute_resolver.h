/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup attribute_resolver Attribute Resolver
 * @ingroup uic_components
 * @brief The role of the attribute resolver is to locate and resolve missing
 * information in the attribute store.
 *
 * The role of the attribute resolver is to try to fetch missing information and
 * align reported and desired values in attributes. It does so by holding a rule
 * set for attributes. Two hooks rules are allowed pr attributes,
 *
 * - A rule on how to bring the attribute desired value to the reported value. This
 *   typically maps to sending a SET command. Several rules could map to the same
 *   set command. In other words, if multiple attributes exist in the set. When
 *   the SET command is executed, it is filled with the desired value of the
 *   attribute that make up the set command.
 * - A rule on how to trigger a reported updated of an attribute, i.e., if the
 *   attribute is unresolved. This rule typically maps to a GET command.
 *
 * @{
 *
 */

#ifndef ATTRIBUTE_RESOLVER_H
#define ATTRIBUTE_RESOLVER_H

#include "sl_status.h"
#include "attribute_store.h"
#include "clock.h"
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

/** Struct to hold the configurations of attribute resolver, which contains
 * function pointer that sends frames towards protocol radio, its init
 * function, retry timeout for sending get commands, and number of its retries.
 *
 * @ingroup attribute_resolver
 */
typedef struct {
  void (*send_init)(void);
  sl_status_t (*send)(attribute_store_node_t node,
                      const uint8_t *frame_data,
                      uint16_t frame_data_len,
                      bool is_set);
  sl_status_t (*abort)(attribute_store_node_t node);
  // Retry timeout for get commands
  clock_time_t get_retry_timeout;
  // Number of times to retry sending a get
  int get_retry_count;
} attribute_resolver_config_t;

/**
 * @brief Generator function for building PAN frames
 *
 * This function prototype is used by the rules engine to
 * retrieve PAN frames that will resolve a get or set attribute.
 *
 * Resolution functions are expected to return the following return codes:
 * - SL_STATUS_OK :            A frame was copied into the frame pointer and
 *                             should be final to resolve the attribute
 * - SL_STATUS_IN_PROGRESS:    A frame was copied into the frame pointer, but
 *                             this attribute will require more frames to be
 *                             fully resolved.
 * - SL_STATUS_ALREADY_EXISTS: The Attribute was solved using some
 *                             calculation/deductions from the Attribute Store
 *                             and no frame needs to be transmitted.
 * - SL_STATUS_IS_WAITING:     The Attribute should not be resolved and should be
 *                             skipped for now.
 * - SL_STATUS_FAIL: (or other codes) No frame was copied due to an error.
 *                                    The attribute should be skipped for now.
 *
 * Function parameters:
 * - attribute_store_node_t node [in]: The attribute store node to resolve.
 *                                     Resolving functions can use this
 *                                     information to retrieve related information
 *                                     from the \ref attribute_store.
 * - uint8_t *frame [out]:             Frame pointer. The data payload for the
 *                                     resolution must be written at the address
 *                                     pointed by this variable
 * - uint16_t *frame_len [out]:        Length variable pointer. The length of the
 *                                     payload written at the frame* address must
 *                                     be written here.
 */
typedef sl_status_t (*attribute_resolver_function_t)(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_len);

/**
 * @brief Initialize the attribute resolver.
 * This function will clear the rulebook and reset the resolver state.
 *
 * @param resolver_config Attribute resolver configurations bundle
 * @returns sl_status_t
 */
sl_status_t
  attribute_resolver_init(attribute_resolver_config_t resolver_config);

/**
 * @brief Teardown the attribute resolver.
 *
 * This function will terminate the attribute resolver process.
 * @returns 0 on success, any other value in case of error.
 */
int attribute_resolver_teardown();

/**
 * @brief Register an attribute rule.
 *
 * This function registers a new rule into the resolver rule book.
 * Only one rule can exist per attribute type. Some attributes have both a set and get
 * rule, some only have a set rule, some have only a get rule, some have no
 * rules.
 *
 * @param node_type Attribute type for which these rules apply.
 * @param set_func  Generator function which will generate the SET command for
 *                  this attribute type.
 * @param get_func Generator function which will generate the GET command for
 *                 this attribute type.
 * @return sl_status_t
 */
sl_status_t
  attribute_resolver_register_rule(attribute_store_type_t node_type,
                                   attribute_resolver_function_t set_func,
                                   attribute_resolver_function_t get_func);

/**
 * @brief Pause the resolution on a given node.
 *
 * @param node   Attribute node that the resolution is paused.
 */
void attribute_resolver_pause_node_resolution(attribute_store_node_t node);

/**
 * @brief Resume the resolution on a given node.
 *
 * @param node   Attribute node that the resolution is resumed
 */
void attribute_resolver_resume_node_resolution(attribute_store_node_t node);

/**
 * @brief Register a listener to be called when a node and all its children
 * has been resolved.
 *
 * This function allows for a callback to be fired when the resolver has
 * completed and resolved a part of the attribute tree. In other words,
 *  the node passed as a parameter and all its children.
 *
 * This feature is used to send wakeup no more information for Z-Wave wakeup
 * nodes. It is also used by the network monitor to identify when the
 * node is no longer interviewed after inclusion.
 *
 * @param node Node to monitor
 * @param callback Function to invoke when the node resolution is done.
 */
void attribute_resolver_set_resolution_listener(
  attribute_store_node_t node, void (*callback)(attribute_store_node_t));

/**
 * @brief Clear a node resolve listener.
 *
 * See @ref  attribute_resolver_set_resolution_listener
 * This will clear the callback function registered for the node.
 * If the combination of node and callback is not found, it will do nothing.
 *
 * @param node Node to clear listener for
 * @param callback Callback function to clear from node
 */
void attribute_resolver_clear_resolution_listener(
  attribute_store_node_t node, void (*callback)(attribute_store_node_t));

/**
 * @brief Instructs the resolver to try to resolve a set rule that was pending
 *
 * This function will have no effect if the node is not in the pending sets.
 * It will remove a pending set from its list and try to resolve it again.
 *
 * @param node Node to resolve again.
 *
 * @returns SL_STATUS_OK if the node will be retried. SL_STATUS_NOT_FAIL otherwise
 */
sl_status_t
  attribute_resolver_restart_set_resolution(attribute_store_node_t node);

/**
 * @brief returns the current configuration for the attribute
 * resolver
 * @returns current configuration object
 */
attribute_resolver_config_t attribute_resolver_get_config();

/**
 * @brief Verifies if the provided node, or any of its child needs resolution
 *
 * This can be used in order to detect if paused node would be resolved if
 * unpaused.
 *
 * @param node The node to verify.
 *
 * @returns true if a get or set rule is to be applied on the node or a node of
 *          its subtree. false otherwise.
 */
bool attribute_resolver_node_or_child_needs_resolution(
  attribute_store_node_t node);

/**
 * @brief This function traverses attribute store to check if any parent
 *        of the attribute node is paused or not.
 *
 * @param node  Attribute node
 * @returns   true if the node or any of its parents is paused
 *            false if none if the node and none of its parents is paused
 */
bool is_node_or_parent_paused(attribute_store_node_t node);

/**
 * @brief Returns true if a node is pendign a set resolution.
 *        i.e. command has been sent and we are waiting for a state update
 *        in the attribute store.
 *
 * @param node  Attribute node
 * @returns   true if the node is pending set execution
 *            false otherwise
 */
bool is_node_pending_set_resolution(attribute_store_node_t node);

/**
 * @brief Log the state of the Attribute Resolver using \ref sl_log.
*/
void attribute_resolver_state_log();

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_RESOLVER_H
/** @} end attribute_resolver */
