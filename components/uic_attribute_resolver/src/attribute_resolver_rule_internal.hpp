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
 * @file attribute_resolver_rule_internal.hpp
 * @brief TODO: Write brief for attribute_resolver_rule_internal
 *
 * TODO: Write component description for attribute_resolver_rule_internal
 *
 * @{
 */

#ifndef ATTRIBUTE_RESOLVER_RULE_INTERNAL_HPP
#define ATTRIBUTE_RESOLVER_RULE_INTERNAL_HPP

#include <set>

#include "attribute_resolver_rule.h"

/**
 * @brief Get a list of attribute types that has the same resolution method as a
 * given attribute type. 
 *
 * Given an attribute type return a list of attributes which will be resolved by
 * the same frame. Note that this only works if there is a one to one relation
 * between attribute type ids and frame rules. Ie, this does not hold true for
 * the ZWAVE VERSION attribute, as here we have many attributes( one for each
 * zwave command class), which are all resolved by a VERSION_COMMAND_CLASS_GET
 *
 * @param rule_type Set or get resolution
 * @param group A collection of all the other attribute in the group
 */
std::set<attribute_store_node_t>
  attribute_resolver_rule_get_group_nodes(resolver_rule_type_t rule_type,
                                          attribute_store_node_t group);

#endif  //ATTRIBUTE_RESOLVER_RULE_INTERNAL_HPP
/** @} end attribute_resolver_rule_internal */
