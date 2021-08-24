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

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "sl_status.h"

#include "zigpc_common_observable.h"

/**
 * @brief General value for index not found
 *
 */
#define ZIGPC_OBSERVER_INDEX_NOT_FOUND -1

/**
 * @brief return index of the registered observable event, if found
 *
 * NOTE: observable is assumed to be not NULL since this function is
 * statically scoped and their callers should do the NULL checking
 *
 * @param observable Observable instance
 * @param event event to find
 * @return int8_t index of observer, -1 if not found
 */
static int8_t
  zigpc_observable_get_event_index(const struct zigpc_observable *observable,
                                   const zigpc_observer_event_t event)
{
  int8_t index = ZIGPC_OBSERVER_INDEX_NOT_FOUND;

  for (int8_t i = 0; (i < observable->event_count)
                     && (index == ZIGPC_OBSERVER_INDEX_NOT_FOUND);
       i++) {
    if (observable->event_list[i].event == event) {
      index = i;
    }
  }

  return index;
}

/**
 * @brief return index of the observer callback, if found
 *
 * @param event_entry Observable event entry
 * @param observer Callback to find
 * @return int8_t index of observer callback, -1 if not found
 */
static int8_t zigpc_observable_get_event_callback_index(
  const struct zigpc_observable_event_entry *event_entry,
  const zigpc_observer_callback_t observer)
{
  int8_t index = ZIGPC_OBSERVER_INDEX_NOT_FOUND;
  for (int8_t i = 0; (i < ZIGPC_OBSERVER_EVENT_CALLBACKS_MAX)
                     && (index == ZIGPC_OBSERVER_INDEX_NOT_FOUND);
       i++) {
    if (event_entry->callbacks[i] == observer) {
      index = i;
    }
  }
  return index;
}

sl_status_t zigpc_observable_notify(const struct zigpc_observable *observable,
                                    const zigpc_observer_event_t event,
                                    void *data)
{
  sl_status_t result = SL_STATUS_OK;
  int8_t event_index;

  if (observable == NULL) {
    result = SL_STATUS_INVALID_PARAMETER;
  }

  if (result == SL_STATUS_OK) {
    event_index = zigpc_observable_get_event_index(observable, event);
    if (event_index == ZIGPC_OBSERVER_INDEX_NOT_FOUND) {
      result = SL_STATUS_INVALID_TYPE;
    }
  }

  if (result == SL_STATUS_OK) {
    for (uint8_t i = 0; i < ZIGPC_OBSERVER_EVENT_CALLBACKS_MAX; i++) {
      if (observable->event_list[event_index].callbacks[i] != NULL) {
        observable->event_list[event_index].callbacks[i](data);
      }
    }
    result = SL_STATUS_OK;
  }

  return result;
}
/**
 * @brief Insert observer callback to be part of observable event
 *
 * @param event_entry Observable event instance.
 * @param callback Event callback to insert.
 * @return sl_status_t SL_STATUS on succes, SL_STATUS_NO_MORE_RESOURCE if the
 * list is full. Fill callback at the first empty spot.
 *
 */
static sl_status_t zigpc_observable_insert_event_callback(
  struct zigpc_observable_event_entry *event_entry,
  const zigpc_observer_callback_t callback)
{
  sl_status_t result  = SL_STATUS_NO_MORE_RESOURCE;
  bool entry_inserted = false;

  for (uint8_t i = 0;
       (i < ZIGPC_OBSERVER_EVENT_CALLBACKS_MAX) && (!entry_inserted);
       i++) {
    if (event_entry->callbacks[i] == NULL) {
      event_entry->callbacks[i] = callback;
      result                    = SL_STATUS_OK;
      entry_inserted            = true;
    }
  }
  return result;
}

sl_status_t zigpc_observable_register(struct zigpc_observable *observable,
                                      const zigpc_observer_event_t event,
                                      zigpc_observer_callback_t callback)
{
  int8_t event_index;
  int8_t event_observer_index;
  sl_status_t result = SL_STATUS_OK;

  if ((observable == NULL) || (callback == NULL)) {
    result = SL_STATUS_INVALID_PARAMETER;
  } else {
    event_index = zigpc_observable_get_event_index(observable, event);
    if (event_index == ZIGPC_OBSERVER_INDEX_NOT_FOUND) {
      result = SL_STATUS_INVALID_TYPE;
    }
  }

  if (result == SL_STATUS_OK) {
    event_observer_index = zigpc_observable_get_event_callback_index(
      &observable->event_list[event_index],
      callback);
    if (event_observer_index == ZIGPC_OBSERVER_INDEX_NOT_FOUND) {
      result = zigpc_observable_insert_event_callback(
        &observable->event_list[event_index],
        callback);
    } else {
      /* Do nothing if this callback is already registered */
      result = SL_STATUS_OK;
    }
  }

  return result;
}

sl_status_t
  zigpc_observable_unregister(struct zigpc_observable *observable,
                              const zigpc_observer_event_t event,
                              const zigpc_observer_callback_t callback)
{
  int8_t event_index;
  int8_t event_observer_index;
  sl_status_t result = SL_STATUS_OK;

  if ((observable == NULL) || (callback == NULL)) {
    result = SL_STATUS_INVALID_PARAMETER;
  }

  if (result == SL_STATUS_OK) {
    event_index = zigpc_observable_get_event_index(observable, event);
    if (event_index == ZIGPC_OBSERVER_INDEX_NOT_FOUND) {
      result = SL_STATUS_INVALID_TYPE;
    }
  }

  if (result == SL_STATUS_OK) {
    event_observer_index = zigpc_observable_get_event_callback_index(
      &observable->event_list[event_index],
      callback);
    if (event_observer_index == ZIGPC_OBSERVER_INDEX_NOT_FOUND) {
      /* If the observer is not present, inform unregistration
      * is successful anyway.
      */
      result = SL_STATUS_OK;
    } else {
      observable->event_list[event_index].callbacks[event_observer_index]
        = NULL;
      result = SL_STATUS_OK;
    }
  }

  return result;
}

/**
 * @brief Check if observable event list has duplicate entries
 *
 * @param event_list List to check
 * @param event_count List length
 * @return true if duplicate entries found
 * @return false if all entries are unique
 */
static inline bool zigpc_observable_event_list_has_duplicates(
  const zigpc_observer_event_t *event_list, const uint8_t event_count)
{
  bool duplicate_found = false;
  for (uint8_t i = 0; (i < event_count) && (!duplicate_found); i++) {
    for (uint8_t j = 0; (j < event_count) && (!duplicate_found); j++) {
      if ((event_list[j] == event_list[i]) && (j != i)) {
        duplicate_found = true;
      }
    }
  }
  return duplicate_found;
}

sl_status_t zigpc_observable_init(struct zigpc_observable *observable,
                                  const zigpc_observer_event_t *event_list,
                                  const uint8_t event_count)
{
  sl_status_t result = SL_STATUS_OK;

  if ((observable == NULL) || (event_list == NULL) || (event_count == 0U)) {
    result = SL_STATUS_INVALID_PARAMETER;
  }

  if ((result == SL_STATUS_OK) && (event_count > ZIGPC_OBSERVER_EVENTS_MAX)) {
    result = SL_STATUS_WOULD_OVERFLOW;
  }

  if ((result == SL_STATUS_OK)
      && (zigpc_observable_event_list_has_duplicates(event_list, event_count)
          == true)) {
    result = SL_STATUS_INVALID_CONFIGURATION;
  }

  if (result == SL_STATUS_OK) {
    for (uint8_t event_index = 0; event_index < event_count; event_index++) {
      observable->event_list[event_index].event = event_list[event_index];
      for (uint8_t i = 0; i < ZIGPC_OBSERVER_EVENT_CALLBACKS_MAX; i++) {
        observable->event_list[event_index].callbacks[i] = NULL;
      }
    }
    observable->event_count = event_count;
  }

  return result;
}

void zigpc_observable_clear(struct zigpc_observable *observable)
{
  /**
   * Instead of clearing all events and callbacks associated with the events,
   * the event_count field is cleared since the rest of the API use this field
   * to manage the observers.
   *
   */
  observable->event_count = 0U;
}
