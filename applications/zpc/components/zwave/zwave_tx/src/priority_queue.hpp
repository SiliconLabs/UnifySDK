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
#ifndef PRIORITY_QUEUE_HPP
#define PRIORITY_QUEUE_HPP

// disable out of bound subscript warning. on every index there should be a test
// for out of bounds
#pragma GCC diagnostic ignored "-Warray-bounds"

#include <type_traits>
#include <functional>
#include <algorithm>
#include <cassert>

#include "sl_log.h"

/**
 * @brief Priority Queue
 *
 * This container arranges nodes in a binary tree, with the highest priority
 * (highest value) at the root of the tree. Note that this tree is <b>not</b>
 * lineair sorted! This means that searching the tree will take linear time.
 * Secondly, This class is designed to be used in memory limited environments
 * (read embedded systems). Hence this class operates without allocating any
 * heap memory.
 *
 * Since only static memory is used, the maximum size of the container needs to
 * determine beforehand. Its up to the implementor to choose a size big enough
 * to accommodate for all use-cases.
 *
 * The interface of this container should look familiar. It tries to mimic, where
 * possible, the same interface as the containers you would find in the std
 * library.
 *
 * # Performance
 *
 * * inserting  = O(log N)
 * * deleting   = O(log N)
 * * search     = O(N)
 * * get Min    = O(1)
 * * space      = O(N)
 *
 */
template<class T, int Size, class KeyGreater = std::greater<T>>
class priority_queue
{
  public:
  using value_type     = typename std::decay<T>::type;
  using iterator       = value_type *;
  using const_iterator = const value_type *;
  using size_type      = int;

  constexpr priority_queue() = default;

  /**
 * @brief inserts a value into the queue. inserts are ordered by their priority.
 * this function takes O(log n) time to do so.
 *
 * Be aware that this container cannot grow past its max size of `max_size()`.
 * this function will return false in this case!
 *
 * @returns true when insert was successful. false when there is no capacity to
 * insert item.
 */
  bool insert(value_type &&item) noexcept
  {
    // since we have a fixed memory size on the buffer. we cannot exceed `Size`
    // panic if we do.
    if (size() >= max_size()) {
      sl_log_error("priority_queue",
                   "Cannot insert a new item since the "
                   "max capacity of %d is reached!",
                   max_size());
      return false;
    }

    // if there is only one item in the queue, we dont have to `heapify`
    // anything.
    if (size() == 0) {
      init_iterators();
      *last = item;
      return true;
    }

    // assign item to the next slot. first increase `last` then assign.
    *(++last) = item;

    // validate the binary heap by moving up the tree. stop when there is a
    // parent which has not a higher priority.
    size_type current = size() - 1;
    size_type parent  = parent_index(current);

    KeyGreater greater;
    while (current != 0 && greater(elements[current], elements[parent])) {
      std::iter_swap(elements + parent, elements + current);
      current = parent;
      parent  = parent_index(current);
    }
    return true;
  }

  /**
 * @brief takes the first item in the queue and returns it. The item is deleted
 *  from the queue. It panics if the queue is empty.
 *
 *  after this operation the priority in the queue is maintained.
 *
 *  Performance: O(log n)
 *
 * @returns the first item in the queue.
 */
  value_type pop_front() noexcept
  {
    assert(!empty());

    // erase returns the iterator following the erased iterator. if that element
    // is the last in the queue, it will point to `nullptr`. since we know that
    // the value is still at spot `elements[0]` return it!.
    if (const auto it = erase(begin()); it == nullptr) {
      return *elements;
    }

    // The first element was swapped to the last postition. Even though from
    // outside of this class, reading `end()` is invalid. we know for sure it
    // contains the previous element. we can safely create a copy of it here.
    return *end();
  }

  /**
 * @brief removes an item from the queue by iterator. Performance: O(log n)
 *
 * @return the iterator following the last removed iterator. Note that the
 * following iterator means the iterator which is next to the current iterator
 * in memory, not the child of the current iterator from a binary tree
 * perspective.
 */
  iterator erase(const_iterator iterator)
  {
    assert(!empty());
    if (size() > 1) {
      std::iter_swap(const_cast<T *>(iterator), last--);

      size_type current = iterator - begin();
      size_type child   = biggest_child(current);

      while (child < size()
             && KeyGreater()(elements[child], elements[current])) {
        std::iter_swap(elements + current, elements + child);
        current = child;
        child   = biggest_child(current);
      }
    } else {
      clear();
      // if there is no item anymore in the queue, begin and end point to
      // nullptr.
      return end();
    }

    // we need to return the next iterator here. because we deleted the current
    // iterator in the tree, the next node collapsed onto the same spot.
    // therefore return the same iterator again.
    return const_cast<priority_queue::iterator>(iterator);
  }

  /**
* @returns number of allocated items in the queue.
*/
  constexpr size_type size() const noexcept
  {
    return end() - begin();
  }

  /**
* @returns the maximum size this queue can grow.
*/
  constexpr size_type max_size() const noexcept
  {
    return Size;
  }

  /**
   * @brief returns if the queue is empty. in this case both `begin()` and
     `end()` point to `nullptr`
* @returns true if there are no items in the queue. false otherwise.
*/
  constexpr bool empty() const noexcept
  {
    return last == nullptr;
  }

  /**
   * @brief if queue != `empty()` it returns a pointer to the beginning of the
     queue. The item with the highest priority
* @returns the iterator to the beginning of the queue. `nullptr` in the case the
  queue is empty
*/
  constexpr iterator begin() noexcept
  {
    return first;
  }

  /**
   * @brief iterator that points past the last element in the queue.
* @returns the iterator to the next element after last in the queue. `nullptr`
  in the case the queue is empty
*/
  constexpr iterator end() noexcept
  {
    if (!last) {
      return last;
    }
    return last + 1;
  }

  /**
   * @brief if queue != `empty()` it returns a pointer to the beginning of the
     queue. The item with the highest priority
* @returns the iterator to the beginning of the queue. `nullptr` in the case the
  queue is empty
*/
  constexpr const_iterator begin() const noexcept
  {
    return first;
  }

  /**
   * @brief iterator that points past the last element in the queue.
* @returns the iterator to the next element after last in the queue. `nullptr`
  in the case the queue is empty
*/
  constexpr const_iterator end() const noexcept
  {
    if (!last) {
      return last;
    }

    return last + 1;
  }

  /**
   * @brief remove all items from the queue.
   *
   * Performance : O(1)
*/
  constexpr void clear() noexcept
  {
    first = nullptr;
    last  = nullptr;
  }

  private:
  constexpr size_type parent_index(size_type i) const
  {
    return (i - 1) / 2;
  }

  constexpr size_type left_index(size_type i) const
  {
    return 2 * i + 1;
  }

  constexpr size_type right_index(size_type i) const
  {
    return 2 * i + 2;
  }

  inline constexpr size_type biggest_child(size_type current) const
  {
    size_type left  = left_index(current);
    size_type right = right_index(current);

    if (right < size() && KeyGreater()(elements[right], elements[left])) {
      return right;
    }
    return left;
  }

  inline void init_iterators()
  {
    first = elements;
    last  = elements;
  }

  value_type elements[Size];
  iterator last  = nullptr;
  iterator first = nullptr;
};

#endif  // PRIORITY_QUEUE_HPP