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
 * @file attribute_resolver_internal.h
 * @brief Private internal functions and definitions for the Attribute resolver.
 *
 * This is a list of private functions used by the Attribute Resolver.
 *
 * @{
 */

#ifndef ATTRIBUTE_RESOLVER_INTERNAL_H
#define ATTRIBUTE_RESOLVER_INTERNAL_H

#include "process.h"
/**
 * @brief Name the of @ref contiki process for the Attribute Resolver.
 *
 * This is used to register the name of theAttribute Resolver Process.
 */
PROCESS_NAME(attribute_resolver_process);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Callback function to inform the attribute resolver that a rule execution
 *  is done for a node
 *
 * @param node                The node for which the rule is conpleted.
 * @param transmission_time   The time in ms that it took, to execute the rule.
 *                            set this to 0 if no data is available.
 */
void on_resolver_rule_execute_complete(attribute_store_node_t node,
                                       clock_time_t transmission_time);

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_RESOLVER_INTERNAL_H
/** @} end attribute_resolver_internal */
