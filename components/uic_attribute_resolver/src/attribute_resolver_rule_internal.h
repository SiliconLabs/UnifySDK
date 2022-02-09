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
 * @file attribute_resolver_rule_internal.h
 * @brief This module offers API for registering and executing rules
 * @ingroup attribute_resolver
 * @{
 */

#ifndef ATTRIBUTE_RESOLVER_RULE_INTERNAL_H
#define ATTRIBUTE_RESOLVER_RULE_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "attribute_resolver.h"
#include "clock.h"

typedef void (*attribute_rule_complete_t)(attribute_store_node_t, clock_time_t);

/**
 * @brief Execute a rule and send the frame
 *
 * This function performs a lookup in the rule book and executes either a
 * get rule or a set rule.
 *
 * @param node Attribute for which we should execute
 * @param set_rule true if the set rule needs to be executed false if the
 *                       get_rule needs to be executed.
 *
 * @return sl_status_t
 * - SL_STATUS_OK rule is executed the attribute_rule_complete_t callback will
 *                be called when we are done executing
 * - SL_STATUS_ALREADY_EXISTS        refer to \ref attribute_resolver_function_t
 * - SL_STATUS_NOT_FOUND             the requested rule is not in the the book
 * - SL_STATUS_NOT_BUSY              we are already executing a rule.
 * - SL_STATUS_WOULD_OVERFLOW        our buffer cannot handle the frame.
 * - SL_STATUS_NOT_READY             The send function cannot service our frame.
 * - SL_STATUS_FAIL (or other codes) refer to \ref attribute_resolver_function_t
 */
sl_status_t attribute_resolver_rule_execute(attribute_store_node_t node,
                                            bool set_rule);

void attribute_resolver_rule_register(attribute_store_type_t node_type,
                                      attribute_resolver_function_t set_func,
                                      attribute_resolver_function_t get_func);

/**
 * @brief Returns if the Rule Book can resolve a Set Rule for the attribute type
 *
 * @param node_type The attribute Store node type for which we want to know
 *                  if there is a Set rule registered.
 * @returns true if a set rule is registered
 *          false if no set rule is registered
 */
bool attribute_resolver_has_set_rule(attribute_store_type_t node_type);

/**
 * @brief Returns if the Rule Book can resolve a Get Rule for the attribute type
 *
 * @param node_type The attribute Store node type for which we want to know
 *                  if there is a Get rule registered.
 * @returns true if a Get rule is registered
 *          false if no Get rule is registered
 */
bool attribute_resolver_has_get_rule(attribute_store_type_t node_type);

/**
 * @brief Initialize the rule book
 *
 * @param compl_func Callback function which is called when we are done
 * executing a rule.
 */
void attribute_resolver_rule_init(attribute_rule_complete_t compl_func);

/**
 * @brief Check if the resolver is currently executing a rule.
 *
 * @return true Resolver is executing
 * @return false  Resolver is not executing.
 */
bool attribute_resolver_rule_busy();

/**
 * @brief Stop waiting for a send data callback for a given node.
 */
void attribute_resolver_rule_abort(attribute_store_node_t node);

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_RESOLVER_RULE_INTERNAL_H
/** @} end attribute_resolver_rule_internal */
