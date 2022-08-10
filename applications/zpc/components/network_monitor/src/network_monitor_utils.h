/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef NETWORK_MONITOR_UTILS_H
#define NETWORK_MONITOR_UTILS_H

// Unify components
#include "attribute_store.h"

// ZPC Components
#include "zwave_utils.h"

// Interfaces
#include "zwave_generic_types.h"

#ifdef __cplusplus
#include <functional>

// the following retain code should be moved to a c++ utility
// library.
namespace __private
{
/**
 * @brief removes elements within the begin and end iterators where predicate =
 * false. it uses stack memory to cache items to delete.
 *
 * its in a private namespace since this function is used by retain and is not
 * to be exposed publicly
 *
 * @param sequence given sequence to retain elements on. e.g. std::map,
 * std::vector
 * @param begin iterator to the begin sequence to iterate
 * @param end   iterator to the end of the sequence to iterate
 * @param predicate function that defines the retain condition.
 */
template<class Sequence> void retain_recursive(
  Sequence &sequence,
  typename Sequence::iterator &begin,
  const typename Sequence::iterator &end,
  const std::function<bool(const typename Sequence::const_iterator &)> predicate)
{
  constexpr int delete_buffer_size                                = 16;
  typename Sequence::const_iterator to_delete[delete_buffer_size] = {};
  size_t delete_index                                             = 0;

  while (begin != end) {
    if (delete_index < delete_buffer_size) {
      if (!predicate(begin)) {
        to_delete[++delete_index] = begin;
      }
    } else {
      retain_recursive(sequence, begin, end, predicate);
      break;
    }
    ++begin;
  }

  // remove from back to first in order to not invalidate iterators
  for (size_t i = delete_index; i > 0; --i) {
    sequence.erase(to_delete[i]);
  }
}
}  // namespace __private

/**
 * @brief modifing sequence operation that iterates over a list and removes
 *  items that do not match the given predicate. e.g. it retains items where
 *  `predicate` equals `true`
 *
 * @param sequence given sequence to retain elements on. e.g. std::map,
 * std::vector
 * @param predicate function that defines the retain condition.
 */
template<class Sequence> static void retain(
  Sequence &sequence,
  const std::function<bool(const typename Sequence::const_iterator &)> predicate)
{
  auto begin     = sequence.begin();
  const auto end = sequence.end();
  __private::retain_recursive(sequence, begin, end, predicate);
}

extern "C" {
#endif

/**
 * @brief Checks if we had a long tx/rx inactivity that would result in
 * an NL node falling asleep.
 *
 * @param node_id         Attribute Store Node for the NodeID
 * @param inactive_time   time span in seconds of the node being inactive
 *
 * @returns true if the last tx/rx is too old and we should consider the node
 *          asleep now.
 */
bool network_monitor_is_node_asleep_due_to_inactivity(
  attribute_store_node_t node_id_node, int32_t inactive_time);

/**
 * @brief Gets the Z-Wave operating mode () from the Attribute Store Node for the NodeID
 *
 * @param node_id_node        NodeID node in the attribute store.
 * @returns zwave_operating_mode_t
 */
zwave_operating_mode_t
  network_monitor_get_operating_mode(attribute_store_node_t node_id_node);

#ifdef __cplusplus
}
#endif
#endif