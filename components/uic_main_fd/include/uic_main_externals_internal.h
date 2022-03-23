/* # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

#ifndef UIC_MAIN_EXTERNALS_INTERNAL_H
#define UIC_MAIN_EXTERNALS_INTERNAL_H

/* contiki includes */
#include "contiki-conf.h"
#include "process.h"
#include "lib/list.h"

#include "uic_main_externals.h"

/**
 * @addtogroup uic_main_ext
 * @{
 */
/**
 * @defgroup uic_main_ext_int Internal Interfaces of the Unify External Event Demultiplexer
 *
 * Mainly used to interface between the generic and the
 * platform-specific parts of the component.
 */

/* TODO-maybe: error handler? */
/** Internal structure to store the external event handlers.
 */
typedef struct {
  /** Contiki list */
  list_t *list;
  /** The function that should be called when events are detected on fd. */
  uic_ext_event_handler_t ev_hndl_func;
  /** The process that should be polled when events are detected on fd. */
  struct process *owning_process;
  /** Interface descriptor for the external interface. */
  int fd;
} uic_main_ext_fd_registration_t;

/** Function type for the registration handler functions.
 *
 * Used in the interface to the platform-specific code.
 */
typedef void (*fd_handler_t)(const uic_main_ext_fd_registration_t *reg);

/** Synchronous I/O multiplexing similar to posix select().
 *
 * Monitors all the external interfaces registered in this component.
 *
 * If an event happens on one of the interfaces, the registered
 * handler will be called.  Then the function returns.
 *
 * If no events happen within \p delay \ref clock_time_t units, the
 * function times out and returns.
 *
 * @param delay Maximum time the call will block, in \ref clock_time_t units.
 */
void uic_main_ext_select(clock_time_t delay);

/** Invoke the handlers in \p reg.
 *
 * Should be called from the platform-specific code in
 * uic_main_ext_platform.c when something happened on the registered
 * external interface.
 *
 * @param reg The external event registration whose handlers should be called.
 */
void uic_main_ext_call_handler(const uic_main_ext_fd_registration_t *reg);

/** Iterator function for the registrations.
 *
 * Run \p iter_function on all registrations in \ref LIST(fd_registrations).
 *
 * @param iter_function The function to map over the registrations.
 */
void uic_main_ext_registration_iter(fd_handler_t iter_function);

/**
 * @}
 */
#endif
