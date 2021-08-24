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
 * @file zigpc_common_observable.h
 * @defgroup zigpc_common_observable ZigPC Common Observable
 * @ingroup zigpc_common
 * @brief A Common Observable module allows components to maintain their list of
 * observers without the need to set up the overhead to register, unregister, and
 * notify observers.
 *
 * Observers are defined as callback functions (zigpc_observer_callback_t) invoked
 * based on the event type it's registered with (zigpc_observer_event_t).
 *
 * The Observable API uses the struct zigpc_observable to allow
 * components(subjects) to store their own specific list of observers.
 *
 * <b>Example usage :</b>
 *
 * A component that wants to notify others should initialize the observer list.
 * The component should also pass the list of events that can be allowed for
 * observers to register with.
 *
 * my_component:
 * <div>
 *  <code>
 *    struct struct zigpc_observable my_observable;<br>
 *    zigpc_observer_event_t my_observable_events [3] = {1, 5, 7};<br>
 *    <br>
 *    zigpc_observable_init(&my_observable, my_observable_events, 3);<br>
 *  </code>
 * </div>
 *
 * Now, my_component can use the Observable API to delegate observer
 * management.
 * <div>
 *  <code>
 *    void my_component_register_observer(event, observer_cb) {<br>
 *      zigpc_observable_register(&my_observable, event, observer_cb);<br>
 *    }<br>
 *    <br>
 *    void my_component_unregister_observer(event, observer_cb) {<br>
 *      zigpc_unregister_observer(&my_observable, event, observer_cb);<br>
 *    }<br>
 *  </code>
 * </div>
 *
 *
 * Now a user of my_component can register observer function(s) based on the event
 * type that is desired.
 *
 * other_component:
 * <div>
 *  <code>
 *    void other_component_event_5_cb(void* data);<br>
 *    void other_component_event_1_cb(void* data);<br>
 *    <br>
 *    my_component_register_observer(5, other_component_event_5_cb);<br>
 *    my_component_register_observer(1, other_component_event_1_cb);<br>
 *  </code>
 * </div>
 *
 * After this, other_component_event_5_cb() will be called whenever
 * my_component notifies with event type 5.
 *
 * other_component should de-register callbacks if the events are no longer to
 * be received.
 *
 * other_component:
 * <div>
 *  <code>
 *    my_component_unregister_observer(5, other_component_event_5_cb);<br>
 *    my_component_unregister_observer(1, other_component_event_1_cb);<br>
 *  </code>
 * </div>
 *
 * To stop my_component from notifying of any events, call the clean API
 *
 * my_component:
 * <div>
 *  <code>
 *    zigpc_unregister_clean(&my_observable);<br>
 *  </code>
 * </div>
 *
 * @{
 */

#ifndef ZIGPC_COMMON_OBSERVER_H
#define ZIGPC_COMMON_OBSERVER_H

#include <stdint.h>

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maximum number of events supported by an observable instance
 *
 */
#define ZIGPC_OBSERVER_EVENTS_MAX 20

/**
 * @brief Maximum number of listeners per event supported by an observable instance
 *
 */
#define ZIGPC_OBSERVER_EVENT_CALLBACKS_MAX 5

/**
 * @brief Observer event type
 *
 */
typedef uint8_t zigpc_observer_event_t;

/**
 * @brief Observer callback that is called by subjects when notifying
 * observers of new events
 *
 * @param event Notify event
 * @param data Notify data
 *
 */
typedef void (*zigpc_observer_callback_t)(void *data);

struct zigpc_observable_event_entry {
  zigpc_observer_event_t event;
  zigpc_observer_callback_t callbacks[ZIGPC_OBSERVER_EVENT_CALLBACKS_MAX];
};

/**
 * @brief Observable instance
 *
 */
struct zigpc_observable {
  struct zigpc_observable_event_entry event_list[ZIGPC_OBSERVER_EVENTS_MAX];
  uint8_t event_count;
};

/**
 * @brief Inform Observers of observable instance of an event with data.
 *
 * @param observable Observable instance
 * @param event Event to call registered callbacks
 * @param data Data to provide to callbacks (Can be NULL)
 * @return sl_status_t SL_STATUS_OK on success
 * SL_STATUS_INVALID_PARAMETER on invalid observable instance
 * SL_STATUS_INVALID_TYPE if event is not registered
 */
sl_status_t zigpc_observable_notify(const struct zigpc_observable *observable,
                                    const zigpc_observer_event_t event,
                                    void *data);
/**
 * @brief Register an observer with a corresponding event to observable instance.
 *
 * If the observer is already registered, nothing will be done.
 *
 * @param observable Observable instance
 * @param event Event to register callback with
 * @param callback New observer callback to register
 * @return sl_status_t SL_STATUS_OK on success
 * SL_STATUS_INVALID_PARAMETER on invalid observable instance or callback
 * SL_STATUS_INVALID_TYPE if event is not approved
 */
sl_status_t zigpc_observable_register(struct zigpc_observable *observable,
                                      const zigpc_observer_event_t event,
                                      zigpc_observer_callback_t callback);
/**
 * @brief Unregister an observer with a corresponding event from observable instance.
 *
 * If the observer is not found, nothing will be done.
 *
 * @param observable Observable instance
 * @param event Event callback is registered to
 * @param callback Observer callback to register
 * @return sl_status_t SL_STATUS_OK on success
 * SL_STATUS_INVALID_PARAMETER on invalid observable instance or callback
 * SL_STATUS_INVALID_TYPE if event is not approved
 */
sl_status_t
  zigpc_observable_unregister(struct zigpc_observable *observable,
                              const zigpc_observer_event_t event,
                              const zigpc_observer_callback_t callback);

/**
 * @brief Initialize observable with an approved event list.
 *
 * NOTE: the event_list should have unique entries
 *
 * @param observable Observable instance
 * @param event_list Event list to register/unregister callbacks with
 * @param event_count Length of event list
 * @return sl_status_t SL_STATUS_OK on success
 * SL_STATUS_INVALID_PARAMETER on invalid observable instance or event_list
 * SL_STATUS_WOULD_OVERFLOW if event_list cannot fit within observable instance
 * SL_STATUS_INVALID_CONFIGURATION if event_list has duplicate entries
 */
sl_status_t zigpc_observable_init(struct zigpc_observable *observable,
                                  const zigpc_observer_event_t *event_list,
                                  const uint8_t event_count);

/**
 * @brief Stop all observers from being notified.
 *
 * @param observable Observable instance
 */
void zigpc_observable_clear(struct zigpc_observable *observable);

#ifdef __cplusplus
}
#endif

#endif /* ZIGPC_COMMON_OBSERVER_H */

/** @} end zigpc_common_observable */
