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
  * @file zwave_command_class_association_internals.hpp C++ Internal Association helpers
  * @brief Internal Association helpers.
  *
  * @{
  */

#ifndef ZWAVE_COMMAND_CLASS_ASSOCIATION_INTERNALS_HPP
#define ZWAVE_COMMAND_CLASS_ASSOCIATION_INTERNALS_HPP

#include "zwave_command_class_association_internals.h"

// Generic C++ includes
#include <set>
#include <vector>

// We want the NODE_ID associations to be first while iterating the set.
struct association_compare {
  bool operator()(const association_t &lhs, const association_t &rhs) const
  {
    return ((lhs.type << 24) | (lhs.node_id << 8) | (lhs.endpoint_id))
           < ((rhs.type << 24) | (rhs.node_id << 8) | (rhs.endpoint_id));
  }
};

using association_set   = std::set<association_t, association_compare>;
using association_bytes = std::vector<uint8_t>;

/**
 * @brief Converts a bytestream from the attribute store to a set of associations.
 *
 * @param [in] source
 * @param [out] target
 *
 */
void get_association_list(association_bytes source, association_set &target);

/**
 * @brief Converts a bytestream from the attribute store to a set of
 * NodeID associations. Everything after the MARKER will be ignored.
 *
 * @param [in] source
 * @param [out] target
 *
 */
void get_node_id_association_list(association_bytes source,
                                  association_set &target);

/**
 * @brief Converts a set of associations to a bytestream for the attribute store.
 *
 * An empty set will result in an ASSOCIATION_MARKER bytestream.
 *
 * @param [in] source
 * @param [out] target
 *
 */
void get_association_bytestream(association_set source,
                                association_bytes &target);

/**
 * @brief Converts a set of associations to a bytestream that contains only
 * NodeID associations.
 *
 * An empty set will result in an empty bytestream.
 *
 * @param [in] source
 * @param [out] target
 *
 */
void get_node_id_association_bytestream(association_set source,
                                        association_bytes &target);

/**
 * @brief Substract Set1 from Set2.
 *
 * @param [in] set_1
 * @param [in] set_2
 * @param [out] result, set that have set_2 elements that are not in set_1
 */
void substract_association_lists(const association_set &set_1,
                                 const association_set &set_2,
                                 association_set &result);

#endif  //ZWAVE_COMMAND_CLASS_ASSOCIATION_INTERNALS_HPP
/** @} end zwave_command_class_association_internals */
